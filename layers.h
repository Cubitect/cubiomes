#ifndef LAYER_H_
#define LAYER_H_

#include <stdlib.h>

#define STRUCT(S) typedef struct S S; struct S

#define OPT_O2 __attribute__((optimize("O2")))


enum BiomeID {
    none = -1,
    ocean = 0, plains, desert, extremeHills, forest, taiga, swampland, river, hell, sky, // 0-9
    frozenOcean, frozenRiver, icePlains, iceMountains, mushroomIsland, mushroomIslandShore, beach, desertHills, forestHills, taigaHills,  // 10-19
    extremeHillsEdge, jungle, jungleHills, jungleEdge, deepOcean, stoneBeach, coldBeach, birchForest, birchForestHills, roofedForest, // 20-29
    coldTaiga, coldTaigaHills, megaTaiga, megaTaigaHills, extremeHillsPlus, savanna, savannaPlateau, mesa, mesaPlateau_F, mesaPlateau, // 30-39
    // 1.13
    skyIslandLow, skyIslandMedium, skyIslandHigh, skyIslandBarren, warmOcean, lukewarmOcean, coldOcean, warmDeepOcean, lukewarmDeepOcean, coldDeepOcean, // 40-49
    frozenDeepOcean,
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

extern Biome biomes[256];

/* initBiomes() has to be called before any of the generators can be used */
void initBiomes();


void setWorldSeed(Layer *layer, long seed);



static inline int getBiomeType(int id)
{
    return biomes[id & 0xff].type;
}

static inline int biomeExists(int id)
{
    return !(biomes[id & 0xff].id & (~0xff));
}

static inline int getTempCategory(int id)
{
    return biomes[id & 0xff].tempCat;
}

static inline int equalOrPlateau(int id1, int id2)
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

static inline int canBeNeighbors(int id1, int id2)
{
    if(equalOrPlateau(id1, id2)) return 1;
    if(!biomeExists(id1) || !biomeExists(id2)) return 0;
    int tempCat1 = getTempCategory(id1); if(tempCat1 == Lush) return 1;
    int tempCat2 = getTempCategory(id2); if(tempCat2 == Lush) return 1;
    return tempCat1 == tempCat2;
}

static inline int isShallowOcean(int id)
{
    return id == ocean || id == frozenOcean ||
           id == warmOcean || id == lukewarmOcean || id == coldOcean;
}

static inline int isOceanic(int id)
{
    switch(id)
    {
    case ocean:
    case deepOcean:
    case warmOcean:
    case warmDeepOcean:
    case lukewarmOcean:
    case lukewarmDeepOcean:
    case coldOcean:
    case coldDeepOcean:
    case frozenOcean:
    case frozenDeepOcean:
        return 1;
    default:
        return 0;
    }
}


static inline int isBiomeSnowy(int id)
{
    return biomeExists(id) && biomes[id&0xff].temp < 0.1;
}

static inline int mcNextInt(Layer *layer, int mod)
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

static inline void setChunkSeed(Layer *layer, long chunkX, long chunkZ)
{
    layer->chunkSeed =  layer->worldSeed;
    layer->chunkSeed *= layer->chunkSeed * 6364136223846793005L + 1442695040888963407L;
    layer->chunkSeed += chunkX;
    layer->chunkSeed *= layer->chunkSeed * 6364136223846793005L + 1442695040888963407L;
    layer->chunkSeed += chunkZ;
    layer->chunkSeed *= layer->chunkSeed * 6364136223846793005L + 1442695040888963407L;
    layer->chunkSeed += chunkX;
    layer->chunkSeed *= layer->chunkSeed * 6364136223846793005L + 1442695040888963407L;
    layer->chunkSeed += chunkZ;
}

static inline void setBaseSeed(Layer *layer, long seed)
{
    layer->baseSeed = seed;
    layer->baseSeed *= layer->baseSeed * 6364136223846793005L + 1442695040888963407L;
    layer->baseSeed += seed;
    layer->baseSeed *= layer->baseSeed * 6364136223846793005L + 1442695040888963407L;
    layer->baseSeed += seed;
    layer->baseSeed *= layer->baseSeed * 6364136223846793005L + 1442695040888963407L;
    layer->baseSeed += seed;

    layer->p = NULL;
    layer->worldSeed = 0;
    layer->chunkSeed = 0;
}

static inline int selectRandom2(Layer *l, int a1, int a2)
{
    int i = mcNextInt(l, 2);
    return i == 0 ? a1 : a2;
}

static inline int selectRandom4(Layer *l, int a1, int a2, int a3, int a4)
{
    int i = mcNextInt(l, 4);
    return i == 0 ? a1 : i == 1 ? a2 : i == 2 ? a3 : a4;
}

static inline int selectModeOrRandom(Layer *l, int a1, int a2, int a3, int a4)
{
    int rndarg = selectRandom4(l, a1, a2, a3, a4);

    if(a2 == a3 && a3 == a4) return a2;
    if(a1 == a2 && a1 == a3) return a1;
    if(a1 == a2 && a1 == a4) return a1;
    if(a1 == a3 && a1 == a4) return a1;
    if(a1 == a2 && a3 != a4) return a1;
    if(a1 == a3 && a2 != a4) return a1;
    if(a1 == a4 && a2 != a3) return a1;
    if(a2 == a3 && a1 != a4) return a2;
    if(a2 == a4 && a1 != a3) return a2;
    if(a3 == a4 && a1 != a2) return a3;

    return rndarg;
}

// A null layer does nothing, and can be used to apply a layer to existing data.
void mapNull(Layer *l, int * __restrict out, int x, int z, int w, int h);

void mapIsland(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapZoom(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapAddIsland(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapRemoveTooMuchOcean(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapAddSnow(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapCoolWarm(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapHeatIce(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapSpecial(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapAddMushroomIsland(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapDeepOcean(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapBiome(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapRiverInit(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapBiomeEdge(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapHills(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapRiver(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapSmooth(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapRareBiome(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapShore(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapRiverMix(Layer *l, int * __restrict out, int x, int z, int w, int h);

void mapOceanTemp(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight);
void mapEdgeOcean(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight);
void mapOceanMix(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight);

void mapVoronoiZoom(Layer *l, int * __restrict out, int x, int z, int w, int h);

#endif /* LAYER_H_ */
