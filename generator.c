#include "generator.h"
#include "layers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



void setupLayer(Layer *l, Layer *p, int s,
        void (*getMap)(const Layer *, int *, int, int, int, int))
{
    l->layerSeed = getLayerSeed(s);
    l->startSalt = 0;
    l->startSeed = 0;
    l->p = p;
    l->p2 = NULL;
    l->scale = 0;
    l->getMap = getMap;
    l->oceanRnd = NULL;
}

void setupMultiLayer(Layer *l, Layer *p1, Layer *p2, int s,
        void (*getMap)(const Layer *, int *, int, int, int, int))
{
    setupLayer(l, p1, s, getMap);
    l->p2 = p2;
}


static void setupScale(Layer *l, int scale)
{
    l->scale = scale;
    int m = 1;
    if (l->getMap == mapZoom) {
        m = 2;
    } else if (l->getMap == mapVoronoiZoom) {
        m = 4;
    }
    if (l->p) {
        setupScale(l->p, scale * m);
    }
    if (l->p2) {
        setupScale(l->p2, scale * m);
    }
}

static LayerStack setupGeneratorImpl(const int mcversion, const int largeBiomes)
{
    if (biomes[plains].id == 0)
    {
        fprintf(stderr, "Warning: The biomes have to be initialised first using initBiomes() before any generator can be used.\n");
    }

    LayerStack g;
    g.layerCnt = L_NUM;
    g.layers = (Layer *) calloc(g.layerCnt, sizeof(Layer));
    Layer *l = g.layers;

    //         LAYER                      PARENT                      SALT  LAYER_FUNCTION
    setupLayer(&l[L_ISLAND_4096],         NULL,                       1,    mapIsland);
    setupLayer(&l[L_ZOOM_2048],           &l[L_ISLAND_4096],          2000, mapZoom);
    setupLayer(&l[L_ADD_ISLAND_2048],     &l[L_ZOOM_2048],            1,    mapAddIsland);
    setupLayer(&l[L_ZOOM_1024],           &l[L_ADD_ISLAND_2048],      2001, mapZoom);
    setupLayer(&l[L_ADD_ISLAND_1024A],    &l[L_ZOOM_1024],            2,    mapAddIsland);
    setupLayer(&l[L_ADD_ISLAND_1024B],    &l[L_ADD_ISLAND_1024A],     50,   mapAddIsland);
    setupLayer(&l[L_ADD_ISLAND_1024C],    &l[L_ADD_ISLAND_1024B],     70,   mapAddIsland);
    setupLayer(&l[L_REMOVE_OCEAN_1024],   &l[L_ADD_ISLAND_1024C],     2,    mapRemoveTooMuchOcean);

    setupLayer(&l[L_ADD_SNOW_1024],       &l[L_REMOVE_OCEAN_1024],    2,    mapAddSnow);
    setupLayer(&l[L_ADD_ISLAND_1024D],    &l[L_ADD_SNOW_1024],        3,    mapAddIsland);
    setupLayer(&l[L_COOL_WARM_1024],      &l[L_ADD_ISLAND_1024D],     2,    mapCoolWarm);
    setupLayer(&l[L_HEAT_ICE_1024],       &l[L_COOL_WARM_1024],       2,    mapHeatIce);
    setupLayer(&l[L_SPECIAL_1024],        &l[L_HEAT_ICE_1024],        3,    mapSpecial);
    setupLayer(&l[L_ZOOM_512],            &l[L_SPECIAL_1024],         2002, mapZoom);
    setupLayer(&l[L_ZOOM_256],            &l[L_ZOOM_512],             2003, mapZoom);
    setupLayer(&l[L_ADD_ISLAND_256],      &l[L_ZOOM_256],             4,    mapAddIsland);
    setupLayer(&l[L_ADD_MUSHROOM_256],    &l[L_ADD_ISLAND_256],       5,    mapAddMushroomIsland);
    setupLayer(&l[L_DEEP_OCEAN_256],      &l[L_ADD_MUSHROOM_256],     4,    mapDeepOcean);
    // biome layer chain
    setupLayer(&l[L_BIOME_256],           &l[L_DEEP_OCEAN_256],       200,
            mcversion != MC_BE ? mapBiome : mapBiomeBE);

    if (mcversion <= MC_1_13)
        setupLayer(&l[L_ZOOM_128],        &l[L_BIOME_256],            1000, mapZoom);
    else
    {
        setupLayer(&l[L14_BAMBOO_256],    &l[L_BIOME_256],            1001, mapAddBamboo);
        setupLayer(&l[L_ZOOM_128],        &l[L14_BAMBOO_256],         1000, mapZoom);
    }

    setupLayer(&l[L_ZOOM_64],             &l[L_ZOOM_128],             1001, mapZoom);
    setupLayer(&l[L_BIOME_EDGE_64],       &l[L_ZOOM_64],              1000, mapBiomeEdge);

    // basic river layer chain, used to determine where hills generate
    setupLayer(&l[L_RIVER_INIT_256],      &l[L_DEEP_OCEAN_256],       100,  mapRiverInit);
    setupLayer(&l[L_ZOOM_128_HILLS],      &l[L_RIVER_INIT_256],       1000, mapZoom);
    setupLayer(&l[L_ZOOM_64_HILLS],       &l[L_ZOOM_128_HILLS],       1001, mapZoom);

    setupMultiLayer(&l[L_HILLS_64], &l[L_BIOME_EDGE_64], &l[L_ZOOM_64_HILLS], 1000,
            (mcversion & 0xff) <= MC_1_12 ? mapHills : mapHills113);

    setupLayer(&l[L_RARE_BIOME_64],       &l[L_HILLS_64],             1001, mapRareBiome);
    setupLayer(&l[L_ZOOM_32],             &l[L_RARE_BIOME_64],        1000, mapZoom);
    setupLayer(&l[L_ADD_ISLAND_32],       &l[L_ZOOM_32],              3,    mapAddIsland);
    setupLayer(&l[L_ZOOM_16],             &l[L_ADD_ISLAND_32],        1001, mapZoom);
    setupLayer(&l[L_SHORE_16],            &l[L_ZOOM_16],              1000, mapShore);
    setupLayer(&l[L_ZOOM_8],              &l[L_SHORE_16],             1002, mapZoom);
    setupLayer(&l[L_ZOOM_4],              &l[L_ZOOM_8],               1003, mapZoom);

    if (largeBiomes != 0) {
        setupLayer(&l[L_ZOOM_LARGE_BIOME_A], &l[L_ZOOM_4],               1004, mapZoom);
        setupLayer(&l[L_ZOOM_LARGE_BIOME_B], &l[L_ZOOM_LARGE_BIOME_A],   1005, mapZoom);
        setupLayer(&l[L_SMOOTH_4],           &l[L_ZOOM_LARGE_BIOME_B],   1000, mapSmooth);
    } else {
        setupLayer(&l[L_SMOOTH_4],           &l[L_ZOOM_4],               1000, mapSmooth);
    }

    // river layer chain
    setupLayer(&l[L_ZOOM_128_RIVER],      &l[L_RIVER_INIT_256],       1000, mapZoom);
    setupLayer(&l[L_ZOOM_64_RIVER],       &l[L_ZOOM_128_RIVER],       1001, mapZoom);
    setupLayer(&l[L_ZOOM_32_RIVER],       &l[L_ZOOM_64_RIVER],        1000, mapZoom);
    setupLayer(&l[L_ZOOM_16_RIVER],       &l[L_ZOOM_32_RIVER],        1001, mapZoom);
    setupLayer(&l[L_ZOOM_8_RIVER],        &l[L_ZOOM_16_RIVER],        1002, mapZoom);
    setupLayer(&l[L_ZOOM_4_RIVER],        &l[L_ZOOM_8_RIVER],         1003, mapZoom);
    setupLayer(&l[L_RIVER_4],             &l[L_ZOOM_4_RIVER],         1,    mapRiver);
    setupLayer(&l[L_SMOOTH_4_RIVER],      &l[L_RIVER_4],              1000, mapSmooth);

    setupMultiLayer(&l[L_RIVER_MIX_4], &l[L_SMOOTH_4], &l[L_SMOOTH_4_RIVER], 100, mapRiverMix);

    if (mcversion <= MC_1_12)
    {
        setupLayer(&l[L_VORONOI_ZOOM_1],   &l[L_RIVER_MIX_4],          10,   mapVoronoiZoom);
    }
    else
    {
        // ocean variants
        setupLayer(&l[L13_OCEAN_TEMP_256], NULL,                       2,    mapOceanTemp);
        l[L13_OCEAN_TEMP_256].oceanRnd = (OceanRnd *) malloc(sizeof(OceanRnd));
        setupLayer(&l[L13_ZOOM_128],       &l[L13_OCEAN_TEMP_256],     2001, mapZoom);
        setupLayer(&l[L13_ZOOM_64],        &l[L13_ZOOM_128],           2002, mapZoom);
        setupLayer(&l[L13_ZOOM_32],        &l[L13_ZOOM_64],            2003, mapZoom);
        setupLayer(&l[L13_ZOOM_16],        &l[L13_ZOOM_32],            2004, mapZoom);
        setupLayer(&l[L13_ZOOM_8],         &l[L13_ZOOM_16],            2005, mapZoom);
        setupLayer(&l[L13_ZOOM_4],         &l[L13_ZOOM_8],             2006, mapZoom);

        setupMultiLayer(&l[L13_OCEAN_MIX_4], &l[L_RIVER_MIX_4], &l[L13_ZOOM_4], 100, mapOceanMix);

        setupLayer(&l[L_VORONOI_ZOOM_1],   &l[L13_OCEAN_MIX_4],        10,   mapVoronoiZoom);
    }

    setupScale(&l[L_VORONOI_ZOOM_1], 1);

    return g;
}

