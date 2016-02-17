/**
 * layers.h
 *
 * Author: Cubitect
 * Created on: 17 Feb 2016
 * Licence: GPLv3
 */

#ifndef LAYER_H_
#define LAYER_H_

#define STRUCT(S) typedef struct S S; struct S

/*
 * Annoyingly the gcc compiler breaks some of the simple layer generation functions on the -O3 optimisation level.
 * Declaring these functions with the attribute below keeps them from misbehaving.
 * (Declaring the looping variables in the function as volatile also works, but is slower in most cases.)
 */
#define OPT_O2 __attribute__((optimize("O2")))

enum BiomeID {
    none = -1,
    ocean = 0, plains, desert, extremeHills, forest, taiga, swampland, river, hell, sky, // 0-9
    frozenOcean, frozenRiver, icePlains, iceMountains, mushroomIsland, mushroomIslandShore, beach, desertHills, forestHills, taigaHills,  // 10-19
    extremeHillsEdge, jungle, jungleHills, jungleEdge, deepOcean, stoneBeach, coldBeach, birchForest, birchForestHills, roofedForest, // 20-29
    coldTaiga, coldTaigaHills, megaTaiga, megaTaigaHills, extremeHillsPlus, savanna, savannaPlateau, mesa, mesaPlateau_F, mesaPlateau, // 30-39
    BIOME_NUM
};

enum BiomeType {
    Ocean, Plains, Desert, Hills, Forest, Taiga, Swamp, River, Hell, Sky, Snow, MushroomIsland, Beach, Jungle, StoneBeach, Savanna, Mesa, BTYPE_NUM
};

enum BiomeTempCategory {
    Oceanic, Warm, Lush, Cold, Freezing, Unknown
};

STRUCT(Biome) {
    int id;
    int type;
    double height;
    double temp;
    int tempCat;
};

STRUCT(Layer) {
    long baseSeed;  // Generator seed (depends only on hierarchy of generator)
    long worldSeed; // based on the seed of the world

    long chunkSeed; // randomiser seed

    void (*getMap)(Layer *layer, int *out, int x, int z, int w, int h);

    Layer *p, *p2;
};

Biome biomes[256];

static void initAddBiome(int id, int tempCat, int biometype, float temp, float height)
{
    if(id & (~0xff)) return;
    biomes[id].id = id;
    biomes[id].type = biometype;
    biomes[id].temp = temp;
    biomes[id].height = height;
    biomes[id].tempCat = tempCat;
}

static void createMutation(int id)
{
    biomes[id+128] = biomes[id];
    biomes[id+128].id = id+128;
}

/* initBiomes() has to be called before any of the generators can be used */
static void initBiomes()
{
    int i;
    for(i = 0; i < 256; i++) biomes[i].id = none;

    const double hDefault = 0.1, hShallowWaters = -0.5, hOceans = -1.0, hDeepOceans = -1.8, hLowPlains = 0.125;
    const double hMidPlains = 0.2, hLowHills = 0.45, hHighPlateaus = 1.5, hMidHills = 1.0, hShores = 0.0;
    const double hRockyWaters = 0.1, hLowIslands = 0.2, hPartiallySubmerged = -0.2;

    initAddBiome(ocean, Oceanic, Ocean, 0.5, hOceans);
    initAddBiome(plains, Lush, Plains, 0.8, hDefault);
    initAddBiome(desert, Warm, Desert, 2.0, hLowPlains);
    initAddBiome(extremeHills, Lush, Hills, 0.2, hMidHills);
    initAddBiome(forest, Lush, Forest, 0.7, hDefault);
    initAddBiome(taiga, Lush, Taiga, 0.25, hMidPlains);
    initAddBiome(swampland, Lush, Swamp, 0.8, hPartiallySubmerged);
    initAddBiome(river, Lush, River, 0.5, hShallowWaters);
    initAddBiome(hell, Warm, Hell, 2.0, hDefault);
    initAddBiome(sky, Lush, Sky, 0.5, hDefault);
    initAddBiome(frozenOcean, Oceanic, Ocean, 0.0, hOceans);
    initAddBiome(frozenRiver, Cold, River, 0.0, hShallowWaters);
    initAddBiome(icePlains, Cold, Snow, 0.0, hLowPlains);
    initAddBiome(iceMountains, Cold, Snow, 0.0, hLowHills);
    initAddBiome(mushroomIsland, Lush, MushroomIsland, 0.9, hLowIslands);
    initAddBiome(mushroomIslandShore, Lush, MushroomIsland, 0.9, hShores);
    initAddBiome(beach, Lush, Beach, 0.8, hShores);
    initAddBiome(desertHills, Warm, Desert, 2.0, hLowHills);
    initAddBiome(forestHills, Lush, Forest, 0.7, hLowHills);
    initAddBiome(taigaHills, Lush, Taiga, 0.25, hLowHills);
    initAddBiome(extremeHillsEdge, Lush, Hills, 0.2, hMidHills);
    initAddBiome(jungle, Lush, Jungle, 0.95, hDefault);
    initAddBiome(jungleHills, Lush, Jungle, 0.95, hLowHills);
    initAddBiome(jungleEdge, Lush, Jungle, 0.95, hDefault);
    initAddBiome(deepOcean, Oceanic, Ocean, 0.5, hDeepOceans);
    initAddBiome(stoneBeach, Lush, StoneBeach, 0.2, hRockyWaters);
    initAddBiome(coldBeach, Cold, Beach, 0.05, hShores);
    initAddBiome(birchForest, Lush, Forest, 0.6, hDefault);
    initAddBiome(birchForestHills, Lush, Forest, 0.6, hLowHills);
    initAddBiome(roofedForest, Lush, Forest, 0.7, hDefault);
    initAddBiome(coldTaiga, Cold, Taiga, -0.5, hMidPlains);
    initAddBiome(coldTaigaHills, Cold, Taiga, -0.5, hLowHills);
    initAddBiome(megaTaiga, Lush, Taiga, 0.3, hMidPlains);
    initAddBiome(megaTaigaHills, Lush, Taiga, 0.3, hLowHills);
    initAddBiome(extremeHillsPlus, Lush, Hills, 0.2, hMidHills);
    initAddBiome(savanna, Warm, Savanna, 1.2, hLowPlains);
    initAddBiome(savannaPlateau, Warm, Savanna, 1.0, hHighPlateaus);
    initAddBiome(mesa, Warm, Mesa, 2.0, hDefault);
    initAddBiome(mesaPlateau_F, Warm, Mesa, 2.0, hHighPlateaus);
    initAddBiome(mesaPlateau, Warm, Mesa, 2.0, hHighPlateaus);

    createMutation(plains);
    createMutation(desert);
    createMutation(extremeHills);
    createMutation(forest);
    createMutation(taiga);
    createMutation(swampland);
    createMutation(icePlains);
    createMutation(jungle);
    createMutation(jungleEdge);
    createMutation(birchForest);
    createMutation(birchForestHills);
    createMutation(roofedForest);
    createMutation(coldTaiga);
    createMutation(megaTaiga);
    createMutation(megaTaigaHills);
    createMutation(extremeHillsPlus);
    createMutation(savanna);
    createMutation(savannaPlateau);
    createMutation(mesa);
    createMutation(mesaPlateau_F);
    createMutation(mesaPlateau);
}

