#include "finders.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

/* Globals */


Biome biomes[256];

const int achievementBiomes[256] = {
    //  0 1 2 3 4 5 6 7 8 9
        1,1,1,1,1,1,1,1,0,0, // 0
        0,1,1,1,1,1,1,1,1,1,
        0,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
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
        perror("Could not open file: ");
        return NULL;
    }

    *scnt = 0;

    while(!feof(fp))
    {
        if(fscanf(fp, "%ld", &seed) == 1) (*scnt)++;
    }

    baseSeeds = (long*) calloc(*scnt, sizeof(*baseSeeds));

    rewind(fp);

    for(long i = 0; i < *scnt && !feof(fp);)
    {
        if(fscanf(fp, "%ld", &baseSeeds[i]) == 1) i++;
    }

    fclose(fp);

    return baseSeeds;
}


void *baseQuadTempleSearchThread(void *data)
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
        printf("Warning: Lower bits for quality %d have not been defined: will try all combinations.\n", info.quality);

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


void baseQuadTempleSearch(const char *fnam, int threads, int quality)
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
            perror("Could't merge file: ");
            break;
        }

        while((n = fread(buffer, sizeof(char), 4096, fpart)))
        {
            if(!fwrite(buffer, sizeof(char), n, fp))
            {
                perror("Could't merge file: ");
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



/**************************** General Biome Checks *****************************
 *
 *
 */


/* getBiomeAtPos
 * ----------------
 * Returns the biome for the position specified.
 */
int getBiomeAtPos(Generator *g, Pos pos)
{
    static int ints[0x1000];

    genArea(g, &ints[0], pos.x, pos.z, 1, 1);

    return ints[0];
}

/* getTemplePos
 * ------------
 * Faster implementation for finding the block position at which the temple
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

/* getTemplePosInRegion
 * --------------------
 * Finds the chunk position within the specified region (32x32 chunks) where
 * the temple generation attempt will occur.
 * [ Closer to vanilla implementation than getTemplePos() ]
 */
Pos getTempleChunkInRegion(long seed, int regionX, int regionZ)
{
    seed = regionX*341873128712 + regionZ*132897987541 + seed + 14357617;
    setSeed(&(seed));

    Pos pos;
    pos.x = nextInt(&seed, 24);
    pos.z = nextInt(&seed, 24);

    return pos;
}


/* findBiomePosition
 * -----------------
 * Finds a suitable pseudo-random location in the specified area.
 * Used to determine the positions of spawn and stongholds.
 * The return value is non-zero if a valid location was found.
 * Warning: accurate, but slow!
 *
 * TODO: Spawn finding not working in 1.10
 */
int findBiomePosition(Generator *g, Pos *out, int centerX, int centerZ, int range, const int *biomeList, long *seed)
{
    int x1 = (centerX-range) >> 2;
    int z1 = (centerZ-range) >> 2;
    int x2 = (centerX+range) >> 2;
    int z2 = (centerZ+range) >> 2;
    int width  = x2 - x1 + 1;
    int height = z2 - z1 + 1;
    int *map = allocCache(g, width, height);

    genArea(g, map, x1, z1, width, height);
    int i, found = 0;

    out->x = 0;
    out->z = 0;

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
    return found;
}


/*s
Pos getSpawn(long seed)
{
    Generator g = setupGenerator();
    g.topLayerIndex = 43;
    applySeed(g, seed);

    Pos spawn;

    setSeed(&seed);

    int found = findBiomePosition(&g, &spawn, 0, 0, 256, biomesToSpawnIn, &seed);

    if(!found)
    {
        //printf("Unable to find spawn biome");
    }

    int var9 = 0;

    while (!this.provider.canCoordinateBeSpawn(var6, var8))
    {
        var6 += var4.nextInt(64) - var4.nextInt(64);
        var8 += var4.nextInt(64) - var4.nextInt(64);
        ++var9;

        if (var9 == 1000)
        {
            break;
        }
    }
}
*/



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

    int *map = allocCache(g, width, height);
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


/* filterAllTempCats
 * -----------------
 * Looks through the seeds in 'seedsIn' and copies those for which all
 * temperature categories are present in the 3x3 area centred on the specified
 * coordinates into 'seedsOut'. The map scale at this layer is 1:1024.
 *
 * seedsIn:      list of seeds to check
 * seedsOut:     output buffer for the candidate seeds
 * seedCnt:      number of seeds in 'seedsIn'
 * centX, centZ: search centre origin (in 1024 block units)
 *
 * Returns the number of found candidates.
 */
long filterAllTempCats(long *seedsIn, long *seedsOut, long seedCnt, int centX, int centZ)
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

    Generator gFilterSnow = setupGenerator();
    gFilterSnow.topLayerIndex = 10;
    Generator gFilterSpecial = setupGenerator();
    gFilterSpecial.topLayerIndex = 13;

    int *cache = allocCache(&gFilterSpecial, sX, sZ);

    // Construct a dummy Edge,Special layer.
    Layer layerSpecial;
    setupLayer(&layerSpecial, NULL, 3, NULL);

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

        // Continue by checking if enough cold and warm categories are present.
        applySeed(&gFilterSnow, seed);
        genArea(&gFilterSnow, cache, pX,pZ, sX,sZ);

        memset(types, 0, sizeof(types));
        for(i = 0; i < sX*sZ; i++)
            types[cache[i]]++;

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
        applySeed(&gFilterSpecial, seed);
        genArea(&gFilterSpecial, cache, pX,pZ, sX,sZ);

        memset(types, 0, sizeof(types));
        for(i = 0; i < sX*sZ; i++)
            types[ cache[i] > 4 ? (cache[i]&0xf) + 4 : cache[i] ]++;

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

    freeGenerator(&gFilterSnow);
    freeGenerator(&gFilterSpecial);
    free(cache);

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
 * Returns the number of seeds found.
 */
long filterAllMajorBiomes(long *seedsIn, long *seedsOut, long seedCnt,
        int pX, int pZ, uint sX, uint sZ)
{
    Generator gFilterMushroom = setupGenerator();
    gFilterMushroom.topLayerIndex = 17;
    Generator gFilterBiomes = setupGenerator();
    gFilterBiomes.topLayerIndex = 19;

    int *cache = allocCache(&gFilterBiomes, sZ, sZ);
    long sidx, seed, hits;
    uint i, id, hasAll;

    int types[BIOME_NUM];

    hits = 0;

    for(sidx = 0; sidx < seedCnt; sidx++)
    {
        /* We can use the Mushroom layer both to check for mushroomIsland biomes
         * and to make sure all temperature categories are present in the area.
         */
        seed = seedsIn[sidx];
        applySeed(&gFilterMushroom, seed);
        genArea(&gFilterMushroom, cache, pX,pZ, sX,sZ);

        memset(types, 0, sizeof(types));
        for(i = 0; i < sX*sZ; i++)
        {
            id = cache[i];
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

        applySeed(&gFilterBiomes, seed);
        genArea(&gFilterBiomes, cache, pX,pZ, sX,sZ);

        memset(types, 0, sizeof(types));
        for(i = 0; i < sX*sZ; i++)
        {
            types[cache[i]]++;
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

    freeGenerator(&gFilterMushroom);
    freeGenerator(&gFilterBiomes);
    free(cache);

    return hits;
}





// very slow!
int getAllBiomeRadius(long seed, const int startRadius)
{
    Generator g = setupGenerator();
    int x, z, r, i, bnum;
    int *map;
    int blist[BIOME_NUM];
    map = allocCache(&g, startRadius*2+1, startRadius*2+1);

    applySeed(&g, seed);
    genArea(&g, map, -startRadius, -startRadius, startRadius*2, startRadius*2);

    for(r = startRadius; r > 0; r--)
    {
        memset(blist, 0, sizeof(int)*BIOME_NUM);
        for(z = startRadius-r; z < startRadius+r; z++)
        {
            for(x = startRadius-r; x < startRadius+r; x++)
            {
                blist[map[x + z*2*startRadius] & 0x7f] = 1;
            }
        }

        for(i = 0, bnum = 0; i < BIOME_NUM; i++) if(blist[i]) bnum++;
        if(bnum < 36) break;
    }


    free(map);
    freeGenerator(&g);

    return r+1;
}





















