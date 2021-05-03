#include "layers.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

//==============================================================================
// Essentials
//==============================================================================

int biomeExists(int mc, int id)
{
    switch (id)
    {
    case ocean...mountain_edge:
        return 1;
    case jungle...jungle_hills:
        return mc >= MC_1_2;
    case jungle_edge...badlands_plateau:
        return mc >= MC_1_7;
    case small_end_islands...end_barrens:
        return mc >= MC_1_9;
    case warm_ocean...deep_frozen_ocean:
        return mc >= MC_1_13;
    case the_void:
        return mc >= MC_1_9;
    case sunflower_plains:
    case desert_lakes:
    case gravelly_mountains:
    case flower_forest:
    case taiga_mountains:
    case swamp_hills:
    case ice_spikes:
    case modified_jungle:
    case modified_jungle_edge:
    case tall_birch_forest:
    case tall_birch_hills:
    case dark_forest_hills:
    case snowy_taiga_mountains:
    case giant_spruce_taiga:
    case giant_spruce_taiga_hills:
    case modified_gravelly_mountains:
    case shattered_savanna:
    case shattered_savanna_plateau:
    case eroded_badlands:
    case modified_wooded_badlands_plateau:
    case modified_badlands_plateau:
        return mc >= MC_1_7;
    case bamboo_jungle:
    case bamboo_jungle_hills:
        return mc >= MC_1_14;
    case soul_sand_valley...basalt_deltas:
        return mc >= MC_1_16;
    default:
        return 0;
    }
}

int isOverworld(int mc, int id)
{
    if (!biomeExists(mc, id))
        return 0;

    switch (id)
    {
    case nether_wastes:
    case the_end:
        return 0;
    case frozen_ocean:
        return mc <= MC_1_6 || mc >= MC_1_13;
    case mountain_edge:
        return mc <= MC_1_6;
    case small_end_islands...end_barrens:
    case deep_warm_ocean:
    case the_void:
        return 0;
    case tall_birch_hills:
        return mc <= MC_1_8 || mc >= MC_1_11;
    case soul_sand_valley...basalt_deltas:
        return 0;
    }
    return 1;
}

int getMutated(int mc, int id)
{
    switch (id)
    {
    case plains:                    return sunflower_plains;
    case desert:                    return desert_lakes;
    case mountains:                 return gravelly_mountains;
    case forest:                    return flower_forest;
    case taiga:                     return taiga_mountains;
    case swamp:                     return swamp_hills;
    case snowy_tundra:              return ice_spikes;
    case jungle:                    return modified_jungle;
    case jungle_edge:               return modified_jungle_edge;
    // emulate MC-98995
    case birch_forest:
        return (mc >= MC_1_9 && mc <= MC_1_10) ? tall_birch_hills : tall_birch_forest;
    case birch_forest_hills:
        return (mc >= MC_1_9 && mc <= MC_1_10) ? none : tall_birch_hills;
    case dark_forest:               return dark_forest_hills;
    case snowy_taiga:               return snowy_taiga_mountains;
    case giant_tree_taiga:          return giant_spruce_taiga;
    case giant_tree_taiga_hills:    return giant_spruce_taiga_hills;
    case wooded_mountains:          return modified_gravelly_mountains;
    case savanna:                   return shattered_savanna;
    case savanna_plateau:           return shattered_savanna_plateau;
    case badlands:                  return eroded_badlands;
    case wooded_badlands_plateau:   return modified_wooded_badlands_plateau;
    case badlands_plateau:          return modified_badlands_plateau;
    default:
        return none;
    }
}

int getCategory(int mc, int id)
{
    switch (id)
    {
    case beach:
    case snowy_beach:
        return beach;

    case desert:
    case desert_hills:
    case desert_lakes:
        return desert;

    case mountains:
    case mountain_edge:
    case wooded_mountains:
    case gravelly_mountains:
    case modified_gravelly_mountains:
        return mountains;

    case forest:
    case wooded_hills:
    case birch_forest:
    case birch_forest_hills:
    case dark_forest:
    case flower_forest:
    case tall_birch_forest:
    case tall_birch_hills:
    case dark_forest_hills:
        return forest;

    case snowy_tundra:
    case snowy_mountains:
    case ice_spikes:
        return snowy_tundra;

    case jungle:
    case jungle_hills:
    case jungle_edge:
    case modified_jungle:
    case modified_jungle_edge:
    case bamboo_jungle:
    case bamboo_jungle_hills:
        return jungle;

    case badlands:
    case eroded_badlands:
    case modified_wooded_badlands_plateau:
    case modified_badlands_plateau:
        return mesa;

    case wooded_badlands_plateau:
    case badlands_plateau:
        return mc <= MC_1_15 ? mesa : badlands_plateau;

    case mushroom_fields:
    case mushroom_field_shore:
        return mushroom_fields;

    case stone_shore:
        return stone_shore;

    case ocean:
    case frozen_ocean:
    case deep_ocean:
    case warm_ocean:
    case lukewarm_ocean:
    case cold_ocean:
    case deep_warm_ocean:
    case deep_lukewarm_ocean:
    case deep_cold_ocean:
    case deep_frozen_ocean:
        return ocean;

    case plains:
    case sunflower_plains:
        return plains;

    case river:
    case frozen_river:
        return river;

    case savanna:
    case savanna_plateau:
    case shattered_savanna:
    case shattered_savanna_plateau:
        return savanna;

    case swamp:
    case swamp_hills:
        return swamp;

    case taiga:
    case taiga_hills:
    case snowy_taiga:
    case snowy_taiga_hills:
    case giant_tree_taiga:
    case giant_tree_taiga_hills:
    case taiga_mountains:
    case snowy_taiga_mountains:
    case giant_spruce_taiga:
    case giant_spruce_taiga_hills:
        return taiga;

    default:
        return none;
    }
}

int areSimilar(int mc, int id1, int id2)
{
    if (id1 == id2) return 1;

    if (mc <= MC_1_15)
    {
        if (id1 == wooded_badlands_plateau || id1 == badlands_plateau)
            return id2 == wooded_badlands_plateau || id2 == badlands_plateau;
    }

    return getCategory(mc, id1) == getCategory(mc, id2);
}

int isMesa(int id)
{
    switch (id)
    {
    case badlands:
    case eroded_badlands:
    case modified_wooded_badlands_plateau:
    case modified_badlands_plateau:
    case wooded_badlands_plateau:
    case badlands_plateau:
        return 1;
    default:
        return 0;
    }
}

int isShallowOcean(int id)
{
    const uint64_t shallow_bits =
            (1ULL << ocean) |
            (1ULL << frozen_ocean) |
            (1ULL << warm_ocean) |
            (1ULL << lukewarm_ocean) |
            (1ULL << cold_ocean);
    return id < 64 && ((1ULL << id) & shallow_bits);
}

int isDeepOcean(int id)
{
    const uint64_t deep_bits =
            (1ULL << deep_ocean) |
            (1ULL << deep_warm_ocean) |
            (1ULL << deep_lukewarm_ocean) |
            (1ULL << deep_cold_ocean) |
            (1ULL << deep_frozen_ocean);
    return id < 64 && ((1ULL << id) & deep_bits);
}

int isOceanic(int id)
{
    const uint64_t ocean_bits =
            (1ULL << ocean) |
            (1ULL << frozen_ocean) |
            (1ULL << warm_ocean) |
            (1ULL << lukewarm_ocean) |
            (1ULL << cold_ocean) |
            (1ULL << deep_ocean) |
            (1ULL << deep_warm_ocean) |
            (1ULL << deep_lukewarm_ocean) |
            (1ULL << deep_cold_ocean) |
            (1ULL << deep_frozen_ocean);
    return id < 64 && ((1ULL << id) & ocean_bits);
}

int isSnowy(int id)
{
    switch (id)
    {
    case frozen_ocean:
    case frozen_river:
    case snowy_tundra:
    case snowy_mountains:
    case snowy_beach:
    case snowy_taiga:
    case snowy_taiga_hills:
    case ice_spikes:
    case snowy_taiga_mountains:
        return 1;
    default:
        return 0;
    }
}


void initBiomes()
{
}


void setLayerSeed(Layer *layer, int64_t worldSeed)
{
    if (layer->p2 != NULL)
        setLayerSeed(layer->p2, worldSeed);

    if (layer->p != NULL)
        setLayerSeed(layer->p, worldSeed);

    if (layer->noise != NULL)
    {
        int64_t s;
        setSeed(&s, worldSeed);
        perlinInit((PerlinNoise*)layer->noise, &s);
    }

    int64_t ls = layer->layerSalt;
    if (ls == 0) // Pre 1.13 the Hills branch stays zero-initialized
    {
        layer->startSalt = 0;
        layer->startSeed = 0;
    }
    else if (ls == -1) // Post 1.14 VoronoiZoom uses SHA256 for initialization
    {
        layer->startSalt = getVoronoiSHA(worldSeed);
        layer->startSeed = 0;
    }
    else
    {
        int64_t st = worldSeed;
        st = mcStepSeed(st, ls);
        st = mcStepSeed(st, ls);
        st = mcStepSeed(st, ls);

        layer->startSalt = st;
        layer->startSeed = mcStepSeed(st, 0);
    }
}

//==============================================================================
// Noise
//==============================================================================


static double lerp(double part, double from, double to)
{
    return from + part * (to - from);
}

/* Table of vectors to cube edge centres (12 + 4 extra) */
const double cEdgeX[] = {1.0,-1.0, 1.0,-1.0, 1.0,-1.0, 1.0,-1.0, 0.0, 0.0, 0.0, 0.0,  1.0, 0.0,-1.0, 0.0};
const double cEdgeY[] = {1.0, 1.0,-1.0,-1.0, 0.0, 0.0, 0.0, 0.0, 1.0,-1.0, 1.0,-1.0,  1.0,-1.0, 1.0,-1.0};
const double cEdgeZ[] = {0.0, 0.0, 0.0, 0.0, 1.0, 1.0,-1.0,-1.0, 1.0, 1.0,-1.0,-1.0,  0.0, 1.0, 0.0,-1.0};

