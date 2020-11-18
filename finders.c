#include "finders.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#define IS_DIR_SEP(C) ((C) == '/' || (C) == '\\')
#define stat _stat
#define mkdir(P,X) _mkdir(P)
#define S_IFDIR _S_IFDIR
#else
#define IS_DIR_SEP(C) ((C) == '/')
#endif

//==============================================================================
// Globals
//==============================================================================


Biome biomes[256];

//==============================================================================
// Saving & Loading Seeds
//==============================================================================


int64_t *loadSavedSeeds(const char *fnam, int64_t *scnt)
{
    FILE *fp = fopen(fnam, "r");

    int64_t seed, i;
    int64_t *baseSeeds;

    if (fp == NULL)
        return NULL;

    *scnt = 0;

    while (!feof(fp))
    {
        if (fscanf(fp, "%" PRId64, &seed) == 1) (*scnt)++;
        else while (!feof(fp) && fgetc(fp) != '\n');
    }

    if (*scnt == 0)
        return NULL;

    baseSeeds = (int64_t*) calloc(*scnt, sizeof(*baseSeeds));

    rewind(fp);

    for (i = 0; i < *scnt && !feof(fp);)
    {
        if (fscanf(fp, "%" PRId64, &baseSeeds[i]) == 1) i++;
        else while (!feof(fp) && fgetc(fp) != '\n');
    }

    fclose(fp);

    return baseSeeds;
}



//==============================================================================
// Finding Structure Positions
//==============================================================================


static int testOutpostPos(int64_t s, int cx, int cz)
{
    s ^= (cx >> 4) ^ ( (cz >> 4) << 4 );
    setSeed(&s);
    next(&s, 32);
    return nextInt(&s, 5) == 0;
}

Pos getStructurePos(StructureConfig config, int64_t seed, int regX, int regZ, int *valid)
{
    Pos pos = {0,0};
    if (valid) *valid = 0;

    if (config.properties == 0)
    {
        pos = getFeaturePos(config, seed, regX, regZ);
        if (valid)
        {
            if (config.structType == Outpost)
            {
                *valid = testOutpostPos(seed, pos.x >> 4, pos.z >> 4);
                // Outposts also require that there are no villages nearby.
                // However, before 1.16 this would include a biome check, so it
                // should be tested for in the position viability check.
            }
            else
            {
                *valid = 1;
            }
        }
    }
    else if (config.properties == LARGE_STRUCT)
    {
        if ((config.chunkRange & (config.chunkRange-1)))
        {
            pos = getLargeStructurePos(config, seed, regX, regZ);
            if (valid) *valid = 1;
        }
    }

    return pos;
}

int isMineshaftChunk(int64_t seed, int chunkX, int chunkZ)
{
    int64_t s = seed;
    setSeed(&s);
    int64_t i = nextLong(&s);
    int64_t j = nextLong(&s);
    s = chunkX * i ^ chunkZ * j ^ seed;
    setSeed(&s);
    return nextDouble(&s) < 0.004;
}

int isTreasureChunk(int64_t seed, int chunkX, int chunkZ)
{
    seed = chunkX*341873128712 + chunkZ*132897987541 + seed + TREASURE_CONFIG.salt;
    setSeed(&seed);
    return nextFloat(&seed) < 0.01;
}



//==============================================================================
// Multi-Structure Checks
//==============================================================================

// TODO: accurate seed testers for two or three structures in range


/* Searches for the optimal AFK position given four structures at positions 'p',
 * each of volume (ax,ay,az).
 *
 * Returned is the number of spawning spaces within reach.
 */
int countBlocksInSpawnRange(Pos p[4], int ax, int ay, int az, Pos *afk)
{
    int minX = 3e7, minZ = 3e7, maxX = -3e7, maxZ = -3e7;
    int bestr, bestn, i, x, z, px, pz;


    // Find corners
    for (i = 0; i < 4; i++)
    {
        if (p[i].x < minX) minX = p[i].x;
        if (p[i].z < minZ) minZ = p[i].z;
        if (p[i].x > maxX) maxX = p[i].x;
        if (p[i].z > maxZ) maxZ = p[i].z;
    }


    // assume that the search area is bound by the inner corners
    maxX += ax;
    maxZ += az;
    bestr = 0;
    bestn = 0;

    double thsq = 128.0*128.0 - ay*ay/4.0;

    for (x = minX; x < maxX; x++)
    {
        for (z = minZ; z < maxZ; z++)
        {
            int inrange = 0;

            for (i = 0; i < 4; i++)
            {
                double dx = p[i].x - (x);
                double dz = p[i].z - (z);

                for (px = 0; px < ax; px++)
                {
                    for (pz = 0; pz < az; pz++)
                    {
                        double ddx = px + dx;
                        double ddz = pz + dz;
                        inrange += (ddx*ddx + ddz*ddz <= thsq);
                    }
                }
            }

            if (inrange > bestr)
            {
                if (afk)
                {
                    afk->x = x;
                    afk->z = z;
                    bestn = 1;
                }
                bestr = inrange;
            }
            else if (inrange == bestr)
            {
                if (afk)
                {
                    afk->x += x;
                    afk->z += z;
                    bestn++;
                }
            }
        }
    }

    if (afk && bestn)
    {
        afk->x /= bestn;
        afk->z /= bestn;
    }

    return bestr;
}


#define MAX_PATHLEN 4096

STRUCT(linked_seeds_t)
{
    int64_t seeds[100];
    size_t len;
    linked_seeds_t *next;
};

STRUCT(threadinfo_t)
{
    // seed range
    int64_t start, end;
    const int64_t *lowBits;
    int lowBitCnt;
    int lowBitN;

    // testing function
    int (*check)(int64_t, void*);
    void *data;

    // output
    char path[MAX_PATHLEN];
    FILE *fp;
    linked_seeds_t ls;
};


static int mkdirp(char *path)
{
    int err = 0, len = strlen(path);
    char *p = path;

#if defined(_WIN32)
    if (p[1] == ':') p += 2;
#endif
    while (IS_DIR_SEP(*p)) p++;

    while (!err && p < path+len)
    {
        char *q = p;
        while (*q && !IS_DIR_SEP(*q))
            q++;

        if (p != path) p[-1] = '/';
        *q = 0;

        struct stat st;
        if (stat(path, &st) == -1)
            err = mkdir(path, 0773);
        else if (!(st.st_mode & S_IFDIR))
            err = 1;

        p = q+1;
    }

    return err;
}


#ifdef USE_PTHREAD
static void *searchAll48Thread(void *data)
#else
static DWORD WINAPI searchAll48Thread(LPVOID data)
#endif
{
// TODO TEST:
// lower bits with various ranges

    threadinfo_t *info = (threadinfo_t*)data;

    int64_t seed = info->start;
    int64_t end = info->end;
    linked_seeds_t *lp = &info->ls;
    lp->len = 0;
    lp->next = NULL;

    if (info->lowBits)
    {
        int64_t hstep = 1LL << info->lowBitN;
        int64_t hmask = ~(hstep - 1);
        int64_t mid;
        int idx;

        mid = info->start & hmask;
        for (idx = 0; (seed = mid | info->lowBits[idx]) < info->start; idx++);

        while (seed <= end)
        {
            if U(info->check(seed, info->data))
            {
                if (info->fp)
                {
                    fprintf(info->fp, "%" PRId64"\n", seed);
                    fflush(info->fp);
                }
                else
                {
                    lp->seeds[lp->len] = seed;
                    lp->len++;
                    if (lp->len >= sizeof(lp->seeds)/sizeof(int64_t))
                    {
                        linked_seeds_t *n =
                            (linked_seeds_t*) malloc(sizeof(linked_seeds_t));
                        if (n == NULL)
                            exit(1);
                        lp->next = n;
                        lp = n;
                        lp->len = 0;
                        lp->next = NULL;
                    }
                }
            }

            idx++;
            if (idx >= info->lowBitCnt)
            {
                idx = 0;
                mid += hstep;
            }

            seed = mid | info->lowBits[idx];
        }
    }
    else
    {
        while (seed <= end)
        {
            if U(info->check(seed, info->data))
            {
                if (info->fp)
                {
                    fprintf(info->fp, "%" PRId64"\n", seed);
                    fflush(info->fp);
                }
                else
                {
                    lp->seeds[lp->len] = seed;
                    lp->len++;
                    if (lp->len >= sizeof(lp->seeds)/sizeof(int64_t))
                    {
                        linked_seeds_t *n =
                            (linked_seeds_t*) malloc(sizeof(linked_seeds_t));
                        if (n == NULL)
                            exit(1);
                        lp->next = n;
                        lp = n;
                        lp->len = 0;
                        lp->next = NULL;
                    }
                }
            }
            seed++;
        }
    }

#ifdef USE_PTHREAD
    pthread_exit(NULL);
#endif
    return 0;
}


