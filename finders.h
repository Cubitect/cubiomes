#ifndef FINDERS_H_
#define FINDERS_H_

#include "generator.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>


#define DEFAULT_THREADS 6
#define SEED_BASE_MAX (1LL << 48)
#define PI 3.141592653589793

#define LARGE_STRUCT 1
#define USE_POW2_RNG 2

STRUCT(StructureConfig) {
    int64_t seed;
    int regionSize, chunkRange;
    int properties;
};

/* For desert temples, igloos, jungle temples and witch huts prior to 1.13. */
extern const StructureConfig FEATURE_CONFIG;

/* 1.13 separated feature seeds by type */
extern const StructureConfig DESERT_PYRAMID_CONFIG;
extern const StructureConfig IGLOO_CONFIG;
extern const StructureConfig JUNGLE_PYRAMID_CONFIG;
extern const StructureConfig SWAMP_HUT_CONFIG;

extern const StructureConfig VILLAGE_CONFIG;
extern const StructureConfig OCEAN_RUIN_CONFIG;
extern const StructureConfig SHIPWRECK_CONFIG;
extern const StructureConfig MONUMENT_CONFIG;
extern const StructureConfig MANSION_CONFIG;

enum {
    Desert_Pyramid, Igloo, Jungle_Pyramid, Swamp_Hut,
    Village, Ocean_Ruin, Shipwreck, Monument, Mansion
};

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
 *  the structure positioning. Additionally the position of most structures in a
 *  world can be translated by applying the following transformation to the
 *  seed:
 *
 *  seed2 = seed1 - dregX * 341873128712 - dregZ * 132897987541;
 *
 *  Here seed1 and seed2 have the same structure positioning, but moved by a
 *  region offset of (dregX,dregZ). [a region is 32x32 chunks].
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


/* isQuadBase
 * ----------
 * Calls the correct quad-base finder for the structure config, if available.
 * (Exits program otherwise.)
 */
int isQuadBase(const StructureConfig sconf, const int64_t seed, const int64_t qual);


/* isTriBase
 * ----------
 * Calls the correct tri-base finder for the structure config, if available.
 * (Exits program otherwise.)
 */
int isTriBase(const StructureConfig sconf, const int64_t seed, const int64_t qual);


/* moveStructure
 * -------------
 * Transposes a base seed such that structures are moved by the specified region
 * vector, (regX, regZ).
 */
int64_t moveStructure(const int64_t baseSeed, const int regX, const int regZ);

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
int64_t *loadSavedSeeds(const char *fnam, int64_t *scnt);

/* search4QuadBases
 * ----------------
 * Starts a multi-threaded search for structure base seeds of the specified
 * quality (chunk tolerance). The result is saved in a file of path 'fnam'.
 */
void search4QuadBases(const char *fnam, int threads,
        const StructureConfig structureConfig, int quality);



/**************************** General Biome Checks *****************************
 */

/* getBiomeAtPos
 * -------------
 * Returns the biome for the specified block position.
 * (Alternatives should be considered in performance critical code.)
 * This function is not threadsafe.
 */
int getBiomeAtPos(const LayerStack g, const Pos pos);


/* getOceanRuinPos
 * ---------------
 * Fast implementation for finding the block position at which an ocean ruin
 * generation attempt will occur in the specified region.
 */
Pos getOceanRuinPos(int64_t seed, const int regionX, const int regionZ);

/* getStructurePos
 * ---------------
 * Fast implementation for finding the block position at which the structure
 * generation attempt will occur in the specified region.
 * This function applies for scattered-feature structureSeeds and villages.
 */
Pos getStructurePos(StructureConfig config, int64_t seed,
        const int regionX, const int regionZ);

/* getStructureChunkInRegion
 * -------------------------
 * Finds the chunk position within the specified region (a square region of
 * chunks depending on structure type) where the structure generation attempt
 * will occur.
 *
 * This function applies for scattered-feature structureSeeds and villages.
 */
Pos getStructureChunkInRegion(StructureConfig config, int64_t seed,
        const int regionX, const int regionZ);

/* getLargeStructurePos
 * --------------------
 * Fast implementation for finding the block position at which the ocean
 * monument or woodland mansion generation attempt will occur in the
 * specified region.
 */
Pos getLargeStructurePos(StructureConfig config, int64_t seed,
        const int regionX, const int regionZ);

/* getLargeStructureChunkInRegion
 * ------------------------------
 * Fast implementation for finding the chunk position at which the ocean
 * monument or woodland mansion generation attempt will occur in the
 * specified region.
 */
