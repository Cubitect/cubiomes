#include "layers.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static void oceanRndInit(OceanRnd *rnd, int64_t seed);


void initAddBiome(int id, int tempCat, int biometype, float temp, float height)
{
    if (id & (~0xff)) return;
    biomes[id].id = id;
    biomes[id].type = biometype;
    biomes[id].temp = temp;
    biomes[id].height = height;
    biomes[id].tempCat = tempCat;
}

void createMutation(int id)
{
    biomes[id+128] = biomes[id];
    biomes[id+128].id = id+128;
}

/* initBiomes() has to be called before any of the generators can be used */
void initBiomes()
{
    int i;
    for (i = 0; i < 256; i++) biomes[i].id = none;

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

    // TODO: determine the 1.13 biome properties
    initAddBiome(skyIslandLow, Lush, Sky, 0, 0);
    initAddBiome(skyIslandMedium, Lush, Sky, 0, 0);
    initAddBiome(skyIslandHigh, Lush, Sky, 0, 0);
    initAddBiome(skyIslandBarren, Lush, Sky, 0, 0);
    initAddBiome(warmOcean, Oceanic, Ocean, 0, 0);
    initAddBiome(lukewarmOcean, Oceanic, Ocean, 0, 0);
    initAddBiome(coldOcean, Oceanic, Ocean, 0, 0);
    initAddBiome(warmDeepOcean, Oceanic, Ocean, 0, 0);
    initAddBiome(lukewarmDeepOcean, Oceanic, Ocean, 0, 0);
    initAddBiome(coldDeepOcean, Oceanic, Ocean, 0, 0);
    initAddBiome(frozenDeepOcean, Oceanic, Ocean, 0, 0);

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


void setWorldSeed(Layer *layer, int64_t seed)
{
    if (layer->p2 != NULL && layer->getMap != mapHills)
        setWorldSeed(layer->p2, seed);

    if (layer->p != NULL)
        setWorldSeed(layer->p, seed);

    if (layer->oceanRnd != NULL)
        oceanRndInit(layer->oceanRnd, seed);

    layer->worldSeed = seed;
    layer->worldSeed *= layer->worldSeed * 6364136223846793005LL + 1442695040888963407LL;
    layer->worldSeed += layer->baseSeed;
    layer->worldSeed *= layer->worldSeed * 6364136223846793005LL + 1442695040888963407LL;
    layer->worldSeed += layer->baseSeed;
    layer->worldSeed *= layer->worldSeed * 6364136223846793005LL + 1442695040888963407LL;
    layer->worldSeed += layer->baseSeed;
}


void mapNull(Layer *l, int * __restrict out, int x, int z, int w, int h)
{
}

void mapSkip(Layer *l, int * __restrict out, int x, int z, int w, int h)
{
    if (l->p == NULL)
    {
        printf("mapSkip() requires a non-null parent layer.\n");
        exit(1);
    }
    l->p->getMap(l->p, out, x, z, w, h);
}


void mapIsland(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    register int x, z;

    const int64_t ws = l->worldSeed;
    const int64_t ss = ws * (ws * 6364136223846793005LL + 1442695040888963407LL);

    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            const int64_t chunkX = (int64_t)(x + areaX);
            const int64_t chunkZ = (int64_t)(z + areaZ);
            register int64_t cs = ss;
            cs += chunkX;
            cs *= cs * 6364136223846793005LL + 1442695040888963407LL;
            cs += chunkZ;
            cs *= cs * 6364136223846793005LL + 1442695040888963407LL;
            cs += chunkX;
            cs *= cs * 6364136223846793005LL + 1442695040888963407LL;
            cs += chunkZ;

            out[x + z*areaWidth] = (cs >> 24) % 10 == 0;
        }
    }

    if (areaX > -areaWidth && areaX <= 0 && areaZ > -areaHeight && areaZ <= 0)
    {
        out[-areaX + -areaZ * areaWidth] = 1;
    }
}

#if defined USE_SIMD && defined __AVX2__

void mapZoom(Layer *l, int* __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pWidth = (areaWidth>>1)+2, pHeight = (areaHeight>>1)+1;
    
    l->p->getMap(l->p, out, areaX>>1, areaZ>>1, pWidth, pHeight+1);
    
    __m256i (*selectRand)(__m256i* cs, int ws, __m256i a1, __m256i a2, __m256i a3, __m256i a4) = (l->p->getMap == mapIsland) ? select8Random4 : select8ModeOrRandom;
    int newWidth = (areaWidth+10)&0xFFFFFFFE;//modified to ignore ends
    int x, z;
    __m256i cs, a, b, a1, b1, toBuf1, toBuf2, aSuf;
    __m256i mask1 = _mm256_setr_epi32(0xFFFFFFFF, 0x0, 0xFFFFFFFF, 0x0, 0xFFFFFFFF, 0x0, 0xFFFFFFFF, 0x0), mask2 = _mm256_setr_epi32(0x0, 0xFFFFFFFF, 0x0, 0xFFFFFFFF, 0x0, 0xFFFFFFFF, 0x0, 0xFFFFFFFF);
    __m256i shuffle = _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7);
    int pX = areaX&0xFFFFFFFE;
    __m256i xs = _mm256_set_epi32(pX+14, pX+12, pX+10, pX+8, pX+6, pX+4, pX+2, pX), zs;
    __m256i v2 = _mm256_set1_epi32(2), v16 = _mm256_set1_epi32(16);
    int* buf = malloc((newWidth+1)*((areaHeight+2)|1)*sizeof(*buf));
    int* idx = buf;
    int* outIdx = out;    
    //z first!
    for (x = 0; x < pWidth-1; x += 8)
    {
        a = _mm256_loadu_si256((__m256i*)(outIdx));//0, 0
        b = _mm256_loadu_si256((__m256i*)(outIdx+1));//1, 0
        zs = _mm256_set1_epi32(areaZ&0xFFFFFFFE);
        for (z = 0; z < pHeight; z++)
        {
            cs = set8ChunkSeeds(l->worldSeed, xs, zs);
            outIdx += pWidth;
            a1 = _mm256_loadu_si256((__m256i*)(outIdx));//0, 1
            b1 = _mm256_loadu_si256((__m256i*)(outIdx+1));//1, 1
            toBuf1 = _mm256_permutevar8x32_epi32(select8Random2(&cs, l->worldSeed, a, a1), shuffle);
            toBuf2 = _mm256_permutevar8x32_epi32(select8Random2(&cs, l->worldSeed, a, b), shuffle);
            aSuf = _mm256_permutevar8x32_epi32(a, shuffle);
            _mm256_maskstore_epi32(idx, mask1, aSuf);
            _mm256_maskstore_epi32(idx+1, mask1, toBuf2);
            _mm256_maskstore_epi32(idx+7, mask2, aSuf);
            _mm256_maskstore_epi32(idx+8, mask2, toBuf2);
            idx += newWidth;
            toBuf2 = _mm256_permutevar8x32_epi32(selectRand(&cs, l->worldSeed, a, b, a1, b1), shuffle);
            _mm256_maskstore_epi32(idx, mask1, toBuf1);
            _mm256_maskstore_epi32(idx+1, mask1, toBuf2);
            _mm256_maskstore_epi32(idx+7, mask2, toBuf1);
            _mm256_maskstore_epi32(idx+8, mask2, toBuf2);
            idx += newWidth;
            a = a1;
            b = b1;
            zs = _mm256_add_epi32(zs, v2);
        }
        outIdx += 8-pHeight*pWidth;        
        idx += 16-pHeight*2*newWidth;    
        xs = _mm256_add_epi32(xs, v16);
    }

    for (z = 0; z < areaHeight; z++)
    {
        memcpy(&out[z*areaWidth], &buf[(z + (areaZ & 1))*newWidth + (areaX & 1)], areaWidth*sizeof(int));
    }

    free(buf);
}

