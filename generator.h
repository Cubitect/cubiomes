#ifndef GENERATOR_H_
#define GENERATOR_H_

#include "layers.h"

// generator flags
#define LARGE_BIOMES            0x1
#define FORCE_OCEAN_VARIANTS    0x4


STRUCT(Generator)
{
    int mc;
    int dim;
    uint32_t flags;
    uint64_t seed;
    uint64_t sha;

    union {
        struct { // MC 1.0 - 1.17
            LayerStack ls;
            Layer xlayer[5]; // buffer for custom entry layers @{1,4,16,64,256}
            Layer *entry;
        };
        struct { // MC 1.18
            BiomeNoise bn;
        };
    };
    NetherNoise nn; // MC 1.16
    EndNoise en; // MC 1.9
};


#ifdef __cplusplus
extern "C"
{
#endif

///=============================================================================
/// Biome Generation
///=============================================================================

/**
 * Sets up a biome generator for a given MC version. The 'flags' can be used to
 * control LARGE_BIOMES or to FORCE_OCEAN_VARIANTS to enable ocean variants at
 * scales higher than normal.
 */
void setupGenerator(Generator *g, int mc, uint32_t flags);

/**
 * Initializes the generator for a given dimension and seed.
 * dim=0:   Overworld
 * dim=-1:  Nether
 * dim=+1:  End
 */
void applySeed(Generator *g, int dim, uint64_t seed);

/**
 * Calculates the buffer size (number of ints) required to generate a cuboidal
 * volume of size (sx, sy, sz). If 'sy' is zero the buffer is calculated for a
 * 2D plane (which is equivalent to sy=1 here).
 * The function allocCache() can be used to allocate the corresponding int
 * buffer using malloc().
 */
size_t getMinCacheSize(const Generator *g, int scale, int sx, int sy, int sz);
int *allocCache(const Generator *g, Range r);

/**
 * Generates the biomes for a cuboidal scaled range given by 'r'.
 * (See description of Range for more detail.)
 *
 * The output is generated inside the cache. Upon success the biome ids can be
 * accessed by indexing as:
 *  cache[ y*r.sx*r.sz + z*r.sx + x ]
 * where (x,y,z) is an relative position inside the range cuboid.
 *
 * The required length of the cache can be determined with getMinCacheSize().
 *
 * The return value is zero upon success.
 */
int genBiomes(const Generator *g, int *cache, Range r);
/**
 * Gets the biome for a specified scaled position. Note that the scale should
 * be either 1 or 4, for block or biome coordinates respectively.
 * Returns none (-1) upon failure.
 */
int getBiomeAt(const Generator *g, int scale, int x, int y, int z);

/**
 * Returns the default layer that corresponds to the given scale.
 * Supported scales are {0, 1, 4, 16, 64, 256}. A scale of zero indicates the
 * custom entry layer 'g->entry'.
 * (Overworld, MC <= 1.17)
 */
const Layer *getLayerForScale(const Generator *g, int scale);


///=============================================================================
/// Layered Biome Generation (old interface up to 1.17)
///=============================================================================

/* Initialize an instance of a layered generator. */
void setupLayerStack(LayerStack *g, int mc, int largeBiomes);

/* Calculates the minimum size of the buffers required to generate an area of
 * dimensions 'sizeX' by 'sizeZ' at the specified layer.
 */
size_t getMinLayerCacheSize(const Layer *layer, int sizeX, int sizeZ);

/* Set up custom layers. */
Layer *setupLayer(Layer *l, mapfunc_t *map, int mc,
    int8_t zoom, int8_t edge, uint64_t saltbase, Layer *p, Layer *p2);

/* Generates the specified area using the current generator settings and stores
 * the biomeIDs in 'out'.
 * The biomeIDs will be indexed in the form: out[x + z*areaWidth]
 * It is recommended that 'out' is allocated using allocCache() for the correct
 * buffer size.
 */
int genArea(const Layer *layer, int *out, int areaX, int areaZ, int areaWidth, int areaHeight);



#ifdef __cplusplus
}
#endif

#endif /* GENERATOR_H_ */

