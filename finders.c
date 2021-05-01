#include "finders.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

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


void setAttemptSeed(int64_t *s, int cx, int cz)
{
    *s ^= (cx >> 4) ^ ( (cz >> 4) << 4 );
    setSeed(s, *s);
    next(s, 31);
}

int getConfig(int structureType, int mc, StructureConfig *sconf)
{
    switch (structureType)
    {
    case Feature:
        *sconf = FEATURE_CONFIG;
        return mc <= MC_1_12;
    case Desert_Pyramid:
        *sconf = mc <= MC_1_12 ? DESERT_PYRAMID_CONFIG_112 : DESERT_PYRAMID_CONFIG;
        return mc >= MC_1_3;
    case Jungle_Pyramid:
        *sconf = mc <= MC_1_12 ? JUNGLE_PYRAMID_CONFIG_112 : JUNGLE_PYRAMID_CONFIG;
        return mc >= MC_1_3;
    case Swamp_Hut:
        *sconf = mc <= MC_1_12 ? SWAMP_HUT_CONFIG_112 : SWAMP_HUT_CONFIG;
        return mc >= MC_1_4;
    case Igloo:
        *sconf = mc <= MC_1_12 ? IGLOO_CONFIG_112 : IGLOO_CONFIG;
        return mc >= MC_1_9;
    case Village:
        *sconf = VILLAGE_CONFIG;
        return 1;
    case Ocean_Ruin:
        *sconf = mc <= MC_1_15 ? OCEAN_RUIN_CONFIG_115 : OCEAN_RUIN_CONFIG;
        return mc >= MC_1_13;
    case Shipwreck:
        *sconf = mc <= MC_1_15 ? SHIPWRECK_CONFIG_115 : SHIPWRECK_CONFIG;
        return mc >= MC_1_13;
    case Ruined_Portal:
        *sconf = RUINED_PORTAL_CONFIG;
        return mc >= MC_1_16;
    case Monument:
        *sconf = MONUMENT_CONFIG;
        return mc >= MC_1_8;
    case End_City:
        *sconf = END_CITY_CONFIG;
        return mc >= MC_1_9;
    case Mansion:
        *sconf = MANSION_CONFIG;
        return mc >= MC_1_11;
    case Outpost:
        *sconf = OUTPOST_CONFIG;
        return mc >= MC_1_14;
    case Treasure:
        *sconf = TREASURE_CONFIG;
        return mc >= MC_1_13;
    case Fortress:
        *sconf = FORTRESS_CONFIG;
        return 1;
    case Bastion:
        *sconf = FORTRESS_CONFIG;
        return mc >= MC_1_16;
    default:
        memset(sconf, 0, sizeof(StructureConfig));
        return 0;
    }
}

int getStructurePos(int structureType, int mc, int64_t seed, int regX, int regZ, Pos *pos)
{
    StructureConfig sconf;
    if (!getConfig(structureType, mc, &sconf))
        return 0;

    switch (structureType)
    {
    case Feature:
    case Desert_Pyramid:
    case Jungle_Pyramid:
    case Swamp_Hut:
    case Igloo:
    case Village:
    case Ocean_Ruin:
    case Shipwreck:
    case Ruined_Portal:
        *pos = getFeaturePos(sconf, seed, regX, regZ);
        return 1;

    case Monument:
    case End_City:
    case Mansion:
        *pos = getLargeStructurePos(sconf, seed, regX, regZ);
        return 1;

    case Outpost:
        *pos = getFeaturePos(sconf, seed, regX, regZ);
        setAttemptSeed(&seed, (pos->x) >> 4, (pos->z) >> 4);
        return nextInt(&seed, 5) == 0;

    case Treasure:
        pos->x = (regX << 4) + 9;
        pos->z = (regZ << 4) + 9;
        return isTreasureChunk(seed, regX, regZ);

    case Fortress:
        if (mc < MC_1_16) {
            setAttemptSeed(&seed, regX << 4, regZ << 4);
            int valid = nextInt(&seed, 3) == 0;
            pos->x = ((regX << 4) + nextInt(&seed, 8) + 4) << 4;
            pos->z = ((regZ << 4) + nextInt(&seed, 8) + 4) << 4;
            return valid;
        } else {
            setSeed(&seed, regX*341873128712 + regZ*132897987541 + seed + sconf.salt);
            pos->x = (regX * sconf.regionSize + nextInt(&seed, 24)) << 4;
            pos->z = (regZ * sconf.regionSize + nextInt(&seed, 24)) << 4;
            return nextInt(&seed, 5) < 2;
        }

    case Bastion:
        setSeed(&seed, regX*341873128712 + regZ*132897987541 + seed + sconf.salt);
        pos->x = (regX * sconf.regionSize + nextInt(&seed, 24)) << 4;
        pos->z = (regZ * sconf.regionSize + nextInt(&seed, 24)) << 4;
        return nextInt(&seed, 5) >= 2;

    default:
        fprintf(stderr,
                "ERR getStructurePos: unsupported structure type %d\n", structureType);
        exit(-1);
    }
    return 0;
}

int isMineshaftChunk(int64_t seed, int chunkX, int chunkZ)
{
    int64_t s;
    setSeed(&s, seed);
    int64_t i = nextLong(&s);
    int64_t j = nextLong(&s);
    s = chunkX * i ^ chunkZ * j ^ seed;
    setSeed(&s, s);
    return nextDouble(&s) < 0.004;
}

int isTreasureChunk(int64_t seed, int chunkX, int chunkZ)
{
    seed = chunkX*341873128712 + chunkZ*132897987541 + seed + TREASURE_CONFIG.salt;
    setSeed(&seed, seed);
    return nextFloat(&seed) < 0.01;
}



//==============================================================================
// Multi-Structure Checks
//==============================================================================

// TODO: accurate seed testers for two or three structures in range



static int blocksInRange(Pos *p, int n, int x, int z, int ax, int az, double rsq)
{
    int i, cnt;

    cnt = 0;
    for (i = 0; i < n; i++)
    {
        double dx = p[i].x - x;
        double dz = p[i].z - z;
        int px, pz;

        for (px = 0; px < ax; px++)
        {
            for (pz = 0; pz < az; pz++)
            {
                double ddx = px + dx;
                double ddz = pz + dz;
                cnt += (ddx*ddx + ddz*ddz <= rsq);
            }
        }
    }
    return cnt;
}

STRUCT(afk_meta_t)
{
    Pos *p;
    int n;
    int *buf;
    int x0, z0, w, h, ax, az;
    double rsq;
    int best;
    int sumn;
    int64_t sumx, sumz;
};

static void checkAfkDist(afk_meta_t *d, int x, int z)
{
    if (x < 0 || z < 0 || x >= d->w || z >= d->h)
        return;
    if (d->buf[z*d->w+x])
        return;

    int q = blocksInRange(d->p, d->n, x+d->x0, z+d->z0, d->ax, d->az, d->rsq);
    d->buf[z*d->w+x] = q;
    if (q >= d->best)
    {
        if (q > d->best)
        {
            d->best = q;
            d->sumn = 1;
            d->sumx = d->x0+x;
            d->sumz = d->z0+z;
        }
        else
        {
            d->sumn += 1;
            d->sumx += d->x0+x;
            d->sumz += d->z0+z;
        }
        checkAfkDist(d, x, z-1);
        checkAfkDist(d, x, z+1);
        checkAfkDist(d, x-1, z);
        checkAfkDist(d, x+1, z);
        checkAfkDist(d, x-1, z-1);
        checkAfkDist(d, x-1, z+1);
        checkAfkDist(d, x+1, z-1);
        checkAfkDist(d, x+1, z+1);
    }
}