static int getBiomeType(int id)
{
    return biomes[id & 0xff].type;
}

static int biomeExists(int id)
{
    return !(biomes[id & 0xff].id & (~0xff));
}

static int getTempCategory(int id)
{
    return biomes[id & 0xff].tempCat;
}

static int equalOrPlateau(int id1, int id2)
{
    if(id1 == id2) return 1;
    if(id1 == mesaPlateau_F || id1 == mesaPlateau) return id2 == mesaPlateau_F || id2 == mesaPlateau;
    if(!biomeExists(id1) || !biomeExists(id2)) return 0;
    // adjust for asymmetric equality (workaround to simulate a bug in the MC java code)
    if(id1 >= 128 || id2 >= 128) {
        // skip biomes that did not overload the isEqualTo() method
        if(id2 == 130 || id2 == 133 || id2 == 134 || id2 == 149 || id2 == 151 || id2 == 155 ||
           id2 == 156 || id2 == 157 || id2 == 158 || id2 == 163 || id2 == 164) return 0;
    }
    return getBiomeType(id1) == getBiomeType(id2);
}

static int canBeNeighbors(int id1, int id2)
{
    if(equalOrPlateau(id1, id2)) return 1;
    if(!biomeExists(id1) || !biomeExists(id2)) return 0;
    int tempCat1 = getTempCategory(id1); if(tempCat1 == Lush) return 1;
    int tempCat2 = getTempCategory(id2); if(tempCat2 == Lush) return 1;
    return tempCat1 == tempCat2;
}

static int isOceanic(int id)
{
    return id == ocean || id == deepOcean || id == frozenOcean;
}

static int isBiomeSnowy(int id)
{
    return biomeExists(id) && biomes[id&0xff].temp < 0.1;
}

static int mcNextInt(Layer *layer, int mod)
{
    int ret = (int)((layer->chunkSeed >> 24) % (long)mod);

    if (ret < 0)
    {
        ret += mod;
    }

    layer->chunkSeed *= layer->chunkSeed * 6364136223846793005L + 1442695040888963407L;
    layer->chunkSeed += layer->worldSeed;
    return ret;
}

void setChunkSeed(Layer *layer, long chunkX, long chunkZ);
void setBaseSeed(Layer *layer, long seed);
void setWorldSeed(Layer *layer, long seed);


void mapIsland(Layer *l, int *out, int x, int z, int w, int h);
void mapZoom(Layer *l, int *out, int x, int z, int w, int h);
void mapAddIsland(Layer *l, int *out, int x, int z, int w, int h);
void mapRemoveTooMuchOcean(Layer *l, int *out, int x, int z, int w, int h);
void mapAddSnow(Layer *l, int *out, int x, int z, int w, int h);
void mapCoolWarm(Layer *l, int *out, int x, int z, int w, int h);
void mapHeatIce(Layer *l, int *out, int x, int z, int w, int h);
void mapSpecial(Layer *l, int *out, int x, int z, int w, int h);
void mapAddMushroomIsland(Layer *l, int *out, int x, int z, int w, int h);
void mapDeepOcean(Layer *l, int *out, int x, int z, int w, int h);
void mapBiome(Layer *l, int *out, int x, int z, int w, int h);
void mapRiverInit(Layer *l, int *out, int x, int z, int w, int h);
void mapBiomeEdge(Layer *l, int *out, int x, int z, int w, int h);
void mapHills(Layer *l, int *out, int x, int z, int w, int h);
void mapRiver(Layer *l, int *out, int x, int z, int w, int h);
void mapSmooth(Layer *l, int *out, int x, int z, int w, int h);
void mapRareBiome(Layer *l, int *out, int x, int z, int w, int h);
void mapShore(Layer *l, int *out, int x, int z, int w, int h);
void mapRiverMix(Layer *l, int *out, int x, int z, int w, int h);
void mapVoronoiZoom(Layer *l, int *out, int x, int z, int w, int h);


#endif /* LAYER_H_ */
