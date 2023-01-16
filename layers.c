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
    if (mc >= MC_1_18)
    {
        if (id >= soul_sand_valley && id <= basalt_deltas)
            return 1;
        if (id >= small_end_islands && id <= end_barrens)
            return 1;

        if (id == deep_dark || id == mangrove_swamp)
            return mc >= MC_1_19;

        switch (id)
        {
        case ocean:
        case plains:
        case desert:
        case mountains:                 // windswept_hills
        case forest:
        case taiga:
        case swamp:
        case river:
        case nether_wastes:
        case the_end:
        case frozen_ocean:
        case frozen_river:
        case snowy_tundra:              // snowy_plains
        case mushroom_fields:
        case beach:
        case jungle:
        case jungle_edge:               // sparse_jungle
        case deep_ocean:
        case stone_shore:               // stony_shore
        case snowy_beach:
        case birch_forest:
        case dark_forest:
        case snowy_taiga:
        case giant_tree_taiga:          // old_growth_pine_taiga
        case wooded_mountains:          // windswept_forest
        case savanna:
        case savanna_plateau:
        case badlands:
        case wooded_badlands_plateau:   // wooded_badlands
        case warm_ocean:
        case lukewarm_ocean:
        case cold_ocean:
        case deep_warm_ocean:
        case deep_lukewarm_ocean:
        case deep_cold_ocean:
        case deep_frozen_ocean:
        case sunflower_plains:
        case gravelly_mountains:        // windswept_gravelly_hills
        case flower_forest:
        case ice_spikes:
        case tall_birch_forest:         // old_growth_birch_forest
        case giant_spruce_taiga:        // old_growth_spruce_taiga
        case shattered_savanna:         // windswept_savanna
        case eroded_badlands:
        case bamboo_jungle:
        case dripstone_caves:
        case lush_caves:
        case meadow:
        case grove:
        case snowy_slopes:
        case stony_peaks:
        case jagged_peaks:
        case frozen_peaks:
            return 1;
        default:
            return 0;
        }
    }

    if (mc <= MC_B1_8)
    {
        switch (id)
        {
        case frozen_ocean:
        case frozen_river:
        case snowy_tundra:
        case mushroom_fields:
        case mushroom_field_shore:
        case the_end:
            return 0;
        }
    }
    if (mc <= MC_1_0)
    {
        switch (id)
        {
        case snowy_mountains:
        case beach:
        case desert_hills:
        case wooded_hills:
        case taiga_hills:
        case mountain_edge:
            return 0;
        }
    }

    if (id >= ocean             && id <= mountain_edge)     return 1;
    if (id >= jungle            && id <= jungle_hills)      return mc >= MC_1_2;
    if (id >= jungle_edge       && id <= badlands_plateau)  return mc >= MC_1_7;
    if (id >= small_end_islands && id <= end_barrens)       return mc >= MC_1_9;
    if (id >= warm_ocean        && id <= deep_frozen_ocean) return mc >= MC_1_13;

    switch (id)
    {
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
    case soul_sand_valley:
    case crimson_forest:
    case warped_forest:
    case basalt_deltas:
        return mc >= MC_1_16_1;
    case dripstone_caves:
    case lush_caves:
        return mc >= MC_1_17;
    default:
        return 0;
    }
}

int isOverworld(int mc, int id)
{
    if (!biomeExists(mc, id))
        return 0;

    if (id >= small_end_islands && id <= end_barrens) return 0;
    if (id >= soul_sand_valley && id <= basalt_deltas) return 0;

    switch (id)
    {
    case nether_wastes:
    case the_end:
        return 0;
    case frozen_ocean:
        return mc <= MC_1_6 || mc >= MC_1_13;
    case mountain_edge:
        return mc <= MC_1_6;
    case deep_warm_ocean:
    case the_void:
        return 0;
    case tall_birch_hills:
        return mc <= MC_1_8 || mc >= MC_1_11;
    case dripstone_caves:
    case lush_caves:
        return mc >= MC_1_18;
    }
    return 1;
}

int getDimension(int id)
{
    if (id >= small_end_islands && id <= end_barrens) return DIM_END;
    if (id >= soul_sand_valley && id <= basalt_deltas) return DIM_NETHER;
    if (id == the_end) return DIM_END;
    if (id == nether_wastes) return DIM_NETHER;
    return DIM_OVERWORLD;
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

    case nether_wastes:
    case soul_sand_valley:
    case crimson_forest:
    case warped_forest:
    case basalt_deltas:
        return nether_wastes;

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
    return (uint32_t) id < 64 && ((1ULL << id) & shallow_bits);
}