// grad()
static double indexedLerp(int idx, double d1, double d2, double d3)
{
    idx &= 0xf;
    return cEdgeX[idx] * d1 + cEdgeY[idx] * d2 + cEdgeZ[idx] * d3;
}

void perlinInit(PerlinNoise *rnd, int64_t *seed)
{
    int i = 0;
    memset(rnd, 0, sizeof(*rnd));
    rnd->a = nextDouble(seed) * 256.0;
    rnd->b = nextDouble(seed) * 256.0;
    rnd->c = nextDouble(seed) * 256.0;

    for (i = 0; i < 256; i++)
    {
        rnd->d[i] = i;
    }
    for (i = 0; i < 256; i++)
    {
        int n3 = nextInt(seed, 256 - i) + i;
        int n4 = rnd->d[i];
        rnd->d[i] = rnd->d[n3];
        rnd->d[n3] = n4;
        rnd->d[i + 256] = rnd->d[i];
    }
}


double samplePerlin(const PerlinNoise *rnd, double d1, double d2, double d3)
{
    d1 += rnd->a;
    d2 += rnd->b;
    d3 += rnd->c;
    int i1 = (int)d1 - (int)(d1 < 0);
    int i2 = (int)d2 - (int)(d2 < 0);
    int i3 = (int)d3 - (int)(d3 < 0);
    d1 -= i1;
    d2 -= i2;
    d3 -= i3;
    double t1 = d1*d1*d1 * (d1 * (d1*6.0-15.0) + 10.0);
    double t2 = d2*d2*d2 * (d2 * (d2*6.0-15.0) + 10.0);
    double t3 = d3*d3*d3 * (d3 * (d3*6.0-15.0) + 10.0);

    i1 &= 0xff;
    i2 &= 0xff;
    i3 &= 0xff;

    int a1 = rnd->d[i1]   + i2;
    int a2 = rnd->d[a1]   + i3;
    int a3 = rnd->d[a1+1] + i3;
    int b1 = rnd->d[i1+1] + i2;
    int b2 = rnd->d[b1]   + i3;
    int b3 = rnd->d[b1+1] + i3;

    double l1 = indexedLerp(rnd->d[a2],   d1,   d2,   d3);
    double l2 = indexedLerp(rnd->d[b2],   d1-1, d2,   d3);
    double l3 = indexedLerp(rnd->d[a3],   d1,   d2-1, d3);
    double l4 = indexedLerp(rnd->d[b3],   d1-1, d2-1, d3);
    double l5 = indexedLerp(rnd->d[a2+1], d1,   d2,   d3-1);
    double l6 = indexedLerp(rnd->d[b2+1], d1-1, d2,   d3-1);
    double l7 = indexedLerp(rnd->d[a3+1], d1,   d2-1, d3-1);
    double l8 = indexedLerp(rnd->d[b3+1], d1-1, d2-1, d3-1);

    l1 = lerp(t1, l1, l2);
    l3 = lerp(t1, l3, l4);
    l5 = lerp(t1, l5, l6);
    l7 = lerp(t1, l7, l8);

    l1 = lerp(t2, l1, l3);
    l5 = lerp(t2, l5, l7);

    return lerp(t3, l1, l5);
}

static double simplexGrad(int idx, double x, double y, double z, double d)
{
    double con = d - x*x - y*y - z*z;
    if (con < 0)
        return 0;
    con *= con;
    return con * con * indexedLerp(idx, x, y, z);
}

double sampleSimplex2D(const PerlinNoise *rnd, double x, double y)
{
    const double SKEW = 0.5 * (sqrt(3) - 1.0);
    const double UNSKEW = (3.0 - sqrt(3)) / 6.0;

    double hf = (x + y) * SKEW;
    int hx = (int)floor(x + hf);
    int hz = (int)floor(y + hf);
    double mhxz = (hx + hz) * UNSKEW;
    double x0 = x - (hx - mhxz);
    double y0 = y - (hz - mhxz);
    int offx = (x0 > y0);
    int offz = !offx;
    double x1 = x0 - offx + UNSKEW;
    double y1 = y0 - offz + UNSKEW;
    double x2 = x0 - 1.0 + 2.0 * UNSKEW;
    double y2 = y0 - 1.0 + 2.0 * UNSKEW;
    int gi0 = rnd->d[0xff & (hz)];
    int gi1 = rnd->d[0xff & (hz + offz)];
    int gi2 = rnd->d[0xff & (hz + 1)];
    gi0 = rnd->d[0xff & (gi0 + hx)];
    gi1 = rnd->d[0xff & (gi1 + hx + offx)];
    gi2 = rnd->d[0xff & (gi2 + hx + 1)];
    double t = 0;
    t += simplexGrad(gi0 % 12, x0, y0, 0.0, 0.5);
    t += simplexGrad(gi1 % 12, x1, y1, 0.0, 0.5);
    t += simplexGrad(gi2 % 12, x2, y2, 0.0, 0.5);
    return 70.0 * t;
}


void doublePerlinInit(DoublePerlinNoise *rnd, int64_t *seed,
        PerlinNoise *octavesA, PerlinNoise *octavesB, int omin, int len)
{
    if (len < 1 || omin+len > 0)
    {
        printf("doublePerlinInit(): unsupported octave range\n");
        return;
    }
    rnd->octaves[0] = octavesA;
    rnd->octaves[1] = octavesB;
    rnd->octcnt = len;

    rnd->amplitude = (10.0 / 6.0) * len / (len + 1);

    int ab, i;
    for (ab = 0; ab < 2; ab++)
    {
        // octave zero is initialized first
        if (omin <= 0 && omin+len > 0)
            perlinInit(&rnd->octaves[ab][-omin], seed);
        else
            skipNextN(seed, 262);

        if (omin+len < 0)
            skipNextN(seed, -(omin+len) * 262);
        for (i = 0; i < len; i++)
            if (i+omin < 0)
                perlinInit(&rnd->octaves[ab][i], seed);

        rnd->persist[ab] = pow(2.0, omin+len - 1);
        rnd->lacuna[ab] = 1.0 / ((1LL << len) - 1.0);
    }
}


static double sampleOctave(const PerlinNoise *octaves, int len,
        double x, double y, double z, double persist, double lacuna)
{
    double v = 0;
    int i;
    for (i = 0; i < len; i++)
    {
        double ax = x * persist;
        double ay = y * persist;
        double az = z * persist;
        ax -= floor(ax / 33554432.0 + 0.5) * 33554432.0;
        ay -= floor(ay / 33554432.0 + 0.5) * 33554432.0;
        az -= floor(az / 33554432.0 + 0.5) * 33554432.0;
        v += lacuna * samplePerlin(octaves+i, ax, ay, az);
        persist *= 0.5;
        lacuna *= 2.0;
    }
    return v;
}

double sampleDoublePerlin(const DoublePerlinNoise *rnd,
        double x, double y, double z)
{
    const double f = 337.0 / 331.0;
    double v = 0;

    v += sampleOctave(rnd->octaves[0], rnd->octcnt, x,   y,   z,
        rnd->persist[0], rnd->lacuna[0]);

    v += sampleOctave(rnd->octaves[1], rnd->octcnt, x*f, y*f, z*f,
        rnd->persist[1], rnd->lacuna[1]);

    return v * rnd->amplitude;
}


//==============================================================================
// Nether (1.16+) and End (1.9+) Biome Generation
//==============================================================================

void setNetherSeed(NetherNoise *nn, int64_t seed)
{
    int64_t s;
    setSeed(&s, seed);
    doublePerlinInit(&nn->temperature, &s, &nn->oct[0], &nn->oct[2], -7, 2);
    setSeed(&s, seed+1);
    doublePerlinInit(&nn->humidity, &s, &nn->oct[4], &nn->oct[6], -7, 2);
}

/* Gets the 3D nether biome at scale 1:4 (for 1.16+).
 */
int getNetherBiome(const NetherNoise *nn, int x, int y, int z, float *ndel)
{
    const float npoints[5][4] = {
        { 0,    0,      0,              nether_wastes       },
        { 0,   -0.5,    0,              soul_sand_valley    },
        { 0.4,  0,      0,              crimson_forest      },
        { 0,    0.5,    0.375*0.375,    warped_forest       },
        {-0.5,  0,      0.175*0.175,    basalt_deltas       },
    };

    float temp = sampleDoublePerlin(&nn->temperature, x, y, z);
    float humidity = sampleDoublePerlin(&nn->humidity, x, y, z);

    int i, id = 0;
    float dmin = FLT_MAX;
    float dmin2 = FLT_MAX;
    for (i = 0; i < 5; i++)
    {
        float dx = npoints[i][0] - temp;
        float dy = npoints[i][1] - humidity;
        float dsq = dx*dx + dy*dy + npoints[i][2];
        if (dsq < dmin)
        {
            dmin2 = dmin;
            dmin = dsq;
            id = i;
        }
        else if (dsq < dmin2)
            dmin2 = dsq;
    }

    if (ndel)
        *ndel = sqrtf(dmin2) - sqrtf(dmin);

    id = (int) npoints[id][3];
    return id;
}


static void fillRad3D(int *out, int x, int z, int w, int h, int y, int yh, int id, float rad)
{
    int r, rsq;
    int i, j, k;
    r = (int) (rad);
    if (r <= 0)
        return;
    rsq = (int) floor(rad * rad);

    for (k = -r; k <= r; k++)
    {
        int ak = y+k;
        if (ak < 0 || ak >= yh)
            continue;
        int ksq = k*k;
        int *yout = &out[(w*h)*ak];

        for (j = -r; j <= r; j++)
        {
            int aj = z+j;
            if (aj < 0 || aj >= h)
                continue;
            int jksq = j*j + ksq;
            for (i = -r; i <= r; i++)
            {
                int ai = x+i;
                if (ai < 0 || ai >= w)
                    continue;
                int ijksq = i*i + jksq;
                if (ijksq > rsq)
                    continue;

                yout[aj*w+ai] = id;
            }
        }
    }
}

