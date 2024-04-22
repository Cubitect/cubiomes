#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include "cubiomes.h"


// Algorithms written by Kriss007
// Logic written by Colin_Henry
// DM colin_henry on discord for questions

// ----------
// macros
// ----------

#define ref(X) const X* const
#define NOISE_REGION_SIZE 6

// ----------
// data types
// ----------

typedef struct {
    double x;
    double z;
} Vec;

typedef struct {
    int minX;
    int minZ;
    int maxX;
    int maxZ;
} BlockArea;

typedef struct {
    Pos3 pos;
    double r;
} EndIsland;

typedef struct {
    int size;
    EndIsland first;
    EndIsland second;
} EndIslandPair;

// this struct is a holder for all the noise column values used in terrain generation
// for a single chunk, only the first 3x3 noise columns are used
// for getTallestBlock, all 6x6 cols. get sampled and stored
typedef struct {
    double noisecolumn[NOISE_REGION_SIZE][NOISE_REGION_SIZE][33];
} NoiseRegion;

typedef struct {
    SurfaceNoise surfaceNoise;
    EndNoise endNoise;
    NoiseRegion noiseRegion;
    uint64_t seed;
} EndTerrainNoise;

// -----------------------
// inline functs, helpers
// -----------------------

inline int min(const int a, const int b) 
{ return a < b ? a : b; }

inline int max(const int a, const int b) 
{ return a > b ? a : b; }

inline void setPos(Pos3* const pos, const int x, const int y, const int z) 
{
    pos->x = x;
    pos->y = y;
    pos->z = z;
}

inline uint64_t getPosAsLong(Pos3* const pos) 
{
    // this encodes the x,y,z coordinates directly as a Java long
    const int PACKED_XZ_LENGTH = 26;
    const int PACKED_Y_LENGTH = 64 - 2*PACKED_XZ_LENGTH;
    const uint64_t PACKED_XZ_MASK = (1ULL << PACKED_XZ_LENGTH) - 1;
    const uint64_t PACKED_Y_MASK = (1ULL << PACKED_Y_LENGTH) - 1;

    uint64_t l = 0ULL;
    l |= ((uint64_t)pos->x & PACKED_XZ_MASK) << (PACKED_XZ_LENGTH + PACKED_Y_LENGTH);
    l |= ((uint64_t)pos->y & PACKED_Y_MASK) << 0;
    return l |= ((uint64_t)pos->z & PACKED_XZ_MASK) << PACKED_Y_LENGTH;
}

#define DEBUGGING_ENABLED false
#define DEBUG if(DEBUGGING_ENABLED)printf

// ----------
// constants
// ----------

const double RD = 0.35;         // WARN: magic number, derived empirically
const double PI = 3.14159265359;
const Pos3 NULL_POS = {-1,-1,-1};
const BlockArea FULL_AREA = {INT_MIN, INT_MIN, INT_MAX, INT_MAX};
const int MC = MC_1_16_1;

// ---------------------------------
// geometry & intersection functions
// ---------------------------------

bool blockIsWithinIsland(ref(EndIsland) island, const int x, const int z) 
{
    // check if the given blockpos intersects the island
    const double r = island->r + RD; 
    const double dx = abs(island->pos.x - x) - 0.5; // WARN more magic numbers
    const double dz = abs(island->pos.z - z) - 0.5;

    return dx * dx + dz * dz <= r * r;
}