int isDeepOcean(int id)
{
    const uint64_t deep_bits =
            (1ULL << deep_ocean) |
            (1ULL << deep_warm_ocean) |
            (1ULL << deep_lukewarm_ocean) |
            (1ULL << deep_cold_ocean) |
            (1ULL << deep_frozen_ocean);
    return (uint32_t) id < 64 && ((1ULL << id) & deep_bits);
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
    return (uint32_t) id < 64 && ((1ULL << id) & ocean_bits);
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

void setLayerSeed(Layer *layer, uint64_t worldSeed)
{
    if (layer->p2 != NULL)
        setLayerSeed(layer->p2, worldSeed);

    if (layer->p != NULL)
        setLayerSeed(layer->p, worldSeed);

    if (layer->noise != NULL)
    {
        uint64_t s;
        setSeed(&s, worldSeed);
        perlinInit((PerlinNoise*)layer->noise, &s);
    }

    uint64_t ls = layer->layerSalt;
    if (ls == 0)
    {   // Pre 1.13 the Hills branch stays zero-initialized
        layer->startSalt = 0;
        layer->startSeed = 0;
    }
    else if (ls == LAYER_INIT_SHA)
    {   // Post 1.14 Voronoi uses SHA256 for initialization
        layer->startSalt = getVoronoiSHA(worldSeed);
        layer->startSeed = 0;
    }
    else
    {
        uint64_t st = worldSeed;
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


void initSurfaceNoise(SurfaceNoise *sn, int dim, uint64_t seed)
{
    uint64_t s;
    setSeed(&s, seed);
    octaveInit(&sn->octmin, &s, sn->oct+0, -15, 16);
    octaveInit(&sn->octmax, &s, sn->oct+16, -15, 16);
    octaveInit(&sn->octmain, &s, sn->oct+32, -7, 8);
    if (dim == DIM_END)
    {
        sn->xzScale = 2.0;
        sn->yScale = 1.0;
        sn->xzFactor = 80;
        sn->yFactor = 160;
    }
    else // DIM_OVERWORLD
    {
        octaveInit(&sn->octsurf, &s, sn->oct+40, -3, 4);
        skipNextN(&s, 262*10);
        octaveInit(&sn->octdepth, &s, sn->oct+44, -15, 16);
        sn->xzScale = 0.9999999814507745;
        sn->yScale = 0.9999999814507745;
        sn->xzFactor = 80;
        sn->yFactor = 160;
    }
}

double sampleSurfaceNoise(const SurfaceNoise *sn, int x, int y, int z)
{
    double xzScale = 684.412 * sn->xzScale;
    double yScale = 684.412 * sn->yScale;
    double xzStep = xzScale / sn->xzFactor;
    double yStep = yScale / sn->yFactor;

    double minNoise = 0;
    double maxNoise = 0;
    double mainNoise = 0;
    double persist = 1.0;
    double dx, dy, dz, sy, ty;
    int i;

    for (i = 0; i < 16; i++)
    {
        dx = maintainPrecision(x * xzScale * persist);
        dy = maintainPrecision(y * yScale  * persist);
        dz = maintainPrecision(z * xzScale * persist);
        sy = yScale * persist;
        ty = y * sy;

        minNoise += samplePerlin(&sn->octmin.octaves[i], dx, dy, dz, sy, ty) / persist;
        maxNoise += samplePerlin(&sn->octmax.octaves[i], dx, dy, dz, sy, ty) / persist;

        if (i < 8)
        {
            dx = maintainPrecision(x * xzStep * persist);
            dy = maintainPrecision(y * yStep  * persist);
            dz = maintainPrecision(z * xzStep * persist);
            sy = yStep * persist;
            ty = y * sy;
            mainNoise += samplePerlin(&sn->octmain.octaves[i], dx, dy, dz, sy, ty) / persist;
        }
        persist /= 2.0;
    }

    return clampedLerp(0.5 + 0.05*mainNoise, minNoise/512.0, maxNoise/512.0);
}


//==============================================================================
// Nether (1.16+) and End (1.9+) Biome Generation
//==============================================================================

void setNetherSeed(NetherNoise *nn, uint64_t seed)
{
    uint64_t s;
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

    y = 0;
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


static void fillRad3D(int *out, int x, int y, int z, int sx, int sy, int sz,
    int id, float rad)
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
        if (ak < 0 || ak >= sy)
            continue;
        int ksq = k*k;
        int *yout = &out[(sx*sz)*ak];

        for (j = -r; j <= r; j++)
        {
            int aj = z+j;
            if (aj < 0 || aj >= sz)
                continue;
            int jksq = j*j + ksq;
            for (i = -r; i <= r; i++)
            {
                int ai = x+i;
                if (ai < 0 || ai >= sx)
                    continue;
                int ijksq = i*i + jksq;
                if (ijksq > rsq)
                    continue;

                yout[aj*sx+ai] = id;
            }
        }
    }
}

int mapNether3D(const NetherNoise *nn, int *out, Range r, float confidence)
{
    int i, j, k;
    if (r.sy <= 0)
        r.sy = 1;
    if (r.scale <= 3)
    {
        printf("mapNether3D() invalid scale for this function\n");
        return 1;
    }
    int scale = r.scale / 4;

    memset(out, 0, sizeof(int) * r.sx*r.sy*r.sz);

    // The noisedelta is the distance between the first and second closest
    // biomes within the noise space. Dividing this by the greatest possible
    // gradient (~0.05) gives a minimum diameter of voxels around the sample
    // cell that will have the same biome.
    float invgrad = 1.0 / (confidence * 0.05 * 2) / scale;

    for (k = 0; k < r.sy; k++)
    {
        int *yout = &out[(r.sx*r.sz)*k];

        for (j = 0; j < r.sz; j++)
        {
            for (i = 0; i < r.sx; i++)
            {
                if (yout[j*r.sx+i])
                    continue;
                //yout[j*w+i] = getNetherBiome(nn, x+i, y+k, z+j, NULL);
                //continue;

                float noisedelta;
                int xi = (r.x+i)*scale;
                int yk = (r.y+k);
                int zj = (r.z+j)*scale;
                int v = getNetherBiome(nn, xi, yk, zj, &noisedelta);
                yout[j*r.sx+i] = v;
                float cellrad = noisedelta * invgrad;
                fillRad3D(out, i, j, k, r.sx, r.sy, r.sz, v, cellrad);
            }
        }
    }
    return 0;
}

int mapNether2D(const NetherNoise *nn, int *out, int x, int z, int w, int h)
{
    Range r = {4, x, z, w, h, 0, 1};
    return mapNether3D(nn, out, r, 1.0);
}

int genNetherScaled(const NetherNoise *nn, int *out, Range r, int mc, uint64_t sha)
{
    if (r.scale <= 0) r.scale = 4;
    if (r.sy == 0) r.sy = 1;

    uint64_t siz = (uint64_t)r.sx*r.sy*r.sz;

    if (mc <= MC_1_15)
    {
        uint64_t i;
        for (i = 0; i < siz; i++)
            out[i] = nether_wastes;
        return 0;
    }

    if (r.scale == 1)
    {
        Range s = getVoronoiSrcRange(r);
        int *src;
        if (siz > 1)
        {   // the source range is large enough that we can try optimizing
            src = out + siz;
            int err = mapNether3D(nn, src, s, 1.0);
            if (err)
                return err;
        }
        else
        {
            src = NULL;
        }

        int i, j, k;
        int *p = out;
        for (k = 0; k < r.sy; k++)
        {
            for (j = 0; j < r.sz; j++)
            {
                for (i = 0; i < r.sx; i++)
                {
                    int x4, z4, y4;
                    voronoiAccess3D(sha, r.x+i, r.y+k, r.z+j, &x4, &y4, &z4);
                    if (src)
                    {
                        x4 -= s.x; y4 -= s.y; z4 -= s.z;
                        *p = src[y4*s.sx*s.sz + z4*s.sx + x4];
                    }
                    else
                    {
                        *p = getNetherBiome(nn, x4, y4, z4, NULL);
                    }
                    p++;
                }
            }
        }
        return 0;
    }
    else
    {
        return mapNether3D(nn, out, r, 1.0);
    }
}


void setEndSeed(EndNoise *en, int mc, uint64_t seed)
{
    uint64_t s;
    setSeed(&s, seed);
    skipNextN(&s, 17292);
    perlinInit(&en->perlin, &s);
    en->mc = mc;
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

    if (abs(hx) <= 15 && abs(hz) <= 15)
        h = 64 * (hx*hx + hz*hz);
    else
        h = 14401;

    for (j = 0; j < 25; j++)
    {
        uint16_t dsj = p_dsj[j];
        uint16_t e;
        uint32_t u;

        // force unroll for(i=0;i<25;i++) in a cross compatible way
        #define x5(i,x)    { x; i++; x; i++; x; i++; x; i++; x; i++; }
        #define for25(i,x) { i = 0; x5(i,x) x5(i,x) x5(i,x) x5(i,x) x5(i,x) }
        for25(i,
            if unlikely(e = p_elev[i])
            {
                if ((u = (p_dsi[i] + (uint32_t)dsj) * e) < h)
                    h = u;
            }
        );
        #undef for25
        #undef x5
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
            uint64_t rsq = rx * rx + rz * rz;
            uint16_t v = 0;
            if (rsq > 4096 && sampleSimplex2D(&en->perlin, rx, rz) < -0.9f)
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
            uint64_t rsq = hx * hx + hz * hz;

            if (rsq <= 4096L)
                out[j*w+i] = the_end;
            else
            {
                hx = 2*hx + 1;
                hz = 2*hz + 1;
                if (en->mc >= MC_1_14)
                {
                    rsq = hx * hx + hz * hz;
                    if ((int)rsq < 0)
                    {
                        out[j*w+i] = small_end_islands;
                        continue;
                    }
                }
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

/* Samples the End height. The coordinates used here represent eight blocks per
 * cell. By default a range of 12 cells is sampled, which can be overriden for
 * optimization purposes.
 */
float getEndHeightNoise(const EndNoise *en, int x, int z, int range)
{
    int hx = x / 2;
    int hz = z / 2;
    int oddx = x % 2;
    int oddz = z % 2;
    int i, j;

    int64_t h = 64 * (x*(int64_t)x + z*(int64_t)z);
    if (range == 0)
        range = 12;

    for (j = -range; j <= range; j++)
    {
        for (i = -range; i <= range; i++)
        {
            int64_t rx = hx + i;
            int64_t rz = hz + j;
            uint64_t rsq = rx*rx + rz*rz;
            uint16_t v = 0;
            if (rsq > 4096 && sampleSimplex2D(&en->perlin, rx, rz) < -0.9f)
            {
                v = (llabs(rx) * 3439 + llabs(rz) * 147) % 13 + 9;
                rx = (oddx - i * 2);
                rz = (oddz - j * 2);
                rsq = rx*rx + rz*rz;
                int64_t noise = rsq * v*v;
                if (noise < h)
                    h = noise;
            }
        }
    }

    float ret = 100 - sqrtf((float) h);
    if (ret < -100) ret = -100;
    if (ret > 80) ret = 80;
    return ret;
}

void sampleNoiseColumnEnd(double column[], const SurfaceNoise *sn,
        const EndNoise *en, int x, int z, int colymin, int colymax)
{
    double depth = getEndHeightNoise(en, x, z, 0) - 8.0f;
    int y;
    for (y = colymin; y <= colymax; y++)
    {
        double noise = sampleSurfaceNoise(sn, x, y, z);
        noise += depth; // falloff for the End is just the depth
        // clamp top and bottom slides from End settings
        noise = clampedLerp((32 + 46 - y) / 64.0, -3000, noise);
        noise = clampedLerp((y - 1) / 7.0, -30, noise);
        column[y - colymin] = noise;
    }
}

/* Given bordering noise columns and a fractional position between those,
 * determine the surface block height (i.e. where the interpolated noise > 0).
 * Note that the noise columns should be of size: ncolxz[ colymax-colymin+1 ]
 */
int getSurfaceHeight(
        const double ncol00[], const double ncol01[],
        const double ncol10[], const double ncol11[],
        int colymin, int colymax, int blockspercell, double dx, double dz)
{
    int y, celly;
    for (celly = colymax-1; celly >= colymin; celly--)
    {
        int idx = celly - colymin;
        double v000 = ncol00[idx];
        double v001 = ncol01[idx];
        double v100 = ncol10[idx];
        double v101 = ncol11[idx];
        double v010 = ncol00[idx+1];
        double v011 = ncol01[idx+1];
        double v110 = ncol10[idx+1];
        double v111 = ncol11[idx+1];

        for (y = blockspercell - 1; y >= 0; y--)
        {
            double dy = y / (double) blockspercell;
            double noise = lerp3(dy, dx, dz, // Note: not x, y, z
                v000, v010, v100, v110,
                v001, v011, v101, v111);
            if (noise > 0)
                return celly * blockspercell + y;
        }
    }
    return 0;
}

int getSurfaceHeightEnd(int mc, uint64_t seed, int x, int z)
{
    EndNoise en;
    setEndSeed(&en, mc, seed);

    SurfaceNoise sn;
    initSurfaceNoise(&sn, DIM_END, seed);

    // end noise columns vary on a grid of cell size = eight
    int cellx = (x >> 3);
    int cellz = (z >> 3);
    double dx = (x & 7) / 8.0;
    double dz = (z & 7) / 8.0;

    // abusing enum for local compile time constants rather than enumeration
    enum { y0 = 0, y1 = 32, yn = y1-y0+1 };
    double ncol00[yn];
    double ncol01[yn];
    double ncol10[yn];
    double ncol11[yn];
    sampleNoiseColumnEnd(ncol00, &sn, &en, cellx, cellz, y0, y1);
    sampleNoiseColumnEnd(ncol01, &sn, &en, cellx, cellz+1, y0, y1);
    sampleNoiseColumnEnd(ncol10, &sn, &en, cellx+1, cellz, y0, y1);
    sampleNoiseColumnEnd(ncol11, &sn, &en, cellx+1, cellz+1, y0, y1);

    return getSurfaceHeight(ncol00, ncol01, ncol10, ncol11, y0, y1, 4, dx, dz);
}

int genEndScaled(const EndNoise *en, int *out, Range r, int mc, uint64_t sha)
{
    if (r.sy == 0)
        r.sy = 1;

    if (mc <= MC_1_8)
    {
        uint64_t i, siz = (uint64_t)r.sx*r.sy*r.sz;
        for (i = 0; i < siz; i++)
            out[i] = the_end;
        return 0;
    }

    int err, iy;

    if (r.scale == 1)
    {
        Range s = getVoronoiSrcRange(r);
        err = mapEnd(en, out, s.x, s.z, s.sx, s.sz);
        if (err) return err;

        if (mc <= MC_1_14)
        {   // up to 1.14 voronoi noise is planar
            Layer lvoronoi;
            memset(&lvoronoi, 0, sizeof(Layer));
            lvoronoi.startSalt = getLayerSalt(10);
            err = mapVoronoi114(&lvoronoi, out, r.x, r.z, r.sx, r.sz);
            if (err) return err;
        }
        else
        {   // in 1.15 voronoi noise varies vertically in the End
            int *src = out + r.sx*r.sy*r.sz;
            memmove(src, out, s.sx*s.sz*sizeof(int));
            for (iy = 0; iy < r.sy; iy++)
            {
                mapVoronoiPlane(
                    sha, out+r.sx*r.sz*iy, src,
                    r.x,r.z,r.sx,r.sz, r.y+iy,
                    s.x,s.z,s.sx,s.sz);
            }
            return 0; // 3D expansion is done => return
        }
    }
    else if (r.scale == 4)
    {
        err = mapEnd(en, out, r.x, r.z, r.sx, r.sz);
        if (err) return err;
    }
    else if (r.scale == 16)
    {
        err = mapEndBiome(en, out, r.x, r.z, r.sx, r.sz);
        if (err) return err;
    }
    else
    {
        float d = r.scale / 8.0;
        int i, j;
        for (j = 0; j < r.sz; j++)
        {
            for (i = 0; i < r.sx; i++)
            {
                int64_t hx = (int64_t)((i+r.x) * d);
                int64_t hz = (int64_t)((j+r.z) * d);
                uint64_t rsq = hx*hx + hz*hz;
                if (rsq <= 16384L)
                {
                    out[j*r.sx+i] = the_end;
                    continue;
                }
                else if (mc >= MC_1_14 && (int)(rsq) < 0)
                {
                    out[j*r.sx+i] = small_end_islands;
                    continue;
                }
                float h = getEndHeightNoise(en, hx, hz, 4);
                if (h > 40)
                    out[j*r.sx+i] = end_highlands;
                else if (h >= 0)
                    out[j*r.sx+i] = end_midlands;
                else if (h >= -20)
                    out[j*r.sx+i] = end_barrens;
                else
                    out[j*r.sx+i] = small_end_islands;
            }
        }
    }

    // expanding 2D into 3D
    for (iy = 1; iy < r.sy; iy++)
    {
        int i, siz = r.sx*r.sz;
        for (i = 0; i < siz; i++)
            out[iy*siz + i] = out[i];
    }

    return 0;
}


//==============================================================================
// Overworld and Nether Biome Generation 1.18
//==============================================================================

static int init_climate_seed(
    DoublePerlinNoise *dpn, PerlinNoise *oct,
    uint64_t xlo, uint64_t xhi, int large, int nptype
    )
{
    Xoroshiro pxr;
    int n = 0;

    switch (nptype)
    {
    case NP_SHIFT: {
        double amp_s[] = {1, 1, 1, 0};
        int len = sizeof(amp_s)/sizeof(double);
        // md5 "minecraft:offset"
        pxr.lo = xlo ^ 0x080518cf6af25384;
        pxr.hi = xhi ^ 0x3f3dfb40a54febd5;
        n += xDoublePerlinInit(dpn, &pxr, oct, amp_s, -3, len);
        } break;

    case NP_TEMPERATURE: {
        double amp_t[] = {1.5, 0, 1, 0, 0, 0};
        int len = sizeof(amp_t)/sizeof(double);
        // md5 "minecraft:temperature" or "minecraft:temperature_large"
        pxr.lo = xlo ^ (large ? 0x944b0073edf549db : 0x5c7e6b29735f0d7f);
        pxr.hi = xhi ^ (large ? 0x4ff44347e9d22b96 : 0xf7d86f1bbc734988);
        n += xDoublePerlinInit(dpn, &pxr, oct, amp_t, large ? -12 : -10, len);
        } break;

    case NP_HUMIDITY: {
        double amp_h[] = {1, 1, 0, 0, 0, 0};
        int len = sizeof(amp_h)/sizeof(double);
        // md5 "minecraft:vegetation" or "minecraft:vegetation_large"
        pxr.lo = xlo ^ (large ? 0x71b8ab943dbd5301 : 0x81bb4d22e8dc168e);
        pxr.hi = xhi ^ (large ? 0xbb63ddcf39ff7a2b : 0xf1c8b4bea16303cd);
        n += xDoublePerlinInit(dpn, &pxr, oct, amp_h, large ? -10 : -8, len);
        } break;

    case NP_CONTINENTALNESS: {
        double amp_c[] = {1, 1, 2, 2, 2, 1, 1, 1, 1};
        int len = sizeof(amp_c)/sizeof(double);
        // md5 "minecraft:continentalness" or "minecraft:continentalness_large"
        pxr.lo = xlo ^ (large ? 0x9a3f51a113fce8dc : 0x83886c9d0ae3a662);
        pxr.hi = xhi ^ (large ? 0xee2dbd157e5dcdad : 0xafa638a61b42e8ad);
        n += xDoublePerlinInit(dpn, &pxr, oct, amp_c, large ? -11 : -9, len);
        } break;

    case NP_EROSION: {
        double amp_e[] = {1, 1, 0, 1, 1};
        int len = sizeof(amp_e)/sizeof(double);
        // md5 "minecraft:erosion" or "minecraft:erosion_large"
        pxr.lo = xlo ^ (large ? 0x8c984b1f8702a951 : 0xd02491e6058f6fd8);
        pxr.hi = xhi ^ (large ? 0xead7b1f92bae535f : 0x4792512c94c17a80);
        n += xDoublePerlinInit(dpn, &pxr, oct, amp_e, large ? -11 : -9, len);
        } break;

    case NP_WEIRDNESS: {
        double amp_w[] = {1, 2, 1, 0, 0, 0};
        int len = sizeof(amp_w)/sizeof(double);
        // md5 "minecraft:ridge"
        pxr.lo = xlo ^ 0xefc8ef4d36102b34;
        pxr.hi = xhi ^ 0x1beeeb324a0f24ea;
        n += xDoublePerlinInit(dpn, &pxr, oct, amp_w, -7, len);
        } break;

    default:
        printf("unsupported climate parameter %d\n", nptype);
        exit(1);
    }
    return n;
}

void setBiomeSeed(BiomeNoise *bn, uint64_t seed, int large)
{
    Xoroshiro pxr;
    xSetSeed(&pxr, seed);
    uint64_t xlo = xNextLong(&pxr);
    uint64_t xhi = xNextLong(&pxr);

    int n = 0, i = 0;
    for (; i < NP_MAX; i++)
        n += init_climate_seed(&bn->climate[i], bn->oct+n, xlo, xhi, large, i);

    if ((size_t)n > sizeof(bn->oct) / sizeof(*bn->oct))
    {
        printf("setBiomeSeed(): BiomeNoise is malformed, buffer too small\n");
        exit(1);
    }
    bn->nptype = -1;
}


enum { CONTINENTALNESS, EROSION, RIDGES, WEIRDNESS };

static void addSplineVal(Spline *rsp, float loc, Spline *val, float der)
{
    rsp->loc[rsp->len] = loc;
    rsp->val[rsp->len] = val;
    rsp->der[rsp->len] = der;
    rsp->len++;
    //if (rsp->len > 12) {
    //    printf("addSplineVal(): too many spline points\n");
    //    exit(1);
    //}
}

static Spline *createFixSpline(SplineStack *ss, float val)
{
    FixSpline *sp = &ss->fstack[ss->flen++];
    sp->len = 1;
    sp->val = val;
    return (Spline*)sp;
}

static float getOffsetValue(float weirdness, float continentalness)
{
    float f0 = 1.0F - (1.0F - continentalness) * 0.5F;
    float f1 = 0.5F * (1.0F - continentalness);
    float f2 = (weirdness + 1.17F) * 0.46082947F;
    float off = f2 * f0 - f1;
    if (weirdness < -0.7F)
        return off > -0.2222F ? off : -0.2222F;
    else
        return off > 0 ? off : 0;
}

static Spline *createSpline_38219(SplineStack *ss, float f, int bl)
{
    Spline *sp = &ss->stack[ss->len++];
    sp->typ = RIDGES;

    float i = getOffsetValue(-1.0F, f);
    float k = getOffsetValue( 1.0F, f);
    float l = 1.0F - (1.0F - f) * 0.5F;
    float u = 0.5F * (1.0F - f);
    l = u / (0.46082947F * l) - 1.17F;

    if (-0.65F < l && l < 1.0F)
    {
        float p, q, r, s;
        u = getOffsetValue(-0.65F, f);
        p = getOffsetValue(-0.75F, f);
        q = (p - i) * 4.0F;
        r = getOffsetValue(l, f);
        s = (k - r) / (1.0F - l);

        addSplineVal(sp, -1.0F,     createFixSpline(ss, i), q);
        addSplineVal(sp, -0.75F,    createFixSpline(ss, p), 0);
        addSplineVal(sp, -0.65F,    createFixSpline(ss, u), 0);
        addSplineVal(sp, l-0.01F,   createFixSpline(ss, r), 0);
        addSplineVal(sp, l,         createFixSpline(ss, r), s);
        addSplineVal(sp, 1.0F,      createFixSpline(ss, k), s);
    }
    else
    {
        u = (k - i) * 0.5F;
        if (bl) {
            addSplineVal(sp, -1.0F, createFixSpline(ss, i > 0.2 ? i : 0.2), 0);
            addSplineVal(sp,  0.0F, createFixSpline(ss, lerp(0.5F, i, k)), u);
        } else {
            addSplineVal(sp, -1.0F, createFixSpline(ss, i), u);
        }
        addSplineVal(sp, 1.0F,      createFixSpline(ss, k), u);
    }
    return sp;
}

static Spline *createFlatOffsetSpline(
    SplineStack *ss, float f, float g, float h, float i, float j, float k)
{
    Spline *sp = &ss->stack[ss->len++];
    sp->typ = RIDGES;

    float l = 0.5F * (g - f); if (l < k) l = k;
    float m = 5.0F * (h - g);

    addSplineVal(sp, -1.0F, createFixSpline(ss, f), l);
    addSplineVal(sp, -0.4F, createFixSpline(ss, g), l < m ? l : m);
    addSplineVal(sp,  0.0F, createFixSpline(ss, h), m);
    addSplineVal(sp,  0.4F, createFixSpline(ss, i), 2.0F*(i-h));
    addSplineVal(sp,  1.0F, createFixSpline(ss, j), 0.7F*(j-i));

    return sp;
}

static Spline *createLandSpline(
    SplineStack *ss, float f, float g, float h, float i, float j, float k, int bl)
{
    Spline *sp1 = createSpline_38219(ss, lerp(i, 0.6F, 1.5F), bl);
    Spline *sp2 = createSpline_38219(ss, lerp(i, 0.6F, 1.0F), bl);
    Spline *sp3 = createSpline_38219(ss, i, bl);
    const float ih = 0.5F * i;
    Spline *sp4 = createFlatOffsetSpline(ss, f-0.15F, ih, ih, ih, i*0.6F, 0.5F);
    Spline *sp5 = createFlatOffsetSpline(ss, f, j*i, g*i, ih, i*0.6F, 0.5F);
    Spline *sp6 = createFlatOffsetSpline(ss, f, j, j, g, h, 0.5F);
    Spline *sp7 = createFlatOffsetSpline(ss, f, j, j, g, h, 0.5F);

    Spline *sp8 = &ss->stack[ss->len++];
    sp8->typ = RIDGES;
    addSplineVal(sp8, -1.0F, createFixSpline(ss, f), 0.0F);
    addSplineVal(sp8, -0.4F, sp6, 0.0F);
    addSplineVal(sp8,  0.0F, createFixSpline(ss, h + 0.07F), 0.0F);

    Spline *sp9 = createFlatOffsetSpline(ss, -0.02F, k, k, g, h, 0.0F);
    Spline *sp = &ss->stack[ss->len++];
    sp->typ = EROSION;
    addSplineVal(sp, -0.85F, sp1, 0.0F);
    addSplineVal(sp, -0.7F,  sp2, 0.0F);
    addSplineVal(sp, -0.4F,  sp3, 0.0F);
    addSplineVal(sp, -0.35F, sp4, 0.0F);
    addSplineVal(sp, -0.1F,  sp5, 0.0F);
    addSplineVal(sp,  0.2F,  sp6, 0.0F);
    if (bl) {
        addSplineVal(sp, 0.4F,  sp7, 0.0F);
        addSplineVal(sp, 0.45F, sp8, 0.0F);
        addSplineVal(sp, 0.55F, sp8, 0.0F);
        addSplineVal(sp, 0.58F, sp7, 0.0F);
    }
    addSplineVal(sp, 0.7F, sp9, 0.0F);
    return sp;
}

float getSpline(const Spline *sp, const float *vals)
{
    if (!sp || sp->len <= 0 || sp->len >= 12)
    {
        printf("getSpline(): bad parameters\n");
        exit(1);
    }

    if (sp->len == 1)
        return ((FixSpline*)sp)->val;

    float f = vals[sp->typ];
    int i;

    for (i = 0; i < sp->len; i++)
        if (sp->loc[i] >= f)
            break;
    if (i == 0 || i == sp->len)
    {
        if (i) i--;
        float v = getSpline(sp->val[i], vals);
        return v + sp->der[i] * (f - sp->loc[i]);
    }
    const Spline *sp1 = sp->val[i-1];
    const Spline *sp2 = sp->val[i];
    float g = sp->loc[i-1];
    float h = sp->loc[i];
    float k = (f - g) / (h - g);
    float l = sp->der[i-1];
    float m = sp->der[i];
    float n = getSpline(sp1, vals);
    float o = getSpline(sp2, vals);
    float p = l * (h - g) - (o - n);
    float q = -m * (h - g) + (o - n);
    float r = lerp(k, n, o) + k * (1.0F - k) * lerp(k, p, q);
    return r;
}

void initBiomeNoise(BiomeNoise *bn, int mc)
{
    SplineStack *ss = &bn->ss;
    memset(ss, 0, sizeof(*ss));
    Spline *sp = &ss->stack[ss->len++];
    sp->typ = CONTINENTALNESS;

    Spline *sp1 = createLandSpline(ss, -0.15F, 0.00F, 0.0F, 0.1F, 0.00F, -0.03F, 0);
    Spline *sp2 = createLandSpline(ss, -0.10F, 0.03F, 0.1F, 0.1F, 0.01F, -0.03F, 0);
    Spline *sp3 = createLandSpline(ss, -0.10F, 0.03F, 0.1F, 0.7F, 0.01F, -0.03F, 1);
    Spline *sp4 = createLandSpline(ss, -0.05F, 0.03F, 0.1F, 1.0F, 0.01F,  0.01F, 1);

    addSplineVal(sp, -1.10F, createFixSpline(ss,  0.044F), 0.0F);
    addSplineVal(sp, -1.02F, createFixSpline(ss, -0.2222F), 0.0F);
    addSplineVal(sp, -0.51F, createFixSpline(ss, -0.2222F), 0.0F);
    addSplineVal(sp, -0.44F, createFixSpline(ss, -0.12F), 0.0F);
    addSplineVal(sp, -0.18F, createFixSpline(ss, -0.12F), 0.0F);
    addSplineVal(sp, -0.16F, sp1, 0.0F);
    addSplineVal(sp, -0.15F, sp1, 0.0F);
    addSplineVal(sp, -0.10F, sp2, 0.0F);
    addSplineVal(sp,  0.25F, sp3, 0.0F);
    addSplineVal(sp,  1.00F, sp4, 0.0F);

    bn->sp = sp;
    bn->mc = mc;
}


/// Biome sampler for MC 1.18
int sampleBiomeNoise(const BiomeNoise *bn, int64_t *np, int x, int y, int z,
    uint64_t *dat, uint32_t sample_flags)
{
    if (bn->nptype >= 0)
    {   // initialized for a specific climate parameter
        if (np)
            memset(np, 0, NP_MAX*sizeof(*np));
        int64_t id = (int64_t) (10000.0 * sampleClimatePara(bn, np, x, z));
        return (int) id;
    }

    float t = 0, h = 0, c = 0, e = 0, d = 0, w = 0;
    double px = x, pz = z;
    if (!(sample_flags & SAMPLE_NO_SHIFT))
    {
        px += sampleDoublePerlin(&bn->climate[NP_SHIFT], x, 0, z) * 4.0;
        pz += sampleDoublePerlin(&bn->climate[NP_SHIFT], z, x, 0) * 4.0;
    }

    c = sampleDoublePerlin(&bn->climate[NP_CONTINENTALNESS], px, 0, pz);
    e = sampleDoublePerlin(&bn->climate[NP_EROSION], px, 0, pz);
    w = sampleDoublePerlin(&bn->climate[NP_WEIRDNESS], px, 0, pz);

    if (!(sample_flags & SAMPLE_NO_DEPTH))
    {
        float np_param[] = {
            c, e, -3.0F * ( fabsf( fabsf(w) - 0.6666667F ) - 0.33333334F ), w,
        };
        double off = getSpline(bn->sp, np_param) + 0.015F;

        //double py = y + sampleDoublePerlin(&bn->shift, y, z, x) * 4.0;
        d = 1.0 - (y << 2) / 128.0 - 83.0/160.0 + off;
    }

    t = sampleDoublePerlin(&bn->climate[NP_TEMPERATURE], px, 0, pz);
    h = sampleDoublePerlin(&bn->climate[NP_HUMIDITY], px, 0, pz);

    int64_t l_np[6];
    int64_t *p_np = np ? np : l_np;
    p_np[0] = (int64_t)(10000.0F*t);
    p_np[1] = (int64_t)(10000.0F*h);
    p_np[2] = (int64_t)(10000.0F*c);
    p_np[3] = (int64_t)(10000.0F*e);
    p_np[4] = (int64_t)(10000.0F*d);
    p_np[5] = (int64_t)(10000.0F*w);

    int id = none;
    if (!(sample_flags & SAMPLE_NO_BIOME))
        id = p2overworld(bn->mc, (const uint64_t*)p_np, dat);
    return id;
}

void setClimateParaSeed(BiomeNoise *bn, uint64_t seed, int large, int nptype)
{
    Xoroshiro pxr;
    xSetSeed(&pxr, seed);
    uint64_t xlo = xNextLong(&pxr);
    uint64_t xhi = xNextLong(&pxr);
    if (nptype == NP_DEPTH)
    {
        int n = 0;
        n += init_climate_seed(bn->climate + NP_CONTINENTALNESS,
            bn->oct + n, xlo, xhi, large,    NP_CONTINENTALNESS);
        n += init_climate_seed(bn->climate + NP_EROSION,
            bn->oct + n, xlo, xhi, large,    NP_EROSION);
        n += init_climate_seed(bn->climate + NP_WEIRDNESS,
            bn->oct + n, xlo, xhi, large,    NP_WEIRDNESS);
    }
    else
    {
        init_climate_seed(bn->climate + nptype, bn->oct, xlo, xhi, large, nptype);
    }
    bn->nptype = nptype;
}

double sampleClimatePara(const BiomeNoise *bn, int64_t *np, double x, double z)
{
    if (bn->nptype == NP_DEPTH)
    {
        float c, e, w;
        c = sampleDoublePerlin(bn->climate + NP_CONTINENTALNESS, x, 0, z);
        e = sampleDoublePerlin(bn->climate + NP_EROSION, x, 0, z);
        w = sampleDoublePerlin(bn->climate + NP_WEIRDNESS, x, 0, z);

        float np_param[] = {
            c, e, -3.0F * ( fabsf( fabsf(w) - 0.6666667F ) - 0.33333334F ), w,
        };
        double off = getSpline(bn->sp, np_param) + 0.015F;
        int y = 0;
        float d = 1.0 - (y << 2) / 128.0 - 83.0/160.0 + off;
        if (np)
        {
            np[2] = (int64_t)(10000.0F*c);
            np[3] = (int64_t)(10000.0F*e);
            np[4] = (int64_t)(10000.0F*d);
            np[5] = (int64_t)(10000.0F*w);
        }
        return d;
    }
    double p = sampleDoublePerlin(bn->climate + bn->nptype, x, 0, z);
    if (np)
        np[bn->nptype] = (int64_t)(10000.0F*p);
    return p;
}

void genBiomeNoiseChunkSection(const BiomeNoise *bn, int out[4][4][4],
    int cx, int cy, int cz, uint64_t *dat)
{
    uint64_t buf = 0;
    int i, j, k;
    int x4 = cx << 2, y4 = cy << 2, z4 = cz << 2;
    if (dat == NULL)
        dat = &buf;
    if (*dat == 0)
    {   // try to determine the ending point of the last chunk section
        sampleBiomeNoise(bn, NULL, x4+3, y4-1, z4+3, dat, 0);
    }

    // iteration order is important
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            for (k = 0; k < 4; ++k) {
                out[i][j][k] = sampleBiomeNoise(bn, NULL, x4+i, y4+j, z4+k, dat, 0);
            }
        }
    }
}

static void genBiomeNoise3D(const BiomeNoise *bn, int *out, Range r, int opt)
{
    uint64_t dat = 0;
    uint64_t *p_dat = opt ? &dat : NULL;
    uint32_t flags = opt ? SAMPLE_NO_SHIFT : 0;
    int i, j, k;
    int *p = out;
    int scale = r.scale > 4 ? r.scale / 4 : 1;
    int mid = scale / 2;
    for (k = 0; k < r.sy; k++)
    {
        int yk = (r.y+k);
        for (j = 0; j < r.sz; j++)
        {
            int zj = (r.z+j)*scale + mid;
            for (i = 0; i < r.sx; i++)
            {
                int xi = (r.x+i)*scale + mid;
                *p = sampleBiomeNoise(bn, NULL, xi, yk, zj, p_dat, flags);
                p++;
            }
        }
    }
}

int genBiomeNoiseScaled(const BiomeNoise *bn, int *out, Range r, int mc, uint64_t sha)
{
    if (mc <= MC_1_17)
        return 1; // bad version

    if (r.sy == 0)
        r.sy = 1;

    uint64_t siz = (uint64_t)r.sx*r.sy*r.sz;
    int i, j, k;

    if (r.scale == 1)
    {
        Range s = getVoronoiSrcRange(r);
        int *src;
        if (siz > 1)
        {   // the source range is large enough that we can try optimizing
            src = out + siz;
            genBiomeNoise3D(bn, src, s, 0);
        }
        else
        {
            src = NULL;
        }

        int *p = out;
        for (k = 0; k < r.sy; k++)
        {
            for (j = 0; j < r.sz; j++)
            {
                for (i = 0; i < r.sx; i++)
                {
                    int x4, z4, y4;
                    voronoiAccess3D(sha, r.x+i, r.y+k, r.z+j, &x4, &y4, &z4);
                    if (src)
                    {
                        x4 -= s.x; y4 -= s.y; z4 -= s.z;
                        *p = src[y4*s.sx*s.sz + z4*s.sx + x4];
                    }
                    else
                    {
                        *p = sampleBiomeNoise(bn, 0, x4, y4, z4, 0, 0);
                    }
                    p++;
                }
            }
        }
    }
    else
    {
        // There is (was?) an optimization that causes MC-241546, and should
        // not be enabled for accurate results. However, if the scale is higher
        // than 1:4, the accuracy becomes questionable anyway. Furthermore
        // situations that want to use a higher scale are usually better off
        // with a faster, if imperfect, result.
        genBiomeNoise3D(bn, out, r, r.scale > 4);
    }
    return 0;
}


int getBiomeDepthAndScale(int id, double *depth, double *scale, int *grass)
{
    const int dh = 62; // default height
    double s = 0, d = 0, g = 0;
    switch (id) {
    case ocean:                         s = 0.100; d = -1.000; g = dh; break;
    case plains:                        s = 0.050; d =  0.125; g = dh; break;
    case desert:                        s = 0.050; d =  0.125; g =  0; break;
    case mountains:                     s = 0.500; d =  1.000; g = dh; break;
    case forest:                        s = 0.200; d =  0.100; g = dh; break;
    case taiga:                         s = 0.200; d =  0.200; g = dh; break;
    case swamp:                         s = 0.100; d = -0.200; g = dh; break;
    case river:                         s = 0.000; d = -0.500; g = 60; break;
    case frozen_ocean:                  s = 0.100; d = -1.000; g = dh; break;
    case frozen_river:                  s = 0.000; d = -0.500; g = 60; break;
    case snowy_tundra:                  s = 0.050; d =  0.125; g = dh; break;
    case snowy_mountains:               s = 0.300; d =  0.450; g = dh; break;
    case mushroom_fields:               s = 0.300; d =  0.200; g =  0; break;
    case mushroom_field_shore:          s = 0.025; d =  0.000; g =  0; break;
    case beach:                         s = 0.025; d =  0.000; g = 64; break;
    case desert_hills:                  s = 0.300; d =  0.450; g =  0; break;
    case wooded_hills:                  s = 0.300; d =  0.450; g = dh; break;
    case taiga_hills:                   s = 0.300; d =  0.450; g = dh; break;
    case mountain_edge:                 s = 0.300; d =  0.800; g = dh; break;
    case jungle:                        s = 0.200; d =  0.100; g = dh; break;
    case jungle_hills:                  s = 0.300; d =  0.450; g = dh; break;
    case jungle_edge:                   s = 0.200; d =  0.100; g = dh; break;
    case deep_ocean:                    s = 0.100; d = -1.800; g = dh; break;
    case stone_shore:                   s = 0.800; d =  0.100; g = 64; break;
    case snowy_beach:                   s = 0.025; d =  0.000; g = 64; break;
    case birch_forest:                  s = 0.200; d =  0.100; g = dh; break;
    case birch_forest_hills:            s = 0.300; d =  0.450; g = dh; break;
    case dark_forest:                   s = 0.200; d =  0.100; g = dh; break;
    case snowy_taiga:                   s = 0.200; d =  0.200; g = dh; break;
    case snowy_taiga_hills:             s = 0.300; d =  0.450; g = dh; break;
    case giant_tree_taiga:              s = 0.200; d =  0.200; g = dh; break;
    case giant_tree_taiga_hills:        s = 0.300; d =  0.450; g = dh; break;
    case wooded_mountains:              s = 0.500; d =  1.000; g = dh; break;
    case savanna:                       s = 0.050; d =  0.125; g = dh; break;
    case savanna_plateau:               s = 0.025; d =  1.500; g = dh; break;
    case badlands:                      s = 0.200; d =  0.100; g =  0; break;
    case wooded_badlands_plateau:       s = 0.025; d =  1.500; g =  0; break;
    case badlands_plateau:              s = 0.025; d =  1.500; g =  0; break;
    case warm_ocean:                    s = 0.100; d = -1.000; g =  0; break;
    case lukewarm_ocean:                s = 0.100; d = -1.000; g = dh; break;
    case cold_ocean:                    s = 0.100; d = -1.000; g = dh; break;
    case deep_warm_ocean:               s = 0.100; d = -1.800; g =  0; break;
    case deep_lukewarm_ocean:           s = 0.100; d = -1.800; g = dh; break;
    case deep_cold_ocean:               s = 0.100; d = -1.800; g = dh; break;
    case deep_frozen_ocean:             s = 0.100; d = -1.800; g = dh; break;
    case sunflower_plains:              s = 0.050; d =  0.125; g = dh; break;
    case desert_lakes:                  s = 0.250; d =  0.225; g =  0; break;
    case gravelly_mountains:            s = 0.500; d =  1.000; g = dh; break;
    case flower_forest:                 s = 0.400; d =  0.100; g = dh; break;
    case taiga_mountains:               s = 0.400; d =  0.300; g = dh; break;
    case swamp_hills:                   s = 0.300; d = -0.100; g = dh; break;
    case ice_spikes:                    s = 0.450; d =  0.425; g =  0; break;
    case modified_jungle:               s = 0.400; d =  0.200; g = dh; break;
    case modified_jungle_edge:          s = 0.400; d =  0.200; g = dh; break;
    case tall_birch_forest:             s = 0.400; d =  0.200; g = dh; break;
    case tall_birch_hills:              s = 0.500; d =  0.550; g = dh; break;
    case dark_forest_hills:             s = 0.400; d =  0.200; g = dh; break;
    case snowy_taiga_mountains:         s = 0.400; d =  0.300; g = dh; break;
    case giant_spruce_taiga:            s = 0.200; d =  0.200; g = dh; break;
    case giant_spruce_taiga_hills:      s = 0.200; d =  0.200; g = dh; break;
    case modified_gravelly_mountains:   s = 0.500; d =  1.000; g = dh; break;
    case shattered_savanna:             s = 1.225; d = 0.3625; g = dh; break;
    case shattered_savanna_plateau:     s = 1.212; d =  1.050; g = dh; break;
    case eroded_badlands:               s = 0.200; d =  0.100; g =  0; break;
    case modified_wooded_badlands_plateau: s = 0.300; d = 0.450; g = 0; break;
    case modified_badlands_plateau:     s = 0.300; d =  0.450; g =  0; break;
    case bamboo_jungle:                 s = 0.200; d =  0.100; g = dh; break;
    case bamboo_jungle_hills:           s = 0.300; d =  0.450; g = dh; break;
    default:
        return 0;
    }
    if (scale) *scale = s;
    if (depth) *depth = d;
    if (grass) *grass = g;
    return 1;
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
    uint64_t ss = l->startSeed;
    uint64_t cs;
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
    if unlikely(err != 0)
        return err;

    int newW = (pW) << 1;
    //int newH = (pH) << 1;
    int idx, v00, v01, v10, v11;
    int *buf = out + pW * pH; //(int*) malloc((newW+1)*(newH+1)*sizeof(*buf));

    const uint32_t st = (uint32_t)l->startSalt;
    const uint32_t ss = (uint32_t)l->startSeed;

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

            int chunkX = (int)((uint32_t)(i + pX) << 1);
            int chunkZ = (int)((uint32_t)(j + pZ) << 1);

            uint32_t cs = ss;
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
        memmove(&out[j*w], &buf[(j + (z & 1))*newW + (x & 1)], w*sizeof(int));
    }
    //free(buf);

    return 0;
}


static inline int select4(uint32_t cs, uint32_t st, int v00, int v01, int v10, int v11)
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
    if unlikely(err != 0)
        return err;

    int newW = (pW) << 1;
    //int newH = (pH) << 1;
    int idx, v00, v01, v10, v11;
    int *buf = out + pW * pH; //(int*) malloc((newW+1)*(newH+1)*sizeof(*buf));

    const uint32_t st = (uint32_t)l->startSalt;
    const uint32_t ss = (uint32_t)l->startSeed;

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

            int chunkX = (int)((uint32_t)(i + pX) << 1);
            int chunkZ = (int)((uint32_t)(j + pZ) << 1);

            uint32_t cs = ss;
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
        memmove(&out[j*w], &buf[(j + (z & 1))*newW + (x & 1)], w*sizeof(int));
    }
    //free(buf);

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
    if unlikely(err != 0)
        return err;

    uint64_t st = l->startSalt;
    uint64_t ss = l->startSeed;
    uint64_t cs;

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
    if unlikely(err != 0)
        return err;

    uint64_t st = l->startSalt;
    uint64_t ss = l->startSeed;
    uint64_t cs;

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

int mapLandB18(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;
    int i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

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

            if (v11 == 0 && (v00 != 0 || v02 != 0 || v20 != 0 || v22 != 0))
            {
                cs = getChunkSeed(ss, i+x, j+z);
                v = mcFirstInt(cs, 3) / 2;
            }
            else if (v11 == 1 && (v00 != 1 || v02 != 1 || v20 != 1 || v22 != 1))
            {
                cs = getChunkSeed(ss, i+x, j+z);
                v = 1 - mcFirstInt(cs, 5) / 4;
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
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

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
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

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
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

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
    if unlikely(err != 0)
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
    if unlikely(err != 0)
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
    if unlikely(err != 0)
        return err;

    uint64_t st = l->startSalt;
    uint64_t ss = l->startSeed;
    uint64_t cs;

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
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

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
    if unlikely(err != 0)
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
    if unlikely(err != 0)
        return err;

    int mc = l->mc;
    uint64_t ss = l->startSeed;
    uint64_t cs;

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
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

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
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

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
    if unlikely(err != 0)
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

    if unlikely(l->p2 == NULL)
    {
        printf("mapHills() requires two parents! Use setupMultiLayer()\n");
        exit(1);
    }

    int err;
    err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    int *riv = out + pW * pH;
    err = l->p2->getMap(l->p2, riv, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    int mc = l->mc;
    uint64_t st = l->startSalt;
    uint64_t ss = l->startSeed;
    uint64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int a11 = out[i+1 + (j+1)*pW]; // biome branch
            int b11 = riv[i+1 + (j+1)*pW]; // river branch
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
                        int a10 = out[i+1 + (j+0)*pW];
                        int a21 = out[i+2 + (j+1)*pW];
                        int a01 = out[i+0 + (j+1)*pW];
                        int a12 = out[i+1 + (j+2)*pW];
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
    if unlikely(err != 0)
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
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

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
                    if (cs & ((uint64_t)1 << 24))
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
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

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
    if unlikely(err != 0)
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
            if (mc <= MC_1_0)
            {
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
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

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
    if unlikely(l->p2 == NULL)
    {
        printf("mapRiverMix() requires two parents! Use setupMultiLayer()\n");
        exit(1);
    }

    int err = l->p->getMap(l->p, out, x, z, w, h); // biome chain
    if unlikely(err != 0)
        return err;

    int idx;
    int mc = l->mc;
    int len = w*h;
    int *buf = out + len;

    err = l->p2->getMap(l->p2, buf, x, z, w, h); // rivers
    if unlikely(err != 0)
        return err;


    for (idx = 0; idx < len; idx++)
    {
        int v = out[idx];

        if (buf[idx] == river && v != ocean && (mc <= MC_1_6 || !isOceanic(v)))
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
            double tmp = samplePerlin(rnd, (i + x) / 8.0, (j + z) / 8.0, 0, 0, 0);

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
    int i, j;
    int lx0, lx1, lz0, lz1, lw, lh;

    if unlikely(l->p2 == NULL)
    {
        printf("mapOceanMix() requires two parents! Use setupMultiLayer()\n");
        exit(1);
    }

    int err = l->p2->getMap(l->p2, out, x, z, w, h);
    if unlikely(err != 0)
        return err;

    // determine the minimum required land area: (x+lx0, z+lz0), (lw, lh)
    // (the extra border is only required if there is warm or frozen ocean)
    lx0 = 0; lx1 = w;
    lz0 = 0; lz1 = h;

    for (j = 0; j < h; j++)
    {
        int jcentre = (j-8 > 0 && j+9 < h);
        for (i = 0; i < w; i++)
        {
            if (jcentre && i-8 > 0 && i+9 < w)
                continue;
            int oceanID = out[i + j*w];
            if (oceanID == warm_ocean || oceanID == frozen_ocean)
            {
                if (i-8 < lx0) lx0 = i-8;
                if (i+9 > lx1) lx1 = i+9;
                if (j-8 < lz0) lz0 = j-8;
                if (j+9 > lz1) lz1 = j+9;
            }
        }
    }

    int *land = out + w*h;
    lw = lx1 - lx0;
    lh = lz1 - lz0;
    err = l->p->getMap(l->p, land, x+lx0, z+lz0, lw, lh);
    if unlikely(err != 0)
        return err;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int landID = land[(i-lx0) + (j-lz0)*lw];
            int oceanID = out[i + j*w];
            int replaceID = 0;
            int ii, jj;

            if (!isOceanic(landID))
            {
                out[i + j*w] = landID;
                continue;
            }

            if (oceanID == warm_ocean  ) replaceID = lukewarm_ocean;
            if (oceanID == frozen_ocean) replaceID = cold_ocean;
            if (replaceID)
            {
                for (ii = -8; ii <= 8; ii += 4)
                {
                    for (jj = -8; jj <= 8; jj += 4)
                    {
                        int id = land[(i+ii-lx0) + (j+jj-lz0)*lw];
                        if (!isOceanic(id))
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

    return 0;
}


Range getVoronoiSrcRange(Range r)
{
    if (r.scale != 1)
    {
        printf("getVoronoiSrcRange() expects input range with scale 1:1\n");
        exit(1);
    }

    Range s; // output has scale 1:4
    int tx = r.x - 2;
    int tz = r.z - 2;
    s.scale = 4;
    s.x = tx >> 2;
    s.z = tz >> 2;
    s.sx = ((tx + r.sx) >> 2) - s.x + 2;
    s.sz = ((tz + r.sz) >> 2) - s.z + 2;
    if (r.sy < 1)
    {
        s.y = s.sy = 0;
    }
    else
    {
        int ty = r.y - 2;
        s.y = ty >> 2;
        s.sy = ((ty + r.sy) >> 2) - s.y + 2;
    }
    return s;
}

static inline void getVoronoiCell(uint64_t sha, int a, int b, int c,
        int *x, int *y, int *z)
{
    uint64_t s = sha;
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

void mapVoronoiPlane(uint64_t sha, int *out, int *src,
    int x, int z, int w, int h, int y, int px, int pz, int pw, int ph)
{
    x -= 2;
    y -= 2;
    z -= 2;
    int x000, x001, x010, x011, x100, x101, x110, x111;
    int y000, y001, y010, y011, y100, y101, y110, y111;
    int z000, z001, z010, z011, z100, z101, z110, z111;
    int pi, pj, ii, jj, dx, dz, pjz, pix, i4, j4;
    int v00, v01, v10, v11, v;
    int prev_skip;
    int64_t r;
    uint64_t d, dmin;
    int i, j;

    for (pj = 0; pj < ph-1; pj++)
    {
        v00 = src[(pj+0)*pw];
        v10 = src[(pj+1)*pw];
        pjz = pz + pj;
        j4 = ((pjz) << 2) - z;
        prev_skip = 1;

        for (pi = 0; pi < pw-1; pi++)
        {
            PREFETCH( out + ((pjz << 2) + 0) * w + pi, 1, 1 );
            PREFETCH( out + ((pjz << 2) + 1) * w + pi, 1, 1 );
            PREFETCH( out + ((pjz << 2) + 2) * w + pi, 1, 1 );
            PREFETCH( out + ((pjz << 2) + 3) * w + pi, 1, 1 );

            v01 = src[(pj+0)*pw + (pi+1)];
            v11 = src[(pj+1)*pw + (pi+1)];
            pix = px + pi;
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
                        out[j*w + i] = v00;
                    }
                }
                prev_skip = 1;
                continue;
            }
            if (prev_skip)
            {
                getVoronoiCell(sha, pix, y-1, pjz+0, &x000, &y000, &z000);
                getVoronoiCell(sha, pix, y+0, pjz+0, &x001, &y001, &z001);
                getVoronoiCell(sha, pix, y-1, pjz+1, &x100, &y100, &z100);
                getVoronoiCell(sha, pix, y+0, pjz+1, &x101, &y101, &z101);
                prev_skip = 0;
            }
            getVoronoiCell(sha, pix+1, y-1, pjz+0, &x010, &y010, &z010);
            getVoronoiCell(sha, pix+1, y+0, pjz+0, &x011, &y011, &z011);
            getVoronoiCell(sha, pix+1, y-1, pjz+1, &x110, &y110, &z110);
            getVoronoiCell(sha, pix+1, y+0, pjz+1, &x111, &y111, &z111);


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

                    out[j*w + i] = v;
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
}

int mapVoronoi(const Layer * l, int * out, int x, int z, int w, int h)
{
    x -= 2;
    z -= 2;
    int px = x >> 2;
    int pz = z >> 2;
    int pw = ((x + w) >> 2) - px + 2;
    int ph = ((z + h) >> 2) - pz + 2;

    if (l->p)
    {
        int err = l->p->getMap(l->p, out, px, pz, pw, ph);
        if (err != 0)
            return err;
    }

    int *src = out + w*h;
    memmove(src, out, pw*ph*sizeof(int));
    mapVoronoiPlane(l->startSalt, out, src, x,z,w,h, 0, px,pz,pw,ph);

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

    int i, j, ii, jj, pi, pj, pix, pjz, i4, j4, mi, mj;
    int v00, v01, v10, v11, v;
    int64_t da1, da2, db1, db2, dc1, dc2, dd1, dd2;
    int64_t sja, sjb, sjc, sjd, da, db, dc, dd;
    int *buf = out + pW * pH;

    uint64_t st = l->startSalt;
    uint64_t ss = l->startSeed;
    uint64_t cs;

    for (pj = 0; pj < pH-1; pj++)
    {
        v00 = out[(pj+0)*pW];
        v01 = out[(pj+1)*pW];
        pjz = pZ + pj;
        j4 = ((pjz) << 2) - z;

        for (pi = 0; pi < pW-1; pi++, v00 = v10, v01 = v11)
        {
            pix = pX + pi;
            i4 = ((pix) << 2) - x;

            // try to prefetch the relevant rows to help prevent cache misses
            PREFETCH( buf + ((pjz << 2) + 0) * w + pi, 1, 1 );
            PREFETCH( buf + ((pjz << 2) + 1) * w + pi, 1, 1 );
            PREFETCH( buf + ((pjz << 2) + 2) * w + pi, 1, 1 );
            PREFETCH( buf + ((pjz << 2) + 3) * w + pi, 1, 1 );

            v10 = out[pi+1 + (pj+0)*pW];
            v11 = out[pi+1 + (pj+1)*pW];

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
                continue;
            }

            cs = getChunkSeed(ss, (pi+pX) << 2, (pj+pZ) << 2);
            da1 = (mcFirstInt(cs, 1024) - 512) * 36;
            cs = mcStepSeed(cs, st);
            da2 = (mcFirstInt(cs, 1024) - 512) * 36;

            cs = getChunkSeed(ss, (pi+pX+1) << 2, (pj+pZ) << 2);
            db1 = (mcFirstInt(cs, 1024) - 512) * 36 + 40*1024;
            cs = mcStepSeed(cs, st);
            db2 = (mcFirstInt(cs, 1024) - 512) * 36;

            cs = getChunkSeed(ss, (pi+pX) << 2, (pj+pZ+1) << 2);
            dc1 = (mcFirstInt(cs, 1024) - 512) * 36;
            cs = mcStepSeed(cs, st);
            dc2 = (mcFirstInt(cs, 1024) - 512) * 36 + 40*1024;

            cs = getChunkSeed(ss, (pi+pX+1) << 2, (pj+pZ+1) << 2);
            dd1 = (mcFirstInt(cs, 1024) - 512) * 36 + 40*1024;
            cs = mcStepSeed(cs, st);
            dd2 = (mcFirstInt(cs, 1024) - 512) * 36 + 40*1024;

            for (jj = 0; jj < 4; jj++)
            {
                j = j4 + jj;
                if (j < 0 || j >= h) continue;

                mj = 10240*jj;
                sja = (mj-da2) * (mj-da2);
                sjb = (mj-db2) * (mj-db2);
                sjc = (mj-dc2) * (mj-dc2);
                sjd = (mj-dd2) * (mj-dd2);

                for (ii = 0; ii < 4; ii++)
                {
                    i = i4 + ii;
                    if (i < 0 || i >= w) continue;

                    mi = 10240*ii;
                    da = (mi-da1) * (mi-da1) + sja;
                    db = (mi-db1) * (mi-db1) + sjb;
                    dc = (mi-dc1) * (mi-dc1) + sjc;
                    dd = (mi-dd1) * (mi-dd1) + sjd;

                    if      unlikely((da < db) && (da < dc) && (da < dd))
                        v = v00;
                    else if unlikely((db < da) && (db < dc) && (db < dd))
                        v = v10;
                    else if unlikely((dc < da) && (dc < db) && (dc < dd))
                        v = v01;
                    else
                        v = v11;

                    buf[j*w + i] = v;
                }
            }
        }
    }

    memmove(out, buf, w*h*sizeof(*buf));

    return 0;
}


uint64_t getVoronoiSHA(uint64_t seed)
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
    m[0] = BSWAP32((uint32_t)(seed));
    m[1] = BSWAP32((uint32_t)(seed >> 32));
    m[2] = 0x80000000;
    for (i = 3; i < 15; i++)
        m[i] = 0;
    m[15] = 0x00000040;

    for (i = 16; i < 64; ++i)
    {
        m[i] = m[i - 7] + m[i - 16];
        x = m[i - 15];
        m[i] += rotr32(x,7) ^ rotr32(x,18) ^ (x >> 3);
        x = m[i - 2];
        m[i] += rotr32(x,17) ^ rotr32(x,19) ^ (x >> 10);
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
        x += rotr32(a4,6) ^ rotr32(a4,11) ^ rotr32(a4,25);
        x += (a4 & a5) ^ (~a4 & a6);

        y = rotr32(a0,2) ^ rotr32(a0,13) ^ rotr32(a0,22);
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

    return BSWAP32(a0) | ((uint64_t)BSWAP32(a1) << 32);
}

void voronoiAccess3D(uint64_t sha, int x, int y, int z, int *x4, int *y4, int *z4)
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