int searchAll48(
        int64_t **          seedbuf,
        int64_t *           buflen,
        const char *        path,
        int                 threads,
        const int64_t *     lowBits,
        int                 lowBitCnt,
        int                 lowBitN,
        int (*check)(int64_t s48, void *data),
        void *              data
        )
{
    threadinfo_t *info = (threadinfo_t*) malloc(threads* sizeof(*info));
    thread_id_t *tids = (thread_id_t*) malloc(threads* sizeof(*tids));
    int i, t;
    int err = 0;

    if (path)
    {
        size_t pathlen = strlen(path);
        char dpath[MAX_PATHLEN];

        // split path into directory and file and create missing directories
        if (pathlen + 8 >= sizeof(dpath))
            goto L_ERR;
        strcpy(dpath, path);

        for (i = pathlen-1; i >= 0; i--)
        {
            if (IS_DIR_SEP(dpath[i]))
            {
                dpath[i] = 0;
                if (mkdirp(dpath))
                    goto L_ERR;
                break;
            }
        }
    }
    else if (seedbuf == NULL || buflen == NULL)
    {
        // no file and no buffer return: no output possible
        goto L_ERR;
    }

    // prepare the thread info and load progress if present
    for (t = 0; t < threads; t++)
    {
        info[t].start = (t * (MASK48+1) / threads);
        info[t].end = ((t+1) * (MASK48+1) / threads - 1);
        info[t].lowBits = lowBits;
        info[t].lowBitCnt = lowBitCnt;
        info[t].lowBitN = lowBitN;
        info[t].check = check;
        info[t].data = data;

        if (path)
        {
            // progress file of this thread
            snprintf(info[t].path, sizeof(info[t].path), "%s.part%d", path, t);
            FILE *fp = fopen(info[t].path, "a+");
            if (fp == NULL)
                goto L_ERR;

            int c, nnl = 0;
            char buf[32];

            // find the last newline
            for (i = 1; i < 32; i++)
            {
                if (fseek(fp, -i, SEEK_END)) break;
                c = fgetc(fp);
                if (c <= 0 || (nnl && c == '\n')) break;
                nnl |= (c != '\n');
            }

            if (i < 32 && !fseek(fp, 1-i, SEEK_END) && fread(buf, i-1, 1, fp) > 0)
            {
                // read the last entry, and replace the start seed accordingly
                int64_t lentry;
                if (sscanf(buf, "%" PRId64, &lentry) == 1)
                {
                    info[t].start = lentry;
                    printf("Continuing thread %d at seed %" PRId64 "\n",
                        t, lentry);
                }
            }

            fseek(fp, 0, SEEK_END);
            info[t].fp = fp;
        }
        else
        {
            info[t].path[0] = 0;
            info[t].fp = NULL;
        }
    }


    // run the threads
#ifdef USE_PTHREAD

    for (t = 0; t < threads; t++)
    {
        pthread_create(&tids[t], NULL, searchAll48Thread, (void*)&info[t]);
    }

    for (t = 0; t < threads; t++)
    {
        pthread_join(tids[t], NULL);
    }

#else

    for (t = 0; t < threads; t++)
    {
        tids[t] = CreateThread(NULL, 0, searchAll48Thread,
            (LPVOID)&info[t], 0, NULL);
    }

    WaitForMultipleObjects(threads, tids, TRUE, INFINITE);

#endif

    if (path)
    {
        // merge partial files
        FILE *fp = fopen(path, "w");
        if (fp == NULL)
            goto L_ERR;

        for (t = 0; t < threads; t++)
        {
            rewind(info[t].fp);

            char buffer[4097];
            size_t n;
            while ((n = fread(buffer, sizeof(char), 4096, info[t].fp)))
            {
                if (!fwrite(buffer, sizeof(char), n, fp))
                {
                    fclose(fp);
                    goto L_ERR;
                }
            }

            fclose(info[t].fp);
            remove(info[t].path);
        }

        fclose(fp);

        if (seedbuf && buflen)
        {
            *seedbuf = loadSavedSeeds(path, buflen);
        }
    }
    else
    {
        // merge linked seed buffers
        *buflen = 0;

        for (t = 0; t < threads; t++)
        {
            linked_seeds_t *lp = &info[t].ls;
            do
            {
                *buflen += lp->len;
                lp = lp->next;
            }
            while (lp);
        }

        *seedbuf = (int64_t*) malloc((*buflen) * sizeof(int64_t));
        if (*seedbuf == NULL)
            exit(1);

        i = 0;
        for (t = 0; t < threads; t++)
        {
            linked_seeds_t *lp = &info[t].ls;
            do
            {
                memcpy(*seedbuf + i, lp->seeds, lp->len * sizeof(int64_t));
                i += lp->len;
                linked_seeds_t *tmp = lp;
                lp = lp->next;
                if (tmp != &info[t].ls)
                    free(tmp);
            }
            while (lp);
        }
    }

    if (0)
L_ERR:
        err = 1;

    free(tids);
    free(info);

    return err;
}



//==============================================================================
// Checking Biomes & Biome Helper Functions
//==============================================================================


int getBiomeAtPos(const LayerStack *g, const Pos pos)
{
    int *map = allocCache(g->entry_1, 1, 1);
    genArea(g->entry_1, map, pos.x, pos.z, 1, 1);
    int biomeID = map[0];
    free(map);
    return biomeID;
}

Pos findBiomePosition(
        const int mcversion,
        const Layer *l,
        int *cache,
        const int centerX,
        const int centerZ,
        const int range,
        const char *isValid,
        int64_t *seed,
        int *passes
        )
{
    int x1 = (centerX-range) >> 2;
    int z1 = (centerZ-range) >> 2;
    int x2 = (centerX+range) >> 2;
    int z2 = (centerZ+range) >> 2;
    int width  = x2 - x1 + 1;
    int height = z2 - z1 + 1;
    int *map;
    int i, j, found;

    Pos out;

    if (l->scale != 4)
    {
        printf("WARN findBiomePosition: require scale = 4, but have %d.\n",
                l->scale);
    }

    map = cache ? cache : allocCache(l, width, height);

    genArea(l, map, x1, z1, width, height);

    out.x = centerX;
    out.z = centerZ;
    found = 0;

    if (mcversion >= MC_1_13)
    {
        for (i = 0, j = 2; i < width*height; i++)
        {
            if (!biomeExists(map[i]) || !isValid[map[i]]) continue;
            if ((found == 0 || nextInt(seed, j++) == 0))
            {
                out.x = (x1 + i%width) << 2;
                out.z = (z1 + i/width) << 2;
                found = 1;
            }
        }
        found = j - 2;
    }
    else
    {
        for (i = 0; i < width*height; i++)
        {
            if (biomeExists(map[i]) && isValid[map[i]] &&
                (found == 0 || nextInt(seed, found + 1) == 0))
            {
                out.x = (x1 + i%width) << 2;
                out.z = (z1 + i/width) << 2;
                ++found;
            }
        }
    }


    if (cache == NULL)
    {
        free(map);
    }

    if (passes != NULL)
    {
        *passes = found;
    }

    return out;
}


int areBiomesViable(
        const Layer *       l,
        int *               cache,
        const int           posX,
        const int           posZ,
        const int           radius,
        const char *        isValid
        )
{
    int x1 = (posX - radius) >> 2;
    int z1 = (posZ - radius) >> 2;
    int x2 = (posX + radius) >> 2;
    int z2 = (posZ + radius) >> 2;
    int width = x2 - x1 + 1;
    int height = z2 - z1 + 1;
    int i;
    int *map;
    int viable;

    if (l->scale != 4)
    {
        printf("WARN areBiomesViable: require scale = 4, but have %d.\n",
                l->scale);
    }

    map = cache ? cache : allocCache(l, width, height);
    viable = !genArea(l, map, x1, z1, width, height);

    if (viable)
    {
        for (i = 0; i < width*height; i++)
        {
            if (!biomeExists(map[i]) || !isValid[ map[i] ])
            {
                viable = 0;
                break;
            }
        }
    }

    if (cache == NULL)
        free(map);
    return viable;
}


