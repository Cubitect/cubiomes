#include "finders.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>

/* Globals */


Biome biomes[256];



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
 *  structures.
 *
 *  Minecraft uses a 48-bit pseudo random number generator (PRNG) to determine
 *  the position of it's structures. The remaining top 16 bits do not influence
 *  the structure positioning. Additionally the position of all temples in a
 *  world can be translated by applying the following transformation to the
 *  seed:
 *
 *  seed2 = seed1 - 14357617 - dregX * 341873128712 - dregZ * 132897987541;
 *
 *  Here seed1 and seed2 have the same structure positioning, but moved by a
 *  region offset of (dregX,dregZ). [a region is 32x32 chunks]
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
 ** Set of all Quad-Witch-Huts
 *
 *  These conditions only leave 32 free bits which can comfortably be brute-
 *  forced to get the entire set of quad-structure candidates. Each of the seeds
 *  found this way describes an entire set of possible quad-witch-huts
 *  (with degrees of freedom for region-transposition, and the top 16-bit bits).
 */


#define SEEDMAX (1L << 48)


typedef struct quad_threadinfo_t
{
    long start, end;
    int threadID;
    int quality;
    const char *fnam;
} quad_threadinfo_t;



const long lowerBaseBitsQ1[] = // for quad-structure with quality 1
        {
                0x2aa7,0x3d99,0x60a9,0x8599
        };

const long lowerBaseBitsQ2[] = // for quad-structure with quality 2
        {
                0x0825,0x0bd7,0x0c77,0x0dd7,0x0dd9,0x0e57,0x111c,0x12bc,0x12c1,
                0x12c8,0x12e7,0x12ec,0x1357,0x1358,0x1638,0x17c9,0x1849,0x1a47,
                0x1c1b,0x1d95,0x22a9,0x241f,0x2899,0x2aa7,0x2bf7,0x2c59,0x2c77,
                0x2d19,0x2dd7,0x2df9,0x2e79,0x32c1,0x32c7,0x32c8,0x32f4,0x32f7,
                0x32f9,0x333f,0x3344,0x3363,0x3368,0x3377,0x37e7,0x39c7,0x3a47,
                0x3a69,0x3ca7,0x3d99,0x41a7,0x44a7,0x44ac,0x4597,0x49a7,0x4aab,
                0x4c59,0x52c8,0x52d7,0x52d8,0x52e7,0x5305,0x5343,0x5348,0x5358,
                0x5367,0x536c,0x537b,0x57c9,0x57e7,0x5849,0x5929,0x59e9,0x5a67,
                0x5a69,0x5c97,0x5d09,0x60a9,0x61a7,0x64ab,0x6bf7,0x6d15,0x6dd7,
                0x6dd9,0x6e57,0x6e79,0x72bf,0x72c8,0x72d7,0x72f7,0x7348,0x7358,
                0x735d,0x7368,0x751c,0x77c9,0x7869,0x79c7,0x7a47,0x80ad,0x82a7,
                0x8599,0x8bd9,0x8bf7,0x8c59,0x8c77,0x8d19,0x8df9,0x8e77,0x8e79,
                0x8fcc,0x9070,0x9118,0x92fc,0x933b,0x9340,0x937c,0x93ec,0x93fc,
                0x95bc,0x9d87,0x9da6,0xa587,0xa598,0xa5a6,0xa69e,0xb0db,0xb288,
                0xb4e3,0xb55d,0xc29a,0xc2a8,0xc7e4,0xca9a,0xcd53,0xd0e5,0xd118,
                0xd5ec,0xf2ff,0xf304,0xf33c,0xf341,0xf7c9,0xf7e7,0xf849,0xf867,
                0xf9c7,0xf9e9,0xfa67,0xfa69,0xfcab
        };