Pos getLargeStructureChunkInRegion(StructureConfig config, int64_t seed,
        const int regionX, const int regionZ);



/************************ Biome Checks for Structures **************************
 *
 * Scattered features only do a simple check of the biome at the block position
 * of the structure origin (i.e. the north-west corner). Before 1.13 the type of
 * structure was determined by the biome, while in 1.13 the scattered feature
 * positions are calculated separately for each type. However, the biome
 * requirements remain the same:
 *
 *  Desert Pyramid: desert or desertHills
 *  Igloo         : icePlains or coldTaiga
 *  Jungle Pyramid: jungle or jungleHills
 *  Swamp Hut     : swampland
 *
 * Similarly, Ocean Ruins and Shipwrecks require any oceanic biome at their
 * block position.
 *
 * Villages, Monuments and Mansions on the other hand require a certain area to
 * be of a valid biome and the check is performed at a 1:4 scale instead of 1:1.
 * (Actually the area for villages has a radius zero, which means it is a simple
 * biome check at a 1:4 scale.)
 */



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
        int64_t *seed,
        int *passes
        );
Pos findBiomePosition113(
        const LayerStack g,
        int *cache,
        const int centerX,
        const int centerZ,
        const int range,
        const int *isValid,
        int64_t *seed,
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
void findStrongholds_pre19(LayerStack *g, int *cache, Pos *locations, int64_t worldSeed);

/* findStrongholds
 * ---------------
 * Finds up to 128 strongholds which generate since MC 1.9. Returns the number
 * of strongholds found within the specified radius.
 * Warning: Slow!
 *
 * g         : generator layer stack [world seed will be updated]
 * cache     : biome buffer, set to NULL for temporary allocation
 * locations : output block positions for the 128 strongholds
 * worldSeed : world seed used for the generator
 * maxRadius : Stop searching if the radius exceeds this value in meters. Set to
 *             0 to return all strongholds.
 */
int findStrongholds(LayerStack *g, int *cache, Pos *locations, int64_t worldSeed, int maxRadius);
int findStrongholds113(LayerStack *g, int *cache, Pos *locations, int64_t worldSeed, int maxRadius);

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
Pos getSpawn(LayerStack *g, int *cache, int64_t worldSeed);
Pos getSpawn113(LayerStack *g, int *cache, int64_t worldSeed);

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


/* isViableFeaturePos
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
 * In the case of isViableFeaturePos() the 'type' argument specifies the type of
 * scattered feature (as an enum) for which the check is performed.
 *
 * The return value is non-zero if the position is valid.
 */
int isViableFeaturePos(const int type, const LayerStack g, int *cache, const int blockX, const int blockZ);
int isViableVillagePos(const LayerStack g, int *cache, const int blockX, const int blockZ);
int isViableOceanMonumentPos(const LayerStack g, int *cache, const int blockX, const int blockZ);
int isViableMansionPos(const LayerStack g, int *cache, const int blockX, const int blockZ);


/************************* FINDING STRUCTURE PROPERTIES ************************
 */

/* chunkGenerateRnd
 * ----------------
 * Initialises and returns a random seed used in the (16x16) chunk generation.
 * This random object is used for recursiveGenerate() which is responsible for
 * generating caves, ravines, mineshafts, and virtually all other structures.
 */
inline static int64_t chunkGenerateRnd(const int64_t worldSeed, const int chunkX, const int chunkZ)
{
    int64_t rnd = worldSeed;
    setSeed(&rnd);
    rnd = (nextLong(&rnd) * chunkX) ^ (nextLong(&rnd) * chunkZ) ^ worldSeed;
    setSeed(&rnd);
    return rnd;
}



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
int64_t filterAllTempCats(
        LayerStack *g,
        int *cache,
        const int64_t *seedsIn,
        int64_t *seedsOut,
        const int64_t seedCnt,
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
int64_t filterAllMajorBiomes(
        LayerStack *g,
        int *cache,
        const int64_t *seedsIn,
        int64_t *seedsOut,
        const int64_t seedCnt,
        const int pX,
        const int pZ,
        const unsigned int sX,
        const unsigned int sZ
        );



static inline const char *int2binstr(int x, int bit)
{
    static char str[33];
    str[0] = '\0';
    for(bit = (1 << bit); bit > 0; bit >>= 1)
        strcat(str, ((x & bit) == bit) ? "1" : "0");

    return str;
}

#endif /* FINDERS_H_ */