int getBiomeRadius(
        const int *         map,
        const int           mapSide,
        const int *         biomes,
        const int           bnum,
        const int           ignoreMutations)
{
    int r, i, b;
    int blist[0x100];
    int mask = ignoreMutations ? 0x7f : 0xff;
    int radiusMax = mapSide / 2;

    if ((mapSide & 1) == 0)
    {
        printf("WARN getBiomeRadius: Side length of the square map should be an odd integer.\n");
    }

    memset(blist, 0, sizeof(blist));

    for (r = 1; r < radiusMax; r++)
    {
        for (i = radiusMax-r; i <= radiusMax+r; i++)
        {
            blist[ map[(radiusMax-r) * mapSide+ i]    & mask ] = 1;
            blist[ map[(radiusMax+r-1) * mapSide + i] & mask ] = 1;
            blist[ map[mapSide*i + (radiusMax-r)]     & mask ] = 1;
            blist[ map[mapSide*i + (radiusMax+r-1)]   & mask ] = 1;
        }

        for (b = 0; b < bnum && blist[biomes[b] & mask]; b++);
        if (b >= bnum)
        {
            break;
        }
    }

    return r != radiusMax ? r : -1;
}



//==============================================================================
// Finding Strongholds and Spawn
//==============================================================================


const char* getValidStrongholdBiomes()
{
    static char validStrongholdBiomes[256];

    if (!validStrongholdBiomes[plains])
    {
        int id;
        for (id = 0; id < 256; id++)
        {
            if (biomeExists(id) && biomes[id].height > 0.0)
                validStrongholdBiomes[id] = 1;
        }
    }

    return validStrongholdBiomes;
}

Pos initFirstStronghold(StrongholdIter *sh, int mc, int64_t s48)
{
    double dist, angle;
    int64_t rnds;
    Pos p;

    rnds = s48;
    setSeed(&rnds);

    angle = 2.0 * PI * nextDouble(&rnds);
    if (mc >= MC_1_9)
        dist = (4.0 * 32.0) + (nextDouble(&rnds) - 0.5) * 32 * 2.5;
    else
        dist = (1.25 + nextDouble(&rnds)) * 32.0;

    p.x = ((int)round(cos(angle) * dist) << 4) + 8;
    p.z = ((int)round(sin(angle) * dist) << 4) + 8;

    if (sh)
    {
        sh->pos.x = sh->pos.z = 0;
        sh->nextapprox = p;
        sh->index = 0;
        sh->ringnum = 0;
        sh->ringmax = 3;
        sh->ringidx = 0;
        sh->angle = angle;
        sh->dist = dist;
        sh->rnds = rnds;
        sh->mc = mc;
    }

    return p;
}

int nextStronghold(StrongholdIter *sh, const LayerStack *g, int *cache)
{
    sh->pos = findBiomePosition(sh->mc, &g->layers[L_RIVER_MIX_4], cache,
        sh->nextapprox.x, sh->nextapprox.z, 112, getValidStrongholdBiomes(),
        &sh->rnds, NULL);

    sh->ringidx++;
    sh->angle += 2 * PI / sh->ringmax;

    if (sh->ringidx == sh->ringmax)
    {
        sh->ringnum++;
        sh->ringidx = 0;
        sh->ringmax = sh->ringmax + 2*sh->ringmax / (sh->ringnum+1);
        if (sh->ringmax > 128-sh->index)
            sh->ringmax = 128-sh->index;
        sh->angle += nextDouble(&sh->rnds) * PI * 2.0;
    }

    if (sh->mc >= MC_1_9)
    {
        sh->dist = (4.0 * 32.0) + (6.0 * sh->ringnum * 32.0) +
            (nextDouble(&sh->rnds) - 0.5) * 32 * 2.5;
    }
    else
    {
        sh->dist = (1.25 + nextDouble(&sh->rnds)) * 32.0;
    }

    sh->nextapprox.x = ((int)round(cos(sh->angle) * sh->dist) << 4) + 8;
    sh->nextapprox.z = ((int)round(sin(sh->angle) * sh->dist) << 4) + 8;
    sh->index++;

    return (sh->mc >= MC_1_9 ? 128 : 3) - (sh->index-1);
}

int findStrongholds(const int mcversion, const LayerStack *g, int *cache,
        Pos *locations, int64_t worldSeed, int maxSH, int maxRing)
{
    const char *validStrongholdBiomes = getValidStrongholdBiomes();
    int i, x, z;
    double distance;

    int currentRing = 0;
    int currentCount = 0;
    int perRing = 3;

    setSeed(&worldSeed); // PRNG
    double angle = nextDouble(&worldSeed) * PI * 2.0;

    const Layer *l = &g->layers[L_RIVER_MIX_4];

    if (mcversion >= MC_1_9)
    {
        if (maxSH <= 0) maxSH = 128;

        for (i = 0; i < maxSH; i++)
        {
            distance = (4.0 * 32.0) + (6.0 * currentRing * 32.0) +
                (nextDouble(&worldSeed) - 0.5) * 32 * 2.5;

            x = (int)round(cos(angle) * distance);
            z = (int)round(sin(angle) * distance);

            locations[i] = findBiomePosition(mcversion, l, cache,
                    (x << 4) + 8, (z << 4) + 8, 112, validStrongholdBiomes,
                    &worldSeed, NULL);

            angle += 2 * PI / perRing;

            currentCount++;
            if (currentCount == perRing)
            {
                // Current ring is complete, move to next ring.
                currentRing++;
                if (currentRing == maxRing)
                {
                    i++;
                    break;
                }

                currentCount = 0;
                perRing = perRing + 2*perRing/(currentRing+1);
                if (perRing > 128-i)
                    perRing = 128-i;
                angle = angle + nextDouble(&worldSeed) * PI * 2.0;
            }
        }
    }
    else
    {
        if (maxSH <= 0) maxSH = 3;

        for (i = 0; i < maxSH; i++)
        {
            distance = (1.25 + nextDouble(&worldSeed)) * 32.0;

            x = (int)round(cos(angle) * distance);
            z = (int)round(sin(angle) * distance);

            locations[i] = findBiomePosition(mcversion, l, cache,
                    (x << 4) + 8, (z << 4) + 8, 112, validStrongholdBiomes,
                    &worldSeed, NULL);

            angle += 2 * PI / 3.0;
        }
    }

    return i;
}


static double getGrassProbability(int64_t seed, int biome, int x, int z)
{
    (void) seed, (void) biome, (void) x, (void) z;
    // TODO: Use ChunkGeneratorOverworld.generateHeightmap for better estimate.
    // TODO: Try to determine the actual probabilities and build a statistic.
    switch (biome)
    {
    case plains:                        return 1.0;
    case mountains:                     return 0.8; // height dependent
    case forest:                        return 1.0;
    case taiga:                         return 1.0;
    case swamp:                         return 0.6; // height dependent
    case river:                         return 0.5;
    case beach:                         return 0.1;
    case wooded_hills:                  return 1.0;
    case taiga_hills:                   return 1.0;
    case mountain_edge:                 return 1.0; // height dependent
    case jungle:                        return 1.0;
    case jungle_hills:                  return 1.0;
    case jungle_edge:                   return 1.0;
    case birch_forest:                  return 1.0;
    case birch_forest_hills:            return 1.0;
    case dark_forest:                   return 0.9;
    case snowy_taiga:                   return 0.2; // below trees
    case snowy_taiga_hills:             return 0.2; // below trees
    case giant_tree_taiga:              return 0.6;
    case giant_tree_taiga_hills:        return 0.6;
    case wooded_mountains:              return 0.2; // height dependent
    case savanna:                       return 1.0;
    case savanna_plateau:               return 1.0;
    case wooded_badlands_plateau:       return 0.1; // height dependent
    case badlands_plateau:              return 0.1; // height dependent

    case sunflower_plains:              return 1.0;
    case gravelly_mountains:            return 0.2;
    case flower_forest:                 return 1.0;
    case taiga_mountains:               return 1.0;
    case swamp_hills:                   return 0.9;
    case modified_jungle:               return 1.0;
    case modified_jungle_edge:          return 1.0;
    case tall_birch_forest:             return 1.0;
    case tall_birch_hills:              return 1.0;
    case dark_forest_hills:             return 0.9;
    case snowy_taiga_mountains:         return 0.2;
    case giant_spruce_taiga:            return 0.6;
    case giant_spruce_taiga_hills:      return 0.6;
    case modified_gravelly_mountains:   return 0.2;
    case shattered_savanna:             return 1.0;
    case shattered_savanna_plateau:     return 1.0;
    case bamboo_jungle:                 return 0.4;
    case bamboo_jungle_hills:           return 0.4;
    // NOTE: in rare circumstances you can get also get grassy islands that are
    // completely in ocean variants...
    default: return 0;
    }
}