bool islandIntersects(ref(EndIsland) island, ref(BlockArea) area) 
{
    // check possibility of intersection by treating the island's shape like a square
    bool canIntersectX = (island->pos.x + island->r + RD) /*island max X*/ > area->minX && (island->pos.x - island->r - RD) /*island min X*/ < area->maxX;
    bool canIntersectZ = (island->pos.z + island->r + RD) /*island max Z*/ > area->minZ && (island->pos.z - island->r - RD) /*island min Z*/ < area->maxZ;
    
    //DEBUG("### INTERSECTION  area %d %d %d %d\n", minX, minZ, maxX, maxZ);
    //DEBUG("### INTERSECTION  island %d %d\n", island->pos.x, island->pos.z);
    if (!canIntersectX || !canIntersectZ)
        return false;
    
    // handle circular shape of the island:
    // if the island's center is within the x or z range of the area, then the island will intersect the area.
    bool centerIntersectsX = island->pos.x > area->minX && island->pos.x < area->maxX;
    bool centerIntersectsZ = island->pos.z > area->minZ && island->pos.z < area->maxZ;
    if (centerIntersectsX || centerIntersectsZ)
        return true;

    // handle edge case (island positioned diagonally relative to the area):
    // check if any of the area's corner are within the island.
    if (blockIsWithinIsland(island, area->minX, area->minZ)) return true;
    if (blockIsWithinIsland(island, area->minX, area->maxZ)) return true;
    if (blockIsWithinIsland(island, area->maxX, area->minZ)) return true;
    if (blockIsWithinIsland(island, area->maxX, area->maxZ)) return true;

    // The island does not intersect the area
    return false;
}


bool anyIslandIntersectsChunk(ref(EndIslandPair) islandPair, int chunkX, int chunkZ) 
{
    BlockArea chunkArea = {chunkX*16, chunkZ*16, chunkX*16+15, chunkZ*16+15};

    if (islandPair->size > 0 && islandIntersects(&(islandPair->first), &chunkArea))
        return true;
    if (islandPair->size > 1 && islandIntersects(&(islandPair->second), &chunkArea))
        return true;

    return false;
}

// adds islands that intersect the BlockArea [area] from [islandPair] to the array [arr]
void addIntersectingIslands(EndIsland* arr, int* arrSize, ref(EndIslandPair) islandPair, ref(BlockArea) area, int minHeight) 
{
    if (islandPair->size > 0) 
    {
        //DEBUG(">> ISLAND #1\n");
        if (islandPair->first.pos.y >= minHeight && islandIntersects(&(islandPair->first), area))
            arr[(*arrSize)++] = islandPair->first;

        if (islandPair->size > 1) 
        {
            //DEBUG(">> ISLAND #2\n");
            if (islandPair->second.pos.y >= minHeight && islandIntersects(&(islandPair->second), area))
                arr[(*arrSize)++] = islandPair->second;
        }
    }
}


// ------------------------------------
// small end islands, algorithm helpers
// ------------------------------------

uint64_t getPopSeed(uint64_t ws, int x, int z)
{
    uint64_t s;
    uint64_t a, b;

    setSeed(&s, ws);
    a = nextLong(&s) | 1;
    b = nextLong(&s) | 1;

    return (x * a + z * b) ^ ws;
}


EndIslandPair generateIslands(Generator* endBiomeGenerator, uint64_t lower48, int xCoordinate, int zCoordinate)
{
    EndIslandPair islandPair = {0}; // empty initializer -> all fields get set to 0

    int biome = getBiomeAt(endBiomeGenerator, 4, (xCoordinate >> 2) + 2, 2, (zCoordinate >> 2) + 2); // getNoiseBiome()
    if (biome != small_end_islands)
        return islandPair;
    
    uint64_t rng = 0;
    setSeed(&rng, getPopSeed(lower48, xCoordinate, zCoordinate));
    int rngResult = nextInt(&rng, 14); // Checking for 1/14 chance to generate
    if (rngResult != 0) // No small island in that chunk
        return islandPair;

    // At least 1 small island in that chunk
    islandPair.size = 1;
    islandPair.first.pos.x = nextInt(&rng, 16) + xCoordinate;
    islandPair.first.pos.y = nextInt(&rng, 16) + 55;
    islandPair.first.pos.z = nextInt(&rng, 16) + zCoordinate;
        
    rngResult = nextInt(&rng, 4);
    if (rngResult == 0) // 2nd small island generated
    {
        islandPair.size = 2;
        islandPair.second.pos.x = nextInt(&rng, 16) + xCoordinate;
        islandPair.second.pos.y = nextInt(&rng, 16) + 55;
        islandPair.second.pos.z = nextInt(&rng, 16) + zCoordinate;
    }

    islandPair.first.r = nextInt(&rng, 3) + 4.0;

    if (islandPair.size == 2) // gen 1st island only when the second one exists
    {
        double r1 = islandPair.first.r; // preserve the original value of radius1
        for (int yOffset = 0; r1 > 0.5; --yOffset) 
        {
            // don't care about block placement
            // shrink the radius randomly, either by 0.5 or 1.5
            r1 -= nextInt(&rng, 2) + 0.5;
        }

        islandPair.second.r = nextInt(&rng, 3) + 4.0;
        // dont care about the lower layers of the second island
    }

    return islandPair;
}

