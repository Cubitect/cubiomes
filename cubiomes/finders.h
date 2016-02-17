/**
 * finders.h
 *
 * Author: Cubitect
 * Created on: 17 Feb 2016
 * Licence: GPLv3
 */

#ifndef FINDERS_H_
#define FINDERS_H_

#include "generator.h"

enum TemplePos { TOPLEFT, TOPRIGHT, BOTTOMLEFT, BOTTOMRIGHT };

static const int oceanMonumentBiomeList[] = {ocean, deepOcean, river, frozenOcean, frozenRiver};

STRUCT(Pos)
{
    int x, z;
};

/****************
 * Seed finders *
 ****************/

/**
 * getQuadHutSeedList
 * ------------------
 * Finds a list of quad-witch-hut seeds.
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
long getQuadHutSeedList(long *seedList, int *listLen, Pos *posList, const long startBaseSeed, const int regionRadius, const int quality);

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
long findQuadTempleCandiate(Pos *outList, const long startSeed, const int regionRadius, const int quality);

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
long findHybridTemple(Pos *out, int *spawnArea, const long startSeed, const int regionRadius, const int minSpawnArea);

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
long findAreaWithAllBiomes(const long startSeed, const long endSeed, const int areaX, const int areaZ, const int width, const int height);


/*******************
 * Feature finders *
 *******************/

void findStrongholds(Generator *g, Pos *locations);

/********************
 * Helper functions *
 ********************/

// Implementations of vanilla mc biome checkers
void findBiomePosition(Generator *g, Pos *out, int centerX, int centerZ, int range, const int *biomeList, long *seed);
int areBiomesViable(Generator *g, int posX, int posZ, int radius, const int *biomeList, const int listLen);

// Directional witch hut position checkers
int checkBR(Pos *posList, const int x, const int z, long seed, const int lower, const int upper);
int checkBL(Pos *posList, const int x, const int z, long seed, const int lower, const int upper);
int checkTR(Pos *posList, const int x, const int z, long seed, const int lower, const int upper);
int checkTL(Pos *posList, const int x, const int z, long seed, const int lower, const int upper);

// List the biomes/types that are in the specified area (using the current generator settings)
// The output is a boolean (int) array.
void listBiomes(Generator *g, int *map, long seed, int *biomeList, const int x, const int z, const int width, const int height);
void listBiomeTypes(Generator *g, int *map, long seed, int *typeList, const int x, const int z, const int width, const int height);

// Various
void getTemplePosInRegion(int *outX, int *outZ, int regionX, int regionZ, long seed);
void getOceanMonumentPosInRegion(int *outX, int *outZ, int regionX, int regionZ, long seed);
int isViableOceanMonumentPos(Generator *g, int blockX, int blockZ);

int arePointsInBiome(Generator *g, Pos *pos, int num, int biome);


/*************************************
 * C copy of the Java Random methods *
 *************************************/

inline void setSeed(long *seed);
inline int next(long *seed, const int bits);
inline int nextInt(long *seed, const int n);
inline long nextLong(long *seed);
inline float nextFloat(long *seed);
inline double nextDouble(long *seed);

// Custom, faster alternative for the first and second call to nextInt(24)
inline int firstInt24(long seed);
inline int secondInt24(long seed);

#endif /* FINDERS_H_ */