static int canCoordinateBeSpawn(const int64_t seed, const LayerStack *g, int *cache, Pos pos)
{
    (void) cache;
    int biome = getBiomeAtPos(g, pos);
    return getGrassProbability(seed, biome, pos.x, pos.z) >= 0.5;
}


static const char* getValidSpawnBiomes()
{
    static const int biomesToSpawnIn[] = {forest, plains, taiga, taiga_hills, wooded_hills, jungle, jungle_hills};
    static char isValid[256];
    unsigned int i;

    if (!isValid[biomesToSpawnIn[0]])
        for (i = 0; i < sizeof(biomesToSpawnIn) / sizeof(int); i++)
            isValid[ biomesToSpawnIn[i] ] = 1;

    return isValid;
}


Pos getSpawn(const int mcversion, const LayerStack *g, int *cache, int64_t worldSeed)
{
    const char *isSpawnBiome = getValidSpawnBiomes();
    Pos spawn;
    int found;
    int i;

    const Layer *l = &g->layers[L_RIVER_MIX_4];

    setSeed(&worldSeed);
    spawn = findBiomePosition(mcversion, l, cache, 0, 0, 256, isSpawnBiome,
            &worldSeed, &found);

    if (!found)
    {
        //printf("Unable to find spawn biome.\n");
        spawn.x = spawn.z = 8;
    }

    if (mcversion >= MC_1_13)
    {
        // TODO: The 1.13 section may need further checking!
        int n2 = 0;
        int n3 = 0;
        int n4 = 0;
        int n5 = -1;

        for (i = 0; i < 1024; i++)
        {
            if (n2 > -16 && n2 <= 16 && n3 > -16 && n3 <= 16)
            {
                int cx = ((spawn.x >> 4) + n2) << 4;
                int cz = ((spawn.z >> 4) + n3) << 4;
                int i2, i3;

                for (i2 = cx; i2 <= cx+15; i2++)
                {
                    for (i3 = cz; i3 <= cz+15; i3++)
                    {
                        Pos pos = {i2, i3};
                        if (canCoordinateBeSpawn(worldSeed, g, cache, pos))
                        {
                            return pos;
                        }
                    }
                }
            }

            if (n2 == n3 || (n2 < 0 && n2 == - n3) || (n2 > 0 && n2 == 1 - n3))
            {
                int n7 = n4;
                n4 = - n5;
                n5 = n7;
            }
            n2 += n4;
            n3 += n5;
        }
    }
    else
    {
        for (i = 0; i < 1000 && !canCoordinateBeSpawn(worldSeed, g, cache, spawn); i++)
        {
            spawn.x += nextInt(&worldSeed, 64) - nextInt(&worldSeed, 64);
            spawn.z += nextInt(&worldSeed, 64) - nextInt(&worldSeed, 64);
        }
    }

    return spawn;
}


Pos estimateSpawn(const int mcversion, const LayerStack *g, int *cache, int64_t worldSeed)
{
    const char *isSpawnBiome = getValidSpawnBiomes();
    Pos spawn;
    int found;

    const Layer *l = &g->layers[L_RIVER_MIX_4];

    setSeed(&worldSeed);
    spawn = findBiomePosition(mcversion, l, cache, 0, 0, 256, isSpawnBiome,
            &worldSeed, &found);

    if (!found)
    {
        spawn.x = spawn.z = 8;
    }

    return spawn;
}



//==============================================================================
// Validating Structure Positions
//==============================================================================


int isViableFeatureBiome(int structureType, int biomeID)
{
    switch (structureType)
    {
    case Desert_Pyramid:
        return biomeID == desert || biomeID == desert_hills;
    case Jungle_Pyramid:
        return (biomeID == jungle || biomeID == jungle_hills ||
                biomeID == bamboo_jungle || biomeID == bamboo_jungle_hills);
    case Swamp_Hut:
        return biomeID == swamp;
    case Igloo:
        return biomeID == snowy_tundra || biomeID == snowy_taiga;
    case Ocean_Ruin:
        return isOceanic(biomeID);
    case Shipwreck:
        return isOceanic(biomeID) || biomeID == beach || biomeID == snowy_beach;
    case Ruined_Portal:
        return 1;
    case Treasure:
        return (biomeID == beach || biomeID == snowy_beach ||
                biomeID == stone_shore || biomeID == mushroom_field_shore);
    case Monument:
        return isOceanic(biomeID);
    case Village:
    case Outpost:
        // differs across MC versions
        return (biomeID == plains || biomeID == desert ||
                biomeID == savanna || biomeID == taiga ||
                biomeID == snowy_taiga || biomeID == snowy_tundra);
    case Mansion:
        return biomeID == dark_forest || biomeID == dark_forest_hills;
    default:
        fprintf(stderr, "ERR isViableFeatureBiome: not implemented for structure type.\n");
        exit(1);
    }
    return 0;
}

static const char *getValidMonumentBiomes1()
{
    static const int oceanMonumentBiomeList1[] =
    {
            ocean, deep_ocean, river, frozen_river,
            frozen_ocean, deep_frozen_ocean, cold_ocean, deep_cold_ocean,
            lukewarm_ocean, deep_lukewarm_ocean, warm_ocean, deep_warm_ocean
    };
    static char isValid[256];
    unsigned int i;

    if (!isValid[oceanMonumentBiomeList1[0]])
        for (i = 0; i < sizeof(oceanMonumentBiomeList1) / sizeof(int); i++)
            isValid[ oceanMonumentBiomeList1[i] ] = 1;

    return isValid;
}

static const char *getValidMonumentBiomes2()
{
    static const int oceanMonumentBiomeList2[] =
    {
            deep_frozen_ocean, deep_cold_ocean, deep_ocean,
            deep_lukewarm_ocean, deep_warm_ocean
    };
    static char isValid[256];
    unsigned int i;

    if (!isValid[oceanMonumentBiomeList2[0]])
        for (i = 0; i < sizeof(oceanMonumentBiomeList2) / sizeof(int); i++)
            isValid[ oceanMonumentBiomeList2[i] ] = 1;

    return isValid;
}

static const char *getValidMansionBiomes()
{
    static const int mansionBiomeList[] = {dark_forest, dark_forest+128};
    static char isValid[256];
    unsigned int i;

    if (!isValid[mansionBiomeList[0]])
        for (i = 0; i < sizeof(mansionBiomeList) / sizeof(int); i++)
            isValid[ mansionBiomeList[i] ] = 1;

    return isValid;
}


static int mapViableBiome(const Layer * l, int * out, int x, int z, int w, int h)
{
    int err = mapBiome(l, out, x, z, w, h);
    if U(err != 0)
        return err;

    int styp = * (const int*) l->data;
    int i, j;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int biomeID = out[i + w*j];
            switch (styp)
            {
            case Desert_Pyramid:
                if (biomeID == desert || getBiomeType(biomeID) == Mesa)
                    return 0;
                break;
            case Jungle_Pyramid:
                if (biomeID == jungle)
                    return 0;
                break;
            case Swamp_Hut:
                if (biomeID == swamp)
                    return 0;
                break;
            case Igloo:
                if (biomeID == snowy_tundra || biomeID == snowy_taiga)
                    return 0;
                break;
            case Ocean_Ruin:
            case Shipwreck:
            case Monument:
                if (isOceanic(biomeID))
                    return 0;
                break;
            case Mansion:
                if (biomeID == dark_forest)
                    return 0;
                break;
            default:
                return 0;
            }
        }
    }

    return 1; // required biomes not found: set err status to stop generator
}

static int mapViableShore(const Layer * l, int * out, int x, int z, int w, int h)
{
    int err = mapShore(l, out, x, z, w, h);
    if U(err != 0)
        return err;

    int styp = * (const int*) l->data;
    int i, j;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int biomeID = out[i + w*j];
            switch (styp)
            {
            case Desert_Pyramid:
            case Jungle_Pyramid:
            case Swamp_Hut:
            case Igloo:
            case Ocean_Ruin:
            case Shipwreck:
            case Village:
            case Monument:
            case Mansion:
                if (isViableFeatureBiome(styp, biomeID))
                    return 0;
                break;

            default:
                return 0;
            }
        }
    }

    return 1;
}