int mapNether3D(const NetherNoise *nn, int *out, int x, int z, int w, int h, int y, int yh, float confidence)
{
    int i, j, k;
    memset(out, 0, sizeof(int) * yh*w*h);

    // The noisedelta is the distance between the first and second closest
    // biomes within the noise space. Dividing this by the greatest possible
    // gradient (~0.05) gives a minimum diameter of voxels around the sample
    // cell that will have the same biome.
    float invgrad = 1.0 / (confidence * 0.05 * 2);

    for (k = 0; k < yh; k++)
    {
        int *yout = &out[(w*h)*k];

        for (j = 0; j < h; j++)
        {
            for (i = 0; i < w; i++)
            {
                if (yout[j*w+i])
                    continue;
                //yout[j*w+i] = getNetherBiome(nn, x+i, y+k, z+j, NULL);
                //continue;

                float noisedelta;
                int v = getNetherBiome(nn, x+i, y+k, z+j, &noisedelta);
                yout[j*w+i] = v;
                float cellrad = noisedelta * invgrad;
                fillRad3D(out, i, j, w, h, k, yh, v, cellrad);
            }
        }
    }
    return 0;
}

int mapNether2D(const NetherNoise *nn, int *out, int x, int z, int w, int h)
{
    return mapNether3D(nn, out, x, z, w, h, 0, 1, 1.0);
}


void setEndSeed(EndNoise *en, int64_t seed)
{
    int64_t s;
    setSeed(&s, seed);
    skipNextN(&s, 17292);
    perlinInit(en, &s);
}

static int getEndBiome(int hx, int hz, const uint16_t *hmap, int hw)
{
    int i, j;
    const uint16_t ds[26] = { // (25-2*i)*(25-2*i)
        //  0    1    2    3    4    5    6    7    8    9   10   11   12
          625, 529, 441, 361, 289, 225, 169, 121,  81,  49,  25,   9,   1,
        // 13   14   15   16   17   18   19   20   21   22   23   24,  25
            1,   9,  25,  49,  81, 121, 169, 225, 289, 361, 441, 529, 625,
    };

    const uint16_t *p_dsi = ds + (hx < 0);
    const uint16_t *p_dsj = ds + (hz < 0);
    const uint16_t *p_elev = hmap;
    uint32_t h;

    if (abs(hx) > 15 || abs(hz) > 15)
        h = 14401;
    else
        h = 64 * (hx*hx + hz*hz);

    for (j = 0; j < 25; j++)
    {
        uint16_t dsj = p_dsj[j];
        uint16_t e;
        uint32_t u;
        for (i = 0; i < 25; i += 5)
        {
            if (U(e = p_elev[i+0]) && (u = (p_dsi[i+0] + (uint32_t)dsj) * e) < h) h = u;
            if (U(e = p_elev[i+1]) && (u = (p_dsi[i+1] + (uint32_t)dsj) * e) < h) h = u;
            if (U(e = p_elev[i+2]) && (u = (p_dsi[i+2] + (uint32_t)dsj) * e) < h) h = u;
            if (U(e = p_elev[i+3]) && (u = (p_dsi[i+3] + (uint32_t)dsj) * e) < h) h = u;
            if (U(e = p_elev[i+4]) && (u = (p_dsi[i+4] + (uint32_t)dsj) * e) < h) h = u;
        }
        p_elev += hw;
    }

    if (h < 3600)
        return end_highlands;
    else if (h <= 10000)
        return end_midlands;
    else if (h <= 14400)
        return end_barrens;

    return small_end_islands;
}

int mapEndBiome(const EndNoise *en, int *out, int x, int z, int w, int h)
{
    int i, j;
    int hw = w + 26;
    int hh = h + 26;
    uint16_t *hmap = (uint16_t*) malloc(hw * hh * sizeof(*hmap));

    for (j = 0; j < hh; j++)
    {
        for (i = 0; i < hw; i++)
        {
            int64_t rx = x + i - 12;
            int64_t rz = z + j - 12;
            uint16_t v = 0;
            if (rx*rx + rz*rz > 4096 && sampleSimplex2D(en, rx, rz) < -0.9f)
            {
                v = (llabs(rx) * 3439 + llabs(rz) * 147) % 13 + 9;
                v *= v;
            }
            hmap[j*hw+i] = v;
        }
    }

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int64_t hx = (i+x);
            int64_t hz = (j+z);

            if (hx*hx + hz*hz <= 4096L)
                out[j*w+i] = the_end;
            else
            {
                hx = 2*hx + 1;
                hz = 2*hz + 1;
                uint16_t *p_elev = &hmap[(hz/2-z)*hw + (hx/2-x)];
                out[j*w+i] = getEndBiome(hx, hz, p_elev, hw);
            }
        }
    }

    free(hmap);
    return 0;
}

int mapEnd(const EndNoise *en, int *out, int x, int z, int w, int h)
{
    int cx = x >> 2;
    int cz = z >> 2;
    int cw = ((x+w) >> 2) + 1 - cx;
    int ch = ((z+h) >> 2) + 1 - cz;

    int *buf = (int*) malloc(cw * ch * sizeof(int));
    mapEndBiome(en, buf, cx, cz, cw, ch);

    int i, j;

    for (j = 0; j < h; j++)
    {
        int cj = ((z+j) >> 2) - cz;
        for (i = 0; i < w; i++)
        {
            int ci = ((x+i) >> 2) - cx;
            int v = buf[cj*cw+ci];
            out[j*w+i] = v;
        }
    }

    free(buf);
    return 0;
}

//==============================================================================
// Layers
//==============================================================================

// convenience function used in several layers
static inline int isAny4(int id, int a, int b, int c, int d)
{
    return id == a || id == b || id == c || id == d;
}

int mapContinent(const Layer * l, int * out, int x, int z, int w, int h)
{
    int64_t ss = l->startSeed;
    int64_t cs;
    int i, j;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            cs = getChunkSeed(ss, i + x, j + z);
            out[i + j*w] = mcFirstIsZero(cs, 10);
        }
    }

    if (x > -w && x <= 0 && z > -h && z <= 0)
    {
        out[-x + -z * w] = 1;
    }

    return 0;
}

int mapZoomFuzzy(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x >> 1;
    int pZ = z >> 1;
    int pW = ((x + w) >> 1) - pX + 1;
    int pH = ((z + h) >> 1) - pZ + 1;
    int i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if U(err != 0)
        return err;

    int newW = (pW) << 1;
    int newH = (pH) << 1;
    int idx, v00, v01, v10, v11;
    int *buf = (int*) malloc((newW+1)*(newH+1)*sizeof(*buf));

    const int st = (int)l->startSalt;
    const int ss = (int)l->startSeed;

    for (j = 0; j < pH; j++)
    {
        idx = (j << 1) * newW;

        v00 = out[(j+0)*pW];
        v01 = out[(j+1)*pW];

        for (i = 0; i < pW; i++, v00 = v10, v01 = v11)
        {
            v10 = out[i+1 + (j+0)*pW];
            v11 = out[i+1 + (j+1)*pW];

            if (v00 == v01 && v00 == v10 && v00 == v11)
            {
                buf[idx] = v00;
                buf[idx + 1] = v00;
                buf[idx + newW] = v00;
                buf[idx + newW + 1] = v00;
                idx += 2;
                continue;
            }

            int chunkX = (i + pX) << 1;
            int chunkZ = (j + pZ) << 1;

            int cs = ss;
            cs += chunkX;
            cs *= cs * 1284865837 + 4150755663;
            cs += chunkZ;
            cs *= cs * 1284865837 + 4150755663;
            cs += chunkX;
            cs *= cs * 1284865837 + 4150755663;
            cs += chunkZ;

            buf[idx] = v00;
            buf[idx + newW] = (cs >> 24) & 1 ? v01 : v00;
            idx++;

            cs *= cs * 1284865837 + 4150755663;
            cs += st;
            buf[idx] = (cs >> 24) & 1 ? v10 : v00;

            cs *= cs * 1284865837 + 4150755663;
            cs += st;
            int r = (cs >> 24) & 3;
            buf[idx + newW] = r==0 ? v00 : r==1 ? v10 : r==2 ? v01 : v11;
            idx++;
        }
    }

    for (j = 0; j < h; j++)
    {
        memcpy(&out[j*w], &buf[(j + (z & 1))*newW + (x & 1)], w*sizeof(int));
    }

    free(buf);

    return 0;
}


static inline int select4(int cs, int st, int v00, int v01, int v10, int v11)
{
    int v;
    int cv00 = (v00 == v10) + (v00 == v01) + (v00 == v11);
    int cv10 = (v10 == v01) + (v10 == v11);
    int cv01 = (v01 == v11);
    if (cv00 > cv10 && cv00 > cv01) {
        v = v00;
    } else if (cv10 > cv00) {
        v = v10;
    } else if (cv01 > cv00) {
        v = v01;
    } else {
        cs *= cs * 1284865837 + 4150755663;
        cs += st;
        int r = (cs >> 24) & 3;
        v = r==0 ? v00 : r==1 ? v10 : r==2 ? v01 : v11;
    }
    return v;
}

