#ifndef LAYER_H_
#define LAYER_H_

#include "noise.h"
#include "biomes.h"


#define LAYER_INIT_SHA          (~0ULL)


enum BiomeTempCategory
{
    Oceanic, Warm, Lush, Cold, Freezing, Special
};

/* Enumeration of the layer indices in the layer stack. */
enum LayerId
{
    // new                  [[deprecated]]
    L_CONTINENT_4096 = 0,   L_ISLAND_4096 = L_CONTINENT_4096,
    L_ZOOM_4096,                                                    // b1.8
    L_LAND_4096,                                                    // b1.8
    L_ZOOM_2048,
    L_LAND_2048,            L_ADD_ISLAND_2048 = L_LAND_2048,
    L_ZOOM_1024,
    L_LAND_1024_A,          L_ADD_ISLAND_1024A = L_LAND_1024_A,
    L_LAND_1024_B,          L_ADD_ISLAND_1024B = L_LAND_1024_B,     // 1.7+
    L_LAND_1024_C,          L_ADD_ISLAND_1024C = L_LAND_1024_C,     // 1.7+
    L_ISLAND_1024,          L_REMOVE_OCEAN_1024 = L_ISLAND_1024,    // 1.7+
    L_SNOW_1024,            L_ADD_SNOW_1024 = L_SNOW_1024,
    L_LAND_1024_D,          L_ADD_ISLAND_1024D = L_LAND_1024_D,     // 1.7+
    L_COOL_1024,            L_COOL_WARM_1024 = L_COOL_1024,         // 1.7+
    L_HEAT_1024,            L_HEAT_ICE_1024 = L_HEAT_1024,          // 1.7+
    L_SPECIAL_1024,                                                 // 1.7+
    L_ZOOM_512,
    L_LAND_512,                                                     // 1.6-
    L_ZOOM_256,
    L_LAND_256,             L_ADD_ISLAND_256 = L_LAND_256,
    L_MUSHROOM_256,         L_ADD_MUSHROOM_256 = L_MUSHROOM_256,
    L_DEEP_OCEAN_256,                                               // 1.7+
    L_BIOME_256,
    L_BAMBOO_256,           L14_BAMBOO_256 = L_BAMBOO_256,          // 1.14+
    L_ZOOM_128,
    L_ZOOM_64,
    L_BIOME_EDGE_64,
    L_NOISE_256,            L_RIVER_INIT_256 = L_NOISE_256,
    L_ZOOM_128_HILLS,
    L_ZOOM_64_HILLS,
    L_HILLS_64,
    L_SUNFLOWER_64,         L_RARE_BIOME_64 = L_SUNFLOWER_64,       // 1.7+
    L_ZOOM_32,
    L_LAND_32,              L_ADD_ISLAND_32 = L_LAND_32,
    L_ZOOM_16,
    L_SHORE_16,             // NOTE: in 1.0 this slot is scale 1:32
    L_SWAMP_RIVER_16,                                               // 1.6-
    L_ZOOM_8,
    L_ZOOM_4,
    L_SMOOTH_4,
    L_ZOOM_128_RIVER,
    L_ZOOM_64_RIVER,
    L_ZOOM_32_RIVER,
    L_ZOOM_16_RIVER,
    L_ZOOM_8_RIVER,
    L_ZOOM_4_RIVER,
    L_RIVER_4,
    L_SMOOTH_4_RIVER,
    L_RIVER_MIX_4,
    L_OCEAN_TEMP_256,       L13_OCEAN_TEMP_256 = L_OCEAN_TEMP_256,  // 1.13+
    L_ZOOM_128_OCEAN,       L13_ZOOM_128 = L_ZOOM_128_OCEAN,        // 1.13+
    L_ZOOM_64_OCEAN,        L13_ZOOM_64 = L_ZOOM_64_OCEAN,          // 1.13+
    L_ZOOM_32_OCEAN,        L13_ZOOM_32 = L_ZOOM_32_OCEAN,          // 1.13+
    L_ZOOM_16_OCEAN,        L13_ZOOM_16 = L_ZOOM_16_OCEAN,          // 1.13+
    L_ZOOM_8_OCEAN,         L13_ZOOM_8 = L_ZOOM_8_OCEAN,            // 1.13+
    L_ZOOM_4_OCEAN,         L13_ZOOM_4 = L_ZOOM_4_OCEAN,            // 1.13+
    L_OCEAN_MIX_4,          L13_OCEAN_MIX_4 = L_OCEAN_MIX_4,        // 1.13+