#elif defined USE_SIMD && defined __SSE4_2__

void mapZoom(Layer *l, int* __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pWidth = (areaWidth>>1)+2, pHeight = (areaHeight>>1)+1;
    
    l->p->getMap(l->p, out, areaX>>1, areaZ>>1, pWidth, pHeight+1);
    
    __m128i (*selectRand)(__m128i* cs, int ws, __m128i a1, __m128i a2, __m128i a3, __m128i a4) = (l->p->getMap == mapIsland) ? select4Random4 : select4ModeOrRandom;
    int newWidth = areaWidth+6&0xFFFFFFFE;//modified to ignore ends
    int x, z;
    __m128i cs, a, b, a1, b1, toBuf1, toBuf2, aSuf;
    __m128i mask1 = _mm_setr_epi32(0xFFFFFFFF, 0x0, 0xFFFFFFFF, 0x0), mask2 = _mm_setr_epi32(0x0, 0xFFFFFFFF, 0x0, 0xFFFFFFFF);
    int pX = areaX&0xFFFFFFFE;
    __m128i xs = _mm_set_epi32(pX+6, pX+4, pX+2, pX), zs;
    __m128i v2 = _mm_set1_epi32(2), v8 = _mm_set1_epi32(8);
    int* buf = malloc((newWidth+1)*(areaHeight+2|1)*sizeof(*buf));
    int* idx = buf;
    int* outIdx = out;
    //z first!
    for (x = 0; x < pWidth-1; x += 4)
    {
        a = _mm_loadu_si128((__m128i_u*)(outIdx));//0, 0
        b = _mm_loadu_si128((__m128i_u*)(outIdx+1));//1, 0
        zs = _mm_set1_epi32(areaZ&0xFFFFFFFE);
        for (z = 0; z < pHeight; z++)
        {
            cs = set4ChunkSeeds(l->worldSeed, xs, zs);
            outIdx += pWidth;
            a1 = _mm_loadu_si128((__m128i_u*)(outIdx));//0, 1
            b1 = _mm_loadu_si128((__m128i_u*)(outIdx+1));//1, 1
            toBuf1 = _mm_shuffle_epi32(select4Random2(&cs, l->worldSeed, a, a1), 0xD8);//11011000->3120->1324
            toBuf2 = _mm_shuffle_epi32(select4Random2(&cs, l->worldSeed, a, b), 0xD8);
            aSuf = _mm_shuffle_epi32(a, 0xD8);
            _mm_maskmoveu_si128(aSuf, mask1, (char*)(idx));
            _mm_maskmoveu_si128(toBuf2, mask1, (char*)(idx+1));
            _mm_maskmoveu_si128(aSuf, mask2, (char*)(idx+3));
            _mm_maskmoveu_si128(toBuf2, mask2, (char*)(idx+4));
            idx += newWidth;
            toBuf2 = _mm_shuffle_epi32(selectRand(&cs, l->worldSeed, a, b, a1, b1), 0xD8);
            _mm_maskmoveu_si128(toBuf1, mask1, (char*)(idx));
            _mm_maskmoveu_si128(toBuf2, mask1, (char*)(idx+1));
            _mm_maskmoveu_si128(toBuf1, mask2, (char*)(idx+3));
            _mm_maskmoveu_si128(toBuf2, mask2, (char*)(idx+4));
            idx += newWidth;            
            a = a1;
            b = b1;
            zs = _mm_add_epi32(zs, v2);
        }
        outIdx += 4-pHeight*pWidth;
        idx += 8-pHeight*2*newWidth;
        xs = _mm_add_epi32(xs, v8);
    }

    for (z = 0; z < areaHeight; z++)
    {
        memcpy(&out[z*areaWidth], &buf[(z + (areaZ & 1))*newWidth + (areaX & 1)], areaWidth*sizeof(int));
    }

    free(buf);
}

#else

void mapZoom(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pX = areaX >> 1;
    int pZ = areaZ >> 1;
    int pWidth =  (areaWidth >> 1) + 2;
    int pHeight = (areaHeight >> 1) + 2;
    int x, z;

    l->p->getMap(l->p, out, pX, pZ, pWidth, pHeight);

    int newWidth = (pWidth-1) << 1;
    int newHeight = (pHeight-1) << 1;
    int idx, a, b;
    int *buf = (int *)malloc((newWidth+1)*(newHeight+1)*sizeof(*buf));

    const int ws = (int)l->worldSeed;
    const int ss = ws * (ws * 1284865837 + 4150755663);

    for (z = 0; z < pHeight - 1; z++)
    {
        idx = (z << 1) * newWidth;
        a = out[(z+0)*pWidth];
        b = out[(z+1)*pWidth];

        for (x = 0; x < pWidth - 1; x++)
        {
            int a1 = out[x+1 + (z+0)*pWidth];
            int b1 = out[x+1 + (z+1)*pWidth];

            const int chunkX = (x + pX) << 1;
            const int chunkZ = (z + pZ) << 1;

            register int cs = ss;
            cs += chunkX;
            cs *= cs * 1284865837 + 4150755663;
            cs += chunkZ;
            cs *= cs * 1284865837 + 4150755663;
            cs += chunkX;
            cs *= cs * 1284865837 + 4150755663;
            cs += chunkZ;

            buf[idx] = a;
            buf[idx + newWidth] = (cs >> 24) & 1 ? b : a;
            idx++;

            cs *= cs * 1284865837 + 4150755663;
            cs += ws;
            buf[idx] = (cs >> 24) & 1 ? a1 : a;


            if (l->p->getMap == mapIsland)
            {
                //selectRandom4
                cs *= cs * 1284865837 + 4150755663;
                cs += ws;
                const int i = (cs >> 24) & 3;
                buf[idx + newWidth] = i==0 ? a : i==1 ? a1 : i==2 ? b : b1;
            }
            else
            {
                //selectModeOrRandom
                if      (a1 == b  && b  == b1) buf[idx + newWidth] = a1;
                else if (a  == a1 && a  == b ) buf[idx + newWidth] = a;
                else if (a  == a1 && a  == b1) buf[idx + newWidth] = a;
                else if (a  == b  && a  == b1) buf[idx + newWidth] = a;
                else if (a  == a1 && b  != b1) buf[idx + newWidth] = a;
                else if (a  == b  && a1 != b1) buf[idx + newWidth] = a;
                else if (a  == b1 && a1 != b ) buf[idx + newWidth] = a;
                else if (a1 == b  && a  != b1) buf[idx + newWidth] = a1;
                else if (a1 == b1 && a  != b ) buf[idx + newWidth] = a1;
                else if (b  == b1 && a  != a1) buf[idx + newWidth] = b;
                else
                {
                    cs *= cs * 1284865837 + 4150755663;
                    cs += ws;
                    const int i = (cs >> 24) & 3;
                    buf[idx + newWidth] = i==0 ? a : i==1 ? a1 : i==2 ? b : b1;
                }
            }

            idx++;
            a = a1;
            b = b1;
        }
    }

    for (z = 0; z < areaHeight; z++)
    {
        memcpy(&out[z*areaWidth], &buf[(z + (areaZ & 1))*newWidth + (areaX & 1)], areaWidth*sizeof(int));
    }

    free(buf);
}
#endif

