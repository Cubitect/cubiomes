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
                0x2aa4,0x3d96,0x60a6,0x8596
        };

const long lowerBaseBitsQ2[] = // for quad-structure with quality 2
        {
                0x0822,0x0bd4,0x0c74,0x0dd4,0x0dd6,0x0e54,0x1119,0x12b9,0x12be,
                0x12c5,0x12e4,0x12e9,0x1354,0x1355,0x1635,0x17c6,0x1846,0x1a44,
                0x1c18,0x1d92,0x22a6,0x241c,0x2896,0x2aa4,0x2bf4,0x2c56,0x2c74,
                0x2d16,0x2dd4,0x2df6,0x2e76,0x32be,0x32c4,0x32c5,0x32f1,0x32f4,
                0x32f6,0x333c,0x3341,0x3360,0x3365,0x3374,0x37e4,0x39c4,0x3a44,
                0x3a66,0x3ca4,0x3d96,0x41a4,0x44a4,0x44a9,0x4594,0x49a4,0x4aa8,
                0x4c56,0x52c5,0x52d4,0x52d5,0x52e4,0x5302,0x5340,0x5345,0x5355,
                0x5364,0x5369,0x5378,0x57c6,0x57e4,0x5846,0x5926,0x59e6,0x5a64,
                0x5a66,0x5c94,0x5d06,0x60a6,0x61a4,0x64a8,0x6bf4,0x6d12,0x6dd4,
                0x6dd6,0x6e54,0x6e76,0x72bc,0x72c5,0x72d4,0x72f4,0x7345,0x7355,
                0x735a,0x7365,0x7519,0x77c6,0x7866,0x79c4,0x7a44,0x80aa,0x82a4,
                0x8596,0x8bd6,0x8bf4,0x8c56,0x8c74,0x8d16,0x8df6,0x8e74,0x8e76,
                0x8fc9,0x906d,0x9115,0x92f9,0x9338,0x933d,0x9379,0x93e9,0x93f9,
                0x95b9,0x9d84,0x9da3,0xa584,0xa595,0xa5a3,0xa69b,0xb0d8,0xb285,
                0xb4e0,0xb55a,0xc297,0xc2a5,0xc7e1,0xca97,0xcd50,0xd0e2,0xd115,
                0xd5e9,0xf2fc,0xf301,0xf339,0xf33e,0xf7c6,0xf7e4,0xf846,0xf864,
                0xf9c4,0xf9e6,0xfa64,0xfa66,0xfca8
        };



int isQuadWitchHutBase(const long seed, const long lower, const long upper)
{
    // seed offsets for the regions (0,0) to (1,1)
    const long reg00base = 14357620;
    const long reg01base = 341873128712 + 14357620;
    const long reg10base = 132897987541 + 14357620;
    const long reg11base = 341873128712 + 132897987541 + 14357620;

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


int isTriWitchHutBase(const long seed, const long lower, const long upper)
{
    // seed offsets for the regions (0,0) to (1,1)
    const long reg00base = 14357620;
    const long reg01base = 341873128712 + 14357620;
    const long reg10base = 132897987541 + 14357620;
    const long reg11base = 341873128712 + 132897987541 + 14357620;

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


int isQuadMonumentBase(const long seed, const int qual)
{
    // seed offsets for the regions (0,0) to (1,1)
    const long reg00base = 10387313;
    const long reg01base = 341873128712 + 10387313;
    const long reg10base = 132897987541 + 10387313;
    const long reg11base = 341873128712 + 132897987541 + 10387313;

    long s, p;

    /*
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
    */

    s = (reg00base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p < 26-qual) return 0;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p < 2*26-qual) return 0;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p < 26-qual) return 0;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p < 2*26-qual) return 0;

    s = (reg01base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p > qual) return 0;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p > qual) return 0;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p < 26-qual) return 0;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p < 2*26-qual) return 0;

    s = (reg10base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p < 26-qual) return 0;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p < 2*26-qual) return 0;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p > qual) return 0;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p > qual) return 0;

    s = (reg11base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p > qual) return 0;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p > qual) return 0;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p > qual) return 0;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p > qual) return 0;

    return 1;
}


