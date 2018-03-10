#include "generator.h"
#include "layers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int *allocCache(Generator *g, int sizeX, int sizeZ)
{
    int size = calcRequiredBuf(&g->layers[g->topLayerIndex-1], sizeX, sizeZ);

    int *ret = (int*) malloc(sizeof(*ret)*size);
    memset(ret, 0, sizeof(*ret)*size);

    return ret;
}

void setupLayer(Layer *l, Layer *p, int s, void (*getMap)(Layer *layer, int *out, int x, int z, int w, int h))
{
    setBaseSeed(l, s);
    l->p = p;
    l->p2 = NULL;
    l->getMap = getMap;
}

void setupMultiLayer(Layer *l, Layer *p1, Layer *p2, int s, void (*getMap)(Layer *layer, int *out, int x, int z, int w, int h))
{
    setBaseSeed(l, s);
    l->p = p1;
    l->p2 = p2;
    l->getMap = getMap;
}

Generator setupGenerator()
{
    return setupGeneratorMC18();
}

Generator setupGeneratorMC18()
{
    if(biomes[plains].id == 0)
    {
        fprintf(stderr, "Warning: The biomes have to be initialised first using initBiomes() before any generator can be used.\n");
    }

    Generator g;

    g.rawSeed = 0;
    g.topLayerIndex = 44;
    g.layerMax = 44;
    g.layers = (Layer*) malloc(sizeof(Layer)*g.topLayerIndex);

    //            LAYER          PARENT      SEED  LAYER_FUNCTION
    setupLayer(&g.layers[ 0],          NULL,    1, mapIsland);
    setupLayer(&g.layers[ 1], &g.layers[ 0], 2000, mapZoom);
    setupLayer(&g.layers[ 2], &g.layers[ 1],    1, mapAddIsland);
    setupLayer(&g.layers[ 3], &g.layers[ 2], 2001, mapZoom);
    setupLayer(&g.layers[ 4], &g.layers[ 3],    2, mapAddIsland);
    setupLayer(&g.layers[ 5], &g.layers[ 4],   50, mapAddIsland);
    setupLayer(&g.layers[ 6], &g.layers[ 5],   70, mapAddIsland);
    setupLayer(&g.layers[ 7], &g.layers[ 6],    2, mapRemoveTooMuchOcean);
    setupLayer(&g.layers[ 8], &g.layers[ 7],    2, mapAddSnow);
    setupLayer(&g.layers[ 9], &g.layers[ 8],    3, mapAddIsland);
    setupLayer(&g.layers[10], &g.layers[ 9],    2, mapCoolWarm); // MAG1024
    setupLayer(&g.layers[11], &g.layers[10],    2, mapHeatIce);
    setupLayer(&g.layers[12], &g.layers[11],    3, mapSpecial);
    setupLayer(&g.layers[13], &g.layers[12], 2002, mapZoom);
    setupLayer(&g.layers[14], &g.layers[13], 2003, mapZoom);
    setupLayer(&g.layers[15], &g.layers[14],    4, mapAddIsland);
    setupLayer(&g.layers[16], &g.layers[15],    5, mapAddMushroomIsland); // MAGSHROOM
    setupLayer(&g.layers[17], &g.layers[16],    4, mapDeepOcean);
    // biome layer chain
    setupLayer(&g.layers[18], &g.layers[17],  200, mapBiome); // MAG256
    setupLayer(&g.layers[19], &g.layers[18], 1000, mapZoom);
    setupLayer(&g.layers[20], &g.layers[19], 1001, mapZoom);
    setupLayer(&g.layers[21], &g.layers[20], 1000, mapBiomeEdge);

    // basic river layer chain, used to determine where hills generate
    setupLayer(&g.layers[22], &g.layers[17],  100, mapRiverInit);
    setupLayer(&g.layers[23], &g.layers[22], 1000, mapZoom);
    setupLayer(&g.layers[24], &g.layers[23], 1001, mapZoom);

    setupMultiLayer(&g.layers[25], &g.layers[21], &g.layers[24], 1000, mapHills); // MAG64

    setupLayer(&g.layers[26], &g.layers[25], 1001, mapRareBiome);
    setupLayer(&g.layers[27], &g.layers[26], 1000, mapZoom);
    setupLayer(&g.layers[28], &g.layers[27],    3, mapAddIsland);
    setupLayer(&g.layers[29], &g.layers[28], 1001, mapZoom);
    setupLayer(&g.layers[30], &g.layers[29], 1000, mapShore);
    setupLayer(&g.layers[31], &g.layers[30], 1002, mapZoom);
    setupLayer(&g.layers[32], &g.layers[31], 1003, mapZoom);
    setupLayer(&g.layers[33], &g.layers[32], 1000, mapSmooth);

    // river layer chain
    setupLayer(&g.layers[34], &g.layers[22], 1000, mapZoom);
    setupLayer(&g.layers[35], &g.layers[34], 1001, mapZoom);
    setupLayer(&g.layers[36], &g.layers[35], 1000, mapZoom);
    setupLayer(&g.layers[37], &g.layers[36], 1001, mapZoom);
    setupLayer(&g.layers[38], &g.layers[37], 1002, mapZoom); // MAG16
    setupLayer(&g.layers[39], &g.layers[38], 1003, mapZoom);
    setupLayer(&g.layers[40], &g.layers[39],    1, mapRiver);
    setupLayer(&g.layers[41], &g.layers[40], 1000, mapSmooth);

    setupMultiLayer(&g.layers[42], &g.layers[33], &g.layers[41], 100, mapRiverMix);
    setupLayer(&g.layers[43], &g.layers[42],   10, mapVoronoiZoom);

    return g;
}