void mapAddIsland(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pX = areaX - 1;
    int pZ = areaZ - 1;
    int pWidth = areaWidth + 2;
    int pHeight = areaHeight + 2;
    int x, z;

    l->p->getMap(l->p, out, pX, pZ, pWidth, pHeight);

    const int64_t ws = l->worldSeed;
    const int64_t ss = ws * (ws * 6364136223846793005LL + 1442695040888963407LL);

    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            int v00 = out[x+0 + (z+0)*pWidth];
            int v20 = out[x+2 + (z+0)*pWidth];
            int v02 = out[x+0 + (z+2)*pWidth];
            int v22 = out[x+2 + (z+2)*pWidth];
            int v11 = out[x+1 + (z+1)*pWidth];

            if (v11 == 0 && (v00 != 0 || v20 != 0 || v02 != 0 || v22 != 0))
            {
                const int64_t chunkX = (int64_t)(x + areaX);
                const int64_t chunkZ = (int64_t)(z + areaZ);
                register int64_t cs = ss;
                cs += chunkX;
                cs *= cs * 6364136223846793005LL + 1442695040888963407LL;
                cs += chunkZ;
                cs *= cs * 6364136223846793005LL + 1442695040888963407LL;
                cs += chunkX;
                cs *= cs * 6364136223846793005LL + 1442695040888963407LL;
                cs += chunkZ;

                int v = 1;
                int inc = 0;

                if (v00 != 0)
                {
                    ++inc; v = v00;
                    cs *= cs * 6364136223846793005LL + 1442695040888963407LL;
                    cs += ws;
                }
                if (v20 != 0)
                {
                    if (++inc == 1 || (cs & (1LL << 24)) == 0) v = v20;
                    cs *= cs * 6364136223846793005LL + 1442695040888963407LL;
                    cs += ws;
                }
                if (v02 != 0)
                {
                    switch(++inc)
                    {
                    case 1: v = v02; break;
                    case 2: if ((cs & (1LL << 24)) == 0) v = v02; break;
                    default: if (((cs >> 24) % 3) == 0) v = v02;
                    }
                    cs *= cs * 6364136223846793005LL + 1442695040888963407LL;
                    cs += ws;
                }
                if (v22 != 0)
                {
                    switch(++inc)
                    {
                    case 1: v = v22; break;
                    case 2: if ((cs & (1LL << 24)) == 0) v = v22; break;
                    case 3: if (((cs >> 24) % 3) == 0) v = v22; break;
                    default: if ((cs & (3LL << 24)) == 0) v = v22;
                    }
                    cs *= cs * 6364136223846793005LL + 1442695040888963407LL;
                    cs += ws;
                }

                if ((cs >> 24) % 3 == 0)
                    out[x + z*areaWidth] = v;
                else if (v == 4)
                    out[x + z*areaWidth] = 4;
                else
                    out[x + z*areaWidth] = 0;
            }
            else if (v11 > 0 && (v00 == 0 || v20 == 0 || v02 == 0 || v22 == 0))
            {
                //setChunkSeed(l, (int64_t)(x + areaX), (int64_t)(z + areaZ));
                //if (mcNextInt(l, 5) == 0)...

                const int64_t chunkX = (int64_t)(x + areaX);
                const int64_t chunkZ = (int64_t)(z + areaZ);

                register int64_t cs = ss;
                cs += chunkX;
                cs *= cs * 6364136223846793005LL + 1442695040888963407LL;
                cs += chunkZ;
                cs *= cs * 6364136223846793005LL + 1442695040888963407LL;
                cs += chunkX;
                cs *= cs * 6364136223846793005LL + 1442695040888963407LL;
                cs += chunkZ;

                if ((cs >> 24) % 5 == 0)
                    out[x + z*areaWidth] = (v11 == 4) ? 4 : 0;
                else
                    out[x + z*areaWidth] = v11;
            }
            else
            {
                out[x + z*areaWidth] = v11;
            }
        }
    }
}


void mapRemoveTooMuchOcean(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pX = areaX - 1;
    int pZ = areaZ - 1;
    int pWidth = areaWidth + 2;
    int pHeight = areaHeight + 2;
    int x, z;

    l->p->getMap(l->p, out, pX, pZ, pWidth, pHeight);

    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            int v11 = out[x+1 + (z+1)*pWidth];
            out[x + z*areaWidth] = v11;

            if (out[x+1 + (z+0)*pWidth] != 0) continue;
            if (out[x+2 + (z+1)*pWidth] != 0) continue;
            if (out[x+0 + (z+1)*pWidth] != 0) continue;
            if (out[x+1 + (z+2)*pWidth] != 0) continue;

            if (v11 == 0)
            {
                setChunkSeed(l, (int64_t)(x + areaX), (int64_t)(z + areaZ));

                if (mcNextInt(l, 2) == 0)
                {
                    out[x + z*areaWidth] = 1;
                }
            }
        }
    }
}


void mapAddSnow(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pX = areaX - 1;
    int pZ = areaZ - 1;
    int pWidth = areaWidth + 2;
    int pHeight = areaHeight + 2;
    int x, z;

    l->p->getMap(l->p, out, pX, pZ, pWidth, pHeight);
    
    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            int v11 = out[x+1 + (z+1)*pWidth];

            if (isShallowOcean(v11))
            {
                out[x + z*areaWidth] = v11;
            }
            else
            {
                setChunkSeed(l, (int64_t)(x + areaX), (int64_t)(z + areaZ));
                int r = mcNextInt(l, 6);
                int v;

                if (r == 0)      v = 4;
                else if (r <= 1) v = 3;
                else            v = 1;

                out[x + z*areaWidth] = v;
            }
        }
    }
}




