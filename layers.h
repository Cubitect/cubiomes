#ifndef LAYER_H_
#define LAYER_H_

#include "javarnd.h"

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>


#if defined USE_SIMD && __AVX2__
#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>
#warning "Using AVX2 extensions."
#elif defined USE_SIMD && defined __SSE4_2__
#include <emmintrin.h>
#include <smmintrin.h>
#warning "Using SSE4.2 extensions."
#else
//#warning "Using no SIMD extensions."
#endif

#define STRUCT(S) typedef struct S S; struct S

#define OPT_O2 __attribute__((optimize("O2")))


enum BiomeID
{
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

enum BiomeType
{
    Ocean, Plains, Desert, Hills, Forest, Taiga, Swamp, River, Hell, Sky, Snow, MushroomIsland, Beach, Jungle, StoneBeach, Savanna, Mesa, BTYPE_NUM
};

enum BiomeTempCategory
{
    Oceanic, Warm, Lush, Cold, Freezing, Unknown
};


STRUCT(Biome)
{
    int id;
    int type;
    double height;
    double temp;
    int tempCat;
};

STRUCT(OceanRnd)
{
    int d[512];
    double a, b, c;
};

STRUCT(Layer)
{
    int64_t baseSeed;   // Generator seed (depends only on layer hierarchy)
    int64_t worldSeed;  // based on the seed of the world

    int64_t chunkSeed;  // randomiser seed

    OceanRnd *oceanRnd; // world seed dependent data for ocean temperatures

    int scale;          // map scale of this layer (map entry = scale x scale blocks)

    void (*getMap)(Layer *layer, int *out, int x, int z, int w, int h);

    Layer *p, *p2;      // parent layers
};


//==============================================================================
// Essentials
//==============================================================================

extern Biome biomes[256];


/* initBiomes() has to be called before any of the generators can be used */
void initBiomes();

/* Applies the given world seed to the layer and all dependent layers. */
void setWorldSeed(Layer *layer, int64_t seed);


//==============================================================================
// Static Helpers
//==============================================================================

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
    if (id1 == id2) return 1;
    if (id1 == mesaPlateau_F || id1 == mesaPlateau) return id2 == mesaPlateau_F || id2 == mesaPlateau;
    if (!biomeExists(id1) || !biomeExists(id2)) return 0;
    // adjust for asymmetric equality (workaround to simulate a bug in the MC java code)
    if (id1 >= 128 || id2 >= 128) {
        // skip biomes that did not overload the isEqualTo() method
        if (id2 == 130 || id2 == 133 || id2 == 134 || id2 == 149 || id2 == 151 || id2 == 155 ||
           id2 == 156 || id2 == 157 || id2 == 158 || id2 == 163 || id2 == 164) return 0;
    }
    return getBiomeType(id1) == getBiomeType(id2);
}

static inline int canBeNeighbors(int id1, int id2)
{
    if (equalOrPlateau(id1, id2)) return 1;
    if (!biomeExists(id1) || !biomeExists(id2)) return 0;
    int tempCat1 = getTempCategory(id1); if (tempCat1 == Lush) return 1;
    int tempCat2 = getTempCategory(id2); if (tempCat2 == Lush) return 1;
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
    int ret = (int)((layer->chunkSeed >> 24) % (int64_t)mod);

    if (ret < 0)
    {
        ret += mod;
    }

    layer->chunkSeed *= layer->chunkSeed * 6364136223846793005LL + 1442695040888963407LL;
    layer->chunkSeed += layer->worldSeed;
    return ret;
}