// returns the block position within an island prioritized by the source code impl. of getTallestBlock()
Pos3 getTopPriorityBlock(ref(EndIsland) island, ref(BlockArea) area) {
    int d = ceil(island->r + RD);
    int islandMinX = max(island->pos.x - d, area->minX);
    int islandMinZ = max(island->pos.z - d, area->minZ);
    int islandMaxX = min(island->pos.x + d, area->maxX);
    int islandMaxZ = min(island->pos.z + d, area->maxZ);

    bool halted = false;
    for (int x = islandMinX; !halted && x <= islandMaxX; x++) 
    {
        for (int z = islandMinZ; !halted && z <= islandMaxZ; z++) 
        {
            if (blockIsWithinIsland(island, x, z)) 
            {
                Pos3 topPriorityBlock = {x, island->pos.y, z};
                return topPriorityBlock;
            }
        }
    }

    return NULL_POS;
}

// returns the result of getTallestBlock() for small end islands within a 33x33 area around the [center] block
Pos3 getIslandTallestBlock(Generator* endBiomeGenerator, uint64_t lower48, ref(Pos3) center, const int maxSurfaceHeight) 
{
    // finds the maximum height (and position where it occurs) of islands 
    // within a 33x33 area around the center block

    BlockArea area = {center->x - 16, center->z - 16, center->x + 16, center->z + 16};
    const int maxR = 7;

    // add all the islands that spawn and intersect the area to the array
    EndIsland islandArray[18];
    int arraySize = 0;

    Pos minChunk = {floor((area.minX-maxR) / 16.0), floor((area.minZ-maxR) / 16.0)};
    Pos maxChunk = {floor((area.maxX+maxR) / 16.0), floor((area.maxZ+maxR) / 16.0)};

    for (int cx = minChunk.x; cx <= maxChunk.x; cx++) 
    {
        for (int cz = minChunk.z; cz <= maxChunk.z; cz++) 
        {
            //DEBUG("> CHUNK POS   %d %d\n", cx, cz);
            EndIslandPair islandInfo = generateIslands(endBiomeGenerator, lower48, cx*16, cz*16);
            addIntersectingIslands(islandArray, &arraySize, &islandInfo, &area, maxSurfaceHeight);
        }
    }
    //DEBUG("ISLAND ARRAY SIZE   %d\n", arraySize);

    // if no islands were added, break
    if (arraySize == 0)
        return NULL_POS;

    // find the maximum height of all the islands
    int maxHeight = 0;
    for (int i = 0; i < arraySize; i++) 
    {
        if (islandArray[i].pos.y > maxHeight)
            maxHeight = islandArray[i].pos.y;
    }

    // The algorithm:
    // for each island with the maximum height, find the block with the highest priority
    // which lies within the target area. Compare with current block as follows:
    // if result is NULL_POS, replace
    // if x position smaller than result's, replace
    // if x positions equal and z position smaller than result's, replace
    Pos3 resultPos = NULL_POS;

    for (int i = 0; i < arraySize; i++) 
    {
        const EndIsland* islandPtr = &(islandArray[i]);
        //DEBUG("ISLAND  %d %d %d\n", islandPtr->pos.x, islandPtr->pos.y, islandPtr->pos.z);
        if (islandPtr->pos.y != maxHeight)
            continue;

        // get top priority block & compare
        Pos3 topPriorityBlock = getTopPriorityBlock(islandPtr, &area);
        if (resultPos.y == -1 || resultPos.x > topPriorityBlock.x || (resultPos.x == topPriorityBlock.x && resultPos.z > topPriorityBlock.z))
            setPos(&resultPos, topPriorityBlock.x, maxHeight, topPriorityBlock.z);
    }

    return resultPos;
}