void mapCoolWarm(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pX = areaX - 1;
    int pZ = areaZ - 1;
    int pWidth = areaWidth + 2;
    int pHeight = areaHeight + 2;
    int x, z;

    l->p->getMap(l->p, out, pX, pZ, pWidth, pHeight);

    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            int v11 = out[x+1 + (z+1)*pWidth];

            if (v11 == 1)
            {
                int v10 = out[x+1 + (z+0)*pWidth];
                int v21 = out[x+2 + (z+1)*pWidth];
                int v01 = out[x+0 + (z+1)*pWidth];
                int v12 = out[x+1 + (z+2)*pWidth];

                if (v10 == 3 || v10 == 4 || v21 == 3 || v21 == 4 || v01 == 3 || v01 == 4 || v12 == 3 || v12 == 4)
                {
                    v11 = 2;
                }
            }

            out[x + z*areaWidth] = v11;
        }
    }
}


void mapHeatIce(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pX = areaX - 1;
    int pZ = areaZ - 1;
    int pWidth = areaWidth + 2;
    int pHeight = areaHeight + 2;
    int x, z;

    l->p->getMap(l->p, out, pX, pZ, pWidth, pHeight);

    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            int v11 = out[x+1 + (z+1)*pWidth];

            if (v11 == 4)
            {
                int v10 = out[x+1 + (z+0)*pWidth];
                int v21 = out[x+2 + (z+1)*pWidth];
                int v01 = out[x+0 + (z+1)*pWidth];
                int v12 = out[x+1 + (z+2)*pWidth];

                if (v10 == 1 || v10 == 2 || v21 == 1 || v21 == 2 || v01 == 1 || v01 == 2 || v12 == 1 || v12 == 2)
                {
                    v11 = 3;
                }
            }

            out[x + z*areaWidth] = v11;
        }
    }
}


void mapSpecial(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    l->p->getMap(l->p, out, areaX, areaZ, areaWidth, areaHeight);

    int x, z;
    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            int v = out[x + z*areaWidth];
            if (v == 0) continue;

            setChunkSeed(l, (int64_t)(x + areaX), (int64_t)(z + areaZ));

            if (mcNextInt(l, 13) == 0)
            {
                v |= (1 + mcNextInt(l, 15)) << 8 & 0xf00;
                // 1 to 1 mapping so 'out' can be overwritten immediately
                out[x + z*areaWidth] = v;
            }
        }
    }
}


void mapAddMushroomIsland(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pX = areaX - 1;
    int pZ = areaZ - 1;
    int pWidth = areaWidth + 2;
    int pHeight = areaHeight + 2;
    int x, z;

    l->p->getMap(l->p, out, pX, pZ, pWidth, pHeight);

    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            int v11 = out[x+1 + (z+1)*pWidth];

            // surrounded by ocean?
            if (v11 == 0 && !out[x+0 + (z+0)*pWidth] && !out[x+2 + (z+0)*pWidth] && !out[x+0 + (z+2)*pWidth] && !out[x+2 + (z+2)*pWidth])
            {
                setChunkSeed(l, (int64_t)(x + areaX), (int64_t)(z + areaZ));
                if (mcNextInt(l, 100) == 0) {
                    out[x + z*areaWidth] = mushroomIsland;
                    continue;
                }
            }

            out[x + z*areaWidth] = v11;
        }
    }
}


void mapDeepOcean(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pX = areaX - 1;
    int pZ = areaZ - 1;
    int pWidth = areaWidth + 2;
    int pHeight = areaHeight + 2;
    int x, z;

    l->p->getMap(l->p, out, pX, pZ, pWidth, pHeight);

    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            int v11 = out[(x+1) + (z+1)*pWidth];

            if (isShallowOcean(v11))
            {
                // count adjacent oceans
                int oceans = 0;
                if (isShallowOcean(out[(x+1) + (z+0)*pWidth])) oceans++;
                if (isShallowOcean(out[(x+2) + (z+1)*pWidth])) oceans++;
                if (isShallowOcean(out[(x+0) + (z+1)*pWidth])) oceans++;
                if (isShallowOcean(out[(x+1) + (z+2)*pWidth])) oceans++;

                if (oceans > 3)
                {
                    if (v11 == warmOcean)
                        v11 = warmDeepOcean;
                    else if (v11 == lukewarmOcean)
                        v11 = lukewarmDeepOcean;
                    else if (v11 == ocean)
                        v11 = deepOcean;
                    else if (v11 == coldOcean)
                        v11 = coldDeepOcean;
                    else if (v11 == frozenOcean)
                        v11 = frozenDeepOcean;
                    else
                        v11 = deepOcean;
                }
            }

            out[x + z*areaWidth] = v11;
        }
    }
}


const int warmBiomes[] = {desert, desert, desert, savanna, savanna, plains};
const int lushBiomes[] = {forest, roofedForest, extremeHills, plains, birchForest, swampland};
const int coldBiomes[] = {forest, extremeHills, taiga, plains};
const int snowBiomes[] = {icePlains, icePlains, icePlains, coldTaiga};

void mapBiome(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    l->p->getMap(l->p, out, areaX, areaZ, areaWidth, areaHeight);

    int x, z;
    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            int idx = x + z*areaWidth;
            int id = out[idx];
            int hasHighBit = (id & 0xf00) >> 8;
            id &= -0xf01;

            if (getBiomeType(id) == Ocean || id == mushroomIsland)
            {
                out[idx] = id;
                continue;
            }

            setChunkSeed(l, (int64_t)(x + areaX), (int64_t)(z + areaZ));

            switch(id){
            case Warm:
                if (hasHighBit) out[idx] = (mcNextInt(l, 3) == 0) ? mesaPlateau : mesaPlateau_F;
                else out[idx] = warmBiomes[mcNextInt(l, 6)];
                break;
            case Lush:
                if (hasHighBit) out[idx] = jungle;
                else out[idx] = lushBiomes[mcNextInt(l, 6)];
                break;
            case Cold:
                if (hasHighBit) out[idx] = megaTaiga;
                else out[idx] = coldBiomes[mcNextInt(l, 4)];
                break;
            case Freezing:
                out[idx] = snowBiomes[mcNextInt(l, 4)];
                break;
            default:
                out[idx] = mushroomIsland;
            }
        }
    }
}


void mapRiverInit(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    l->p->getMap(l->p, out, areaX, areaZ, areaWidth, areaHeight);

    int x, z;
    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            if (out[x + z*areaWidth] > 0)
            {
                setChunkSeed(l, (int64_t)(x + areaX), (int64_t)(z + areaZ));
                out[x + z*areaWidth] = mcNextInt(l, 299999)+2;
            }
            else
            {
                out[x + z*areaWidth] = 0;
            }
        }
    }
}


// replaceEdgeIfNecessary() always returns 0 in the only place it is used in
// Minecraft, making it redundant.
/*
static inline int replaceEdgeIfNecessary(int *out, int idx, int v10, int v21, int v01, int v12, int id, int baseID, int edgeID)
{
    if (!equalOrPlateau(id, baseID)) return 0;

    if (canBeNeighbors(v10, baseID) && canBeNeighbors(v21, baseID) && canBeNeighbors(v01, baseID) && canBeNeighbors(v12, baseID))
        out[idx] = id;
    else
        out[idx] = edgeID;

    return 1;
}
*/