/// This is the most common layer, and generally the second most performance
/// critical after mapAddIsland.
int mapZoom(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x >> 1;
    int pZ = z >> 1;
    int pW = ((x + w) >> 1) - pX + 1; // (w >> 1) + 2;
    int pH = ((z + h) >> 1) - pZ + 1; // (h >> 1) + 2;
    int i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if U(err != 0)
        return err;

    int newW = (pW) << 1;
    int newH = (pH) << 1;
    int idx, v00, v01, v10, v11;
    int *buf = (int*) malloc((newW+1)*(newH+1)*sizeof(*buf));

    const int st = (int)l->startSalt;
    const int ss = (int)l->startSeed;

    for (j = 0; j < pH; j++)
    {
        idx = (j << 1) * newW;

        v00 = out[(j+0)*pW];
        v01 = out[(j+1)*pW];

        for (i = 0; i < pW; i++, v00 = v10, v01 = v11)
        {
            v10 = out[i+1 + (j+0)*pW];
            v11 = out[i+1 + (j+1)*pW];

            if (v00 == v01 && v00 == v10 && v00 == v11)
            {
                buf[idx] = v00;
                buf[idx + 1] = v00;
                buf[idx + newW] = v00;
                buf[idx + newW + 1] = v00;
                idx += 2;
                continue;
            }

            int chunkX = (i + pX) << 1;
            int chunkZ = (j + pZ) << 1;

            int cs = ss;
            cs += chunkX;
            cs *= cs * 1284865837 + 4150755663;
            cs += chunkZ;
            cs *= cs * 1284865837 + 4150755663;
            cs += chunkX;
            cs *= cs * 1284865837 + 4150755663;
            cs += chunkZ;

            buf[idx] = v00;
            buf[idx + newW] = (cs >> 24) & 1 ? v01 : v00;
            idx++;

            cs *= cs * 1284865837 + 4150755663;
            cs += st;
            buf[idx] = (cs >> 24) & 1 ? v10 : v00;

            buf[idx + newW] = select4(cs, st, v00, v01, v10, v11);

            idx++;
        }
    }

    for (j = 0; j < h; j++)
    {
        memcpy(&out[j*w], &buf[(j + (z & 1))*newW + (x & 1)], w*sizeof(int));
    }

    free(buf);

    return 0;
}

/// This is the most performance crittical layer, especially for getBiomeAtPos.
int mapLand(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if U(err != 0)
        return err;

    int64_t st = l->startSalt;
    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        int *vz0 = out + (j+0)*pW;
        int *vz1 = out + (j+1)*pW;
        int *vz2 = out + (j+2)*pW;

        int v00 = vz0[0], vt0 = vz0[1];
        int v02 = vz2[0], vt2 = vz2[1];
        int v20, v22;
        int v11, v;

        for (i = 0; i < w; i++)
        {
            v11 = vz1[i+1];
            v20 = vz0[i+2];
            v22 = vz2[i+2];
            v = v11;

            switch (v11)
            {
            case ocean:
                if (v00 || v20 || v02 || v22) // corners have non-ocean
                {
                    /*
                    setChunkSeed(l,x+i,z+j);
                    int inc = 1;
                    if(v00 != 0 && mcNextInt(l,inc++) == 0) v = v00;
                    if(v20 != 0 && mcNextInt(l,inc++) == 0) v = v20;
                    if(v02 != 0 && mcNextInt(l,inc++) == 0) v = v02;
                    if(v22 != 0 && mcNextInt(l,inc++) == 0) v = v22;
                    if(mcNextInt(l,3) == 0) out[x + z*areaWidth] = v;
                    else if(v == 4)         out[x + z*areaWidth] = 4;
                    else                    out[x + z*areaWidth] = 0;
                    */

                    cs = getChunkSeed(ss, i+x, j+z);
                    int inc = 0;
                    v = 1;

                    if (v00 != ocean)
                    {
                        ++inc; v = v00;
                        cs = mcStepSeed(cs, st);
                    }
                    if (v20 != ocean)
                    {
                        if (++inc == 1 || mcFirstIsZero(cs, 2)) v = v20;
                        cs = mcStepSeed(cs, st);
                    }
                    if (v02 != ocean)
                    {
                        switch (++inc)
                        {
                        case 1:     v = v02; break;
                        case 2:     if (mcFirstIsZero(cs, 2)) v = v02; break;
                        default:    if (mcFirstIsZero(cs, 3)) v = v02;
                        }
                        cs = mcStepSeed(cs, st);
                    }
                    if (v22 != ocean)
                    {
                        switch (++inc)
                        {
                        case 1:     v = v22; break;
                        case 2:     if (mcFirstIsZero(cs, 2)) v = v22; break;
                        case 3:     if (mcFirstIsZero(cs, 3)) v = v22; break;
                        default:    if (mcFirstIsZero(cs, 4)) v = v22;
                        }
                        cs = mcStepSeed(cs, st);
                    }

                    if (v != forest)
                    {
                        if (!mcFirstIsZero(cs, 3))
                            v = ocean;
                    }
                }
                break;

            case forest:
                break;

            default:
                if (v00 == 0 || v20 == 0 || v02 == 0 || v22 == 0)
                {
                    cs = getChunkSeed(ss, i+x, j+z);
                    if (mcFirstIsZero(cs, 5))
                        v = 0;
                }
            }

            out[i + j*w] = v;
            v00 = vt0; vt0 = v20;
            v02 = vt2; vt2 = v22;
        }
    }

    return 0;
}

int mapLand16(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if U(err != 0)
        return err;

    int64_t st = l->startSalt;
    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        int *vz0 = out + (j+0)*pW;
        int *vz1 = out + (j+1)*pW;
        int *vz2 = out + (j+2)*pW;

        int v00 = vz0[0], vt0 = vz0[1];
        int v02 = vz2[0], vt2 = vz2[1];
        int v20, v22;
        int v11, v;

        for (i = 0; i < w; i++)
        {
            v11 = vz1[i+1];
            v20 = vz0[i+2];
            v22 = vz2[i+2];
            v = v11;


            if (v11 != 0 || (v00 == 0 && v20 == 0 && v02 == 0 && v22 == 0))
            {
                if (v11 != 0 && (v00 == 0 || v20 == 0 || v02 == 0 || v22 == 0))
                {
                    cs = getChunkSeed(ss, i+x, j+z);
                    if (mcFirstIsZero(cs, 5))
                        v = (v == snowy_tundra) ? frozen_ocean : ocean;
                }
            }
            else
            {
                cs = getChunkSeed(ss, i+x, j+z);
                int inc = 0;
                v = 1;

                if (v00 != ocean)
                {
                    ++inc; v = v00;
                    cs = mcStepSeed(cs, st);
                }
                if (v20 != ocean)
                {
                    if (++inc == 1 || mcFirstIsZero(cs, 2)) v = v20;
                    cs = mcStepSeed(cs, st);
                }
                if (v02 != ocean)
                {
                    switch (++inc)
                    {
                    case 1:     v = v02; break;
                    case 2:     if (mcFirstIsZero(cs, 2)) v = v02; break;
                    default:    if (mcFirstIsZero(cs, 3)) v = v02;
                    }
                    cs = mcStepSeed(cs, st);
                }
                if (v22 != ocean)
                {
                    switch (++inc)
                    {
                    case 1:     v = v22; break;
                    case 2:     if (mcFirstIsZero(cs, 2)) v = v22; break;
                    case 3:     if (mcFirstIsZero(cs, 3)) v = v22; break;
                    default:    if (mcFirstIsZero(cs, 4)) v = v22;
                    }
                    cs = mcStepSeed(cs, st);
                }

                if (!mcFirstIsZero(cs, 3))
                    v = (v == snowy_tundra) ? frozen_ocean : ocean;
            }

            out[i + j*w] = v;
            v00 = vt0; vt0 = v20;
            v02 = vt2; vt2 = v22;
        }
    }

    return 0;
}


int mapIsland(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if U(err != 0)
        return err;

    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[i+1 + (j+1)*pW];
            out[i + j*w] = v11;

            if (v11 == Oceanic)
            {
                if (out[i+1 + (j+0)*pW] != Oceanic) continue;
                if (out[i+2 + (j+1)*pW] != Oceanic) continue;
                if (out[i+0 + (j+1)*pW] != Oceanic) continue;
                if (out[i+1 + (j+2)*pW] != Oceanic) continue;

                cs = getChunkSeed(ss, i+x, j+z);
                if (mcFirstIsZero(cs, 2))
                {
                    out[i + j*w] = 1;
                }
            }
        }
    }

    return 0;
}

int mapSnow16(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if U(err != 0)
        return err;

    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[i+1 + (j+1)*pW];
            if (v11 == 0)
            {
                out[i + j*w] = v11;
                continue;
            }
            cs = getChunkSeed(ss, i+x, j+z);
            out[i + j*w] = mcFirstIsZero(cs, 5) ? snowy_tundra : plains;
        }
    }

    return 0;
}

int mapSnow(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if U(err != 0)
        return err;

    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[i+1 + (j+1)*pW];

            if (isShallowOcean(v11))
            {
                out[i + j*w] = v11;
            }
            else
            {
                cs = getChunkSeed(ss, i+x, j+z);
                int r = mcFirstInt(cs, 6);
                int v;

                if      (r == 0) v = Freezing;
                else if (r <= 1) v = Cold;
                else             v = Warm;

                out[i + j*w] = v;
            }
        }
    }

    return 0;
}


int mapCool(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if U(err != 0)
        return err;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[i+1 + (j+1)*pW];

            if (v11 == Warm)
            {
                int v10 = out[i+1 + (j+0)*pW];
                int v21 = out[i+2 + (j+1)*pW];
                int v01 = out[i+0 + (j+1)*pW];
                int v12 = out[i+1 + (j+2)*pW];

                if (isAny4(Cold, v10, v21, v01, v12) || isAny4(Freezing, v10, v21, v01, v12))
                {
                    v11 = Lush;
                }
            }

            out[i + j*w] = v11;
        }
    }

    return 0;
}


int mapHeat(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if U(err != 0)
        return err;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[i+1 + (j+1)*pW];

            if (v11 == Freezing)
            {
                int v10 = out[i+1 + (j+0)*pW];
                int v21 = out[i+2 + (j+1)*pW];
                int v01 = out[i+0 + (j+1)*pW];
                int v12 = out[i+1 + (j+2)*pW];

                if (isAny4(Warm, v10, v21, v01, v12) || isAny4(Lush, v10, v21, v01, v12))
                {
                    v11 = Cold;
                }
            }

            out[i + j*w] = v11;
        }
    }

    return 0;
}


