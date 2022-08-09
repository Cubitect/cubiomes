#ifndef FINDERS_H_
#define FINDERS_H_

#include "generator.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>

typedef HANDLE thread_id_t;

#else
#define USE_PTHREAD
#include <pthread.h>

typedef pthread_t thread_id_t;

#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define MASK48 (((int64_t)1 << 48) - 1)
#define PI 3.141592653589793

#define LARGE_STRUCT 1
#define CHUNK_STRUCT 2

enum StructureType
{
    Feature, // for locations of temple generation attempts pre 1.13
    Desert_Pyramid,
    Jungle_Temple, Jungle_Pyramid = Jungle_Temple,
    Swamp_Hut,
    Igloo,
    Village,
    Ocean_Ruin,
    Shipwreck,
    Monument,
    Mansion,
    Outpost,
    Ruined_Portal,
    Ruined_Portal_N,
    Ancient_City,
    Treasure,
    Mineshaft,
    Fortress,
    Bastion,
    End_City,
    End_Gateway,
    FEATURE_NUM
};

enum // village house types prior to 1.14
{
    HouseSmall, Church, Library, WoodHut, Butcher, FarmLarge, FarmSmall,
    Blacksmith, HouseLarge, HOUSE_NUM
};


STRUCT(StructureConfig)
{
    int             salt;
    char            regionSize;
    char            chunkRange;
    unsigned char   structType;
    unsigned char   properties;
};

#define _sc static const StructureConfig

/* for desert pyramids, jungle temples, witch huts and igloos prior to 1.13 */
_sc FEATURE_CONFIG            = { 14357617, 32, 24, Feature, 0};
_sc IGLOO_CONFIG_112          = { 14357617, 32, 24, Igloo, 0};
_sc SWAMP_HUT_CONFIG_112      = { 14357617, 32, 24, Swamp_Hut, 0};
_sc DESERT_PYRAMID_CONFIG_112 = { 14357617, 32, 24, Desert_Pyramid, 0};
_sc JUNGLE_PYRAMID_CONFIG_112 = { 14357617, 32, 24, Jungle_Pyramid, 0};

/* ocean features before 1.16 */
_sc OCEAN_RUIN_CONFIG_115     = { 14357621, 16,  8, Ocean_Ruin, 0};
_sc SHIPWRECK_CONFIG_115      = {165745295, 15,  7, Shipwreck, 0};

/* 1.13 separated feature seeds by type */
_sc DESERT_PYRAMID_CONFIG     = { 14357617, 32, 24, Desert_Pyramid, 0};
_sc IGLOO_CONFIG              = { 14357618, 32, 24, Igloo, 0};
_sc JUNGLE_PYRAMID_CONFIG     = { 14357619, 32, 24, Jungle_Pyramid, 0};
_sc SWAMP_HUT_CONFIG          = { 14357620, 32, 24, Swamp_Hut, 0};
_sc OUTPOST_CONFIG            = {165745296, 32, 24, Outpost, 0};
_sc VILLAGE_CONFIG_117        = { 10387312, 32, 24, Village, 0};
_sc VILLAGE_CONFIG            = { 10387312, 34, 26, Village, 0};
_sc OCEAN_RUIN_CONFIG         = { 14357621, 20, 12, Ocean_Ruin, 0};
_sc SHIPWRECK_CONFIG          = {165745295, 24, 20, Shipwreck, 0};
_sc MONUMENT_CONFIG           = { 10387313, 32, 27, Monument, LARGE_STRUCT};
_sc MANSION_CONFIG            = { 10387319, 80, 60, Mansion, LARGE_STRUCT};
_sc RUINED_PORTAL_CONFIG      = { 34222645, 40, 25, Ruined_Portal, 0}; // overworld
_sc RUINED_PORTAL_N_CONFIG_117= { 34222645, 25, 15, Ruined_Portal_N, 0}; // nether
_sc ANCIENT_CITY_CONFIG       = { 20083232, 24, 16, Ancient_City, 0};

_sc TREASURE_CONFIG           = { 10387320,  1,  1, Treasure, CHUNK_STRUCT};
_sc MINESHAFT_CONFIG          = {        0,  1,  1, Mineshaft, CHUNK_STRUCT};

// nether and end structures
_sc FORTRESS_CONFIG_115       = {        0, 16,  8, Fortress, 0};
_sc FORTRESS_CONFIG           = { 30084232, 27, 23, Fortress, 0};
_sc BASTION_CONFIG            = { 30084232, 27, 23, Bastion, 0};
_sc END_CITY_CONFIG           = { 10387313, 20,  9, End_City, LARGE_STRUCT};

// for the scattered return gateways
_sc END_GATEWAY_CONFIG_115    = {    30000,  1,  1, End_Gateway, CHUNK_STRUCT};
_sc END_GATEWAY_CONFIG        = {    40013,  1,  1, End_Gateway, CHUNK_STRUCT};

#undef _sc

STRUCT(Pos)
{
    int x, z;
};

enum
{
    CFB_APPROX      = 0x01, // enabled aggresive filtering, trading accuracy
};
STRUCT(BiomeFilter)
{
    // bitfields for biomes required at their respecive layers
    uint64_t tempsToFind; // Special (1:1024)
    uint64_t otempToFind; // OceanTemp (1:256)
    uint64_t majorToFind; // Biome (1:256)
    uint64_t edgesToFind; // Edge (1:64) [mod64: as special case for bamboo]
    // bitfields for biomes to find at RareBiome(1:64), Shore(1:16) and Mix(1:4)
    // layers for (biomeID < 64) and modified (biomeID >= 128 && biomeID < 192)
    uint64_t raresToFind, raresToFindM;
    uint64_t shoreToFind, shoreToFindM;
    uint64_t riverToFind, riverToFindM;
    uint64_t oceanToFind; // all required ocean types

    int specialCnt; // number of special temperature categories required

    uint32_t flags;

    // the biome exclusion aborts generation when the area contains no biomes
    // that can generate the excluded biomes
    uint64_t tempsToExcl;
    uint64_t majorToExcl;
    uint64_t edgesToExcl;
    uint64_t raresToExcl, raresToExclM;
    uint64_t shoreToExcl, shoreToExclM;
    uint64_t riverToExcl, riverToExclM;

    uint64_t biomeToExcl, biomeToExclM;
    uint64_t biomeToFind, biomeToFindM;
    uint64_t biomeToPick, biomeToPickM;
};