static inline int replaceEdge(int *out, int idx, int v10, int v21, int v01, int v12, int id, int baseID, int edgeID)
{
    if (id != baseID) return 0;

    if (equalOrPlateau(v10, baseID) && equalOrPlateau(v21, baseID) && equalOrPlateau(v01, baseID) && equalOrPlateau(v12, baseID))
        out[idx] = id;
    else
        out[idx] = edgeID;

    return 1;
}

void mapBiomeEdge(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pX = areaX - 1;
    int pZ = areaZ - 1;
    int pWidth = areaWidth + 2;
    int pHeight = areaHeight + 2;
    int x, z;

    l->p->getMap(l->p, out, pX, pZ, pWidth, pHeight);

    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            int v11 = out[x+1 + (z+1)*pWidth];

            int v10 = out[x+1 + (z+0)*pWidth];
            int v21 = out[x+2 + (z+1)*pWidth];
            int v01 = out[x+0 + (z+1)*pWidth];
            int v12 = out[x+1 + (z+2)*pWidth];

            if (/*!replaceEdgeIfNecessary(out, x + z*areaWidth, v10, v21, v01, v12, v11, extremeHills, extremeHillsEdge) &&*/
               !replaceEdge(out, x + z*areaWidth, v10, v21, v01, v12, v11, mesaPlateau_F, mesa) &&
               !replaceEdge(out, x + z*areaWidth, v10, v21, v01, v12, v11, mesaPlateau, mesa) &&
               !replaceEdge(out, x + z*areaWidth, v10, v21, v01, v12, v11, megaTaiga, taiga))
            {
                if (v11 == desert)
                {
                    if (v10 != icePlains && v21 != icePlains && v01 != icePlains && v12 != icePlains)
                    {
                        out[x + z*areaWidth] = v11;
                    }
                    else
                    {
                        out[x + z*areaWidth] = extremeHillsPlus;
                    }
                }
                else if (v11 == swampland)
                {
                    if (v10 != desert && v21 != desert && v01 != desert && v12 != desert &&
                       v10 != coldTaiga && v21 != coldTaiga && v01 != coldTaiga && v12 != coldTaiga &&
                       v10 != icePlains && v21 != icePlains && v01 != icePlains && v12 != icePlains)
                    {
                        if (v10 != jungle && v12 != jungle && v21 != jungle && v01 != jungle)
                            out[x + z*areaWidth] = v11;
                        else
                            out[x + z*areaWidth] = jungleEdge;
                    }
                    else
                    {
                        out[x + z*areaWidth] = plains;
                    }
                }
                else
                {
                    out[x + z*areaWidth] = v11;
                }
            }
        }
    }
}


void mapHills(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pX = areaX - 1;
    int pZ = areaZ - 1;
    int pWidth = areaWidth + 2;
    int pHeight = areaHeight + 2;
    int x, z;
    int *buf = NULL;

    if (l->p2 == NULL)
    {
        printf("mapHills() requires two parents! Use setupMultiLayer()\n");
        exit(1);
    }

    buf = (int *) malloc(pWidth*pHeight*sizeof(int));

    l->p->getMap(l->p, out, pX, pZ, pWidth, pHeight);
    memcpy(buf, out, pWidth*pHeight*sizeof(int));

    l->p2->getMap(l->p2, out, pX, pZ, pWidth, pHeight);

    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            setChunkSeed(l, (int64_t)(x + areaX), (int64_t)(z + areaZ));
            int a11 = buf[x+1 + (z+1)*pWidth]; // biome branch
            int b11 = out[x+1 + (z+1)*pWidth]; // river branch
            int idx = x + z*areaWidth;

            int var12 = (b11 - 2) % 29 == 0;

            if (a11 != 0 && b11 >= 2 && (b11 - 2) % 29 == 1 && a11 < 128)
            {
                out[idx] = (biomeExists(a11 + 128)) ? a11 + 128 : a11;
            }
            else if (mcNextInt(l, 3) != 0 && !var12)
            {
                out[idx] = a11;
            }
            else
            {
                int hillID = a11;

                switch(a11){
                case desert:
                    hillID = desertHills; break;
                case forest:
                    hillID = forestHills; break;
                case birchForest:
                    hillID = birchForestHills; break;
                case roofedForest:
                    hillID = plains; break;
                case taiga:
                    hillID = taigaHills; break;
                case megaTaiga:
                    hillID = megaTaigaHills; break;
                case coldTaiga:
                    hillID = coldTaigaHills; break;
                case plains:
                    hillID = (mcNextInt(l, 3) == 0) ? forestHills : forest; break;
                case icePlains:
                    hillID = iceMountains; break;
                case jungle:
                    hillID = jungleHills; break;
                case ocean:
                    hillID = deepOcean; break;
                case extremeHills:
                    hillID = extremeHillsPlus; break;
                case savanna:
                    hillID = savannaPlateau; break;
                default:
                    if (equalOrPlateau(a11, mesaPlateau_F))
                        hillID = mesa;
                    else if (a11 == deepOcean && mcNextInt(l, 3) == 0)
                        hillID = (mcNextInt(l, 2) == 0) ? plains : forest;
                    break;
                }

                if (var12 && hillID != a11)
                {
                    if (biomeExists(hillID + 128))
                        hillID += 128;
                    else
                        hillID = a11;
                }

                if (hillID == a11)
                {
                    out[idx] = a11;
                }
                else
                {
                    int a10 = buf[x+1 + (z+0)*pWidth];
                    int a21 = buf[x+2 + (z+1)*pWidth];
                    int a01 = buf[x+0 + (z+1)*pWidth];
                    int a12 = buf[x+1 + (z+2)*pWidth];
                    int equals = 0;

                    if (equalOrPlateau(a10, a11)) equals++;
                    if (equalOrPlateau(a21, a11)) equals++;
                    if (equalOrPlateau(a01, a11)) equals++;
                    if (equalOrPlateau(a12, a11)) equals++;

                    if (equals >= 3)
                        out[idx] = hillID;
                    else
                        out[idx] = a11;
                }
            }
        }
    }

    free(buf);
}