int mapSpecial(const Layer * l, int * out, int x, int z, int w, int h)
{
    int err = l->p->getMap(l->p, out, x, z, w, h);
    if U(err != 0)
        return err;

    int64_t st = l->startSalt;
    int64_t ss = l->startSeed;
    int64_t cs;

    int i, j;
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v = out[i + j*w];
            if (v == 0) continue;

            cs = getChunkSeed(ss, i+x, j+z);

            if (mcFirstIsZero(cs, 13))
            {
                cs = mcStepSeed(cs, st);
                v |= (1 + mcFirstInt(cs, 15)) << 8 & 0xf00;
                // 1 to 1 mapping so 'out' can be overwritten immediately
                out[i + j*w] = v;
            }
        }
    }

    return 0;
}


int mapMushroom(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if U(err != 0)
        return err;

    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[i+1 + (j+1)*pW];

            // surrounded by ocean?
            if (v11 == 0 &&
                !out[i+0 + (j+0)*pW] && !out[i+2 + (j+0)*pW] &&
                !out[i+0 + (j+2)*pW] && !out[i+2 + (j+2)*pW])
            {
                cs = getChunkSeed(ss, i+x, j+z);
                if (mcFirstIsZero(cs, 100))
                    v11 = mushroom_fields;
            }

            out[i + j*w] = v11;
        }
    }

    return 0;
}


int mapDeepOcean(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if U(err != 0)
        return err;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[(i+1) + (j+1)*pW];

            if (isShallowOcean(v11))
            {
                // count adjacent oceans
                int oceans = 0;
                if (isShallowOcean(out[(i+1) + (j+0)*pW])) oceans++;
                if (isShallowOcean(out[(i+2) + (j+1)*pW])) oceans++;
                if (isShallowOcean(out[(i+0) + (j+1)*pW])) oceans++;
                if (isShallowOcean(out[(i+1) + (j+2)*pW])) oceans++;

                if (oceans >= 4)
                {
                    switch (v11)
                    {
                    case warm_ocean:
                        v11 = deep_warm_ocean;
                        break;
                    case lukewarm_ocean:
                        v11 = deep_lukewarm_ocean;
                        break;
                    case ocean:
                        v11 = deep_ocean;
                        break;
                    case cold_ocean:
                        v11 = deep_cold_ocean;
                        break;
                    case frozen_ocean:
                        v11 = deep_frozen_ocean;
                        break;
                    default:
                        v11 = deep_ocean;
                    }
                }
            }

            out[i + j*w] = v11;
        }
    }

    return 0;
}


const int warmBiomes[] = {desert, desert, desert, savanna, savanna, plains};
const int lushBiomes[] = {forest, dark_forest, mountains, plains, birch_forest, swamp};
const int coldBiomes[] = {forest, mountains, taiga, plains};
const int snowBiomes[] = {snowy_tundra, snowy_tundra, snowy_tundra, snowy_taiga};

const int oldBiomes[] = { desert, forest, mountains, swamp, plains, taiga, jungle };
const int oldBiomes11[] = { desert, forest, mountains, swamp, plains, taiga };
//const int lushBiomesBE[] = {forest, dark_forest, mountains, plains, plains, plains, birch_forest, swamp};

int mapBiome(const Layer * l, int * out, int x, int z, int w, int h)
{
    int err = l->p->getMap(l->p, out, x, z, w, h);
    if U(err != 0)
        return err;

    int mc = l->mc;
    int64_t ss = l->startSeed;
    int64_t cs;

    int i, j;
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v;
            int idx = i + j*w;
            int id = out[idx];
            int hasHighBit = (id & 0xf00);
            id &= ~0xf00;

            if (mc <= MC_1_6)
            {
                if (id == ocean || id == mushroom_fields)
                {
                    out[idx] = id;
                    continue;
                }

                cs = getChunkSeed(ss, i + x, j + z);

                if (mc <= MC_1_1)
                    v = oldBiomes11[mcFirstInt(cs, 6)];
                else
                    v = oldBiomes[mcFirstInt(cs, 7)];

                if (id != plains && (v != taiga || mc <= MC_1_2))
                    v = snowy_tundra;
            }
            else
            {
                if (isOceanic(id) || id == mushroom_fields)
                {
                    out[idx] = id;
                    continue;
                }

                cs = getChunkSeed(ss, i + x, j + z);

                switch (id)
                {
                case Warm:
                    if (hasHighBit) v = mcFirstIsZero(cs, 3) ? badlands_plateau : wooded_badlands_plateau;
                    else v = warmBiomes[mcFirstInt(cs, 6)];
                    break;
                case Lush:
                    if (hasHighBit) v = jungle;
                    else v = lushBiomes[mcFirstInt(cs, 6)];
                    break;
                case Cold:
                    if (hasHighBit) v = giant_tree_taiga;
                    else v = coldBiomes[mcFirstInt(cs, 4)];
                    break;
                case Freezing:
                    v = snowBiomes[mcFirstInt(cs, 4)];
                    break;
                default:
                    v = mushroom_fields;
                }
            }

            out[idx] = v;
        }
    }

    return 0;
}


int mapNoise(const Layer * l, int * out, int x, int z, int w, int h)
{
    int err = l->p->getMap(l->p, out, x, z, w, h);
    if U(err != 0)
        return err;

    int64_t ss = l->startSeed;
    int64_t cs;

    int mod = (l->mc <= MC_1_6) ? 2 : 299999;

    int i, j;
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            if (out[i + j*w] > 0)
            {
                cs = getChunkSeed(ss, i + x, j + z);
                out[i + j*w] = mcFirstInt(cs, mod)+2;
            }
            else
            {
                out[i + j*w] = 0;
            }
        }
    }

    return 0;
}


int mapBamboo(const Layer * l, int * out, int x, int z, int w, int h)
{
    int err = l->p->getMap(l->p, out, x, z, w, h);
    if U(err != 0)
        return err;

    int64_t ss = l->startSeed;
    int64_t cs;

    int i, j;
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int idx = i + j*w;
            if (out[idx] != jungle) continue;

            cs = getChunkSeed(ss, i + x, j + z);
            if (mcFirstIsZero(cs, 10))
            {
                out[idx] = bamboo_jungle;
            }
        }
    }

    return 0;
}


static inline int replaceEdge(int *out, int idx, int mc, int v10, int v21, int v01, int v12, int id, int baseID, int edgeID)
{
    if (id != baseID) return 0;

    if (areSimilar(mc, v10, baseID) && areSimilar(mc, v21, baseID) &&
        areSimilar(mc, v01, baseID) && areSimilar(mc, v12, baseID))
        out[idx] = id;
    else
        out[idx] = edgeID;

    return 1;
}


int mapBiomeEdge(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;
    int mc = l->mc;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if U(err != 0)
        return err;

    for (j = 0; j < h; j++)
    {
        int *vz0 = out + (j+0)*pW;
        int *vz1 = out + (j+1)*pW;
        int *vz2 = out + (j+2)*pW;

        for (i = 0; i < w; i++)
        {
            int v11 = vz1[i+1];
            int v10 = vz0[i+1];
            int v21 = vz1[i+2];
            int v01 = vz1[i+0];
            int v12 = vz2[i+1];

            if (!replaceEdge(out, i + j*w, mc, v10, v21, v01, v12, v11, wooded_badlands_plateau, badlands) &&
                !replaceEdge(out, i + j*w, mc, v10, v21, v01, v12, v11, badlands_plateau, badlands) &&
                !replaceEdge(out, i + j*w, mc, v10, v21, v01, v12, v11, giant_tree_taiga, taiga))
            {
                if (v11 == desert)
                {
                    if (!isAny4(snowy_tundra, v10, v21, v01, v12))
                    {
                        out[i + j*w] = v11;
                    }
                    else
                    {
                        out[i + j*w] = wooded_mountains;
                    }
                }
                else if (v11 == swamp)
                {
                    if (!isAny4(desert, v10, v21, v01, v12) &&
                        !isAny4(snowy_taiga, v10, v21, v01, v12) &&
                        !isAny4(snowy_tundra, v10, v21, v01, v12))
                    {
                        if (!isAny4(jungle, v10, v21, v01, v12) &&
                            !isAny4(bamboo_jungle, v10, v21, v01, v12))
                            out[i + j*w] = v11;
                        else
                            out[i + j*w] = jungle_edge;
                    }
                    else
                    {
                        out[i + j*w] = plains;
                    }
                }
                else
                {
                    out[i + j*w] = v11;
                }
            }
        }
    }

    return 0;
}