int isTriMonumentBase(const long seed, const int qual)
{
    // seed offsets for the regions (0,0) to (1,1)
    const long reg00base = 10387313;
    const long reg01base = 341873128712 + 10387313;
    const long reg10base = 132897987541 + 10387313;
    const long reg11base = 341873128712 + 132897987541 + 10387313;

    long s, p;
    int incomplete = 0;

    /*
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
    */

    s = (reg00base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p < 26-qual) goto incomp11;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p < 2*26-qual) goto incomp11;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p < 26-qual) goto incomp11;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p < 2*26-qual) goto incomp11;

    if(0)
    {
        incomp11:
        incomplete = 1;
    }

    s = (reg01base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p > qual) goto incomp01;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p > qual) goto incomp01;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p < 26-qual) goto incomp01;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p < 2*26-qual) goto incomp01;

    if(0)
    {
        incomp01:
        if(incomplete) return 0;
        incomplete = 2;
    }

    s = (reg10base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p < 26-qual) goto incomp10;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p < 2*26-qual) goto incomp10;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p > qual) goto incomp10;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p > qual) goto incomp10;

    if(0)
    {
        incomp10:
        if(incomplete) return 0;
        incomplete = 3;
    }

    s = (reg11base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p > qual) goto incomp00;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p > qual) goto incomp00;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p > qual) goto incomp00;
    s = (s * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p > qual) goto incomp00;

    if(0)
    {
        incomp00:
        if(incomplete) return 0;
        incomplete = 4;
    }

    return incomplete ? incomplete : -1;
}

