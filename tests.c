#include "finders.h"
#include "util.h"

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

uint32_t getRef(int mc, int bits, const char *path)
{
    initBiomes();
    LayerStack g;
    setupGenerator(&g, mc);
    Layer *l = g.entry_4;
    int *ids = allocCache(l, 1, 1);

    FILE *fp = NULL;
    if (path)
        fp = fopen(path, "w");

    int r = 1 << (bits-1);
    int h = 0;
    int x, z;
    for (x = -r; x < r; x++)
    {
        for (z = -r; z < r; z++)
        {
            int64_t s = (int64_t)( (z << bits) ^ x );
            setLayerSeed(l, s);
            //applySeed(&g, s);
            genArea(l, ids, x, z, 1, 1);
            h ^= hash32( (int) s ^ (ids[0] << 2*bits) );

            if (fp)
                fprintf(fp, "%5d%6d%4d\n", x, z, ids[0]);
        }
    }
    if (fp)
        fclose(fp);
    free(ids);
    return h;
}

int testBiomeGen1x1(const int *mc, const uint32_t *expect, int bits, int cnt)
{
    int test;
    uint32_t h;
    int ok = 1;

    for (test = 0; test < cnt; test++)
    {
        printf("  [%d/%d] MC 1.%-2d: expecting %08x ... ",
            test+1, cnt, mc[test], expect[test]);
        fflush(stdout);

        h = getRef(mc[test], bits, NULL);
        printf("got %08x %s\e[0m\n", 
            h, h == expect[test] ? "\e[1;92mOK" : "\e[1;91mFAILED");
        ok &= (h == expect[test]);
    }

    return ok;
}

int testOverworldBiomes()
{
    const int mc_vers[] = {
        MC_1_16, MC_1_15, MC_1_13, MC_1_12, MC_1_9, MC_1_7,
        MC_1_6,
    };
    const uint32_t b6_hashes[] = {
        0xde9a6574, 0x3a568a6d, 0x96c97323, 0xbc75e996, 0xe27a45a2, 0xbc75e996,
        0x15b47206,
    };
    const uint32_t b10_hashes[] = {
        0xfdede71d, 0xca8005d7, 0x399f7cc8, 0xb3363967, 0x17e5592f, 0xb3363967,
        0xa52e377c,
    };
    const int testcnt = sizeof(mc_vers) / sizeof(int);

    printf("Testing 1x1 biome generation (quick):\n");
    if (!testBiomeGen1x1(mc_vers, b6_hashes, 6, testcnt))
        return -1;

    printf("Testing 1x1 biome generation (thorough):\n");
    if (!testBiomeGen1x1(mc_vers, b10_hashes, 10, testcnt))
        return -1;

    return 0;
}


int gw = 16, gh = 16;

int64_t testPerfEnd(int64_t n, void *data)
{
    EndNoise *en = (EndNoise*) data;
    int ids[gw*gh];
    int64_t r = 0;

    while (n-->0)
    {
        int x = rand() % 10000 - 5000;
        int z = rand() % 10000 - 5000;
        mapEndBiome(en, ids, x, z, gw, gh);
        r ^= ids[0];
    }
    return r;
}

int64_t testPerfNether(int64_t n, void *data)
{
    NetherNoise *nn = (NetherNoise*) data;
    int ids[gw*gh];
    int64_t r = 0;

    while (n-->0)
    {
        int x = rand() % 10000 - 5000;
        int z = rand() % 10000 - 5000;
        mapNether2D(nn, ids, x, z, gw, gh);
        r ^= ids[0];
    }
    return r;
}

int testPerformance()
{
    double tmin, tavg;

    EndNoise en;
    setEndSeed(&en, 12345);
    benchmark(testPerfEnd, &en, &tmin, &tavg);
    printf("End %dx%d    -> min:%10.0lf ns | avg:%10.0lf ns | conf:%4.2lf %%\n",
        gw, gh, 1e9*tmin, 1e9*tavg, 100 * (tavg-tmin) / (tavg+tmin));

    NetherNoise nn;
    setNetherSeed(&nn, 12345);
    benchmark(testPerfNether, &nn, &tmin, &tavg);
    printf("Nether %dx%d -> min:%10.0lf ns | avg:%10.0lf ns | conf:%4.2lf %%\n",
        gw, gh, 1e9*tmin, 1e9*tavg, 100 * (tavg-tmin) / (tavg+tmin));
}


int main()
{
    //testOverworldBiomes();
    testPerformance();
    return 0;
}



