#include "generator.h"
#include "layers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void setupLayer(int scale, Layer *l, Layer *p, int s, void (*getMap)(Layer *layer, int *out, int x, int z, int w, int h))
{
    setBaseSeed(l, s);
    l->scale = scale;
    l->p = p;
    l->p2 = NULL;
    l->getMap = getMap;
    l->oceanRnd = NULL;
}

void setupMultiLayer(int scale, Layer *l, Layer *p1, Layer *p2, int s, void (*getMap)(Layer *layer, int *out, int x, int z, int w, int h))
{
    setBaseSeed(l, s);
    l->scale = scale;
    l->p = p1;
    l->p2 = p2;
    l->getMap = getMap;
    l->oceanRnd = NULL;
}


LayerStack setupGenerator(const int mcversion)
{
    if (mcversion <= MC_1_12)
        return setupGeneratorMC17();
    else
        return setupGeneratorMC113();
}


LayerStack setupGeneratorMC17()
{
    if (biomes[plains].id == 0)
    {
        fprintf(stderr, "Warning: The biomes have to be initialised first using initBiomes() before any generator can be used.\n");
    }

    LayerStack g;

    g.layerNum = 44;
    g.layers = (Layer*) malloc(sizeof(Layer)*g.layerNum);

    //         SCALE    LAYER          PARENT      SEED  LAYER_FUNCTION
    setupLayer(4096, &g.layers[ 0],          NULL,    1, mapIsland);
    setupLayer(2048, &g.layers[ 1], &g.layers[ 0], 2000, mapZoom);
    setupLayer(2048, &g.layers[ 2], &g.layers[ 1],    1, mapAddIsland);
    setupLayer(1024, &g.layers[ 3], &g.layers[ 2], 2001, mapZoom);
    setupLayer(1024, &g.layers[ 4], &g.layers[ 3],    2, mapAddIsland);
    setupLayer(1024, &g.layers[ 5], &g.layers[ 4],   50, mapAddIsland);
    setupLayer(1024, &g.layers[ 6], &g.layers[ 5],   70, mapAddIsland);
    setupLayer(1024, &g.layers[ 7], &g.layers[ 6],    2, mapRemoveTooMuchOcean);
    setupLayer(1024, &g.layers[ 8], &g.layers[ 7],    2, mapAddSnow);
    setupLayer(1024, &g.layers[ 9], &g.layers[ 8],    3, mapAddIsland);
    setupLayer(1024, &g.layers[10], &g.layers[ 9],    2, mapCoolWarm);
    setupLayer(1024, &g.layers[11], &g.layers[10],    2, mapHeatIce);
    setupLayer(1024, &g.layers[12], &g.layers[11],    3, mapSpecial);
    setupLayer( 512, &g.layers[13], &g.layers[12], 2002, mapZoom);
    setupLayer( 256, &g.layers[14], &g.layers[13], 2003, mapZoom);
    setupLayer( 256, &g.layers[15], &g.layers[14],    4, mapAddIsland);
    setupLayer( 256, &g.layers[16], &g.layers[15],    5, mapAddMushroomIsland);
    setupLayer( 256, &g.layers[17], &g.layers[16],    4, mapDeepOcean);
    // biome layer chain
    setupLayer( 256, &g.layers[18], &g.layers[17],  200, mapBiome);
    setupLayer( 128, &g.layers[19], &g.layers[18], 1000, mapZoom);
    setupLayer(  64, &g.layers[20], &g.layers[19], 1001, mapZoom);
    setupLayer(  64, &g.layers[21], &g.layers[20], 1000, mapBiomeEdge);

    // basic river layer chain, used to determine where hills generate
    setupLayer( 256, &g.layers[22], &g.layers[17],  100, mapRiverInit);
    setupLayer( 128, &g.layers[23], &g.layers[22], 1000, mapZoom);
    setupLayer(  64, &g.layers[24], &g.layers[23], 1001, mapZoom);

    setupMultiLayer(64, &g.layers[25], &g.layers[21], &g.layers[24], 1000, mapHills);

    setupLayer(  64, &g.layers[26], &g.layers[25], 1001, mapRareBiome);
    setupLayer(  32, &g.layers[27], &g.layers[26], 1000, mapZoom);
    setupLayer(  32, &g.layers[28], &g.layers[27],    3, mapAddIsland);
    setupLayer(  16, &g.layers[29], &g.layers[28], 1001, mapZoom);
    setupLayer(  16, &g.layers[30], &g.layers[29], 1000, mapShore);
    setupLayer(   8, &g.layers[31], &g.layers[30], 1002, mapZoom);
    setupLayer(   4, &g.layers[32], &g.layers[31], 1003, mapZoom);
    setupLayer(   4, &g.layers[33], &g.layers[32], 1000, mapSmooth);

    // river layer chain
    setupLayer( 128, &g.layers[34], &g.layers[22], 1000, mapZoom);
    setupLayer(  64, &g.layers[35], &g.layers[34], 1001, mapZoom);
    setupLayer(  32, &g.layers[36], &g.layers[35], 1000, mapZoom);
    setupLayer(  16, &g.layers[37], &g.layers[36], 1001, mapZoom);
    setupLayer(   8, &g.layers[38], &g.layers[37], 1002, mapZoom);
    setupLayer(   4, &g.layers[39], &g.layers[38], 1003, mapZoom);
    setupLayer(   4, &g.layers[40], &g.layers[39],    1, mapRiver);
    setupLayer(   4, &g.layers[41], &g.layers[40], 1000, mapSmooth);

    setupMultiLayer(4, &g.layers[42], &g.layers[33], &g.layers[41], 100, mapRiverMix);
    setupLayer(   1, &g.layers[43], &g.layers[42],   10, mapVoronoiZoom);

    return g;
}