STRUCT(StrongholdIter)
{
    Pos pos;        // accurate location of current stronghold
    Pos nextapprox; // approxmimate location (+/-112 blocks) of next stronghold
    int index;      // stronghold index counter
    int ringnum;    // ring number for index
    int ringmax;    // max index within ring
    int ringidx;    // index within ring
    double angle;   // next angle within ring
    double dist;    // next distance from origin (in chunks)
    uint64_t rnds;  // random number seed (48 bit)
    int mc;         // minecraft version
};


STRUCT(StructureVariant)
{
    uint8_t abandoned; // is zombie village
    uint8_t giant; // giant portal variant
    char variant;
    const char *name;
    short biome;
    uint8_t rotation; // 0:0, 1:cw90, 2:cw180, 3:cw270=ccw90
    uint8_t mirror;
    char x, y, z;
    char sx, sy, sz;
};


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


/***************************** Structure Positions *****************************
 *
 *  For most structure positions, Minecraft divides the world into a grid of
 *  regions (usually 32x32 chunks) and performs one generation attempt
 *  somewhere in each region. The position of this attempt is governed by the
 *  structure type, the region coordiates and the lower 48-bits of the world
 *  seed. The remaining top 16 bits do not influence structure positions.
 *  The dependency on the region coordinates is linear for both the X and Z
 *  directions, which means that the positions of most structures in a world
 *  can be translated by applying the following transformation to a seed:
 *
 *  seed2 = seed1 - dregX * 341873128712 - dregZ * 132897987541;
 *
 *  Here seed1 and seed2 have the same structure positioning, but moved by a
 *  region offset of (dregX,dregZ).
 *
 *  Another property of note is that seed1 at region (0,0) is simply the world
 *  seed plus a constant that is specific to the stucture type (its salt). This
 *  means that some structure types share quad-bases which are just offset by
 *  their respective salt differences.
 *
 *
 ** Quad-Witch-Huts
 *
 *  For a quad-structure, we mainly care about relative positioning, so we can
 *  get away with just checking the regions near the origin: (0,0),(0,1),(1,0)
 *  and (1,1) and then move the structures to the desired position.
 *
 *  Futhermore, the PRNG that determines the chunk positions inside each region,
 *  performs some modular arithmatic on the 48-bit numbers which causes some
 *  restrictions on the lower bits when looking for near perfect structure
 *  positions. This is difficult to prove, but can be used to reduce the number
 *  of free bits to 28 which can be comfortably brute-forced to get the entire
 *  set of quad-structure candidates. Each of the seeds found this way
 *  describes entire set of possible quad-witch-huts (with degrees of freedom
 *  for region-transposition, as well as the top 16-bit bits).
 */


// lower 20 bits, only the very best constellations
// (the structure salt has to be subtracted before use)
static const uint64_t low20QuadIdeal[] =
{
        0x43f18,0xc751a,0xf520a,
};

// lower 20 bits, the classic quad-structure constellations
static const uint64_t low20QuadClassic[] =
{
        0x43f18,0x79a0a,0xc751a,0xf520a,
};

// for any valid quad-structure constellation with a structure size:
// (7+1,7+43+1,9+1) which corresponds to a fall-damage based quad-witch-farm,
// but may require a perfect player position
static const uint64_t low20QuadHutNormal[] =
{
        0x43f18,0x65118,0x75618,0x79a0a, 0x89718,0x9371a,0xa5a08,0xb5e18,
        0xc751a,0xf520a,
};

// for any valid quad-structure constellation with a structure size:
// (7+1,7+1,9+1) which corresponds to quad-witch-farms without drop chute
static const uint64_t low20QuadHutBarely[] =
{
        0x1272d,0x17908,0x367b9,0x43f18, 0x487c9,0x487ce,0x50aa7,0x647b5,
        0x65118,0x75618,0x79a0a,0x89718, 0x9371a,0x967ec,0xa3d0a,0xa5918,
        0xa591d,0xa5a08,0xb5e18,0xc6749, 0xc6d9a,0xc751a,0xd7108,0xd717a,
        0xe2739,0xe9918,0xee1c4,0xf520a,
};

//==============================================================================
// Moving Structures
//==============================================================================

/* Transposes a base seed such that structures are moved by the specified region
 * vector, (regX, regZ).
 */
static inline uint64_t moveStructure(uint64_t baseSeed, int regX, int regZ)
{
    return (baseSeed - regX*341873128712 - regZ*132897987541) & 0xffffffffffff;
}



//==============================================================================
// Saving & Loading Seeds
//==============================================================================

/* Loads a list of seeds from a file. The seeds should be written as decimal
 * ASCII numbers separated by newlines.
 * @fnam: file path
 * @scnt: number of valid seeds found in the file, which is also the number of
 *        elements in the returned buffer
 *
 * Return a pointer to a dynamically allocated seed list.
 */
uint64_t *loadSavedSeeds(const char *fnam, uint64_t *scnt);



//==============================================================================
// Finding Structure Positions
//==============================================================================


/* Selects the structure configuration for a given version. Returns zero upon
 * failure (e.g. version does not support structure type).
 */
int getStructureConfig(int structureType, int mc, StructureConfig *sconf);

/* The library can be compiled to use a custom internal getter for structure
 * configurations. For this, the macro STRUCT_CONFIG_OVERRIDE should be defined
 * as true and the function getStructureConfig_override() should be defined
 * with a custom function body. However, note this is experimental and not all
 * structure configs may work. (Ideally only change structure salts.)
 */
#if STRUCT_CONFIG_OVERRIDE
int getStructureConfig_override(int stype, int mc, StructureConfig *sconf);
#endif