int mapHills(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;
    int *buf = NULL;

    if U(l->p2 == NULL)
    {
        printf("mapHills() requires two parents! Use setupMultiLayer()\n");
        exit(1);
    }

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if U(err != 0)
        return err;

    buf = (int *) malloc(pW*pH*sizeof(int));
    memcpy(buf, out, pW*pH*sizeof(int));

    err = l->p2->getMap(l->p2, out, pX, pZ, pW, pH);
    if U(err != 0)
    {
        free(buf);
        return err;
    }

    int mc = l->mc;
    int64_t st = l->startSalt;
    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int a11 = buf[i+1 + (j+1)*pW]; // biome branch
            int b11 = out[i+1 + (j+1)*pW]; // river branch
            int idx = i + j*w;
            int bn = -1;

            if (mc >= MC_1_7)
                bn = (b11 - 2) % 29;

            if (bn == 1 && b11 >= 2 && !isShallowOcean(a11))
            {
                int m = getMutated(mc, a11);
                if (m > 0)
                    out[idx] = m;
                else
                    out[idx] = a11;
            }
            else
            {
                cs = getChunkSeed(ss, i + x, j + z);
                if (bn == 0 || mcFirstIsZero(cs, 3))
                {
                    int hillID = a11;

                    switch (a11)
                    {
                    case desert:
                        hillID = desert_hills;
                        break;
                    case forest:
                        hillID = wooded_hills;
                        break;
                    case birch_forest:
                        hillID = birch_forest_hills;
                        break;
                    case dark_forest:
                        hillID = plains;
                        break;
                    case taiga:
                        hillID = taiga_hills;
                        break;
                    case giant_tree_taiga:
                        hillID = giant_tree_taiga_hills;
                        break;
                    case snowy_taiga:
                        hillID = snowy_taiga_hills;
                        break;
                    case plains:
                        if (mc <= MC_1_6) {
                            hillID = forest;
                            break;
                        }
                        cs = mcStepSeed(cs, st);
                        hillID = mcFirstIsZero(cs, 3) ? wooded_hills : forest;
                        break;
                    case snowy_tundra:
                        hillID = snowy_mountains;
                        break;
                    case jungle:
                        hillID = jungle_hills;
                        break;
                    case bamboo_jungle:
                        hillID = bamboo_jungle_hills;
                        break;
                    case ocean:
                        if (mc >= MC_1_7)
                            hillID = deep_ocean;
                        break;
                    case mountains:
                        if (mc >= MC_1_7)
                            hillID = wooded_mountains;
                        break;
                    case savanna:
                        hillID = savanna_plateau;
                        break;
                    default:
                        if (areSimilar(mc, a11, wooded_badlands_plateau))
                            hillID = badlands;
                        else if (isDeepOcean(a11))
                        {
                            cs = mcStepSeed(cs, st);
                            if (mcFirstIsZero(cs, 3))
                            {
                                cs = mcStepSeed(cs, st);
                                hillID = mcFirstIsZero(cs, 2) ? plains : forest;
                            }
                        }
                        break;
                    }

                    if (bn == 0 && hillID != a11)
                    {
                        hillID = getMutated(mc, hillID);
                        if (hillID < 0)
                            hillID = a11;
                    }

                    if (hillID != a11)
                    {
                        int a10 = buf[i+1 + (j+0)*pW];
                        int a21 = buf[i+2 + (j+1)*pW];
                        int a01 = buf[i+0 + (j+1)*pW];
                        int a12 = buf[i+1 + (j+2)*pW];
                        int equals = 0;

                        if (areSimilar(mc, a10, a11)) equals++;
                        if (areSimilar(mc, a21, a11)) equals++;
                        if (areSimilar(mc, a01, a11)) equals++;
                        if (areSimilar(mc, a12, a11)) equals++;

                        if (equals >= 3 + (mc <= MC_1_6))
                            out[idx] = hillID;
                        else
                            out[idx] = a11;
                    }
                    else
                    {
                        out[idx] = a11;
                    }
                }
                else
                {
                    out[idx] = a11;
                }
            }
        }
    }

    free(buf);
    return 0;
}


static inline int reduceID(int id)
{
    return id >= 2 ? 2 + (id & 1) : id;
}

int mapRiver(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if U(err != 0)
        return err;

    int mc = l->mc;

    for (j = 0; j < h; j++)
    {
        int *vz0 = out + (j+0)*pW;
        int *vz1 = out + (j+1)*pW;
        int *vz2 = out + (j+2)*pW;

        for (i = 0; i < w; i++)
        {
            int v01 = vz1[i+0];
            int v11 = vz1[i+1];
            int v21 = vz1[i+2];
            int v10 = vz0[i+1];
            int v12 = vz2[i+1];

            if (mc >= MC_1_7)
            {
                v01 = reduceID(v01);
                v11 = reduceID(v11);
                v21 = reduceID(v21);
                v10 = reduceID(v10);
                v12 = reduceID(v12);
            }
            else if (v11 == 0)
            {
                out[i + j * w] = river;
                continue;
            }

            if (v11 == v01 && v11 == v10 && v11 == v12 && v11 == v21)
            {
                out[i + j * w] = -1;
            }
            else
            {
                out[i + j * w] = river;
            }
        }
    }

    return 0;
}


int mapSmooth(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if U(err != 0)
        return err;

    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        int *vz0 = out + (j+0)*pW;
        int *vz1 = out + (j+1)*pW;
        int *vz2 = out + (j+2)*pW;

        for (i = 0; i < w; i++)
        {
            int v11 = vz1[i+1];
            int v01 = vz1[i+0];
            int v10 = vz0[i+1];

            if (v11 != v01 || v11 != v10)
            {
                int v21 = vz1[i+2];
                int v12 = vz2[i+1];
                if (v01 == v21 && v10 == v12)
                {
                    cs = getChunkSeed(ss, i+x, j+z);
                    if (cs & ((int64_t)1 << 24))
                        v11 = v10;
                    else
                        v11 = v01;
                }
                else
                {
                    if (v01 == v21) v11 = v01;
                    if (v10 == v12) v11 = v10;
                }
            }

            out[i + j * w] = v11;
        }
    }

    return 0;
}


int mapSunflower(const Layer * l, int * out, int x, int z, int w, int h)
{
    int i, j;

    int err = l->p->getMap(l->p, out, x, z, w, h);
    if U(err != 0)
        return err;

    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v = out[i + j * w];

            if (v == plains)
            {
                cs = getChunkSeed(ss, i + x, j + z);
                if (mcFirstIsZero(cs, 57))
                {
                    out[i + j*w] = sunflower_plains;
                }
            }
        }
    }

    return 0;
}


inline static int replaceOcean(int *out, int idx, int v10, int v21, int v01, int v12, int id, int replaceID)
{
    if (isOceanic(id)) return 0;

    if (isOceanic(v10) || isOceanic(v21) || isOceanic(v01) || isOceanic(v12))
        out[idx] = replaceID;
    else
        out[idx] = id;

    return 1;
}

inline static int isAll4JFTO(int mc, int a, int b, int c, int d)
{
    return
        (getCategory(mc, a) == jungle || a == forest || a == taiga || isOceanic(a)) &&
        (getCategory(mc, b) == jungle || b == forest || b == taiga || isOceanic(b)) &&
        (getCategory(mc, c) == jungle || c == forest || c == taiga || isOceanic(c)) &&
        (getCategory(mc, d) == jungle || d == forest || d == taiga || isOceanic(d));
}

inline static int isAny4Oceanic(int a, int b, int c, int d)
{
    return isOceanic(a) || isOceanic(b) || isOceanic(c) || isOceanic(d);
}

int mapShore(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if U(err != 0)
        return err;

    int mc = l->mc;

    for (j = 0; j < h; j++)
    {
        int *vz0 = out + (j+0)*pW;
        int *vz1 = out + (j+1)*pW;
        int *vz2 = out + (j+2)*pW;

        for (i = 0; i < w; i++)
        {
            int v11 = vz1[i+1];
            int v10 = vz0[i+1];
            int v21 = vz1[i+2];
            int v01 = vz1[i+0];
            int v12 = vz2[i+1];

            if (v11 == mushroom_fields)
            {
                if (isAny4(ocean, v10, v21, v01, v12))
                    out[i + j*w] = mushroom_field_shore;
                else
                    out[i + j*w] = v11;
                continue;
            }

            if (mc <= MC_1_6)
            {
                if (v11 == mountains)
                {
                    if (v10 != mountains || v21 != mountains || v01 != mountains || v12 != mountains)
                        v11 = mountain_edge;
                }
                else if (v11 != ocean && v11 != river && v11 != swamp)
                {
                    if (isAny4(ocean, v10, v21, v01, v12))
                        v11 = beach;
                }
                out[i + j*w] = v11;
            }
            else if (getCategory(mc, v11) == jungle)
            {
                if (isAll4JFTO(mc, v10, v21, v01, v12))
                {
                    if (isAny4Oceanic(v10, v21, v01, v12))
                        out[i + j*w] = beach;
                    else
                        out[i + j*w] = v11;
                }
                else
                {
                    out[i + j*w] = jungle_edge;
                }
            }
            else if (v11 == mountains || v11 == wooded_mountains /* || v11 == mountain_edge*/)
            {
                replaceOcean(out, i + j*w, v10, v21, v01, v12, v11, stone_shore);
            }
            else if (isSnowy(v11))
            {
                replaceOcean(out, i + j*w, v10, v21, v01, v12, v11, snowy_beach);
            }
            else if (v11 == badlands || v11 == wooded_badlands_plateau)
            {
                if (!isAny4Oceanic(v10, v21, v01, v12))
                {
                    if (isMesa(v10) && isMesa(v21) && isMesa(v01) && isMesa(v12))
                        out[i + j*w] = v11;
                    else
                        out[i + j*w] = desert;
                }
                else
                {
                    out[i + j*w] = v11;
                }
            }
            else
            {
                if (v11 != ocean && v11 != deep_ocean && v11 != river && v11 != swamp)
                {
                    if (isAny4Oceanic(v10, v21, v01, v12))
                        out[i + j*w] = beach;
                    else
                        out[i + j*w] = v11;
                }
                else
                {
                    out[i + j*w] = v11;
                }
            }
        }
    }

    return 0;
}

int mapSwampRiver(const Layer * l, int * out, int x, int z, int w, int h)
{
    int i, j;

    int err = l->p->getMap(l->p, out, x, z, w, h);
    if U(err != 0)
        return err;

    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v = out[i + j*w];
            if (v != swamp && v != jungle && v != jungle_hills)
                continue;

            cs = getChunkSeed(ss, i + x, j + z);
            if (mcFirstIsZero(cs, (v == swamp) ? 6 : 8))
                v = river;

            out[i + j*w] = v;
        }
    }

    return 0;
}


