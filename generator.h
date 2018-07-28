#ifndef GENERATOR_H_
#define GENERATOR_H_

#include "layers.h"

enum
{
    MC_1_7, MC_1_8, MC_1_9, MC_1_10, MC_1_11, MC_1_12, MC_1_13
};

/* Enumeration of the layer indices in the generator.
 */
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

