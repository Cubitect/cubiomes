#include "finders.h"
#include "util.h"

#include <sys/time.h>
#include <time.h>
#include <float.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <limits.h>

static uint32_t hash32(uint32_t x)
{
    x ^= x >> 15;
    x *= 0xd168aaad;
    x ^= x >> 15;
    x *= 0xaf723597;
    x ^= x >> 15;
    return x;
}

static double now()
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

uint32_t getRef(int mc, int dim, int bits, int scale, int spread, const char *path)
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
            int id = getBiomeAt(&g, scale, x, y, z);
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
        printf("  [%*d/%*d] MC %-6s dim=%-2d: expecting %08x ... ",
               1+(cnt>9), test+1, 1+(cnt>9), cnt, mc2str(mc[test]), dim, expect[test]);
        fflush(stdout);

        double t = -now();
        h = getRef(mc[test], dim, bits, 4, spread, NULL);
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

    SurfaceNoise sn;

    double t = -now();
    uint32_t hash = 0;
    uint64_t s;
    for (s = 0; s < 1000; s++)
    {
        int d = 40000;
        int x = hash32(s << 5) % d - d/2;
        int y = ((int)(hash32(s << 7) % 384) - 64);
        int z = hash32(s << 9) % d - d/2;
        int w = 1 + hash32(s << 11) % 128;
        int h = 1 + hash32(s << 13) % 128;

        applySeed(&g, dim, s);
        Range r = {scale, x, z, w, h, y, 1};
        int *ids = allocCache(&g, r);
        genBiomes(&g, ids, r);
        /*
        float *surf = malloc(4 * w * h);
        initSurfaceNoise(&sn, dim, s);
        mapApproxHeight(surf, 0, &g, &sn, x, z, w, h);
        for (int i = 0; i < w*h; i++)
            ids[i] = (int) surf[i];
        free(surf);
        */

        int i = 0;
        hash = 0;
        for (i = 0; i < w*h; i++)
            hash = hash32(hash ^ hash32(ids[i] + (i << 17)));
        free(ids);
    }
    t += now();
    printf("  MC %-6s dim %-2d @ 1:%-3d - %08x [%ld msec]\n",
        mc2str(mc), dim, scale, hash, (long)(t*1e3));
    return hash;
}




int testGeneration()
{
    const int mc_vers[] = {
        MC_1_20, MC_1_19, MC_1_19_2, MC_1_18,
        MC_1_16, MC_1_15,   MC_1_13, MC_1_12, MC_1_9,  MC_1_7,
        MC_1_6,  MC_1_2,    MC_1_1,  MC_1_0,  MC_B1_8,
    };
    const uint32_t b6_hashes[] = {
        0x0f8888ab, 0x391c36ec, 0xea3e8c1c, 0xade7f891,
        0xde9a6574, 0x3a568a6d, 0x96c97323, 0xbc75e996, 0xe27a45a2, 0xbc75e996,
        0x15b47206, 0x2d7e0fed, 0x5cbf4709, 0xbd794adb, 0x00000000,
    };
    const int testcnt = sizeof(mc_vers) / sizeof(int);

    printf("Testing 1x1 biome generation (quick):\n");
    if (!testBiomeGen1x1(mc_vers, b6_hashes, 0, 6, 1, testcnt))
        return -1;

    //Generator g;
    //setupGenerator(&g, MC_1_18, 0);
    //applySeed(&g, 0, 1234);
    //Pos p = getSpawn(&g);
    //printf("%d %d\n", p.x, p.z);

    //StrongholdIter sh;
    //initFirstStronghold(&sh, g.mc, g.seed);
    //while (nextStronghold(&sh, &g) > 0)
    //    printf("Stronghold #: (%6d, %6d)\n", sh.pos.x, sh.pos.z);

    //printf("Area generation tests:\n");
    //testAreas(MC_1_19, 0, 4);
    //testAreas(MC_1_18, 0, 4);
    //testAreas(MC_1_17, 0, 4);

    const uint32_t b10_hashes[] = {
        0x00000000, 0x00000000, 0x00000000,
        0xfdede71d, 0xca8005d7, 0x399f7cc8, 0xb3363967, 0x17e5592f, 0xb3363967,
        0xa52e377c, 0xdb1df71d, 0x58e86947, 0xe1e89cc3, 0x00000000,
    };
    printf("Testing 1x1 biome generation (thorough):\n");
    if (!testBiomeGen1x1(mc_vers, b10_hashes, 0, 10, 1, testcnt))
        return -1;
    return 0;
}