bool chunkHasIslandBlocks(ref(Pos) chunkPos, Generator* endBiomeGenerator, uint64_t lower48) 
{
    int chunkX = chunkPos->x;
    int chunkZ = chunkPos->z;

    // most likely case, check it first
    EndIslandPair islandPair = generateIslands(endBiomeGenerator, lower48, chunkX * 16, chunkZ * 16);
    if(islandPair.size > 0)
        return true;

    // check all surrounding islands
    for (int cx = chunkX-1; cx <= chunkX+1; cx++) 
    {
        for (int cz = chunkZ-1; cz <= chunkZ+1; cz++) 
        {
            if (cx == chunkX && cz == chunkZ) 
                continue;
            
            islandPair = generateIslands(endBiomeGenerator, lower48, cx * 16, cz * 16);
            if (anyIslandIntersectsChunk(&islandPair, chunkX, chunkZ))
                return true;
        }
    }

    return false;
}


// ---------------------------------
// optimized terrain gen functions
// ---------------------------------

void initEndTerrainNoise(EndTerrainNoise* const etn, uint64_t lower48) 
{
    etn->seed = lower48;

    initSurfaceNoise(&(etn->surfaceNoise), DIM_END, lower48);
    setEndSeed(&(etn->endNoise), MC, lower48);

    NoiseRegion nreg = {0};
    etn->noiseRegion = nreg;
}


void sampleColumn(EndTerrainNoise* const etn, int cellX, int cellZ, const int minX, const int minZ)
{
    const int minCellX = minX >> 3;
    const int minCellZ = minZ >> 3;

    sampleNoiseColumnEnd(
        etn->noiseRegion.noisecolumn[cellX][cellZ], &(etn->surfaceNoise), &(etn->endNoise), // noise structs
        cellX + minCellX, cellZ + minCellZ, // noise column coords
        0, 32 // y0, y1
    );
}


void sampleNoiseColumnsCached(EndTerrainNoise* const etn, bool noiseCalculated[][NOISE_REGION_SIZE], int cellX, int cellZ, const int minX, const int minZ)
{
    // sample all noise columns in area (cellX, cellZ), (cellX+1, cellZ+1) that haven't been sampled before
    for (int cellDX = 0; cellDX <= 1; cellDX++)
    {
        for (int cellDZ = 0; cellDZ <= 1; cellDZ++)
        {
            if (!noiseCalculated[cellX + cellDX][cellZ + cellDZ]) 
            {
                sampleColumn(etn, cellX + cellDX, cellZ + cellDZ, minX, minZ);
                // DEBUG("sampled noise: min block %d %d,  cell: %d %d\n", minX, minZ, cellX + cellDX, cellZ + cellDZ);
                noiseCalculated[cellX + cellDX][cellZ + cellDZ] = true;
            }
        }
    }
}


int getHeightAt(EndTerrainNoise* const etn, int arrayCellX, int arrayCellZ, int x, int z)
{
    double dx = (x & 7) / 8.0;
    double dz = (z & 7) / 8.0;

    return getSurfaceHeight(
        etn->noiseRegion.noisecolumn[arrayCellX][arrayCellZ],   etn->noiseRegion.noisecolumn[arrayCellX][arrayCellZ+1],
        etn->noiseRegion.noisecolumn[arrayCellX+1][arrayCellZ], etn->noiseRegion.noisecolumn[arrayCellX+1][arrayCellZ+1],
        0, 32, 4, dx, dz
    );
}