int isViableStructurePos(int structureType, int mcversion, LayerStack *g,
        int64_t seed, int blockX, int blockZ)
{
    int *map = NULL;
    Layer *l;
    int biome;
    int viable;

    int64_t chunkX = blockX >> 4;
    int64_t chunkZ = blockZ >> 4;

    Layer lbiome = g->layers[L_BIOME_256];
    Layer lshore = g->layers[L_SHORE_16];

    g->layers[L_BIOME_256].data = (void*) &structureType;
    g->layers[L_BIOME_256].getMap = mapViableBiome;
    g->layers[L_SHORE_16].data = (void*) &structureType;
    g->layers[L_SHORE_16].getMap = mapViableShore;

    switch (structureType)
    {
    case Desert_Pyramid:
    case Jungle_Pyramid:
    case Swamp_Hut:
    case Igloo:
    case Ocean_Ruin:
    case Shipwreck:
    case Treasure:
        if (mcversion < MC_1_16)
        {
            l = &g->layers[L_VORONOI_ZOOM_1];
        }
        else
        {   // In 1.16 the position and layer for the biome dependence changed
            // to the centre of a chunk at scale 4. Using L_RIVER_MIX_4
            // (without ocean type) should be fine for ruins and wrecks.
            l = &g->layers[L_RIVER_MIX_4];
            blockX = (chunkX << 2) + 2;
            blockZ = (chunkZ << 2) + 2;
        }
        setWorldSeed(l, seed);
        map = allocCache(l, 1, 1);
        if (genArea(l, map, blockX, blockZ, 1, 1))
            goto L_NOT_VIABLE;
        if (!isViableFeatureBiome(structureType, map[0]))
            goto L_NOT_VIABLE;
        goto L_VIABLE;

    case Village:
        if (mcversion < MC_1_16)
        {   // TODO: check this (and if it makes a difference)
            blockX >>= 2;
            blockZ >>= 2;
        }
        else
        {
            blockX = (chunkX << 2) + 2;
            blockZ = (chunkZ << 2) + 2;
        }
        l = &g->layers[L_RIVER_MIX_4];
        setWorldSeed(l, seed);
        map = allocCache(l, 1, 1);
        if (genArea(l, map, blockX, blockZ, 1, 1))
            goto L_NOT_VIABLE;
        biome = map[0];
        if (biome == plains || biome == desert || biome == savanna || biome == taiga)
            goto L_VIABLE;
        else if (mcversion >= MC_1_14 && biome == snowy_tundra)
            goto L_VIABLE;
        else if (mcversion == MC_BE && biome == snowy_taiga)
            goto L_VIABLE;
        else
            goto L_NOT_VIABLE;

    case Outpost:
    {
        if (!testOutpostPos(seed, chunkX, chunkZ))
            goto L_NOT_VIABLE;
        if (mcversion < MC_1_16)
        {
            l = &g->layers[L_VORONOI_ZOOM_1];
        }
        else
        {
            l = &g->layers[L_RIVER_MIX_4];
            blockX = (chunkX << 2) + 2;
            blockZ = (chunkZ << 2) + 2;
        }
        setWorldSeed(l, seed);
        map = allocCache(l, 1, 1);
        if (genArea(l, map, blockX, blockZ, 1, 1))
            goto L_NOT_VIABLE;
        biome = map[0];
        // TODO: support for MC_BE
        if (biome != plains && biome != desert && biome != taiga && biome != snowy_tundra && biome != savanna)
            goto L_NOT_VIABLE;
        // look for villages within 10 chunks
        int cx0 = (chunkX-10), cx1 = (chunkX+10);
        int cz0 = (chunkZ-10), cz1 = (chunkZ+10);
        int rx, rz;
        for (rz = cz0 >> 5; rz <= cz1 >> 5; rz++)
        {
            for (rx = cx0 >> 5; rx <= cx1 >> 5; rx++)
            {
                Pos p = getFeaturePos(VILLAGE_CONFIG, seed, rx, rz);
                int cx = p.x >> 4, cz = p.z >> 4;
                if (cx >= cx0 && cx <= cx1 && cz >= cz0 && cz <= cz1)
                {
                    if (mcversion >= MC_1_16)
                        goto L_NOT_VIABLE;
                    if (isViableStructurePos(Village, mcversion, g, seed, p.x, p.z))
                        goto L_NOT_VIABLE;
                    goto L_VIABLE;
                }
            }
        }
        goto L_VIABLE;
    }

    case Monument:
        if (mcversion >= MC_1_9)
        {
            // Monuments require two viability checks with the ocean layer
            // branch => worth checking for potential deep ocean beforehand.
            l = &g->layers[L_SHORE_16];
            setWorldSeed(l, seed);
            map = allocCache(l, 1, 1);
            if (genArea(l, map, chunkX, chunkZ, 1, 1))
                goto L_NOT_VIABLE;
        }
        else
        {
            // In 1.8 monuments require only a single deep ocean block.
            l = g->entry_1;
            setWorldSeed(l, seed);
            map = allocCache(l, 1, 1);
            if (genArea(l, map, blockX, blockZ, 1, 1))
                goto L_NOT_VIABLE;
        }
        if (!isDeepOcean(map[0]))
            goto L_NOT_VIABLE;
        if (mcversion >= MC_1_13)
            l = &g->layers[L13_OCEAN_MIX_4];
        else
            l = &g->layers[L_RIVER_MIX_4];
        setWorldSeed(l, seed);
        if (mcversion < MC_1_9 || areBiomesViable(l, NULL, blockX, blockZ, 16, getValidMonumentBiomes2()))
            if (areBiomesViable(l, NULL, blockX, blockZ, 29, getValidMonumentBiomes1()))
                goto L_VIABLE;
        goto L_NOT_VIABLE;

    case Mansion:
        l = &g->layers[L_RIVER_MIX_4];
        setWorldSeed(l, seed);
        if (areBiomesViable(l, NULL, blockX, blockZ, 32, getValidMansionBiomes()))
            goto L_VIABLE;
        goto L_NOT_VIABLE;

    case Ruined_Portal:
        goto L_VIABLE;

    default:
        fprintf(stderr, "ERR isViableStructurePos: validation for structure type not implemented");
        goto L_NOT_VIABLE;
    }

L_NOT_VIABLE:
    viable = 0;
    if (0) {
L_VIABLE:
        viable = 1;
    }

    g->layers[L_BIOME_256] = lbiome;
    g->layers[L_SHORE_16] = lshore;
    if (map)
        free(map);

    return viable;
}


//==============================================================================
// Finding Properties of Structures
//==============================================================================


int isZombieVillage(const int mcversion, const int64_t worldSeed,
        const int regionX, const int regionZ)
{
    Pos pos;
    int64_t seed = worldSeed;

    if (mcversion < MC_1_10)
    {
        printf("Warning: Zombie villages were only introduced in MC 1.10.\n");
    }

    // get the chunk position of the village
    seed = regionX*341873128712 + regionZ*132897987541 + seed + VILLAGE_CONFIG.salt;
    seed = (seed ^ 0x5deece66dLL);// & ((1LL << 48) - 1);

    seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    pos.x = (seed >> 17) % VILLAGE_CONFIG.chunkRange;

    seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    pos.z = (seed >> 17) % VILLAGE_CONFIG.chunkRange;

    pos.x += regionX * VILLAGE_CONFIG.regionSize;
    pos.z += regionZ * VILLAGE_CONFIG.regionSize;

    // jump to the random number check that determines whether this is village
    // is zombie infested
    int64_t rnd = chunkGenerateRnd(worldSeed, pos.x , pos.z);
    skipNextN(&rnd, mcversion == MC_1_13 ? 10 : 11);

    return nextInt(&rnd, 50) == 0;
}


int64_t getHouseList(const int64_t worldSeed, const int chunkX, const int chunkZ,
        int *out)
{
    int64_t rnd = chunkGenerateRnd(worldSeed, chunkX, chunkZ);
    skipNextN(&rnd, 1);

    out[HouseSmall] = nextInt(&rnd, 4 - 2 + 1) + 2;
    out[Church]     = nextInt(&rnd, 1 - 0 + 1) + 0;
    out[Library]    = nextInt(&rnd, 2 - 0 + 1) + 0;
    out[WoodHut]    = nextInt(&rnd, 5 - 2 + 1) + 2;
    out[Butcher]    = nextInt(&rnd, 2 - 0 + 1) + 0;
    out[FarmLarge]  = nextInt(&rnd, 4 - 1 + 1) + 1;
    out[FarmSmall]  = nextInt(&rnd, 4 - 2 + 1) + 2;
    out[Blacksmith] = nextInt(&rnd, 1 - 0 + 1) + 0;
    out[HouseLarge] = nextInt(&rnd, 3 - 0 + 1) + 0;

    return rnd;
}

//==============================================================================
// Seed Filters
//==============================================================================