int k_tot;
struct _f_para { double v; double *buf; int x, z, w, h; };
int _f1(void *data, int x, int z, double v)
{
    struct _f_para d = *(struct _f_para*) data;
    d.buf[(x-d.x)*d.w + (z-d.z)] = d.v;
    k_tot++;
    return 0;
}

void testNoiseRangeFinder()
{
    int n = 100;
    unsigned char *pix = (unsigned char*) malloc(n*n*3);
    double *buf = (double*) malloc(n*n*8);
    double bmin, bmax;
    int i, j;

    int64_t seed = 0, seed_max = 200;
    int bad = 0;
    for (seed = 0; seed < seed_max; seed++)
    {
    //printf("%ld\n", seed);
    Generator g;
    setupGenerator(&g, MC_1_18, 0);
    applySeed(&g, 0, seed);

    int x = 100, z = -100;
    bmin = 99999, bmax = -99999;
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            double t = sampleDoublePerlin(&g.bn.climate[NP_HUMIDITY], x+i, 0, z+j);
            if (t < bmin) bmin = t;
            if (t > bmax) bmax = t;
            buf[i*n+j] = t;
        }
    }

    struct _f_para f_p = {-1e9, buf, x, z, n, n };
    double tmin, tmax;
    int k = k_tot;
    getParaRange(&g.bn.climate[NP_HUMIDITY], &tmin, &tmax, x, z, n, n, &f_p, _f1);
    if (fabs(tmin-bmin*1e4)>.01||fabs(tmax-bmax*1e4)>.01)
    {
        printf("=========================== BAD ============================\n");
        printf("seed:%-8ld temp = [%g %g], best = [%g %g]\n",
            seed, tmin, tmax, bmin * 10000, bmax * 10000);
        bad++;
    }
    //printf("wh:%d (%d) -> k:%d\n", 2*n, n*n, k_tot - k);

    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            unsigned char *p = pix + 3*(i*n+j);
            double b = buf[i*n+j];
            if (b == bmin || b == bmax) {
                p[1] = 0xff; p[0] = p[2] = 0;
                continue;
            }
            if (b == -1e9) {
                p[0] = 0xff; p[1] = p[2] = 0;
                continue;
            }
            if (b == -1e9+1) {
                p[2] = 0xff; p[0] = p[1] = 0;
                continue;
            }
            p[0] = p[1] = p[2] = (unsigned char)
                ((b - bmin) / (bmax - bmin) * 256);
        }
    }
    if (bad >= 10) break;
    }

    printf("bad:%d k_tot: %d / %ld ~ %g : %d\n", bad, k_tot, seed, k_tot / (double)seed, n*n);
    savePPM("img.ppm", pix, n, n);
}


int64_t bbounds[256][6][2]; // [biome][np][min/max]

int _f2(void *data, int x, int z, double v)
{
    int64_t np[6];
    Generator *g = (Generator*) data;
    int id = sampleBiomeNoise(&g->bn, np, x, -64+rand()%384, z, 0, SAMPLE_NO_SHIFT);
    int i;
    for (i = 0; i < 6; i++)
    {
        if (np[i] < bbounds[id][i][0]) bbounds[id][i][0] = np[i];
        if (np[i] > bbounds[id][i][1]) bbounds[id][i][1] = np[i];
    }
    return 0;
}

