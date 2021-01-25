#ifndef LAYER_H_
#define LAYER_H_

#include "javarnd.h"

#define __STDC_FORMAT_MACROS 1

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

#define STRUCT(S) typedef struct S S; struct S

#if __GNUC__
#define PREFETCH(PTR,RW,LOC)    __builtin_prefetch(PTR,RW,LOC)
#define L(COND)                 (__builtin_expect(!!(COND),1))  // [[likely]]
#define U(COND)                 (__builtin_expect((COND),0))    // [[unlikely]]
#else
#define PREFETCH(PTR,RW,LOC)
#define L(COND)                 (COND)
#define U(COND)                 (COND)
#endif


enum BiomeID
{
    none = -1,
    // 0
    ocean = 0,
    plains,
    desert,
    mountains,                  extremeHills = mountains,
    forest,
    taiga,
    swamp,                      swampland = swamp,
    river,
    nether_wastes,              hell = nether_wastes,
    the_end,                    sky = the_end,
    // 10
    frozen_ocean,               frozenOcean = frozen_ocean,
    frozen_river,               frozenRiver = frozen_river,
    snowy_tundra,               icePlains = snowy_tundra,
    snowy_mountains,            iceMountains = snowy_mountains,
    mushroom_fields,            mushroomIsland = mushroom_fields,
    mushroom_field_shore,       mushroomIslandShore = mushroom_field_shore,
    beach,
    desert_hills,               desertHills = desert_hills,
    wooded_hills,               forestHills = wooded_hills,
    taiga_hills,                taigaHills = taiga_hills,
    // 20
    mountain_edge,              extremeHillsEdge = mountain_edge,
    jungle,
    jungle_hills,               jungleHills = jungle_hills,
    jungle_edge,                jungleEdge = jungle_edge,
    deep_ocean,                 deepOcean = deep_ocean,
    stone_shore,                stoneBeach = stone_shore,
    snowy_beach,                coldBeach = snowy_beach,
    birch_forest,               birchForest = birch_forest,
    birch_forest_hills,         birchForestHills = birch_forest_hills,
    dark_forest,                roofedForest = dark_forest,
    // 30
    snowy_taiga,                coldTaiga = snowy_taiga,
    snowy_taiga_hills,          coldTaigaHills = snowy_taiga_hills,
    giant_tree_taiga,           megaTaiga = giant_tree_taiga,
    giant_tree_taiga_hills,     megaTaigaHills = giant_tree_taiga_hills,
    wooded_mountains,           extremeHillsPlus = wooded_mountains,
    savanna,
    savanna_plateau,            savannaPlateau = savanna_plateau,
    badlands,                   mesa = badlands,
    wooded_badlands_plateau,    mesaPlateau_F = wooded_badlands_plateau,
    badlands_plateau,           mesaPlateau = badlands_plateau,
    // 40  --  1.13
    small_end_islands,
    end_midlands,
    end_highlands,
    end_barrens,
    warm_ocean,                 warmOcean = warm_ocean,
    lukewarm_ocean,             lukewarmOcean = lukewarm_ocean,
    cold_ocean,                 coldOcean = cold_ocean,
    deep_warm_ocean,            warmDeepOcean = deep_warm_ocean,
    deep_lukewarm_ocean,        lukewarmDeepOcean = deep_lukewarm_ocean,
    deep_cold_ocean,            coldDeepOcean = deep_cold_ocean,
    // 50
    deep_frozen_ocean,          frozenDeepOcean = deep_frozen_ocean,
    BIOME_NUM,

    the_void = 127,

    // mutated variants
    sunflower_plains                = plains+128,
    desert_lakes                    = desert+128,
    gravelly_mountains              = mountains+128,
    flower_forest                   = forest+128,
    taiga_mountains                 = taiga+128,
    swamp_hills                     = swamp+128,
    ice_spikes                      = snowy_tundra+128,
    modified_jungle                 = jungle+128,
    modified_jungle_edge            = jungle_edge+128,
    tall_birch_forest               = birch_forest+128,
    tall_birch_hills                = birch_forest_hills+128,
    dark_forest_hills               = dark_forest+128,
    snowy_taiga_mountains           = snowy_taiga+128,
    giant_spruce_taiga              = giant_tree_taiga+128,
    giant_spruce_taiga_hills        = giant_tree_taiga_hills+128,
    modified_gravelly_mountains     = wooded_mountains+128,
    shattered_savanna               = savanna+128,
    shattered_savanna_plateau       = savanna_plateau+128,
    eroded_badlands                 = badlands+128,
    modified_wooded_badlands_plateau = wooded_badlands_plateau+128,
    modified_badlands_plateau       = badlands_plateau+128,
    // 1.14
    bamboo_jungle                   = 168,
    bamboo_jungle_hills             = 169,
    // 1.16
    soul_sand_valley                = 170,
    crimson_forest                  = 171,
    warped_forest                   = 172,
    basalt_deltas                   = 173,
};