BiomeFilter setupBiomeFilter(const int *biomeList, int listLen)
{
    BiomeFilter bf;
    int i, id;

    memset(&bf, 0, sizeof(bf));

    for (i = 0; i < listLen; i++)
    {
        id = biomeList[i];
        if (id & ~0xbf) // i.e. not in ranges [0,64),[128,192)
        {
            fprintf(stderr, "ERR: biomeID=%d not supported by filter.\n", id);
            exit(-1);
        }

        switch (id)
        {
        case mushroom_fields:
            // mushroom shores can generate with hills and at rivers
            bf.raresToFind |= (1ULL << mushroom_fields);
            // fall through
        case mushroom_field_shore:
            bf.tempsToFind |= (1ULL << Oceanic);
            bf.majorToFind |= (1ULL << mushroom_fields);
            bf.riverToFind |= (1ULL << id);
            break;

        case badlands_plateau:
        case wooded_badlands_plateau:
        case badlands:
        case eroded_badlands:
        case modified_badlands_plateau:
        case modified_wooded_badlands_plateau:
            bf.tempsToFind |= (1ULL << (Warm+Special));
            if (id == badlands_plateau || id == modified_badlands_plateau)
                bf.majorToFind |= (1ULL << badlands_plateau);
            if (id == wooded_badlands_plateau || id == modified_wooded_badlands_plateau)
                bf.majorToFind |= (1ULL << wooded_badlands_plateau);
            if (id < 128) {
                bf.raresToFind |= (1ULL << id);
                bf.riverToFind |= (1ULL << id);
            } else {
                bf.raresToFindM |= (1ULL << (id-128));
                bf.riverToFindM |= (1ULL << (id-128));
            }
            break;

        case jungle:
        case jungle_edge:
        case jungle_hills:
        case modified_jungle:
        case modified_jungle_edge:
        case bamboo_jungle:
        case bamboo_jungle_hills:
            bf.tempsToFind |= (1ULL << (Lush+Special));
            bf.majorToFind |= (1ULL << jungle);
            if (id == bamboo_jungle || id == bamboo_jungle_hills) {
                // bamboo%64 are End biomes, so we can reuse the edgesToFind
                bf.edgesToFind |= (1ULL << (bamboo_jungle & 0x3f));
            } else if (id == jungle_edge) {
                // un-modified jungle_edge can be created at shore layer
                bf.riverToFind |= (1ULL << jungle_edge);
            } else {
                if (id == modified_jungle_edge)
                    bf.edgesToFind |= (1ULL << jungle_edge);
                else
                    bf.edgesToFind |= (1ULL << jungle);
                if (id < 128) {
                    bf.raresToFind |= (1ULL << id);
                    bf.riverToFind |= (1ULL << id);
                } else {
                    bf.raresToFindM |= (1ULL << (id-128));
                    bf.riverToFindM |= (1ULL << (id-128));
                }
            }
            break;

        case giant_tree_taiga:
        case giant_tree_taiga_hills:
        case giant_spruce_taiga:
        case giant_spruce_taiga_hills:
            bf.tempsToFind |= (1ULL << (Cold+Special));
            bf.majorToFind |= (1ULL << giant_tree_taiga);
            bf.edgesToFind |= (1ULL << giant_tree_taiga);
            if (id < 128) {
                bf.raresToFind |= (1ULL << id);
                bf.riverToFind |= (1ULL << id);
            } else {
                bf.raresToFindM |= (1ULL << (id-128));
                bf.riverToFindM |= (1ULL << (id-128));
            }
            break;

        case savanna:
        case savanna_plateau:
        case shattered_savanna:
        case shattered_savanna_plateau:
        case desert_hills:
        case desert_lakes:
            bf.tempsToFind |= (1ULL << Warm);
            if (id == desert_hills || id == desert_lakes) {
                bf.majorToFind |= (1ULL << desert);
                bf.edgesToFind |= (1ULL << desert);
            } else {
                bf.majorToFind |= (1ULL << savanna);
                bf.edgesToFind |= (1ULL << savanna);
            }
            if (id < 128) {
                bf.raresToFind |= (1ULL << id);
                bf.riverToFind |= (1ULL << id);
            } else {
                bf.raresToFindM |= (1ULL << (id-128));
                bf.riverToFindM |= (1ULL << (id-128));
            }
            break;

        case dark_forest:
        case dark_forest_hills:
        case birch_forest:
        case birch_forest_hills:
        case tall_birch_forest:
        case tall_birch_hills:
        case swamp:
        case swamp_hills:
            bf.tempsToFind |= (1ULL << Lush);
            if (id == dark_forest || id == dark_forest_hills) {
                bf.majorToFind |= (1ULL << dark_forest);
                bf.edgesToFind |= (1ULL << dark_forest);
            }
            else if (id == birch_forest || id == birch_forest_hills ||
                     id == tall_birch_forest || id == tall_birch_hills) {
                bf.majorToFind |= (1ULL << birch_forest);
                bf.edgesToFind |= (1ULL << birch_forest);
            }
            else if (id == swamp || id == swamp_hills) {
                bf.majorToFind |= (1ULL << swamp);
                bf.edgesToFind |= (1ULL << swamp);
            }
            if (id < 128) {
                bf.raresToFind |= (1ULL << id);
                bf.riverToFind |= (1ULL << id);
            } else {
                bf.raresToFindM |= (1ULL << (id-128));
                bf.riverToFindM |= (1ULL << (id-128));
            }
            break;

        case snowy_taiga:
        case snowy_taiga_hills:
        case snowy_taiga_mountains:
        case snowy_tundra:
        case snowy_mountains:
        case ice_spikes:
        case frozen_river:
            bf.tempsToFind |= (1ULL << Freezing);
            if (id == snowy_taiga || id == snowy_taiga_hills ||
                id == snowy_taiga_mountains)
                bf.edgesToFind |= (1ULL << snowy_taiga);
            else
                bf.edgesToFind |= (1ULL << snowy_tundra);
            if (id == frozen_river) {
                bf.raresToFind |= (1ULL << snowy_tundra);
                bf.riverToFind |= (1ULL << id);
            } else if (id < 128) {
                bf.raresToFind |= (1ULL << id);
                bf.riverToFind |= (1ULL << id);
            } else {
                bf.raresToFindM |= (1ULL << (id-128));
                bf.riverToFindM |= (1ULL << (id-128));
            }
            break;

        case sunflower_plains:
            bf.raresToFindM |= (1ULL << (id-128));
            bf.riverToFindM |= (1ULL << (id-128));
            break;

        case snowy_beach:
            bf.tempsToFind |= (1ULL << Freezing);
            // fall through
        case beach:
        case stone_shore:
            bf.riverToFind |= (1ULL << id);
            break;

        case mountains:
            bf.majorToFind |= (1ULL << mountains);
            // fall through
        case wooded_mountains:
            bf.raresToFind |= (1ULL << id);
            bf.riverToFind |= (1ULL << id);
            break;
        case gravelly_mountains:
            bf.majorToFind |= (1ULL << mountains);
            // fall through
        case modified_gravelly_mountains:
            bf.raresToFindM |= (1ULL << (id-128));
            bf.riverToFindM |= (1ULL << (id-128));
            break;

        case taiga:
        case taiga_hills:
            bf.edgesToFind |= (1ULL << taiga);
            bf.raresToFind |= (1ULL << id);
            bf.riverToFind |= (1ULL << id);
            break;
        case taiga_mountains:
            bf.edgesToFind |= (1ULL << taiga);
            bf.raresToFindM |= (1ULL << (id-128));
            bf.riverToFindM |= (1ULL << (id-128));
            break;

        case plains:
        case forest:
        case wooded_hills:
            bf.raresToFind |= (1ULL << id);
            bf.riverToFind |= (1ULL << id);
            break;
        case flower_forest:
            bf.raresToFindM |= (1ULL << (id-128));
            bf.riverToFindM |= (1ULL << (id-128));
            break;

        case desert: // can generate at shore layer
            bf.riverToFind |= (1ULL << id);
            break;

        default:
            if (isOceanic(id)) {
                bf.tempsToFind |= (1ULL << Oceanic);
                bf.oceanToFind |= (1ULL << id);
                if (isShallowOcean(id)) {
                    bf.otempToFind |= (1ULL << id);
                } else {
                    bf.raresToFind |= (1ULL << deep_ocean);
                    bf.riverToFind |= (1ULL << deep_ocean);
                    if (id == deep_warm_ocean)
                        bf.otempToFind |= (1ULL << warm_ocean);
                    else if (id == deep_lukewarm_ocean)
                        bf.otempToFind |= (1ULL << lukewarm_ocean);
                    else if (id == deep_ocean)
                        bf.otempToFind |= (1ULL << ocean);
                    else if (id == deep_cold_ocean)
                        bf.otempToFind |= (1ULL << cold_ocean);
                    else if (id == deep_frozen_ocean)
                        bf.otempToFind |= (1ULL << frozen_ocean);
                }
            } else {
                if (id < 64)
                    bf.riverToFind |= (1ULL << id);
                else
                    bf.riverToFindM |= (1ULL << (id-128));
            }
            break;
        }
    }

    bf.shoreToFind = bf.riverToFind;
    bf.shoreToFind &= ~((1ULL << river) | (1ULL << frozen_river));
    bf.shoreToFindM = bf.riverToFindM;

    bf.specialCnt = 0;
    bf.specialCnt += !!(bf.tempsToFind & (1ULL << (Warm+Special)));
    bf.specialCnt += !!(bf.tempsToFind & (1ULL << (Lush+Special)));
    bf.specialCnt += !!(bf.tempsToFind & (1ULL << (Cold+Special)));

    return bf;
}