bool chunkHasTerrainBlocks(ref(Pos) chunkPos, EndTerrainNoise* const etn, bool minHeight30) 
{
    const int minX = chunkPos->x << 4;
    const int minZ = chunkPos->z << 4;
    bool noiseCalculated[NOISE_REGION_SIZE][NOISE_REGION_SIZE] = {0};

    // calculate all the surface heights based on generated noise, return on good result
    // notice that noise sampling is cached and done when it's actually needed, which grants a decent
    // speedup when checking chunks that have lots of blocks in them
    for (int x = minX; x <= minX + 15; x++)
    {
        for (int z = minZ; z <= minZ + 15; z++)
        {
            const int arrayCellX = (x >> 3) - (minX >> 3);
            const int arrayCellZ = (z >> 3) - (minZ >> 3);
            //DEBUG("$$ CELL %d %d\n", cellX, cellZ);

            sampleNoiseColumnsCached(etn, noiseCalculated, arrayCellX, arrayCellZ, minX, minZ);

            // get the surface height using the calculated noise vals
            int height = getHeightAt(etn, arrayCellX, arrayCellZ, x, z);
            if (height > 0 && (!minHeight30 || height >= 30)) 
                return true;
        }
    }

    //DEBUG("* EMPTY CHUNK");
    return false;
}


Pos3 getTerrainTallestBlock(ref(Pos3) center, EndTerrainNoise* const etn) 
{
    BlockArea searchArea = {center->x - 16, center->z - 16, center->x + 16, center->z + 16};
    Pos3 terrainMax = NULL_POS;

    // precalculating noise
    // no need to do cached on-demand sampling cause we're checking all the blocks in the area anyway
    for (int i = 0; i < NOISE_REGION_SIZE; i++)
        for (int j = 0; j < NOISE_REGION_SIZE; j++)
            sampleColumn(etn, i, j, searchArea.minX, searchArea.minZ);
    
    int minCellX = searchArea.minX >> 3;
    int minCellZ = searchArea.minZ >> 3;

    for (int x = searchArea.minX; x <= searchArea.maxX; x++)
    {
        for (int z = searchArea.minZ; z <= searchArea.maxZ; z++)
        {
            int arrayCellX = (x >> 3) - minCellX;
            int arrayCellZ = (z >> 3) - minCellZ;

            // get the surface height using the calculated noise vals
            int height = getHeightAt(etn, arrayCellX, arrayCellZ, x, z);
            if (height > terrainMax.y) 
                setPos(&terrainMax, x, height, z);
        }
    }

    return terrainMax;
}


// ---------------------------------
// gateway generation algorithm
// ---------------------------------

Pos getMainGateway(uint64_t lower48) // Fucntion that finds the first gateway spawned upon dragon death
{
    // hardcoded gateway positions fix a lot of the issues with floating point arithmetic
    static const Pos gateways[20] = {
        {96,0}, {91,29}, {77,56}, {56,77}, {29,91},
        {-1,96}, {-30,91}, {-57,77}, {-78,56}, {-92,29},
        {-96,-1}, {-92,-30}, {-78,-57}, {-57,-78}, {-30,-92},
        {0,-96}, {29,-92}, {56,-78}, {77,-57}, {91,-30}
    };

    uint64_t rng = 0;
	setSeed(&rng, lower48);
	int ix = nextInt(&rng, 20);
    return gateways[ix];
}