void findBiomeParaBounds()
{
    int i, j;
    for (i = 0; i < 256; i++)
    {
        for (j = 0; j < 6; j++)
        {
            bbounds[i][j][0] = +1e8;
            bbounds[i][j][1] = -1e8;
        }
    }

    Generator g;
    setupGenerator(&g, MC_1_21, 0);
    int64_t s;
    int r = 1000;
    for (s = 0; s < 20000; s++)
    {
        int64_t seed = ((int64_t)hash32(s) << 32) ^ hash32(rand());
        applySeed(&g, 0, seed);
        double tmin, tmax;
        int x = rand() % 10000 - 5000;
        int z = rand() % 10000 - 5000;
        getParaRange(&g.bn.climate[NP_TEMPERATURE],     &tmin, &tmax, x-r, z-r, r, r, &g, _f2);
        getParaRange(&g.bn.climate[NP_HUMIDITY],        &tmin, &tmax, x-r, z-r, r, r, &g, _f2);
        getParaRange(&g.bn.climate[NP_EROSION],         &tmin, &tmax, x-r, z-r, r, r, &g, _f2);
        getParaRange(&g.bn.climate[NP_CONTINENTALNESS], &tmin, &tmax, x-r, z-r, r, r, &g, _f2);
        getParaRange(&g.bn.climate[NP_WEIRDNESS],       &tmin, &tmax, x-r, z-r, r, r, &g, _f2);
        if (s % 1000 == 999)
            printf(".\n");
    }

    for (i = 0; i < 256; i++)
    {
        if (!isOverworld(MC_1_21, i))
            continue;

        printf("{%-24s", biome2str(MC_1_21, i));
        for (j = 0; j < 6; j++)
        {
            printf(", %6ld,%6ld", bbounds[i][j][0], bbounds[i][j][1]);
        }
        printf("},\n");
    }
}

static void canGenerateTest(int mc, int layerId)
{
    Generator g;
    setupGenerator(&g, mc, 0);
    int ids[0x1000];
    int idcnt[256] = {};
    int i;
    uint64_t seed;
    Layer *layer = g.ls.layers + layerId;

    for (seed = 0; seed < 1e6; seed++)
    {
        applySeed(&g, DIM_OVERWORLD, seed);
        genArea(layer, ids, 0, 0, 1, 1);
        int id = ids[0];
        idcnt[id]++;
    }
    int ok = 1;
    for (i = 0; i < 256; i++)
    {
        int cnt = idcnt[i];
        int can = canBiomeGenerate(layerId, mc, 0, i);
        if (cnt == 0 && can == 0)
            continue;
        if (cnt != 0 && can == 1)
            continue;
        ok = 0;
        printf("can:%d, cnt:%d (%s)\n", can, cnt, biome2str(mc, i));
    }
    printf("canBiomesGenerate() for MC %-4s, layer (%d) %s!\n",
        mc2str(mc), layerId, ok ? "PASSED" : "FAILED");
}

void testCanBiomesGenerate()
{
    canGenerateTest(MC_B1_8, L_BIOME_256);
    canGenerateTest(MC_B1_8, L_ZOOM_64);
    canGenerateTest(MC_B1_8, L_ZOOM_16);
    canGenerateTest(MC_B1_8, L_RIVER_MIX_4);
    canGenerateTest(MC_1_0, L_BIOME_256);
    canGenerateTest(MC_1_0, L_ZOOM_64);
    canGenerateTest(MC_1_0, L_ZOOM_16);
    canGenerateTest(MC_1_0, L_RIVER_MIX_4);
    canGenerateTest(MC_1_6, L_BIOME_256);
    canGenerateTest(MC_1_6, L_HILLS_64);
    canGenerateTest(MC_1_6, L_SWAMP_RIVER_16);
    canGenerateTest(MC_1_6, L_RIVER_MIX_4);
    canGenerateTest(MC_1_7, L_BIOME_256);
    canGenerateTest(MC_1_7, L_SUNFLOWER_64);
    canGenerateTest(MC_1_7, L_SHORE_16);
    canGenerateTest(MC_1_7, L_RIVER_MIX_4);
    canGenerateTest(MC_1_17, L_BAMBOO_256);
    canGenerateTest(MC_1_17, L_SUNFLOWER_64);
    canGenerateTest(MC_1_17, L_SHORE_16);
    canGenerateTest(MC_1_17, L_OCEAN_MIX_4);
    canGenerateTest(MC_1_17, L_OCEAN_TEMP_256);
}


