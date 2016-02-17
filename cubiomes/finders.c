/**
 * finders.c
 *
 * Author: Cubitect
 * Created on: 17 Feb 2016
 * Licence: GPLv3
 */

#include "finders.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/*************************
 * Seed finder functions *
 *************************/

/**
 * getQuadHutSeedList
 * ------------------
 * Finds a list of quad-witch-hut seeds.
 *
 * This function exploits the fact that the temple generation code only uses the lowest 48 bits of the seed,
 * while the biome generation uses the full 64 bits (more or less [1]). It first finds the next quad-temple
 * candidate after 'startBaseSeed' within the lower 48 bits using findQuadTempleCandiate(). It then loops
 * over the highest two bytes and stores every seeds that has a swampland biome at each temple position.
 *
 * [1] There is still some complex non-random dependence on the lower 6 bytes early on in the generation. This
 *     causes some base-seeds to never generate a swamp at the necessary locations. This function also includes
 *     some optimisation to detect and avoid such seeds. (The code for this can certainly be improved though.)
 *
 * seedList: list of the found 64-bit quad-witch-hut seeds, this should be able to store 65536 seeds
 * listLen: number of seeds that were found in the batch
 * posList: list of the block positions of the 4 witch huts indexed using the enum 'TemplePos'
 *          (this will be the same for all the seeds in 'seedList')
 * startBaseSeed: 48-bit seed that the search starts from
 * regionRadius: number of regions, from 0,0 for which the search is performed
 * quality: specifies the number of corner chunks that qualify for the search
 *          i.e. 0 = perfect position, 1 = one of the 4 chunks in each corner, 2 = one of 9 chunks etc.
 *
 * On success the return value is the base seed that was used to generate the seed lists.
 */
long getQuadHutSeedList(long *seedList, int *listLen, Pos *posList, const long startBaseSeed, const int regionRadius, const int quality)
{
    static Generator g256, g4;
    int *map256;
    long baseSeed = startBaseSeed;
    unsigned long i;
    int x, z, j, swampNum[4], swampArea;

    // testSeedNum: number of seeds to test before deciding if the seed is worth investigating further
    const int testSeedNum = 48;
    // minSwamp256: minimum number swamps found on MAG256 for which the search is continued
    const int minSwamp256 = quality;

    if(g4.layerMax == 0)
    {
        g256 = setupGenerator();
        g4 = setupGenerator();
        setGenScale(&g256, MAG256);
        setGenScale(&g4, MAG4);
    }

    map256 = allocCache(&g256, 2, 2);

    // find the next suitable base seed
    while(1)
    {
        baseSeed = findQuadTempleCandiate(posList, baseSeed, regionRadius, quality);
        x = posList[TOPLEFT].x / 256;
        z = posList[TOPLEFT].z / 256;

        // It turns out that even though the biomes use the entire 64-bits of the seed
        // there is still some dependence in the lower 48-bits where biomes can generate.
        // However, this dependence seems complex, and so we just try out a few seed to see
        // if this base seed is suitable.
        memset(swampNum, 0, sizeof(swampNum));
        for(i = 0; i < 0xffff; i += 0xffff/testSeedNum)
        {
            applySeed(&g256, baseSeed+(long)(i << 48));
            genArea(&g256, map256, x, z, 2, 2);
            for(j = 0; j < 4; j++) if(map256[j] == swampland) swampNum[j]++;
        }

        swampArea = 0;
        for(j = 0; j < 4; j++) if(swampNum[j]) {
            swampArea++;
        }
        if(swampArea >= minSwamp256) break;

        contSearch:
        baseSeed++;
    }

    for(i = 0; i < 4; i++)
    {
        posList[i].x += 8;
        posList[i].z += 8;
    }

    *listLen = 0;
    for(i = 0; i < 0xffff; i++)
    {
        applySeed(&g4, baseSeed+(long)(i<<48));

        if(arePointsInBiome(&g4, posList, 4, swampland))
        {
            seedList[*listLen] = baseSeed+(long)(i<<48);
            (*listLen)++;
        }

        // Yes, I used goto. Get over it.
        // If the seed density is less than about 1 in 64 then we're better off moving on to the next seed.
        if(*listLen < 0x01 && i > 0x40) goto contSearch;
        if(*listLen < 0x12 && i > 0x400) goto contSearch;
        if(*listLen < 0x42 && i > 0x1000) goto contSearch;
    }

    for(i = 0; i < 4; i++)
    {
        posList[i].x -= 8;
        posList[i].z -= 8;
    }

    return baseSeed;
}


