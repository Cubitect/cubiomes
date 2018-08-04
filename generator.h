#ifndef GENERATOR_H_
#define GENERATOR_H_

#include "layers.h"

/* Minecraft versions */
enum
{
    MC_1_7, MC_1_8, MC_1_9, MC_1_10, MC_1_11, MC_1_12, MC_1_13
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
    L_REMOVE_TOO_MUCH_OCEAN_1024,
    L_ADD_SNOW_1024,
    L_ADD_ISLAND_1024D,
    L_COOL_WARM_1024,
    L_HEAT_ICE_1024,
    L_SPECIAL_1024,  /* Good entry for: temperature categories */
    L_ZOOM_512,
    L_ZOOM_256,
    L_ADD_ISLAND_256,
    L_ADD_MUSHROOM_ISLAND_256, /* Good entry for: mushroom biomes */
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

    L_NUM,

    // 1.13 layers
    L13_OCEAN_TEMP_256 = 43,
    L13_ZOOM_128,
    L13_ZOOM_64,
    L13_ZOOM_32,
    L13_ZOOM_16,
    L13_ZOOM_8,
    L13_ZOOM_4,
    L13_OCEAN_MIX_4,
    L13_VORONOI_ZOOM_1,

    L13_NUM
};


/******************************** BIOME TABLES *********************************
 * The biome tables below are lists of the biomes that can be present at some
 * notable layers. Of cause, layers that are applied later in the hierarchy will
 * also contain these biomes.
 */

//==============================================================================
// MC 1.13 Biome Tables
//==============================================================================

static const int BIOMES_L13_OCEAN_TEMP_256[] =
{
        ocean, frozenOcean, warmOcean, lukewarmOcean, coldOcean
};

static const int BIOMES_L13_OCEAN_MIX_4[] =
{
        ocean, plains, desert, extremeHills, forest, taiga, swampland, river, /*hell, sky,*/ // 0-9
        frozenOcean, frozenRiver, icePlains, iceMountains, mushroomIsland, mushroomIslandShore, beach, desertHills, forestHills, taigaHills,  // 10-19
        /*extremeHillsEdge,*/ jungle, jungleHills, jungleEdge, deepOcean, stoneBeach, coldBeach, birchForest, birchForestHills, roofedForest, // 20-29
        coldTaiga, coldTaigaHills, megaTaiga, megaTaigaHills, extremeHillsPlus, savanna, savannaPlateau, mesa, mesaPlateau_F, mesaPlateau, // 30-39
        /*skyIslandLow, skyIslandMedium, skyIslandHigh, skyIslandBarren,*/ warmOcean, lukewarmOcean, coldOcean, /*warmDeepOcean,*/ lukewarmDeepOcean, coldDeepOcean, // 40-49
        frozenDeepOcean,
        // Modified variants...
        plains+128, desert+128, extremeHills+128, forest+128, taiga+128, swampland+128,
        icePlains+128, jungle+128, jungleEdge+128, birchForest+128, birchForestHills+128, roofedForest+128,
        coldTaiga+128, megaTaiga+128, megaTaigaHills+128, extremeHillsPlus+128, savanna+128, savannaPlateau+128, mesa+128, mesaPlateau_F+128, mesaPlateau+128
};


//==============================================================================
// MC 1.7 Biome Tables
//==============================================================================

/* L_ADD_MUSHROOM_ISLAND_256 and L_DEEP_OCEAN_256
 * add the mushroomIsland and deepOcean biomes respectively, however the rest of
 * the biomes are incomplete and are better described by temperature categories
 * with special modifications bits.
 */