void mapHills113(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pX = areaX - 1;
    int pZ = areaZ - 1;
    int pWidth = areaWidth + 2;
    int pHeight = areaHeight + 2;
    int x, z;
    int *buf = NULL;

    if (l->p2 == NULL)
    {
        printf("mapHills() requires two parents! Use setupMultiLayer()\n");
        exit(1);
    }

    buf = (int *) malloc(pWidth*pHeight*sizeof(int));

    l->p->getMap(l->p, out, pX, pZ, pWidth, pHeight);
    memcpy(buf, out, pWidth*pHeight*sizeof(int));

    l->p2->getMap(l->p2, out, pX, pZ, pWidth, pHeight);

    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            setChunkSeed(l, (int64_t)(x + areaX), (int64_t)(z + areaZ));
            int a11 = buf[x+1 + (z+1)*pWidth]; // biome branch
            int b11 = out[x+1 + (z+1)*pWidth]; // river branch
            int idx = x + z*areaWidth;

            int bn = (b11 - 2) % 29;

            if (!(isOceanic(a11) || b11 < 2 || bn != 1 || a11 >= 128))
            {
                out[idx] = (biomeExists(a11 + 128)) ? a11 + 128 : a11;
            }
            else if (mcNextInt(l, 3) == 0 || bn == 0)
            {
                int hillID = a11;

                switch(a11){
                case desert:
                    hillID = desertHills; break;
                case forest:
                    hillID = forestHills; break;
                case birchForest:
                    hillID = birchForestHills; break;
                case roofedForest:
                    hillID = plains; break;
                case taiga:
                    hillID = taigaHills; break;
                case megaTaiga:
                    hillID = megaTaigaHills; break;
                case coldTaiga:
                    hillID = coldTaigaHills; break;
                case plains:
                    hillID = (mcNextInt(l, 3) == 0) ? forestHills : forest; break;
                case icePlains:
                    hillID = iceMountains; break;
                case jungle:
                    hillID = jungleHills; break;
                case ocean:
                    hillID = deepOcean; break;
                case extremeHills:
                    hillID = extremeHillsPlus; break;
                case savanna:
                    hillID = savannaPlateau; break;
                default:
                    if (equalOrPlateau(a11, mesaPlateau_F))
                        hillID = mesa;
                    else if ((a11 == deepOcean || a11 == lukewarmDeepOcean ||
                             a11 == coldDeepOcean || a11 == frozenDeepOcean) &&
                             mcNextInt(l, 3) == 0)
                        hillID = (mcNextInt(l, 2) == 0) ? plains : forest;
                    break;
                }

                if (bn == 0 && hillID != a11)
                {
                    if (biomeExists(hillID + 128))
                        hillID += 128;
                    else
                        hillID = a11;
                }

                if (hillID != a11)
                {
                    int a10 = buf[x+1 + (z+0)*pWidth];
                    int a21 = buf[x+2 + (z+1)*pWidth];
                    int a01 = buf[x+0 + (z+1)*pWidth];
                    int a12 = buf[x+1 + (z+2)*pWidth];
                    int equals = 0;

                    if (equalOrPlateau(a10, a11)) equals++;
                    if (equalOrPlateau(a21, a11)) equals++;
                    if (equalOrPlateau(a01, a11)) equals++;
                    if (equalOrPlateau(a12, a11)) equals++;

                    if (equals >= 3)
                        out[idx] = hillID;
                    else
                        out[idx] = a11;
                }
                else
                {
                    out[idx] = a11;
                }
            }
            else
            {
                out[idx] = a11;
            }
        }
    }

    free(buf);
}



static inline int reduceID(int id)
{
    return id >= 2 ? 2 + (id & 1) : id;
}

void mapRiver(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pX = areaX - 1;
    int pZ = areaZ - 1;
    int pWidth = areaWidth + 2;
    int pHeight = areaHeight + 2;
    int x, z;

    l->p->getMap(l->p, out, pX, pZ, pWidth, pHeight);

    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            int v01 = reduceID(out[x+0 + (z+1)*pWidth]);
            int v21 = reduceID(out[x+2 + (z+1)*pWidth]);
            int v10 = reduceID(out[x+1 + (z+0)*pWidth]);
            int v12 = reduceID(out[x+1 + (z+2)*pWidth]);
            int v11 = reduceID(out[x+1 + (z+1)*pWidth]);

            if (v11 == v01 && v11 == v10 && v11 == v21 && v11 == v12)
            {
                out[x + z * areaWidth] = -1;
            }
            else
            {
                out[x + z * areaWidth] = river;
            }
        }
    }
}


void mapSmooth(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pX = areaX - 1;
    int pZ = areaZ - 1;
    int pWidth = areaWidth + 2;
    int pHeight = areaHeight + 2;
    int x, z;

    l->p->getMap(l->p, out, pX, pZ, pWidth, pHeight);

    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            int v11 = out[x+1 + (z+1)*pWidth];
            int v10 = out[x+1 + (z+0)*pWidth];
            int v21 = out[x+2 + (z+1)*pWidth];
            int v01 = out[x+0 + (z+1)*pWidth];
            int v12 = out[x+1 + (z+2)*pWidth];

            if (v01 == v21 && v10 == v12)
            {
                setChunkSeed(l, (int64_t)(x + areaX), (int64_t)(z + areaZ));

                if (mcNextInt(l, 2) == 0)
                    v11 = v01;
                else
                    v11 = v10;
            }
            else
            {
                if (v01 == v21) v11 = v01;
                if (v10 == v12) v11 = v10;
            }

            out[x + z * areaWidth] = v11;
        }
    }
}


void mapRareBiome(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pX = areaX - 1;
    int pZ = areaZ - 1;
    int pWidth = areaWidth + 2;
    int pHeight = areaHeight + 2;
    int x, z;

    l->p->getMap(l->p, out, pX, pZ, pWidth, pHeight);

    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            setChunkSeed(l, (int64_t)(x + areaX), (int64_t)(z + areaZ));
            int v11 = out[x+1 + (z+1)*pWidth];

            if (mcNextInt(l, 57) == 0 && v11 == plains)
            {
                // Sunflower Plains
                out[x + z*areaWidth] = plains + 128;
            }
            else
            {
                out[x + z*areaWidth] = v11;
            }
        }
    }
}


inline static int replaceOcean(int *out, int idx, int v10, int v21, int v01, int v12, int id, int replaceID)
{
    if (isOceanic(id)) return 0;

    if (!isOceanic(v10) && !isOceanic(v21) && !isOceanic(v01) && !isOceanic(v12))
        out[idx] = id;
    else
        out[idx] = replaceID;

    return 1;
}

inline static int isBiomeJFTO(int id)
{
    return biomeExists(id) && (getBiomeType(id) == Jungle || id == forest || id == taiga || isOceanic(id));
}

