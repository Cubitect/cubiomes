#include "finders.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <float.h>

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


uint64_t *loadSavedSeeds(const char *fnam, uint64_t *scnt)
{
    FILE *fp = fopen(fnam, "r");

    uint64_t seed, i;
    uint64_t *baseSeeds;

    if (fp == NULL)
        return NULL;

    *scnt = 0;

    while (!feof(fp))
    {
        if (fscanf(fp, "%" PRId64, (int64_t*)&seed) == 1) (*scnt)++;
        else while (!feof(fp) && fgetc(fp) != '\n');
    }

    if (*scnt == 0)
        return NULL;

    baseSeeds = (uint64_t*) calloc(*scnt, sizeof(*baseSeeds));

    rewind(fp);

    for (i = 0; i < *scnt && !feof(fp);)
    {
        if (fscanf(fp, "%" PRId64, (int64_t*)&baseSeeds[i]) == 1) i++;
        else while (!feof(fp) && fgetc(fp) != '\n');
    }

    fclose(fp);

    return baseSeeds;
}



//==============================================================================
// Finding Structure Positions
//==============================================================================


void setAttemptSeed(uint64_t *s, int cx, int cz)
{
    *s ^= (uint64_t)(cx >> 4) ^ ( (uint64_t)(cz >> 4) << 4 );
    setSeed(s, *s);
    next(s, 31);
}

uint64_t getPopulationSeed(int mc, uint64_t ws, int x, int z)
{
    Xoroshiro xr;
    uint64_t s;
    uint64_t a, b;

    if (mc >= MC_1_18)
    {
        xSetSeed(&xr, ws);
        a = xNextLongJ(&xr);
        b = xNextLongJ(&xr);
    }
    else
    {
        setSeed(&s, ws);
        a = nextLong(&s);
        b = nextLong(&s);
    }
    if (mc >= MC_1_13)
    {
        a |= 1; b |= 1;
    }
    else
    {
        a = (int64_t)a / 2 * 2 + 1;
        b = (int64_t)b / 2 * 2 + 1;
    }
    return (x * a + z * b) ^ ws;
}


int getStructureConfig(int structureType, int mc, StructureConfig *sconf)
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
        *sconf = mc <= MC_1_17 ? VILLAGE_CONFIG_117 : VILLAGE_CONFIG;
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
    case Ruined_Portal_N:
        *sconf = RUINED_PORTAL_N_CONFIG;
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
    case Mineshaft:
        *sconf = MINESHAFT_CONFIG;
        return 1;
    case Fortress:
        *sconf = mc <= MC_1_15 ? FORTRESS_CONFIG_115 : FORTRESS_CONFIG;
        return 1;
    case Bastion:
        *sconf = BASTION_CONFIG;
        return mc >= MC_1_16;
    case End_Gateway:
        *sconf = END_GATEWAY_CONFIG;
        return mc >= MC_1_13;
    default:
        memset(sconf, 0, sizeof(StructureConfig));
        return 0;
    }
}


// like getFeaturePos(), but modifies the rng seed
static inline
void getRegPos(Pos *p, uint64_t *s, int rx, int rz, StructureConfig sc)
{
    setSeed(s, rx*341873128712ULL + rz*132897987541ULL + *s + sc.salt);
    p->x = (int)(((uint64_t)rx * sc.regionSize + nextInt(s, sc.chunkRange)) << 4);
    p->z = (int)(((uint64_t)rz * sc.regionSize + nextInt(s, sc.chunkRange)) << 4);
}

int getStructurePos(int structureType, int mc, uint64_t seed, int regX, int regZ, Pos *pos)
{
    StructureConfig sconf;
#if STRUCT_CONFIG_OVERRIDE
    if (!getStructureConfig_override(structureType, mc, &sconf))
#else
    if (!getStructureConfig(structureType, mc, &sconf))
#endif
    {
        return 0;
    }

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
    case Ruined_Portal_N:
        *pos = getFeaturePos(sconf, seed, regX, regZ);
        return 1;

    case Monument:
    case Mansion:
        *pos = getLargeStructurePos(sconf, seed, regX, regZ);
        return 1;

    case End_City:
        *pos = getLargeStructurePos(sconf, seed, regX, regZ);
        return (pos->x*(int64_t)pos->x + pos->z*(int64_t)pos->z) >= 1008*1008LL;

    case Outpost:
        *pos = getFeaturePos(sconf, seed, regX, regZ);
        setAttemptSeed(&seed, (pos->x) >> 4, (pos->z) >> 4);
        return nextInt(&seed, 5) == 0;

    case Treasure:
        pos->x = (int)( ((uint32_t)regX << 4) + 9 );
        pos->z = (int)( ((uint32_t)regZ << 4) + 9 );
        seed = regX*341873128712ULL + regZ*132897987541ULL + seed + sconf.salt;
        setSeed(&seed, seed);
        return nextFloat(&seed) < 0.01;

    case Mineshaft:
        return getMineshafts(mc, seed, regX, regZ, regX, regZ, pos, 1);

    case Fortress:
        if (mc >= MC_1_18) {
            *pos = getFeaturePos(sconf, seed, regX, regZ);
            seed = chunkGenerateRnd(seed, pos->x >> 4, pos->z >> 4);
            return nextInt(&seed, 5) < 2;
        } else if (mc >= MC_1_16) {
            getRegPos(pos, &seed, regX, regZ, sconf);
            return nextInt(&seed, 5) < 2;
        } else {
            setAttemptSeed(&seed, regX << 4, regZ << 4);
            int valid = nextInt(&seed, 3) == 0;
            pos->x = (int)((((uint64_t)regX << 4) + nextInt(&seed,8) + 4) << 4);
            pos->z = (int)((((uint64_t)regZ << 4) + nextInt(&seed,8) + 4) << 4);
            return valid;
        }

    case Bastion:
        if (mc >= MC_1_18) {
            *pos = getFeaturePos(sconf, seed, regX, regZ);
            seed = chunkGenerateRnd(seed, pos->x >> 4, pos->z >> 4);
            return nextInt(&seed, 5) >= 2;
        } else {
            getRegPos(pos, &seed, regX, regZ, sconf);
            return nextInt(&seed, 5) >= 2;
        }

    case End_Gateway:
        pos->x = (int)( ((uint32_t)regX << 4) );
        pos->z = (int)( ((uint32_t)regZ << 4) );
        seed = getPopulationSeed(mc, seed, pos->x, pos->z);
        if (mc >= MC_1_18)
        {
            Xoroshiro xr;
            seed += 10000*4;
            xSetSeed(&xr, seed);
            if (xNextFloat(&xr) >= 1.0/700)
                return 0;
            pos->x += xNextIntJ(&xr, 16);
            pos->z += xNextIntJ(&xr, 16);
        }
        else
        {
            seed = getPopulationSeed(mc, seed, pos->x, pos->z);
            setSeed(&seed, seed + sconf.salt);
            if (mc <= MC_1_16) {
                if (nextInt(&seed, 700) != 0)
                    return 0;
            } else {
                if (nextFloat(&seed) >= 1.0/700)
                    return 0;
            }
            pos->x += nextInt(&seed, 16);
            pos->z += nextInt(&seed, 16);
        }
        return 1;

    default:
        fprintf(stderr,
                "ERR getStructurePos: unsupported structure type %d\n", structureType);
        exit(-1);
    }
    return 0;
}


int getMineshafts(int mc, uint64_t seed, int cx0, int cz0, int cx1, int cz1,
        Pos *out, int nout)
{
    uint64_t s;
    setSeed(&s, seed);
    uint64_t a = nextLong(&s);
    uint64_t b = nextLong(&s);
    int i, j;
    int n = 0;

    for (i = cx0; i <= cx1; i++)
    {
        uint64_t aix = i * a ^ seed;

        for (j = cz0; j <= cz1; j++)
        {
            setSeed(&s, aix ^ j * b);

            if (mc >= MC_1_13)
            {
                if U(nextDouble(&s) < 0.004)
                {
                    if (out && n < nout)
                    {
                        out[n].x = (int)((uint32_t)i << 4);
                        out[n].z = (int)((uint32_t)j << 4);
                    }
                    n++;
                }
            }
            else
            {
                skipNextN(&s, 1);
                if U(nextDouble(&s) < 0.004)
                {
                    int d = i;
                    if (-i > d) d = -i;
                    if (+j > d) d = +j;
                    if (-j > d) d = -j;
                    if (d >= 80 || nextInt(&s, 80) < d)
                    {
                        if (out && n < nout)
                        {
                            out[n].x = (int)((uint32_t)i << 4);
                            out[n].z = (int)((uint32_t)j << 4);
                        }
                        n++;
                    }
                }
            }
        }
    }

    return n;
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
    uint64_t seeds[100];
    size_t len;
    linked_seeds_t *next;
};