Pos getOptimalAfk(Pos p[4], int ax, int ay, int az, int *spcnt)
{
    int64_t minX = INT_MAX, minZ = INT_MAX, maxX = INT_MIN, maxZ = INT_MIN;
    int64_t w, h, i;

    for (i = 0; i < 4; i++)
    {
        if (p[i].x < minX) minX = p[i].x;
        if (p[i].z < minZ) minZ = p[i].z;
        if (p[i].x > maxX) maxX = p[i].x;
        if (p[i].z > maxZ) maxZ = p[i].z;
    }

    minX += ax/2;
    minZ += az/2;
    maxX += ax/2;
    maxZ += az/2;

    double rsq = 128.0*128.0 - ay*ay/4.0;

    w = maxX - minX;
    h = maxZ - minZ;
    Pos afk = {p[0].x + ax / 2, p[0].z + az / 2};
    int cnt = ax*az;

    afk_meta_t d;
    d.p = p;
    d.n = 4;
    d.buf = (int*) calloc(w*h, sizeof(int));
    d.x0 = minX;
    d.z0 = minZ;
    d.w = w;
    d.h = h;
    d.ax = ax;
    d.az = az;
    d.rsq = rsq;

    int v[6];
    Pos dsp[6] = {
        {(p[0].x + p[2].x) / 2, (p[0].z + p[2].z) / 2},
        {(p[1].x + p[3].x) / 2, (p[1].z + p[3].z) / 2},
        {(p[0].x + p[1].x) / 2, (p[0].z + p[1].z) / 2},
        {(p[2].x + p[3].x) / 2, (p[2].z + p[3].z) / 2},
        {(p[0].x + p[3].x) / 2, (p[0].z + p[3].z) / 2},
        {(p[1].x + p[2].x) / 2, (p[1].z + p[2].z) / 2},
    };
    for (i = 0; i < 6; i++)
        v[i] = blocksInRange(p, 4, dsp[i].x, dsp[i].z, ax, az, rsq);

    for (i = 0; i < 6; i++)
    {
        // pick out the highest
        int j, jmax = 0, vmax = 0;
        for (j = 0; j < 6; j++)
        {
            if (v[j] > vmax)
            {
                jmax = j;
                vmax = v[j];
            }
        }
        if (vmax <= ax*az)  // highest is less or equal to a single structure
            break;

        d.best = vmax;
        d.sumn = 0;
        d.sumx = 0;
        d.sumz = 0;
        checkAfkDist(&d, dsp[jmax].x - d.x0, dsp[jmax].z - d.z0);
        if (d.best > cnt)
        {
            cnt = d.best;
            afk.x = (int) round(d.sumx / (double)d.sumn);
            afk.z = (int) round(d.sumz / (double)d.sumn);
            if (cnt >= 3*ax*az)
                break;
        }
        v[jmax] = 0;
    }

    if (spcnt)
        *spcnt = cnt;

    free(d.buf);

    return afk;
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
            goto L_err;
        strcpy(dpath, path);

        for (i = pathlen-1; i >= 0; i--)
        {
            if (IS_DIR_SEP(dpath[i]))
            {
                dpath[i] = 0;
                if (mkdirp(dpath))
                    goto L_err;
                break;
            }
        }
    }
    else if (seedbuf == NULL || buflen == NULL)
    {
        // no file and no buffer return: no output possible
        goto L_err;
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
                goto L_err;

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
            goto L_err;

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
                    goto L_err;
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
L_err:
        err = 1;

    free(tids);
    free(info);

    return err;
}

static inline
int scanForQuadBits(const StructureConfig sconf, int radius, int64_t s48,
        int64_t lbit, int lbitn, int64_t invB, int64_t x, int64_t z,
        int64_t w, int64_t h, Pos *qplist, int n)
{
    const int64_t m = (1LL << lbitn);
    const int64_t A = 341873128712LL;
    // for lbitn=20: invB = 132477LL;

    if (n < 1)
        return 0;
    lbit &= m-1;

    int64_t i, j;
    int cnt = 0;
    for (i = x; i <= x+w; i++)
    {
        int64_t sx = s48 + A * i;
        j = (z & ~(m-1)) | ((lbit - sx) * invB & (m-1));
        if (j < z)
            j += m;
        for (; j <= z+h; j += m)
        {
            int64_t sp = moveStructure(s48, -i, -j);
            if ((sp & (m-1)) != lbit)
                continue;

            if (isQuadBase(sconf, sp, radius))
            {
                qplist[cnt].x = i;
                qplist[cnt].z = j;
                cnt++;
                if (cnt >= n)
                    return cnt;
            }
        }
    }

    return cnt;
}

int scanForQuads(
        const StructureConfig sconf, int radius, int64_t s48,
        const int64_t *lowBits, int lowBitCnt, int lowBitN, int64_t salt,
        int x, int z, int w, int h, Pos *qplist, int n)
{
    int i, cnt = 0;
    int64_t invB;
    if (lowBitN == 20)
        invB = 132477LL;
    else if (lowBitN == 48)
        invB = 211541297333629LL;
    else
        invB = mulInv(132897987541LL, (1LL << lowBitN));

    for (i = 0; i < lowBitCnt; i++)
    {
        cnt += scanForQuadBits(sconf, radius, s48, lowBits[i]-salt, lowBitN, invB,
                x, z, w, h, qplist+cnt, n-cnt);
        if (cnt >= n)
            break;
    }

    return cnt;
}



//==============================================================================
// Checking Biomes & Biome Helper Functions
//==============================================================================


