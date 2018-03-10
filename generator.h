#ifndef GENERATOR_H_
#define GENERATOR_H_

#include "layers.h"


STRUCT(Generator) {
	Layer *layers;
	int topLayerIndex;
	int layerMax;
	long rawSeed;
};

// Initialise an instance of a generator
Generator setupGenerator();
Generator setupGeneratorMC18();
Generator setupGeneratorMC113();


// Sets the top level layer of a generator.
void setTopLevel(Generator *g, int topLevel);

// Cleans up and frees the generator layers
void freeGenerator(Generator *g);

// Allocates an amount of memory required to generate an area of dimensions
// 'sizeX' by 'sizeZ' for the magnification of the current top layer.
int *allocCache(Generator *layer, int sizeX, int sizeZ);

// Set up custom layers
void setupLayer(Layer *l, Layer *p, int s, void (*getMap)(Layer *layer, int *out, int x, int z, int w, int h));
void setupMultiLayer(Layer *l, Layer *p1, Layer *p2, int s, void (*getMap)(Layer *layer, int *out, int x, int z, int w, int h));

// Calculates the minimum size of the buffers required to generate an area of dimensions
// 'sizeX' by 'sizeZ' at the scale of the layer.
int calcRequiredBuf(Layer *layer, int sizeX, int sizeZ);

// Sets the world seed for the generator
void applySeed(Generator *g, long seed);

/*
 * genArea
 * -------
 * Generates the specified area using the current generator settings and stores the biomeIDs in 'out'.
 * The biomeIDs will be indexed in the form: out[x + z*areaWidth]
 * It is recommended that 'out' is allocated using allocCache() for the correct buffer size.
 */
void genArea(Generator *g, int *out, int startX, int startZ, int areaWidth, int areaHeight);


#endif /* GENERATOR_H_ */