enum BiomeType
{
    Void = -1,
    Ocean, Plains, Desert, Hills, Forest, Taiga, Swamp, River, Nether, Sky, Snow, MushroomIsland, Beach, Jungle, StoneBeach, Savanna, Mesa,
    BTYPE_NUM
};

enum BiomeTempCategory
{
    Oceanic, Warm, Lush, Cold, Freezing, Special
};


STRUCT(Biome)
{
    int id;
    int type;
    double height;
    double temp;
    int tempCat;
    int mutated;
};


STRUCT(PerlinNoise)
{
    int d[512];
    double a, b, c;
};


STRUCT(Layer)
{
    int64_t layerSeed;  // (depends only on layer salt)
    int64_t startSalt;  // (world seed dependent) = worldGenSeed, used for RND beyond the first
    int64_t startSeed;  // (world seed dependent) starting point for chunk seeds

    void *noise;        // seed dependent data for noise maps
    void *data;         // generic data for custom layers

    int scale;          // map scale of this layer (map entry = scale x scale blocks)
    int edge;           // maximum border required from parent layer

    int (*getMap)(const Layer *, int *, int, int, int, int);

    Layer *p, *p2;      // parent layers
};

#ifdef __cplusplus
extern "C"
{
#endif

//==============================================================================
// Essentials
//==============================================================================

extern Biome biomes[256];


/* initBiomes() has to be called before any of the generators can be used */
void initBiomes();

/* Applies the given world seed to the layer and all dependent layers. */
void setWorldSeed(Layer *layer, int64_t worldSeed);


void perlinInit(PerlinNoise *rnd, int64_t seed);
double samplePerlin(const PerlinNoise *rnd, double d1, double d2, double d3);

//==============================================================================
// Static Helpers
//==============================================================================


static inline int getBiomeType(int id)
{
    return (id & (~0xff)) ? Void : biomes[id].type;
}

static inline int biomeExists(int id)
{
    return !(id & (~0xff)) && !(biomes[id].id & (~0xff));
}

static inline int getTempCategory(int id)
{
    return (id & (~0xff)) ? Void : biomes[id].tempCat;
}

static inline int areSimilar112(int id1, int id2)
{
    if (id1 == id2) return 1;
    if (id1 == wooded_badlands_plateau || id1 == badlands_plateau)
        return id2 == wooded_badlands_plateau || id2 == badlands_plateau;
    if (!biomeExists(id1) || !biomeExists(id2)) return 0;
    // adjust for asymmetric equality (workaround to simulate a bug in the MC java code)
    if (id1 >= 128 || id2 >= 128) {
        // skip biomes that did not overload the isEqualTo() method
        if (id2 == 130 || id2 == 133 || id2 == 134 || id2 == 149 || id2 == 151 || id2 == 155 ||
            id2 == 156 || id2 == 157 || id2 == 158 || id2 == 163 || id2 == 164) return 0;
    }
    return getBiomeType(id1) == getBiomeType(id2);
}

static inline int areSimilar(int id1, int id2)
{
    if (id1 == id2) return 1;
    if (id1 == wooded_badlands_plateau || id1 == badlands_plateau)
        return id2 == wooded_badlands_plateau || id2 == badlands_plateau;
    if (!biomeExists(id1) || !biomeExists(id2)) return 0;
    return getBiomeType(id1) == getBiomeType(id2);
}

static inline int isShallowOcean(int id)
{
    const uint64_t shallow_bits =
            (1ULL << ocean) |
            (1ULL << frozen_ocean) |
            (1ULL << warm_ocean) |
            (1ULL << lukewarm_ocean) |
            (1ULL << cold_ocean);
    return id < 64 && ((1ULL << id) & shallow_bits);
}

static inline int isDeepOcean(int id)
{
    const uint64_t deep_bits =
            (1ULL << deep_ocean) |
            (1ULL << deep_warm_ocean) |
            (1ULL << deep_lukewarm_ocean) |
            (1ULL << deep_cold_ocean) |
            (1ULL << deep_frozen_ocean);
    return id < 64 && ((1ULL << id) & deep_bits);
}

static inline int isOceanic(int id)
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


static inline int isBiomeSnowy(int id)
{
    return biomeExists(id) && biomes[id].temp < 0.1;
}


/**
 * The seed pipeline:
 *
 * Salt of Layer                   -> layerSeed (ls)
 * layerSeed (ls) & worldSeed (ws) -> startSalt (st) & startSeed (ss)
 * startSeed (ls) & coords (x,z)   -> chunkSeed (cs)
 *
 * The chunkSeed alone is enough to generate the first RND integer with:
 *   mcFirstInt(cs, mod)
 * subsequent RND integers are generated by stepping the chunkSeed forwards,
 * salted with startSalt:
 *   cs_next = mcStepSeed(cs, st)
 */

static inline int64_t mcStepSeed(int64_t s, int64_t salt)
{
    return s * (s * 6364136223846793005LL + 1442695040888963407LL) + salt;
}

static inline int mcFirstInt(int64_t s, int mod)
{
    int ret = (int)((s >> 24) % mod);
    if (ret < 0)
        ret += mod;
    return ret;
}

static inline int mcFirstIsZero(int64_t s, int mod)
{
    return (int)((s >> 24) % mod) == 0;
}

static inline int64_t getChunkSeed(int64_t ss, int x, int z)
{
    int64_t cs = ss + x;
    cs = mcStepSeed(cs, z);
    cs = mcStepSeed(cs, x);
    cs = mcStepSeed(cs, z);
    return cs;
}

static inline int64_t getLayerSeed(int64_t salt)
{
    int64_t ls = mcStepSeed(salt, salt);
    ls = mcStepSeed(ls, salt);
    ls = mcStepSeed(ls, salt);
    return ls;
}

static inline int64_t getStartSalt(int64_t ws, int64_t ls)
{
    int64_t st = ws;
    st = mcStepSeed(st, ls);
    st = mcStepSeed(st, ls);
    st = mcStepSeed(st, ls);
    return st;
}

static inline int64_t getStartSeed(int64_t ws, int64_t ls)
{
    int64_t ss = ws;
    ss = getStartSalt(ss, ls);
    ss = mcStepSeed(ss, 0);
    return ss;
}



//==============================================================================
// Layers
//==============================================================================

int mapIsland               (const Layer *, int *, int, int, int, int);
int mapZoomIsland           (const Layer *, int *, int, int, int, int);
int mapZoom                 (const Layer *, int *, int, int, int, int);
int mapAddIsland            (const Layer *, int *, int, int, int, int);
int mapRemoveTooMuchOcean   (const Layer *, int *, int, int, int, int);
int mapAddSnow              (const Layer *, int *, int, int, int, int);
int mapCoolWarm             (const Layer *, int *, int, int, int, int);
int mapHeatIce              (const Layer *, int *, int, int, int, int);
int mapSpecial              (const Layer *, int *, int, int, int, int);
int mapAddMushroomIsland    (const Layer *, int *, int, int, int, int);
int mapDeepOcean            (const Layer *, int *, int, int, int, int);
int mapBiome                (const Layer *, int *, int, int, int, int);
int mapBiomeBE              (const Layer *, int *, int, int, int, int);
int mapAddBamboo            (const Layer *, int *, int, int, int, int);
int mapRiverInit            (const Layer *, int *, int, int, int, int);
int mapBiomeEdge            (const Layer *, int *, int, int, int, int);
int mapHills112             (const Layer *, int *, int, int, int, int);
int mapRiver                (const Layer *, int *, int, int, int, int);
int mapSmooth               (const Layer *, int *, int, int, int, int);
int mapRareBiome            (const Layer *, int *, int, int, int, int);
int mapShore                (const Layer *, int *, int, int, int, int);
int mapRiverMix             (const Layer *, int *, int, int, int, int);

// 1.13 layers
int mapHills                (const Layer *, int *, int, int, int, int);
int mapOceanTemp            (const Layer *, int *, int, int, int, int);
int mapOceanMix             (const Layer *, int *, int, int, int, int);

// final layer 1:1
int mapVoronoiZoom          (const Layer *, int *, int, int, int, int);


#ifdef __cplusplus
}
#endif

#endif /* LAYER_H_ */