int mapRiverMix(const Layer * l, int * out, int x, int z, int w, int h)
{
    int idx;
    int len;
    int *buf;

    if U(l->p2 == NULL)
    {
        printf("mapRiverMix() requires two parents! Use setupMultiLayer()\n");
        exit(1);
    }


    int err = l->p->getMap(l->p, out, x, z, w, h); // biome chain
    if U(err != 0)
        return err;

    len = w*h;
    buf = (int *) malloc(len*sizeof(int));
    memcpy(buf, out, len*sizeof(int));

    err = l->p2->getMap(l->p2, out, x, z, w, h); // rivers
    if U(err != 0)
    {
        free(buf);
        return err;
    }

    int mc = l->mc;

    for (idx = 0; idx < len; idx++)
    {
        int v = buf[idx];

        if (out[idx] == river && v != ocean && (mc < MC_1_7 || !isOceanic(v)))
        {
            if (v == snowy_tundra)
                v = frozen_river;
            else if (v == mushroom_fields || v == mushroom_field_shore)
                v = mushroom_field_shore;
            else
                v = river;
        }

        out[idx] = v;
    }

    free(buf);
    return 0;
}


int mapOceanTemp(const Layer * l, int * out, int x, int z, int w, int h)
{
    int i, j;
    const PerlinNoise *rnd = (const PerlinNoise*) l->noise;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            double tmp = samplePerlin(rnd, (i + x) / 8.0, (j + z) / 8.0, 0);

            if (tmp > 0.4)
                out[i + j*w] = warm_ocean;
            else if (tmp > 0.2)
                out[i + j*w] = lukewarm_ocean;
            else if (tmp < -0.4)
                out[i + j*w] = frozen_ocean;
            else if (tmp < -0.2)
                out[i + j*w] = cold_ocean;
            else
                out[i + j*w] = ocean;
        }
    }

    return 0;
}


int mapOceanMix(const Layer * l, int * out, int x, int z, int w, int h)
{
    int *land, *otyp;
    int i, j;
    int lx0, lx1, lz0, lz1, lw, lh;

    if U(l->p2 == NULL)
    {
        printf("mapOceanMix() requires two parents! Use setupMultiLayer()\n");
        exit(1);
    }

    int err = l->p2->getMap(l->p2, out, x, z, w, h);
    if U(err != 0)
        return err;

    otyp = (int *) malloc(w*h*sizeof(int));
    memcpy(otyp, out, w*h*sizeof(int));

    // determine the minimum required land area
    lx0 = 0; lx1 = w;
    lz0 = 0; lz1 = h;

    for (j = 0; j < h; j++)
    {
        int jcentre = (j-8 > 0 && j+9 < h);
        for (i = 0; i < w; i++)
        {
            if (jcentre && i-8 > 0 && i+9 < w)
                continue;
            int oceanID = otyp[i + j*w];
            if (oceanID == warm_ocean || oceanID == frozen_ocean)
            {
                if (i-8 < lx0) lx0 = i-8;
                if (i+9 > lx1) lx1 = i+9;
                if (j-8 < lz0) lz0 = j-8;
                if (j+9 > lz1) lz1 = j+9;
            }
        }
    }

    lw = lx1 - lx0;
    lh = lz1 - lz0;
    err = l->p->getMap(l->p, out, x+lx0, z+lz0, lw, lh);
    if U(err != 0)
    {
        free(otyp);
        return err;
    }

    land = (int *) malloc(lw*lh*sizeof(int));
    memcpy(land, out, lw*lh*sizeof(int));


    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int landID, oceanID, replaceID;

            landID = land[(i-lx0) + (j-lz0)*lw];
            int ii, jj;

            if (!isOceanic(landID))
            {
                out[i + j*w] = landID;
                continue;
            }

            oceanID = otyp[i + j*w];
            if      (oceanID == warm_ocean  ) replaceID = lukewarm_ocean;
            else if (oceanID == frozen_ocean) replaceID = cold_ocean;
            else replaceID = -1;

            if (replaceID > 0)
            {
                for (ii = -8; ii <= 8; ii += 4)
                {
                    for (jj = -8; jj <= 8; jj += 4)
                    {
                        if (!isOceanic(land[(i+ii-lx0) + (j+jj-lz0)*lw]))
                        {
                            out[i + j*w] = replaceID;
                            goto loop_x;
                        }
                    }
                }
            }

            if (landID == deep_ocean)
            {
                switch (oceanID)
                {
                case lukewarm_ocean:
                    oceanID = deep_lukewarm_ocean;
                    break;
                case ocean:
                    oceanID = deep_ocean;
                    break;
                case cold_ocean:
                    oceanID = deep_cold_ocean;
                    break;
                case frozen_ocean:
                    oceanID = deep_frozen_ocean;
                    break;
                }
            }

            out[i + j*w] = oceanID;

            loop_x:;
        }
    }

    free(land);
    free(otyp);

    return 0;
}

static inline void getVoronoiCell(int64_t sha, int a, int b, int c,
        int *x, int *y, int *z)
{
    int64_t s = sha;
    s = mcStepSeed(s, a);
    s = mcStepSeed(s, b);
    s = mcStepSeed(s, c);
    s = mcStepSeed(s, a);
    s = mcStepSeed(s, b);
    s = mcStepSeed(s, c);

    *x = (((s >> 24) & 1023) - 512) * 36;
    s = mcStepSeed(s, sha);
    *y = (((s >> 24) & 1023) - 512) * 36;
    s = mcStepSeed(s, sha);
    *z = (((s >> 24) & 1023) - 512) * 36;
}

int mapVoronoi(const Layer * l, int * out, int x, int z, int w, int h)
{
    x -= 2;
    z -= 2;
    int pX = x >> 2;
    int pZ = z >> 2;
    int pW = ((x + w) >> 2) - pX + 2;
    int pH = ((z + h) >> 2) - pZ + 2;

    if (l->p)
    {
        int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
        if (err != 0)
            return err;
    }

    int64_t sha = l->startSalt;
    int *buf = (int *) malloc(w*h*sizeof(*buf));

    int x000, x001, x010, x011, x100, x101, x110, x111;
    int y000, y001, y010, y011, y100, y101, y110, y111;
    int z000, z001, z010, z011, z100, z101, z110, z111;
    int pi, pj, ii, jj, dx, dz, pjz, pix, i4, j4;
    int v00, v01, v10, v11, v;
    int prev_skip;
    int64_t r;
    uint64_t d, dmin;
    int i, j;

    for (pj = 0; pj < pH-1; pj++)
    {
        v00 = out[(pj+0)*pW];
        v10 = out[(pj+1)*pW];
        pjz = pZ + pj;
        j4 = ((pjz) << 2) - z;
        prev_skip = 1;

        for (pi = 0; pi < pW-1; pi++)
        {
            PREFETCH( buf + ((pjz << 2) + 0) * w + pi, 1, 1 );
            PREFETCH( buf + ((pjz << 2) + 1) * w + pi, 1, 1 );
            PREFETCH( buf + ((pjz << 2) + 2) * w + pi, 1, 1 );
            PREFETCH( buf + ((pjz << 2) + 3) * w + pi, 1, 1 );

            v01 = out[(pj+0)*pW + (pi+1)];
            v11 = out[(pj+1)*pW + (pi+1)];
            pix = pX + pi;
            i4 = ((pix) << 2) - x;

            if (v00 == v01 && v00 == v10 && v00 == v11)
            {
                for (jj = 0; jj < 4; jj++)
                {
                    j = j4 + jj;
                    if (j < 0 || j >= h) continue;
                    for (ii = 0; ii < 4; ii++)
                    {
                        i = i4 + ii;
                        if (i < 0 || i >= w) continue;
                        buf[j*w + i] = v00;
                    }
                }
                prev_skip = 1;
                continue;
            }
            if (prev_skip)
            {
                getVoronoiCell(sha, pix, -1, pjz+0, &x000, &y000, &z000);
                getVoronoiCell(sha, pix,  0, pjz+0, &x001, &y001, &z001);
                getVoronoiCell(sha, pix, -1, pjz+1, &x100, &y100, &z100);
                getVoronoiCell(sha, pix,  0, pjz+1, &x101, &y101, &z101);
                prev_skip = 0;
            }
            getVoronoiCell(sha, pix+1, -1, pjz+0, &x010, &y010, &z010);
            getVoronoiCell(sha, pix+1,  0, pjz+0, &x011, &y011, &z011);
            getVoronoiCell(sha, pix+1, -1, pjz+1, &x110, &y110, &z110);
            getVoronoiCell(sha, pix+1,  0, pjz+1, &x111, &y111, &z111);


            for (jj = 0; jj < 4; jj++)
            {
                j = j4 + jj;
                if (j < 0 || j >= h) continue;
                for (ii = 0; ii < 4; ii++)
                {
                    i = i4 + ii;
                    if (i < 0 || i >= w) continue;

                    const int A = 40*1024;
                    const int B = 20*1024;
                    dx = ii * 10*1024;
                    dz = jj * 10*1024;
                    dmin = (uint64_t)-1;

                    v = v00;
                    d = 0;
                    r = x000 - 0 + dx;  d += r*r;
                    r = y000 + B;       d += r*r;
                    r = z000 - 0 + dz;  d += r*r;
                    if (d < dmin) { dmin = d; }
                    d = 0;
                    r = x001 - 0 + dx;  d += r*r;
                    r = y001 - B;       d += r*r;
                    r = z001 - 0 + dz;  d += r*r;
                    if (d < dmin) { dmin = d; }

                    d = 0;
                    r = x010 - A + dx;  d += r*r;
                    r = y010 + B;       d += r*r;
                    r = z010 - 0 + dz;  d += r*r;
                    if (d < dmin) { dmin = d; v = v01; }
                    d = 0;
                    r = x011 - A + dx;  d += r*r;
                    r = y011 - B;       d += r*r;
                    r = z011 - 0 + dz;  d += r*r;
                    if (d < dmin) { dmin = d; v = v01; }

                    d = 0;
                    r = x100 - 0 + dx;  d += r*r;
                    r = y100 + B;       d += r*r;
                    r = z100 - A + dz;  d += r*r;
                    if (d < dmin) { dmin = d; v = v10; }
                    d = 0;
                    r = x101 - 0 + dx;  d += r*r;
                    r = y101 - B;       d += r*r;
                    r = z101 - A + dz;  d += r*r;
                    if (d < dmin) { dmin = d; v = v10; }

                    d = 0;
                    r = x110 - A + dx;  d += r*r;
                    r = y110 + B;       d += r*r;
                    r = z110 - A + dz;  d += r*r;
                    if (d < dmin) { dmin = d; v = v11; }
                    d = 0;
                    r = x111 - A + dx;  d += r*r;
                    r = y111 - B;       d += r*r;
                    r = z111 - A + dz;  d += r*r;
                    if (d < dmin) { dmin = d; v = v11; }

                    buf[j*w + i] = v;
                }
            }

            x000 = x010;
            y000 = y010;
            z000 = z010;
            x100 = x110;
            y100 = y110;
            z100 = z110;
            x001 = x011;
            y001 = y011;
            z001 = z011;
            x101 = x111;
            y101 = y111;
            z101 = z111;
            v00 = v01;
            v10 = v11;
        }
    }

    memcpy(out, buf, w*h*sizeof(*buf));
    free(buf);
    return 0;
}