LayerStack setupGeneratorMC113()
{
    if (biomes[plains].id == 0)
    {
        fprintf(stderr, "Warning: The biomes have to be initialised first using initBiomes() before any generator can be used.\n");
    }

    LayerStack g;

    g.layerNum = 52;
    g.layers = (Layer *) malloc(sizeof(Layer) * g.layerNum);

    //         SCALE    LAYER          PARENT      SEED  LAYER_FUNCTION
    setupLayer(4096, &g.layers[ 0],          NULL,    1, mapIsland);
    setupLayer(2048, &g.layers[ 1], &g.layers[ 0], 2000, mapZoom);
    setupLayer(2048, &g.layers[ 2], &g.layers[ 1],    1, mapAddIsland);
    setupLayer(1024, &g.layers[ 3], &g.layers[ 2], 2001, mapZoom);
    setupLayer(1024, &g.layers[ 4], &g.layers[ 3],    2, mapAddIsland);
    setupLayer(1024, &g.layers[ 5], &g.layers[ 4],   50, mapAddIsland);
    setupLayer(1024, &g.layers[ 6], &g.layers[ 5],   70, mapAddIsland);
    setupLayer(1024, &g.layers[ 7], &g.layers[ 6],    2, mapRemoveTooMuchOcean);

    setupLayer(1024, &g.layers[ 8], &g.layers[ 7],    2, mapAddSnow);
    setupLayer(1024, &g.layers[ 9], &g.layers[ 8],    3, mapAddIsland);
    setupLayer(1024, &g.layers[10], &g.layers[ 9],    2, mapCoolWarm);
    setupLayer(1024, &g.layers[11], &g.layers[10],    2, mapHeatIce);
    setupLayer(1024, &g.layers[12], &g.layers[11],    3, mapSpecial);
    setupLayer( 512, &g.layers[13], &g.layers[12], 2002, mapZoom);
    setupLayer( 256, &g.layers[14], &g.layers[13], 2003, mapZoom);
    setupLayer( 256, &g.layers[15], &g.layers[14],    4, mapAddIsland);
    setupLayer( 256, &g.layers[16], &g.layers[15],    5, mapAddMushroomIsland);
    setupLayer( 256, &g.layers[17], &g.layers[16],    4, mapDeepOcean);
    // biome layer chain
    setupLayer( 256, &g.layers[18], &g.layers[17],  200, mapBiome);
    setupLayer( 128, &g.layers[19], &g.layers[18], 1000, mapZoom);
    setupLayer(  64, &g.layers[20], &g.layers[19], 1001, mapZoom);
    setupLayer(  64, &g.layers[21], &g.layers[20], 1000, mapBiomeEdge);

    // basic river layer chain, used to determine where hills generate
    setupLayer( 256, &g.layers[22], &g.layers[17],  100, mapRiverInit);
    setupLayer( 128, &g.layers[23], &g.layers[22], 1000, mapZoom);
    setupLayer(  64, &g.layers[24], &g.layers[23], 1001, mapZoom);

    setupMultiLayer(64, &g.layers[25], &g.layers[21], &g.layers[24], 1000, mapHills113);

    setupLayer(  64, &g.layers[26], &g.layers[25], 1001, mapRareBiome);
    setupLayer(  32, &g.layers[27], &g.layers[26], 1000, mapZoom);
    setupLayer(  32, &g.layers[28], &g.layers[27],    3, mapAddIsland);
    setupLayer(  16, &g.layers[29], &g.layers[28], 1001, mapZoom);
    setupLayer(  16, &g.layers[30], &g.layers[29], 1000, mapShore);
    setupLayer(   8, &g.layers[31], &g.layers[30], 1002, mapZoom);
    setupLayer(   4, &g.layers[32], &g.layers[31], 1003, mapZoom);
    setupLayer(   4, &g.layers[33], &g.layers[32], 1000, mapSmooth);

    // river layer chain
    setupLayer( 128, &g.layers[34], &g.layers[22], 1000, mapZoom);
    setupLayer(  64, &g.layers[35], &g.layers[34], 1001, mapZoom);
    setupLayer(  32, &g.layers[36], &g.layers[35], 1000, mapZoom);
    setupLayer(  16, &g.layers[37], &g.layers[36], 1001, mapZoom);
    setupLayer(   8, &g.layers[38], &g.layers[37], 1002, mapZoom);
    setupLayer(   4, &g.layers[39], &g.layers[38], 1003, mapZoom);
    setupLayer(   4, &g.layers[40], &g.layers[39],    1, mapRiver);
    setupLayer(   4, &g.layers[41], &g.layers[40], 1000, mapSmooth);

    setupMultiLayer(4, &g.layers[42], &g.layers[33], &g.layers[41], 100, mapRiverMix);

    // ocean variants
    setupLayer( 256, &g.layers[43],          NULL,    2, mapOceanTemp);
    g.layers[43].oceanRnd = (OceanRnd *) malloc(sizeof(OceanRnd));
    setupLayer( 128, &g.layers[44], &g.layers[43], 2001, mapZoom);
    setupLayer(  64, &g.layers[45], &g.layers[44], 2002, mapZoom);
    setupLayer(  32, &g.layers[46], &g.layers[45], 2003, mapZoom);
    setupLayer(  16, &g.layers[47], &g.layers[46], 2004, mapZoom);
    setupLayer(   8, &g.layers[48], &g.layers[47], 2005, mapZoom);
    setupLayer(   4, &g.layers[49], &g.layers[48], 2006, mapZoom);

    setupMultiLayer(4, &g.layers[50], &g.layers[42], &g.layers[49], 100, mapOceanMix);

    setupLayer(1, &g.layers[51], &g.layers[50],   10, mapVoronoiZoom);

    return g;
}