// BIOMES_L_BIOME_256: Changes temperature to weighted biomes:
// Warm         -> [desert, desert, desert, savanna, savanna, plains]
// Warm,special -> [mesaPlateau, mesaPlateau_F, mesaPlateau_F]
// Lush         -> [forest, roofedForest, extremeHills, plains, birchForest, swampland]
// Lush,special -> [jungle]
// Cold         -> [forest, extremeHills, taiga, plains]
// Cold,special -> [megaTaiga]
// Freezing     -> [icePlains, icePlains, icePlains, coldTaiga]
static const int BIOMES_L_BIOME_256[] =
{
        ocean, plains, desert, extremeHills, forest, taiga, swampland, /*river, hell, sky,*/ // 0-9
        /*frozenOcean, frozenRiver,*/ icePlains, /*iceMountains,*/ mushroomIsland, /*mushroomIslandShore, beach, desertHills, forestHills, taigaHills,*/  // 10-19
        /*extremeHillsEdge,*/ jungle, /*jungleHills, jungleEdge,*/ deepOcean, /*stoneBeach, coldBeach,*/ birchForest, /*birchForestHills,*/ roofedForest, // 20-29
        coldTaiga, /*coldTaigaHills,*/ megaTaiga, /*megaTaigaHills, extremeHillsPlus,*/ savanna, /*savannaPlateau, mesa,*/ mesaPlateau_F, mesaPlateau, // 30-39
};

// Introduces biomes: jungleEdge, extremeHillsPlus, mesa
static const int BIOMES_L_BIOME_EDGE_64[] =
{
        ocean, plains, desert, extremeHills, forest, taiga, swampland, /*river, hell, sky,*/ // 0-9
        /*frozenOcean, frozenRiver,*/ icePlains, /*iceMountains,*/ mushroomIsland, /*mushroomIslandShore, beach, desertHills, forestHills, taigaHills,*/  // 10-19
        /*extremeHillsEdge,*/ jungle, /*jungleHills,*/ jungleEdge, deepOcean, /*stoneBeach, coldBeach,*/ birchForest, /*birchForestHills,*/ roofedForest, // 20-29
        coldTaiga, /*coldTaigaHills,*/ megaTaiga, /*megaTaigaHills,*/ extremeHillsPlus, savanna, /*savannaPlateau,*/ mesa, mesaPlateau_F, mesaPlateau, // 30-39
};

// Introduces biomes: iceMountains, desertHills, forestHills, taigaHills,
// jungleHills, birchForestHills, coldTaigaHills, megaTaigaHills, savannaPlateau
// and all 21 mutated biomes
static const int BIOMES_L_HILLS_64[] =
{
        ocean, plains, desert, extremeHills, forest, taiga, swampland, /*river, hell, sky,*/ // 0-9
        /*frozenOcean, frozenRiver,*/ icePlains, iceMountains, mushroomIsland, /*mushroomIslandShore, beach,*/ desertHills, forestHills, taigaHills,  // 10-19
        /*extremeHillsEdge,*/ jungle, jungleHills, jungleEdge, deepOcean, /*stoneBeach, coldBeach,*/ birchForest, birchForestHills, roofedForest, // 20-29
        coldTaiga, coldTaigaHills, megaTaiga, megaTaigaHills, extremeHillsPlus, savanna, savannaPlateau, mesa, mesaPlateau_F, mesaPlateau, // 30-39
        // Modified variants...
        plains+128, desert+128, extremeHills+128, forest+128, taiga+128, swampland+128,
        icePlains+128, jungle+128, jungleEdge+128, birchForest+128, birchForestHills+128, roofedForest+128,
        coldTaiga+128, megaTaiga+128, megaTaigaHills+128, extremeHillsPlus+128, savanna+128, savannaPlateau+128, mesa+128, mesaPlateau_F+128, mesaPlateau+128
};