static inline void setChunkSeed(Layer *layer, int64_t chunkX, int64_t chunkZ)
{
    layer->chunkSeed =  layer->worldSeed;
    layer->chunkSeed *= layer->chunkSeed * 6364136223846793005LL + 1442695040888963407LL;
    layer->chunkSeed += chunkX;
    layer->chunkSeed *= layer->chunkSeed * 6364136223846793005LL + 1442695040888963407LL;
    layer->chunkSeed += chunkZ;
    layer->chunkSeed *= layer->chunkSeed * 6364136223846793005LL + 1442695040888963407LL;
    layer->chunkSeed += chunkX;
    layer->chunkSeed *= layer->chunkSeed * 6364136223846793005LL + 1442695040888963407LL;
    layer->chunkSeed += chunkZ;
}

static inline void setBaseSeed(Layer *layer, int64_t seed)
{
    layer->baseSeed = seed;
    layer->baseSeed *= layer->baseSeed * 6364136223846793005LL + 1442695040888963407LL;
    layer->baseSeed += seed;
    layer->baseSeed *= layer->baseSeed * 6364136223846793005LL + 1442695040888963407LL;
    layer->baseSeed += seed;
    layer->baseSeed *= layer->baseSeed * 6364136223846793005LL + 1442695040888963407LL;
    layer->baseSeed += seed;

    layer->p = NULL;
    layer->worldSeed = 0;
    layer->chunkSeed = 0;
}

#if defined USE_SIMD && __AVX2__

static inline __m256i set8ChunkSeeds(int ws, __m256i xs, __m256i zs)
{
    __m256i out = _mm256_set1_epi32(ws);
    __m256i mul = _mm256_set1_epi32(1284865837);
    __m256i add = _mm256_set1_epi32(4150755663);
    out = _mm256_add_epi32(xs, _mm256_mullo_epi32(out, _mm256_add_epi32(add, _mm256_mullo_epi32(out, mul))));
    out = _mm256_add_epi32(zs, _mm256_mullo_epi32(out, _mm256_add_epi32(add, _mm256_mullo_epi32(out, mul))));
    out = _mm256_add_epi32(xs, _mm256_mullo_epi32(out, _mm256_add_epi32(add, _mm256_mullo_epi32(out, mul))));
    return _mm256_add_epi32(zs, _mm256_mullo_epi32(out, _mm256_add_epi32(add, _mm256_mullo_epi32(out, mul))));
}

static inline __m256i mc8NextInt(__m256i* cs, int ws, int mask)
{
    __m256i and = _mm256_set1_epi32(mask);
    __m256i ret = _mm256_and_si256(and, _mm256_srli_epi32(*cs, 24));
    *cs = _mm256_add_epi32(_mm256_set1_epi32(ws), _mm256_mullo_epi32(*cs, _mm256_add_epi32(_mm256_set1_epi32(4150755663), _mm256_mullo_epi32(*cs, _mm256_set1_epi32(1284865837)))));
    return _mm256_add_epi32(ret, _mm256_and_si256(and, _mm256_cmpgt_epi32(_mm256_set1_epi32(0), ret)));;
}

static inline __m256i select8Random2(__m256i* cs, int ws, __m256i a1, __m256i a2)
{
    __m256i cmp = _mm256_cmpeq_epi32(_mm256_set1_epi32(0), mc8NextInt(cs, ws, 0x1));
    return _mm256_or_si256(_mm256_and_si256(cmp, a1), _mm256_andnot_si256(cmp, a2));
}

static inline __m256i select8Random4(__m256i* cs, int ws, __m256i a1, __m256i a2, __m256i a3, __m256i a4)
{
    __m256i val = mc8NextInt(cs, ws, 0x3);
    __m256i v2 = _mm256_set1_epi32(2);
    __m256i cmp1 = _mm256_cmpeq_epi32(val, _mm256_set1_epi32(0));
    __m256i cmp2 = _mm256_cmpeq_epi32(v2, val);
    __m256i cmp3 = _mm256_cmpgt_epi32(v2, val);
    return _mm256_or_si256(
        _mm256_and_si256(cmp3, _mm256_or_si256(_mm256_and_si256(cmp1, a1), _mm256_andnot_si256(cmp1, a2))),
        _mm256_andnot_si256(cmp3, _mm256_or_si256(_mm256_and_si256(cmp2, a3), _mm256_andnot_si256(cmp2, a4)))
    );
}