    L_VORONOI_1,            L_VORONOI_ZOOM_1 = L_VORONOI_1,

    // largeBiomes layers
    L_ZOOM_LARGE_A,
    L_ZOOM_LARGE_B,
    L_ZOOM_L_RIVER_A,
    L_ZOOM_L_RIVER_B,

    L_NUM
};


struct Layer;
typedef int (mapfunc_t)(const struct Layer *, int *, int, int, int, int);

STRUCT(Layer)
{
    mapfunc_t *getMap;

    int8_t mc;          // minecraft version
    int8_t zoom;        // zoom factor of layer
    int8_t edge;        // maximum border required from parent layer
    int scale;          // scale of this layer (cell = scale x scale blocks)

    uint64_t layerSalt; // processed salt or initialization mode
    uint64_t startSalt; // (depends on world seed) used to step PRNG forward
    uint64_t startSeed; // (depends on world seed) start for chunk seeds

    void *noise;        // (depends on world seed) noise map data
    void *data;         // generic data for custom layers

    Layer *p, *p2;      // parent layers
};

// Overworld biome generator up to 1.17
STRUCT(LayerStack)
{
    Layer layers[L_NUM];
    Layer *entry_1;     // entry scale (1:1) [L_VORONOI_1]
    Layer *entry_4;     // entry scale (1:4) [L_RIVER_MIX_4|L_OCEAN_MIX_4]
    // unofficial entries for other scales (latest sensible layers):
    Layer *entry_16;    // [L_SWAMP_RIVER_16|L_SHORE_16]
    Layer *entry_64;    // [L_HILLS_64|L_SUNFLOWER_64]
    Layer *entry_256;   // [L_BIOME_256|L_BAMBOO_256]
    PerlinNoise oceanRnd;
};


#ifdef __cplusplus
extern "C"
{
#endif

//==============================================================================
// Essentials
//==============================================================================

/* Applies the given world seed to the layer and all dependent layers. */
void setLayerSeed(Layer *layer, uint64_t worldSeed);

//==============================================================================
// Layers
//==============================================================================

//                             old names
mapfunc_t mapContinent;     // mapIsland
mapfunc_t mapZoomFuzzy;
mapfunc_t mapZoom;
mapfunc_t mapLand;          // mapAddIsland
mapfunc_t mapLand16;
mapfunc_t mapLandB18;
mapfunc_t mapIsland;        // mapRemoveTooMuchOcean
mapfunc_t mapSnow;          // mapAddSnow
mapfunc_t mapSnow16;
mapfunc_t mapCool;          // mapCoolWarm
mapfunc_t mapHeat;          // mapHeatIce
mapfunc_t mapSpecial;
mapfunc_t mapMushroom;      // mapAddMushroomIsland
mapfunc_t mapDeepOcean;
mapfunc_t mapBiome;
mapfunc_t mapBamboo;        // mapAddBamboo
mapfunc_t mapNoise;         // mapRiverInit
mapfunc_t mapBiomeEdge;
mapfunc_t mapHills;
mapfunc_t mapRiver;
mapfunc_t mapSmooth;
mapfunc_t mapSunflower;     // mapRareBiome
mapfunc_t mapShore;
mapfunc_t mapSwampRiver;
mapfunc_t mapRiverMix;
mapfunc_t mapOceanTemp;
mapfunc_t mapOceanMix;

// final layer 1:1
mapfunc_t mapVoronoi;       // mapVoronoiZoom
mapfunc_t mapVoronoi114;

// With 1.15 voronoi changed in preparation for 3D biome generation.
// Biome generation now stops at scale 1:4 OceanMix and voronoi is just an
// access algorithm, mapping the 1:1 scale onto its 1:4 correspondent.
// It is seeded by the first 8-bytes of the SHA-256 hash of the world seed.
ATTR(const)
uint64_t getVoronoiSHA(uint64_t worldSeed);
void voronoiAccess3D(uint64_t sha, int x, int y, int z, int *x4, int *y4, int *z4);

// Applies a 2D voronoi mapping at height 'y' to a 'src' plane, where
// src_range [px,pz,pw,ph] -> out_range [x,z,w,h] have to match the scaling.
void mapVoronoiPlane(uint64_t sha, int *out, int *src,
    int x, int z, int w, int h, int y, int px, int pz, int pw, int ph);


#ifdef __cplusplus
}
#endif

#endif /* LAYER_H_ */
