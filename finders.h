#ifndef FINDERS_H_
#define FINDERS_H_

#include "generator.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>

typedef HANDLE thread_id_t;

#else
#define USE_PTHREAD
#include <pthread.h>

typedef pthread_t thread_id_t;

#endif


#define SEED_BASE_MAX (1LL << 48)
#define PI 3.141592653589793

#define LARGE_STRUCT 1
#define CHUNK_STRUCT 2

enum
{
    Desert_Pyramid, Igloo, Jungle_Pyramid, Swamp_Hut,
    Village, Ocean_Ruin, Shipwreck, Monument, Mansion, Outpost,
    Ruined_Portal
};

enum // village house types prior to 1.14
{
    HouseSmall, Church, Library, WoodHut, Butcher, FarmLarge, FarmSmall,
    Blacksmith, HouseLarge, HOUSE_NUM
};

STRUCT(StructureConfig)
{
    int64_t seed;
    int regionSize, chunkRange;
    int properties;
};

/* for desert temples, igloos, jungle temples and witch huts prior to 1.13 */
static const StructureConfig FEATURE_CONFIG        = { 14357617, 32, 24, 0};

/* ocean features before 1.16 */
static const StructureConfig OCEAN_RUIN_CONFIG_113 = { 14357621, 16,  8, 0};
static const StructureConfig SHIPWRECK_CONFIG_113  = {165745295, 15,  7, 0};

/* 1.13 separated feature seeds by type */
static const StructureConfig DESERT_PYRAMID_CONFIG = { 14357617, 32, 24, 0};
static const StructureConfig IGLOO_CONFIG          = { 14357618, 32, 24, 0};
static const StructureConfig JUNGLE_PYRAMID_CONFIG = { 14357619, 32, 24, 0};
static const StructureConfig SWAMP_HUT_CONFIG      = { 14357620, 32, 24, 0};

static const StructureConfig OUTPOST_CONFIG        = {165745296, 32, 24, 0};
static const StructureConfig VILLAGE_CONFIG        = { 10387312, 32, 24, 0};
static const StructureConfig OCEAN_RUIN_CONFIG     = { 14357621, 20, 12, 0};
static const StructureConfig SHIPWRECK_CONFIG      = {165745295, 24, 20, 0};
static const StructureConfig MONUMENT_CONFIG       = { 10387313, 32, 27, LARGE_STRUCT};
static const StructureConfig MANSION_CONFIG        = { 10387319, 80, 60, LARGE_STRUCT};
static const StructureConfig RUINED_PORTAL_CONFIG  = { 34222645, 40, 25, 0}; // overworld variant

// structures that check each chunk individually
static const StructureConfig TREASURE_CONFIG       = { 10387320,  1,  0, CHUNK_STRUCT};

//==============================================================================
// Biome Tables
//==============================================================================

static const int templeBiomeList[] = {desert, desert_hills, jungle, jungle_hills, swamp, snowy_tundra, snowy_taiga};
static const int biomesToSpawnIn[] = {forest, plains, taiga, taiga_hills, wooded_hills, jungle, jungle_hills};
static const int villageBiomeList[] = {plains, desert, savanna, taiga, snowy_tundra};
static const int villageBiomeListBE[] = {plains, desert, savanna, taiga, snowy_tundra, snowy_taiga};
static const int mansionBiomeList[] = {dark_forest, dark_forest+128};
static const int oceanMonumentBiomeList1[] =
{
        ocean, deep_ocean, river, frozen_river,
        frozen_ocean, deep_frozen_ocean, cold_ocean, deep_cold_ocean,
        lukewarm_ocean, deep_lukewarm_ocean, warm_ocean, deep_warm_ocean
};
static const int oceanMonumentBiomeList2[] =
{
        deep_frozen_ocean, deep_cold_ocean, deep_ocean, deep_lukewarm_ocean, deep_warm_ocean
};

static const int achievementBiomes_1_7[] =
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