/* Finds the block position of the structure generation attempt in a given
 * region. You can use isViableStructurePos() to test if the necessary biome
 * requirements are met for the structure to actually generate at that position.
 * Some structure types may fail to produce a valid position in the given
 * region regardless of biomes, in which case the function returns zero.
 *
 * @structureType   : structure type
 * @mc              : minecraft version
 * @seed            : world seed (only the lower 48-bits are relevant)
 * @regX,regZ       : region coordinates (the region size depends on type)
 * @pos             : output block position
 *
 * Returns zero if the position is invalid, or non-zero otherwise.
 */
int getStructurePos(int structureType, int mc, uint64_t seed, int regX, int regZ, Pos *pos);

/* The inline functions below get the generation attempt position given a
 * structure configuration. Most small structures use the getFeature..
 * variants, which have a uniform distribution, while large structures
 * (monuments and mansions) have a triangular distribution.
 */
static inline ATTR(const)
Pos getFeaturePos(StructureConfig config, uint64_t seed, int regX, int regZ);

static inline ATTR(const)
Pos getFeatureChunkInRegion(StructureConfig config, uint64_t seed, int regX, int regZ);

static inline ATTR(const)
Pos getLargeStructurePos(StructureConfig config, uint64_t seed, int regX, int regZ);

static inline ATTR(const)
Pos getLargeStructureChunkInRegion(StructureConfig config, uint64_t seed, int regX, int regZ);

/* Checks a chunk area, starting at (chunkX, chunkZ) with size (chunkW, chunkH)
 * for Mineshaft positions. If not NULL, positions are written to the buffer
 * 'out' up to a maximum number of 'nout'. The return value is the number of
 * chunks with Mineshafts in the area.
 */
int getMineshafts(int mc, uint64_t seed, int chunkX, int chunkZ,
        int chunkW, int chunkH, Pos *out, int nout);

// not exacly a structure
static inline ATTR(const)
int isSlimeChunk(uint64_t seed, int chunkX, int chunkZ)
{
    uint64_t rnd = seed;
    rnd += (int)(chunkX * 0x5ac0db);
    rnd += (int)(chunkX * chunkX * 0x4c1906);
    rnd += (int)(chunkZ * 0x5f24f);
    rnd += (int)(chunkZ * chunkZ) * 0x4307a7ULL;
    rnd ^= 0x3ad8025fULL;
    setSeed(&rnd, rnd);
    return nextInt(&rnd, 10) == 0;
}


//==============================================================================
// Multi-Structure-Base Checks
//==============================================================================


/* This function determines if the lower 48-bits of a seed qualify as a
 * quad-base. This implies that the four structures in the adjacent regions
 * (0,0)-(1,1) will attempt to generate close enough together to be within the
 * specified block radius of a single block position. The quad-structure can be
 * moved to a different location by applying moveStructure() to the quad-base.
 * The upper 16 bits of the seed can be chosen freely, as they do not affect
 * structure positions.
 *
 * This function is a wrapper for more specific filtering functions which can
 * be found below. Using the correct quad-base finder directly can be faster as
 * it is more likely to avoid code branching and offers more control over the
 * quality of the structure positions.
 *
 * The return value is zero if the seed is not a quad-base, and equal to the
 * radius of the enclosing sphere if it is, and can be used as a measure of
 * quality for the quad-base (smaller is better).
 */
static inline float isQuadBase(const StructureConfig sconf, uint64_t seed, int radius);

/* Determines if the specified seed qualifies as a quad-base, given a required
 * structure size. The structure size should include the actual dimensions of
 * the structure and any additional size requirements where despawning shall
 * not occur (such as fall damage drop chutes). A smaller size requirement can
 * yield more seeds and relax constraints for other structure positions.
 * (Since most structures use the same positioning algorithm with an offset,
 * this also affects restrictions on the placement of other structure types.)
 *
 * The function variants are:
 *  isQuadBaseFeature24Classic() - finds only the classic constellations
 *  isQuadBaseFeature24() - optimisation for region=32,range=24,radius=128
 *  isQuadBaseFeature() - for small features (chunkRange not a power of 2)
 *  isQuadBaseLarge() - for large structures (chunkRange not a power of 2)
 *
 * The function returns the actual block radius to the furthest block inside
 * any of the four structures or zero if the seed does not satisfy the
 * quad-base requirements.
 *
 * @sconf       : structure configuration
 * @seed        : world seed (only the lower 48-bits are relevant)
 * @ax,ay,az    : required structure size
 * @radius      : maximum radius for a sphere that encloses all four structures
 *
 * Implementation sidenote:
 * Inline actually matters here, as these functions are not small and compilers
 * generally don't want to inline them. However, these functions usually return
 * so quickly that the function call is a major contributor to the overall time.
 */

#ifdef __GNUC__
#define FORCE_INLINE __attribute__((always_inline, const))
#else
#define FORCE_INLINE
#endif

static inline FORCE_INLINE
float isQuadBaseFeature24Classic (const StructureConfig sconf, uint64_t seed);

static inline FORCE_INLINE
float isQuadBaseFeature24 (const StructureConfig sconf, uint64_t seed,
        int ax, int ay, int az);

static inline FORCE_INLINE
float isQuadBaseFeature (const StructureConfig sconf, uint64_t seed,
        int ax, int ay, int az, int radius);

static inline FORCE_INLINE
float isQuadBaseLarge (const StructureConfig sconf, uint64_t seed,
        int ax, int ay, int az, int radius);


/* Starts a multi-threaded search through all 48-bit seeds. Since this can
 * potentially be a lengthy calculation, results can be written to temporary
 * files immediately, in order to save progress in case of interruption. Seeds
 * are tested using the function 'check' which takes a 48-bit seed and a custom
 * 'data' argument. The output can be a dynamically allocated seed buffer
 * and/or a destination file [which can be loaded using loadSavedSeeds()].
 * Optionally, only a subset of the lower 20 bits are searched.
 *
 * @seedbuf     output seed buffer (nullable for file only)
 * @buflen      length of output buffer (nullable)
 * @path        output file path (nullable, also toggles temporary files)
 * @threads     number of threads to use
 * @lowBits     lower bit subset (nullable)
 * @lowBitCnt   length of lower bit subset
 * @lowBitN     number of bits in the subset values
 * @check       the testing function, should return non-zero for desired seeds
 * @data        custon data argument passed to 'check'
 *
 * Returns zero upon success.
 */