STRUCT(threadinfo_t)
{
    // seed range
    uint64_t start, end;
    const uint64_t *lowBits;
    int lowBitCnt;
    int lowBitN;

    // testing function
    int (*check)(uint64_t, void*);
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
        else if (!S_ISDIR(st.st_mode))
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

    uint64_t seed = info->start;
    uint64_t end = info->end;
    linked_seeds_t *lp = &info->ls;
    lp->len = 0;
    lp->next = NULL;

    if (info->lowBits)
    {
        uint64_t hstep = 1ULL << info->lowBitN;
        uint64_t hmask = ~(hstep - 1);
        uint64_t mid;
        int idx;

        mid = info->start & hmask;
        for (idx = 0; (seed = mid | info->lowBits[idx]) < info->start; idx++);

        while (seed <= end)
        {
            if U(info->check(seed, info->data))
            {
                if (info->fp)
                {
                    fprintf(info->fp, "%" PRId64"\n", (int64_t)seed);
                    fflush(info->fp);
                }
                else
                {
                    lp->seeds[lp->len] = seed;
                    lp->len++;
                    if (lp->len >= sizeof(lp->seeds)/sizeof(uint64_t))
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
                    fprintf(info->fp, "%" PRId64"\n", (int64_t)seed);
                    fflush(info->fp);
                }
                else
                {
                    lp->seeds[lp->len] = seed;
                    lp->len++;
                    if (lp->len >= sizeof(lp->seeds)/sizeof(uint64_t))
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
        uint64_t **         seedbuf,
        uint64_t *          buflen,
        const char *        path,
        int                 threads,
        const uint64_t *    lowBits,
        int                 lowBitCnt,
        int                 lowBitN,
        int (*check)(uint64_t s48, void *data),
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

        *seedbuf = (uint64_t*) malloc((*buflen) * sizeof(uint64_t));
        if (*seedbuf == NULL)
            exit(1);

        i = 0;
        for (t = 0; t < threads; t++)
        {
            linked_seeds_t *lp = &info[t].ls;
            do
            {
                memcpy(*seedbuf + i, lp->seeds, lp->len * sizeof(uint64_t));
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
int scanForQuadBits(const StructureConfig sconf, int radius, uint64_t s48,
        uint64_t lbit, int lbitn, uint64_t invB, int64_t x, int64_t z,
        int64_t w, int64_t h, Pos *qplist, int n)
{
    const uint64_t m = (1ULL << lbitn);
    const uint64_t A = 341873128712ULL;
    // for lbitn=20: invB = 132477LL;

    if (n < 1)
        return 0;
    lbit &= m-1;

    int64_t i, j;
    int cnt = 0;
    for (i = x; i <= x+w; i++)
    {
        uint64_t sx = s48 + A * i;
        j = (z & ~(m-1)) | ((lbit - sx) * invB & (m-1));
        if (j < z)
            j += m;
        for (; j <= z+h; j += m)
        {
            uint64_t sp = moveStructure(s48, -i, -j);
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
        const StructureConfig sconf, int radius, uint64_t s48,
        const uint64_t *lowBits, int lowBitCnt, int lowBitN, uint64_t salt,
        int x, int z, int w, int h, Pos *qplist, int n)
{
    int i, cnt = 0;
    uint64_t invB;
    if (lowBitN == 20)
        invB = 132477ULL;
    else if (lowBitN == 48)
        invB = 211541297333629ULL;
    else
        invB = mulInv(132897987541ULL, (1ULL << lowBitN));

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


Pos locateBiome(
    const Generator *g, int x, int y, int z, int radius,
    const char *validBiomes, uint64_t *rng, int *passes)
{
    Pos out = {x, z};
    int i, j, found;
    found = 0;

    if (g->mc >= MC_1_18)
    {
        x >>= 2;
        z >>= 2;
        radius >>= 2;
        uint64_t dat = 0;

        for (j = -radius; j <= radius; j++)
        {
            for (i = -radius; i <= radius; i++)
            {
                int id, xi = x+i, zj = z+j;
                // emulate dependent biome generation MC-241546
                //id = getBiomeAt(g, 4, xi, y, zj);
                id = sampleBiomeNoise(&g->bn, NULL, xi, y, zj, &dat, 0);

                if (!validBiomes[id]) continue;
                if (found == 0 || nextInt(rng, found+1) == 0)
                {
                    out.x = (x+i) << 2;
                    out.z = (z+j) << 2;
                }
                found++;
            }
        }
    }
    else
    {
        int x1 = (x-radius) >> 2;
        int z1 = (z-radius) >> 2;
        int x2 = (x+radius) >> 2;
        int z2 = (z+radius) >> 2;
        int width  = x2 - x1 + 1;
        int height = z2 - z1 + 1;

        Range r = {4, x1, z1, width, height, y, 1};
        int *ids = allocCache(g, r);
        genBiomes(g, ids, r);

        if (g->mc >= MC_1_13)
        {
            for (i = 0, j = 2; i < width*height; i++)
            {
                if (!validBiomes[ids[i]]) continue;
                if (found == 0 || nextInt(rng, j++) == 0)
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
                if (!validBiomes[ids[i]]) continue;
                if (found == 0 || nextInt(rng, found + 1) == 0)
                {
                    out.x = (x1 + i%width) << 2;
                    out.z = (z1 + i/width) << 2;
                    ++found;
                }
            }
        }

        free(ids);
    }


    if (passes != NULL)
    {
        *passes = found;
    }

    return out;
}



static inline int valid_1x1(const Generator *g, int x, int y, int z,
    Range r, int *buf, const char *valid)
{
    int *p = buf + (x-r.x) + (z-r.z)*r.sx + (y-r.y)*(r.sx*r.sz);
    if (*p)
        return 1;
    *p = -1;
    int id = getBiomeAt(g, 4, x, y, z);
    return valid[id];
}

int areBiomesViable(
    const Generator *g, int x, int y, int z, int rad,
    const char *validBiomes, int approx)
{
    int x1 = (x - rad) >> 2, x2 = (x + rad) >> 2, sx = x2 - x1 + 1;
    int z1 = (z - rad) >> 2, z2 = (z + rad) >> 2, sz = z2 - z1 + 1;
    int y1, y2, sy;
    if (g->mc >= MC_1_18)
    {
        y1 = (y - rad) >> 2, y2 = (y + rad) >> 2, sy = y2 - y1 + 1;
    }
    else
    {
        y1 = y2 = 0, sy = 1;
    }

    Range r = {4, x1, z1, sx, sz, y1, sy};
    int *ids = allocCache(g, r);
    int i, j, k;
    int viable = 1;
    const char *v = validBiomes;

    // check corners
    if (!valid_1x1(g, x1, y1, z1, r, ids, v)) goto L_no;
    if (!valid_1x1(g, x2, y2, z2, r, ids, v)) goto L_no;
    if (!valid_1x1(g, x1, y1, z2, r, ids, v)) goto L_no;
    if (!valid_1x1(g, x2, y2, z1, r, ids, v)) goto L_no;
    if (g->mc >= MC_1_18)
    {   // 3D
        if (!valid_1x1(g, x1, y2, z1, r, ids, v)) goto L_no;
        if (!valid_1x1(g, x2, y1, z2, r, ids, v)) goto L_no;
        if (!valid_1x1(g, x1, y2, z2, r, ids, v)) goto L_no;
        if (!valid_1x1(g, x2, y1, z1, r, ids, v)) goto L_no;
    }
    if (approx >= 1) goto L_yes;

    if (g->mc >= MC_1_18)
    {
        for (i = 0; i < sx; i++)
        {
            for (j = 0; j < sy; j++)
            {
                for (k = 0; k < sz; k++)
                {
                    if (!valid_1x1(g, x1+i, y1+j, z1+k, r, ids, v))
                        goto L_no;
                }
            }
        }
    }
    else if ((viable = !genBiomes(g, ids, r)))
    {
        for (i = 0; i < sx*sy*sz; i++)
        {
            if (!v[ids[i]])
                goto L_no;
        }
    }

    if (0) L_yes: viable = 1;
    if (0) L_no:  viable = 0;
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
        bamboo_jungle, bamboo_jungle_hills, dripstone_caves, lush_caves, meadow,
        grove, snowy_slopes, stony_peaks, jagged_peaks, frozen_peaks,
    };
    unsigned int i;
    static char v15[256], v17[256], v18[256];
    char *valid = (mc <= MC_1_15 ? v15 : mc <= MC_1_17 ? v17 : v18);

    if (!valid[strongholdBiomes[0]])
    {
        for (i = 0; i < sizeof(strongholdBiomes)/sizeof(int); i++)
            valid[ strongholdBiomes[i] ] = 1;

        if (mc >= MC_1_18)
        {
            valid[stone_shore] = 0;
        }
        else if (mc >= MC_1_16)
        {   // simulate MC-199298
            valid[bamboo_jungle] = 0;
            valid[bamboo_jungle_hills] = 0;
        }
    }
    return valid;
}

Pos initFirstStronghold(StrongholdIter *sh, int mc, uint64_t s48)
{
    double dist, angle;
    uint64_t rnds;
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

int nextStronghold(StrongholdIter *sh, const Generator *g)
{
    sh->pos = locateBiome(g, sh->nextapprox.x, 0, sh->nextapprox.z, 112,
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


static double getGrassProbability(uint64_t seed, int biome, int x, int z)
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
    static const int biomesToSpawnIn[] = {
        forest, plains, taiga, taiga_hills, wooded_hills, jungle, jungle_hills
    };
    static char isValid[256];
    unsigned int i;

    if (!isValid[biomesToSpawnIn[0]])
        for (i = 0; i < sizeof(biomesToSpawnIn) / sizeof(int); i++)
            isValid[ biomesToSpawnIn[i] ] = 1;

    return isValid;
}


static int findServerSpawn(const Generator *g, int chunkX, int chunkZ,
    double *bx, double *bz, double *bn, double *accum)
{
    int x, z;

    if (g->mc >= MC_1_18)
    {
        // It seems the search for spawn in 1.18 looks for a block with a
        // solid top and a height above sea level. We can approximate this by
        // looking for a non-ocean biome at y=63 ~> y=16 at scale 1:4.
        for (x = 0; x < 4; x++)
        {
            for (z = 0; z < 4; z++)
            {
                int x4 = (chunkX << 2) + x, z4 = (chunkZ << 2) + z;
                int id = getBiomeAt(g, 4, x4, 16, z4);
                if (isOceanic(id) || id == river)
                    continue;
                *bx = x4 << 2;
                *bz = z4 << 2;
                *bn = 1;
                return 1;
            }
        }
        return 0;
    }
    else
    {
        Range r = {1, chunkX << 4, chunkZ << 4, 16, 16, 0, 1};
        int *area = allocCache(g, r);
        genBiomes(g, area, r);

        for (x = 0; x < 16; x++)
        {
            for (z = 0; z < 16; z++)
            {
                Pos pos = {r.x+x, r.z+z};
                int id = area[z*16 + x];
                double gp = getGrassProbability(g->mc, id, pos.x, pos.z);
                if (gp == 0)
                    continue;

                *bx += *accum * gp * pos.x;
                *bz += *accum * gp * pos.z;
                *bn += *accum * gp;

                *accum *= 1 - gp;
                if (*accum < 0.001)
                {
                    free(area);
                    return 1;
                }
            }
        }
        free(area);
        return 0;
    }
}

static
uint64_t getSpawnDist(const Generator *g, int x, int z)
{
    int64_t np[6];
    uint32_t flags = SAMPLE_NO_DEPTH | SAMPLE_NO_BIOME;
    sampleBiomeNoise(&g->bn, np, x>>2, 0, z>>2, NULL, flags);
    const int64_t spawn_np[][2] = {
        {-10000,10000},{-10000,10000},{-1100,10000},{-10000,10000},{0,0},
        {-10000,-1600},{1600,10000} // [6]: weirdness for the second noise point
    };
    uint64_t ds = 0, ds1 = 0, ds2 = 0;
    uint64_t a, b, q, i;
    for (i = 0; i < 5; i++)
    {
        a = +np[i] - (uint64_t)spawn_np[i][1];
        b = -np[i] + (uint64_t)spawn_np[i][0];
        q = (int64_t)a > 0 ? a : (int64_t)b > 0 ? b : 0;
        ds += q * q;
    }
    a = +np[5] - (uint64_t)spawn_np[5][1];
    b = -np[5] + (uint64_t)spawn_np[5][0];
    q = (int64_t)a > 0 ? a : (int64_t)b > 0 ? b : 0;
    ds1 = ds + q*q;
    a = +np[5] - (uint64_t)spawn_np[6][1];
    b = -np[5] + (uint64_t)spawn_np[6][0];
    q = (int64_t)a > 0 ? a : (int64_t)b > 0 ? b : 0;
    ds2 = ds + q*q;
    return ds1 <= ds2 ? ds1 : ds2;
}

static
void findFittest(const Generator *g, Pos *pos, uint64_t *fitness, double maxrad, double step)
{
    double rad = step, ang = 0;
    Pos p = *pos;
    while (rad <= maxrad)
    {
        int x = p.x + (int)(sin(ang) * rad);
        int z = p.z + (int)(cos(ang) * rad);
        // calc fitness
        double d = ((double)x*x + (double)z*z) / (2500*2500);
        uint64_t fit = (uint64_t)(d*d * 1e8);
        // calculate the distance to the noise points for spawn
        fit += getSpawnDist(g, x, z);
        if (fit < *fitness)
        {
            pos->x = x;
            pos->z = z;
            *fitness = fit;
        }

        ang += step / rad;
        if (ang <= M_PI*2)
            continue;
        ang = 0;
        rad += step;
    }
}

static
Pos findFittestPos(const Generator *g)
{
    Pos spawn = {0, 0};
    uint64_t fitness = getSpawnDist(g, 0, 0);
    findFittest(g, &spawn, &fitness, 2048.0, 512.0);
    findFittest(g, &spawn, &fitness, 512.0, 32.0);
    // center of chunk
    spawn.x = ((spawn.x >> 4) << 4) + 8;
    spawn.z = ((spawn.z >> 4) << 4) + 8;
    return spawn;
}


Pos getSpawn(const Generator *g)
{
    const char *isSpawnBiome = getValidSpawnBiomes();
    Pos spawn;
    int found;
    int i;

    uint64_t rnd = 0;
    if (g->mc <= MC_1_17)
    {
        setSeed(&rnd, g->seed);
        spawn = locateBiome(g, 0, 63, 0, 256, isSpawnBiome, &rnd, &found);
        if (!found)
        {
            spawn.x = spawn.z = 8;
        }
    }
    else
    {
        spawn = findFittestPos(g);
    }

    double accum = 1;
    double bx = 0;
    double bz = 0;
    double bn = 0;
    double gp;

    if (g->mc >= MC_1_13)
    {
        int j, k, u, v;
        j = k = u = v = 0;
        for (i = 0; i < 1024; i++)
        {
            if (j > -16 && j <= 16 && k > -16 && k <= 16)
            {
                if (findServerSpawn(g, (spawn.x>>4)+j, (spawn.z>>4)+k,
                    &bx, &bz, &bn, &accum))
                {
                    spawn.x = (int) round(bx / bn);
                    spawn.z = (int) round(bz / bn);
                    return spawn;
                }
            }

            if (j == k || (j < 0 && j == -k) || (j > 0 && j == 1 - k))
            {
                int tmp = u;
                u = -v;
                v = tmp;
            }
            j += u;
            k += v;
        }
    }
    else
    {
        for (i = 0; i < 1000; i++)
        {
            int biome = getBiomeAt(g, 1, spawn.x, 0, spawn.z);
            gp = getGrassProbability(g->seed, biome, spawn.x, spawn.z);

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

Pos estimateSpawn(const Generator *g)
{
    const char *isSpawnBiome = getValidSpawnBiomes();
    Pos spawn;

    if (g->mc <= MC_1_17)
    {
        int found;
        uint64_t rnd;
        setSeed(&rnd, g->seed);
        spawn = locateBiome(g, 0, 63, 0, 256, isSpawnBiome, &rnd, &found);
        if (!found)
        {
            spawn.x = spawn.z = 8;
        }
        if (g->mc >= MC_1_13)
        {
            spawn.x &= ~0xf;
            spawn.z &= ~0xf;
        }
    }
    else
    {
        spawn = findFittestPos(g);
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
        return biomeID == snowy_tundra || biomeID == snowy_taiga || biomeID == snowy_slopes;

    case Ocean_Ruin:
        if (mc < MC_1_13) return 0;
        return isOceanic(biomeID);

    case Shipwreck:
        if (mc < MC_1_13) return 0;
        return isOceanic(biomeID) || biomeID == beach || biomeID == snowy_beach;

    case Ruined_Portal:
    case Ruined_Portal_N:
        return mc >= MC_1_16;

    case Treasure:
        if (mc < MC_1_13) return 0;
        return biomeID == beach || biomeID == snowy_beach;

    case Mineshaft:
        return isOverworld(mc, biomeID);

    case Monument:
        if (mc < MC_1_8) return 0;
        return isDeepOcean(biomeID);

    case Outpost:
        if (mc < MC_1_14) return 0;
        if (mc >= MC_1_18) {
            switch (biomeID) {
            case desert:
            case plains:
            case savanna:
            case snowy_plains:
            case taiga:
            case meadow:
            case frozen_peaks:
            case jagged_peaks:
            case stony_peaks:
            case snowy_slopes:
            case grove:
                return 1;
            default:
                return 0;
            }
        }
        // fall through
    case Village:
        if (biomeID == plains || biomeID == desert || biomeID == savanna)
            return 1;
        if (mc >= MC_1_10 && biomeID == taiga)
            return 1;
        if (mc >= MC_1_14 && biomeID == snowy_tundra)
            return 1;
        if (mc >= MC_1_18 && biomeID == meadow)
            return 1;
        return 0;

    case Mansion:
        if (mc < MC_1_11) return 0;
        return biomeID == dark_forest || biomeID == dark_forest_hills;

    case Fortress:
        return (biomeID == nether_wastes || biomeID == soul_sand_valley ||
                biomeID == warped_forest || biomeID == crimson_forest ||
                biomeID == basalt_deltas);

    case Bastion:
        if (mc < MC_1_16) return 0;
        return (biomeID == nether_wastes || biomeID == soul_sand_valley ||
                biomeID == warped_forest || biomeID == crimson_forest);

    case End_City:
        if (mc < MC_1_9) return 0;
        return biomeID == end_midlands || biomeID == end_highlands;

    case End_Gateway:
        if (mc < MC_1_13) return 0;
        return biomeID == end_highlands;

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


int isViableStructurePos(int structureType, Generator *g, int x, int z, uint32_t flags)
{
    int approx = 0; // enables approximation levels
    int viable = 0;

    int64_t chunkX = x >> 4;
    int64_t chunkZ = z >> 4;

    // Structures are positioned at the chunk origin, but the biome check is
    // performed near the middle of the chunk [(9,9) in 1.13, TODO: check 1.7]
    // In 1.16 the biome check is always performed at (2,2) with layer scale=4.
    int sampleX, sampleZ;
    int id;


    if (g->dim == -1) // Nether
    {
        if (structureType == Fortress)
            return 1; // fortresses generate in all Nether biomes and versions
        if (g->mc < MC_1_16)
            return 0;
        if (structureType == Ruined_Portal_N)
            return 1;
        if (g->mc >= MC_1_18 && structureType == Bastion)
        {
            StructureVariant sv = getBastionType(g->mc, g->seed, x, z);
            switch (sv.rotation) {
                case 0: sampleX = -1+sv.sx; sampleZ = -1+sv.sz; break;
                case 1: sampleX = +1-sv.sz; sampleZ = -1+sv.sx; break;
                case 2: sampleX = +1-sv.sx; sampleZ = +1-sv.sz; break;
                case 3: sampleX = -1+sv.sz; sampleZ = +1-sv.sx; break;
                default: return 0; // unreachable
            }
            sampleX = ((chunkX << 5) + sampleX) / 2 >> 2;
            sampleZ = ((chunkZ << 5) + sampleZ) / 2 >> 2;
        }
        else
        {
            sampleX = (chunkX << 2) + 2;
            sampleZ = (chunkZ << 2) + 2;
        }
        id = getBiomeAt(g, 4, sampleX, 0, sampleZ);
        return isViableFeatureBiome(g->mc, structureType, id);
    }
    else if (g->dim == +1) // End
    {
        switch (structureType)
        {
        case End_City:
            if (g->mc < MC_1_9) return 0;
            break;
        case End_Gateway:
            if (g->mc < MC_1_13) return 0;
            break;
        default:
            return 0;
        }
        // End biomes vary only on a per-chunk scale (1:16)
        // voronoi pre-1.15 shouldn't matter for End Cities as the check will
        // be near the chunk center
        id = getBiomeAt(g, 16, chunkX, 0, chunkZ);
        return isViableFeatureBiome(g->mc, structureType, id) ? id : 0;
    }

    // Overworld

    Layer lbiome, lshore, *entry = 0;
    int data[2] = { structureType, g->mc };

    if (g->mc <= MC_1_17)
    {
        lbiome = g->ls.layers[L_BIOME_256];
        lshore = g->ls.layers[L_SHORE_16];
        entry = g->entry;

        g->ls.layers[L_BIOME_256].data = (void*) data;
        g->ls.layers[L_BIOME_256].getMap = mapViableBiome;
        g->ls.layers[L_SHORE_16].data = (void*) data;
        g->ls.layers[L_SHORE_16].getMap = mapViableShore;
    }

    switch (structureType)
    {
    case Ocean_Ruin:
    case Shipwreck:
    case Treasure:
        if (g->mc < MC_1_13) goto L_not_viable;
        goto L_feature;
    case Igloo:
        if (g->mc < MC_1_9) goto L_not_viable;
        goto L_feature;
    case Desert_Pyramid:
    case Jungle_Pyramid:
    case Swamp_Hut:
L_feature:
        if (g->mc >= MC_1_16)
        {
            if (g->mc < MC_1_18)
                g->entry = &g->ls.layers[L_RIVER_MIX_4];
            sampleX = (chunkX << 2) + 2;
            sampleZ = (chunkZ << 2) + 2;
        }
        else
        {
            g->entry = &g->ls.layers[L_VORONOI_1];
            sampleX = (chunkX << 4) + 9;
            sampleZ = (chunkZ << 4) + 9;
        }
        id = getBiomeAt(g, 0, sampleX, 319>>2, sampleZ);
        if (id < 0 || !isViableFeatureBiome(g->mc, structureType, id))
            goto L_not_viable;
        goto L_viable;

    case Village:
        if (g->mc <= MC_1_17)
        {
            g->entry = &g->ls.layers[L_RIVER_MIX_4];
            sampleX = (chunkX << 2) + 2;
            sampleZ = (chunkZ << 2) + 2;
            id = getBiomeAt(g, 0, sampleX, 0, sampleZ);
            if (id < 0 || !isViableFeatureBiome(g->mc, structureType, id))
                goto L_not_viable;
            if (flags && (uint32_t) id != flags)
                goto L_not_viable;
            viable = id; // biome for viablility, useful for further analysis
            goto L_viable;
        }
        else
        {   // In 1.18 village types are checked sepertely...
            const int vv[] = { plains, desert, savanna, taiga, snowy_tundra };
            size_t i;
            for (i = 0; i < sizeof(vv)/sizeof(int); i++) {
                if (flags && flags != (uint32_t) vv[i])
                    continue;
                StructureVariant vt = getVillageType(g->mc, g->seed, x, z, vv[i]);
                switch (vt.rotation) {
                    case 0: sampleX = -1+vt.sx; sampleZ = -1+vt.sz; break;
                    case 1: sampleX = +1-vt.sz; sampleZ = -1+vt.sx; break;
                    case 2: sampleX = +1-vt.sx; sampleZ = +1-vt.sz; break;
                    case 3: sampleX = -1+vt.sz; sampleZ = +1-vt.sx; break;
                    default: return 0; // unreachable
                }
                sampleX = ((chunkX << 5) + sampleX) / 2 >> 2;
                sampleZ = ((chunkZ << 5) + sampleZ) / 2 >> 2;
                id = getBiomeAt(g, 0, sampleX, 319>>2, sampleZ);
                if (id == vv[i] || (id == meadow && vv[i] == plains)) {
                    viable = id;
                    goto L_viable;
                }
            }
            goto L_not_viable;
        }

    case Outpost:
    {
        if (g->mc < MC_1_14)
            goto L_not_viable;
        uint64_t rng = g->seed;
        setAttemptSeed(&rng, chunkX, chunkZ);
        if (nextInt(&rng, 5) != 0)
            goto L_not_viable;
        // look for villages within 10 chunks
        StructureConfig vilconf;
        if (!getStructureConfig(Village, g->mc, &vilconf))
            goto L_not_viable;
        int cx0 = (chunkX-10), cx1 = (chunkX+10);
        int cz0 = (chunkZ-10), cz1 = (chunkZ+10);
        int rx0 = cx0 / vilconf.regionSize - (cx0 < 0);
        int rx1 = cx1 / vilconf.regionSize - (cx1 < 0);
        int rz0 = cz0 / vilconf.regionSize - (cz0 < 0);
        int rz1 = cz1 / vilconf.regionSize - (cz1 < 0);
        int rx, rz;
        for (rz = rz0; rz <= rz1; rz++)
        {
            for (rx = rx0; rx <= rx1; rx++)
            {
                Pos p = getFeaturePos(vilconf, g->seed, rx, rz);
                int cx = p.x >> 4, cz = p.z >> 4;
                if (cx >= cx0 && cx <= cx1 && cz >= cz0 && cz <= cz1)
                {
                    if (g->mc >= MC_1_16)
                        goto L_not_viable;
                    if (isViableStructurePos(Village, g, p.x, p.z, 0))
                        goto L_not_viable;
                    goto L_viable;
                }
            }
        }
        if (g->mc >= MC_1_18)
        {
            rng = chunkGenerateRnd(g->seed, chunkX, chunkZ);
            switch (nextInt(&rng, 4)) {
                case 0: sampleX = +15; sampleZ = +15; break;
                case 1: sampleX = -15; sampleZ = +15; break;
                case 2: sampleX = -15; sampleZ = -15; break;
                case 3: sampleX = +15; sampleZ = -15; break;
                default: return 0; // unreachable
            }
            sampleX = ((chunkX << 5) + sampleX) / 2 >> 2;
            sampleZ = ((chunkZ << 5) + sampleZ) / 2 >> 2;
        }
        else if (g->mc >= MC_1_16)
        {
            g->entry = &g->ls.layers[L_RIVER_MIX_4];
            sampleX = (chunkX << 2) + 2;
            sampleZ = (chunkZ << 2) + 2;
        }
        else
        {
            g->entry = &g->ls.layers[L_VORONOI_1];
            sampleX = (chunkX << 4) + 9;
            sampleZ = (chunkZ << 4) + 9;
        }
        id = getBiomeAt(g, 0, sampleX, 319>>2, sampleZ);
        if (id < 0 || !isViableFeatureBiome(g->mc, structureType, id))
            goto L_not_viable;
        goto L_viable;
    }

    case Monument:
        if (g->mc < MC_1_8)
            goto L_not_viable;
        else if (g->mc == MC_1_8)
        {   // In 1.8 monuments require only a single deep ocean block.
            id = getBiomeAt(g, 1, (chunkX << 4) + 8, 0, (chunkZ << 4) + 8);
            if (id < 0 || !isDeepOcean(id))
                goto L_not_viable;
        }
        else if (g->mc <= MC_1_17)
        {   // Monuments require two viability checks with the ocean layer
            // branch => worth checking for potential deep ocean beforehand.
            g->entry = &g->ls.layers[L_SHORE_16];
            id = getBiomeAt(g, 0, chunkX, 0, chunkZ);
            if (id < 0 || !isDeepOcean(id))
                goto L_not_viable;
        }
        sampleX = (chunkX << 4) + 8;
        sampleZ = (chunkZ << 4) + 8;
        if (g->mc >= MC_1_9 && g->mc <= MC_1_17)
        {   // check for deep ocean center
            if (!areBiomesViable(g, sampleX, 63, sampleZ, 16, getValidMonumentBiomes2(), approx))
                goto L_not_viable;
        }
        else if (g->mc >= MC_1_18)
        {   // check is done at y level of ocean floor - approx. with y = 36
            id = getBiomeAt(g, 4, sampleX>>2, 36>>2, sampleZ>>2);
            if (!isDeepOcean(id))
                goto L_not_viable;
        }
        if (areBiomesViable(g, sampleX, 63, sampleZ, 29, getValidMonumentBiomes1(), approx))
            goto L_viable;
        goto L_not_viable;

    case Mansion:
        if (g->mc < MC_1_11)
            goto L_not_viable;
        if (g->mc <= MC_1_17)
        {
            sampleX = (chunkX << 4) + 8;
            sampleZ = (chunkZ << 4) + 8;
            if (!areBiomesViable(g, sampleX, 0, sampleZ, 32, getValidMansionBiomes(), approx))
                goto L_not_viable;
        }
        else
        {   // In 1.18 the generation gets the minimum surface height among the
            // four structure corners (note structure has rotation).
            // This minimum height has to be y >= 60. The biome check is done
            // at the center position at that height.
            // TODO: get surface height
            sampleX = (chunkX << 4) + 7;
            sampleZ = (chunkZ << 4) + 7;
            id = getBiomeAt(g, 4, sampleX>>2, 319>>4, sampleZ>>2);
            if (id < 0 || !isViableFeatureBiome(g->mc, structureType, id))
                goto L_not_viable;
        }
        goto L_viable;

    case Ruined_Portal:
    case Ruined_Portal_N:
        if (g->mc >= MC_1_16)
            goto L_viable;
        goto L_not_viable;

    case Mineshaft:
        goto L_viable;

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
    if (g->mc <= MC_1_17)
    {
        g->ls.layers[L_BIOME_256] = lbiome;
        g->ls.layers[L_SHORE_16] = lshore;
        g->entry = entry;
    }
    return viable;
}


int isViableStructureTerrain(int structType, Generator *g, int x, int z)
{
    int id, sx, sz;
    if (g->mc < MC_1_18)
        return 1;
    if (structType == Desert_Pyramid || structType == Jungle_Temple)
    {
        sx = (structType == Desert_Pyramid ? 21 : 12);
        sz = (structType == Desert_Pyramid ? 21 : 15);
    }
    else if (structType == Mansion)
    {
        int cx = x >> 4, cz = z >> 4;
        uint64_t rng = chunkGenerateRnd(g->seed, cx, cz);
        int rot = nextInt(&rng, 4);
        sx = 5;
        sz = 5;
        if (rot == 0) { sx = -5; }
        if (rot == 1) { sx = -5; sz = -5; }
        if (rot == 2) { sz = -5; }
        x = (cx << 4) + 7;
        z = (cz << 4) + 7;
    }
    else
    {
        return 1;
    }

    id = getBiomeAt(g, 4, (x+sx)>>2, 15, (z)>>2);
    if (isOceanic(id) || id == river || id == frozen_river)
        return 0;
    id = getBiomeAt(g, 4, (x)>>2, 15, (z+sz)>>2);
    if (isOceanic(id) || id == river || id == frozen_river)
        return 0;
    id = getBiomeAt(g, 4, (x+sx)>>2, 15, (z+sz)>>2);
    if (isOceanic(id) || id == river || id == frozen_river)
        return 0;

    return 1;
}



/* Given bordering noise columns and a fractional position between those,
 * determine the surface block height (i.e. where the interpolated noise > 0).
 * Note that the noise columns should be of size: ncolxz[ colheight+1 ]
 */
int getSurfaceHeight(
        const double ncol00[], const double ncol01[],
        const double ncol10[], const double ncol11[],
        int colymin, int colymax, int blockspercell, double dx, double dz);

void sampleNoiseColumnEnd(double column[], const SurfaceNoise *sn,
        const EndNoise *en, int x, int z, int colymin, int colymax);

int isViableEndCityTerrain(const EndNoise *en, const SurfaceNoise *sn,
        int blockX, int blockZ)
{
    int chunkX = blockX >> 4;
    int chunkZ = blockZ >> 4;
    blockX = (chunkX << 4) + 7;
    blockZ = (chunkZ << 4) + 7;
    int cellx = (blockX >> 3);
    int cellz = (blockZ >> 3);
    // TODO: make sure upper bound is ok
    const int y0 = 15, y1 = 18; // only check range that could yield h >= 60
    double ncol[3][3][y1-y0+1];

    sampleNoiseColumnEnd(ncol[0][0], sn, en, cellx, cellz, y0, y1);
    sampleNoiseColumnEnd(ncol[0][1], sn, en, cellx, cellz+1, y0, y1);
    sampleNoiseColumnEnd(ncol[1][0], sn, en, cellx+1, cellz, y0, y1);
    sampleNoiseColumnEnd(ncol[1][1], sn, en, cellx+1, cellz+1, y0, y1);

    int h00, h01, h10, h11;
    h00 = getSurfaceHeight(ncol[0][0], ncol[0][1], ncol[1][0], ncol[1][1],
            y0, y1, 4, (blockX & 7) / 8.0, (blockZ & 7) / 8.0);

    uint64_t cs;
    setSeed(&cs, chunkX + chunkZ * 10387313ULL);
    switch (nextInt(&cs, 4))
    {
    case 0: // (++) 0
        sampleNoiseColumnEnd(ncol[0][2], sn, en, cellx+0, cellz+2, y0, y1);
        sampleNoiseColumnEnd(ncol[1][2], sn, en, cellx+1, cellz+2, y0, y1);
        sampleNoiseColumnEnd(ncol[2][0], sn, en, cellx+2, cellz+0, y0, y1);
        sampleNoiseColumnEnd(ncol[2][1], sn, en, cellx+2, cellz+1, y0, y1);
        sampleNoiseColumnEnd(ncol[2][2], sn, en, cellx+2, cellz+2, y0, y1);
        h01 = getSurfaceHeight(ncol[0][1], ncol[0][2], ncol[1][1], ncol[1][2],
                y0, y1, 4, ((blockX    ) & 7) / 8.0, ((blockZ + 5) & 7) / 8.0);
        h10 = getSurfaceHeight(ncol[1][0], ncol[1][1], ncol[2][0], ncol[2][1],
                y0, y1, 4, ((blockX + 5) & 7) / 8.0, ((blockZ    ) & 7) / 8.0);
        h11 = getSurfaceHeight(ncol[1][1], ncol[1][2], ncol[2][1], ncol[2][2],
                y0, y1, 4, ((blockX + 5) & 7) / 8.0, ((blockZ + 5) & 7) / 8.0);
        break;

    case 1: // (-+) 90
        sampleNoiseColumnEnd(ncol[0][2], sn, en, cellx+0, cellz+2, y0, y1);
        sampleNoiseColumnEnd(ncol[1][2], sn, en, cellx+1, cellz+2, y0, y1);
        h01 = getSurfaceHeight(ncol[0][1], ncol[0][2], ncol[1][1], ncol[1][2],
                y0, y1, 4, ((blockX    ) & 7) / 8.0, ((blockZ + 5) & 7) / 8.0);
        h10 = getSurfaceHeight(ncol[0][0], ncol[0][1], ncol[1][0], ncol[1][1],
                y0, y1, 4, ((blockX - 5) & 7) / 8.0, ((blockZ    ) & 7) / 8.0);
        h11 = getSurfaceHeight(ncol[0][1], ncol[0][2], ncol[1][1], ncol[1][2],
                y0, y1, 4, ((blockX - 5) & 7) / 8.0, ((blockZ + 5) & 7) / 8.0);
        break;

    case 2: // (--) 180
        h01 = getSurfaceHeight(ncol[0][0], ncol[0][1], ncol[1][0], ncol[1][1],
                y0, y1, 4, ((blockX    ) & 7) / 8.0, ((blockZ - 5) & 7) / 8.0);
        h10 = getSurfaceHeight(ncol[0][0], ncol[0][1], ncol[1][0], ncol[1][1],
                y0, y1, 4, ((blockX - 5) & 7) / 8.0, ((blockZ    ) & 7) / 8.0);
        h11 = getSurfaceHeight(ncol[0][0], ncol[0][1], ncol[1][0], ncol[1][1],
                y0, y1, 4, ((blockX - 5) & 7) / 8.0, ((blockZ - 5) & 7) / 8.0);
        break;

    case 3: // (+-) 270
        sampleNoiseColumnEnd(ncol[2][0], sn, en, cellx+2, cellz+0, y0, y1);
        sampleNoiseColumnEnd(ncol[2][1], sn, en, cellx+2, cellz+1, y0, y1);
        h01 = getSurfaceHeight(ncol[0][0], ncol[0][1], ncol[1][0], ncol[1][1],
                y0, y1, 4, ((blockX    ) & 7) / 8.0, ((blockZ - 5) & 7) / 8.0);
        h10 = getSurfaceHeight(ncol[1][0], ncol[1][1], ncol[2][0], ncol[2][1],
                y0, y1, 4, ((blockX + 5) & 7) / 8.0, ((blockZ    ) & 7) / 8.0);
        h11 = getSurfaceHeight(ncol[1][0], ncol[1][1], ncol[2][0], ncol[2][1],
                y0, y1, 4, ((blockX + 5) & 7) / 8.0, ((blockZ - 5) & 7) / 8.0);
        break;

    default:
        return 0; // error
    }
    //printf("%d %d %d %d\n", h00, h01, h10, h11);
    if (h01 < h00) h00 = h01;
    if (h10 < h00) h00 = h10;
    if (h11 < h00) h00 = h11;
    return h00 >= 60;
}


//==============================================================================
// Finding Properties of Structures
//==============================================================================


StructureVariant getVillageType(int mc, uint64_t seed, int blockX, int blockZ, int biomeID)
{
    StructureVariant r = { 0, 0, 0, 0, 0, 0, 0 };
    if (!isViableFeatureBiome(mc, Village, biomeID))
        return r;

    uint64_t rng = chunkGenerateRnd(seed, blockX >> 4, blockZ >> 4);

    r.biome = biomeID;

    if (mc >= MC_1_14)
    {
        r.rotation = nextInt(&rng, 4);
        int t;
        switch (biomeID)
        {
        case meadow:
            r.biome = plains;
            // fallthrough
        case plains:
            t = nextInt(&rng, 204);
            if      (t <  50) { r.variant = 0; r.sx =  9; r.sy = 4; r.sz =  9; } // plains_fountain_01
            else if (t < 100) { r.variant = 1; r.sx = 10; r.sy = 7; r.sz = 10; } // plains_meeting_point_1
            else if (t < 150) { r.variant = 2; r.sx =  8; r.sy = 5; r.sz = 15; } // plains_meeting_point_2
            else if (t < 200) { r.variant = 3; r.sx = 11; r.sy = 9; r.sz = 11; } // plains_meeting_point_3
            else if (t < 201) { r.variant = 0; r.sx =  9; r.sy = 4; r.sz =  9; r.abandoned = 1; }
            else if (t < 202) { r.variant = 1; r.sx = 10; r.sy = 7; r.sz = 10; r.abandoned = 1; }
            else if (t < 203) { r.variant = 2; r.sx =  8; r.sy = 5; r.sz = 15; r.abandoned = 1; }
            else if (t < 204) { r.variant = 3; r.sx = 11; r.sy = 9; r.sz = 11; r.abandoned = 1; }
            break;
        case desert:
            t = nextInt(&rng, 250);
            if      (t <  98) { r.variant = 1; r.sx = 17; r.sy = 6; r.sz =  9; } // desert_meeting_point_1
            else if (t < 196) { r.variant = 2; r.sx = 12; r.sy = 6; r.sz = 12; } // desert_meeting_point_2
            else if (t < 245) { r.variant = 3; r.sx = 15; r.sy = 6; r.sz = 15; } // desert_meeting_point_3
            else if (t < 247) { r.variant = 1; r.sx = 17; r.sy = 6; r.sz =  9; r.abandoned = 1; }
            else if (t < 249) { r.variant = 2; r.sx = 12; r.sy = 6; r.sz = 12; r.abandoned = 1; }
            else if (t < 250) { r.variant = 3; r.sx = 15; r.sy = 6; r.sz = 15; r.abandoned = 1; }
            break;
        case savanna:
            t = nextInt(&rng, 459);
            if      (t < 100) { r.variant = 1; r.sx = 14; r.sy = 5; r.sz = 12; } // savanna_meeting_point_1
            else if (t < 150) { r.variant = 2; r.sx = 11; r.sy = 6; r.sz = 11; } // savanna_meeting_point_2
            else if (t < 300) { r.variant = 3; r.sx =  9; r.sy = 6; r.sz = 11; } // savanna_meeting_point_3
            else if (t < 450) { r.variant = 4; r.sx =  9; r.sy = 6; r.sz =  9; } // savanna_meeting_point_4
            else if (t < 452) { r.variant = 1; r.sx = 14; r.sy = 5; r.sz = 12; r.abandoned = 1; }
            else if (t < 453) { r.variant = 2; r.sx = 11; r.sy = 6; r.sz = 11; r.abandoned = 1; }
            else if (t < 456) { r.variant = 3; r.sx =  9; r.sy = 6; r.sz = 11; r.abandoned = 1; }
            else if (t < 459) { r.variant = 4; r.sx =  9; r.sy = 6; r.sz =  9; r.abandoned = 1; }
            break;
        case taiga:
            t = nextInt(&rng, 100);
            if      (t <  49) { r.variant = 1; r.sx = 22; r.sy = 3; r.sz = 18; } // taiga_meeting_point_1
            else if (t <  98) { r.variant = 2; r.sx =  9; r.sy = 7; r.sz =  9; } // taiga_meeting_point_2
            else if (t <  99) { r.variant = 1; r.sx = 22; r.sy = 3; r.sz = 18; r.abandoned = 1; }
            else if (t < 100) { r.variant = 2; r.sx =  9; r.sy = 7; r.sz =  9; r.abandoned = 1; }
            break;
        case snowy_tundra:
            t = nextInt(&rng, 306);
            if      (t < 100) { r.variant = 1; r.sx = 12; r.sy = 8; r.sz =  8; } // snowy_meeting_point_1
            else if (t < 150) { r.variant = 2; r.sx = 11; r.sy = 5; r.sz =  9; } // snowy_meeting_point_2
            else if (t < 300) { r.variant = 3; r.sx =  7; r.sy = 7; r.sz =  7; } // snowy_meeting_point_3
            else if (t < 302) { r.variant = 1; r.sx = 12; r.sy = 8; r.sz =  8; r.abandoned = 1; }
            else if (t < 303) { r.variant = 2; r.sx = 11; r.sy = 5; r.sz =  9; r.abandoned = 1; }
            else if (t < 306) { r.variant = 3; r.sx =  7; r.sy = 7; r.sz =  7; r.abandoned = 1; }
            break;
        default:
            break;
        }
    }
    else if (mc >= MC_1_10)
    {
        skipNextN(&rng, mc == MC_1_13 ? 10 : 11);
        r.abandoned = nextInt(&rng, 50) == 0;
    }

    return r;
}

StructureVariant getBastionType(int mc, uint64_t seed, int blockX, int blockZ)
{
    (void) mc;
    StructureVariant r = { 0, 0, 0, 0, 0, 0, 0 };
    uint64_t rng = chunkGenerateRnd(seed, blockX >> 4, blockZ >> 4);
    r.biome = -1;
    r.rotation = nextInt(&rng, 4);
    r.variant = nextInt(&rng, 4);
    switch (r.variant)
    {
    case 0: r.sx = 46; r.sy = 24; r.sz = 46; break; // units/air_base
    case 1: r.sx = 30; r.sy = 24; r.sz = 48; break; // hoglin_stable/air_base
    case 2: r.sx = 38; r.sy = 48; r.sz = 38; break; // treasure/big_air_full
    case 3: r.sx = 16; r.sy = 32; r.sz = 32; break; // bridge/starting_pieces/entrance_base
    }
    return r;
}

uint64_t getHouseList(uint64_t worldSeed, int chunkX, int chunkZ,
        int *out)
{
    uint64_t rng = chunkGenerateRnd(worldSeed, chunkX, chunkZ);
    skipNextN(&rng, 1);

    out[HouseSmall] = nextInt(&rng, 4 - 2 + 1) + 2;
    out[Church]     = nextInt(&rng, 1 - 0 + 1) + 0;
    out[Library]    = nextInt(&rng, 2 - 0 + 1) + 0;
    out[WoodHut]    = nextInt(&rng, 5 - 2 + 1) + 2;
    out[Butcher]    = nextInt(&rng, 2 - 0 + 1) + 0;
    out[FarmLarge]  = nextInt(&rng, 4 - 1 + 1) + 1;
    out[FarmSmall]  = nextInt(&rng, 4 - 2 + 1) + 2;
    out[Blacksmith] = nextInt(&rng, 1 - 0 + 1) + 0;
    out[HouseLarge] = nextInt(&rng, 3 - 0 + 1) + 0;

    return rng;
}

//==============================================================================
// Seed Filters
//==============================================================================


BiomeFilter setupBiomeFilter(
    const int *required, int requiredLen,
    const int *excluded, int excludedLen)
{
    BiomeFilter bf;
    int i, id;

    memset(&bf, 0, sizeof(bf));

    for (i = 0; i < excludedLen; i++)
    {
        id = excluded[i];
        if (id < 128)
            bf.biomeToExcl |= (1ULL << id);
        else
            bf.biomeToExclM |= (1ULL << (id-128));
    }

    for (i = 0; i < requiredLen; i++)
    {
        id = required[i];
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


typedef struct
{
    Generator *g;
    int *ids;
    Range r;
    uint32_t flags;
    uint64_t b, m;
    uint64_t breq, mreq;
    uint64_t bexc, mexc;
    volatile char *stop;
} gdt_info_t;

static int f_graddesc_test(void *data, int x, int z, double p)
{
    (void) p;
    gdt_info_t *info = (gdt_info_t *) data;
    if (info->stop && *info->stop)
        return 1;
    int idx = (z - info->r.z) * info->r.sx + (x - info->r.x);
    if (info->ids[idx] != -1)
        return 0;
    int id = getBiomeAt(info->g, info->r.scale, x, info->r.y, z);
    info->ids[idx] = id;
    if (id < 128) info->b |= (1ULL << id);
    else info->m |= (1ULL << (id-128));

    if (info->bexc || info->mexc)
    {
        if ((info->b & info->bexc) || (info->m & info->mexc))
            return 1; // found an excluded biome
    }
    else if (info->flags & CFB_MATCH_ANY)
    {
        if ((info->b & info->breq) || (info->m & info->mreq))
            return 1; // one of the required biomes is present
    }
    else
    {   // no excluded: do the current biomes satisfy the condition?
        if (((info->b & info->breq) ^ info->breq) == 0 &&
            ((info->m & info->mreq) ^ info->mreq) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int checkForBiomes(
        Generator     * g,
        int           * cache,
        Range           r,
        int             dim,
        uint64_t        seed,
        BiomeFilter     filter,
        uint32_t        flags,
        volatile char * stop
        )
{
    if (stop && *stop)
        return 0;
    int i, j, k, ret;

    if (g->mc <= MC_1_17 && dim == 0)
    {
        Layer *entry = (Layer*) getLayerForScale(g, r.scale);
        ret = checkForBiomesAtLayer(&g->ls, entry, cache, seed,
            r.x, r.z, r.sx, r.sz, filter, flags);
        if (ret == 0 && r.sy > 1 && cache)
        {
            for (i = 0; i < r.sy; i++)
            {   // overworld has no vertical noise: expanding 2D into 3D
                for (j = 0; j < r.sx*r.sz; j++)
                    cache[i*r.sx*r.sz + j] = cache[j];
            }
        }
        return ret;
    }

    int *ids, id;
    if (cache)
        ids = cache;
    else
        ids = allocCache(g, r);

    if (g->dim != dim || g->seed != seed)
    {
        applySeed(g, dim, seed);
    }

    gdt_info_t info;
    info.g = g;
    info.ids = ids;
    info.r = r;
    info.flags = flags;
    info.b = info.m = 0;
    info.breq = filter.riverToFind;
    info.mreq = filter.riverToFindM;
    info.bexc = filter.biomeToExcl;
    info.mexc = filter.biomeToExclM;
    info.breq &= ~((1ULL << ocean) | (1ULL << deep_ocean));
    info.breq |= filter.oceanToFind;
    info.stop = stop;

    ret = 0;
    memset(ids, -1, r.sx * r.sz * sizeof(int));

    int n = r.sx*r.sy*r.sz;
    int trials = n;
    struct touple { int i, x, y, z; } *buf = NULL;

    if (r.scale == 4 && r.sx * r.sz > 64)
    {
        // Do a gradient descent to find the min/max of some climate parameters
        // and check the biomes along the way. This gives a much better chance
        // of fining the biomes that require the more exteme conditions early.
        double tmin, tmax;
        int err = 0;
        do
        {
            err = getParaRange(&g->bn.temperature, &tmin, &tmax,
                r.x, r.z, r.sx, r.sz, &info, f_graddesc_test);
            if (err) break;
            err = getParaRange(&g->bn.humidity, &tmin, &tmax,
                r.x, r.z, r.sx, r.sz, &info, f_graddesc_test);
            if (err) break;
            err = getParaRange(&g->bn.erosion, &tmin, &tmax,
                r.x, r.z, r.sx, r.sz, &info, f_graddesc_test);
            if (err) break;
            //err = getParaRange(&g->bn.continentalness, &tmin, &tmax,
            //    r.x, r.z, r.sx, r.sz, &info, f_graddesc_test);
            //if (err) break;
            //err = getParaRange(&g->bn.weirdness, &tmin, &tmax,
            //    r.x, r.z, r.sx, r.sz, &info, f_graddesc_test);
            //if (err) break;
        }
        while (0);
        if (err || (stop && *stop) || (flags & CFB_APPROX)) 
            goto L_end;
    }

    // We'll shuffle the coordinates so we'll generate the biomes in a
    // stochasitc mannor.
    buf = (struct touple*) malloc(n * sizeof(*buf));

    id = 0;
    for (k = 0; k < r.sy; k++)
    {
        for (j = 0; j < r.sz; j++)
        {
            for (i = 0; i < r.sx; i++)
            {
                buf[id].i = id;
                buf[id].x = i;
                buf[id].y = k;
                buf[id].z = j;
                id++;
            }
        }
    }

    // Determine a number of trials that gives a decent chance to sample all
    // the biomes that are present, assuming a completely random and
    // independent biome distribution. (This is actually not at all the case.)
    if (flags & CFB_APPROX)
    {
        int t = 400 + (int) sqrt(n);
        if (trials > t)
            trials = t;
    }

    for (i = 0; i < trials; i++)
    {
        struct touple t;
        j = n - i;
        k = rand() % j;
        t = buf[k];
        if (k != j-1)
        {
            buf[k] = buf[j-1];
            buf[j-1] = t;
        }

        if (stop && *stop)
            break;
        if (t.y == 0 && info.ids[t.i] != -1)
            continue;
        id = getBiomeAt(g, r.scale, r.x+t.x, r.y+t.y, r.z+t.z);
        info.ids[t.i] = id;
        if (id < 128) info.b |= (1ULL << id);
        else info.m |= (1ULL << (id-128));

        if (info.bexc || info.mexc)
        {
            if ((info.b & info.bexc) || (info.m & info.mexc))
                break; // found an excluded biome
        }
        else if (flags & CFB_MATCH_ANY)
        {
            if ((info.b & info.breq) || (info.m & info.mreq))
                break; // one of the required biomes is present
        }
        else
        {   // no excluded: do the current biomes satisfy the condition?
            if (((info.b & info.breq) ^ info.breq) == 0 && 
                ((info.m & info.mreq) ^ info.mreq) == 0)
                break;
        }
    }
    
L_end:
    if (stop && *stop)
    {
        ret = 0;
    }
    else
    {
        ret = (info.b & info.bexc) == 0 && (info.m & info.mexc) == 0;
        if (flags & CFB_MATCH_ANY)
            ret &= (info.b & info.breq) || (info.m & info.mreq);
        else
            ret &= (((info.b & info.breq) ^ info.breq) == 0 && 
                    ((info.m & info.mreq) ^ info.mreq) == 0);
    }

    if (buf)
        free(buf);
    if (ids != cache)
        free(ids);
    return ret;
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
        uint64_t ss = l->startSeed;
        uint64_t cs;

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
        uint64_t ss = l->startSeed;
        uint64_t cs;

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


int checkForBiomesAtLayer(
        LayerStack    * g,
        Layer         * entry,
        int           * cache,
        uint64_t        seed,
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
        l = entry;

        int i, j;
        int bx = x * l->scale;
        int bz = z * l->scale;
        int bw = w * l->scale;
        int bh = h * l->scale;
        int x0, z0, x1, z1;
        uint64_t ss, cs;
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
    int *ids;
    if (cache)
        ids = cache;
    else
        ids = (int*) calloc(getMinLayerCacheSize(entry, w, h), sizeof(int));

    filter_data_t fd[9];
    swapMap(fd+0, &filter, l+L_OCEAN_MIX_4,    mapFilterOceanMix);
    swapMap(fd+1, &filter, l+L_RIVER_MIX_4,    mapFilterRiverMix);
    swapMap(fd+2, &filter, l+L_SHORE_16,       mapFilterShore);
    swapMap(fd+3, &filter, l+L_SUNFLOWER_64,   mapFilterRareBiome);
    swapMap(fd+4, &filter, l+L_BIOME_EDGE_64,  mapFilterBiomeEdge);
    swapMap(fd+5, &filter, l+L_OCEAN_TEMP_256, mapFilterOceanTemp);
    swapMap(fd+6, &filter, l+L_BIOME_256,      mapFilterBiome);
    swapMap(fd+7, &filter, l+L_MUSHROOM_256,   mapFilterMushroom);
    swapMap(fd+8, &filter, l+L_SPECIAL_1024,   mapFilterSpecial);

    setLayerSeed(entry, seed);
    int ret = !entry->getMap(entry, ids, x, z, w, h);
    if (ret)
    {
        uint64_t req, b = 0, bm = 0;
        unsigned int i;
        for (i = 0; i < w*h; i++)
        {
            int id = ids[i];
            if (id < 128) b |= (1ULL << id);
            else bm |= (1ULL << (id-128));
        }
        req = filter.riverToFind;
        req &= ~((1ULL << ocean) | (1ULL << deep_ocean));
        req |= filter.oceanToFind;
        if ((b & req) ^ req)
            ret = 0;
        req = filter.riverToFindM;
        if ((bm & req) ^ req)
            ret = 0;
        if ((b & filter.biomeToExcl) || (bm & filter.biomeToExclM))
            ret = 0;
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


int checkForTemps(LayerStack *g, uint64_t seed, int x, int z, int w, int h, const int tc[9])
{
    uint64_t ls = getLayerSalt(3); // L_SPECIAL_1024 layer seed
    uint64_t ss = getStartSeed(seed, ls);

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
    int *area = (int*) calloc(getMinLayerCacheSize(l, w, h), sizeof(int));
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
        if (id == frozen_ocean && mc >= MC_1_7)
            return 0;
        if (isDeepOcean(id) && id != deep_ocean)
            return 0;
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
    case L_BAMBOO_256:
        if (mc < MC_1_14 && layer == L_BAMBOO_256) goto L_bad_layer;
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

    case L_OCEAN_MIX_4:
        if (mc <= MC_1_12) goto L_bad_layer;
        // fallthrough

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



double getParaDescent(const DoublePerlinNoise *para, double factor,
    int x, int z, int w, int h, int i0, int j0, int maxrad,
    int maxiter, double alpha, void *data, int (*func)(void*,int,int,double))
{
    /// Do a gradient descent on a grid...
    /// To start with, we will just consider a step size of 1 in one axis:
    ///   Try going in positive x: if gradient is upwards go to negative x
    ///   then do the same with z - if all 4 directions go upwards then we have
    ///   found a minimum, otherwise repeat.
    /// We can remember and try the direction from the previous cycle first to
    /// reduce the number of wrong guesses.
    ///
    /// We can also use a larger step size than 1, as long as we believe that
    /// the minimum is not in between. To determine if this is viable, we check
    /// the step size of 1 first, and then jump if the gradient appears large
    /// enough in that direction.
    ///
    ///TODO:
    /// The perlin noise can be sampled continuously, so more established
    /// minima algorithms can also be considered...

    int dirx = 0, dirz = 0, dira;
    int k, i, j;
    double v, vd, va;
    v = factor * sampleDoublePerlin(para, x+i0, 0, z+j0);
    if (func)
    {
        if (func(data, x+i0, z+j0, factor < 0 ? -v : v))
            return nan("");
    }

    i = i0; j = j0;
    for (k = 0; k < maxiter; k++)
    {
        if (dirx == 0) dirx = +1;
        if (i+dirx >= 0 && i+dirx < w)
            vd = factor * sampleDoublePerlin(para, x+i+dirx, 0, z+j);
        else vd = v;
        if (vd >= v)
        {
            dirx *= -1;
            if (i+dirx >= 0 && i+dirx < w)
                vd = factor * sampleDoublePerlin(para, x+i+dirx, 0, z+j);
            else vd = v;
            if (vd >= v)
                dirx = 0;
        }
        if (dirx)
        {
            dira = (int)(dirx * alpha * (v - vd));
            if (abs(dira) > 2 && i+dira >= 0 && i+dira < w)
            {   // try jumping by more than 1
                va = factor * sampleDoublePerlin(para, x+i+dira, 0, z+j);
                if (va < vd)
                {
                    i += dira;
                    v = va;
                    goto L_x_end;
                }
            }
            v = vd;
            i += dirx;
        L_x_end:
            if (func)
            {
                if (func(data, x+i, z+j, factor < 0 ? -v : v))
                    return nan("");
            }
        }

        if (dirz == 0) dirz = +1;
        if (j+dirz >= 0 && j+dirz < h)
            vd = factor * sampleDoublePerlin(para, x+i, 0, z+j+dirz);
        else vd = v;
        if (vd >= v)
        {
            dirz *= -1;
            if (j+dirz >= 0 && j+dirz < h)
                vd = factor * sampleDoublePerlin(para, x+i, 0, z+j+dirz);
            else vd = v;
            if (vd >= v)
                dirz = 0;
        }
        if (dirz)
        {
            dira = (int)(dirz * alpha * (v - vd));
            if (abs(dira) > 2 && j+dira >= 0 && j+dira < h)
            {   // try jumping by more than 1
                va = factor * sampleDoublePerlin(para, x+i, 0, z+j+dira);
                if (va < vd)
                {
                    j += dira;
                    v = va;
                    goto L_z_end;
                }
            }
            j += dirz;
            v = vd;
        L_z_end:
            if (func)
            {
                if (func(data, x+i, z+j, factor < 0 ? -v : v))
                    return nan("");
            }
        }
        if (dirx == 0 && dirz == 0)
        {   // this is very likely a fix point
            // but there could be a minimum along a diagonal path in rare cases
            int c;
            for (c = 0; c < 4; c++)
            {
                dirx = (c & 1) ? -1 : +1;
                dirz = (c & 2) ? -1 : +1;
                if (i+dirx < 0 || i+dirx >= w || j+dirz < 0 || j+dirz >= h)
                    continue;
                vd = factor * sampleDoublePerlin(para, x+i+dirx, 0, z+j+dirz);
                if (vd < v)
                {
                    v = vd;
                    i += dirx;
                    j += dirz;
                    break;
                }
            }
            if (c >= 4)
                break;
        }
        if (abs(i - i0) > maxrad || abs(j - j0) > maxrad)
            break; // we have gone too far from the origin
    }

    return v;
}


int getParaRange(const DoublePerlinNoise *para, double *pmin, double *pmax,
    int x, int z, int w, int h, void *data, int (*func)(void*,int,int,double))
{
    const double beta = 1.5;
    const double factor = 10000;
    const double perlin_grad = 2.0 * 1.875; // max perlin noise gradient
    double v, lmin, lmax, dr, vdif, small_regime;
    char *skip = NULL;
    int i, j, step, ii, jj, ww, hh, skipsiz;
    int maxrad, maxiter;
    int err = 1;

    *pmin = DBL_MAX;
    *pmax = -DBL_MAX;

    lmin = DBL_MAX, lmax = 0;
    for (i = 0; i < para->octA.octcnt; i++)
    {
        double lac = para->octA.octaves[i].lacunarity;
        if (lac < lmin) lmin = lac;
        if (lac > lmax) lmax = lac;
    }

    // Sort out the small area cases where we are less likely to improve upon
    // checking all positions.
    small_regime = 1e3 * sqrt(lmax);
    if (w*h < small_regime)
    {
        for (j = 0; j < h; j++)
        {
            for (i = 0; i < w; i++)
            {
                v = factor * sampleDoublePerlin(para, x+i, 0, z+j);
                if (func)
                {
                    err = func(data, x+i, z+j, v);
                    if (err)
                        return err;
                }
                if (v < *pmin) *pmin = v;
                if (v > *pmax) *pmax = v;
            }
        }
        return 0;
    }

    // Start with the largest noise period to get some bounds for pmin, pmax
    step = (int) (0.5 / lmin - FLT_EPSILON) + 1;

    dr = lmax / lmin * beta;
    for (j = 0; j < h; j += step)
    {
        for (i = 0; i < w; i += step)
        {
            v = getParaDescent(para, +factor, x, z, w, h, i, j,
                step, step, dr, data, func);
            if (v != v) goto L_end;
            if (v < *pmin) *pmin = v;
            v = -getParaDescent(para, -factor, x, z, w, h, i, j,
                step, step, dr, data, func);
            if (v != v) goto L_end;
            if (v > *pmax) *pmax = v;
        }
    }

    //(*(double*)data) = -1e9+1; // testing
    if (lmin == lmax)
        return 0;

    step = (int) (1.0 / (perlin_grad * lmax + FLT_EPSILON)) + 1;

    /// We can determine the maximum contribution we expect from all noise
    /// periods for a distance of step. If this does not account for the
    /// necessary difference, we can skip that point.
    vdif = 0;
    for (i = 0; i < para->octA.octcnt; i++)
    {
        const PerlinNoise *p = para->octA.octaves + i;
        double contrib = step * p->lacunarity * 1.0;
        if (contrib > 1.0) contrib = 1;
        vdif += contrib * p->amplitude;
    }
    for (i = 0; i < para->octB.octcnt; i++)
    {
        const double lac_factB = 337.0 / 331.0;
        const PerlinNoise *p = para->octB.octaves + i;
        double contrib = step * p->lacunarity * lac_factB;
        if (contrib > 1.0) contrib = 1;
        vdif += contrib * p->amplitude;
    }
    vdif = fabs(factor * vdif * para->amplitude);
    //printf("%g %g %g\n", para->amplitude, 1./lmin, 1./lmax);
    //printf("first pass: [%g %g] diff=%g step:%d\n", *pmin, *pmax, vdif, step);

    maxrad = step;
    maxiter = step*2;
    ww = (w+step-1) / step;
    hh = (h+step-1) / step;
    skipsiz = (ww+1) * (hh+1) * sizeof(*skip);
    skip = (char*) malloc(skipsiz);

    // look for minima
    memset(skip, 0, skipsiz);

    for (jj = 0; jj <= hh; jj++)
    {
        j = jj * step; if (j >= h) j = h-1;
        for (ii = 0; ii <= ww; ii++)
        {
            i = ii * step; if (i >= w) i = w-1;
            if (skip[jj*ww+ii]) continue;

            v = factor * sampleDoublePerlin(para, x+i, 0, z+j);
            if (func)
            {
                int e = func(data, x+i, z+j, v);
                if (e)
                {
                    err = e;
                    goto L_end;
                }
            }
            // not looking for maxima yet, but we'll update the bounds anyway
            if (v > *pmax) *pmax = v;

            dr = beta * (v - *pmin) / vdif;
            if (dr > 1.0)
            {   // difference is too large -> mark visinity to be skipped
                int a, b, r = (int) dr;
                for (b = 0; b < r; b++)
                {
                    if (b+jj < 0 || b+jj >= hh) continue;
                    for (a = -r+1; a < r; a++)
                    {
                        if (a+ii < 0 || a+ii >= ww) continue;
                        skip[(b+jj)*ww + (a+ii)] = 1;
                    }
                }
                continue;
            }
            v = getParaDescent(para, +factor, x, z, w, h, i, j,
                maxrad, maxiter, dr, data, func);
            if (v != v) goto L_end;
            if (v < *pmin) *pmin = v;
        }
    }

    // look for maxima
    memset(skip, 0, skipsiz);

    for (jj = 0; jj <= hh; jj++)
    {
        j = jj * step; if (j >= h) j = h-1;
        for (ii = 0; ii <= ww; ii++)
        {
            i = ii * step; if (i >= w) i = w-1;
            if (skip[jj*ww+ii]) continue;

            v = -factor * sampleDoublePerlin(para, x+i, 0, z+j);
            if (func)
            {
                int e = func(data, x+i, z+j, -v);
                if (e)
                {
                    err = e;
                    goto L_end;
                }
            }

            dr = beta * (v + *pmax) / vdif;
            if (dr > 1.0)
            {   // difference too large -> mark visinity to be skipped
                int a, b, r = (int) dr;
                for (b = 0; b < r; b++)
                {
                    if (b+jj < 0 || b+jj >= hh) continue;
                    for (a = -r+1; a < r; a++)
                    {
                        if (a+ii < 0 || a+ii >= ww) continue;
                        skip[(b+jj)*ww + (a+ii)] = 1;
                    }
                }
                continue;
            }
            v = -getParaDescent(para, -factor, x, z, w, h, i, j,
                maxrad, maxiter, dr, data, func);
            if (v != v) goto L_end;
            if (v > *pmax) *pmax = v;
        }
    }

    err = 0;
L_end:
    if (skip)
        free(skip);
    return err;
}

#define IMIN INT_MIN
#define IMAX INT_MAX
static const int g_biome_para_range_18[][13] = {
/// biome                   temperature  humidity     continental. erosion      depth        weirdness
{ocean                   , -1500, 2000,  IMIN, IMAX, -4550,-1900,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{plains                  , -4500, 5500,  IMIN, 1000, -1900, IMAX,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{desert                  ,  5500, IMAX,  IMIN, IMAX, -1900, IMAX,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{windswept_hills         ,  IMIN, 2000,  IMIN, 1000, -1899, IMAX,  4500, 5500,  IMIN, IMAX,  IMIN, IMAX},
{forest                  , -4500, 5500, -1000, 3000, -1900, IMAX,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{taiga                   ,  IMIN,-1500,  1000, IMAX, -1900, IMAX,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{swamp                   , -4500, IMAX,  IMIN, IMAX, -1100, IMAX,  5500, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{river                   , -4500, IMAX,  IMIN, IMAX, -1900, IMAX,  IMIN, IMAX,  IMIN, IMAX,  -500,  500},
{frozen_ocean            ,  IMIN,-4501,  IMIN, IMAX, -4550,-1900,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{frozen_river            ,  IMIN,-4501,  IMIN, IMAX, -1900, IMAX,  IMIN, IMAX,  IMIN, IMAX,  -500,  500},
{snowy_plains            ,  IMIN,-4500,  IMIN, 1000, -1900, IMAX,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{mushroom_fields         ,  IMIN, IMAX,  IMIN, IMAX, IMIN,-10500,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{beach                   , -4500, 5500,  IMIN, IMAX, -1900,-1100, -2225, IMAX,  IMIN, IMAX,  IMIN, 2666},
{jungle                  ,  2000, 5500,  1000, IMAX, -1900, IMAX,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{sparse_jungle           ,  2000, 5500,  1000, 3000, -1899, IMAX,  IMIN, IMAX,  IMIN, IMAX,  -500, IMAX},
{deep_ocean              , -1500, 2000,  IMIN, IMAX,-10500,-4551,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{stony_shore             ,  IMIN, IMAX,  IMIN, IMAX, -1900,-1100,  IMIN,-2225,  IMIN, IMAX,  IMIN, IMAX},
{snowy_beach             ,  IMIN,-4500,  IMIN, IMAX, -1900,-1100, -2225, IMAX,  IMIN, IMAX,  IMIN, 2666},
{birch_forest            , -1500, 2000,  1000, 3000, -1900, IMAX,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{dark_forest             , -1500, 2000,  3000, IMAX, -1900, IMAX,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{snowy_taiga             ,  IMIN,-4500, -1000, IMAX, -1900, IMAX,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{old_growth_pine_taiga   , -4500,-1500,  3000, IMAX, -1899, IMAX,  IMIN, IMAX,  IMIN, IMAX,  -500, IMAX},
{windswept_forest        ,  IMIN, 2000,  1000, IMAX, -1899, IMAX,  4500, 5500,  IMIN, IMAX,  IMIN, IMAX},
{savanna                 ,  2000, 5500,  IMIN,-1000, -1900, IMAX,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{savanna_plateau         ,  2000, 5500,  IMIN,-1000, -1100, IMAX,  IMIN,  500,  IMIN, IMAX,  IMIN, IMAX},
{badlands                ,  5500, IMAX,  IMIN, 1000, -1899, IMAX,  IMIN,  500,  IMIN, IMAX,  IMIN, IMAX},
{wooded_badlands         ,  5500, IMAX,  1000, IMAX, -1899, IMAX,  IMIN,  500,  IMIN, IMAX,  IMIN, IMAX},
{warm_ocean              ,  5500, IMAX,  IMIN, IMAX,-10500,-1900,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{lukewarm_ocean          ,  2001, 5500,  IMIN, IMAX, -4550,-1900,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{cold_ocean              , -4500,-1501,  IMIN, IMAX, -4550,-1900,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{deep_lukewarm_ocean     ,  2001, 5500,  IMIN, IMAX,-10500,-4551,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{deep_cold_ocean         , -4500,-1501,  IMIN, IMAX,-10500,-4551,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{deep_frozen_ocean       ,  IMIN,-4501,  IMIN, IMAX,-10500,-4551,  IMIN, IMAX,  IMIN, IMAX,  IMIN, IMAX},
{sunflower_plains        , -1500, 2000,  IMIN,-3500, -1899, IMAX,  IMIN, IMAX,  IMIN, IMAX,  -500, IMAX},
{windswept_gravelly_hills,  IMIN,-1500,  IMIN,-1000, -1899, IMAX,  4500, 5500,  IMIN, IMAX,  IMIN, IMAX},
{flower_forest           , -1500, 2000,  IMIN,-3500, -1900, IMAX,  IMIN, IMAX,  IMIN, IMAX,  IMIN, -500},
{ice_spikes              ,  IMIN,-4500,  IMIN,-3500, -1899, IMAX,  IMIN, IMAX,  IMIN, IMAX,  -500, IMAX},
{old_growth_birch_forest , -1500, 2000,  1000, 3000, -1899, IMAX,  IMIN, IMAX,  IMIN, IMAX,  -500, IMAX},
{old_growth_spruce_taiga , -4500,-1500,  3000, IMAX, -1900, IMAX,  IMIN, IMAX,  IMIN, IMAX,  IMIN, -500},
{windswept_savanna       , -1500, IMAX,  IMIN, 3000, -1899,  300,  4500, 5500,  IMIN, IMAX,   501, IMAX},
{eroded_badlands         ,  5500, IMAX,  IMIN,-1000, -1899, IMAX,  IMIN,  500,  IMIN, IMAX,  IMIN, IMAX},
{bamboo_jungle           ,  2000, 5500,  3000, IMAX, -1899, IMAX,  IMIN, IMAX,  IMIN, IMAX,  -500, IMAX},
{dripstone_caves         ,  IMIN, IMAX,  IMIN, 6999,  3001, IMAX,  IMIN, IMAX,  1000, 9500,  IMIN, IMAX},
{lush_caves              ,  IMIN, IMAX,  2001, IMAX,  IMIN, IMAX,  IMIN, IMAX,  1000, 9500,  IMIN, IMAX},
{meadow                  , -4500, 2000,  IMIN, 3000,   300, IMAX, -7799,  500,  IMIN, IMAX,  IMIN, IMAX},
{grove                   ,  IMIN, 2000, -1000, IMAX, -1899, IMAX,  IMIN,-3750,  IMIN, IMAX,  IMIN, IMAX},
{snowy_slopes            ,  IMIN, 2000,  IMIN,-1000, -1899, IMAX,  IMIN,-3750,  IMIN, IMAX,  IMIN, IMAX},
{jagged_peaks            ,  IMIN, 2000,  IMIN, IMAX, -1899, IMAX,  IMIN,-3750,  IMIN, IMAX, -9333,-4001},
{frozen_peaks            ,  IMIN, 2000,  IMIN, IMAX, -1899, IMAX,  IMIN,-3750,  IMIN, IMAX,  4000, 9333},
{stony_peaks             ,  2000, 5500,  IMIN, IMAX, -1899, IMAX,  IMIN,-3750,  IMIN, IMAX, -9333, 9333},
};

/**
 * Gets the min/max possible noise parameter values at which the given biome
 * can generate. The values are in min/max pairs in order:
 * temperature, humidity, continentalness, erosion, depth, weirdness.
 */
const int *getBiomeParaLimits(int mc, int id)
{
    if (mc < MC_1_18)
        return NULL;
    int i, n = sizeof(g_biome_para_range_18) / sizeof(g_biome_para_range_18[0]);
    for (i = 0; i < n; i++)
    {
        if (g_biome_para_range_18[i][0] == id)
            return &g_biome_para_range_18[i][1];
    }
    return NULL;
}

/**
 * Determines which biomes are able to generate given climate parameter limits.
 * Possible biomes are marked non-zero in the 'ids'.
 */
void getPossibleBiomesForLimits(char ids[256], int mc, int limits[6][2])
{
    int i, j, n;
    memset(ids, 0, 256*sizeof(char));

    if (mc >= MC_1_18)
    {
        n = sizeof(g_biome_para_range_18) / sizeof(g_biome_para_range_18[0]);
        for (i = 0; i < n; i++)
        {
            const int *bp = &g_biome_para_range_18[i][1];
            int id = bp[-1];
            for (j = 0; j < 6; j++)
            {
                if (limits[j][0] <= bp[2*j+1] && limits[j][1] >= bp[2*j+0])
                    ids[id]++;
            }
        }
        for (i = 0; i < 256; i++)
            ids[i] = ids[i] >= 6;
    }
}




