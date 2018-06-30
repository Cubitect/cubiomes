#ifndef FINDERS_H_
#define FINDERS_H_

#include "generator.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>


#define THREADS 6
#define SEEDMAX (1L << 48)

enum {SWAMP_HUT = 1, IGLOO, DESERT_TEMPLE, JUNGLE_TEMPLE};

static const int templeBiomeList[] = {desert, desertHills, jungle, jungleHills, swampland, icePlains, coldTaiga};
static const int biomesToSpawnIn[] = {forest, plains, taiga, taigaHills, forestHills, jungle, jungleHills};
static const int oceanMonumentBiomeList[] = {ocean, deepOcean, river, frozenOcean, frozenRiver};
static const int villageBiomeList[] = {plains, desert, savanna, taiga};
static const int mansionBiomeList[] = {roofedForest, roofedForest+128};

static const int achievementBiomes[] =
{
        ocean, plains, desert, extremeHills, forest, taiga, swampland, river, /*hell, sky,*/ // 0-9
        /*frozenOcean,*/ frozenRiver, icePlains, iceMountains, mushroomIsland, mushroomIslandShore, beach, desertHills, forestHills, taigaHills,  // 10-19
        /*extremeHillsEdge,*/ jungle, jungleHills, jungleEdge, deepOcean, stoneBeach, coldBeach, birchForest, birchForestHills, roofedForest, // 20-29
        coldTaiga, coldTaigaHills, megaTaiga, megaTaigaHills, extremeHillsPlus, savanna, savannaPlateau, mesa, mesaPlateau_F, mesaPlateau // 30-39
};




STRUCT(Pos)
{
    int x, z;
};


extern Biome biomes[256];



/******************************** SEED FINDING *********************************
 *
 *  If we want to find rare seeds that meet multiple custom criteria then we
 *  should test each condition, starting with the one that is the cheapest
 *  to test for, while ruling out the most seeds.
 *
 *  Biome checks are quite expensive and should be applied late in the
 *  condition chain (to avoid as many unnecessary checks as possible).
 *  Fortunately we can often rule out vast amounts of seeds before hand.
 */



/*************************** Quad-Structure Checks *****************************
 *
 *  Several tricks can be applied to determine candidate seeds for quad
 *  temples (inc. witch huts).
 *
 *  Minecraft uses a 48-bit pseudo random number generator (PRNG) to determine
 *  the position of it's structures. The remaining top 16 bits do not influence
 *  the structure positioning. Additionally the position of all temples in a
 *  world can be translated by applying the following transformation to the
 *  seed:
 *
 *  seed2 = seed1 - structureSeed - dregX * 341873128712 - dregZ * 132897987541;
 *
 *  Here seed1 and seed2 have the same structure positioning, but moved by a
 *  region offset of (dregX,dregZ). [a region is 32x32 chunks]. The value of
 *  structureSeed depends on the type of structure, 14357617 for desert temples,
 *  14357618 for igloos, 14357619 for jungle temples and 14357620 for witch
 *  huts.
 *
 *  For a quad-structure, we mainly care about relative positioning, so we can
 *  get away with just checking the regions near the origin: (0,0),(0,1),(1,0)
 *  and (1,1) and then move the structures to the desired position.
 *
 *  Lastly we can recognise a that the transformation of relative region-
 *  coordinates imposes some restrictions in the PRNG, such that
 *  perfect-position quad-structure-seeds can only have certain values for the
 *  lower 16-bits in their seeds.
 *
 *
 ** The Set of all Quad-Witch-Huts
 *
 *  These conditions only leave 32 free bits which can comfortably be brute-
 *  forced to get the entire set of quad-structure candidates. Each of the seeds
 *  found this way describes an entire set of possible quad-witch-huts (with
 *  degrees of freedom for region-transposition, and the top 16-bit bits).
 */

// helper functions
int isQuadWitchHutBase(const long seed, const long lower, const long upper);
int isTriWitchHutBase(const long seed, const long lower, const long upper);

/* moveTemple
 * ----------
 * Transposes a base seed such that structures are moved by the specified region
 * vector, (regX, regZ).
 */
long moveTemple(const long baseSeed, const int regX, const int regZ);

/* loadSavedSeeds
 * --------------
 * Loads a list of seeds from a file. The seeds should be written as decimal
 * UFT-8 numbers separated by newlines.
 *
 * fnam: file path
 * scnt: number of valid seeds found in the file = length of the returned buffer
 *
 * Return a pointer to dynamically allocated seed list.
 */
long *loadSavedSeeds(const char *fnam, long *scnt);

/* baseQuadWitchHutSearch
 * --------------------
 * Starts a multi-threaded search for quad-temple base seeds of the specified
 * quality (chunk tolerance). The result is saved in a file of path 'fnam'.
 */
void baseQuadWitchHutSearch(const char *fnam, int threads, int quality);



/**************************** General Biome Checks *****************************
 */

/* getBiomeAtPos
 * ----------------
 * Returns the biome for the specified block position.
 * (Alternatives should be considered in performance critical code.)
 */