LayerStack setupGenerator(const int mcversion)
{
    return setupGeneratorImpl(mcversion, 0);
}

LayerStack setupLargeBiomesGenerator(const int mcversion)
{
    return setupGeneratorImpl(mcversion, 1);
}

void freeGenerator(LayerStack g)
{
    int i;
    for(i = 0; i < g.layerCnt; i++)
    {
        if (g.layers[i].oceanRnd != NULL)
            free(g.layers[i].oceanRnd);
    }

    free(g.layers);
}


/* Recursively calculates the minimum buffer size required to generate an area
 * of the specified size from the current layer onwards.
 */
static void getMaxArea(const Layer *layer, int areaX, int areaZ, int *maxX, int *maxZ)
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
        areaX = (areaX >> 2) + 3;
        areaZ = (areaZ >> 2) + 3;
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
            layer->getMap != mapRareBiome &&
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

int calcRequiredBuf(const Layer *layer, int areaX, int areaZ)
{
    int maxX = areaX, maxZ = areaZ;
    getMaxArea(layer, areaX, areaZ, &maxX, &maxZ);

    return maxX * maxZ;
}

int *allocCache(const Layer *layer, int sizeX, int sizeZ)
{
    int size = calcRequiredBuf(layer, sizeX, sizeZ);

    int *ret = (int *) malloc(sizeof(*ret)*size);
    memset(ret, 0, sizeof(*ret)*size);

    return ret;
}


void applySeed(LayerStack *g, int64_t seed)
{
    // the seed has to be applied recursively
    setWorldSeed(&g->layers[L_VORONOI_ZOOM_1], seed);
}

void genArea(const Layer *layer, int *out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    memset(out, 0, areaWidth*areaHeight*sizeof(*out));
    layer->getMap(layer, out, areaX, areaZ, areaWidth, areaHeight);
}