int searchAll48(
        uint64_t **         seedbuf,
        uint64_t *          buflen,
        const char *        path,
        int                 threads,
        const uint64_t *    lowBits,
        int                 lowBitCnt,
        int                 lowBitN,
        int (*check)(uint64_t s48, void *data),
        void *              data
        );

/* Finds the optimal AFK location for four structures of size (ax,ay,az),
 * located at the positions of 'p'. The AFK position is determined by looking
 * for whole block coordinates which offer the maximum number of spawning
 * spaces on the horizontal plane, which have the vertical structure height, ay,
 * inside the enclosing sphere of radius 128 blocks. If there are multiple
 * positions of this type (such as when all structures can be enclosed
 * completly inside the sphere with some tollerance) then an average of those
 * equally valid positions is returned.
 *
 * @p           : positions of the structures
 * @ax,ay,az    : size of one structure
 * @spcnt       : output number of planar spawning spaces in reach (nullable)
 *
 * Returns an optimal block-coordinate to operate a farm.
 */
Pos getOptimalAfk(Pos p[4], int ax, int ay, int az, int *spcnt);

/* Scans the seed 's48' for quad-structures in the given area of region
 * coordiantes. The search is performed for only a specific set of lower bits
 * of the transformed bases (each constellation of quad-structures is
 * considered separately).
 *
 * @sconf       : structure config
 * @radius      : radius for isQuadBase (use 128 for quad-huts)
 * @s48         : 48-bit seed to scan
 * @lowBits     : consider transformations that yield one of these lower bits
 * @lowBitCnt   : length of lower bit subset
 * @lowBitN     : number of bits in the subset values (0 < lowBitN <= 48)
 * @salt        : salt subtracted from subset values (useful for protobases)
 * @x,z,w,h     : area to scan in region coordinates (inclusive)
 * @qplist      : output region coordinates for the descovered quad-structures
 * @n           : maximum number of quad-structures to look for
 *<
 * Returns the number of quad-structures found (up to 'n').
 */
int scanForQuads(
        const StructureConfig sconf, int radius, uint64_t s48,
        const uint64_t *lowBits, int lowBitCnt, int lowBitN, uint64_t salt,
        int x, int z, int w, int h, Pos *qplist, int n);

//==============================================================================
// Checking Biomes & Biome Helper Functions
//==============================================================================


/* Get the shadow seed.
 */
static inline uint64_t getShadow(uint64_t seed)
{
    return -7379792620528906219LL - seed;
}

/* Finds a suitable pseudo-random location in the specified area.
 * This function is used to determine the positions of spawn and strongholds.
 * Warning: accurate, but slow!
 *
 * @g           : generator for Overworld biomes
 * @x,y,z       : origin for the search
 * @radius      : square 'radius' of the search
 * @validBiomes : boolean array of valid biome ids (size = 256)
 * @rnd         : random obj, initialise using setSeed(rnd, world_seed)
 * @passes      : (output) number of valid biomes passed, NULL to ignore
 */
Pos locateBiome(
        const Generator *g, int x, int y, int z, int radius,
        const char *validBiomes, uint64_t *rng, int *passes);


//==============================================================================
// Finding Strongholds and Spawn
//==============================================================================

/* Finds the approximate location of the first stronghold (+/-112 blocks),
 * which can be determined from the lower 48 bits of the world seed without
 * biome checks. If 'sh' is not NULL, it will be initialized for iteration
 * using nextStronghold() to get the accurate stronghold locations, as well as
 * the subsequent approximate stronghold positions.
 *
 * @sh      : stronghold iterator to be initialized (nullable)
 * @mc      : minecraft version (changes in 1.7, 1.9, 1.13)
 * @s48     : world seed (only 48-bit are relevant)
 *
 * Returns the approximate block position of the first stronghold.
 */
Pos initFirstStronghold(StrongholdIter *sh, int mc, uint64_t s48);

/* Performs the biome checks for the stronghold iterator and finds its accurate
 * location, as well as the approximate location of the next stronghold.
 *
 * @sh      : stronghold iteration state, holding position info
 * @g       : generator, should be initialized for Overworld generation
 *
 * Returns the number of further strongholds after this one.
 */
int nextStronghold(StrongholdIter *sh, const Generator *g);

/* Finds the spawn point in the world.
 * Warning: Slow, and may be inaccurate because the world spawn depends on
 * grass blocks!
 */
Pos getSpawn(const Generator *g);

/* Finds the approximate spawn point in the world.
 */
Pos estimateSpawn(const Generator *g);


//==============================================================================
// Validating Structure Positions
//==============================================================================

/* Performs a biome check near the specified block coordinates to determine
 * whether a structure of the given type could spawn there. You can get the
 * block positions using getStructurePos().
 * The generator, 'g', should be initialized for the correct MC version,
 * dimension and seed. The generator may be temporarily modified during the
 * function call, but will be restored upon return.
 * The 'flags' argument is optional structure specific information, such as the
 * biome variant for villages.
 */
int isViableStructurePos(int structType, Generator *g, int blockX, int blockZ, uint32_t flags);

/* Checks if the specified structure type could generate in the given biome.
 */
int isViableFeatureBiome(int mc, int structureType, int biomeID);

/* Some structures in 1.18 now only spawn if the surface is sufficiently high
 * at all four bounding box corners. This affects primarily Desert_Pyramids,
 * Jungle_Temples and Mansions.
 * Currently cubiomes does not provide overworld surface height and cannot
 * check it, but we can rule out some unlikely positions based biomes.
 *
 * This function is meant only for the 1.18 Overworld and is subject to change.
 */