static int mapFilterSpecial(const Layer * l, int * out, int x, int z, int w, int h)
{
    const BiomeFilter *bf = (const BiomeFilter*) l->data;
    int i, j;
    uint64_t temps;

    /// pre-gen checks
    int specialcnt = bf->specialCnt;
    if (specialcnt > 0)
    {
        int64_t ss = l->startSeed;
        int64_t cs;

        for (j = 0; j < h; j++)
        {
            for (i = 0; i < w; i++)
            {
                cs = getChunkSeed(ss, x+i, z+j);
                if (mcFirstIsZero(cs, 13))
                    specialcnt--;
            }
        }
        if (specialcnt > 0)
            return 1;
    }

    int err = mapSpecial(l, out, x, z, w, h);
    if U(err != 0)
        return err;

    temps = 0;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int id = out[i + w*j];
            int isspecial = id & 0xf00;
            id &= ~0xf00;
            if (isspecial && id != Freezing)
               temps |= (1ULL << (id+Special));
            else
               temps |= (1ULL << id);
        }
    }

    if ((temps & bf->tempsToFind) ^ bf->tempsToFind)
        return 1;
    return 0;
}

static int mapFilterMushroom(const Layer * l, int * out, int x, int z, int w, int h)
{
    const BiomeFilter *bf = (const BiomeFilter*) l->data;
    int i, j;
    int err;

    if (w*h < 100 && (bf->majorToFind & (1ULL << mushroom_fields)))
    {
        int64_t ss = l->startSeed;
        int64_t cs;

        for (j = 0; j < h; j++)
        {
            for (i = 0; i < w; i++)
            {
                cs = getChunkSeed(ss, i+x, j+z);
                if (mcFirstIsZero(cs, 100))
                    goto L_GENERATE;
            }
        }
        return 1;
    }

L_GENERATE:
    err = mapAddMushroomIsland(l, out, x, z, w, h);
    if U(err != 0)
        return err;

    if (bf->majorToFind & (1ULL << mushroom_fields))
    {
        for (i = 0; i < w*h; i++)
            if (out[i] == mushroom_fields)
                return 0;
        return 1;
    }
    return 0;
}

static int mapFilterBiome(const Layer * l, int * out, int x, int z, int w, int h)
{
    const BiomeFilter *bf = (const BiomeFilter*) l->data;
    int i, j;
    uint64_t b;

    int err = mapBiome(l, out, x, z, w, h);
    if U(err != 0)
        return err;

    b = 0;
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int id = out[i + w*j];
            b |= (1ULL << id);
        }
    }

    if ((b & bf->majorToFind) ^ bf->majorToFind)
        return 1;
    return 0;
}

static int mapFilterOceanTemp(const Layer * l, int * out, int x, int z, int w, int h)
{
    const BiomeFilter *bf = (const BiomeFilter*) l->data;
    int i, j;
    uint64_t b;

    int err = mapOceanTemp(l, out, x, z, w, h);
    if U(err != 0)
        return err;

    b = 0;
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int id = out[i + w*j];
            b |= (1ULL << id);
        }
    }

    if ((b & bf->otempToFind) ^ bf->otempToFind)
        return 1;
    return 0;
}

static int mapFilterBiomeEdge(const Layer * l, int * out, int x, int z, int w, int h)
{
    const BiomeFilter *bf = (const BiomeFilter*) l->data;
    uint64_t b;
    int i;
    int err;

    err = mapBiomeEdge(l, out, x, z, w, h);
    if U(err != 0)
        return err;

    b = 0;
    for (i = 0; i < w*h; i++)
        b |= (1ULL << (out[i] & 0x3f));

    if ((b & bf->edgesToFind) ^ bf->edgesToFind)
        return 1;
    return 0;
}

static int mapFilterRareBiome(const Layer * l, int * out, int x, int z, int w, int h)
{
    const BiomeFilter *bf = (const BiomeFilter*) l->data;
    uint64_t b, bm;
    int i;
    int err;

    err = mapRareBiome(l, out, x, z, w, h);
    if U(err != 0)
        return err;

    b = 0; bm = 0;
    for (i = 0; i < w*h; i++)
    {
        int id = out[i];
        if (id < 128) b |= (1ULL << id);
        else bm |= (1ULL << (id-128));
    }

    if ((b & bf->raresToFind) ^ bf->raresToFind)
        return 1;
    if ((bm & bf->raresToFindM) ^ bf->raresToFindM)
        return 1;
    return 0;
}

static int mapFilterShore(const Layer * l, int * out, int x, int z, int w, int h)
{
    const BiomeFilter *bf = (const BiomeFilter*) l->data;
    uint64_t b, bm;
    int i;

    int err = mapShore(l, out, x, z, w, h);
    if U(err != 0) return err;

    b = 0; bm = 0;
    for (i = 0; i < w*h; i++)
    {
        int id = out[i];
        if (id < 128) b |= (1ULL << id);
        else bm |= (1ULL << (id-128));
    }

    if ((b & bf->shoreToFind) ^ bf->shoreToFind)
        return 1;
    if ((bm & bf->shoreToFindM) ^ bf->shoreToFindM)
        return 1;
    return 0;
}

static int mapFilterRiverMix(const Layer * l, int * out, int x, int z, int w, int h)
{
    const BiomeFilter *bf = (const BiomeFilter*) l->data;
    uint64_t b, bm;
    int i;

    int err = mapRiverMix(l, out, x, z, w, h);
    if U(err != 0) return err;

    b = 0; bm = 0;
    for (i = 0; i < w*h; i++)
    {
        int id = out[i];
        if (id < 128) b |= (1ULL << id);
        else bm |= (1ULL << (id-128));
    }

    if ((b & bf->riverToFind) ^ bf->riverToFind)
        return 1;
    if ((bm & bf->riverToFindM) ^ bf->riverToFindM)
        return 1;
    return 0;
}

static int mapFilterOceanMix(const Layer * l, int * out, int x, int z, int w, int h)
{
    const BiomeFilter *bf = (const BiomeFilter*) l->data;
    uint64_t b;
    int i;
    int err;

    if (bf->riverToFind)
    {
        err = mapRiverMix(l, out, x, z, w, h);
        if (err) return err;
    }

    err = mapOceanMix(l, out, x, z, w, h);
    if U(err != 0) return err;

    b = 0;
    for (i = 0; i < w*h; i++)
    {
        int id = out[i];
        if (id < 128) b |= (1ULL << id);
    }

    if ((b & bf->oceanToFind) ^ bf->oceanToFind)
        return 1;
    return 0;
}


