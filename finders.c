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



typedef struct quad_threadinfo_t
{
    int64_t start, end;
    int64_t structureSeed;
    int threadID;
    int quality;
    const char *fnam;
} quad_threadinfo_t;



const int64_t lowerBaseBitsQ1[] = // for quad-structure with quality 1
        {
                0x3f18,0x520a,0x751a,0x9a0a
        };

const int64_t lowerBaseBitsQ2[] = // for quad-structure with quality 2
        {
                0x0770,0x0775,0x07ad,0x07b2,0x0c3a,0x0c58,0x0cba,0x0cd8,0x0e38,
                0x0e5a,0x0ed8,0x0eda,0x111c,0x1c96,0x2048,0x20e8,0x2248,0x224a,
                0x22c8,0x258d,0x272d,0x2732,0x2739,0x2758,0x275d,0x27c8,0x27c9,
                0x2aa9,0x2c3a,0x2cba,0x2eb8,0x308c,0x3206,0x371a,0x3890,0x3d0a,
                0x3f18,0x4068,0x40ca,0x40e8,0x418a,0x4248,0x426a,0x42ea,0x4732,
                0x4738,0x4739,0x4765,0x4768,0x476a,0x47b0,0x47b5,0x47d4,0x47d9,
                0x47e8,0x4c58,0x4e38,0x4eb8,0x4eda,0x5118,0x520a,0x5618,0x5918,
                0x591d,0x5a08,0x5e18,0x5f1c,0x60ca,0x6739,0x6748,0x6749,0x6758,
                0x6776,0x67b4,0x67b9,0x67c9,0x67d8,0x67dd,0x67ec,0x6c3a,0x6c58,
                0x6cba,0x6d9a,0x6e5a,0x6ed8,0x6eda,0x7108,0x717a,0x751a,0x7618,
                0x791c,0x8068,0x8186,0x8248,0x824a,0x82c8,0x82ea,0x8730,0x8739,
                0x8748,0x8768,0x87b9,0x87c9,0x87ce,0x87d9,0x898d,0x8c3a,0x8cda,
                0x8e38,0x8eb8,0x951e,0x9718,0x9a0a,0xa04a,0xa068,0xa0ca,0xa0e8,
                0xa18a,0xa26a,0xa2e8,0xa2ea,0xa43d,0xa4e1,0xa589,0xa76d,0xa7ac,
                0xa7b1,0xa7ed,0xa85d,0xa86d,0xaa2d,0xb1f8,0xb217,0xb9f8,0xba09,
                0xba17,0xbb0f,0xc54c,0xc6f9,0xc954,0xc9ce,0xd70b,0xd719,0xdc55,
                0xdf0b,0xe1c4,0xe556,0xe589,0xea5d
        };