/**
 * findQuadTempleCandiate
 * ----------------------
 * Searches for the next seed, where four temples would generate in adjacent corners of touching regions.
 * Biomes are not checked here!
 *
 * outList: block positions of the 4 temples, indexed using the enum 'TemplePos'
 * startSeed: starting base seed, this should be a 48-bit number
 * regionRadius: number of regions, from 0,0 for which the search is performed
 * quality: specifies the number of corner chunks that qualify for the search
 *          i.e. 0 = perfect position, 1 = one of the 4 chunks in each corner, 2 = one of 9 chunks etc.
 *
 * On success the return value is the next base seed that can generate 4 adjacent temples.
 */
long findQuadTempleCandiate(Pos *outList, const long startSeed, const int regionRadius, const int quality)
{
    const int end = regionRadius-1;
    const int start = -regionRadius;
    const int lower = quality, upper = 23-quality;
    int z, x;
    register long seed, seedXpar, seedZpar;

    for(seed = startSeed;; seed++)
    {
        for(z = start, seedZpar = start*132897987541; z < end; z+=2, seedZpar += 2*132897987541)
        {
            for(x = start, seedXpar = start*341873128712; x < end; x+=2, seedXpar += 2*341873128712)
            {
                long seedHut = seedXpar + seedZpar + seed + 14357617;
                // get the x-coordinate of the chunk position for the temple within the region
                // (note that this can never be within the highest 8 chunks of the region)
                int posX = firstInt24(seedHut);

                // right
                if(posX >= upper)
                {
                    int posZ = secondInt24(seedHut);

                    if(posZ >= upper)
                    {
                        // bottom-right
                        if( checkTL(outList, x+1, z+1, seed, lower, upper) &&
                            checkTR(outList, x  , z+1, seed, lower, upper) &&
                            checkBL(outList, x+1, z  , seed, lower, upper))
                        {
                            outList[TOPLEFT].x = (x*32 + posX)*16;
                            outList[TOPLEFT].z = (z*32 + posZ)*16;
                            return seed;
                        }
                    }
                    else if(posZ <= lower && z > start)
                    {
                        // top-right
                        if( checkTL(outList, x+1, z  , seed, lower, upper) &&
                            checkBL(outList, x+1, z-1, seed, lower, upper) &&
                            checkBR(outList, x  , z-1, seed, lower, upper))
                        {
                            outList[BOTTOMLEFT].x = (x*32 + posX)*16;
                            outList[BOTTOMLEFT].z = (z*32 + posZ)*16;
                            return seed;
                        }
                    }
                }
                // left
                else if(posX <= lower && x > start)
                {
                    int posZ = secondInt24(seedHut);

                    if(posZ >= upper)
                    {
                        // bottom-left
                        if( checkTL(outList, x  , z+1, seed, lower, upper) &&
                            checkTR(outList, x-1, z+1, seed, lower, upper) &&
                            checkBR(outList, x-1, z  , seed, lower, upper))
                        {
                            outList[TOPRIGHT].x = (x*32 + posX)*16;
                            outList[TOPRIGHT].z = (z*32 + posZ)*16;
                            return seed;
                        }
                    }
                    else if(posZ <= lower && z > start)
                    {
                        // top-left
                        if( checkTR(outList, x-1, z  , seed, lower, upper) &&
                            checkBR(outList, x-1, z-1, seed, lower, upper) &&
                            checkBL(outList, x  , z-1, seed, lower, upper))
                        {
                            outList[BOTTOMRIGHT].x = (x*32 + posX)*16;
                            outList[BOTTOMRIGHT].z = (z*32 + posZ)*16;
                            return seed;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

/**
 * findHybridTemple
 * ----------------
 * Finds the next seed with a desert temple that generates partially inside a swamp biome.
 *
 * out: block position of the hybrid temple
 * spawnArea: area (in block XZ-coordinates) within the found temple that are inside a swamp.
 * startSeed: starting seed for the search
 * regionRadius: number of regions, from 0,0 for which the search is performed
 * minSpawnArea: minimum area in block coordinates that are in a swamp
 *               (A desert temple has a 21*21 = 441 block area total.)
 *
 * On success the return value is the new found seed, 0 otherwise.
 *
 * Unfortunately this function appears to be redundant, as normal spawning rules seem to apply in these hybrid temples.
 */
long findHybridTemple(Pos *out, int *spawnArea, const long startSeed, const int regionRadius, const int minSpawnArea)
{
    static Generator g;
    int *map256, *map16, *map4, *map1;
    const int end = regionRadius-1;
    const int start = -regionRadius;
    const int difseg = (end-start)*2;
    long seed;
    int x, z, rz, rx, posx, posz;
    const int width4 = 5;
    const int minswamp4 = minSpawnArea/16;
    int i, swampnum;

    if(g.layerMax == 0)
    {
        g = setupGenerator();
    }

    setGenScale(&g, MAG256);
    map256 = allocCache(&g, difseg, difseg);
    map16 = allocCache(&g, 3, 3);
    map4 = allocCache(&g, width4, width4);
    map1 = allocCache(&g, 21, 21);

    for(seed = startSeed; ; seed++)
    {
        setGenScale(&g, MAG256);
        applySeed(&g, seed);
        genArea(&g, map256, start, start, difseg, difseg);

        for(z = 0; z < difseg; z+=2)
        {
            for(x = 0; x < difseg; x+=2)
            {
                // get the four 256x256 areas in this region
                int v00 = map256[x+0 + (z+0)*difseg];
                int v01 = map256[x+0 + (z+1)*difseg];
                int v10 = map256[x+1 + (z+0)*difseg];
                int v11 = map256[x+1 + (z+1)*difseg];

                // do we have a swamp?
                if((v00 == swampland || v01 == swampland || v10 == swampland || v11 == swampland))
                {
                    rx = start + x/2;
                    rz = start + z/2;
                    getTemplePosInRegion(&posx, &posz, rx, rz, seed);

                    // chunk position
                    posx = (rx*32 + posx);
                    posz = (rz*32 + posz);

                    setGenScale(&g, MAG16);
                    applySeed(&g, seed);
                    genArea(&g, map16, posx, posz, 3, 3);

                    int hasSwamp = 0, hasDesert = 0;
                    for(i = 0; i < 9; i++)
                    {
                        if(map16[i] == swampland) hasSwamp++;
                        if(map16[i] == desert || map16[i] == desertHills) hasDesert++;
                    }
                    if(hasSwamp < 3 || !hasDesert) continue;


                    posx *= 4; posz *= 4;

                    setGenScale(&g, MAG4);
                    applySeed(&g, seed);
                    genArea(&g, map4, posx+2, posz+2, 1, 1);

                    // do we have a good chance the temple does not start in a swamp, but is surrounded by it
                    if(map4[0] != desert && map4[0] != desertHills) continue;


                    genArea(&g, map4, posx, posz, width4, width4);

                    swampnum = 0;
                    for(i = 0; i < width4*width4; i++)
                    {
                        if(map4[i] == swampland) swampnum++;
                    }

                    if(swampnum >= minswamp4)
                    {
                        posx *= 4; posz *= 4;
                        setGenScale(&g, MAG1);
                        genArea(&g, map1, posx, posz, 21, 21);
                        *spawnArea = 0;
                        for(i = 0; i < 21*21; i++)
                            if(map1[i] == swampland) (*spawnArea)++;
                        out->x = posx;
                        out->z = posz;
                        free(map256);
                        free(map16);
                        free(map4);
                        free(map1);
                        return seed;
                    }
                }
            }
        }
    }

    free(map256);
    free(map16);
    free(map4);
    free(map1);
    return 0;
}

/**
 * findAreaWithAllBiomes
 * ---------------------
 * Finds the next seed for which all the biomes (ignoring mutations) are present in the specified area.
 *
 * The search starts at 'startSeed' and ends when either an appropriate seed has been found or when 'endSeed' has been reached.
 * 'areaX' & 'areaZ' specify the block coordinated of the area, and 'width' & 'height' specify its dimensions.
 *
 * On success the return value is the new found seed. The return value is 0 when 'endSeed' was reached without finding a seed.
 */
long findAreaWithAllBiomes(const long startSeed, const long endSeed, const int areaX, const int areaZ, const int width, const int height)
{
    static Generator g1024, gsh, g256, g4;
    int *map1024, *mapsh, *map256, *map4;
    const int w1024 = width/1024+2, h1024 = height/1024+2;
    const int w256 = width/256+2, h256 = height/256+2;
    const int w4 = width/4+2, h4 = height/4+2;
    const int areaX1024 = areaX/1024-1, areaZ1024 = areaZ/1024-1;
    const int areaX256 = areaX/256-1, areaZ256 = areaZ/256-1;
    const int areaX4 = areaX/4-1, areaZ4 = areaZ/4-1;

    long seed;
    int i;
    int x, z, hasSpecial;

    if(g1024.layerMax == 0)
    {
        g1024 = setupGenerator();
        gsh = setupGenerator();
        g256 = setupGenerator();
        g4 = setupGenerator();
        setGenScale(&g1024, MAG1024);
        setGenScale(&gsh, MAGSHROOM);
        setGenScale(&g256, MAG256);
        setGenScale(&g4, MAG4);
    }

    map1024 = allocCache(&g1024, w1024, h1024);
    mapsh = allocCache(&gsh, w256, h256);
    map256 = allocCache(&g256, w256, h256);
    map4 = allocCache(&g4, w4, h4);

    volatile int biomeCnt;
    int typeList[BTYPE_NUM];
    int biomeList[BIOME_NUM];

    for(seed = startSeed; seed <= endSeed; seed++)
    {
        applySeed(&g1024, seed);
        listBiomeTypes(&g1024, map1024, seed, typeList, areaX1024, areaZ1024, w1024, h1024);
        if(!typeList[Forest] || !typeList[Ocean] || !typeList[Plains] || !typeList[Desert] || !typeList[Hills]) continue;

        // Apply the 'Special' layer to see if there is a chance for a Mega Taiga biome (6% faster)
        hasSpecial = 0;
        for(z = 0; z < h1024 && !hasSpecial; z++)
        {
            for(x = 0; x < w1024; x++)
            {
                int v = map1024[x + z*w1024];
                if(v == 0) continue;

                setChunkSeed(&g1024.layers[12], (long)(x + areaX1024), (long)(z + areaZ1024));

                if(mcNextInt(&g1024.layers[12], 13) == 0) {
                    hasSpecial++;
                    break;
                }
            }
        }
        if(!hasSpecial) continue;

        // Check for a mushroom island early on, as this excludes a great proportion of seeds.
        applySeed(&gsh, seed);
        listBiomeTypes(&gsh, mapsh, seed, typeList, areaX256, areaZ256, w256, h256);
        if(!typeList[MushroomIsland]) {
            continue;
        }

        // Check the necessary biomes types are present, which ignores mutations, variation and rivers.
        applySeed(&g256, seed);
        listBiomeTypes(&g256, map256, seed, typeList, areaX256, areaZ256, w256, h256);

        // List of all types: Ocean, Plains, Desert, Hills, Forest, Taiga, Swamp, River, Hell, Sky, Snow, MushroomIsland, Beach, Jungle, StoneBeach, Savanna, Mesa
        if(!typeList[Ocean] || !typeList[Plains] || !typeList[Desert] || !typeList[Hills]) continue;
        if(!typeList[Forest] || !typeList[Taiga] || !typeList[Swamp] || !typeList[Snow]) continue;
        if(!typeList[MushroomIsland] || !typeList[Jungle] || !typeList[Savanna] || !typeList[Mesa]) continue;

        // Finally count the number of biomes of the fully generated map.
        // There are 36 biomes that could generate in total.
        applySeed(&g4, seed);
        listBiomes(&g4, map4, seed, biomeList, areaX4, areaZ4, w4, h4);

        biomeCnt = 0;
        for(i = 0; i < BIOME_NUM; i++){
            if(biomeList[i]) biomeCnt++;
        }

        // give an update whenever a seed got this far through the sieves.
        // printf("update:%ld:%d\n", seed, biomeCnt);

        if(biomeCnt >= 36){
            free(map1024);
            free(mapsh);
            free(map256);
            free(map4);
            return seed;
        }
    }

    return 0;
}


/********************
 * Helper functions *
 ********************/


/* findBiomePosition
 * -----------------
 * Finds a suitable pseudo-random location in the specified area.
 * Used to determine the positions of spawn and stongholds.
 * Warning: accurate, but slow!
 */
void findBiomePosition(Generator *g, Pos *out, int centerX, int centerZ, int range, const int *biomeList, long *seed)
{
    int x1 = (centerX-range) >> 2;
    int z1 = (centerZ-range) >> 2;
    int x2 = (centerX+range) >> 2;
    int z2 = (centerZ+range) >> 2;
    int width  = x2 - x1 + 1;
    int height = z2 - z1 + 1;
    int    *map = allocCache(g, width, height);

    genArea(g, map, x1, z1, width, height);
    int i, found = 0;

    for(i = 0; i < width*height; i++)
    {
        int biome = map[i];

        if(biomeList[biome & 0xff] && (found == 0 || nextInt(seed, found + 1) == 0))
        {
            out->x = (x1 + i%width) << 2;
            out->z = (z1 + i/width) << 2;
            ++found;
        }
    }

    free(map);
}

/* areBiomesViable
 * ---------------
 * Determines if the given area contains only biomes specified by 'biomeList'.
 * Used to determine the positions of ocean monuments and villages.
 * Warning: accurate, but slow!
 */
int areBiomesViable(Generator *g, int posX, int posZ, int radius, const int *biomeList, const int listLen)
{
    int x1 = (posX - radius) >> 2;
    int z1 = (posZ - radius) >> 2;
    int x2 = (posX + radius) >> 2;
    int z2 = (posZ + radius) >> 2;
    int width = x2 - x1 + 1;
    int height = z2 - z1 + 1;
    int i, j;

    int    *map = allocCache(g, width, height);
    genArea(g, map, x1, z1, width, height);

    for(i = 0; i < width*height; i++)
    {
        for(j = 0; j < listLen; j++)
        {
            if(map[i] == biomeList[j]) break;
        }
        if(j >= listLen)
        {
            free(map);
            return 0;
        }
    }

    free(map);
    return 1;
}

// Breaks in 1.9
void findStrongholds(Generator *g, Pos *locations)
{
    static int validStrongholdBiomes[256];
    const int SHNUM = 3;
    long worldSeed = g->rawSeed;
    int i;

    if(!validStrongholdBiomes[plains])
    {
        int id;
        for(id = 0; id < 256; id++)
        {
            if(biomeExists(id) && biomes[id].height > 0.0) validStrongholdBiomes[id] = 1;
        }
    }

    if(g->mag != MAG4)
    {
        printf("findStrongholds() requires a 1 to 4 generator resolution.\n");
        for(i = 0; i < SHNUM; i++)
        {
            locations[i].x = locations[i].z = 0;
        }
        return;
    }

    setSeed(&worldSeed);
    double angle = nextDouble(&worldSeed) * 3.141592653589793 * 2.0;

    for(i = 0; i < SHNUM; i++)
    {
        double distance = (1.25 + nextDouble(&worldSeed)) * 32.0;
        int x = (int)round(cos(angle) * distance);
        int z = (int)round(sin(angle) * distance);

        findBiomePosition(g, &locations[i], (x << 4) + 8, (z << 4) + 8, 112, validStrongholdBiomes, &worldSeed);

        angle += 6.283185307179586 / (double)SHNUM;
    }
}

/* getTemplePosInRegion
 * --------------------
 * finds the chunk position within the specified region (32x32 chunks) where the temple generation attempt will occur.
 */
void getTemplePosInRegion(int *outX, int *outZ, int regionX, int regionZ, long seed)
{
    seed = regionX*341873128712 + regionZ*132897987541 + seed + 14357617;
    setSeed(&(seed));
    *outX = nextInt(&seed, 24);
    *outZ = nextInt(&seed, 24);
}

/* getOceanMonumentPosInRegion
 * --------------------
 * finds the chunk position within the specified region (32x32 chunks) where the ocean monument generation attempt will occur.
 */
void getOceanMonumentPosInRegion(int *outX, int *outZ, int regionX, int regionZ, long seed)
{
    seed = regionX*341873128712 + regionZ*132897987541 + seed + 10387313;
    setSeed(&(seed));

    *outX = (nextInt(&seed, 27) + nextInt(&seed, 27)) / 2;
    *outZ = (nextInt(&seed, 27) + nextInt(&seed, 27)) / 2;
}

/* isViableOceanMonumentPos
 * --------------------
 * checks if the given block coordinates are valid for an Ocean Monument
 * (This seems to have changed in MC 1.9)
 */
int isViableOceanMonumentPos(Generator *g, int blockX, int blockZ)
{
    int ret, mag = g->mag;
    setGenScale(g, MAG4);
    int    map[0x1000];
    genArea(g, map, blockX>>1, blockZ>>1, 1, 1);
    if(map[0] != deepOcean)
    {
        setGenScale(g, mag);
        return 0;
    }
    ret = areBiomesViable(g, blockX, blockZ, 29, oceanMonumentBiomeList, sizeof(oceanMonumentBiomeList)/sizeof(int));
    setGenScale(g, mag);
    return ret;
}



int checkBR(Pos *posList, const int x, const int z, long seed, const int lower, const int upper)
{
    long s = x*341873128712 + z*132897987541 + seed + 14357617;
    int Xpos = firstInt24(s);
    if(Xpos >= upper)
    {
        int Zpos = secondInt24(s);
        if(Zpos >= upper)
        {
            posList[TOPLEFT].x = (x*32 + Xpos)*16;
            posList[TOPLEFT].z = (z*32 + Zpos)*16;
            return 1;
        }
    }
    return 0;
}

int checkBL(Pos *posList, const int x, const int z, long seed, const int lower, const int upper)
{
    long s = x*341873128712 + z*132897987541 + seed + 14357617;
    int Xpos = firstInt24(s);
    if(Xpos <= lower)
    {
        int Zpos = secondInt24(s);
        if(Zpos >= upper)
        {
            posList[TOPRIGHT].x = (x*32 + Xpos)*16;
            posList[TOPRIGHT].z = (z*32 + Zpos)*16;
            return 1;
        }
    }
    return 0;
}

int checkTR(Pos *posList, const int x, const int z, long seed, const int lower, const int upper)
{
    long s = x*341873128712 + z*132897987541 + seed + 14357617;
    int Xpos = firstInt24(s);
    if(Xpos >= upper)
    {
        int Zpos = secondInt24(s);
        if(Zpos <= lower)
        {
            posList[BOTTOMLEFT].x = (x*32 + Xpos)*16;
            posList[BOTTOMLEFT].z = (z*32 + Zpos)*16;
            return 1;
        }
    }
    return 0;
}

int checkTL(Pos *posList, const int x, const int z, long seed, const int lower, const int upper)
{
    long s = x*341873128712 + z*132897987541 + seed + 14357617;
    int Xpos = firstInt24(s);
    if(Xpos <= lower)
    {
        int Zpos = secondInt24(s);
        if(Zpos <= lower)
        {
            posList[BOTTOMRIGHT].x = (x*32 + Xpos)*16;
            posList[BOTTOMRIGHT].z = (z*32 + Zpos)*16;
            return 1;
        }
    }
    return 0;
}


/* arePointsInBiome
 * ----------------
 * returns non-zero if all the points appear to be in the specified biome.
 */
int arePointsInBiome(Generator *g, Pos *pos, const int num, const int biome)
{
    static int ints[0x1000];

    int i;
    for(i = 0; i < 4; i++)
    {
        genArea(g, &ints[0], pos[i].x/4, pos[i].z/4, 1, 1);
        if(ints[0] != biome) {
            return 0;
        }
    }
    return 1;
}

void listBiomes(Generator *g, int *map, long seed, int *biomeList, const int x, const int z, const int width, const int height)
{
    const int size = width*height;

    int i;
    memset(biomeList, 0, sizeof(int)*BIOME_NUM);

    genArea(g, map, x, z, width, height);

    for(i = 0; i < size; i++){
        biomeList[map[i] & 0x7f] = 1;
    }
}

void listBiomeTypes(Generator *g, int *map, long seed, int *typeList, const int x, const int z, const int width, const int height)
{
    const int size = width*height;
    int i;
    memset(typeList, 0, sizeof(int)*BTYPE_NUM);

    genArea(g, map, x, z, width, height);

    for(i = 0; i < size; i++){
        typeList[getBiomeType(map[i])] = 1;
    }
}


/* C copy of the Java Random methods */

inline void setSeed(long *seed)
{
    *seed = (*seed ^ 0x5DEECE66DL) & ((1L << 48) - 1);
}

inline int next(long *seed, const int bits)
{
    *seed = (*seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
    return (int) (*seed >> (48 - bits));
}

inline int nextInt(long *seed, const int n)
{
    int bits, val;
    do {
        bits = next(seed, 31);
        val = bits % n;
    }
    while(bits - val + (n - 1) < 0);
    return val;
}

inline long nextLong(long *seed)
{
    return ((long) next(seed, 32) << 32) + next(seed, 32);
}

inline float nextFloat(long *seed)
{
    return next(seed, 24) / (float) (1 << 24);
}

inline double nextDouble(long *seed)
{
    return (((long) next(seed, 26) << 27) + next(seed, 27)) / (double) (1L << 53);
}


// Stripped variants for the first and second call to nextInt(24).

inline int firstInt24(long seed)
{
    seed ^= 0x5DEECE66D;
    seed = (seed * 0x5DEECE66D) & 0xffffffffffff;
    seed >>= 17;
    return seed % 24;
}

inline int secondInt24(long seed)
{
    seed ^= 0x5DEECE66D;
    seed = (seed * 0x5DEECE66D + 0xB) & 0xffffffffffff;
    seed = (seed * 0x5DEECE66D) & 0xffffffffffff;
    seed >>= 17;
    return seed % 24;
}