int checkForBiomes(
        LayerStack *    g,
        int             layerID,
        int *           cache,
        int64_t         seed,
        int             x,
        int             z,
        unsigned int    w,
        unsigned int    h,
        BiomeFilter     filter,
        int             protoCheck
        )
{
    Layer *l;

    if (protoCheck)
    {
        l = &g->layers[layerID];

        int i, j;
        int bx = x * l->scale;
        int bz = z * l->scale;
        int bw = w * l->scale;
        int bh = h * l->scale;
        int x0, z0, x1, z1;
        int64_t ss, cs;
        uint64_t potential, required;

        int specialcnt = filter.specialCnt;
        if (specialcnt > 0)
        {
            l = &g->layers[L_SPECIAL_1024];
            x0 = (bx) / l->scale; if (x < 0) x0--;
            z0 = (bz) / l->scale; if (z < 0) z0--;
            x1 = (bx + bw) / l->scale; if (x+(int)w >= 0) x1++;
            z1 = (bz + bh) / l->scale; if (z+(int)h >= 0) z1++;
            ss = getStartSeed(seed, l->layerSeed);

            for (j = z0; j <= z1; j++)
            {
                for (i = x0; i <= x1; i++)
                {
                    cs = getChunkSeed(ss, i, j);
                    if (mcFirstIsZero(cs, 13))
                        specialcnt--;
                }
            }
            if (specialcnt > 0)
                return 0;
        }

        l = &g->layers[L_BIOME_256];
        x0 = bx / l->scale; if (x < 0) x0--;
        z0 = bz / l->scale; if (z < 0) z0--;
        x1 = (bx + bw) / l->scale; if (x+(int)w >= 0) x1++;
        z1 = (bz + bh) / l->scale; if (z+(int)h >= 0) z1++;

        if (filter.majorToFind & (1ULL << mushroom_fields))
        {
            ss = getStartSeed(seed, g->layers[L_ADD_MUSHROOM_256].layerSeed);

            for (j = z0; j <= z1; j++)
            {
                for (i = x0; i <= x1; i++)
                {
                    cs = getChunkSeed(ss, i, j);
                    if (mcFirstIsZero(cs, 100))
                        goto L_HAS_PROTO_MUSHROOM;
                }
            }
            return 0;
        }
L_HAS_PROTO_MUSHROOM:

        potential = 0;
        required = filter.majorToFind & (
                (1ULL << badlands_plateau) | (1ULL << wooded_badlands_plateau) |
                (1ULL << desert) | (1ULL << savanna) | (1ULL << plains) |
                (1ULL << forest) | (1ULL << dark_forest) | (1ULL << mountains) |
                (1ULL << birch_forest) | (1ULL << swamp));

        ss = getStartSeed(seed, l->layerSeed);

        for (j = z0; j <= z1; j++)
        {
            for (i = x0; i <= x1; i++)
            {
                cs = getChunkSeed(ss, i, j);
                int cs6 = mcFirstInt(cs, 6);
                int cs3 = mcFirstInt(cs, 3);
                int cs4 = mcFirstInt(cs, 4);

                if (cs3) potential |= (1ULL << badlands_plateau);
                else potential |= (1ULL << wooded_badlands_plateau);

                switch (cs6)
                {
                case 0: potential |= (1ULL << desert) | (1ULL << forest); break;
                case 1: potential |= (1ULL << desert) | (1ULL << dark_forest); break;
                case 2: potential |= (1ULL << desert) | (1ULL << mountains); break;
                case 3: potential |= (1ULL << savanna) | (1ULL << plains); break;
                case 4: potential |= (1ULL << savanna) | (1ULL << birch_forest); break;
                case 5: potential |= (1ULL << plains) | (1ULL << swamp); break;
                }

                if (cs4 == 3) potential |= (1ULL << snowy_taiga);
                else potential |= (1ULL << snowy_tundra);
            }
        }

        if ((potential & required) ^ required)
            return 0;
    }

    l = &g->layers[layerID];
    int *map = cache ? cache : allocCache(l, w, h);

    g->layers[L_SPECIAL_1024].data          = (void*) &filter;
    g->layers[L_SPECIAL_1024].getMap        = mapFilterSpecial;
    g->layers[L_ADD_MUSHROOM_256].data      = (void*) &filter;
    g->layers[L_ADD_MUSHROOM_256].getMap    = mapFilterMushroom;
    g->layers[L_BIOME_256].data             = (void*) &filter;
    g->layers[L_BIOME_256].getMap           = mapFilterBiome;
    g->layers[L13_OCEAN_TEMP_256].data      = (void*) &filter;
    g->layers[L13_OCEAN_TEMP_256].getMap    = mapFilterOceanTemp;
    g->layers[L_BIOME_EDGE_64].data         = (void*) &filter;
    g->layers[L_BIOME_EDGE_64].getMap       = mapFilterBiomeEdge;
    g->layers[L_RARE_BIOME_64].data         = (void*) &filter;
    g->layers[L_RARE_BIOME_64].getMap       = mapFilterRareBiome;
    g->layers[L_SHORE_16].data              = (void*) &filter;
    g->layers[L_SHORE_16].getMap            = mapFilterShore;
    g->layers[L_RIVER_MIX_4].data           = (void*) &filter;
    g->layers[L_RIVER_MIX_4].getMap         = mapFilterRiverMix;
    g->layers[L13_OCEAN_MIX_4].data         = (void*) &filter;
    g->layers[L13_OCEAN_MIX_4].getMap       = mapFilterOceanMix;

    setWorldSeed(l, seed);
    int ret = !l->getMap(l, map, x, z, w, h);
    if (ret)
    {
        uint64_t required, b = 0, bm = 0;
        unsigned int i;
        for (i = 0; i < w*h; i++)
        {
            int id = map[i];
            if (id < 128) b |= (1ULL << id);
            else bm |= (1ULL << (id-128));
        }
        required = filter.riverToFind;
        required &= ~((1ULL << ocean) | (1ULL << deep_ocean));
        required |= filter.oceanToFind;
        if ((b & required) ^ required)
            ret = -1;
        required = filter.riverToFindM;
        if ((bm & required) ^ required)
            ret = -1;
    }

    g->layers[L_SPECIAL_1024].data          = NULL;
    g->layers[L_SPECIAL_1024].getMap        = mapSpecial;
    g->layers[L_ADD_MUSHROOM_256].data      = NULL;
    g->layers[L_ADD_MUSHROOM_256].getMap    = mapAddMushroomIsland;
    g->layers[L_BIOME_256].data             = NULL;
    g->layers[L_BIOME_256].getMap           = mapBiome;
    g->layers[L13_OCEAN_TEMP_256].data      = NULL;
    g->layers[L13_OCEAN_TEMP_256].getMap    = mapOceanTemp;
    g->layers[L_BIOME_EDGE_64].data         = NULL;
    g->layers[L_BIOME_EDGE_64].getMap       = mapBiomeEdge;
    g->layers[L_RARE_BIOME_64].data         = NULL;
    g->layers[L_RARE_BIOME_64].getMap       = mapRareBiome;
    g->layers[L_SHORE_16].data              = NULL;
    g->layers[L_SHORE_16].getMap            = mapShore;
    g->layers[L_RIVER_MIX_4].data           = NULL;
    g->layers[L_RIVER_MIX_4].getMap         = mapRiverMix;
    g->layers[L13_OCEAN_MIX_4].data         = NULL;
    g->layers[L13_OCEAN_MIX_4].getMap       = mapOceanMix;

    if (cache == NULL)
        free(map);

    return ret;
}


int hasAllTemps(LayerStack *g, int64_t seed, int x1024, int z1024)
{
    int64_t ls;
    ls = getLayerSeed(3); // L_SPECIAL_1024 layer seed

    int64_t ss = getStartSeed(seed, ls);
    int spbits = 0, spcnt = 0;

    if (mcFirstIsZero(getChunkSeed(ss, x1024-1, z1024-1), 13))
    { spbits |= (1<<0); spcnt++; }
    if (mcFirstIsZero(getChunkSeed(ss, x1024  , z1024-1), 13))
    { spbits |= (1<<1); spcnt++; }
    if (mcFirstIsZero(getChunkSeed(ss, x1024+1, z1024-1), 13))
    { spbits |= (1<<2); spcnt++; }
    if (mcFirstIsZero(getChunkSeed(ss, x1024-1, z1024  ), 13))
    { spbits |= (1<<3); spcnt++; }
    if (mcFirstIsZero(getChunkSeed(ss, x1024  , z1024  ), 13))
    { spbits |= (1<<4); spcnt++; }
    if (mcFirstIsZero(getChunkSeed(ss, x1024+1, z1024  ), 13))
    { spbits |= (1<<5); spcnt++; }
    if (mcFirstIsZero(getChunkSeed(ss, x1024-1, z1024+1), 13))
    { spbits |= (1<<6); spcnt++; }
    if (mcFirstIsZero(getChunkSeed(ss, x1024  , z1024+1), 13))
    { spbits |= (1<<7); spcnt++; }
    if (mcFirstIsZero(getChunkSeed(ss, x1024+1, z1024+1), 13))
    { spbits |= (1<<8); spcnt++; }

    if (spcnt < 3)
        return 0;

    // approx. ~2.7% of seeds make it to here

    int buf[20*20];
    int i;

    setWorldSeed(&g->layers[L_HEAT_ICE_1024], seed);
    genArea(&g->layers[L_HEAT_ICE_1024], buf, x1024-1, z1024-1, 3, 3);

    uint64_t bm = 0;
    for (i = 0; i < 9; i++)
    {
        int id = buf[i];
        if (id != 0 && id != Freezing && (spbits & (1<<i)))
           bm |= (1ULL << (id+Special));
        else
           bm |= (1ULL << id);
    }

    // approx. 1 in 100000 seeds satisfy such an all-temperatures cluster
    return ((bm & 0x1df) ^ 0x1df) == 0;
}