int getBiomeAtPos(const LayerStack g, const Pos pos);


/* getWitchHutChunkInRegion
 * ----------------------
 * Finds the chunk position within the specified region (32x32 chunks) where
 * the temple generation attempt will occur.
 */
Pos getWitchHutChunkInRegion(long seed, const int regionX, const int regionZ);


/* getWitchHutPos
 * ------------
 * Fast implementation for finding the block position at which the temple
 * generation attempt will occur in the specified region.
 */
Pos getWitchHutPos(long seed, const long regionX, const long regionZ);

/* getVillagePos
 * -------------
 * Fast implementation for finding the block position at which the village
 * generation attempt will occur in the specified region.
 */
Pos getVillagePos(long seed, const long regionX, const long regionZ);

/* getOceanMonumentPos
 * -------------------
 * Fast implementation for finding the block position at which the ocean
 * monument generation attempt will occur in the specified region.
 */
Pos getOceanMonumentPos(long seed, const long regionX, const long regionZ);

/* getMansionPos
 * -------------
 * Fast implementation for finding the block position at which the woodland
 * mansions generation attempt will occur in the specified 80x80 chunk area.
 *
 * area80X, area80Z: area coordinates in units 1280 blocks (= 80 chunks)
 */
Pos getMansionPos(long seed, const long area80X, const long area80Z);


/* findBiomePosition
 * -----------------
 * Finds a suitable pseudo-random location in the specified area.
 * Used to determine the positions of spawn and stongholds.
 * Warning: accurate, but slow!
 *
 * g                : generator layer stack
 * cache            : biome buffer, set to NULL for temporary allocation
 * centreX, centreZ : origin for the search
 * range            : 'radius' of the search
 * isValid          : boolean array of valid biome ids (size = 256)
 * seed             : seed used for the RNG
 *                    (initialise RNG using setSeed(&seed))
 * passes           : number of valid biomes passed, set to NULL to ignore this
 */
Pos findBiomePosition(
        const LayerStack g,
        int *cache,
        const int centerX,
        const int centerZ,
        const int range,
        const int *isValid,
        long *seed,
        int *passes
        );


/* findStrongholds_pre19
 * ---------------------
 * Finds the 3 stronghold positions for the specified world seed up to MC 1.9.
 * Warning: Slow!
 *
 * g         : generator layer stack [world seed will be updated]
 * cache     : biome buffer, set to NULL for temporary allocation
 * locations : output block positions for the 3 strongholds
 * worldSeed : world seed used for the generator
 */
void findStrongholds_pre19(LayerStack *g, int *cache, Pos *locations, long worldSeed);

/* getSpawn
 * --------
 * Finds the spawn point in the world.
 * Warning: Slow, and may be inaccurate because the world spawn depends on
 * grass blocks!
 *
 * g         : generator layer stack [world seed will be updated]
 * cache     : biome buffer, set to NULL for temporary allocation
 * worldSeed : world seed used for the generator
 */
Pos getSpawn(LayerStack *g, int *cache, long worldSeed);



/* areBiomesViable
 * ---------------
 * Determines if the given area contains only biomes specified by 'biomeList'.
 * Used to determine the positions of villages, ocean monument and mansions.
 * Warning: accurate, but slow!
 *
 * g          : generator layer stack
 * cache      : biome buffer, set to NULL for temporary allocation
 * posX, posZ : centre for the check
 * radius     : 'radius' of the check area
 * isValid    : boolean array of valid biome ids (size = 256)
 */
int areBiomesViable(
        const LayerStack g,
        int *cache,
        const int posX,
        const int posZ,
        const int radius,
        const int *isValid
        );



/* isViableWitchHutPos
 * isViableVillagePos
 * isViableOceanMonumentPos
 * isViableMansionPos
 * ------------------------
 * Perform the biome check at the specified block coordinates to determine
 * whether the corresponding structure would spawn. You can get the block
 * positions using the appropriate getXXXPos() function.
 *
 * g              : generator layer stack [set seed beforehand with applySeed()]
 * cache          : biome buffer, set to NULL for temporary allocation
 * blockX, blockZ : block coordinates
 *
 * The return value is non-zero if the position is valid, and in the case of
 * isViableWitchHutPos() the return value is an enum of the temple type.
 */
int isViableWitchHutPos(const LayerStack g, int *cache, const long blockX, const long blockZ);
int isViableVillagePos(const LayerStack g, int *cache, const long blockX, const long blockZ);
int isViableOceanMonumentPos(const LayerStack g, int *cache, const long blockX, const long blockZ);
int isViableMansionPos(const LayerStack g, int *cache, const long blockX, const long blockZ);



/* getBiomeRadius
 * --------------
 * Finds the smallest radius (by square around the origin) at which all the
 * specified biomes are present. The input map is assumed to be a square of
 * side length 'sideLen'.
 *
 * map             : square biome map to be tested
 * sideLen         : side length of the square map (should be 2*radius+1)
 * biomes          : list of biomes to check for
 * bnum            : length of 'biomes'
 * ignoreMutations : flag to count mutated biomes as their original form
 *
 * Return the radius on the square map that covers all biomes in the list.
 * If the map does not contain all the specified biomes, -1 is returned.
 */