int isQuadTempleBase(const long seed, const long lower, const long upper)
{
    // seed offsets for the regions (0,0) to (1,1)
    const long reg00base = 14357617;
    const long reg01base = 341873128712 + 14357617;
    const long reg10base = 132897987541 + 14357617;
    const long reg11base = 341873128712 + 132897987541 + 14357617;

    long s;

    s = (reg00base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    if((s >> 17) % 24 < upper) return 0;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    if((s >> 17) % 24 < upper) return 0;

    s = (reg01base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    if((s >> 17) % 24 > lower) return 0;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    if((s >> 17) % 24 < upper) return 0;

    s = (reg10base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    if((s >> 17) % 24 < upper) return 0;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    if((s >> 17) % 24 > lower) return 0;

    s = (reg11base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    if((s >> 17) % 24 > lower) return 0;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    if((s >> 17) % 24 > lower) return 0;

    return 1;
}

int isTriTempleBase(const long seed, const long lower, const long upper)
{
    // seed offsets for the regions (0,0) to (1,1)
    const long reg00base = 14357617;
    const long reg01base = 341873128712 + 14357617;
    const long reg10base = 132897987541 + 14357617;
    const long reg11base = 341873128712 + 132897987541 + 14357617;

    long s;
    int missing = 0;

    s = (reg00base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    if((s >> 17) % 24 < upper ||
      (((s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff) >> 17) % 24 < upper)
    {
        missing++;
    }

    s = (reg01base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    if((s >> 17) % 24 > lower ||
      (((s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff) >> 17) % 24 < upper)
    {
        if(missing) return 0;
        missing++;
    }

    s = (reg10base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    if((s >> 17) % 24 < upper ||
      (((s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff) >> 17) % 24 > lower)
    {
        if(missing) return 0;
        missing++;
    }

    s = (reg11base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    if((s >> 17) % 24 > lower ||
      (((s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff) >> 17) % 24 > lower)
    {
        if(missing) return 0;
    }

    return 1;
}


long moveTemple(const long baseSeed, const int regionX, const int regionZ)
{
    return (baseSeed - regionX*341873128712 - regionZ*132897987541) & 0xffffffffffff;
}


long *loadSavedSeeds(const char *fnam, long *scnt)
{
    FILE *fp = fopen(fnam, "r");

    long seed;
    long *baseSeeds;

    if(fp == NULL)
    {
        perror("ERR loadSavedSeeds: ");
        return NULL;
    }

    *scnt = 0;

    while(!feof(fp))
    {
        if(fscanf(fp, "%ld", &seed) == 1) (*scnt)++;
        else while(!feof(fp) && fgetc(fp) != '\n');
    }

    baseSeeds = (long*) calloc(*scnt, sizeof(*baseSeeds));

    rewind(fp);

    for(long i = 0; i < *scnt && !feof(fp);)
    {
        if(fscanf(fp, "%ld", &baseSeeds[i]) == 1) i++;
        else while(!feof(fp) && fgetc(fp) != '\n');
    }

    fclose(fp);

    return baseSeeds;
}


static void *baseQuadTempleSearchThread(void *data)
{
    quad_threadinfo_t info = *(quad_threadinfo_t*)data;

    const long lower = info.quality;
    const long upper = 23-info.quality;
    const long start = info.start;
    const long end   = info.end;

    long seed;

    const long *lowerBits;
    int lowerBitsCnt;
    int lowerBitsIdx = 0;

    if(info.quality == 1)
    {
        lowerBits = &lowerBaseBitsQ1[0];
        lowerBitsCnt = sizeof(lowerBaseBitsQ1) / sizeof(lowerBaseBitsQ1[0]);
    }
    else if(info.quality == 2)
    {
        lowerBits = &lowerBaseBitsQ2[0];
        lowerBitsCnt = sizeof(lowerBaseBitsQ2) / sizeof(lowerBaseBitsQ2[0]);
    }
    else
    {
        printf("WARN baseQuadTempleSearchThread: "
               "Lower bits for quality %d have not been defined => "
               "will try all combinations.\n", info.quality);

        static long lowerBaseBitsAll[65536];
        lowerBits = &lowerBaseBitsAll[0];
        lowerBitsCnt = sizeof(lowerBaseBitsAll) / sizeof(lowerBaseBitsAll[0]);

        int i;
        for(i = 0; i < 65536; i++) lowerBaseBitsAll[i] = i;
    }

    char fnam[256];
    sprintf(fnam, "%s.part%d", info.fnam, info.threadID);

    FILE *fp = fopen(fnam, "a+");

    seed = start;

    // Check the last entry in the file and use it as a starting point if it
    // exists. (I.e. loading the saved progress.)
    if(!fseek(fp, -31, SEEK_END))
    {
        char buf[32];
        if(fread(buf, 30, 1, fp) > 0)
        {
            char *last_newline = strrchr(buf, '\n');

            if(sscanf(last_newline, "%ld", &seed) == 1)
            {
                while(lowerBits[lowerBitsIdx] <= (seed & 0xffff))
                    lowerBitsIdx++;

                seed = (seed & 0x0000ffffffff0000) + lowerBits[lowerBitsIdx];

                printf("Thread %d starting from: %ld\n", info.threadID, seed);
            }
            else
            {
                seed = start;
            }
        }
    }


    fseek(fp, 0, SEEK_END);


    while(seed < end)
    {
        if(isQuadTempleBase(seed, lower, upper))
        {
            fprintf(fp, "%ld\n", seed);
            fflush(fp);
            //printf("Thread %d: %ld\n", info.threadID, seed);
        }

        lowerBitsIdx++;
        if(lowerBitsIdx >= lowerBitsCnt)
        {
            lowerBitsIdx = 0;
            seed += 0x10000;
        }
        seed = (seed & 0x0000ffffffff0000) + lowerBits[lowerBitsIdx];
    }

    fclose(fp);

    return NULL;
}


void baseQuadTempleSearch(const char *fnam, const int threads, const int quality)
{
    pthread_t threadID[threads];
    quad_threadinfo_t info[threads];
    long t;

    for(t = 0; t < threads; t++)
    {
        info[t].threadID = t;
        info[t].start = (t * SEEDMAX / threads) & 0x0000ffffffff0000;
        info[t].end = ((info[t].start + (SEEDMAX-1) / threads) & 0x0000ffffffff0000) + 1;
        info[t].fnam = fnam;
        info[t].quality = quality;
    }

    for(t = 0; t < threads; t++)
    {
        pthread_create(&threadID[t], NULL, baseQuadTempleSearchThread, (void*)&info[t]);
    }

    for(t = 0; t < threads; t++)
    {
        pthread_join(threadID[t], NULL);
    }

    // merge thread parts

    char fnamThread[256];
    char buffer[4097];
    FILE *fp = fopen(fnam, "w");
    FILE *fpart;
    int n;

    for(t = 0; t < threads; t++)
    {
        sprintf(fnamThread, "%s.part%d", info[t].fnam, info[t].threadID);

        fpart = fopen(fnamThread, "r");

        if(fpart == NULL)
        {
            perror("ERR baseQuadTempleSearch: ");
            break;
        }

        while((n = fread(buffer, sizeof(char), 4096, fpart)))
        {
            if(!fwrite(buffer, sizeof(char), n, fp))
            {
                perror("ERR baseQuadTempleSearch: ");
                fclose(fp);
                fclose(fpart);
                return;
            }
        }

        fclose(fpart);

        remove(fnamThread);
    }

    fclose(fp);
}



/*************************** General Purpose Checks ****************************
 */


/* getBiomeAtPos
 * ----------------
 * Returns the biome for the specified block position.
 */
int getBiomeAtPos(const LayerStack g, const Pos pos)
{
    static int ints[0x1000];

    genArea(&g.layers[g.layerNum-1], &ints[0], pos.x, pos.z, 1, 1);

    return ints[0];
}

/* getTemplePos
 * ------------
 * Fast implementation for finding the block position at which the temple
 * generation attempt will occur in the specified region.
 */
Pos getTemplePos(long seed, const long regionX, const long regionZ)
{
    Pos pos;

    // set seed
    seed = regionX*341873128712 + regionZ*132897987541 + seed + 14357617;
    seed = (seed ^ 0x5DEECE66DL);// & ((1L << 48) - 1);

    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.x = (seed >> 17) % 24;

    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.z = (seed >> 17) % 24;

    pos.x = regionX*512 + (pos.x << 4) + 8;
    pos.z = regionZ*512 + (pos.z << 4) + 8;
    return pos;
}


/* getTempleChunkInRegion
 * ----------------------
 * Finds the chunk position within the specified region (32x32 chunks) where
 * the temple generation attempt will occur.
 */
Pos getTempleChunkInRegion(long seed, const int regionX, const int regionZ)
{
    /*
    // Vanilla like implementation.
    seed = regionX*341873128712 + regionZ*132897987541 + seed + 14357617;
    setSeed(&(seed));

    Pos pos;
    pos.x = nextInt(&seed, 24);
    pos.z = nextInt(&seed, 24);
    */
    Pos pos;

    // set seed
    seed = regionX*341873128712 + regionZ*132897987541 + seed + 14357617;
    seed = (seed ^ 0x5DEECE66DL);// & ((1L << 48) - 1);

    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.x = (seed >> 17) % 24;

    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.z = (seed >> 17) % 24;

    return pos;
}


/* getVillagePos
 * -------------
 * Fast implementation for finding the block position at which the village
 * generation attempt will occur in the specified region.
 */
Pos getVillagePos(long seed, const long regionX, const long regionZ)
{
    Pos pos;

    // set seed
    seed = regionX*341873128712 + regionZ*132897987541 + seed + 10387312;
    seed = (seed ^ 0x5DEECE66DL);// & ((1L << 48) - 1);

    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.x = (seed >> 17) % 24;

    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.z = (seed >> 17) % 24;

    pos.x = regionX*512 + (pos.x << 4) + 8;
    pos.z = regionZ*512 + (pos.z << 4) + 8;
    return pos;
}


/* getOceanMonumentPos
 * -------------------
 * Fast implementation for finding the block position at which the ocean
 * monument generation attempt will occur in the specified region.
 */
Pos getOceanMonumentPos(long seed, const long regionX, const long regionZ)
{
    Pos pos;

    // set seed
    seed = regionX*341873128712 + regionZ*132897987541 + seed + 10387313;
    seed = (seed ^ 0x5DEECE66DL);// & ((1L << 48) - 1);

    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.x = (seed >> 17) % 27;
    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.x += (seed >> 17) % 27;

    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.z = (seed >> 17) % 27;
    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.z += (seed >> 17) % 27;

    pos.x = regionX*512 + (pos.x << 3) + 8;
    pos.z = regionZ*512 + (pos.z << 3) + 8;
    return pos;
}


/* getMansionPos
 * -------------
 * Fast implementation for finding the block position at which the woodland
 * mansions generation attempt will occur in the specified 80x80 chunk area.
 *
 * area80X, area80Z: area coordinates in units 1280 blocks (= 80 chunks)
 */
Pos getMansionPos(long seed, const long area80X, const long area80Z)
{
    Pos pos;

    // set seed
    seed = area80X*341873128712 + area80Z*132897987541 + seed + 10387319;
    seed = (seed ^ 0x5DEECE66DL);// & ((1L << 48) - 1);

    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.x = (seed >> 17) % 60;
    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.x += (seed >> 17) % 60;

    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.z = (seed >> 17) % 60;
    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.z += (seed >> 17) % 60;

    pos.x = area80X*1280 + (pos.x << 3) + 8;
    pos.z = area80Z*1280 + (pos.z << 3) + 8;
    return pos;
}


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
 *                    (usually you want to initialise this with the world seed)
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
        )
{
    int x1 = (centerX-range) >> 2;
    int z1 = (centerZ-range) >> 2;
    int x2 = (centerX+range) >> 2;
    int z2 = (centerZ+range) >> 2;
    int width  = x2 - x1 + 1;
    int height = z2 - z1 + 1;
    int *map;
    int i, found;

    Layer *layer = &g.layers[L_RIVER_MIX_4];
    Pos out;

    if(layer->scale != 4)
    {
        printf("WARN findBiomePosition: The generator has unexpected scale %d at layer %d.\n",
                layer->scale, L_RIVER_MIX_4);
    }

    map = cache ? cache : allocCache(layer, width, height);

    genArea(layer, map, x1, z1, width, height);

    out.x = 0;
    out.z = 0;
    found = 0;

    for(i = 0; i < width*height; i++)
    {
        if(isValid[map[i] & 0xff] && (found == 0 || nextInt(seed, found + 1) == 0))
        {
            out.x = (x1 + i%width) << 2;
            out.z = (z1 + i/width) << 2;
            ++found;
        }
    }

    if(cache == NULL)
    {
        free(map);
    }

    if(passes != NULL)
    {
        *passes = found;
    }

    return out;
}


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
void findStrongholds_pre19(LayerStack *g, int *cache, Pos *locations, long worldSeed)
{
    static int validStrongholdBiomes[256];
    const int SHNUM = 3;
    int i;

    if(!validStrongholdBiomes[plains])
    {
        int id;
        for(id = 0; id < 256; id++)
        {
            if(biomeExists(id) && biomes[id].height > 0.0) validStrongholdBiomes[id] = 1;
        }
    }

    setWorldSeed(&g->layers[L_RIVER_MIX_4], worldSeed);

    setSeed(&worldSeed);
    double angle = nextDouble(&worldSeed) * 3.141592653589793 * 2.0;


    for(i = 0; i < SHNUM; i++)
    {
        double distance = (1.25 + nextDouble(&worldSeed)) * 32.0;
        int x = (int)round(cos(angle) * distance);
        int z = (int)round(sin(angle) * distance);

        locations[i] = findBiomePosition(*g, cache, (x << 4) + 8, (z << 4) + 8, 112,
                validStrongholdBiomes, &worldSeed, NULL);

        angle += 6.283185307179586 / (double)SHNUM;
    }
}


/* TODO: Estimate whether the given positions could be spawn based on biomes.
 */
static int canCoordinateBeSpawn(LayerStack *g, int *cache, Pos pos)
{
    return 1;
}

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
Pos getSpawn(LayerStack *g, int *cache, long worldSeed)
{
    static int isSpawnBiome[0x100];
    Pos spawn;
    int found;
    uint i;

    if(!isSpawnBiome[biomesToSpawnIn[0]])
    {
        for(i = 0; i < sizeof(biomesToSpawnIn) / sizeof(int); i++)
        {
            isSpawnBiome[ biomesToSpawnIn[i] ] = 1;
        }
    }

    applySeed(g, worldSeed);
    setSeed(&worldSeed);

    spawn = findBiomePosition(*g, cache, 0, 0, 256, isSpawnBiome, &worldSeed, &found);

    if(!found)
    {
        printf("Unable to find spawn biome.\n");
        spawn.x = spawn.z = 8;
    }

    for(i = 0; i < 1000 && !canCoordinateBeSpawn(g, cache, spawn); i++)
    {
        spawn.x += nextInt(&worldSeed, 64) - nextInt(&worldSeed, 64);
        spawn.z += nextInt(&worldSeed, 64) - nextInt(&worldSeed, 64);
    }

    return spawn;
}



/* areBiomesViable
 * ---------------
 * Determines if the given area contains only biomes specified by 'biomeList'.
 * Used to determine the positions of ocean monuments and villages.
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
        )
{
    int x1 = (posX - radius) >> 2;
    int z1 = (posZ - radius) >> 2;
    int x2 = (posX + radius) >> 2;
    int z2 = (posZ + radius) >> 2;
    int width = x2 - x1 + 1;
    int height = z2 - z1 + 1;
    int i;
    int *map;

    Layer *layer = &g.layers[L_RIVER_MIX_4];

    if(layer->scale != 4)
    {
        printf("WARN areBiomesViable: The generator has unexpected scale %d at layer %d.\n",
                layer->scale, L_RIVER_MIX_4);
    }

    map = cache ? cache : allocCache(layer, width, height);

    genArea(layer, map, x1, z1, width, height);

    for(i = 0; i < width*height; i++)
    {
        if(!isValid[ map[i] & 0xff ])
        {
            if(cache == NULL) free(map);
            return 0;
        }
    }

    if(cache == NULL) free(map);
    return 1;
}



int isViableTemplePos(const LayerStack g, int *cache, const long blockX, const long blockZ)
{
    static int map[0x100];
    genArea(&g.layers[L_VORONOI_ZOOM_1], map, blockX, blockZ, 1, 1);

    if(map[0] == jungle || map[0] == jungleHills) return JUNGLE_TEMPLE;
    if(map[0] == swampland) return SWAMP_HUT;
    if(map[0] == icePlains || map[0] == coldTaiga) return IGLOO;
    if(map[0] == desert || map[0] == desertHills) return DESERT_TEMPLE;

    return 0;
}

int isViableVillagePos(const LayerStack g, int *cache, const long blockX, const long blockZ)
{
    static int isVillageBiome[0x100];

    if(!isVillageBiome[villageBiomeList[0]])
    {
        uint i;
        for(i = 0; i < sizeof(villageBiomeList) / sizeof(int); i++)
        {
            isVillageBiome[ villageBiomeList[i] ] = 1;
        }
    }

    return areBiomesViable(g, cache, blockX, blockZ, 0, isVillageBiome);
}

int isViableOceanMonumentPos(const LayerStack g, int *cache, const long blockX, const long blockZ)
{
    static int isWaterBiome[0x100];
    static int isDeepOcean[0x100];

    if(!isWaterBiome[oceanMonumentBiomeList[0]])
    {
        uint i;
        for(i = 0; i < sizeof(oceanMonumentBiomeList) / sizeof(int); i++)
        {
            isWaterBiome[ oceanMonumentBiomeList[i] ] = 1;
        }

        isDeepOcean[deepOcean] = 1;
    }

    return areBiomesViable(g, cache, blockX, blockZ, 16, isDeepOcean) &&
            areBiomesViable(g, cache, blockX, blockZ, 29, isWaterBiome);
}

int isViableMansionPos(const LayerStack g, int *cache, const long blockX, const long blockZ)
{
    static int isMansionBiome[0x100];

    if(!isMansionBiome[mansionBiomeList[0]])
    {
        uint i;
        for(i = 0; i < sizeof(mansionBiomeList) / sizeof(int); i++)
        {
            isMansionBiome[ mansionBiomeList[i] ] = 1;
        }
    }

    return areBiomesViable(g, cache, blockX, blockZ, 32, isMansionBiome);
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
        const int ignoreMutations)
{
    int r, i, b;
    int blist[0x100];
    int mask = ignoreMutations ? 0x7f : 0xff;
    int radiusMax = mapSide / 2;

    if((mapSide & 1) == 0)
    {
        printf("WARN getBiomeRadius: Side length of the square map should be an odd integer.\n");
    }

    memset(blist, 0, sizeof(blist));

    for(r = 1; r < radiusMax; r++)
    {
        for(i = radiusMax-r; i <= radiusMax+r; i++)
        {
            blist[ map[(radiusMax-r) * mapSide+ i]    & mask ] = 1;
            blist[ map[(radiusMax+r-1) * mapSide + i] & mask ] = 1;
            blist[ map[mapSide*i + (radiusMax-r)]     & mask ] = 1;
            blist[ map[mapSide*i + (radiusMax+r-1)]   & mask ] = 1;
        }

        for(b = 0; b < bnum && blist[biomes[b] & mask]; b++);
        if(b >= bnum)
        {
            break;
        }
    }

    return r != radiusMax ? r : -1;
}


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
        const int centZ)
{
    /* We require all temperature categories, including the special variations
     * in order to get all main biomes. This gives 8 required values:
     * Oceanic, Warm, Lush, Cold, Freezing,
     * Special Warm, Special Lush, Special Cold
     * These categories generate at Layer 13: Edge, Special.
     *
     * Note: The scale at this layer is 1:1024 and each element can "leak" its
     * biome values up to 1024 blocks outwards into the negative coordinates
     * (due to the Zoom layers).
     *
     * The plan is to check if the 3x3 area contains all 8 temperature types.
     * For this, we can check even earlier at Layer 10: Add Island, that each of
     * the Warm, Cold and Freezing categories are present.
     */

    /* Edit:
     * All the biomes that are generated by a simple Cold climate can actually
     * be generated later on. So I have commented out the Cold requirements.
     */

    const int pX = centX-1, pZ = centZ-1;
    const int sX = 3, sZ = 3;
    int *map;

    Layer *lFilterSnow = &g->layers[L_ADD_SNOW_1024];
    Layer *lFilterSpecial = &g->layers[L_SPECIAL_1024];

    map = cache ? cache : allocCache(lFilterSpecial, sX, sZ);

    // Construct a dummy Edge,Special layer.
    Layer layerSpecial;
    setupLayer(1024, &layerSpecial, NULL, 3, NULL);

    long sidx, hits, seed;
    int types[9];
    int specialCnt;
    int i, j;

    hits = 0;

    for(sidx = 0; sidx < seedCnt; sidx++)
    {
        seed = seedsIn[sidx];

        /***  Pre-Generation Checks  ***/

        // We require at least 3 special temperature categories which can be
        // tested for without going through the previous layers. (We'll get
        // false positives due to Oceans, but this works fine to rule out some
        // seeds early on.)
        setWorldSeed(&layerSpecial, seed);
        specialCnt = 0;
        for(i = 0; i < sX; i++)
        {
            for(j = 0; j < sZ; j++)
            {
                setChunkSeed(&layerSpecial, (long)(i+pX), (long)(j+pZ));
                if(mcNextInt(&layerSpecial, 13) == 0)
                    specialCnt++;
            }
        }

        if(specialCnt < 3)
        {
            continue;
        }

        /***  Cold/Warm Check  ***/

        // Continue by checking if enough cold and warm categories are present.#
        setWorldSeed(lFilterSnow, seed);
        genArea(lFilterSnow, map, pX,pZ, sX,sZ);

        memset(types, 0, sizeof(types));
        for(i = 0; i < sX*sZ; i++)
            types[map[i]]++;

        // 1xOcean needs to be present
        // 4xWarm need to turn into Warm, Lush, Special Warm and Special Lush
        // 1xFreezing that needs to stay Freezing
        // 3x(Cold + Freezing) for Cold, Special Cold and Freezing
        if( types[Ocean] < 1 || types[Warm] < 4 || types[Freezing] < 1 ||
            types[Cold]+types[Freezing] < 2)
        {
            continue;
        }

        /***  Complete Temperature Category Check  ***/

        // Check that all temperature variants are present.
        setWorldSeed(lFilterSpecial, seed);
        genArea(lFilterSpecial, map, pX,pZ, sX,sZ);

        memset(types, 0, sizeof(types));
        for(i = 0; i < sX*sZ; i++)
            types[ map[i] > 4 ? (map[i]&0xf) + 4 : map[i] ]++;

        if( types[Ocean] < 1  || types[Warm] < 1     || types[Lush] < 1 ||
            /*types[Cold] < 1   ||*/ types[Freezing] < 1 ||
            types[Warm+4] < 1 || types[Lush+4] < 1   || types[Cold+4] < 1)
        {
            continue;
        }

        /*
        for(i = 0; i < sX*sZ; i++)
        {
            printf("%c%d ", " s"[cache[i] > 4], cache[i]&0xf);
            if(i % sX == sX-1) printf("\n");
        }
        printf("\n");*/

        // Save the candidate.
        seedsOut[hits] = seed;
        hits++;
    }

    if(cache == NULL) free(map);
    return hits;
}





const int majorBiomes[] = {
        ocean, plains, desert, extremeHills, forest, taiga, swampland,
        icePlains, mushroomIsland, jungle, deepOcean, birchForest, roofedForest,
        coldTaiga, megaTaiga, savanna, mesaPlateau_F, mesaPlateau
};


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
        const uint sZ)
{
    Layer *lFilterMushroom = &g->layers[L_ADD_MUSHROOM_ISLAND_256];
    Layer *lFilterBiomes = &g->layers[L_BIOME_256];

    int *map;
    long sidx, seed, hits;
    uint i, id, hasAll;

    int types[BIOME_NUM];

    map = cache ? cache : allocCache(lFilterBiomes, sX, sZ);

    hits = 0;

    for(sidx = 0; sidx < seedCnt; sidx++)
    {
        /* We can use the Mushroom layer both to check for mushroomIsland biomes
         * and to make sure all temperature categories are present in the area.
         */
        seed = seedsIn[sidx];
        setWorldSeed(lFilterMushroom, seed);
        genArea(lFilterMushroom, map, pX,pZ, sX,sZ);

        memset(types, 0, sizeof(types));
        for(i = 0; i < sX*sZ; i++)
        {
            id = map[i];
            if(id >= BIOME_NUM) id = (id & 0xf) + 4;
            types[id]++;
        }

        if( types[Ocean] < 1  || types[Warm] < 1     || types[Lush] < 1 ||
         /* types[Cold] < 1   || */ types[Freezing] < 1 ||
            types[Warm+4] < 1 || types[Lush+4] < 1   || types[Cold+4] < 1 ||
            types[mushroomIsland] < 1)
        {
            continue;
        }

        /***  Find all major biomes  ***/

        setWorldSeed(lFilterBiomes, seed);
        genArea(lFilterBiomes, map, pX,pZ, sX,sZ);

        memset(types, 0, sizeof(types));
        for(i = 0; i < sX*sZ; i++)
        {
            types[map[i]]++;
        }

        hasAll = 1;
        for(i = 0; i < sizeof(majorBiomes) / sizeof(*majorBiomes); i++)
        {
            // plains, taiga and deepOcean can be generated in later layers.
            // Also small islands of Forests can be generated in deepOcean
            // biomes, but we are going to ignore those.
            if(majorBiomes[i] == plains ||
               majorBiomes[i] == taiga ||
               majorBiomes[i] == deepOcean)
            {
                continue;
            }

            if(types[majorBiomes[i]] < 1)
            {
                hasAll = 0;
                break;
            }
        }
        if(!hasAll)
        {
            continue;
        }

        seedsOut[hits] = seed;
        hits++;
    }

    if(cache == NULL) free(map);
    return hits;
}
