int isQuadFeatureBase(const int64_t structureSeed, const int64_t seed,
        const int64_t lower, const int64_t upper)
{
    // seed offsets for the regions (0,0) to (1,1)
    const int64_t reg00base = structureSeed;
    const int64_t reg01base = 341873128712 + structureSeed;
    const int64_t reg10base = 132897987541 + structureSeed;
    const int64_t reg11base = 341873128712 + 132897987541 + structureSeed;

    int64_t s;

    s = (reg00base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    if((s >> 17) % 24 < upper) return 0;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    if((s >> 17) % 24 < upper) return 0;

    s = (reg01base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    if((s >> 17) % 24 > lower) return 0;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    if((s >> 17) % 24 < upper) return 0;

    s = (reg10base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    if((s >> 17) % 24 < upper) return 0;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    if((s >> 17) % 24 > lower) return 0;

    s = (reg11base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    if((s >> 17) % 24 > lower) return 0;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    if((s >> 17) % 24 > lower) return 0;

    return 1;
}


int isTriFeatureBase(const int64_t structureSeed, const int64_t seed,
        const int64_t lower, const int64_t upper)
{
    // seed offsets for the regions (0,0) to (1,1)
    const int64_t reg00base = structureSeed;
    const int64_t reg01base = 341873128712 + structureSeed;
    const int64_t reg10base = 132897987541 + structureSeed;
    const int64_t reg11base = 341873128712 + 132897987541 + structureSeed;

    int64_t s;
    int missing = 0;

    s = (reg00base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    if((s >> 17) % 24 < upper ||
      (((s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff) >> 17) % 24 < upper)
    {
        missing++;
    }

    s = (reg01base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    if((s >> 17) % 24 > lower ||
      (((s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff) >> 17) % 24 < upper)
    {
        if(missing) return 0;
        missing++;
    }

    s = (reg10base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    if((s >> 17) % 24 < upper ||
      (((s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff) >> 17) % 24 > lower)
    {
        if(missing) return 0;
        missing++;
    }

    s = (reg11base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    if((s >> 17) % 24 > lower ||
      (((s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff) >> 17) % 24 > lower)
    {
        if(missing) return 0;
    }

    return 1;
}

int64_t moveStructure(const int64_t baseSeed, const int regionX, const int regionZ)
{
    return (baseSeed - regionX*341873128712 - regionZ*132897987541) & 0xffffffffffff;
}


int isQuadMonumentBase(const int64_t seed, const int qual)
{
    // seed offsets for the regions (0,0) to (1,1)
    const int64_t reg00base = MONUMENT_SEED;
    const int64_t reg01base = 341873128712 + MONUMENT_SEED;
    const int64_t reg10base = 132897987541 + MONUMENT_SEED;
    const int64_t reg11base = 341873128712 + 132897987541 + MONUMENT_SEED;

    int64_t s, p;

    /*
    seed = regionX*341873128712 + regionZ*132897987541 + seed + 10387313;
    seed = (seed ^ 0x5DEECE66DLL);// & ((1LL << 48) - 1);

    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    pos.x = (seed >> 17) % 27;
    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    pos.x += (seed >> 17) % 27;

    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    pos.z = (seed >> 17) % 27;
    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    pos.z += (seed >> 17) % 27;
    */

    s = (reg00base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p < 26-qual) return 0;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p < 2*26-qual) return 0;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p < 26-qual) return 0;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p < 2*26-qual) return 0;

    s = (reg01base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p > qual) return 0;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p > qual) return 0;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p < 26-qual) return 0;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p < 2*26-qual) return 0;

    s = (reg10base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p < 26-qual) return 0;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p < 2*26-qual) return 0;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p > qual) return 0;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p > qual) return 0;

    s = (reg11base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p > qual) return 0;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p > qual) return 0;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p > qual) return 0;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p > qual) return 0;

    return 1;
}