int isViableStructureTerrain(int structType, Generator *g, int blockX, int blockZ);

/* End Cities require a sufficiently high surface in addition to a biome check.
 * The world seed should be applied to the EndNoise and SurfaceNoise before
 * calling this function. (Use initSurfaceNoiseEnd() for initialization.)
 */
int isViableEndCityTerrain(const EndNoise *en, const SurfaceNoise *sn,
        int blockX, int blockZ);

//==============================================================================
// Finding Properties of Structures
//==============================================================================

/* Initialises and returns a random seed used in the (16x16) chunk generation.
 * This random object is used for recursiveGenerate() which is responsible for
 * generating caves, ravines, mineshafts, and virtually all other structures.
 */
inline static
uint64_t chunkGenerateRnd(uint64_t worldSeed, int chunkX, int chunkZ)
{
    uint64_t rnd;
    setSeed(&rnd, worldSeed);
    rnd = (nextLong(&rnd) * chunkX) ^ (nextLong(&rnd) * chunkZ) ^ worldSeed;
    setSeed(&rnd, rnd);
    return rnd;
}

int getVariant(StructureVariant *sv, int structType, int mc, uint64_t seed,
        int blockX, int blockZ, int biomeID);

/* Finds the number of each type of house that generate in a village
 * (mc < MC_1_14)
 * @worldSeed      : world seed
 * @chunkX, chunkZ : 16x16 chunk position of the village origin
 * @housesOut      : output number of houses for each entry in the house type
 *                   enum (i.e this should be an array of length HOUSE_NUM)
 *
 * Returns the random object seed after finding these numbers.
 */
uint64_t getHouseList(uint64_t worldSeed, int chunkX, int chunkZ, int *housesOut);




//==============================================================================
// Seed Filters (for versions up to 1.17)
//==============================================================================


/* Creates a biome filter configuration from a given list of required and
 * excluded biomes. Biomes should not appear in both lists. Lists of length
 * zero may be passed as null.
 */
void setupBiomeFilter(
    BiomeFilter *bf,
    int mc, uint32_t flags,
    const int *required, int requiredLen,
    const int *excluded, int excludedLen,
    const int *matchany, int matchanyLen);

/* Starts to generate the specified range and checks if the biomes meet the
 * requirements of the biome filter, returning either:
 * 0 (failed),
 * 1 (okay, area is fully generated), or
 * 2 (okay, incomplete generation).
 *
 * The area will be generated inside the cache (if != NULL) but is only
 * defined if the generation was fully completed (check return value).
 * More aggressive filtering can be enabled with the flags which may yield
 * some false negatives in exchange for speed.
 *
 * The generator should be set up for the correct version, however the
 * dimension and seed will be applied internally. This will modify the
 * generator into a partially initialized state that is not valid to use
 * outside this function without re-applying a seed.
 *
 * @g           : biome generator
 * @cache       : working buffer and output (nullable)
 * @r           : range to be checked
 * @dim         : dimension (0:Overworld, -1:Nether, +1:End)
 * @seed        : world seed
 * @filter      : biome requirements to be met
 * @flags       : enables features (see below)
 * @stop        : occasional check for abort (nullable)
 */
int checkForBiomes(
        Generator         * g,
        int               * cache,
        Range               r,
        int                 dim,
        uint64_t            seed,
        const BiomeFilter * filter,
        volatile char     * stop // should be atomic, but is fine as stop flag
        );

/* Specialization of checkForBiomes() for a LayerStack, i.e. the Overworld up
 * to 1.17.
 *
 * @ls          : layered generator (will be modified!)
 * @entry       : generation entry point (setLayerSeed() may be applied here)
 * @cache       : working buffer, and output (if != NULL)
 * @seed        : world seed
 * @x,z,w,h     : requested area
 * @filter      : biomes to be checked for
 */
int checkForBiomesAtLayer(
        LayerStack        * ls,
        Layer             * entry,
        int               * cache,
        uint64_t            seed,
        int                 x,
        int                 z,
        unsigned int        w,
        unsigned int        h,
        const BiomeFilter * filter
        );

/* Checks that the area (x,z,w,h) at layer Special, scale 1:1024 contains the
 * temperature category requirements defined by 'tc' as:
 * if (tc[TEMP_CAT] >= 0) require at least this many entries of this category
 * if (tc[TEMP_CAT] <  0) avoid, there shall be no entries of this category
 * TEMP_CAT is any of:
 * Oceanic, Warm, Lush, Cold, Freeing, Special+Warm, Special+Lush, Special+Cold
 * For 1.7-1.17 only.
 */
int checkForTemps(LayerStack *g, uint64_t seed, int x, int z, int w, int h, const int tc[9]);

/* Checks if a biome may generate given a version and layer ID as entry point.
 * The supported layers are:
 * L_BIOME_256, L_BAMBOO_256, L_BIOME_EDGE_64, L_HILLS_64, L_SUNFLOWER_64,
 * L_SHORE_16, L_RIVER_MIX_4, L_OCEAN_MIX_4, L_VORONOI_1
 * (provided the version matches)
 */
int canBiomeGenerate(int layerId, int mc, int biomeID);

/* Given a biome 'id' at a generation 'layerId', this functions finds which
 * biomes may generate from it. The result is stored in the bitfields:
 * mL : for ids 0-63
 * mM : for ids 128-191
 */
void genPotential(uint64_t *mL, uint64_t *mM, int layerId, int mc, int id);

/* Gets the biomes that can generate in the given version and layer ID.
 * In contrast to canBiomeGenerate() and genPotential() it also supports 1.18+,
 * where the layerId is ignored.
 * mL : for ids 0-63
 * mM : for ids 128-191
 */
void getAvailableBiomes(uint64_t *mL, uint64_t *mM, int layerId, int mc);

//==============================================================================
// Biome Noise Finders (for 1.18+)
//==============================================================================

