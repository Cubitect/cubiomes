#ifndef GENERATOR_H_
#define GENERATOR_H_

#include "layers.h"

/* Minecraft versions */
enum MCversion
{
    MC_1_7, MC_1_8, MC_1_9, MC_1_10, MC_1_11, MC_1_12, MC_1_13, MC_1_14,
    MC_1_15, MC_1_16,
    MC_BE = 128
};

/* Enumeration of the layer indices in the generator. */
enum
{
    L_ISLAND_4096 = 0,
    L_ZOOM_2048,
    L_ADD_ISLAND_2048,
    L_ZOOM_1024,
    L_ADD_ISLAND_1024A,
    L_ADD_ISLAND_1024B,
    L_ADD_ISLAND_1024C,
    L_REMOVE_OCEAN_1024,
    L_ADD_SNOW_1024,
    L_ADD_ISLAND_1024D,
    L_COOL_WARM_1024,
    L_HEAT_ICE_1024,
    L_SPECIAL_1024,  /* Good entry for: temperature categories */
    L_ZOOM_512,
    L_ZOOM_256,
    L_ADD_ISLAND_256,
    L_ADD_MUSHROOM_256, /* Good entry for: mushroom biomes */
    L_DEEP_OCEAN_256,
    L_BIOME_256, /* Good entry for: major biome types */
    L_ZOOM_128,
    L_ZOOM_64,
    L_BIOME_EDGE_64,
    L_RIVER_INIT_256,
    L_ZOOM_128_HILLS,
    L_ZOOM_64_HILLS,
    L_HILLS_64, /* Good entry for: minor biome types */
    L_RARE_BIOME_64,
    L_ZOOM_32,
    L_ADD_ISLAND_32,
    L_ZOOM_16,
    L_SHORE_16,
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
    L_VORONOI_ZOOM_1,

    // 1.13 layers
    L13_OCEAN_TEMP_256,
    L13_ZOOM_128,
    L13_ZOOM_64,
    L13_ZOOM_32,
    L13_ZOOM_16,
    L13_ZOOM_8,
    L13_ZOOM_4,
    L13_OCEAN_MIX_4,

    // 1.14 layers
    L14_BAMBOO_256,

    // largeBiomes layers
    L_ZOOM_LARGE_BIOME_A,
    L_ZOOM_LARGE_BIOME_B,

    L_NUM
};


STRUCT(LayerStack)
{
    Layer layers[L_NUM];
    Layer *entry_1; // entry layer, scale (1:1) [L_VORONOI_ZOOM_1]
    Layer *entry_4; // entry layer, scale (1:4) [L_RIVER_MIX_4|L13_OCEAN_MIX_4]
    PerlinNoise oceanRnd;
};

typedef int (*mapfunc_t)(const Layer *, int *, int, int, int, int);