// Introduces biomes: mushroomIslandShore, beach, stoneBeach, coldBeach
static const int BIOMES_L_SHORE_16[] =
{
        ocean, plains, desert, extremeHills, forest, taiga, swampland, /*river, hell, sky,*/ // 0-9
        /*frozenOcean, frozenRiver,*/ icePlains, iceMountains, mushroomIsland, mushroomIslandShore, beach, desertHills, forestHills, taigaHills,  // 10-19
        /*extremeHillsEdge,*/ jungle, jungleHills, jungleEdge, deepOcean, stoneBeach, coldBeach, birchForest, birchForestHills, roofedForest, // 20-29
        coldTaiga, coldTaigaHills, megaTaiga, megaTaigaHills, extremeHillsPlus, savanna, savannaPlateau, mesa, mesaPlateau_F, mesaPlateau, // 30-39
        // Modified variants...
        plains+128, desert+128, extremeHills+128, forest+128, taiga+128, swampland+128,
        icePlains+128, jungle+128, jungleEdge+128, birchForest+128, birchForestHills+128, roofedForest+128,
        coldTaiga+128, megaTaiga+128, megaTaigaHills+128, extremeHillsPlus+128, savanna+128, savannaPlateau+128, mesa+128, mesaPlateau_F+128, mesaPlateau+128
};

// Merges the river branch and adds frozenRiver biome
static const int BIOMES_L_RIVER_MIX_4[] =
{
        ocean, plains, desert, extremeHills, forest, taiga, swampland, river, /*hell, sky,*/ // 0-9
        /*frozenOcean,*/ frozenRiver, icePlains, iceMountains, mushroomIsland, mushroomIslandShore, beach, desertHills, forestHills, taigaHills,  // 10-19
        /*extremeHillsEdge,*/ jungle, jungleHills, jungleEdge, deepOcean, stoneBeach, coldBeach, birchForest, birchForestHills, roofedForest, // 20-29
        coldTaiga, coldTaigaHills, megaTaiga, megaTaigaHills, extremeHillsPlus, savanna, savannaPlateau, mesa, mesaPlateau_F, mesaPlateau, // 30-39
        // Modified variants...
        plains+128, desert+128, extremeHills+128, forest+128, taiga+128, swampland+128,
        icePlains+128, jungle+128, jungleEdge+128, birchForest+128, birchForestHills+128, roofedForest+128,
        coldTaiga+128, megaTaiga+128, megaTaigaHills+128, extremeHillsPlus+128, savanna+128, savannaPlateau+128, mesa+128, mesaPlateau_F+128, mesaPlateau+128
};


STRUCT(LayerStack)
{
    Layer *layers;
    int layerNum;
};

/* Initialise an instance of a generator. */
LayerStack setupGenerator(const int mcversion);
LayerStack setupGeneratorMC17();
LayerStack setupGeneratorMC113();

/* Cleans up and frees the generator layers */
void freeGenerator(LayerStack g);


/* Calculates the minimum size of the buffers required to generate an area of
 * dimensions 'sizeX' by 'sizeZ' at the specified layer.
 */
int calcRequiredBuf(Layer *layer, int areaX, int areaZ);

/* Allocates an amount of memory required to generate an area of dimensions
 * 'sizeX' by 'sizeZ' for the magnification of the current top layer.
 */
int *allocCache(Layer *layer, int sizeX, int sizeZ);


/* Set up custom layers. */
void setupLayer(int scale, Layer *l, Layer *p, int s, void (*getMap)(Layer *layer, int *out, int x, int z, int w, int h));
void setupMultiLayer(int scale, Layer *l, Layer *p1, Layer *p2, int s, void (*getMap)(Layer *layer, int *out, int x, int z, int w, int h));


/* Sets the world seed for the generator */
void applySeed(LayerStack *g, int64_t seed);

/* Generates the specified area using the current generator settings and stores
 * the biomeIDs in 'out'.
 * The biomeIDs will be indexed in the form: out[x + z*areaWidth]
 * It is recommended that 'out' is allocated using allocCache() for the correct
 * buffer size.
 */
void genArea(Layer *layer, int *out, int areaX, int areaZ, int areaWidth, int areaHeight);


#endif /* GENERATOR_H_ */