/**
 * Runs a gradient descent towards the minimum of the noise parameter times a
 * given factor. The algorithm is restricted to the area (x,z,w,h) and starts
 * at (i0,j0) relative to (x,z). The iteration is terminated when either
 * 1) a fix point has been reached,
 * 2) maxiter iterations have been completed,
 * 3) or the sampling position has moved more than maxrad away from (i0,j0).
 *
 * Alpha is an optimization argument that is used to determine the length of
 * large steps based on the current gradient.
 *
 * Optionally, the iteration can also call the custom function:
 *  func(data, x, z, factor*para_noise(x,z));
 *
 * The return value is the minimum value reached.
 */
double getParaDescent(const DoublePerlinNoise *para, double factor,
    int x, int z, int w, int h, int i0, int j0, int maxrad,
    int maxiter, double alpha, void *data, int (*func)(void*,int,int,double));

/**
 * Determines the value range of a climate noise parameter over the given area.
 * The sampling has scale 1:4 and sampling shift is not considered, so biomes
 * could potentially *leak* in at the boarders.
 * An optional function:
 *  func(data, x, z, climate_noise(x,z))
 * is called in each gradient descent iteration. If this function returns
 * non-zero the search is aborted, the results are undefined and a non-zero
 * error is returned.
 *
 * The results are written to pmin and pmax (which would be cast to an integer
 * during boime mapping).
 */
int getParaRange(const DoublePerlinNoise *para, double *pmin, double *pmax,
    int x, int z, int w, int h, void *data, int (*func)(void*,int,int,double));

/**
 * Gets the min/max parameter values within which a biome change can occur.
 */
const int *getBiomeParaExtremes(int mc);

/**
 * Gets the min/max possible noise parameter values at which the given biome
 * can generate. The values are in min/max pairs in order of:
 * temperature, humidity, continentalness, erosion, depth, weirdness.
 */
const int *getBiomeParaLimits(int mc, int id);

/**
 * Determines which biomes are able to generate given a set of climate
 * parameter limits. Possible biomes are marked non-zero in the 'ids'.
 */
void getPossibleBiomesForLimits(char ids[256], int mc, int limits[6][2]);


//==============================================================================
// Implementaions for Functions that Ideally Should be Inlined
//==============================================================================


static inline ATTR(const)
Pos getFeatureChunkInRegion(StructureConfig config, uint64_t seed, int regX, int regZ)
{
    /*
    // Vanilla like implementation.
    setSeed(&seed, regX*341873128712 + regZ*132897987541 + seed + config.salt);

    Pos pos;
    pos.x = nextInt(&seed, 24);
    pos.z = nextInt(&seed, 24);
    */
    Pos pos;
    const uint64_t K = 0x5deece66dULL;
    const uint64_t M = (1ULL << 48) - 1;
    const uint64_t b = 0xb;

    // set seed
    seed = seed + regX*341873128712ULL + regZ*132897987541ULL + config.salt;
    seed = (seed ^ K);
    seed = (seed * K + b) & M;

    uint64_t r = config.chunkRange;
    if (r & (r-1))
    {
        pos.x = (int)(seed >> 17) % r;
        seed = (seed * K + b) & M;
        pos.z = (int)(seed >> 17) % r;
    }
    else
    {
        // Java RNG treats powers of 2 as a special case.
        pos.x = (int)((r * (seed >> 17)) >> 31);
        seed = (seed * K + b) & M;
        pos.z = (int)((r * (seed >> 17)) >> 31);
    }

    return pos;
}

static inline ATTR(const)
Pos getFeaturePos(StructureConfig config, uint64_t seed, int regX, int regZ)
{
    Pos pos = getFeatureChunkInRegion(config, seed, regX, regZ);

    pos.x = (int)(((uint64_t)regX*config.regionSize + pos.x) << 4);
    pos.z = (int)(((uint64_t)regZ*config.regionSize + pos.z) << 4);
    return pos;
}

static inline ATTR(const)
Pos getLargeStructureChunkInRegion(StructureConfig config, uint64_t seed, int regX, int regZ)
{
    Pos pos;
    const uint64_t K = 0x5deece66dULL;
    const uint64_t M = (1ULL << 48) - 1;
    const uint64_t b = 0xb;

    //TODO: power of two chunk ranges...

    // set seed
    seed = seed + regX*341873128712ULL + regZ*132897987541ULL + config.salt;
    seed = (seed ^ K);

    seed = (seed * K + b) & M;
    pos.x = (int)(seed >> 17) % config.chunkRange;
    seed = (seed * K + b) & M;
    pos.x += (int)(seed >> 17) % config.chunkRange;

    seed = (seed * K + b) & M;
    pos.z = (int)(seed >> 17) % config.chunkRange;
    seed = (seed * K + b) & M;
    pos.z += (int)(seed >> 17) % config.chunkRange;

    pos.x >>= 1;
    pos.z >>= 1;

    return pos;
}

static inline ATTR(const)
Pos getLargeStructurePos(StructureConfig config, uint64_t seed, int regX, int regZ)
{
    Pos pos = getLargeStructureChunkInRegion(config, seed, regX, regZ);

    pos.x = (int)(((uint64_t)regX*config.regionSize + pos.x) << 4);
    pos.z = (int)(((uint64_t)regZ*config.regionSize + pos.z) << 4);
    return pos;
}