void freeGenerator(LayerStack g)
{
    int i;
    for(i = 0; i < g.layerNum; i++)
    {
        if (g.layers[i].oceanRnd != NULL)
            free(g.layers[i].oceanRnd);
    }

    free(g.layers);
}


/* Recursively calculates the minimum buffer size required to generate an area
 * of the specified size from the current layer onwards.
 */
static void getMaxArea(Layer *layer, int areaX, int areaZ, int *maxX, int *maxZ)
{
    if (layer == NULL)
        return;

    if (layer->getMap == mapZoom)
    {
        areaX = (areaX >> 1) + 2;
        areaZ = (areaZ >> 1) + 2;
    }
    else if (layer->getMap == mapVoronoiZoom)
    {
        areaX = (areaX >> 2) + 2;
        areaZ = (areaZ >> 2) + 2;
    }
    else if (layer->getMap == mapOceanMix)
    {
        areaX += 17;
        areaZ += 17;
    }
    else
    {
        if (layer->getMap != mapNull &&
            layer->getMap != mapSkip &&
            layer->getMap != mapIsland &&
            layer->getMap != mapSpecial &&
            layer->getMap != mapBiome &&
            layer->getMap != mapRiverInit &&
            layer->getMap != mapRiverMix &&
            layer->getMap != mapOceanTemp)
        {
            areaX += 2;
            areaZ += 2;
        }
    }

    if (areaX > *maxX) *maxX = areaX;
    if (areaZ > *maxZ) *maxZ = areaZ;

    getMaxArea(layer->p, areaX, areaZ, maxX, maxZ);
    getMaxArea(layer->p2, areaX, areaZ, maxX, maxZ);
}

int calcRequiredBuf(Layer *layer, int areaX, int areaZ)
{
    int maxX = areaX, maxZ = areaZ;
    getMaxArea(layer, areaX, areaZ, &maxX, &maxZ);

    return maxX * maxZ;
}

int *allocCache(Layer *layer, int sizeX, int sizeZ)
{
    int size = calcRequiredBuf(layer, sizeX, sizeZ);

    int *ret = (int *) malloc(sizeof(*ret)*size);
    memset(ret, 0, sizeof(*ret)*size);

    return ret;
}


void applySeed(LayerStack *g, int64_t seed)
{
    // the seed has to be applied recursively
    setWorldSeed(&g->layers[g->layerNum-1], seed);
}

void genArea(Layer *layer, int *out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    memset(out, 0, areaWidth*areaHeight*sizeof(*out));
    layer->getMap(layer, out, areaX, areaZ, areaWidth, areaHeight);
}