void mapShore(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int pX = areaX - 1;
    int pZ = areaZ - 1;
    int pWidth = areaWidth + 2;
    int pHeight = areaHeight + 2;
    int x, z;

    l->p->getMap(l->p, out, pX, pZ, pWidth, pHeight);

    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            int v11 = out[x+1 + (z+1)*pWidth];
            int v10 = out[x+1 + (z+0)*pWidth];
            int v21 = out[x+2 + (z+1)*pWidth];
            int v01 = out[x+0 + (z+1)*pWidth];
            int v12 = out[x+1 + (z+2)*pWidth];

            int biome = biomeExists(v11) ? v11 : 0;

            if (v11 == mushroomIsland)
            {
                if (v10 != ocean && v21 != ocean && v01 != ocean && v12 != ocean)
                    out[x + z*areaWidth] = v11;
                else
                    out[x + z*areaWidth] = mushroomIslandShore;
            }
            else if (/*biome < 128 &&*/ getBiomeType(biome) == Jungle)
            {
                if (isBiomeJFTO(v10) && isBiomeJFTO(v21) && isBiomeJFTO(v01) && isBiomeJFTO(v12))
                {
                    if (!isOceanic(v10) && !isOceanic(v21) && !isOceanic(v01) && !isOceanic(v12))
                        out[x + z*areaWidth] = v11;
                    else
                        out[x + z*areaWidth] = beach;
                }
                else
                {
                    out[x + z*areaWidth] = jungleEdge;
                }
            }
            else if (v11 != extremeHills && v11 != extremeHillsPlus && v11 != extremeHillsEdge)
            {
                if (isBiomeSnowy(biome))
                {
                    replaceOcean(out, x + z*areaWidth, v10, v21, v01, v12, v11, coldBeach);
                }
                else if (v11 != mesa && v11 != mesaPlateau_F)
                {
                    if (v11 != ocean && v11 != deepOcean && v11 != river && v11 != swampland)
                    {
                        if (!isOceanic(v10) && !isOceanic(v21) && !isOceanic(v01) && !isOceanic(v12))
                            out[x + z*areaWidth] = v11;
                        else
                            out[x + z*areaWidth] = beach;
                    }
                    else
                    {
                        out[x + z*areaWidth] = v11;
                    }
                }
                else
                {
                    if (!isOceanic(v10) && !isOceanic(v21) && !isOceanic(v01) && !isOceanic(v12))
                    {
                        if (getBiomeType(v10) == Mesa && getBiomeType(v21) == Mesa && getBiomeType(v01) == Mesa && getBiomeType(v12) == Mesa)
                            out[x + z*areaWidth] = v11;
                        else
                            out[x + z*areaWidth] = desert;
                    }
                    else
                    {
                        out[x + z*areaWidth] = v11;
                    }
                }
            }
            else
            {
                replaceOcean(out, x + z*areaWidth, v10, v21, v01, v12, v11, stoneBeach);
            }
        }
    }
}


void mapRiverMix(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int idx;
    int len;
    int *buf;

    if (l->p2 == NULL)
    {
        printf("mapRiverMix() requires two parents! Use setupMultiLayer()\n");
        exit(1);
    }

    len = areaWidth*areaHeight;
    buf = (int *) malloc(len*sizeof(int));

    l->p->getMap(l->p, out, areaX, areaZ, areaWidth, areaHeight); // biome chain
    memcpy(buf, out, len*sizeof(int));

    l->p2->getMap(l->p2, out, areaX, areaZ, areaWidth, areaHeight); // rivers

    for (idx = 0; idx < len; idx++)
    {
        if (isOceanic(buf[idx]))
        {
            out[idx] = buf[idx];
        }
        else
        {
            if (out[idx] == river)
            {
                if (buf[idx] == icePlains)
                    out[idx] = frozenRiver;
                else if (buf[idx] == mushroomIsland || buf[idx] == mushroomIslandShore)
                    out[idx] = mushroomIslandShore;
                else
                    out[idx] = out[idx] & 255;
            }
            else
            {
                out[idx] = buf[idx];
            }
        }
    }

    free(buf);
}



/* Initialises data for the ocean temperature types using the world seed.
 * This function is called when the world seed is applied in setWorldSeed().
 */
static void oceanRndInit(OceanRnd *rnd, int64_t seed)
{
    int i = 0;
    memset(rnd, 0, sizeof(*rnd));
    setSeed(&seed);
    rnd->a = nextDouble(&seed) * 256.0;
    rnd->b = nextDouble(&seed) * 256.0;
    rnd->c = nextDouble(&seed) * 256.0;

    for (i = 0; i < 256; i++)
    {
        rnd->d[i] = i;
    }
    for (i = 0; i < 256; i++)
    {
        int n3 = nextInt(&seed, 256 - i) + i;
        int n4 = rnd->d[i];
        rnd->d[i] = rnd->d[n3];
        rnd->d[n3] = n4;
        rnd->d[i + 256] = rnd->d[i];
    }
}

static double lerp(const double part, const double from, const double to)
{
    return from + part * (to - from);
}

/* Table of vectors to cube edge centres (12 + 4 extra), used for ocean PRNG */
const double cEdgeX[] = {1.0,-1.0, 1.0,-1.0, 1.0,-1.0, 1.0,-1.0, 0.0, 0.0, 0.0, 0.0,  1.0, 0.0,-1.0, 0.0};
const double cEdgeY[] = {1.0, 1.0,-1.0,-1.0, 0.0, 0.0, 0.0, 0.0, 1.0,-1.0, 1.0,-1.0,  1.0,-1.0, 1.0,-1.0};
const double cEdgeZ[] = {0.0, 0.0, 0.0, 0.0, 1.0, 1.0,-1.0,-1.0, 1.0, 1.0,-1.0,-1.0,  0.0, 1.0, 0.0,-1.0};

static double indexedLerp(int idx, const double d1, const double d2, const double d3)
{
    idx &= 0xf;
    return cEdgeX[idx] * d1 + cEdgeY[idx] * d2 + cEdgeZ[idx] * d3;
}


static double getOceanTemp(const OceanRnd *rnd, double d1, double d2, double d3)
{
    d1 += rnd->a;
    d2 += rnd->b;
    d3 += rnd->c;
    int i1 = (int)d1 - (int)(d1 < 0);
    int i2 = (int)d2 - (int)(d2 < 0);
    int i3 = (int)d3 - (int)(d3 < 0);
    d1 -= i1;
    d2 -= i2;
    d3 -= i3;
    double t1 = d1*d1*d1 * (d1 * (d1*6.0-15.0) + 10.0);
    double t2 = d2*d2*d2 * (d2 * (d2*6.0-15.0) + 10.0);
    double t3 = d3*d3*d3 * (d3 * (d3*6.0-15.0) + 10.0);

    i1 &= 0xff;
    i2 &= 0xff;
    i3 &= 0xff;

    int a1 = rnd->d[i1]   + i2;
    int a2 = rnd->d[a1]   + i3;
    int a3 = rnd->d[a1+1] + i3;
    int b1 = rnd->d[i1+1] + i2;
    int b2 = rnd->d[b1]   + i3;
    int b3 = rnd->d[b1+1] + i3;

    double l1 = indexedLerp(rnd->d[a2],   d1,   d2,   d3);
    double l2 = indexedLerp(rnd->d[b2],   d1-1, d2,   d3);
    double l3 = indexedLerp(rnd->d[a3],   d1,   d2-1, d3);
    double l4 = indexedLerp(rnd->d[b3],   d1-1, d2-1, d3);
    double l5 = indexedLerp(rnd->d[a2+1], d1,   d2,   d3-1);
    double l6 = indexedLerp(rnd->d[b2+1], d1-1, d2,   d3-1);
    double l7 = indexedLerp(rnd->d[a3+1], d1,   d2-1, d3-1);
    double l8 = indexedLerp(rnd->d[b3+1], d1-1, d2-1, d3-1);

    l1 = lerp(t1, l1, l2);
    l3 = lerp(t1, l3, l4);
    l5 = lerp(t1, l5, l6);
    l7 = lerp(t1, l7, l8);

    l1 = lerp(t2, l1, l3);
    l5 = lerp(t2, l5, l7);

    return lerp(t3, l1, l5);
}