static ATTR(const)
float getEnclosingRadius(
    int x0, int z0, int x1, int z1, int x2, int z2, int x3, int z3,
    int ax, int ay, int az, int reg, int gap)
{
    // convert chunks to blocks
    x0 = (x0 << 4);
    z0 = (z0 << 4);
    x1 = ((reg+x1) << 4) + ax;
    z1 = ((reg+z1) << 4) + az;
    x2 = ((reg+x2) << 4) + ax;
    z2 = (z2 << 4);
    x3 = (x3 << 4);
    z3 = ((reg+z3) << 4) + az;

    int sqrad = 0x7fffffff;

    // build the inner rectangle containing the center point
    int cbx0 = (x1 > x2 ? x1 : x2) - gap;
    int cbz0 = (z1 > z3 ? z1 : z3) - gap;
    int cbx1 = (x0 < x3 ? x0 : x3) + gap;
    int cbz1 = (z0 < z2 ? z0 : z2) + gap;
    int x, z;

    // brute force the ideal center position
    for (z = cbz0; z <= cbz1; z++)
    {
        for (x = cbx0; x <= cbx1; x++)
        {
            int sq = 0;
            int s;
            s = (x-x0)*(x-x0) + (z-z0)*(z-z0); if (s > sq) sq = s;
            s = (x-x1)*(x-x1) + (z-z1)*(z-z1); if (s > sq) sq = s;
            s = (x-x2)*(x-x2) + (z-z2)*(z-z2); if (s > sq) sq = s;
            s = (x-x3)*(x-x3) + (z-z3)*(z-z3); if (s > sq) sq = s;
            if (sq < sqrad)
                sqrad = sq;
        }
    }

    return sqrad < 0x7fffffff ? sqrtf(sqrad + ay*ay/4.0f) : 0xffff;
}


static inline float isQuadBase(const StructureConfig sconf, uint64_t seed, int radius)
{
    switch(sconf.structType)
    {
    case Swamp_Hut:
        if (radius == 128)
            return isQuadBaseFeature24(sconf, seed, 7+1, 7+1, 9+1);//7+1, 7+43+1, 9+1);
        else
            return isQuadBaseFeature(sconf, seed, 7+1, 7+1, 9+1, radius);
    case Desert_Pyramid:
    case Jungle_Pyramid:
    case Igloo:
    case Village:
        // nothing special spawns here, why would you want these?
        if (radius == 128)
            return isQuadBaseFeature24(sconf, seed, 0, 0, 0);
        else
            return isQuadBaseFeature(sconf, seed, 0, 0, 0, radius);

    case Outpost:
        // Outposts are tricky. They require an additional 1 in 5 PRNG pass to
        // generate and no village nearby. Also perfect quad-outposts don't
        // exist as they are too large, given that the generation point will
        // always be 8 chunks apart. However, the watchtower can be offset to
        // the generation attempt by a chunk or two (TODO: investivgate this!).
        return isQuadBaseFeature(sconf, seed, 72, 54, 72, radius);

    case Monument:
        return isQuadBaseLarge(sconf, seed, 58, 23, 58, radius);

    //case Mansion:
    //case Ocean_Ruin:
    //case Shipwreck:
    //case Ruined_Portal:

    default:
        fprintf(stderr, "isQuadBase: not implemented for structure type %d\n",
                sconf.structType);
        exit(-1);
    }

    return 0;
}

// optimised version for regionSize=32,chunkRange=24,radius=128
static inline ATTR(always_inline, const)
float isQuadBaseFeature24(const StructureConfig sconf, uint64_t seed,
        int ax, int ay, int az)
{
    seed += sconf.salt;
    uint64_t s00 = seed;
    uint64_t s11 = 341873128712ULL + 132897987541ULL + seed;
    const uint64_t K = 0x5deece66dULL;

    int x0, z0, x1, z1, x2, z2, x3, z3;
    int x, z;

    // check that the two structures in the opposing diagonal quadrants are
    // close enough together
    s00 ^= K;
    JAVA_NEXT_INT24(s00, x0); if L(x0 < 20) return 0;
    JAVA_NEXT_INT24(s00, z0); if L(z0 < 20) return 0;

    s11 ^= K;
    JAVA_NEXT_INT24(s11, x1); if L(x1 > x0-20) return 0;
    JAVA_NEXT_INT24(s11, z1); if L(z1 > z0-20) return 0;

    x = x1 + 32 - x0;
    z = z1 + 32 - z0;
    if (x*x + z*z > 255)
        return 0;

    uint64_t s01 = 341873128712ULL + seed;
    uint64_t s10 = 132897987541ULL + seed;

    s01 ^= K;
    JAVA_NEXT_INT24(s01, x2); if L(x2 >= 4) return 0;
    JAVA_NEXT_INT24(s01, z2); if L(z2 < 20) return 0;

    s10 ^= K;
    JAVA_NEXT_INT24(s10, x3); if L(x3 < 20) return 0;
    JAVA_NEXT_INT24(s10, z3); if L(z3 >= 4) return 0;

    x = x2 + 32 - x3;
    z = z3 + 32 - z2;
    if (x*x + z*z > 255)
        return 0;

    // only approx. 1 in 100M seeds makes it here, now we have to determine if
    // there is a sphere, centered on a block, which is in range of all four
    // structures

    float sqrad = getEnclosingRadius(x0,z0,x1,z1,x2,z2,x3,z3,ax,ay,az,32,128);
    return sqrad < 128 ? sqrad : 0;
}

// variant of isQuadBaseFeature24 which finds only the classic constellations
static inline ATTR(always_inline, const)
float isQuadBaseFeature24Classic(const StructureConfig sconf, uint64_t seed)
{
    seed += sconf.salt;
    uint64_t s00 = seed;
    uint64_t s11 = 341873128712ULL + 132897987541ULL + seed;
    const uint64_t K = 0x5deece66dULL;
    int p;

    // check that the two structures in the opposing diagonal quadrants are
    // close enough together
    s00 ^= K;
    JAVA_NEXT_INT24(s00, p); if L(p < 22) return 0;
    JAVA_NEXT_INT24(s00, p); if L(p < 22) return 0;

    s11 ^= K;
    JAVA_NEXT_INT24(s11, p); if L(p > 1) return 0;
    JAVA_NEXT_INT24(s11, p); if L(p > 1) return 0;

    uint64_t s01 = 341873128712ULL + seed;
    uint64_t s10 = 132897987541ULL + seed;

    s01 ^= K;
    JAVA_NEXT_INT24(s01, p); if L(p > 1) return 0;
    JAVA_NEXT_INT24(s01, p); if L(p < 22) return 0;

    s10 ^= K;
    JAVA_NEXT_INT24(s10, p); if L(p < 22) return 0;
    JAVA_NEXT_INT24(s10, p); if L(p > 1) return 0;

    return 1; // should actually return one of 122.781311 or 127.887650
}

