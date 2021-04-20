#include "finders.h"
#include "util.h"

uint32_t hash32(uint32_t x)
{
    x ^= x >> 15;
    x *= 0xd168aaad;
    x ^= x >> 15;
    x *= 0xaf723597;
    x ^= x >> 15;
    return x;
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

int main()
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