static inline __m256i select8ModeOrRandom(__m256i* cs, int ws, __m256i a1, __m256i a2, __m256i a3, __m256i a4)
{
    __m256i cmp1 = _mm256_cmpeq_epi32(a1, a2);
    __m256i cmp2 = _mm256_cmpeq_epi32(a1, a3);
    __m256i cmp3 = _mm256_cmpeq_epi32(a1, a4);
    __m256i cmp4 = _mm256_cmpeq_epi32(a2, a3);
    __m256i cmp5 = _mm256_cmpeq_epi32(a2, a4);
    __m256i cmp6 = _mm256_cmpeq_epi32(a3, a4);
    __m256i isa1 = _mm256_or_si256(
                       _mm256_andnot_si256(cmp6, cmp1),
                       _mm256_or_si256 (
                           _mm256_andnot_si256(cmp5, cmp2),
                           _mm256_andnot_si256(cmp4, cmp3)
                       )
                   );
    __m256i isa2 = _mm256_or_si256(
                       _mm256_andnot_si256(cmp3, cmp4),
                       _mm256_andnot_si256(cmp2, cmp5)
                   );
    __m256i isa3 = _mm256_andnot_si256(cmp1, cmp6);

    return _mm256_or_si256(
        _mm256_andnot_si256(
            _mm256_or_si256(
                isa1,
                _mm256_or_si256(isa2, isa3)
            ),
            select8Random4(cs, ws, a1, a2, a3, a4)
        ),
        _mm256_or_si256(
            _mm256_and_si256(isa1, a1),
            _mm256_or_si256(
                _mm256_and_si256(isa2, a2),
                _mm256_and_si256(isa3, a3)
            )
        )
    );
}

#elif defined USE_SIMD && defined __SSE4_2__

static inline __m128i set4ChunkSeeds(int ws, __m128i xs, __m128i zs)
{
    __m128i out = _mm_set1_epi32(ws);
    __m128i mul = _mm_set1_epi32(1284865837);
    __m128i add = _mm_set1_epi32(4150755663);
    out = _mm_add_epi32(xs, _mm_mullo_epi32(out, _mm_add_epi32(add, _mm_mullo_epi32(out, mul))));
    out = _mm_add_epi32(zs, _mm_mullo_epi32(out, _mm_add_epi32(add, _mm_mullo_epi32(out, mul))));
    out = _mm_add_epi32(xs, _mm_mullo_epi32(out, _mm_add_epi32(add, _mm_mullo_epi32(out, mul))));
    return _mm_add_epi32(zs, _mm_mullo_epi32(out, _mm_add_epi32(add, _mm_mullo_epi32(out, mul))));
}

static inline __m128i mc4NextInt(__m128i* cs, int ws, int mask)
{
    __m128i and = _mm_set1_epi32(mask);
    __m128i ret = _mm_and_si128(and, _mm_srli_epi32(*cs, 24));
    *cs = _mm_add_epi32( _mm_set1_epi32(ws), _mm_mullo_epi32(*cs, _mm_add_epi32(_mm_set1_epi32(4150755663), _mm_mullo_epi32(*cs, _mm_set1_epi32(1284865837)))));
    return _mm_add_epi32(ret, _mm_and_si128(and, _mm_cmplt_epi32(ret, _mm_set1_epi32(0))));;
}

static inline __m128i select4Random2(__m128i* cs, int ws, __m128i a1, __m128i a2)
{
    __m128i cmp = _mm_cmpeq_epi32(_mm_set1_epi32(0), mc4NextInt(cs, ws, 0x1));
    return _mm_or_si128(_mm_and_si128(cmp, a1), _mm_andnot_si128(cmp, a2));
}

