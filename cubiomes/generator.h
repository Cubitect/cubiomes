/**
 * generator.h
 *
 * Author: Cubitect
 * Created on: 17 Feb 2016
 * Licence: GPLv3
 */

#ifndef GENERATOR_H_
#define GENERATOR_H_

#include "layers.h"

/* If speed is more important than accuracy, here are some magnification modes for the generator.
 * The map generation is done in stages (layers), some of which zoom into the map, magnifying it.
 * These magnification modes control at which stage/magnification the map generation is aborted.
 *
 * Notes on magnification modes:
 *     MAG1    -  1 to 1 scale, exact map of the biomes
 *     MAG4    -  1 to 4 scale, used by some vanilla mc-generation as a faster alternative to the full scale map
 *     MAG16   -  1 to 16 scale, contains all biomes, except rivers
 *     MAG64   -  1 to 64 scale, misses oceanic features like beaches, small islands and rare biomes (Sunflower Plains).
 *     MAG256  -  1 to 256 scale, contains all the basic biomes, no variations (e.g. no hills and no mutations).
 *     MAGSHROOM, 1 to 256 scale, exploits that Mushroom Islands are determined before most other biomes.
 *     MAG1024 -  1 to 1024 scale, extreme scale that only contains basic information where biome types generate.
 */
enum Magnification { MAG1, MAG4, MAG16, MAG64, MAG256, MAGSHROOM, MAG1024 };

STRUCT(Generator) {
    Layer *layers;
    int layerNum;
    int layerMax;
    int mag;
    long rawSeed;
};

// Initialise an instance of a generator
Generator setupGenerator();

// Sets the magnification mode of the generator
void setGenScale(Generator *g, int magnification);

// Cleans up and frees the generator layers
void freeGenerator(Generator *g);

// Allocates an amount of memory required to generate an area of dimensions 'sizeX' by 'sizeZ'
int *allocCache(Generator *g, int sizeX, int sizeZ);

// Calculates the minimum size of the buffers required to generate an area of dimensions 'sizeX' by 'sizeZ'
int calcRequiredBuf(Generator *g, int sizeX, int sizeZ);

// Sets the world seed for the generator
void applySeed(Generator *g, long seed);

/**
 * genArea
 * -------
 * Generates the specified area using the current generator settings and stores the biomeIDs in 'out'.
 * The biomeIDs will be indexed in the form: out[x + z*areaWidth]
 * It is recommended that 'out' is allocated using allocCache() for the correct buffer size.
 */
void genArea(Generator *g, int *out, int startX, int startZ, int areaWidth, int areaHeight);

#endif /* GENERATOR_H_ */