STRUCT(BiomeFilter)
{
    // bitfield for required temperature categories, including special variants
    uint64_t tempCat;
    // bitfield for the required ocean types
    uint64_t oceansToFind;
    // bitfield of required biomes without modification bit
    uint64_t biomesToFind;
    // bitfield of required modified biomes
    uint64_t modifiedToFind; // TODO: add checks for bamboo_jungle*

    // check that there is a minimum of both special and normal temperatures
    int tempNormal, tempSpecial;
    // check for the temperatures specified by tempCnt (1:1024)
    int doTempCheck;
    // check for mushroom potential
    int requireMushroom;
    // combine a more detailed mushroom and temperature check (1:256)
    int doShroomAndTempCheck;
    // early check for 1.13 ocean types (1:256)
    int doOceanTypeCheck;
    //
    int doMajorBiomeCheck;
    // pre-generation biome checks in layer L_BIOME_256
    int checkBiomePotential;
    //
    int doScale4Check;
};

#ifdef __cplusplus
extern "C"
{
#endif

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
 *
 */



//==============================================================================
// Moving Structures
//==============================================================================

/* Transposes a base seed such that structures are moved by the specified region
 * vector, (regX, regZ).
 */
static inline int64_t moveStructure(const int64_t baseSeed,
        const int regX, const int regZ)
{
    return (baseSeed - regX*341873128712 - regZ*132897987541) & 0xffffffffffff;
}



//==============================================================================
// Saving & Loading Seeds
//==============================================================================

/* Loads a list of seeds from a file. The seeds should be written as decimal
 * UFT-8 numbers separated by newlines.
 * @fnam: file path
 * @scnt: number of valid seeds found in the file, which is also the number of
 *        elements in the returned buffer
 *
 * Return a pointer to dynamically allocated seed list.
 */
int64_t *loadSavedSeeds(const char *fnam, int64_t *scnt);



//==============================================================================
// Multi-Structure-Base Checks
//==============================================================================

/* Calls the correct quad-base finder for the structure config, if available.
 * (Exits program otherwise.)
 */
int isQuadBase(const StructureConfig sconf, const int64_t seed, const int64_t qual);

/* Calls the correct tri-base finder for the structure config, if available.
 * (Exits program otherwise.)
 */
int isTriBase(const StructureConfig sconf, const int64_t seed, const int64_t qual);

/* Starts a multi-threaded search for structure base seeds  of the specified
 * quality (chunk tolerance). The result is saved in a file of path 'fnam'.
 */
void search4QuadBases(const char *fnam, int threads,
        const StructureConfig structureConfig, int quality);

void checkVec4QuadBases(const StructureConfig sconf, int64_t seeds[256]);

//==============================================================================
// Finding Structure Positions
//==============================================================================

/* Fast implementation for finding the block position at which the structure
 * generation attempt will occur within the specified region.
 * This function applies for scattered-feature structureSeeds and villages.
 */
Pos getStructurePos(const StructureConfig config, int64_t seed,
        const int regionX, const int regionZ);

/* Finds the chunk position within the specified region (a square region of
 * chunks depending on structure type) where the structure generation attempt
 * will occur.
 * This function applies for scattered-feature structureSeeds and villages.
 */
Pos getStructureChunkInRegion(const StructureConfig config, int64_t seed,
        const int regionX, const int regionZ);

/* Fast implementation for finding the block position at which the ocean
 * monument or woodland mansion generation attempt will occur within the
 * specified region.
 */
Pos getLargeStructurePos(const StructureConfig config, int64_t seed,
        const int regionX, const int regionZ);

/* Fast implementation for finding the chunk position at which the ocean
 * monument or woodland mansion generation attempt will occur within the
 * specified region.
 */
Pos getLargeStructureChunkInRegion(const StructureConfig config, int64_t seed,
        const int regionX, const int regionZ);

/* Some structures check each chunk individually for viability.
 * The placement and biome check within a valid chunk is at block position (9,9).
 */
int isMineshaftChunk(int64_t seed, const int chunkX, const int chunkZ);
int isTreasureChunk(int64_t seed, const int chunkX, const int chunkZ);


//==============================================================================
// Checking Biomes & Biome Helper Functions
//==============================================================================

/* Returns the biome for the specified block position.
 * (Alternatives should be considered first in performance critical code.)
 */
int getBiomeAtPos(const LayerStack g, const Pos pos);

/* Finds a suitable pseudo-random location in the specified area.
 * This function is used to determine the positions of spawn and strongholds.
 * Warning: accurate, but slow!
 *
 * @mcversion        : Minecraft version (changed in: 1.7, 1.13)
 * @g                : generator layer stack
 * @cache            : biome buffer, set to NULL for temporary allocation
 * @centreX, centreZ : origin for the search
 * @range            : square 'radius' of the search
 * @isValid          : boolean array of valid biome ids (size = 256)
 * @seed             : seed used for the RNG
 *                     (initialise RNG using setSeed(&seed))
 * @passes           : number of valid biomes passed, set to NULL to ignore this
 */
Pos findBiomePosition(
        const int           mcversion,
        const LayerStack    g,
        int *               cache,
        const int           centerX,
        const int           centerZ,
        const int           range,
        const int *         isValid,
        int64_t *           seed,
        int *               passes
        );

/* Determines if the given area contains only biomes specified by 'biomeList'.
 * This function is used to determine the positions of villages, ocean monuments
 * and mansions.
 * Warning: accurate, but slow!
 *
 * @g          : generator layer stack
 * @cache      : biome buffer, set to NULL for temporary allocation
 * @posX, posZ : centre for the check
 * @radius     : 'radius' of the check area
 * @isValid    : boolean array of valid biome ids (size = 256)
 */
int areBiomesViable(
        const LayerStack    g,
        int *               cache,
        const int           posX,
        const int           posZ,
        const int           radius,
        const int *         isValid
        );

/* Finds the smallest radius (by square around the origin) at which all the
 * specified biomes are present. The input map is assumed to be a square of
 * side length 'sideLen'.
 *
 * @map             : square biome map to be tested
 * @sideLen         : side length of the square map (should be 2*radius+1)
 * @biomes          : list of biomes to check for
 * @bnum            : length of 'biomes'
 * @ignoreMutations : flag to count mutated biomes as their original form
 *
 * Return the radius on the square map that covers all biomes in the list.
 * If the map does not contain all the specified biomes, -1 is returned.
 */
int getBiomeRadius(
        const int *     map,
        const int       mapSide,
        const int *     biomes,
        const int       bnum,
        const int       ignoreMutations);



//==============================================================================
// Finding Strongholds and Spawn
//==============================================================================

/* Finds the block positions of the strongholds in the world. Note that the
 * number of strongholds was increased from 3 to 128 in MC 1.9.
 * Warning: Slow!
 *
 * @mcversion : Minecraft version (changed in 1.7, 1.9, 1.13)
 * @g         : generator layer stack [worldSeed should be applied before call!]
 * @cache     : biome buffer, set to NULL for temporary allocation
 * @locations : output block positions
 * @worldSeed : world seed of the generator
 * @maxSH     : Stop when this many strongholds have been found. A value of 0
 *              defaults to 3 for mcversion <= MC_1_8, and to 128 for >= MC_1_9.
 * @maxRadius : Stop searching if the radius exceeds this value in meters.
 *              Set this to 0 to ignore this condition.
 *
 * Returned is the number of strongholds found.
 */
int findStrongholds(
        const int           mcversion,
        LayerStack *        g,
        int *               cache,
        Pos *               locations,
        int64_t             worldSeed,
        int                 maxSH,
        const int           maxRadius
        );

/* Finds the spawn point in the world.
 * Warning: Slow, and may be inaccurate because the world spawn depends on
 * grass blocks!
 *
 * @mcversion : Minecraft version (changed in 1.7, 1.13)
 * @g         : generator layer stack [worldSeed should be applied before call!]
 * @cache     : biome buffer, set to NULL for temporary allocation
 * @worldSeed : world seed used for the generator
 */
Pos getSpawn(const int mcversion, LayerStack *g, int *cache, int64_t worldSeed);

/* Finds the approximate spawn point in the world.
 *
 * @mcversion : Minecraft version (changed in 1.7, 1.13)
 * @g         : generator layer stack [worldSeed should be applied before call!]
 * @cache     : biome buffer, set to NULL for temporary allocation
 * @worldSeed : world seed used for the generator
 */
Pos estimateSpawn(const int mcversion, LayerStack *g, int *cache, int64_t worldSeed);


//==============================================================================
// Validating Structure Positions
//==============================================================================

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


/* These functions perform a biome check at the specified block coordinates to
 * determine whether the corresponding structure would spawn there. You can get
 * the block positions using the appropriate getXXXPos() function.
 *
 * @g              : generator layer stack [set seed using applySeed()]
 * @cache          : biome buffer, set to NULL for temporary allocation
 * @blockX, blockZ : block coordinates
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



//==============================================================================
// Finding Properties of Structures
//==============================================================================

/* Initialises and returns a random seed used in the (16x16) chunk generation.
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

/* Checks if the village in the given region would be infested by zombies.
 * (Minecraft 1.10+)
 */
int isZombieVillage(const int mcversion, const int64_t worldSeed,
        const int regionX, const int regionZ);

/* Checks if the village in the given region would generate as a baby zombie
 * village. (The fact that these exist could be regarded as a bug.)
 * (Minecraft 1.12)
 */
int isBabyZombieVillage(const int mcversion, const int64_t worldSeed,
        const int regionX, const int regionZ);

/* Finds the number of each type of house that generate in a village.
 * @worldSeed      : world seed
 * @chunkX, chunkZ : 16x16 chunk position of the village origin
 * @housesOut      : output number of houses for each entry in the house type
 *                   enum (i.e this should be an array of length HOUSE_NUM)
 *
 * Returns the random object seed after finding these numbers.
 */
int64_t getHouseList(const int64_t worldSeed, const int chunkX, const int chunkZ,
        int *housesOut);


//==============================================================================
// Seed Filters
//==============================================================================

/* Looks through the seeds in 'seedsIn' and copies those for which all
 * temperature categories are present in the 3x3 area centred on the specified
 * coordinates into 'seedsOut'. The map scale at this layer is 1:1024.
 *
 * @g            : generator layer stack, (NOTE: seed will be modified)
 * @cache        : biome buffer, set to NULL for temporary allocation
 * @seedsIn      : list of seeds to check
 * @seedsOut     : output buffer for the candidate seeds
 * @seedCnt      : number of seeds in 'seedsIn'
 * @centX, centZ : search origin centre (in 1024 block units)
 *
 * Returns the number of found candidates.
 */
int64_t filterAllTempCats(
        LayerStack *        g,
        int *               cache,
        const int64_t *     seedsIn,
        int64_t *           seedsOut,
        const int64_t       seedCnt,
        const int           centX,
        const int           centZ
        );

/* Looks through the list of seeds in 'seedsIn' and copies those that have all
 * major overworld biomes in the specified area into 'seedsOut'. These checks
 * are done at a scale of 1:256.
 *
 * @g           : generator layer stack, (NOTE: seed will be modified)
 * @cache       : biome buffer, set to NULL for temporary allocation
 * @seedsIn     : list of seeds to check
 * @seedsOut    : output buffer for the candidate seeds
 * @seedCnt     : number of seeds in 'seedsIn'
 * @pX, pZ      : search starting coordinates (in 256 block units)
 * @sX, sZ      : size of the searching area (in 256 block units)
 *
 * Returns the number of seeds found.
 */
int64_t filterAllMajorBiomes(
        LayerStack *        g,
        int *               cache,
        const int64_t *     seedsIn,
        int64_t *           seedsOut,
        const int64_t       seedCnt,
        const int           pX,
        const int           pZ,
        const unsigned int  sX,
        const unsigned int  sZ
        );

/* Creates a biome filter configuration from a given list of biomes.
 */
BiomeFilter setupBiomeFilter(const int *biomeList, int listLen);

/* Tries to determine if the biomes configured in the filter will generate in
 * this seed within the specified area. The smallest layer scale checked is
 * given by 'minscale'. Lowering this value terminate the search earlier and
 * yield more false positives.
 */
int64_t checkForBiomes(
        LayerStack *        g,
        int *               cache,
        const int64_t       seed,
        const int           blockX,
        const int           blockZ,
        const unsigned int  width,
        const unsigned int  height,
        const BiomeFilter   filter,
        const int           minscale);

#ifdef __cplusplus
}
#endif

#endif /* FINDERS_H_ */