static inline __m128i select4Random4(__m128i* cs, int ws, __m128i a1, __m128i a2, __m128i a3, __m128i a4)
{
    __m128i val = mc4NextInt(cs, ws, 0x3);
    __m128i v2 = _mm_set1_epi32(2);
    __m128i cmp1 = _mm_cmpeq_epi32(val, _mm_set1_epi32(0));
    __m128i cmp2 = _mm_cmpeq_epi32(val, v2);
    __m128i cmp3 = _mm_cmplt_epi32(val, v2);
    return _mm_or_si128(
        _mm_and_si128(cmp3, _mm_or_si128(_mm_and_si128(cmp1, a1), _mm_andnot_si128(cmp1, a2))),
        _mm_andnot_si128(cmp3, _mm_or_si128(_mm_and_si128(cmp2, a3), _mm_andnot_si128(cmp2, a4)))
    );
}

static inline __m128i select4ModeOrRandom(__m128i* cs, int ws, __m128i a1, __m128i a2, __m128i a3, __m128i a4)
{
    //((a == b)&(c != d) | (a == c)&(b != d) | (a == d)&(b != c))&a | ((b == c)&(a != d) | (b == d)&(a != c))&b | ((c == d)&(a != b))&c
    __m128i cmp1 = _mm_cmpeq_epi32(a1, a2);
    __m128i cmp2 = _mm_cmpeq_epi32(a1, a3);
    __m128i cmp3 = _mm_cmpeq_epi32(a1, a4);
    __m128i cmp4 = _mm_cmpeq_epi32(a2, a3);
    __m128i cmp5 = _mm_cmpeq_epi32(a2, a4);
    __m128i cmp6 = _mm_cmpeq_epi32(a3, a4);
    __m128i isa1 = _mm_or_si128(
                       _mm_andnot_si128(cmp6, cmp1),
                       _mm_or_si128 (
                           _mm_andnot_si128(cmp5, cmp2),
                           _mm_andnot_si128(cmp4, cmp3)
                       )
                   );
    __m128i isa2 = _mm_or_si128(
                       _mm_andnot_si128(cmp3, cmp4),
                       _mm_andnot_si128(cmp2, cmp5)
                   );
    __m128i isa3 = _mm_andnot_si128(cmp1, cmp6);
    return _mm_or_si128(
        _mm_andnot_si128(
            _mm_or_si128(
                isa1,
                _mm_or_si128(isa2, isa3)
            ),
            select4Random4(cs, ws, a1, a2, a3, a4)
        ),
        _mm_or_si128(
            _mm_and_si128(isa1, a1),
            _mm_or_si128(
                _mm_and_si128(isa2, a2),
                _mm_and_si128(isa3, a3)
            )
        )
    );
}

#else

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

    if (a2 == a3 && a3 == a4) return a2;
    if (a1 == a2 && a1 == a3) return a1;
    if (a1 == a2 && a1 == a4) return a1;
    if (a1 == a3 && a1 == a4) return a1;
    if (a1 == a2 && a3 != a4) return a1;
    if (a1 == a3 && a2 != a4) return a1;
    if (a1 == a4 && a2 != a3) return a1;
    if (a2 == a3 && a1 != a4) return a2;
    if (a2 == a4 && a1 != a3) return a2;
    if (a3 == a4 && a1 != a2) return a3;

    return rndarg;
}

#endif

//==============================================================================
// Layers
//==============================================================================

// A null layer does nothing, and can be used to apply a layer to existing data.
void mapNull(Layer *l, int * __restrict out, int x, int z, int w, int h);
// A skip layer simply calls its first parent without modification.
// This can be used as an easy way to skip a layer in a generator.
void mapSkip(Layer *l, int * __restrict out, int x, int z, int w, int h);

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

// 1.13 layers
void mapHills113(Layer *l, int * __restrict out, int x, int z, int w, int h);
void mapOceanTemp(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight);
void mapOceanMix(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight);

void mapVoronoiZoom(Layer *l, int * __restrict out, int x, int z, int w, int h);

#endif /* LAYER_H_ */
