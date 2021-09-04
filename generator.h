#ifndef GENERATOR_H_
#define GENERATOR_H_

#include "layers.h"


/* Enumeration of the layer indices in the generator. */
enum
{
    // new                  [[deprecated]]
    L_CONTINENT_4096 = 0,   L_ISLAND_4096 = L_CONTINENT_4096,
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

/* Initialise an instance of a generator. */
void setupGenerator(LayerStack *g, int mc);

/* Initialise an instance of a generator with largeBiomes configuration. */
void setupGeneratorLargeBiomes(LayerStack *g, int mc, int largeBiomes);


/* Calculates the minimum size of the buffers required to generate an area of
 * dimensions 'sizeX' by 'sizeZ' at the specified layer.
 */
size_t calcRequiredBuf(const Layer *layer, int areaX, int areaZ);

/* Allocates an amount of memory required to generate an area of dimensions
 * 'sizeX' by 'sizeZ' for the magnification of the given layer.
 */
int *allocCache(const Layer *layer, int sizeX, int sizeZ);


/* Set up custom layers. */
Layer *setupLayer(LayerStack *g, int layerId, mapfunc_t *map, int mc,
    int8_t zoom, int8_t edge, uint64_t saltbase, Layer *p, Layer *p2);

/* Sets the world seed for the generator */
void applySeed(LayerStack *g, uint64_t seed);

/* Generates the specified area using the current generator settings and stores
 * the biomeIDs in 'out'.
 * The biomeIDs will be indexed in the form: out[x + z*areaWidth]
 * It is recommended that 'out' is allocated using allocCache() for the correct
 * buffer size.
 */
int genArea(const Layer *layer, int *out, int areaX, int areaZ, int areaWidth, int areaHeight);


/* Generate nether or end biomes at scales: 1:1, 1:4, 1:16, or 1:64
 * @mc          minecaft version
 * @seed        world seed
 * @scale       mapping scale of output, has to be one of 1, 4, 16, or 64
 * @out         output buffer, out[yi*w*h + zi*w + xi], size = w*h*(y1-y0+1)
 *              for voronoi (scale=1) add 7 to each dimension as buffer
 * @x,z,w,h     planar area
 * @y0,y1       min and max vertical dimensions (inclusive)
 * @return      zero upon success
 */
int genNetherScaled(int mc, uint64_t seed, int scale, int *out,
        int x, int z, int w, int h, int y0, int y1);
int genEndScaled(int mc, uint64_t seed, int scale, int *out,
        int x, int z, int w, int h);


#ifdef __cplusplus
}
#endif

#endif /* GENERATOR_H_ */