Generator setupGeneratorMC113()
{
    if(biomes[plains].id == 0)
    {
        fprintf(stderr, "Warning: The biomes have to be initialised first using initBiomes() before any generator can be used.\n");
    }

    Generator g;

    g.rawSeed = 0;
    g.topLayerIndex = 53;
    g.layerMax = 53;
    g.layers = (Layer*) malloc(sizeof(Layer)*g.topLayerIndex);

    //            LAYER          PARENT      SEED  LAYER_FUNCTION
    setupLayer(&g.layers[ 0],          NULL,    1, mapIsland);
    setupLayer(&g.layers[ 1], &g.layers[ 0], 2000, mapZoom);
    setupLayer(&g.layers[ 2], &g.layers[ 1],    1, mapAddIsland);
    setupLayer(&g.layers[ 3], &g.layers[ 2], 2001, mapZoom);
    setupLayer(&g.layers[ 4], &g.layers[ 3],    2, mapAddIsland);
    setupLayer(&g.layers[ 5], &g.layers[ 4],   50, mapAddIsland);
    setupLayer(&g.layers[ 6], &g.layers[ 5],   70, mapAddIsland);
    setupLayer(&g.layers[ 7], &g.layers[ 6],    2, mapRemoveTooMuchOcean);

    setupLayer(&g.layers[ 8], &g.layers[ 7],    2, mapAddSnow);
    setupLayer(&g.layers[ 9], &g.layers[ 8],    3, mapAddIsland);
    setupLayer(&g.layers[10], &g.layers[ 9],    2, mapCoolWarm); // MAG1024
    setupLayer(&g.layers[11], &g.layers[10],    2, mapHeatIce);
    setupLayer(&g.layers[12], &g.layers[11],    3, mapSpecial);
    setupLayer(&g.layers[13], &g.layers[12], 2002, mapZoom);
    setupLayer(&g.layers[14], &g.layers[13], 2003, mapZoom);
    setupLayer(&g.layers[15], &g.layers[14],    4, mapAddIsland);
    setupLayer(&g.layers[16], &g.layers[15],    5, mapAddMushroomIsland); // MAGSHROOM
    setupLayer(&g.layers[17], &g.layers[16],    4, mapDeepOcean);
    // biome layer chain
    setupLayer(&g.layers[18], &g.layers[17],  200, mapBiome); // MAG256
    setupLayer(&g.layers[19], &g.layers[18], 1000, mapZoom);
    setupLayer(&g.layers[20], &g.layers[19], 1001, mapZoom);
    setupLayer(&g.layers[21], &g.layers[20], 1000, mapBiomeEdge);

    // basic river layer chain, used to determine where hills generate
    setupLayer(&g.layers[22], &g.layers[17],  100, mapRiverInit);
    setupLayer(&g.layers[23], &g.layers[22], 1000, mapZoom);
    setupLayer(&g.layers[24], &g.layers[23], 1001, mapZoom);

    setupMultiLayer(&g.layers[25], &g.layers[21], &g.layers[24], 1000, mapHills); // MAG64

    setupLayer(&g.layers[26], &g.layers[25], 1001, mapRareBiome);
    setupLayer(&g.layers[27], &g.layers[26], 1000, mapZoom);
    setupLayer(&g.layers[28], &g.layers[27],    3, mapAddIsland);
    setupLayer(&g.layers[29], &g.layers[28], 1001, mapZoom);
    setupLayer(&g.layers[30], &g.layers[29], 1000, mapShore);
    setupLayer(&g.layers[31], &g.layers[30], 1002, mapZoom);
    setupLayer(&g.layers[32], &g.layers[31], 1003, mapZoom);
    setupLayer(&g.layers[33], &g.layers[32], 1000, mapSmooth);

    // river layer chain
    setupLayer(&g.layers[34], &g.layers[22], 1000, mapZoom);
    setupLayer(&g.layers[35], &g.layers[34], 1001, mapZoom);
    setupLayer(&g.layers[36], &g.layers[35], 1000, mapZoom);
    setupLayer(&g.layers[37], &g.layers[36], 1001, mapZoom);
    setupLayer(&g.layers[38], &g.layers[37], 1002, mapZoom); // MAG16
    setupLayer(&g.layers[39], &g.layers[38], 1003, mapZoom);
    setupLayer(&g.layers[40], &g.layers[39],    1, mapRiver);
    setupLayer(&g.layers[41], &g.layers[40], 1000, mapSmooth);

    setupMultiLayer(&g.layers[42], &g.layers[33], &g.layers[41], 100, mapRiverMix);

    // ocean variants
    // This branch is connected to layer 7, but makes no use of it (only size).
    // I.e this branch is independent of previous layers.
    setupLayer(&g.layers[43], &g.layers[ 7],    2, mapOceanTemp);
    setupLayer(&g.layers[44], &g.layers[43],    2, mapEdgeOcean);
    setupLayer(&g.layers[45], &g.layers[44], 2001, mapZoom);
    setupLayer(&g.layers[46], &g.layers[45], 2002, mapZoom);
    setupLayer(&g.layers[47], &g.layers[46], 2003, mapZoom);
    setupLayer(&g.layers[48], &g.layers[47], 2004, mapZoom);
    setupLayer(&g.layers[49], &g.layers[48], 2005, mapZoom);
    setupLayer(&g.layers[50], &g.layers[49], 2006, mapZoom);

    setupMultiLayer(&g.layers[51], &g.layers[42], &g.layers[50], 100, mapOceanMix);

    setupLayer(&g.layers[52], &g.layers[51],   10, mapVoronoiZoom);

    return g;
}