int getBiomeAtPos(const LayerStack *g, const Pos pos)
{
    int *ids = allocCache(g->entry_1, 1, 1);
    genArea(g->entry_1, ids, pos.x, pos.z, 1, 1);
    int biomeID = ids[0];
    free(ids);
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
    int *ids;
    int i, j, found;

    Pos out;

    if (l->scale != 4)
    {
        printf("WARN findBiomePosition: require scale = 4, but have %d.\n",
                l->scale);
    }

    ids = cache ? cache : allocCache(l, width, height);

    genArea(l, ids, x1, z1, width, height);

    out.x = centerX;
    out.z = centerZ;
    found = 0;

    if (mcversion >= MC_1_13)
    {
        for (i = 0, j = 2; i < width*height; i++)
        {
            if (!isValid[ids[i]]) continue;
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
            if (isValid[ids[i]] && (found == 0 || nextInt(seed, found + 1) == 0))
            {
                out.x = (x1 + i%width) << 2;
                out.z = (z1 + i/width) << 2;
                ++found;
            }
        }
    }


    if (cache == NULL)
    {
        free(ids);
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
    int *ids;
    int viable;

    if (l->scale != 4)
    {
        printf("WARN areBiomesViable: require scale = 4, but have %d.\n",
                l->scale);
    }

    ids = cache ? cache : allocCache(l, width, height);
    viable = !genArea(l, ids, x1, z1, width, height);

    if (viable)
    {
        for (i = 0; i < width*height; i++)
        {
            if (!isValid[ ids[i] ])
            {
                viable = 0;
                break;
            }
        }
    }

    if (cache == NULL)
        free(ids);
    return viable;
}


//==============================================================================
// Finding Strongholds and Spawn
//==============================================================================


const char* getValidStrongholdBiomes(int mc)
{
    static const int strongholdBiomes[] = {
        plains, desert, mountains, forest, taiga, snowy_tundra, snowy_mountains,
        mushroom_fields, mushroom_field_shore, desert_hills, wooded_hills,
        taiga_hills, mountain_edge, jungle,jungle_hills, jungle_edge,
        stone_shore, birch_forest, birch_forest_hills, dark_forest, snowy_taiga,
        snowy_taiga_hills, giant_tree_taiga, giant_tree_taiga_hills,
        wooded_mountains, savanna, savanna_plateau, badlands,
        wooded_badlands_plateau, badlands_plateau, sunflower_plains,
        desert_lakes, gravelly_mountains, flower_forest, taiga_mountains,
        ice_spikes, modified_jungle, modified_jungle_edge, tall_birch_forest,
        tall_birch_hills, dark_forest_hills, snowy_taiga_mountains,
        giant_spruce_taiga, giant_spruce_taiga_hills,
        modified_gravelly_mountains, shattered_savanna,
        shattered_savanna_plateau, eroded_badlands,
        modified_wooded_badlands_plateau, modified_badlands_plateau,
        bamboo_jungle, bamboo_jungle_hills,
    };

    static char isValid115[256], isValid[256];
    unsigned int i;

    if (mc <= MC_1_15)
    {
        if (!isValid115[strongholdBiomes[0]])
            for (i = 0; i < sizeof(strongholdBiomes) / sizeof(int); i++)
                isValid115[ strongholdBiomes[i] ] = 1;
        return isValid115;
    }
    else
    {   // simulate MC-199298
        if (!isValid[strongholdBiomes[0]])
            for (i = 0; i < sizeof(strongholdBiomes) / sizeof(int); i++)
                isValid[ strongholdBiomes[i] ] = 1;
        isValid[bamboo_jungle] = 0;
        isValid[bamboo_jungle_hills] = 0;
        return isValid;
    }
}

Pos initFirstStronghold(StrongholdIter *sh, int mc, int64_t s48)
{
    double dist, angle;
    int64_t rnds;
    Pos p;

    setSeed(&rnds, s48);

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
        sh->nextapprox.x, sh->nextapprox.z, 112,
        getValidStrongholdBiomes(sh->mc), &sh->rnds, NULL);

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

int findStrongholds(const int mc, const LayerStack *g, int *cache,
        Pos *locations, int64_t worldSeed, int maxSH, int maxRing)
{
    const char *validStrongholdBiomes = getValidStrongholdBiomes(mc);
    int i, x, z;
    double distance;

    int currentRing = 0;
    int currentCount = 0;
    int perRing = 3;
    int64_t rnd;

    setSeed(&rnd, worldSeed);
    double angle = nextDouble(&rnd) * PI * 2.0;

    const Layer *l = &g->layers[L_RIVER_MIX_4];

    if (mc >= MC_1_9)
    {
        if (maxSH <= 0) maxSH = 128;

        for (i = 0; i < maxSH; i++)
        {
            distance = (4.0 * 32.0) + (6.0 * currentRing * 32.0) +
                (nextDouble(&rnd) - 0.5) * 32 * 2.5;

            x = (int)round(cos(angle) * distance);
            z = (int)round(sin(angle) * distance);

            locations[i] = findBiomePosition(mc, l, cache,
                    (x << 4) + 8, (z << 4) + 8, 112, validStrongholdBiomes,
                    &rnd, NULL);

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
                angle = angle + nextDouble(&rnd) * PI * 2.0;
            }
        }
    }
    else
    {
        if (maxSH <= 0) maxSH = 3;

        for (i = 0; i < maxSH; i++)
        {
            distance = (1.25 + nextDouble(&rnd)) * 32.0;

            x = (int)round(cos(angle) * distance);
            z = (int)round(sin(angle) * distance);

            locations[i] = findBiomePosition(mc, l, cache,
                    (x << 4) + 8, (z << 4) + 8, 112, validStrongholdBiomes,
                    &rnd, NULL);

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
    case swamp:                         return 0.3; // height dependent
    case river:                         return 0.15;
    case beach:                         return 0.0;
    case snowy_tundra:                  return 0.02;
    case snowy_mountains:               return 0.02;
    case wooded_hills:                  return 1.0;
    case taiga_hills:                   return 1.0;
    case mountain_edge:                 return 1.0; // height dependent
    case jungle:                        return 1.0;
    case jungle_hills:                  return 1.0;
    case jungle_edge:                   return 1.0;
    case birch_forest:                  return 1.0;
    case birch_forest_hills:            return 1.0;
    case dark_forest:                   return 0.9;
    case snowy_taiga:                   return 0.1; // below trees
    case snowy_taiga_hills:             return 0.1; // below trees
    case giant_tree_taiga:              return 0.6;
    case giant_tree_taiga_hills:        return 0.6;
    case wooded_mountains:              return 0.2; // height dependent
    case savanna:                       return 1.0;
    case savanna_plateau:               return 0.9;
    case wooded_badlands_plateau:       return 0.0; // height dependent
    case badlands_plateau:              return 0.0; // height dependent

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
    case snowy_taiga_mountains:         return 0.1;
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
    int64_t rnd;

    setSeed(&rnd, worldSeed);
    spawn = findBiomePosition(mcversion, l, cache, 0, 0, 256, isSpawnBiome,
            &rnd, &found);

    if (!found)
    {
        //printf("Unable to find spawn biome.\n");
        spawn.x = spawn.z = 8;
    }

    double accum = 1;
    double bx = 0;
    double bz = 0;
    double bn = 0;

    if (mcversion >= MC_1_13)
    {
        int *area = allocCache(g->entry_1, 16, 16);
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
                int x, z;

                genArea(g->entry_1, area, cx, cz, 16, 16);

                for (x = 0; x < 16; x++)
                {
                    for (z = 0; z < 16; z++)
                    {
                        Pos pos = {cx+x, cz+z};
                        int biome = area[z*16 + x];
                        double gp = getGrassProbability(worldSeed, biome,
                            pos.x, pos.z);
                        if (gp == 0)
                            continue;

                        bx += accum * gp * pos.x;
                        bz += accum * gp * pos.z;
                        bn += accum * gp;

                        accum *= 1 - gp;
                        if (accum < 0.001)
                        {
                            free(area);
                            spawn.x = (int) round(bx / bn);
                            spawn.z = (int) round(bz / bn);
                            return spawn;
                        }
                    }
                }
            }

            if (n2 == n3 || (n2 < 0 && n2 == - n3) || (n2 > 0 && n2 == 1 - n3))
            {
                int n7 = n4;
                n4 = -n5;
                n5 = n7;
            }
            n2 += n4;
            n3 += n5;
        }

        free(area);
    }
    else
    {
        for (i = 0; i < 1000; i++)
        {
            int biome = getBiomeAtPos(g, spawn);
            double gp = getGrassProbability(worldSeed, biome, spawn.x, spawn.z);

            bx += accum * gp * spawn.x;
            bz += accum * gp * spawn.z;
            bn += accum * gp;

            accum *= 1 - gp;
            if (accum < 0.001)
            {
                spawn.x = (int) round(bx / bn);
                spawn.z = (int) round(bz / bn);
                break;
            }

            spawn.x += nextInt(&rnd, 64) - nextInt(&rnd, 64);
            spawn.z += nextInt(&rnd, 64) - nextInt(&rnd, 64);
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
    int64_t rnd;
    setSeed(&rnd, worldSeed);
    spawn = findBiomePosition(mcversion, l, cache, 0, 0, 256, isSpawnBiome,
            &rnd, &found);

    if (!found)
    {
        spawn.x = spawn.z = 8;
    }

    if (mcversion >= MC_1_13)
    {
        spawn.x &= ~0xf;
        spawn.z &= ~0xf;
    }

    return spawn;
}



//==============================================================================
// Validating Structure Positions
//==============================================================================


int isViableFeatureBiome(int mc, int structureType, int biomeID)
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
        if (mc < MC_1_9) return 0;
        return biomeID == snowy_tundra || biomeID == snowy_taiga;

    case Ocean_Ruin:
        if (mc < MC_1_13) return 0;
        return isOceanic(biomeID);

    case Shipwreck:
        if (mc < MC_1_13) return 0;
        return isOceanic(biomeID) || biomeID == beach || biomeID == snowy_beach;

    case Ruined_Portal:
        return mc >= MC_1_16;

    case Treasure:
        if (mc < MC_1_13) return 0;
        return biomeID == beach || biomeID == snowy_beach;

    case Monument:
        if (mc < MC_1_8) return 0;
        return isDeepOcean(biomeID);

    case Outpost:
        if (mc < MC_1_14) return 0;
        // fall through
    case Village:
        if (biomeID == plains || biomeID == desert || biomeID == savanna)
            return 1;
        if (mc >= MC_1_10 && biomeID == taiga)
            return 1;
        if (mc >= MC_1_14 && biomeID == snowy_tundra)
            return 1;
        return 0;

    case Mansion:
        if (mc < MC_1_11) return 0;
        return biomeID == dark_forest || biomeID == dark_forest_hills;

    case Fortress:
        return (biomeID == nether_wastes || biomeID == soul_sand_valley ||
                biomeID == warped_forest || biomeID == crimson_forest);

    case Bastion:
        if (mc < MC_1_16) return 0;
        return (biomeID == nether_wastes || biomeID == soul_sand_valley ||
                biomeID == warped_forest || biomeID == crimson_forest ||
                biomeID == basalt_deltas);

    case End_City:
        if (mc < MC_1_9) return 0;
        return biomeID == end_midlands || biomeID == end_highlands;

    default:
        fprintf(stderr,
                "isViableFeatureBiome: not implemented for structure type %d.\n",
                structureType);
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

    int styp = ((const int*) l->data)[0];
    int i, j;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int biomeID = out[i + w*j];
            switch (styp)
            {
            case Desert_Pyramid:
                if (biomeID == desert || isMesa(biomeID))
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
            case Treasure:
                if (isOceanic(biomeID))
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

    int styp = ((const int*) l->data)[0];
    int mc   = ((const int*) l->data)[1];
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
            case Treasure:
                if (isViableFeatureBiome(mc, styp, biomeID))
                    return 0;
                break;

            default:
                return 0;
            }
        }
    }

    return 1;
}