#ifdef __cplusplus
extern "C"
{
#endif

/* Initialise an instance of a generator. */
void setupGenerator(LayerStack *g, int mcversion);

/* Initialise an instance of a generator with largeBiomes configuration. */
void setupLargeBiomesGenerator(LayerStack *g, int mcversion);


/* Calculates the minimum size of the buffers required to generate an area of
 * dimensions 'sizeX' by 'sizeZ' at the specified layer.
 */
int calcRequiredBuf(const Layer *layer, int areaX, int areaZ);

/* Allocates an amount of memory required to generate an area of dimensions
 * 'sizeX' by 'sizeZ' for the magnification of the given layer.
 */
int *allocCache(const Layer *layer, int sizeX, int sizeZ);


/* Set up custom layers. */
void setupLayer(Layer *l, Layer *p, int s, mapfunc_t getMap);
void setupMultiLayer(Layer *l, Layer *p1, Layer *p2, int s, mapfunc_t getMap);

/* Sets the world seed for the generator */
void applySeed(LayerStack *g, int64_t seed);

/* Generates the specified area using the current generator settings and stores
 * the biomeIDs in 'out'.
 * The biomeIDs will be indexed in the form: out[x + z*areaWidth]
 * It is recommended that 'out' is allocated using allocCache() for the correct
 * buffer size.
 */
int genArea(const Layer *layer, int *out, int areaX, int areaZ, int areaWidth, int areaHeight);



static inline int isOverworldBiome(int mc, int id)
{
    // check if the biome actually generates in this version
    switch (id)
    {
    case ocean...river:                                 return 1;
    case frozen_ocean:                                  return mc >= MC_1_13;
    case frozen_river...badlands_plateau:               return id != mountain_edge;
    case warm_ocean...deep_frozen_ocean:                return mc >= MC_1_13 && id != deep_warm_ocean;
    case sunflower_plains...modified_badlands_plateau:  return 1;
    case bamboo_jungle...bamboo_jungle_hills:           return mc >= MC_1_14;
    default:
        return 0;
    }
}

/******************************** BIOME TABLES *********************************
 * The biome tables below are lists of the biomes that can be present at some
 * notable layers. Of cause, layers that are applied later in the hierarchy will
 * also contain these biomes.
 */


//==============================================================================
// MC Biome Tables
//==============================================================================

/* L_ADD_MUSHROOM_ISLAND_256 and L_DEEP_OCEAN_256
 * add the mushroom_fields and deep_ocean biomes respectively, however the rest of
 * the biomes are incomplete and are better described by temperature categories
 * with special modifications bits.
 */

// BIOMES_L_BIOME_256: Changes temperature to weighted biomes:
// Warm         -> [desert, desert, desert, savanna, savanna, plains]
// Warm,special -> [badlands_plateau, wooded_badlands_plateau, wooded_badlands_plateau]
// Lush         -> [forest, dark_forest, mountains, plains, birch_forest, swamp]
// Lush,special -> [jungle]
// Cold         -> [forest, mountains, taiga, plains]
// Cold,special -> [giant_tree_taiga]
// Freezing     -> [snowy_tundra, snowy_tundra, snowy_tundra, snowy_taiga]
static const int BIOMES_L_BIOME_256[] =
{
        ocean, plains, desert, mountains, forest, taiga, swamp, /*river, hell, sky,*/ // 0-9
        /*frozen_ocean, frozen_river,*/ snowy_tundra, /*snowy_mountains,*/ mushroom_fields, /*mushroom_field_shore, beach, desert_hills, wooded_hills, taiga_hills,*/  // 10-19
        /*mountain_edge,*/ jungle, /*jungle_hills, jungleEdge,*/ deep_ocean, /*stone_shore, snowy_beach,*/ birch_forest, /*birch_forest_hills,*/ dark_forest, // 20-29
        snowy_taiga, /*snowy_taiga_hills,*/ giant_tree_taiga, /*giant_tree_taiga_hills, wooded_mountains,*/ savanna, /*savanna_plateau, badlands,*/ wooded_badlands_plateau, badlands_plateau, // 30-39
};

// Introduces biomes: jungle_edge, wooded_mountains, badlands
// Inherits new biomes: bamboo_jungle
static const int BIOMES_L_BIOME_EDGE_64[] =
{
        ocean, plains, desert, mountains, forest, taiga, swamp, /*river, hell, sky,*/ // 0-9
        /*frozen_ocean, frozen_river,*/ snowy_tundra, /*snowy_mountains,*/ mushroom_fields, /*mushroom_field_shore, beach, desert_hills, wooded_hills, taiga_hills,*/  // 10-19
        /*mountain_edge,*/ jungle, /*jungle_hills,*/ jungle_edge, deep_ocean, /*stone_shore, snowy_beach,*/ birch_forest, /*birch_forest_hills,*/ dark_forest, // 20-29
        snowy_taiga, /*snowy_taiga_hills,*/ giant_tree_taiga, /*giant_tree_taiga_hills,*/ wooded_mountains, savanna, /*savanna_plateau,*/ badlands, wooded_badlands_plateau, badlands_plateau, // 30-39
        bamboo_jungle, // 168
};

// Introduces biomes: snowy_mountains, desert_hills, wooded_hills, taiga_hills,
// jungle_hills, birch_forest_hills, snowy_taiga_hills, giant_tree_taiga_hills, savanna_plateau,
// bamboo_jungle_hills
// and all 21 mutated biomes
static const int BIOMES_L_HILLS_64[] =
{
        ocean, plains, desert, mountains, forest, taiga, swamp, /*river, hell, sky,*/ // 0-9
        /*frozen_ocean, frozen_river,*/ snowy_tundra, snowy_mountains, mushroom_fields, /*mushroom_field_shore, beach,*/ desert_hills, wooded_hills, taiga_hills,  // 10-19
        /*mountain_edge,*/ jungle, jungle_hills, jungle_edge, deep_ocean, /*stone_shore, snowy_beach,*/ birch_forest, birch_forest_hills, dark_forest, // 20-29
        snowy_taiga, snowy_taiga_hills, giant_tree_taiga, giant_tree_taiga_hills, wooded_mountains, savanna, savanna_plateau, badlands, wooded_badlands_plateau, badlands_plateau, // 30-39

        // Modified variants...
        plains+128, desert+128, mountains+128, forest+128, taiga+128, swamp+128,
        snowy_tundra+128, jungle+128, jungleEdge+128, birch_forest+128, birch_forest_hills+128, dark_forest+128,
        snowy_taiga+128, giant_tree_taiga+128, giant_tree_taiga_hills+128, wooded_mountains+128, savanna+128, savanna_plateau+128, badlands+128, wooded_badlands_plateau+128, badlands_plateau+128,

        bamboo_jungle, bamboo_jungle_hills, // 168, 169
};

// Introduces biomes: mushroom_field_shore, beach, stone_shore, snowy_beach
// Inherits new biiomes: sunflower_plains
static const int BIOMES_L_SHORE_16[] =
{
        ocean, plains, desert, mountains, forest, taiga, swamp, /*river, hell, sky,*/ // 0-9
        /*frozen_ocean, frozen_river,*/ snowy_tundra, snowy_mountains, mushroom_fields, mushroom_field_shore, beach, desert_hills, wooded_hills, taiga_hills,  // 10-19
        /*mountain_edge,*/ jungle, jungle_hills, jungle_edge, deep_ocean, stone_shore, snowy_beach, birch_forest, birch_forest_hills, dark_forest, // 20-29
        snowy_taiga, snowy_taiga_hills, giant_tree_taiga, giant_tree_taiga_hills, wooded_mountains, savanna, savanna_plateau, badlands, wooded_badlands_plateau, badlands_plateau, // 30-39
        // Modified variants...
        plains+128, desert+128, mountains+128, forest+128, taiga+128, swamp+128,
        snowy_tundra+128, jungle+128, jungleEdge+128, birch_forest+128, birch_forest_hills+128, dark_forest+128,
        snowy_taiga+128, giant_tree_taiga+128, giant_tree_taiga_hills+128, wooded_mountains+128, savanna+128, savanna_plateau+128, badlands+128, wooded_badlands_plateau+128, badlands_plateau+128,

        bamboo_jungle, bamboo_jungle_hills, // 168, 169
};

// Merges the river branch and adds frozen_river biome
static const int BIOMES_L_RIVER_MIX_4[] =
{
        ocean, plains, desert, mountains, forest, taiga, swamp, river, /*hell, sky,*/ // 0-9
        /*frozen_ocean,*/ frozen_river, snowy_tundra, snowy_mountains, mushroom_fields, mushroom_field_shore, beach, desert_hills, wooded_hills, taiga_hills,  // 10-19
        /*mountain_edge,*/ jungle, jungle_hills, jungle_edge, deep_ocean, stone_shore, snowy_beach, birch_forest, birch_forest_hills, dark_forest, // 20-29
        snowy_taiga, snowy_taiga_hills, giant_tree_taiga, giant_tree_taiga_hills, wooded_mountains, savanna, savanna_plateau, badlands, wooded_badlands_plateau, badlands_plateau, // 30-39
        // Modified variants...
        plains+128, desert+128, mountains+128, forest+128, taiga+128, swamp+128,
        snowy_tundra+128, jungle+128, jungleEdge+128, birch_forest+128, birch_forest_hills+128, dark_forest+128,
        snowy_taiga+128, giant_tree_taiga+128, giant_tree_taiga_hills+128, wooded_mountains+128, savanna+128, savanna_plateau+128, badlands+128, wooded_badlands_plateau+128, badlands_plateau+128,

        bamboo_jungle, bamboo_jungle_hills, // 168, 169
};

// Merges ocean variants
static const int BIOMES_L13_OCEAN_MIX_4[] =
{
        ocean, plains, desert, mountains, forest, taiga, swamp, river, /*hell, sky,*/ // 0-9
        frozen_ocean, frozen_river, snowy_tundra, snowy_mountains, mushroom_fields, mushroom_field_shore, beach, desert_hills, wooded_hills, taiga_hills,  // 10-19
        /*mountain_edge,*/ jungle, jungle_hills, jungle_edge, deep_ocean, stone_shore, snowy_beach, birch_forest, birch_forest_hills, dark_forest, // 20-29
        snowy_taiga, snowy_taiga_hills, giant_tree_taiga, giant_tree_taiga_hills, wooded_mountains, savanna, savanna_plateau, badlands, wooded_badlands_plateau, badlands_plateau, // 30-39
        /*skyIslandLow, skyIslandMedium, skyIslandHigh, skyIslandBarren,*/ warm_ocean, lukewarm_ocean, cold_ocean, /*deep_warm_ocean,*/ deep_lukewarm_ocean, deep_cold_ocean, // 40-49
        deep_frozen_ocean,
        // Modified variants...
        plains+128, desert+128, mountains+128, forest+128, taiga+128, swamp+128,
        snowy_tundra+128, jungle+128, jungleEdge+128, birch_forest+128, birch_forest_hills+128, dark_forest+128,
        snowy_taiga+128, giant_tree_taiga+128, giant_tree_taiga_hills+128, wooded_mountains+128, savanna+128, savanna_plateau+128, badlands+128, wooded_badlands_plateau+128, badlands_plateau+128,

        bamboo_jungle, bamboo_jungle_hills, // 168, 169
};


#ifdef __cplusplus
}
#endif

#endif /* GENERATOR_H_ */