int getBiomeRadius(
        const int *map,
        const int mapSide,
        const int *biomes,
        const int bnum,
        const int ignoreMutations);


/******************************** Seed Filters *********************************
 */

/* filterAllTempCats
 * -----------------
 * Looks through the seeds in 'seedsIn' and copies those for which all
 * temperature categories are present in the 3x3 area centred on the specified
 * coordinates into 'seedsOut'. The map scale at this layer is 1:1024.
 *
 * g           : generator layer stack, (NOTE: seed will be modified)
 * cache       : biome buffer, set to NULL for temporary allocation
 * seedsIn     : list of seeds to check
 * seedsOut    : output buffer for the candidate seeds
 * seedCnt     : number of seeds in 'seedsIn'
 * centX, centZ: search origin centre (in 1024 block units)
 *
 * Returns the number of found candidates.
 */
long filterAllTempCats(
        LayerStack *g,
        int *cache,
        const long *seedsIn,
        long *seedsOut,
        const long seedCnt,
        const int centX,
        const int centZ);


/* filterAllMajorBiomes
 * --------------------
 * Looks through the list of seeds in 'seedsIn' and copies those that have all
 * major overworld biomes in the specified area into 'seedsOut'. These checks
 * are done at a scale of 1:256.
 *
 * g           : generator layer stack, (NOTE: seed will be modified)
 * cache       : biome buffer, set to NULL for temporary allocation
 * seedsIn     : list of seeds to check
 * seedsOut    : output buffer for the candidate seeds
 * seedCnt     : number of seeds in 'seedsIn'
 * pX, pZ      : search starting coordinates (in 256 block units)
 * sX, sZ      : size of the searching area (in 256 block units)
 *
 * Returns the number of seeds found.
 */
long filterAllMajorBiomes(
        LayerStack *g,
        int *cache,
        const long *seedsIn,
        long *seedsOut,
        const long seedCnt,
        const int pX,
        const int pZ,
        const uint sX,
        const uint sZ
        );

/********************** C copy of the Java Random methods **********************
 */

static inline void setSeed(long *seed)
{
    *seed = (*seed ^ 0x5DEECE66DL) & ((1L << 48) - 1);
}

static inline int next(long *seed, const int bits)
{
    *seed = (*seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
    return (int) (*seed >> (48 - bits));
}

static inline int nextInt(long *seed, const int n)
{
    int bits, val;
    do {
        bits = next(seed, 31);
        val = bits % n;
    }
    while(bits - val + (n - 1) < 0);
    return val;
}

static inline long nextLong(long *seed)
{
    return ((long) next(seed, 32) << 32) + next(seed, 32);
}

static inline float nextFloat(long *seed)
{
    return next(seed, 24) / (float) (1 << 24);
}

static inline double nextDouble(long *seed)
{
    return (((long) next(seed, 26) << 27) + next(seed, 27)) / (double) (1L << 53);
}



// Custom, faster alternative for the first and second call to nextInt(24)

static inline int firstInt24(long seed)
{
    seed ^= 0x5deece66d;
    seed = (seed * 0x5deece66d) & 0xffffffffffff;
    seed >>= 17;
    return seed % 24;
}

static inline int secondInt24(long seed)
{
    seed ^= 0x5deece66d;
    seed = (seed * 0x5deece66d + 0xB) & 0xffffffffffff;
    seed = (seed * 0x5deece66d) & 0xffffffffffff;
    seed >>= 17;
    return seed % 24;
}

/**
 * invSeed48()
 * -----------
 * Returns the previous 48-bit seed which will generate 'nseed'.
 * The upper 16 bits are ignored, both here and in the generator.
 */
static inline long invSeed48(long nseed)
{
    const long x = 0x5deece66d;
    const long xinv = 0xdfe05bcb1365L;
    const long y = 0xbL;
    const long m48 = 0xffffffffffffL;

    long a = nseed >> 32;
    long b = nseed & 0xffffffffL;
    if(b & 0x80000000L) a++;

    long q = ((b << 16) - y - (a << 16)*x) & m48;
    for(long k = 0; k <= 5; k++)
    {
        long d = (x - (q + (k << 48))) % x;
        d = (d + x) % x; // force the modulo and keep it positive
        if(d < 65536)
        {
            long c = ((q + d) * xinv) & m48;
            if(c < 65536)
            {
                return ((((a << 16) + c) - y) * xinv) & m48;
            }
        }
    }
    return -1;
}


static inline const char *int2binstr(int x, int bit)
{
    static char str[33];
    str[0] = '\0';
    for(bit = (1 << bit); bit > 0; bit >>= 1)
        strcat(str, ((x & bit) == bit) ? "1" : "0");

    return str;
}

#endif /* FINDERS_H_ */