static inline ATTR(always_inline, const)
float isQuadBaseFeature(const StructureConfig sconf, uint64_t seed,
        int ax, int ay, int az, int radius)
{
    seed += sconf.salt;
    uint64_t s00 = seed;
    uint64_t s11 = 341873128712ULL + 132897987541ULL + seed;
    const uint64_t M = (1ULL << 48) - 1;
    const uint64_t K = 0x5deece66dULL;
    const uint64_t b = 0xb;

    int x0, z0, x1, z1, x2, z2, x3, z3;
    int x, z;

    const int R = sconf.regionSize;
    const int C = sconf.chunkRange;
    int cd = radius/8;
    int rm = R - (int)sqrtf(cd*cd - (R-C+1)*(R-C+1));

    uint64_t s;

    s = s00 ^ K;
    s = (s * K + b) & M; x0 = (int)(s >> 17) % C; if L(x0 <= rm) return 0;
    s = (s * K + b) & M; z0 = (int)(s >> 17) % C; if L(z0 <= rm) return 0;

    s = s11 ^ K;
    s = (s * K + b) & M; x1 = (int)(s >> 17) % C; if L(x1 >= x0-rm) return 0;
    s = (s * K + b) & M; z1 = (int)(s >> 17) % C; if L(z1 >= z0-rm) return 0;

    // check that the two structures in the opposing diagonal quadrants are
    // close enough together

    x = x1 + R - x0;
    z = z1 + R - z0;
    if L(x*x + z*z > cd*cd)
        return 0;

    uint64_t s01 = 341873128712ULL + seed;
    uint64_t s10 = 132897987541ULL + seed;

    s = s01 ^ K;
    s = (s * K + b) & M; x2 = (int)(s >> 17) % C; if L(x2 >= C-rm) return 0;
    s = (s * K + b) & M; z2 = (int)(s >> 17) % C; if L(z2 <= rm) return 0;

    s = s10 ^ K;
    s = (s * K + b) & M; x3 = (int)(s >> 17) % C; if L(x3 <= rm) return 0;
    s = (s * K + b) & M; z3 = (int)(s >> 17) % C; if L(z3 >= C-rm) return 0;

    x = x2 + R - x3;
    z = z3 + R - z2;
    if L(x*x + z*z > cd*cd)
        return 0;

    float sqrad = getEnclosingRadius(
        x0,z0,x1,z1,x2,z2,x3,z3,ax,ay,az,sconf.regionSize,radius);
    return sqrad < radius ? sqrad : 0;
}


static inline ATTR(always_inline, const)
float isQuadBaseLarge(const StructureConfig sconf, uint64_t seed,
        int ax, int ay, int az, int radius)
{
    // Good quad-monument bases are very rare indeed and the search takes much
    // longer since it cannot be abbreviated by the low-20-bit method. For a
    // complete list of bases see the implementation of cubiomes-viewer.

    const uint64_t M = (1ULL << 48) - 1;
    const uint64_t K = 0x5deece66dULL;
    const uint64_t b = 0xb;

    seed += sconf.salt;
    uint64_t s00 = seed;
    uint64_t s01 = 341873128712ULL + seed;
    uint64_t s10 = 132897987541ULL + seed;
    uint64_t s11 = 341873128712ULL + 132897987541ULL + seed;

    // p1 = nextInt(range); p2 = nextInt(range); pos = (p1+p2)>>1
    const int R = sconf.regionSize;
    const int C = sconf.chunkRange;
    int rm = (int)(2 * R + ((ax<az?ax:az) - 2*radius + 7) / 8);

    uint64_t s;
    int p;
    int x0,z0,x1,z1,x2,z2,x3,z3;

    s = s00 ^ K;
    s = (s * K + b) & M; p =  (int)(s >> 17) % C;
    s = (s * K + b) & M; p += (int)(s >> 17) % C; if L(p <= rm) return 0;
    x0 = p;
    s = (s * K + b) & M; p =  (int)(s >> 17) % C;
    s = (s * K + b) & M; p += (int)(s >> 17) % C; if L(p <= rm) return 0;
    z0 = p;

    s = s11 ^ K;
    s = (s * K + b) & M; p =  (int)(s >> 17) % C;
    s = (s * K + b) & M; p += (int)(s >> 17) % C; if L(p > x0-rm) return 0;
    x1 = p;
    s = (s * K + b) & M; p =  (int)(s >> 17) % C;
    s = (s * K + b) & M; p += (int)(s >> 17) % C; if L(p > z0-rm) return 0;
    z1 = p;

    s = ((x1-x0)>>1)*((x1-x0)>>1) + ((z1-z0)>>1)*((z1-z0)>>1);
    if (s > (uint64_t)4*radius*radius)
        return 0;

    s = s01 ^ K;
    s = (s * K + b) & M; p =  (int)(s >> 17) % C;
    s = (s * K + b) & M; p += (int)(s >> 17) % C; if L(p > x0-rm) return 0;
    x2 = p;
    s = (s * K + b) & M; p =  (int)(s >> 17) % C;
    s = (s * K + b) & M; p += (int)(s >> 17) % C; if L(p <= rm) return 0;
    z2 = p;

    s = s10 ^ K;
    s = (s * K + b) & M; p =  (int)(s >> 17) % C;
    s = (s * K + b) & M; p += (int)(s >> 17) % C; if L(p <= rm) return 0;
    x3 = p;
    s = (s * K + b) & M; p =  (int)(s >> 17) % C;
    s = (s * K + b) & M; p += (int)(s >> 17) % C; if L(p > z0-rm) return 0;
    z3 = p;

    float sqrad = getEnclosingRadius(
            x0>>1,z0>>1, x1>>1,z1>>1, x2>>1,z2>>1, x3>>1,z3>>1,
            ax,ay,az, sconf.regionSize, radius);
    return sqrad < radius ? sqrad : 0;
}


#ifdef __cplusplus
}
#endif

#endif /* FINDERS_H_ */