int isViableStructurePos(int structureType, int mc, LayerStack *g,
        int64_t seed, int blockX, int blockZ)
{
    int *ids = NULL;
    Layer *l;
    int viable = 0;

    int64_t chunkX = blockX >> 4;
    int64_t chunkZ = blockZ >> 4;

    // Structures are positioned at the chunk origin, but the biome check is
    // performed near the middle of the chunk [(9,9) in 1.13, TODO: check 1.7]
    // In 1.16 the biome check is always performed at (2,2) with layer scale=4.
    int biomeX, biomeZ;

    Layer lbiome = g->layers[L_BIOME_256];
    Layer lshore = g->layers[L_SHORE_16];

    int data[2] = { structureType, mc };

    g->layers[L_BIOME_256].data = (void*) data;
    g->layers[L_BIOME_256].getMap = mapViableBiome;
    g->layers[L_SHORE_16].data = (void*) data;
    g->layers[L_SHORE_16].getMap = mapViableShore;

    switch (structureType)
    {
    case Ocean_Ruin:
    case Shipwreck:
    case Treasure:
        if (mc < MC_1_13) goto L_not_viable;
        goto L_feature;
    case Igloo:
        if (mc < MC_1_9) goto L_not_viable;
        goto L_feature;
    case Desert_Pyramid:
    case Jungle_Pyramid:
    case Swamp_Hut:
L_feature:
        if (mc < MC_1_16)
        {
            l = &g->layers[L_VORONOI_ZOOM_1];
            biomeX = (chunkX << 4) + 9;
            biomeZ = (chunkZ << 4) + 9;
        }
        else
        {   // NOTE: L_RIVER_MIX_4 skips the ocean types, should be fine for
            // ocean ruins and ship wrecks.
            l = &g->layers[L_RIVER_MIX_4];
            biomeX = (chunkX << 2) + 2;
            biomeZ = (chunkZ << 2) + 2;
        }
        setLayerSeed(l, seed);
        ids = allocCache(l, 1, 1);
        if (genArea(l, ids, biomeX, biomeZ, 1, 1))
            goto L_not_viable;
        if (!isViableFeatureBiome(mc, structureType, ids[0]))
            goto L_not_viable;
        goto L_viable;

    case Village:
        l = &g->layers[L_RIVER_MIX_4];
        biomeX = (chunkX << 2) + 2;
        biomeZ = (chunkZ << 2) + 2;
        setLayerSeed(l, seed);
        ids = allocCache(l, 1, 1);
        if (genArea(l, ids, biomeX, biomeZ, 1, 1))
            goto L_not_viable;
        if (!isViableFeatureBiome(mc, structureType, ids[0]))
            goto L_not_viable;
        viable = ids[0]; // biome for viablility value as it may be useful for further analysis
        goto L_viable;

    case Outpost:
    {
        if (mc < MC_1_14)
            goto L_not_viable;
        int64_t rnd = seed;
        setAttemptSeed(&rnd, chunkX, chunkZ);
        if (nextInt(&rnd, 5) != 0)
            goto L_not_viable;
        if (mc < MC_1_16)
        {
            l = &g->layers[L_VORONOI_ZOOM_1];
            biomeX = (chunkX << 4) + 9;
            biomeZ = (chunkZ << 4) + 9;
        }
        else
        {   // NOTE: this skips the ocean type check
            l = &g->layers[L_RIVER_MIX_4];
            biomeX = (chunkX << 2) + 2;
            biomeZ = (chunkZ << 2) + 2;
        }
        setLayerSeed(l, seed);
        ids = allocCache(l, 1, 1);
        if (genArea(l, ids, biomeX, biomeZ, 1, 1))
            goto L_not_viable;
        if (!isViableFeatureBiome(mc, structureType, ids[0]))
            goto L_not_viable;
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
                    if (mc >= MC_1_16)
                        goto L_not_viable;
                    if (isViableStructurePos(Village, mc, g, seed, p.x, p.z))
                        goto L_not_viable;
                    goto L_viable;
                }
            }
        }
        goto L_viable;
    }

    case Monument:
        if (mc < MC_1_8)
            goto L_not_viable;
        else if (mc == MC_1_8)
        {   // In 1.8 monuments require only a single deep ocean block.
            l = g->entry_1;
            setLayerSeed(l, seed);
            ids = allocCache(l, 1, 1);
            if (genArea(l, ids, (chunkX << 4) + 8, (chunkZ << 4) + 8, 1, 1))
                goto L_not_viable;
        }
        else
        {   // Monuments require two viability checks with the ocean layer
            // branch => worth checking for potential deep ocean beforehand.
            l = &g->layers[L_SHORE_16];
            setLayerSeed(l, seed);
            ids = allocCache(l, 1, 1);
            if (genArea(l, ids, chunkX, chunkZ, 1, 1))
                goto L_not_viable;
        }
        if (!isDeepOcean(ids[0]))
            goto L_not_viable;
        if (mc >= MC_1_13)
            l = &g->layers[L13_OCEAN_MIX_4];
        else
            l = &g->layers[L_RIVER_MIX_4];
        biomeX = (chunkX << 4) + 8; // areBiomesViable expects block positions
        biomeZ = (chunkZ << 4) + 8;
        setLayerSeed(l, seed);
        if (mc < MC_1_9 || areBiomesViable(l, NULL, biomeX, biomeZ, 16, getValidMonumentBiomes2()))
            if (areBiomesViable(l, NULL, biomeX, biomeZ, 29, getValidMonumentBiomes1()))
                goto L_viable;
        goto L_not_viable;

    case Mansion:
        if (mc < MC_1_11)
            goto L_not_viable;
        l = &g->layers[L_RIVER_MIX_4];
        biomeX = (chunkX << 4) + 8;
        biomeZ = (chunkZ << 4) + 8;
        setLayerSeed(l, seed);
        if (areBiomesViable(l, NULL, biomeX, biomeZ, 32, getValidMansionBiomes()))
            goto L_viable;
        goto L_not_viable;

    case Ruined_Portal:
        if (mc >= MC_1_16)
            goto L_viable;
        goto L_not_viable;

    default:
        fprintf(stderr,
                "isViableStructurePos: validation for structure type %d not implemented\n",
                structureType);
        goto L_not_viable;
    }