int isTriMonumentBase(const int64_t seed, const int qual)
{
    // seed offsets for the regions (0,0) to (1,1)
    const int64_t reg00base = MONUMENT_SEED;
    const int64_t reg01base = 341873128712 + MONUMENT_SEED;
    const int64_t reg10base = 132897987541 + MONUMENT_SEED;
    const int64_t reg11base = 341873128712 + 132897987541 + MONUMENT_SEED;

    int64_t s, p;
    int incomplete = 0;

    /*
    seed = regionX*341873128712 + regionZ*132897987541 + seed + 10387313;
    seed = (seed ^ 0x5DEECE66DLL);// & ((1LL << 48) - 1);

    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    pos.x = (seed >> 17) % 27;
    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    pos.x += (seed >> 17) % 27;

    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    pos.z = (seed >> 17) % 27;
    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    pos.z += (seed >> 17) % 27;
    */

    s = (reg00base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p < 26-qual) goto incomp11;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p < 2*26-qual) goto incomp11;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p < 26-qual) goto incomp11;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p < 2*26-qual) goto incomp11;

    if(0)
    {
        incomp11:
        incomplete = 1;
    }

    s = (reg01base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p > qual) goto incomp01;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p > qual) goto incomp01;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p < 26-qual) goto incomp01;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p < 2*26-qual) goto incomp01;

    if(0)
    {
        incomp01:
        if(incomplete) return 0;
        incomplete = 2;
    }

    s = (reg10base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p < 26-qual) goto incomp10;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p < 2*26-qual) goto incomp10;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p > qual) goto incomp10;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p > qual) goto incomp10;

    if(0)
    {
        incomp10:
        if(incomplete) return 0;
        incomplete = 3;
    }

    s = (reg11base + seed) ^ 0x5DEECE66DL; // & 0xffffffffffff;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p > qual) goto incomp00;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p += (s >> 17) % 27;
    if(p > qual) goto incomp00;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    p = (s >> 17) % 27;
    if(p > qual) goto incomp00;
    s = (s * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
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



int64_t *loadSavedSeeds(const char *fnam, int64_t *scnt)
{
    FILE *fp = fopen(fnam, "r");

    int64_t seed;
    int64_t *baseSeeds;

    if(fp == NULL)
    {
        perror("ERR loadSavedSeeds: ");
        return NULL;
    }

    *scnt = 0;

    while(!feof(fp))
    {
        if(fscanf(fp, "%"PRId64, &seed) == 1) (*scnt)++;
        else while(!feof(fp) && fgetc(fp) != '\n');
    }

    baseSeeds = (int64_t*) calloc(*scnt, sizeof(*baseSeeds));

    rewind(fp);

    for(int64_t i = 0; i < *scnt && !feof(fp);)
    {
        if(fscanf(fp, "%"PRId64, &baseSeeds[i]) == 1) i++;
        else while(!feof(fp) && fgetc(fp) != '\n');
    }

    fclose(fp);

    return baseSeeds;
}


static void *search4QuadBasesThread(void *data)
{
    quad_threadinfo_t info = *(quad_threadinfo_t*)data;

    const int64_t lower = info.quality;
    const int64_t upper = 23-info.quality;
    const int64_t start = info.start;
    const int64_t end   = info.end;
    const int64_t structureSeed = info.structureSeed;

    int64_t seed;

    int64_t *lowerBits;
    int lowerBitsCnt;
    int lowerBitsIdx = 0;
    int i;

    lowerBits = (int64_t *) malloc(0x10000 * sizeof(int64_t));

    if(info.quality == 1)
    {
        lowerBitsCnt = sizeof(lowerBaseBitsQ1) / sizeof(lowerBaseBitsQ1[0]);
        for(i = 0; i < lowerBitsCnt; i++)
        {
            lowerBits[i] = (lowerBaseBitsQ1[i] - structureSeed) & 0xffff;
        }
    }
    else if(info.quality == 2)
    {
        lowerBitsCnt = sizeof(lowerBaseBitsQ2) / sizeof(lowerBaseBitsQ2[0]);
        for(i = 0; i < lowerBitsCnt; i++)
        {
            lowerBits[i] = (lowerBaseBitsQ2[i] - structureSeed) & 0xffff;
        }
    }
    else
    {
        printf("WARN search4QuadBasesThread: "
               "Lower bits for quality %d have not been defined => "
               "will try all combinations.\n", info.quality);

        lowerBitsCnt = 0x10000;
        for(i = 0; i < lowerBitsCnt; i++) lowerBits[i] = i;
    }

    char fnam[256];
    sprintf(fnam, "%s.part%d", info.fnam, info.threadID);

    FILE *fp = fopen(fnam, "a+");
    if (fp == NULL) {
        fprintf(stderr, "Could not open \"%s\" for writing.\n", fnam);
        free(lowerBits);
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

            if(sscanf(last_newline, "%"PRId64, &seed) == 1)
            {
                while(lowerBits[lowerBitsIdx] <= (seed & 0xffff))
                    lowerBitsIdx++;

                seed = (seed & 0x0000ffffffff0000) + lowerBits[lowerBitsIdx];

                printf("Thread %d starting from: %"PRId64"\n", info.threadID, seed);
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
        if(isQuadFeatureBase(structureSeed, seed, lower, upper))
        {
            fprintf(fp, "%"PRId64"\n", seed);
            fflush(fp);
            //printf("Thread %d: %"PRId64"\n", info.threadID, seed);
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
    free(lowerBits);

    return NULL;
}


void search4QuadBases(const char *fnam, const int threads,
        const int64_t structureSeed, const int quality)
{
    pthread_t threadID[threads];
    quad_threadinfo_t info[threads];
    int64_t t;

    for(t = 0; t < threads; t++)
    {
        info[t].threadID = t;
        info[t].start = (t * SEEDMAX / threads) & 0x0000ffffffff0000;
        info[t].end = ((info[t].start + (SEEDMAX-1) / threads) & 0x0000ffffffff0000) + 1;
        info[t].fnam = fnam;
        info[t].quality = quality;
        info[t].structureSeed = structureSeed;
    }

    for(t = 0; t < threads; t++)
    {
        pthread_create(&threadID[t], NULL, search4QuadBasesThread, (void*)&info[t]);
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
            perror("ERR search4QuadBases: ");
            break;
        }

        while((n = fread(buffer, sizeof(char), 4096, fpart)))
        {
            if(!fwrite(buffer, sizeof(char), n, fp))
            {
                perror("ERR search4QuadBases: ");
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
 * -------------
 * Returns the biome for the specified block position.
 * (Alternatives should be considered in performance critical code.)
 * This function is not threadsafe.
 */
int getBiomeAtPos(const LayerStack g, const Pos pos)
{
    static int ints[0x1000];

    genArea(&g.layers[g.layerNum-1], &ints[0], pos.x, pos.z, 1, 1);

    return ints[0];
}

/* getStructurePos
 * ---------------
 * Fast implementation for finding the block position at which the structure
 * generation attempt will occur in the specified region.
 * This function applies for scattered-feature structureSeeds and villages.
 */
Pos getStructurePos(const int64_t structureSeed, int64_t seed,
        const int64_t regionX, const int64_t regionZ)
{
    Pos pos;

    // set seed
    seed = regionX*341873128712 + regionZ*132897987541 + seed + structureSeed;
    seed = (seed ^ 0x5DEECE66DLL);// & ((1LL << 48) - 1);

    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    pos.x = (seed >> 17) % 24;

    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    pos.z = (seed >> 17) % 24;

    pos.x = regionX*512 + (pos.x << 4) + 8;
    pos.z = regionZ*512 + (pos.z << 4) + 8;
    return pos;
}


/* getStructureChunkInRegion
 * -------------------------
 * Finds the chunk position within the specified region (32x32 chunks) where
 * the structure generation attempt will occur.
 * This function applies for scattered-feature structureSeeds and villages.
 */
Pos getStructureChunkInRegion(const int64_t structureSeed, int64_t seed,
        const int regionX, const int regionZ)
{
    /*
    // Vanilla like implementation.
    seed = regionX*341873128712 + regionZ*132897987541 + seed + structureSeed;
    setSeed(&(seed));

    Pos pos;
    pos.x = nextInt(&seed, 24);
    pos.z = nextInt(&seed, 24);
    */
    Pos pos;

    // set seed
    seed = regionX*341873128712 + regionZ*132897987541 + seed + structureSeed;
    seed = (seed ^ 0x5DEECE66DLL);// & ((1LL << 48) - 1);

    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    pos.x = (seed >> 17) % 24;

    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    pos.z = (seed >> 17) % 24;

    return pos;
}


/* getOceanMonumentChunk
 * ---------------------
 * Fast implementation for finding the chunk relative to the region at which the
 * ocean monument generation attempt will occur.
 */

Pos getOceanMonumentChunk(int64_t seed, const int64_t regionX, const int64_t regionZ)
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
Pos getOceanMonumentPos(int64_t seed, const int64_t regionX, const int64_t regionZ)
{
    Pos pos;

    // set seed
    seed = regionX*341873128712 + regionZ*132897987541 + seed + MONUMENT_SEED;
    seed = (seed ^ 0x5DEECE66DLL) & ((1LL << 48) - 1);

    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    pos.x = (seed >> 17) % 27;
    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    pos.x += (seed >> 17) % 27;

    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    pos.z = (seed >> 17) % 27;
    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
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
Pos getMansionPos(int64_t seed, const int64_t area80X, const int64_t area80Z)
{
    Pos pos;

    // set seed
    seed = area80X*341873128712 + area80Z*132897987541 + seed + MANSION_SEED;
    seed = (seed ^ 0x5DEECE66DLL);// & ((1LL << 48) - 1);

    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    pos.x = (seed >> 17) % 60;
    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    pos.x += (seed >> 17) % 60;

    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
    pos.z = (seed >> 17) % 60;
    seed = (seed * 0x5DEECE66DLL + 0xBLL) & 0xffffffffffff;
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
        int64_t *seed,
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

    out.x = centerX;
    out.z = centerZ;
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


int* getValidStrongholdBiomes() {
    static int validStrongholdBiomes[256];

    if(!validStrongholdBiomes[plains])
    {
        int id;
        for(id = 0; id < 256; id++)
        {
            if(biomeExists(id) && biomes[id].height > 0.0) validStrongholdBiomes[id] = 1;
        }
    }

    return validStrongholdBiomes;
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
void findStrongholds_pre19(LayerStack *g, int *cache, Pos *locations, int64_t worldSeed)
{
    const int SHNUM = 3;
    int i;
    int *validStrongholdBiomes = getValidStrongholdBiomes();

    setWorldSeed(&g->layers[L_RIVER_MIX_4], worldSeed);

    setSeed(&worldSeed);
    double angle = nextDouble(&worldSeed) * PI * 2.0;


    for(i = 0; i < SHNUM; i++)
    {
        double distance = (1.25 + nextDouble(&worldSeed)) * 32.0;
        int x = (int)round(cos(angle) * distance);
        int z = (int)round(sin(angle) * distance);

        locations[i] = findBiomePosition(*g, cache, (x << 4) + 8, (z << 4) + 8, 112,
                validStrongholdBiomes, &worldSeed, NULL);

        angle += 2 * PI / (double)SHNUM;
    }
}


/* findStrongholds
 * ---------------------
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
int findStrongholds(LayerStack *g, int *cache, Pos *locations, int64_t worldSeed, int maxRadius)
{
    const int SHNUM = 128;
    int i;
    int *validStrongholdBiomes = getValidStrongholdBiomes();

    int currentRing = 0;
    int currentCount = 0;
    int perRing = 3;

    setSeed(&worldSeed);
    double angle = nextDouble(&worldSeed) * PI * 2.0;

    for (i=0; i<SHNUM; i++) {
        double distance = (4.0 * 32.0) +
            (6.0 * currentRing * 32.0) +
            (nextDouble(&worldSeed) - 0.5) * 32 * 2.5;

        if (maxRadius && distance*16 > maxRadius)
            return i;

        int x = (int)round(cos(angle) * distance);
        int z = (int)round(sin(angle) * distance);

        locations[i] = findBiomePosition(*g, cache, (x << 4) + 8, (z << 4) + 8, 112,
                validStrongholdBiomes, &worldSeed, NULL);
        angle += 2 * PI / perRing;
        currentCount++;
        if (currentCount == perRing) {
            // Current ring is complete, move to next ring.
            currentRing++;
            currentCount = 0;
            perRing = perRing + 2*perRing/(currentRing+1);
            if (perRing > SHNUM-i)
                perRing = SHNUM-i;
            angle = angle + nextDouble(&worldSeed) * PI * 2.0;
        }
    }

    return SHNUM;
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
Pos getSpawn(LayerStack *g, int *cache, int64_t worldSeed)
{
    static int isSpawnBiome[0x100];
    Pos spawn;
    int found;
    unsigned int i;

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



int isViableFeaturePos(const LayerStack g, int *cache,
        const int64_t blockX, const int64_t blockZ)
{
    static int map[0x100];
    genArea(&g.layers[L_VORONOI_ZOOM_1], map, blockX, blockZ, 1, 1);

    if(map[0] == jungle || map[0] == jungleHills) return Jungle_Pyramid;
    if(map[0] == swampland) return Swamp_Hut;
    if(map[0] == icePlains || map[0] == coldTaiga) return Igloo;
    if(map[0] == desert || map[0] == desertHills) return Desert_Pyramid;
    if(isOceanic(map[0])) return Ocean_Ruin;

    return 0;
}

int isViableVillagePos(const LayerStack g, int *cache,
        const int64_t blockX, const int64_t blockZ)
{
    static int isVillageBiome[0x100];

    if(!isVillageBiome[villageBiomeList[0]])
    {
        unsigned int i;
        for(i = 0; i < sizeof(villageBiomeList) / sizeof(int); i++)
        {
            isVillageBiome[ villageBiomeList[i] ] = 1;
        }
    }

    return areBiomesViable(g, cache, blockX, blockZ, 0, isVillageBiome);
}

int isViableOceanMonumentPos(const LayerStack g, int *cache,
        const int64_t blockX, const int64_t blockZ)
{
    static int isWaterBiome[0x100];
    static int isDeepOcean[0x100];

    if(!isWaterBiome[oceanMonumentBiomeList[0]])
    {
        unsigned int i;
        for(i = 0; i < sizeof(oceanMonumentBiomeList) / sizeof(int); i++)
        {
            isWaterBiome[ oceanMonumentBiomeList[i] ] = 1;
        }

        isDeepOcean[deepOcean] = 1;
    }

    return areBiomesViable(g, cache, blockX, blockZ, 16, isDeepOcean) &&
            areBiomesViable(g, cache, blockX, blockZ, 29, isWaterBiome);
}

int isViableMansionPos(const LayerStack g, int *cache,
        const int64_t blockX, const int64_t blockZ)
{
    static int isMansionBiome[0x100];

    if(!isMansionBiome[mansionBiomeList[0]])
    {
        unsigned int i;
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
int64_t filterAllTempCats(
        LayerStack *g,
        int *cache,
        const int64_t *seedsIn,
        int64_t *seedsOut,
        const int64_t seedCnt,
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

    int64_t sidx, hits, seed;
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
                setChunkSeed(&layerSpecial, (int64_t)(i+pX), (int64_t)(j+pZ));
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
int64_t filterAllMajorBiomes(
        LayerStack *g,
        int *cache,
        const int64_t *seedsIn,
        int64_t *seedsOut,
        const int64_t seedCnt,
        const int pX,
        const int pZ,
        const unsigned int sX,
        const unsigned int sZ)
{
    Layer *lFilterMushroom = &g->layers[L_ADD_MUSHROOM_ISLAND_256];
    Layer *lFilterBiomes = &g->layers[L_BIOME_256];

    int *map;
    int64_t sidx, seed, hits;
    unsigned int i, id, hasAll;

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
