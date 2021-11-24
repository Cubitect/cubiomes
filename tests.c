#include "finders.h"
#include "util.h"

#include <sys/time.h>
#include <time.h>
#include <float.h>
#include <stdlib.h>

uint32_t hash32(uint32_t x)
{
    x ^= x >> 15;
    x *= 0xd168aaad;
    x ^= x >> 15;
    x *= 0xaf723597;
    x ^= x >> 15;
    return x;
}

double now()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

/* Runs a performance test using function f(). The function should take a
 * number of trial runs and a data argument [dat], and return an arbitrary
 * consumable value (useful to avoid ommission by optimization).
 * The minimum and mean times for a trial run will be stored in tmin and tavg.
 * The benchmark aims to take just over one second.
 * Returned is the total number of trials performed.
 */
int64_t
benchmark(int64_t (*f)(int64_t n, void*), void *dat, double *tmin, double *tavg)
{
    const double maxt = 1.0;
    const double mintt = 1e-2;
    int64_t cnt = 0;
    int64_t ntt = 1;
    int64_t consume = 0;
    double t, _tavg = 0, _tmin = DBL_MAX;

    for (ntt = 1; ; ntt *= 2)
    {
        t = -now();
        consume ^= f(ntt, dat);
        t += now();
        if (t >= mintt)
            break;
    }
    do
    {
        t = -now();
        consume ^= f(ntt, dat);
        t += now();
        cnt++;
        _tavg += t;
        if (t < _tmin)
            _tmin = t;
    }
    while (_tavg < maxt);

    cnt *= ntt;
    if (tmin) *tmin = _tmin / ntt;
    if (tavg) *tavg = _tavg / cnt;

    static volatile int64_t v_consume;
    v_consume ^= consume;
    return cnt;
}

uint32_t getRef(int mc, int dim, int bits, int spread, const char *path)
{
    Generator g;
    setupGenerator(&g, mc, 0);

    FILE *fp = NULL;
    if (path) fp = fopen(path, "w");

    int r = 1 << (bits-1);
    int h = 0;
    int x, z;
    for (x = -r; x < r; x++)
    {
        for (z = -r; z < r; z++)
        {
            int64_t s = (int64_t)( (z << bits) ^ x );
            applySeed(&g, dim, s);
            int y = (int)((hash32((int)s) & 0x7fffffff) % 384 - 64) >> 2;
            int id = getBiomeAt(&g, 4, x, y, z);
            h ^= hash32( (int) s ^ (id << 2*bits) );
            if (fp)
                //fprintf(fp, "%5d%6d%4d\n", x*spread, z*spread, id);
                fprintf(fp, "%2ld @ %2d %4d %2d - %4d %08x\n", s, x, y, z, id, h);
        }
    }
    if (fp && fp != stdout)
        fclose(fp);
    return h;
}

int testBiomeGen1x1(const int *mc, const uint32_t *expect, int dim, int bits, int spread, int cnt)
{
    int test;
    uint32_t h;
    int ok = 1;

    for (test = 0; test < cnt; test++)
    {
        printf("  [%*d/%*d] MC 1.%-2d dim=%-2d: expecting %08x ... ",
               1+(cnt>9), test+1, 1+(cnt>9), cnt, mc[test], dim, expect[test]);
        fflush(stdout);

        double t = -now();
        h = getRef(mc[test], dim, bits, spread, NULL);
        t += now();
        printf("got %08x %s\e[0m (%ld msec)\n",
            h, h == expect[test] ? "\e[1;92mOK" : "\e[1;91mFAILED",
            (long)(t*1e3));
        ok &= (h == expect[test]);
    }

    return ok;
}


uint32_t testAreas(int mc, int dim, int scale)
{
    Generator g;
    setupGenerator(&g, mc, 0);

    double t = -now();
    uint32_t hash = 0;
    uint64_t s;
    for (s = 0; s < 100; s++)
    {
        int d = 10000;
        int x = hash32(s << 5) % d - d/2;
        int y = ((int)(hash32(s << 7) % 384) - 64);
        int z = hash32(s << 9) % d - d/2;
        int w = 1 + hash32(s << 11) % 128; w = 128;
        int h = 1 + hash32(s << 13) % 128; h = 128;

        applySeed(&g, dim, s);
        Range r = {scale, x, z, w, h, y, 1};
        int *ids = allocCache(&g, r);
        genBiomes(&g, ids, r);
        int i = 0;
        hash = 0;
        for (i = 0; i < w*h; i++)
            hash = hash32(hash ^ hash32(ids[i] + (i << 17)));
        free(ids);
    }
    t += now();
    printf("  MC 1.%-2d dim %-2d @ 1:%-3d - %08x [%ld msec]\n",
        mc, dim, scale, hash, (long)(t*1e3));
    return hash;
}




int testGeneration()
{
    const int mc_vers[] = {
        MC_1_18,
        MC_1_16, MC_1_15, MC_1_13, MC_1_12, MC_1_9, MC_1_7,
        MC_1_6,  MC_1_2,  MC_1_1,  MC_1_0,
    };
    const uint32_t b6_hashes[] = {
        0xade7f891,
        0xde9a6574, 0x3a568a6d, 0x96c97323, 0xbc75e996, 0xe27a45a2, 0xbc75e996,
        0x15b47206, 0x2d7e0fed, 0x5cbf4709, 0xbd794adb,
    };
    const int testcnt = sizeof(mc_vers) / sizeof(int);

    //printf("Testing 1x1 biome generation (quick):\n");
    //if (!testBiomeGen1x1(mc_vers, b6_hashes, 0, 6, 1, testcnt))
    //    return -1;

    Generator g;
    setupGenerator(&g, MC_1_18, 0);
    applySeed(&g, 0, 1234);
    Pos p = getSpawn(&g);
    printf("%d %d\n", p.x, p.z);

    StrongholdIter sh;
    initFirstStronghold(&sh, g.mc, g.seed);
    while (nextStronghold(&sh, &g) > 0)
        printf("Stronghold #: (%6d, %6d)\n", sh.pos.x, sh.pos.z);

    printf("Area generation tests:\n");
    testAreas(MC_1_18, 0, 1);
    testAreas(MC_1_18, 0, 4);
    testAreas(MC_1_18, 0, 16);
    testAreas(MC_1_18, 0, 64);

    //const uint32_t b10_hashes[] = {
    //    0x00000000,
    //    0xfdede71d, 0xca8005d7, 0x399f7cc8, 0xb3363967, 0x17e5592f, 0xb3363967,
    //    0xa52e377c, 0xdb1df71d, 0x58e86947, 0xe1e89cc3,
    //};
    //printf("Testing 1x1 biome generation (thorough):\n");
    //if (!testBiomeGen1x1(mc_vers, b10_hashes, 0, 10, 1, testcnt))
    //    return -1;
    return 0;
}


int main()
{
    testGeneration();

    return 0;
}



