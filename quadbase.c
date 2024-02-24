#include "quadbase.h"
#include "util.h"

#include <string.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>


#if defined(_WIN32)

#include <windows.h>
typedef HANDLE thread_id_t;
#include <direct.h>
#define IS_DIR_SEP(C)   ((C) == '/' || (C) == '\\')
#define stat            _stat
#define mkdir(P,X)      _mkdir(P)
#define S_IFDIR         _S_IFDIR

#ifndef _S_ISTYPE
#define _S_ISTYPE(mode, mask)  (((mode) & _S_IFMT) == (mask))
#endif

#ifndef S_ISDIR
#define S_ISDIR(mode) _S_ISTYPE((mode), _S_IFDIR)
#endif

#else

#define USE_PTHREAD
#include <pthread.h>
typedef pthread_t       thread_id_t;
#define IS_DIR_SEP(C)   ((C) == '/')

#endif



//==============================================================================
// Multi-Structure Checks
//==============================================================================

int getQuadHutCst(uint64_t low20)
{
    const uint64_t *cst;
    for (cst = low20QuadIdeal; *cst; cst++)
        if (*cst == low20)
            return CST_IDEAL;
    for (cst = low20QuadClassic; *cst; cst++)
        if (*cst == low20)
            return CST_CLASSIC;
    for (cst = low20QuadHutNormal; *cst; cst++)
        if (*cst == low20)
            return CST_NORMAL;
    for (cst = low20QuadHutBarely; *cst; cst++)
        if (*cst == low20)
            return CST_BARELY;
    return CST_NONE;
}

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
    int lowBitN;
    char skipStart;

    // testing function
    int (*check)(uint64_t, void*);
    void *data;

    // abort check
    volatile char *stop;

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
            err = mkdir(path, 0755);
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
        int idx, cnt;

        for (cnt = 0; info->lowBits[cnt]; cnt++);

        mid = info->start & hmask;
        for (idx = 0; (seed = mid | info->lowBits[idx]) < info->start; idx++);

        while (seed <= end)
        {
            if unlikely(info->check(seed, info->data))
            {
                if (seed == info->start && info->skipStart) {} // skip
                else if (info->fp)
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
            if (idx >= cnt)
            {
                idx = 0;
                mid += hstep;
                if (info->stop && *info->stop)
                    break;
            }

            seed = mid | info->lowBits[idx];
        }
    }
    else
    {
        while (seed <= end)
        {
            if unlikely(info->check(seed, info->data))
            {
                if (seed == info->start && info->skipStart) {} // skip
                else if (info->fp)
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
            if ((seed & 0xfff) == 0 && info->stop && *info->stop)
                break;
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
        int                 lowBitN,
        int (*check)(uint64_t s48, void *data),
        void *              data,
        volatile char *     stop
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
        info[t].lowBitN = lowBitN;
        info[t].skipStart = 0;
        info[t].check = check;
        info[t].data = data;
        info[t].stop = stop;

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
                    info[t].skipStart = 1;
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

    if (stop && *stop)
        goto L_err;

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
        const uint64_t *lowBits, int lowBitN, uint64_t salt,
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

    for (i = 0; lowBits[i]; i++)
    {
        cnt += scanForQuadBits(sconf, radius, s48, lowBits[i]-salt, lowBitN, invB,
                x, z, w, h, qplist+cnt, n-cnt);
        if (cnt >= n)
            break;
    }

    return cnt;
}