// Searches for the optimal AFK position given four structures at positions 'p',
// each of volume (ax,ay,az). 
// Returned is the number of spawning spaces in reach.
int countBlocksInSpawnRange(Pos p[4], const int ax, const int ay, const int az)
{
    int minX = 3e7, minZ = 3e7, maxX = -3e7, maxZ = -3e7;
    int best;


    // Find corners
    for(int i = 0; i < 4; i++)
    {
        if(p[i].x < minX) minX = p[i].x;
        if(p[i].z < minZ) minZ = p[i].z;
        if(p[i].x > maxX) maxX = p[i].x;
        if(p[i].z > maxZ) maxZ = p[i].z;
    }


    // assume that the search area is bound by the inner corners
    maxX += ax;
    maxZ += az;
    best = 0;

    double thsq = 128.0*128.0 - az*az/4.0;

    for(int x = minX; x < maxX; x++)
    {
        for(int z = minZ; z < maxZ; z++)
        {
            int inrange = 0;

            for(int i = 0; i < 4; i++)
            {
                double dx = p[i].x - (x+0.5);
                double dz = p[i].z - (z+0.5);

                for(int px = 0; px < ax; px++)
                {
                    for(int pz = 0; pz < az; pz++)
                    {
                        double ddx = px + dx;
                        double ddz = pz + dz;
                        inrange += (ddx*ddx + ddz*ddz <= thsq);
                    }
                }
            }

            if(inrange > best)
            {
                best = inrange;
            }
        }
    }

    return best;
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


static void *baseQuadWitchHutSearchThread(void *data)
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
        printf("WARN baseQuadWitchHutSearchThread: "
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
    if (fp == NULL) {
        fprintf(stderr, "Could not open \"%s\" for writing.\n", fnam);
        exit(-1);
    }

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
        if(isQuadWitchHutBase(seed, lower, upper))
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


void baseQuadWitchHutSearch(const char *fnam, const int threads, const int quality)
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
        pthread_create(&threadID[t], NULL, baseQuadWitchHutSearchThread, (void*)&info[t]);
    }

    for(t = 0; t < threads; t++)
    {
        pthread_join(threadID[t], NULL);
    }

    // merge thread parts

    char fnamThread[256];
    char buffer[4097];
    FILE *fp = fopen(fnam, "w");
    if (fp == NULL) {
        fprintf(stderr, "Could not open \"%s\" for writing.\n", fnam);
        exit(-1);
    }
    FILE *fpart;
    int n;

    for(t = 0; t < threads; t++)
    {
        sprintf(fnamThread, "%s.part%d", info[t].fnam, info[t].threadID);

        fpart = fopen(fnamThread, "r");

        if(fpart == NULL)
        {
            perror("ERR baseQuadWitchHutSearch: ");
            break;
        }

        while((n = fread(buffer, sizeof(char), 4096, fpart)))
        {
            if(!fwrite(buffer, sizeof(char), n, fp))
            {
                perror("ERR baseQuadWitchHutSearch: ");
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
 * Returns the biome for the specified block position. This function is not
 * threadsafe.
 * (Alternatives should be considered in performance critical code.)
 */
int getBiomeAtPos(const LayerStack g, const Pos pos)
{
    static int ints[0x1000];

    genArea(&g.layers[g.layerNum-1], &ints[0], pos.x, pos.z, 1, 1);

    return ints[0];
}

/* getWitchHutPos
 * ------------
 * Fast implementation for finding the block position at which the witch hut
 * generation attempt will occur in the specified region.
 */
Pos getWitchHutPos(long seed, const long regionX, const long regionZ)
{
    Pos pos;

    // set seed
    seed = regionX*341873128712 + regionZ*132897987541 + seed + 14357620;
    seed = (seed ^ 0x5DEECE66DL);// & ((1L << 48) - 1);

    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.x = (seed >> 17) % 24;

    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.z = (seed >> 17) % 24;

    pos.x = regionX*512 + (pos.x << 4) + 8;
    pos.z = regionZ*512 + (pos.z << 4) + 8;
    return pos;
}


/* getWitchHutChunkInRegion
 * ----------------------
 * Finds the chunk position within the specified region (32x32 chunks) where
 * the witch hut generation attempt will occur.
 */
Pos getWitchHutChunkInRegion(long seed, const int regionX, const int regionZ)
{
    /*
    // Vanilla like implementation.
    seed = regionX*341873128712 + regionZ*132897987541 + seed + 14357620;
    setSeed(&(seed));

    Pos pos;
    pos.x = nextInt(&seed, 24);
    pos.z = nextInt(&seed, 24);
    */
    Pos pos;

    // set seed
    seed = regionX*341873128712 + regionZ*132897987541 + seed + 14357620;
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


/* getOceanMonumentChunk
 * ---------------------
 * Fast implementation for finding the chunk relative to the region at which the
 * ocean monument generation attempt will occur.
 */

Pos getOceanMonumentChunk(long seed, const long regionX, const long regionZ)
{
    Pos pos;

    // set seed
    seed = regionX*341873128712 + regionZ*132897987541 + seed + 10387313;
    seed = (seed ^ 0x5DEECE66DL) & ((1L << 48) - 1);

    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.x = (seed >> 17) % 27;
    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.x += (seed >> 17) % 27;

    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.z = (seed >> 17) % 27;
    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.z += (seed >> 17) % 27;

    pos.x >>= 1;
    pos.z >>= 1;
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
    seed = (seed ^ 0x5DEECE66DL) & ((1L << 48) - 1);

    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.x = (seed >> 17) % 27;
    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.x += (seed >> 17) % 27;

    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.z = (seed >> 17) % 27;
    seed = (seed * 0x5DEECE66DL + 0xBL) & 0xffffffffffff;
    pos.z += (seed >> 17) % 27;

    pos.x = regionX*32 + (pos.x >> 1);
    pos.z = regionZ*32 + (pos.z >> 1);
    pos.x = pos.x*16 + 8;
    pos.z = pos.z*16 + 8;
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

    pos.x = area80X*80 + (pos.x >> 1);
    pos.z = area80Z*80 + (pos.z >> 1);
    pos.x = pos.x*16 + 8;
    pos.z = pos.z*16 + 8;
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
        // Unable to find spawn biome.
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