void findStructures(int structureType, int mc, int dim, uint64_t seed,
    int x0, int z0, int x1, int z1)
{
    // set up a biome generator
    Generator g;
    setupGenerator(&g, mc, 0);
    applySeed(&g, dim, seed);

    // ignore this if you are not looking for end cities
    SurfaceNoise sn;
    if (structureType == End_City)
        initSurfaceNoise(&sn, DIM_END, seed);

    StructureConfig sconf;
    if (!getStructureConfig(structureType, mc, &sconf))
        return; // bad version or structure

    // segment area into structure regions
    double blocksPerRegion = sconf.regionSize * 16.0;
    int rx0 = (int) floor(x0 / blocksPerRegion);
    int rz0 = (int) floor(z0 / blocksPerRegion);
    int rx1 = (int) ceil(x1 / blocksPerRegion);
    int rz1 = (int) ceil(z1 / blocksPerRegion);
    int i, j;

    for (j = rz0; j <= rz1; j++)
    {
        for (i = rx0; i <= rx1; i++)
        {   // check the structure generation attempt in region (i, j)
            Pos pos;
            if (!getStructurePos(structureType, mc, seed, i, j, &pos))
                continue; // this region is not suitable
            if (pos.x < x0 || pos.x > x1 || pos.z < z0 || pos.z > z1)
                continue; // structure is outside the specified area
            if (!isViableStructurePos(structureType, &g, pos.x, pos.z, 0))
                continue; // biomes are not viable
            if (structureType == End_City)
            {   // end cities have a dedicated terrain checker
                if (!isViableEndCityTerrain(&g, &sn, pos.x, pos.z))
                    continue;
            }
            else if (mc >= MC_1_18)
            {   // some structures in 1.18+ depend on the terrain
                if (!isViableStructureTerrain(structureType, &g, pos.x, pos.z))
                    continue;
            }

            int id = getBiomeAt(&g, 4, pos.x>>2, 320>>2, pos.z>>2);
            StructureVariant sv;
            getVariant(&sv, structureType, mc, seed, pos.x, pos.z, id);
            int x = pos.x + sv.x;
            int z = pos.z + sv.z;
            printf("%d, %d : [%d %d %d] - [%d %d %d]\n", pos.x, pos.z,
                x, sv.y, z, x+sv.sx, sv.y+sv.sy, z+sv.sz
                );
        }
    }
}

int getStructureConfig_override(int stype, int mc, StructureConfig *sconf)
{
    return getStructureConfig(stype, mc, sconf);
}



int main()
{
    /*
    int mc = MC_1_21;
    uint64_t seed = 2;

    double t0 = 0, t1 = 0;
    t0 -= now();

    EndNoise en;
    setEndSeed(&en, mc, seed);

    SurfaceNoise sn;
    initSurfaceNoise(&sn, DIM_END, seed);

    Pos src[20];
    getFixedEndGateways(mc, seed, src);

    t0 += now();
    t1 -= now();

    for (int i = 0; i < 20; i++)
    {
        Pos dst = getLinkedGatewayPos(&en, &sn, seed, src[i]);
        printf("%d %d -> %d %d\n", src[i].x, src[i].z, dst.x, dst.z);
    }
    t1 += now();

    printf("Time: %g -> %g sec\n", t0, t1);
    */

    //findStructures(Trial_Chambers, MC_1_21, 0, 1, 3056, 3440, 3056, 3440);

    //endHeight(MC_1_21, 1, 80>>1, 1216>>1, 32, 32, 2);

    //endHeight(MC_1_15, 1, 370704, 96, 32, 32, 1);

    //testAreas(MC_1_21, 1, 1);
    //testAreas(MC_1_21, 0, 4);
    //testAreas(mc, 0, 16);
    //testAreas(mc, 0, 256);
    //testCanBiomesGenerate();
    //testGeneration();
    //findBiomeParaBounds();

    return 0;
}