Pos3 linkedGateway(uint64_t lower48) // Main fnction that returns linked gateway position (the one in the outer end)
{
	Pos mainGateway = getMainGateway(lower48);
    DEBUG("MAIN GATEWAY   %d %d\n", mainGateway.x, mainGateway.z);

    double rootlen = sqrt(mainGateway.x * mainGateway.x + mainGateway.z * mainGateway.z);

    Vec normalizedVector = {mainGateway.x / rootlen, mainGateway.z / rootlen};
    DEBUG("NORMALIZED VEC   %lf %lf\n", normalizedVector.x, normalizedVector.z);

    Vec gatewayVector = {normalizedVector.x * 1024.0, normalizedVector.z * 1024.0};
    Vec incrementVector = {normalizedVector.x * 16.0, normalizedVector.z * 16.0};
    DEBUG("GATEWAY VEC   %lf %lf\n", gatewayVector.x, gatewayVector.z);

    // biome source
    Generator endBiomeGenerator;
    setupGenerator(&endBiomeGenerator, MC, 0);
    applySeed(&endBiomeGenerator, 1, lower48);

    // terrain source
    EndTerrainNoise etn;
    initEndTerrainNoise(&etn, lower48);

    // skip non-empty chunks
    for (int n = 0; n < 16; n++) //Checking towards the main end island to see if there are blocks (in case the original vector plopped me in the middle of a huge island)
    {
        Pos chunkPos = {floor(gatewayVector.x / 16), floor(gatewayVector.z / 16)};

        if (!chunkHasIslandBlocks(&chunkPos, &endBiomeGenerator, lower48) && !chunkHasTerrainBlocks(&chunkPos, &etn, false)) 
            break;
        
        DEBUG("SKIP non-empty %lf %lf\n", gatewayVector.x, gatewayVector.z);
        gatewayVector.x -= incrementVector.x; // Going towards the main island
        gatewayVector.z -= incrementVector.z;
    }
    
    // skip empty chunks
    for (int n = 0; n < 16; n++) //Checking away from the main end island to see if there are blocks (in case the original vector plopped me in the middle of the void)
    {
        Pos chunkPos = {floor(gatewayVector.x / 16), floor(gatewayVector.z / 16)};

        if (chunkHasIslandBlocks(&chunkPos, &endBiomeGenerator, lower48)) 
            break;
        if (chunkHasTerrainBlocks(&chunkPos, &etn, false)) 
            break;
        
        DEBUG("SKIP empty %lf %lf\n", gatewayVector.x, gatewayVector.z);
        gatewayVector.x += incrementVector.x; // Going away from the main island
        gatewayVector.z += incrementVector.z;
    }

    // findValidSpawnInChunk substitute
    Pos chunkPos = {floor(gatewayVector.x / 16), floor(gatewayVector.z / 16)};
    bool chunkHasValidSpawn = chunkHasIslandBlocks(&chunkPos, &endBiomeGenerator, lower48) 
        || chunkHasTerrainBlocks(&chunkPos, &etn, true);
    
    if (!chunkHasValidSpawn) 
    {
        // 75 will most likely be the highest block around, we can get a really good approximation of the position just
        // by taking the top-priority block of the created island
        Pos3 blockPos = {floor(gatewayVector.x + 0.5), 75, floor(gatewayVector.z + 0.5)};
        DEBUG("-INFO Failed to find suitable block, settling on: %d %d %d\n", blockPos.x, blockPos.y, blockPos.z);
        
        uint64_t rng = 0;
        setSeed(&rng, getPosAsLong(&blockPos)); // sloppy Mojang seeding
        EndIsland artificialIsland = {blockPos, nextInt(&rng, 3) + 4.0};

        Pos3 gateway = getTopPriorityBlock(&artificialIsland, &FULL_AREA);
        gateway.y += 10;

        DEBUG("----- (ARTIFICIAL) GATEWAY   %d %d %d\n", gateway.x, gateway.y, gateway.z);
        return gateway;
    }

    // set the center of the search to the southeast block of the chunk
    int cx = floor(gatewayVector.x / 16);
    int cz = floor(gatewayVector.z / 16);
    Pos3 center = {cx*16 + 15, 0, cz*16 + 15};

    // calculate maximum surface height around center
    Pos3 surfaceMax = getTerrainTallestBlock(&center, &etn);

    // calculate maximum small end island height around center
    Pos3 islandMax = getIslandTallestBlock(&endBiomeGenerator, lower48, &center, surfaceMax.y);
    DEBUG("ISLAND MAX  %d %d %d\n", islandMax.x, islandMax.y, islandMax.z);

    // compare the two max positions and choose the one that has higher priority
    // (not exactly good practice, but it looks kinda funny)
    bool shouldChooseSurface = 
        surfaceMax.y > islandMax.y 
        || (surfaceMax.y == islandMax.y 
            && (surfaceMax.x < islandMax.x 
                || (surfaceMax.x == islandMax.x 
                    && surfaceMax.z < islandMax.z)));

    Pos3 gateway;
    if (shouldChooseSurface) 
        gateway = surfaceMax;
    else 
        gateway = islandMax;
    gateway.y += 10; // BlockPos.above(10)

    if (gateway.y <= 10)
        gateway = NULL_POS; // should never happen if everything is correct

    DEBUG("----- GATEWAY   %d %d %d\n", gateway.x, gateway.y, gateway.z);
    return gateway;
}