int mapVoronoi114(const Layer * l, int * out, int x, int z, int w, int h)
{
    x -= 2;
    z -= 2;
    int pX = x >> 2;
    int pZ = z >> 2;
    int pW = ((x + w) >> 2) - pX + 2;
    int pH = ((z + h) >> 2) - pZ + 2;

    if (l->p)
    {
        int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
        if (err != 0)
            return err;
    }

    int newW = pW << 2;
    int newH = pH << 2;
    int *buf = (int *) malloc((newW+1)*(newH+1)*sizeof(*buf));
    int i, j;

    int64_t st = l->startSalt;
    int64_t ss = l->startSeed;
    int64_t cs;

    for (j = 0; j < pH-1; j++)
    {
        int v00 = out[(j+0)*pW];
        int v01 = out[(j+1)*pW];
        int v10, v11;

        for (i = 0; i < pW-1; i++, v00 = v10, v01 = v11)
        {
            int ii, jj;
            int *pbuf = buf + (j << 2) * newW + (i << 2);

            // try to prefetch the relevant rows to help prevent cache misses
            PREFETCH( pbuf + newW*0, 1, 1 );
            PREFETCH( pbuf + newW*1, 1, 1 );
            PREFETCH( pbuf + newW*2, 1, 1 );
            PREFETCH( pbuf + newW*3, 1, 1 );

            v10 = out[i+1 + (j+0)*pW];
            v11 = out[i+1 + (j+1)*pW];

            if (v00 == v01 && v00 == v10 && v00 == v11)
            {
                for (jj = 0; jj < 4; jj++)
                    for (ii = 0; ii < 4; ii++)
                        pbuf[ii + jj*newW] = v00;
                continue;
            }

            cs = getChunkSeed(ss, (i+pX) << 2, (j+pZ) << 2);
            int64_t da1 = (mcFirstInt(cs, 1024) - 512) * 36;
            cs = mcStepSeed(cs, st);
            int64_t da2 = (mcFirstInt(cs, 1024) - 512) * 36;

            cs = getChunkSeed(ss, (i+pX+1) << 2, (j+pZ) << 2);
            int64_t db1 = (mcFirstInt(cs, 1024) - 512) * 36 + 40*1024;
            cs = mcStepSeed(cs, st);
            int64_t db2 = (mcFirstInt(cs, 1024) - 512) * 36;

            cs = getChunkSeed(ss, (i+pX) << 2, (j+pZ+1) << 2);
            int64_t dc1 = (mcFirstInt(cs, 1024) - 512) * 36;
            cs = mcStepSeed(cs, st);
            int64_t dc2 = (mcFirstInt(cs, 1024) - 512) * 36 + 40*1024;

            cs = getChunkSeed(ss, (i+pX+1) << 2, (j+pZ+1) << 2);
            int64_t dd1 = (mcFirstInt(cs, 1024) - 512) * 36 + 40*1024;
            cs = mcStepSeed(cs, st);
            int64_t dd2 = (mcFirstInt(cs, 1024) - 512) * 36 + 40*1024;

            for (jj = 0; jj < 4; jj++)
            {
                int mj = 10240*jj;
                int64_t sja = (mj-da2) * (mj-da2);
                int64_t sjb = (mj-db2) * (mj-db2);
                int64_t sjc = (mj-dc2) * (mj-dc2);
                int64_t sjd = (mj-dd2) * (mj-dd2);
                int *p = pbuf + jj*newW;

                for (ii = 0; ii < 4; ii++)
                {
                    int mi = 10240*ii;
                    int64_t da = (mi-da1) * (mi-da1) + sja;
                    int64_t db = (mi-db1) * (mi-db1) + sjb;
                    int64_t dc = (mi-dc1) * (mi-dc1) + sjc;
                    int64_t dd = (mi-dd1) * (mi-dd1) + sjd;

                    int v;
                    if      U((da < db) && (da < dc) && (da < dd))
                        v = v00;
                    else if U((db < da) && (db < dc) && (db < dd))
                        v = v10;
                    else if U((dc < da) && (dc < db) && (dc < dd))
                        v = v01;
                    else
                        v = v11;

                    p[ii] = v;
                }
            }
        }
    }

    for (j = 0; j < h; j++)
    {
        memcpy(&out[j * w], &buf[(j + (z & 3))*newW + (x & 3)], w*sizeof(int));
    }

    free(buf);
    return 0;
}


inline static __attribute__((always_inline,const))
uint32_t rotr(uint32_t a, int b) { return (a >> b) | (a << (32-b)); }

int64_t getVoronoiSHA(int64_t seed)
{
    static const uint32_t K[64] = {
        0x428a2f98,0x71374491, 0xb5c0fbcf,0xe9b5dba5,
        0x3956c25b,0x59f111f1, 0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01, 0x243185be,0x550c7dc3,
        0x72be5d74,0x80deb1fe, 0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786, 0x0fc19dc6,0x240ca1cc,
        0x2de92c6f,0x4a7484aa, 0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d, 0xb00327c8,0xbf597fc7,
        0xc6e00bf3,0xd5a79147, 0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138, 0x4d2c6dfc,0x53380d13,
        0x650a7354,0x766a0abb, 0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b, 0xc24b8b70,0xc76c51a3,
        0xd192e819,0xd6990624, 0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08, 0x2748774c,0x34b0bcb5,
        0x391c0cb3,0x4ed8aa4a, 0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f, 0x84c87814,0x8cc70208,
        0x90befffa,0xa4506ceb, 0xbef9a3f7,0xc67178f2,
    };
    static const uint32_t B[8] = {
        0x6a09e667,0xbb67ae85, 0x3c6ef372,0xa54ff53a,
        0x510e527f,0x9b05688c, 0x1f83d9ab,0x5be0cd19,
    };

    uint32_t m[64];
    uint32_t a0,a1,a2,a3,a4,a5,a6,a7;
    uint32_t i, x, y;
    m[0] = __builtin_bswap32((uint32_t)(seed));
    m[1] = __builtin_bswap32((uint32_t)(seed >> 32));
    m[2] = 0x80000000;
    for (i = 3; i < 15; i++)
        m[i] = 0;
    m[15] = 0x00000040;

    for (i = 16; i < 64; ++i)
    {
        m[i] = m[i - 7] + m[i - 16];
        x = m[i - 15];
        m[i] += rotr(x,7) ^ rotr(x,18) ^ (x >> 3);
        x = m[i - 2];
        m[i] += rotr(x,17) ^ rotr(x,19) ^ (x >> 10);
    }

    a0 = B[0];
    a1 = B[1];
    a2 = B[2];
    a3 = B[3];
    a4 = B[4];
    a5 = B[5];
    a6 = B[6];
    a7 = B[7];

    for (i = 0; i < 64; i++)
    {
        x = a7 + K[i] + m[i];
        x += rotr(a4,6) ^ rotr(a4,11) ^ rotr(a4,25);
        x += (a4 & a5) ^ (~a4 & a6);

        y = rotr(a0,2) ^ rotr(a0,13) ^ rotr(a0,22);
        y += (a0 & a1) ^ (a0 & a2) ^ (a1 & a2);

        a7 = a6;
        a6 = a5;
        a5 = a4;
        a4 = a3 + x;
        a3 = a2;
        a2 = a1;
        a1 = a0;
        a0 = x + y;
    }

    a0 += B[0];
    a1 += B[1];

    return __builtin_bswap32(a0) | ((int64_t)__builtin_bswap32(a1) << 32);
}

void voronoiAccess3D(int64_t sha, int x, int y, int z, int *x4, int *y4, int *z4)
{
    x -= 2;
    y -= 2;
    z -= 2;
    int pX = x >> 2;
    int pY = y >> 2;
    int pZ = z >> 2;
    int dx = (x & 3) * 10240;
    int dy = (y & 3) * 10240;
    int dz = (z & 3) * 10240;
    int ax = 0, ay = 0, az = 0;
    uint64_t dmin = (uint64_t)-1;
    int i;

    for (i = 0; i < 8; i++)
    {
        int bx = (i & 4) != 0;
        int by = (i & 2) != 0;
        int bz = (i & 1) != 0;
        int cx = pX + bx;
        int cy = pY + by;
        int cz = pZ + bz;
        int rx, ry, rz;

        getVoronoiCell(sha, cx, cy, cz, &rx, &ry, &rz);

        rx += dx - 40*1024*bx;
        ry += dy - 40*1024*by;
        rz += dz - 40*1024*bz;

        uint64_t d = rx*(uint64_t)rx + ry*(uint64_t)ry + rz*(uint64_t)rz;
        if (d < dmin)
        {
            dmin = d;
            ax = cx;
            ay = cy;
            az = cz;
        }
    }

    if (x4) *x4 = ax;
    if (y4) *y4 = ay;
    if (z4) *z4 = az;
}