void setTopLevel(Generator *g, int topLevel)
{
    if(topLevel > g->layerMax)
    {
        printf("Warning: Top layer index is greater than the number of layers in this generator.");
    }
    else
    {
        g->topLayerIndex = topLevel;
    }
}



static void getMaxArea(Layer *layer, int areaX, int areaZ, int *maxX, int *maxZ)
{
    if(layer == NULL)
        return;

    if(layer->getMap == mapZoom)
    {
        areaX = (areaX >> 1) + 2;
        areaZ = (areaZ >> 1) + 2;
    }
    else if(layer->getMap == mapVoronoiZoom)
    {
        areaX = (areaX >> 2) + 2;
        areaZ = (areaZ >> 2) + 2;
    }
    else
    {
        if( layer->getMap != mapIsland &&
            layer->getMap != mapSpecial &&
            layer->getMap != mapBiome &&
            layer->getMap != mapRiverInit &&
            layer->getMap != mapRiverMix )
        {
            areaX += 2;
            areaZ += 2;
        }
    }

    if(areaX > *maxX) *maxX = areaX;
    if(areaZ > *maxZ) *maxZ = areaZ;

    getMaxArea(layer->p, areaX, areaZ, maxX, maxZ);
    getMaxArea(layer->p2, areaX, areaZ, maxX, maxZ);
}


int calcRequiredBuf(Layer *layer, int areaX, int areaZ)
{
    int maxX = areaX, maxZ = areaZ;
    getMaxArea(layer, areaX, areaZ, &maxX, &maxZ);

    return maxX * maxZ;
}

void freeGenerator(Generator *g)
{
    free(g->layers);
}

void applySeed(Generator *g, long seed)
{
    g->rawSeed = seed;

    // the seed has to be applied recursively
    setWorldSeed(&g->layers[g->topLayerIndex-1], seed);
}

void genArea(Generator *g, int *out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    Layer *l = &g->layers[g->topLayerIndex-1];
    memset(out, 0, areaWidth*areaHeight*sizeof(*out));
    l->getMap(l, out, areaX, areaZ, areaWidth, areaHeight);
}