void mapOceanTemp(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int x, z;
    OceanRnd *rnd = l->oceanRnd;

    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            double tmp = getOceanTemp(rnd, (x + areaX) / 8.0, (z + areaZ) / 8.0, 0);

            if (tmp > 0.4)
                out[x + z*areaWidth] = warmOcean;
            else if (tmp > 0.2)
                out[x + z*areaWidth] = lukewarmOcean;
            else if (tmp < -0.4)
                out[x + z*areaWidth] = frozenOcean;
            else if (tmp < -0.2)
                out[x + z*areaWidth] = coldOcean;
            else
                out[x + z*areaWidth] = ocean;
        }
    }
}

/* Warning: this function is horribly slow compared to other layers! */
void mapOceanMix(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    int landX = areaX-8, landZ = areaZ-8;
    int landWidth = areaWidth+17, landHeight = areaHeight+17;
    int *map1, *map2;

    if (l->p2 == NULL)
    {
        printf("mapOceanMix() requires two parents! Use setupMultiLayer()\n");
        exit(1);
    }

    l->p->getMap(l->p, out, landX, landZ, landWidth, landHeight);
    map1 = (int *) malloc(landWidth*landHeight*sizeof(int));
    memcpy(map1, out, landWidth*landHeight*sizeof(int));

    l->p2->getMap(l->p2, out, areaX, areaZ, areaWidth, areaHeight);
    map2 = (int *) malloc(areaWidth*areaHeight*sizeof(int));
    memcpy(map2, out, areaWidth*areaHeight*sizeof(int));


    int x, z;

    for (z = 0; z < areaHeight; z++)
    {
        for (x = 0; x < areaWidth; x++)
        {
            int landID = map1[(x+8) + (z+8)*landWidth];
            int oceanID = map2[x + z*areaWidth];

            if (!isOceanic(landID))
            {
                out[x + z*areaWidth] = landID;
                continue;
            }

            for (int i = -8; i <= 8; i += 4)
            {
                for (int j = -8; j <= 8; j += 4)
                {
                    int nearbyID = map1[(x+i+8) + (z+j+8)*landWidth];

                    if (isOceanic(nearbyID)) continue;

                    if (oceanID == warmOcean)
                    {
                        out[x + z*areaWidth] = lukewarmOcean;
                        goto loop_x;
                    }

                    if (oceanID == frozenOcean)
                    {
                        out[x + z*areaWidth] = coldOcean;
                        goto loop_x;
                    }
                }
            }

            if (landID == deepOcean)
            {
                if (oceanID == lukewarmOcean)
                {
                    out[x + z*areaWidth] = lukewarmDeepOcean;
                    continue;
                }
                if (oceanID == ocean)
                {
                    out[x + z*areaWidth] = deepOcean;
                    continue;
                }
                if (oceanID == coldOcean)
                {
                    out[x + z*areaWidth] = coldDeepOcean;
                    continue;
                }
                if (oceanID == frozenOcean)
                {
                    out[x + z*areaWidth] = frozenDeepOcean;
                    continue;
                }
            }

            out[x + z*areaWidth] = oceanID;

            loop_x:;
        }
    }

    free(map1);
    free(map2);
}



void mapVoronoiZoom(Layer *l, int * __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    areaX -= 2;
    areaZ -= 2;
    int pX = areaX >> 2;
    int pZ = areaZ >> 2;
    int pWidth = (areaWidth >> 2) + 2;
    int pHeight = (areaHeight >> 2) + 2;
    int newWidth = (pWidth-1) << 2;
    int newHeight = (pHeight-1) << 2;
    int x, z, i, j;
    int *buf = (int *)malloc((newWidth+1)*(newHeight+1)*sizeof(*buf));

    l->p->getMap(l->p, out, pX, pZ, pWidth, pHeight);

    for (z = 0; z < pHeight - 1; z++)
    {
        int v00 = out[(z+0)*pWidth];
        int v01 = out[(z+1)*pWidth];

        for (x = 0; x < pWidth - 1; x++)
        {
            setChunkSeed(l, (x+pX) << 2, (z+pZ) << 2);
            double da1 = (mcNextInt(l, 1024) / 1024.0 - 0.5) * 3.6;
            double da2 = (mcNextInt(l, 1024) / 1024.0 - 0.5) * 3.6;

            setChunkSeed(l, (x+pX+1) << 2, (z+pZ) << 2);
            double db1 = (mcNextInt(l, 1024) / 1024.0 - 0.5) * 3.6 + 4.0;
            double db2 = (mcNextInt(l, 1024) / 1024.0 - 0.5) * 3.6;

            setChunkSeed(l, (x+pX) << 2, (z+pZ+1) << 2);
            double dc1 = (mcNextInt(l, 1024) / 1024.0 - 0.5) * 3.6;
            double dc2 = (mcNextInt(l, 1024) / 1024.0 - 0.5) * 3.6 + 4.0;

            setChunkSeed(l, (x+pX+1) << 2, (z+pZ+1) << 2);
            double dd1 = (mcNextInt(l, 1024) / 1024.0 - 0.5) * 3.6 + 4.0;
            double dd2 = (mcNextInt(l, 1024) / 1024.0 - 0.5) * 3.6 + 4.0;

            int v10 = out[x+1 + (z+0)*pWidth] & 255;
            int v11 = out[x+1 + (z+1)*pWidth] & 255;

            for (j = 0; j < 4; j++)
            {
                int idx = ((z << 2) + j) * newWidth + (x << 2);

                for (i = 0; i < 4; i++)
                {
                    double da = (j-da2)*(j-da2) + (i-da1)*(i-da1);
                    double db = (j-db2)*(j-db2) + (i-db1)*(i-db1);
                    double dc = (j-dc2)*(j-dc2) + (i-dc1)*(i-dc1);
                    double dd = (j-dd2)*(j-dd2) + (i-dd1)*(i-dd1);

                    if (da < db && da < dc && da < dd)
                    {
                        buf[idx++] = v00;
                    }
                    else if (db < da && db < dc && db < dd)
                    {
                        buf[idx++] = v10;
                    }
                    else if (dc < da && dc < db && dc < dd)
                    {
                        buf[idx++] = v01;
                    }
                    else
                    {
                        buf[idx++] = v11;
                    }
                }
            }

            v00 = v10;
            v01 = v11;
        }
    }

    for (z = 0; z < areaHeight; z++)
    {
        memcpy(&out[z * areaWidth], &buf[(z + (areaZ & 3))*newWidth + (areaX & 3)], areaWidth*sizeof(int));
    }

    free(buf);
}