L_viable:
    if (!viable)
        viable = 1;
L_not_viable:

    g->layers[L_BIOME_256] = lbiome;
    g->layers[L_SHORE_16] = lshore;
    if (ids)
        free(ids);

    return viable;
}

int isViableNetherStructurePos(int structureType, int mc, NetherNoise *nn,
        int64_t seed, int blockX, int blockZ)
{
    if (mc < MC_1_16)
        return structureType == Fortress;

    blockX = ((blockX >> 4) << 2) + 2;
    blockZ = ((blockZ >> 4) << 2) + 2;
    setNetherSeed(nn, seed);
    int biomeID = getNetherBiome(nn, blockX, 0, blockZ, NULL);
    return isViableFeatureBiome(mc, structureType, biomeID);
}


//==============================================================================
// Finding Properties of Structures
//==============================================================================


VillageType getVillageType(int mc, int64_t seed, int blockX, int blockZ, int biomeID)
{
    VillageType r = { 0, 0, 0 };
    if (!isViableFeatureBiome(mc, Village, biomeID))
        return r;

    int64_t rnd = chunkGenerateRnd(seed, blockX >> 4, blockZ >> 4);

    r.biome = biomeID;

    if (mc >= MC_1_14)
    {
        skipNextN(&rnd, 1);
        int t;
        switch (biomeID)
        {
        case plains:
            t = nextInt(&rnd, 204);
            if      (t <  50) { r.variant = 0; } // plains_fountain_01
            else if (t < 100) { r.variant = 1; } // plains_meeting_point_1
            else if (t < 150) { r.variant = 2; } // plains_meeting_point_2
            else if (t < 200) { r.variant = 3; } // plains_meeting_point_3
            else if (t < 201) { r.variant = 0; r.abandoned = 1; }
            else if (t < 202) { r.variant = 1; r.abandoned = 1; }
            else if (t < 203) { r.variant = 2; r.abandoned = 1; }
            else if (t < 204) { r.variant = 3; r.abandoned = 1; }
            break;
        case desert:
            t = nextInt(&rnd, 250);
            if      (t <  98) { r.variant = 1; } // desert_meeting_point_1
            else if (t < 196) { r.variant = 2; } // desert_meeting_point_2
            else if (t < 245) { r.variant = 3; } // desert_meeting_point_3
            else if (t < 247) { r.variant = 1; r.abandoned = 1; }
            else if (t < 249) { r.variant = 2; r.abandoned = 1; }
            else if (t < 250) { r.variant = 3; r.abandoned = 1; }
            break;
        case savanna:
            t = nextInt(&rnd, 459);
            if      (t < 100) { r.variant = 1; } // savanna_meeting_point_1
            else if (t < 150) { r.variant = 2; } // savanna_meeting_point_2
            else if (t < 300) { r.variant = 3; } // savanna_meeting_point_3
            else if (t < 450) { r.variant = 4; } // savanna_meeting_point_4
            else if (t < 452) { r.variant = 1; r.abandoned = 1; }
            else if (t < 453) { r.variant = 2; r.abandoned = 1; }
            else if (t < 456) { r.variant = 3; r.abandoned = 1; }
            else if (t < 459) { r.variant = 4; r.abandoned = 1; }
            break;
        case taiga:
            t = nextInt(&rnd, 100);
            if      (t <  49) { r.variant = 1; } // taiga_meeting_point_1
            else if (t <  98) { r.variant = 2; } // taiga_meeting_point_2
            else if (t <  99) { r.variant = 1; r.abandoned = 1; }
            else if (t < 100) { r.variant = 2; r.abandoned = 1; }
            break;
        case snowy_tundra:
            t = nextInt(&rnd, 306);
            if      (t < 100) { r.variant = 1; } // snowy_meeting_point_1
            else if (t < 150) { r.variant = 2; } // snowy_meeting_point_2
            else if (t < 300) { r.variant = 3; } // snowy_meeting_point_3
            else if (t < 302) { r.variant = 1; r.abandoned = 1; }
            else if (t < 303) { r.variant = 2; r.abandoned = 1; }
            else if (t < 306) { r.variant = 3; r.abandoned = 1; }
            break;
        default:
            break;
        }
    }
    else if (mc >= MC_1_10)
    {
        skipNextN(&rnd, mc == MC_1_13 ? 10 : 11);
        r.abandoned = nextInt(&rnd, 50) == 0;
    }

    return r;
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
            fprintf(stderr, "setupBiomeFilter: biomeID=%d not supported.\n", id);
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
                bf.raresToFindM |= (1ULL << (id-128));
                bf.riverToFindM |= (1ULL << (id-128));
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
                    if (id != lukewarm_ocean && id != cold_ocean)
                        bf.otempToFind |= (1ULL << id);
                } else {
                    bf.raresToFind |= (1ULL << deep_ocean);
                    bf.riverToFind |= (1ULL << deep_ocean);
                    if (id == deep_warm_ocean)
                        bf.otempToFind |= (1ULL << warm_ocean);
                    else if (id == deep_ocean)
                        bf.otempToFind |= (1ULL << ocean);
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


STRUCT(filter_data_t)
{
    const BiomeFilter *bf;
    int (*map)(const Layer *, int *, int, int, int, int);
};

static int mapFilterSpecial(const Layer * l, int * out, int x, int z, int w, int h)
{
    const filter_data_t *f = (const filter_data_t*) l->data;
    int i, j;
    uint64_t temps;

    /// pre-gen checks
    int specialcnt = f->bf->specialCnt;
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

    int err = f->map(l, out, x, z, w, h);
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

    if ((temps & f->bf->tempsToFind) ^ f->bf->tempsToFind)
        return 1;
    return 0;
}

static int mapFilterMushroom(const Layer * l, int * out, int x, int z, int w, int h)
{
    const filter_data_t *f = (const filter_data_t*) l->data;
    int i, j;
    int err;

    if (w*h < 100 && (f->bf->majorToFind & (1ULL << mushroom_fields)))
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
    err = f->map(l, out, x, z, w, h);
    if U(err != 0)
        return err;

    if (f->bf->majorToFind & (1ULL << mushroom_fields))
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
    const filter_data_t *f = (const filter_data_t*) l->data;
    int i, j;
    uint64_t b;

    int err = f->map(l, out, x, z, w, h);
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

    if ((b & f->bf->majorToFind) ^ f->bf->majorToFind)
        return 1;
    return 0;
}

static int mapFilterOceanTemp(const Layer * l, int * out, int x, int z, int w, int h)
{
    const filter_data_t *f = (const filter_data_t*) l->data;
    int i, j;
    uint64_t b;

    int err = f->map(l, out, x, z, w, h);
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

    if ((b & f->bf->otempToFind) ^ f->bf->otempToFind)
        return 1;
    return 0;
}

static int mapFilterBiomeEdge(const Layer * l, int * out, int x, int z, int w, int h)
{
    const filter_data_t *f = (const filter_data_t*) l->data;
    uint64_t b;
    int i;
    int err;

    err = f->map(l, out, x, z, w, h);
    if U(err != 0)
        return err;

    b = 0;
    for (i = 0; i < w*h; i++)
        b |= (1ULL << (out[i] & 0x3f));

    if ((b & f->bf->edgesToFind) ^ f->bf->edgesToFind)
        return 1;
    return 0;
}

static int mapFilterRareBiome(const Layer * l, int * out, int x, int z, int w, int h)
{
    const filter_data_t *f = (const filter_data_t*) l->data;
    uint64_t b, bm;
    int i;
    int err;

    err = f->map(l, out, x, z, w, h);
    if U(err != 0)
        return err;

    b = 0; bm = 0;
    for (i = 0; i < w*h; i++)
    {
        int id = out[i];
        if (id < 128) b |= (1ULL << id);
        else bm |= (1ULL << (id-128));
    }

    if ((b & f->bf->raresToFind) ^ f->bf->raresToFind)
        return 1;
    if ((bm & f->bf->raresToFindM) ^ f->bf->raresToFindM)
        return 1;
    return 0;
}

static int mapFilterShore(const Layer * l, int * out, int x, int z, int w, int h)
{
    const filter_data_t *f = (const filter_data_t*) l->data;
    uint64_t b, bm;
    int i;

    int err = f->map(l, out, x, z, w, h);
    if U(err != 0)
        return err;

    b = 0; bm = 0;
    for (i = 0; i < w*h; i++)
    {
        int id = out[i];
        if (id < 128) b |= (1ULL << id);
        else bm |= (1ULL << (id-128));
    }

    if ((b & f->bf->shoreToFind) ^ f->bf->shoreToFind)
        return 1;
    if ((bm & f->bf->shoreToFindM) ^ f->bf->shoreToFindM)
        return 1;
    return 0;
}

static int mapFilterRiverMix(const Layer * l, int * out, int x, int z, int w, int h)
{
    const filter_data_t *f = (const filter_data_t*) l->data;
    uint64_t b, bm;
    int i;

    int err = f->map(l, out, x, z, w, h);
    if U(err != 0)
        return err;

    b = 0; bm = 0;
    for (i = 0; i < w*h; i++)
    {
        int id = out[i];
        if (id < 128) b |= (1ULL << id);
        else bm |= (1ULL << (id-128));
    }

    if ((b & f->bf->riverToFind) ^ f->bf->riverToFind)
        return 1;
    if ((bm & f->bf->riverToFindM) ^ f->bf->riverToFindM)
        return 1;
    return 0;
}

static int mapFilterOceanMix(const Layer * l, int * out, int x, int z, int w, int h)
{
    const filter_data_t *f = (const filter_data_t*) l->data;
    uint64_t b;
    int i;
    int err;

    if (f->bf->riverToFind)
    {
        err = l->p->getMap(l->p, out, x, z, w, h); // RiverMix
        if (err)
            return err;
    }

    err = f->map(l, out, x, z, w, h);
    if U(err != 0)
        return err;

    b = 0;
    for (i = 0; i < w*h; i++)
    {
        int id = out[i];
        if (id < 128) b |= (1ULL << id);
    }

    if ((b & f->bf->oceanToFind) ^ f->bf->oceanToFind)
        return 1;
    return 0;
}

void swapMap(filter_data_t *fd, BiomeFilter *bf, Layer *l,
        int (*map)(const Layer *, int *, int, int, int, int))
{
    fd->bf = bf;
    fd->map = l->getMap;
    l->data = (void*) fd;
    l->getMap = map;
}

void restoreMap(filter_data_t *fd, Layer *l)
{
    l->getMap = fd->map;
    l->data = NULL;
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

    if (protoCheck) // TODO: protoCheck for 1.6-
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
            ss = getStartSeed(seed, l->layerSalt);

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
            ss = getStartSeed(seed, g->layers[L_MUSHROOM_256].layerSalt);

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

        ss = getStartSeed(seed, l->layerSalt);

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

    l = g->layers;
    int *ids = cache ? cache : allocCache(&l[layerID], w, h);

    filter_data_t fd[9];
    swapMap(fd+0, &filter, l+L_OCEAN_MIX_4,     mapFilterOceanMix);
    swapMap(fd+1, &filter, l+L_RIVER_MIX_4,     mapFilterRiverMix);
    swapMap(fd+2, &filter, l+L_SHORE_16,        mapFilterShore);
    swapMap(fd+3, &filter, l+L_SUNFLOWER_64,    mapFilterRareBiome);
    swapMap(fd+4, &filter, l+L_BIOME_EDGE_64,   mapFilterBiomeEdge);
    swapMap(fd+5, &filter, l+L_OCEAN_TEMP_256,  mapFilterOceanTemp);
    swapMap(fd+6, &filter, l+L_BIOME_256,       mapFilterBiome);
    swapMap(fd+7, &filter, l+L_MUSHROOM_256,    mapFilterMushroom);
    swapMap(fd+8, &filter, l+L_SPECIAL_1024,    mapFilterSpecial);

    setLayerSeed(&l[layerID], seed);
    int ret = !l[layerID].getMap(&l[layerID], ids, x, z, w, h);
    if (ret)
    {
        uint64_t required, b = 0, bm = 0;
        unsigned int i;
        for (i = 0; i < w*h; i++)
        {
            int id = ids[i];
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

    restoreMap(fd+8, l+L_SPECIAL_1024);
    restoreMap(fd+7, l+L_MUSHROOM_256);
    restoreMap(fd+6, l+L_BIOME_256);
    restoreMap(fd+5, l+L_OCEAN_TEMP_256);
    restoreMap(fd+4, l+L_BIOME_EDGE_64);
    restoreMap(fd+3, l+L_SUNFLOWER_64);
    restoreMap(fd+2, l+L_SHORE_16);
    restoreMap(fd+1, l+L_RIVER_MIX_4);
    restoreMap(fd+0, l+L_OCEAN_MIX_4);

    if (cache == NULL)
        free(ids);

    return ret;
}


int checkForTemps(LayerStack *g, int64_t seed, int x, int z, int w, int h, const int tc[9])
{
    int64_t ls = getLayerSalt(3); // L_SPECIAL_1024 layer seed
    int64_t ss = getStartSeed(seed, ls);

    int i, j;
    int scnt = 0;

    if (tc[Special+Warm] > 0) scnt += tc[Special+Warm];
    if (tc[Special+Lush] > 0) scnt += tc[Special+Lush];
    if (tc[Special+Cold] > 0) scnt += tc[Special+Cold];

    if (scnt > 0)
    {
        for (j = 0; j < h; j++)
        {
            for (i = 0; i < w; i++)
            {
                if (mcFirstIsZero(getChunkSeed(ss, x+i, z+j), 13))
                    scnt--;
            }
        }
        if (scnt > 0)
            return 0;
    }

    Layer *l = &g->layers[L_SPECIAL_1024];
    int ccnt[9] = {0};
    int *area = allocCache(l, w, h);
    int ret = 1;

    setLayerSeed(l, seed);
    genArea(l, area, x, z, w, h);

    for (i = 0; i < w*h; i++)
    {
        int id = area[i];
        int t = id & 0xff;
        if (id != t && t != Freezing)
            t += Special;
        ccnt[t]++;
    }
    for (i = 0; i < 9; i++)
    {
        if (ccnt[i] < tc[i] || (ccnt[i] && tc[i] < 0))
        {
            ret = 0;
            break;
        }
    }

    free(area);
    return ret;
}


int canBiomeGenerate(int layerId, int mc, int id)
{
    int dofilter = 0;

    if (dofilter || layerId == L_BIOME_256)
    {
        dofilter = 1;
        if (id >= 64)
            return 0;
    }
    if (dofilter || (layerId == L_BAMBOO_256 && mc >= MC_1_14))
    {
        dofilter = 1;
        switch (id)
        {
        case jungle_edge:
        case wooded_mountains:
        case badlands:
            return 0;
        }
    }
    if (dofilter || (layerId == L_BIOME_EDGE_64 && mc >= MC_1_7))
    {
        dofilter = 1;
        if (id >= 64 && id != bamboo_jungle)
            return 0;
        switch (id)
        {
        case snowy_mountains:
        case desert_hills:
        case wooded_hills:
        case taiga_hills:
        case jungle_hills:
        case birch_forest_hills:
        case snowy_taiga_hills:
        case giant_tree_taiga_hills:
        case savanna_plateau:
            return 0;
        }
    }
    if (dofilter || layerId == L_HILLS_64)
    {
        dofilter = 1;
        // sunflower_plains actually generate at Hills layer as well
    }
    if (dofilter || (layerId == L_SUNFLOWER_64 && mc >= MC_1_7))
    {
        dofilter = 1;
        switch (id)
        {
        case frozen_ocean:
        case mushroom_field_shore:
        case beach:
        case stone_shore:
        case snowy_beach:
            return 0;
        }
    }
    if (dofilter || layerId == L_SHORE_16)
    {
        dofilter = 1;
        if (id == river || id == frozen_river)
            return 0;
    }
    if (dofilter || layerId == L_RIVER_MIX_4)
    {
        dofilter = 1;
        switch (id)
        {
        case frozen_ocean:
            if (mc >= MC_1_7)
                return 0;
            break;
        case warm_ocean...deep_frozen_ocean:
            return 0;
        }
    }
    if (dofilter || (layerId == L_OCEAN_MIX_4 && mc >= MC_1_13))
    {
        dofilter = 1;
    }

    if (!dofilter && layerId != L_VORONOI_1)
    {
        printf("canBiomeGenerate(): unsupported layer (%d) or version (%d)\n",
            layerId, mc);
        return 0;
    }
    return isOverworld(mc, id);
}


// TODO: This function requires testing across versions
void genPotential(uint64_t *mL, uint64_t *mM, int layer, int mc, int id)
{
    // filter out bad biomes
    if (layer >= L_BIOME_256 && !canBiomeGenerate(layer, mc, id))
        return;

    switch (layer)
    {
    case L_SPECIAL_1024: // biomes added in (L_SPECIAL_1024, L_MUSHROOM_256]
        if (mc <= MC_1_6) goto L_bad_layer;
        if (id == Oceanic)
            genPotential(mL, mM, L_MUSHROOM_256, mc, mushroom_fields);
        if ((id & ~0xf00) >= Oceanic && (id & ~0xf00) <= Freezing)
            genPotential(mL, mM, L_MUSHROOM_256, mc, id);
        break;

    case L_MUSHROOM_256: // biomes added in (L_MUSHROOM_256, L_DEEP_OCEAN_256]
        if (mc >= MC_1_7) {
            if (id == Oceanic)
                genPotential(mL, mM, L_DEEP_OCEAN_256, mc, deep_ocean);
            if ((id & ~0xf00) >= Oceanic && (id & ~0xf00) <= Freezing)
                genPotential(mL, mM, L_DEEP_OCEAN_256, mc, id);
        } else { // (L_MUSHROOM_256, L_BIOME_256] for 1.6
            if (id == ocean || id == mushroom_fields) {
                genPotential(mL, mM, L_BIOME_256, mc, id);
            } else {
                genPotential(mL, mM, L_BIOME_256, mc, desert);
                genPotential(mL, mM, L_BIOME_256, mc, forest);
                genPotential(mL, mM, L_BIOME_256, mc, mountains);
                genPotential(mL, mM, L_BIOME_256, mc, swamp);
                genPotential(mL, mM, L_BIOME_256, mc, plains);
                genPotential(mL, mM, L_BIOME_256, mc, taiga);
                if (mc >= MC_1_2)
                    genPotential(mL, mM, L_BIOME_256, mc, jungle);
                if (id != plains)
                    genPotential(mL, mM, L_BIOME_256, mc, snowy_tundra);
            }
        }
        break;

    case L_DEEP_OCEAN_256: // biomes added in (L_DEEP_OCEAN_256, L_BIOME_256]
        if (mc <= MC_1_6) goto L_bad_layer;
        switch (id & ~0xf00)
        {
        case Warm:
            if (id & 0xf00) {
                genPotential(mL, mM, L_BIOME_256, mc, badlands_plateau);
                genPotential(mL, mM, L_BIOME_256, mc, wooded_badlands_plateau);
            } else {
                genPotential(mL, mM, L_BIOME_256, mc, desert);
                genPotential(mL, mM, L_BIOME_256, mc, savanna);
                genPotential(mL, mM, L_BIOME_256, mc, plains);
            }
            break;
        case Lush:
            if (id & 0xf00) {
                genPotential(mL, mM, L_BIOME_256, mc, jungle);
            } else {
                genPotential(mL, mM, L_BIOME_256, mc, forest);
                genPotential(mL, mM, L_BIOME_256, mc, dark_forest);
                genPotential(mL, mM, L_BIOME_256, mc, mountains);
                genPotential(mL, mM, L_BIOME_256, mc, plains);
                genPotential(mL, mM, L_BIOME_256, mc, birch_forest);
                genPotential(mL, mM, L_BIOME_256, mc, swamp);
            }
            break;
        case Cold:
            if (id & 0xf00) {
                genPotential(mL, mM, L_BIOME_256, mc, giant_tree_taiga);
            } else {
                genPotential(mL, mM, L_BIOME_256, mc, forest);
                genPotential(mL, mM, L_BIOME_256, mc, mountains);
                genPotential(mL, mM, L_BIOME_256, mc, taiga);
                genPotential(mL, mM, L_BIOME_256, mc, plains);
            }
            break;
        case Freezing:
            genPotential(mL, mM, L_BIOME_256, mc, snowy_tundra);
            genPotential(mL, mM, L_BIOME_256, mc, snowy_taiga);
            break;
        default:
            id &= ~0xf00;
            genPotential(mL, mM, L_BIOME_256, mc, id);
        }
        break;

    case L_BIOME_256: // biomes added in (L_BIOME_256, L_BIOME_EDGE_64]
        if (mc >= MC_1_7) {
            if (mc >= MC_1_14 && id == jungle)
                genPotential(mL, mM, L_BIOME_EDGE_64, mc, bamboo_jungle);
            if (id == wooded_badlands_plateau || id == badlands_plateau)
                genPotential(mL, mM, L_BIOME_EDGE_64, mc, badlands);
            else if(id == giant_tree_taiga)
                genPotential(mL, mM, L_BIOME_EDGE_64, mc, taiga);
            else if (id == desert)
                genPotential(mL, mM, L_BIOME_EDGE_64, mc, wooded_mountains);
            else if (id == swamp) {
                genPotential(mL, mM, L_BIOME_EDGE_64, mc, jungle_edge);
                genPotential(mL, mM, L_BIOME_EDGE_64, mc, plains);
            }
            genPotential(mL, mM, L_BIOME_EDGE_64, mc, id);
            break;
        }
        // (L_BIOME_256, L_HILLS_64] for 1.6
        // fallthrough

    case L_BIOME_EDGE_64: // biomes added in (L_BIOME_EDGE_64, L_HILLS_64]
        if (mc <= MC_1_6 && layer == L_BIOME_EDGE_64) goto L_bad_layer;
        if (!isShallowOcean(id) && getMutated(mc, id) > 0)
             genPotential(mL, mM, L_HILLS_64, mc, getMutated(mc, id));
        switch (id)
        {
        case desert:
            genPotential(mL, mM, L_HILLS_64, mc, desert_hills);
            break;
        case forest:
            genPotential(mL, mM, L_HILLS_64, mc, wooded_hills);
            break;
        case birch_forest:
            genPotential(mL, mM, L_HILLS_64, mc, birch_forest_hills);
            genPotential(mL, mM, L_HILLS_64, mc, getMutated(mc, birch_forest_hills));
            break;
        case dark_forest:
            genPotential(mL, mM, L_HILLS_64, mc, plains);
            genPotential(mL, mM, L_HILLS_64, mc, getMutated(mc, plains));
            break;
        case taiga:
            genPotential(mL, mM, L_HILLS_64, mc, taiga_hills);
            break;
        case giant_tree_taiga:
            genPotential(mL, mM, L_HILLS_64, mc, giant_tree_taiga_hills);
            genPotential(mL, mM, L_HILLS_64, mc, getMutated(mc, giant_tree_taiga_hills));
            break;
        case plains:
            if (mc >= MC_1_7)
                genPotential(mL, mM, L_HILLS_64, mc, wooded_hills);
            genPotential(mL, mM, L_HILLS_64, mc, forest);
            genPotential(mL, mM, L_HILLS_64, mc, getMutated(mc, forest));
            break;
        case snowy_tundra:
            genPotential(mL, mM, L_HILLS_64, mc, snowy_mountains);
            break;
        case bamboo_jungle:
            genPotential(mL, mM, L_HILLS_64, mc, bamboo_jungle_hills);
            break;
        case ocean:
            if (mc >= MC_1_7)
                genPotential(mL, mM, L_HILLS_64, mc, deep_ocean);
            break;
        case mountains:
            if (mc >= MC_1_7) {
                genPotential(mL, mM, L_HILLS_64, mc, wooded_mountains);
                genPotential(mL, mM, L_HILLS_64, mc, getMutated(mc, wooded_mountains));
            }
            break;
        case savanna:
            genPotential(mL, mM, L_HILLS_64, mc, savanna_plateau);
            genPotential(mL, mM, L_HILLS_64, mc, getMutated(mc, savanna_plateau));
            break;
        default:
            if (areSimilar(mc, id, wooded_badlands_plateau))
            {
                genPotential(mL, mM, L_HILLS_64, mc, badlands);
                genPotential(mL, mM, L_HILLS_64, mc, getMutated(mc, badlands));
            }
            else if (isDeepOcean(id))
            {
                genPotential(mL, mM, L_HILLS_64, mc, plains);
                genPotential(mL, mM, L_HILLS_64, mc, forest);
                genPotential(mL, mM, L_HILLS_64, mc, getMutated(mc, plains));
                genPotential(mL, mM, L_HILLS_64, mc, getMutated(mc, forest));
            }
        }
        genPotential(mL, mM, L_HILLS_64, mc, id);
        break;

    case L_HILLS_64: // biomes added in (L_HILLS_64, L_RARE_BIOME_64]
        if (mc <= MC_1_6) { // (L_HILLS_64, L_SHORE_16] for 1.6
            if (id == mushroom_fields)
                genPotential(mL, mM, L_SHORE_16, mc, mushroom_field_shore);
            else if (id == mountains)
                genPotential(mL, mM, L_SHORE_16, mc, mountain_edge);
            else if (id != ocean && id != river && id != swamp)
                genPotential(mL, mM, L_SHORE_16, mc, beach);
            genPotential(mL, mM, L_SHORE_16, mc, id);
        } else {
            if (id == plains)
                genPotential(mL, mM, L_SUNFLOWER_64, mc, sunflower_plains);
            genPotential(mL, mM, L_SUNFLOWER_64, mc, id);
        }
        break;

    case L_SUNFLOWER_64: // biomes added in (L_SUNFLOWER_64, L_SHORE_16] 1.7+
        if (mc <= MC_1_6) goto L_bad_layer;
        if (id == mushroom_fields)
            genPotential(mL, mM, L_SHORE_16, mc, mushroom_field_shore);
        else if (getCategory(mc, id) == jungle) {
            genPotential(mL, mM, L_SHORE_16, mc, beach);
            genPotential(mL, mM, L_SHORE_16, mc, jungle_edge);
        }
        else if (id == mountains || id == wooded_mountains || id == mountain_edge)
            genPotential(mL, mM, L_SHORE_16, mc, stone_shore);
        else if (isSnowy(id))
            genPotential(mL, mM, L_SHORE_16, mc, snowy_beach);
        else if (id == badlands || id == wooded_badlands_plateau)
            genPotential(mL, mM, L_SHORE_16, mc, desert);
        else if (id != ocean && id != deep_ocean && id != river && id != swamp)
            genPotential(mL, mM, L_SHORE_16, mc, beach);
        genPotential(mL, mM, L_SHORE_16, mc, id);
        break;

    case L_SHORE_16: // biomes added in (L_SHORE_16, L_RIVER_MIX_4]
        if (id == snowy_tundra)
            genPotential(mL, mM, L_RIVER_MIX_4, mc, frozen_river);
        else if (id == mushroom_fields || id == mushroom_field_shore)
            genPotential(mL, mM, L_RIVER_MIX_4, mc, mushroom_field_shore);
        else if (id != ocean && (mc < MC_1_7 || !isOceanic(id)))
            genPotential(mL, mM, L_RIVER_MIX_4, mc, river);
        genPotential(mL, mM, L_RIVER_MIX_4, mc, id);
        break;

    case L_RIVER_MIX_4: // biomes added in (L_RIVER_MIX_4, L_VORONOI_1]
        if (mc >= MC_1_13 && isOceanic(id)) {
            if (id == ocean) {
                genPotential(mL, mM, L_VORONOI_1, mc, ocean);
                genPotential(mL, mM, L_VORONOI_1, mc, warm_ocean);
                genPotential(mL, mM, L_VORONOI_1, mc, lukewarm_ocean);
                genPotential(mL, mM, L_VORONOI_1, mc, cold_ocean);
                genPotential(mL, mM, L_VORONOI_1, mc, frozen_ocean);
            } else if (id == deep_ocean) {
                genPotential(mL, mM, L_VORONOI_1, mc, deep_ocean);
                genPotential(mL, mM, L_VORONOI_1, mc, deep_lukewarm_ocean);
                genPotential(mL, mM, L_VORONOI_1, mc, deep_cold_ocean);
                genPotential(mL, mM, L_VORONOI_1, mc, deep_frozen_ocean);
            }
            else break;
        }
        genPotential(mL, mM, L_VORONOI_1, mc, id);
        break;

    case L_VORONOI_1:
        if (id < 128)   *mL |= 1ULL << id;
        else            *mM |= 1ULL << (id-128);
        break;

    default:
        printf("genPotential() not implemented for layer %d\n", layer);
    }
    if (0)
    {
    L_bad_layer:
        printf("genPotential() bad layer %d for version\n", layer);
    }
}






