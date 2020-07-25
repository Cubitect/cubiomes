#include "finders.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>


STRUCT(quad_threadinfo_t)
{
    int64_t start, end;
    StructureConfig sconf;
    int threadID;
    int quality;
    const char *fnam;
};


//==============================================================================
// Globals
//==============================================================================


Biome biomes[256];


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


//==============================================================================
// Saving & Loading Seeds
//==============================================================================


int64_t *loadSavedSeeds(const char *fnam, int64_t *scnt)
{
    FILE *fp = fopen(fnam, "r");

    int64_t seed, i;
    int64_t *baseSeeds;

    if (fp == NULL)
    {
        perror("ERR loadSavedSeeds: ");
        return NULL;
    }

    *scnt = 0;

    while (!feof(fp))
    {
        if (fscanf(fp, "%" PRId64, &seed) == 1) (*scnt)++;
        else while (!feof(fp) && fgetc(fp) != '\n');
    }

    baseSeeds = (int64_t*) calloc(*scnt, sizeof(*baseSeeds));

    rewind(fp);

    for (i = 0; i < *scnt && !feof(fp);)
    {
        if (fscanf(fp, "%" PRId64, &baseSeeds[i]) == 1) i++;
        else while (!feof(fp) && fgetc(fp) != '\n');
    }

    fclose(fp);

    return baseSeeds;
}


//==============================================================================
// Multi-Structure Checks
//==============================================================================

int isQuadFeatureBase(const StructureConfig sconf, const int64_t seed, const int qual)
{
    // seed offsets for the regions (0,0) to (1,1)
    const int64_t reg00base = sconf.salt;
    const int64_t reg01base = 341873128712 + sconf.salt;
    const int64_t reg10base = 132897987541 + sconf.salt;
    const int64_t reg11base = 341873128712 + 132897987541 + sconf.salt;

    const int range = sconf.chunkRange;
    const int upper = range - qual - 1;
    const int lower = qual;

    int64_t s;

    s = (reg00base + seed) ^ 0x5deece66dLL; // & 0xffffffffffff;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    if ((int)(s >> 17) % range < upper) return 0;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    if ((int)(s >> 17) % range < upper) return 0;

    s = (reg01base + seed) ^ 0x5deece66dLL; // & 0xffffffffffff;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    if ((int)(s >> 17) % range > lower) return 0;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    if ((int)(s >> 17) % range < upper) return 0;

    s = (reg10base + seed) ^ 0x5deece66dLL; // & 0xffffffffffff;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    if ((int)(s >> 17) % range < upper) return 0;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    if ((int)(s >> 17) % range > lower) return 0;

    s = (reg11base + seed) ^ 0x5deece66dLL; // & 0xffffffffffff;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    if ((int)(s >> 17) % range > lower) return 0;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    if ((int)(s >> 17) % range > lower) return 0;

    return 1;
}


void checkVec4QuadBases(const StructureConfig sconf, int64_t seeds[256])
{
    const int64_t reg00base = sconf.salt;
    const int64_t reg01base = 341873128712 + sconf.salt;
    const int64_t reg10base = 132897987541 + sconf.salt;
    const int64_t reg11base = 341873128712 + 132897987541 + sconf.salt;

    int i;
    for (i = 0; i < 256; i++)
    {
        int64_t seed = seeds[i];
        int64_t s;
        int dx, dz;

        s = (reg00base + seed) ^ 0x5deece66dLL;
        s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
        int x00 = (int)(s >> 17) % sconf.chunkRange;
        if (x00 <= 16) { seeds[i] = 0; continue; }
        s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
        int z00 = (int)(s >> 17) % sconf.chunkRange;
        if (z00 <= 16) { seeds[i] = 0; continue; }
        x00 -= 32; z00 -= 32;

        s = (reg11base + seed) ^ 0x5deece66dLL;
        s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
        int x11 = (int)(s >> 17) % sconf.chunkRange;
        dx = x11 - x00;
        if (dx >= 16) { seeds[i] = 0; continue; }
        s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
        int z11 = (int)(s >> 17) % sconf.chunkRange;
        dz = z11 - z00;
        if (dz >= 16) { seeds[i] = 0; continue; }
        if (dx*dx + dz*dz >= 16*16) { seeds[i] = 0; continue; }

        s = (reg01base + seed) ^ 0x5deece66dLL;
        s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
        int x01 = (int)(s >> 17) % sconf.chunkRange;
        dx = x01 - x00;
        if (dx >= 16) { seeds[i] = 0; continue; }
        s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
        int z01 = (int)(s >> 17) % sconf.chunkRange;
        z01 -= 32;
        dz = z11 - z01;
        if (dz >= 16) { seeds[i] = 0; continue; }

        s = (reg10base + seed) ^ 0x5deece66dLL;
        s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
        int x10 = (int)(s >> 17) % sconf.chunkRange;
        x10 -= 32;
        dx = x01 - x10;
        if (dx >= 16) { seeds[i] = 0; continue; }
        s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
        int z10 = (int)(s >> 17) % sconf.chunkRange;
        dz = z10 - z01;
        if (dz >= 16) { seeds[i] = 0; continue; }

        dx = (x11 > x01 ? x11 : x01) - (x10 < x00 ? x10 : x00);
        dz = (z11 > z10 ? z11 : z10) - (z01 < z00 ? z01 : z00);
        if (dx*dx + dz*dz >= 256) { seeds[i] = 0; continue; } // 15.5 chunks
    }
}

int isTriFeatureBase(const StructureConfig sconf, const int64_t seed, const int qual)
{
    // seed offsets for the regions (0,0) to (1,1)
    const int64_t reg00base = sconf.salt;
    const int64_t reg01base = 341873128712 + sconf.salt;
    const int64_t reg10base = 132897987541 + sconf.salt;
    const int64_t reg11base = 341873128712 + 132897987541 + sconf.salt;

    const int range = sconf.chunkRange;
    const int upper = range - qual - 1;
    const int lower = qual;

    int64_t s;
    int missing = 0;

    s = (reg00base + seed) ^ 0x5deece66dLL; // & 0xffffffffffff;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    if ((int)(s >> 17) % range < upper ||
        (int)(((s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff) >> 17) % range < upper)
    {
        missing++;
    }

    s = (reg01base + seed) ^ 0x5deece66dLL; // & 0xffffffffffff;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    if ((int)(s >> 17) % range > lower ||
        (int)(((s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff) >> 17) % range < upper)
    {
        if (missing) return 0;
        missing++;
    }

    s = (reg10base + seed) ^ 0x5deece66dLL; // & 0xffffffffffff;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    if ((int)(s >> 17) % range < upper ||
        (int)(((s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff) >> 17) % range > lower)
    {
        if (missing) return 0;
        missing++;
    }

    s = (reg11base + seed) ^ 0x5deece66dLL; // & 0xffffffffffff;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    if ((int)(s >> 17) % range > lower ||
        (int)(((s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff) >> 17) % range > lower)
    {
        if (missing) return 0;
    }

    return 1;
}


int isLargeQuadBase(const StructureConfig sconf, const int64_t seed, const int qual)
{
    // seed offsets for the regions (0,0) to (1,1)
    const int64_t reg00base = sconf.salt;
    const int64_t reg01base = 341873128712 + sconf.salt;
    const int64_t reg10base = 132897987541 + sconf.salt;
    const int64_t reg11base = 341873128712 + 132897987541 + sconf.salt;

    // p1 = nextInt(range); p2 = nextInt(range); pos = (p1+p2)>>1
    const int range = sconf.chunkRange;
    const int rmax  = (qual << 1) + 1;         // p1 <= rmax && p1+p2 <= rmax
    const int rmin2 = (range - qual - 1) << 1; // p1+p2 >= rmin2
    const int rmin1 = rmin2 - range + 1;       // p1 >= rmin1

    int64_t s;
    int p;

    s = (reg00base + seed) ^ 0x5deece66dLL; // & 0xffffffffffff;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p = (int)(s >> 17) % range;
    if (p < rmin1) return 0;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p += (int)(s >> 17) % range;
    if (p < rmin2) return 0;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p = (int)(s >> 17) % range;
    if (p < rmin1) return 0;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p += (int)(s >> 17) % range;
    if (p < rmin2) return 0;

    s = (reg01base + seed) ^ 0x5deece66dLL; // & 0xffffffffffff;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p = (int)(s >> 17) % range;
    if (p > rmax) return 0;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p += (int)(s >> 17) % range;
    if (p > rmax) return 0;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p = (int)(s >> 17) % range;
    if (p < rmin1) return 0;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p += (int)(s >> 17) % range;
    if (p < rmin2) return 0;

    s = (reg10base + seed) ^ 0x5deece66dLL; // & 0xffffffffffff;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p = (int)(s >> 17) % range;
    if (p < rmin1) return 0;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p += (int)(s >> 17) % range;
    if (p < rmin2) return 0;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p = (int)(s >> 17) % range;
    if (p > rmax) return 0;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p += (int)(s >> 17) % range;
    if (p > rmax) return 0;

    s = (reg11base + seed) ^ 0x5deece66dLL; // & 0xffffffffffff;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p = (int)(s >> 17) % range;
    if (p > rmax) return 0;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p += (int)(s >> 17) % range;
    if (p > rmax) return 0;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p = (int)(s >> 17) % range;
    if (p > rmax) return 0;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p += (int)(s >> 17) % range;
    if (p > rmax) return 0;

    return 1;
}


int isLargeTriBase(const StructureConfig sconf, const int64_t seed, const int qual)
{
    // seed offsets for the regions (0,0) to (1,1)
    const int64_t reg00base = sconf.salt;
    const int64_t reg01base = 341873128712 + sconf.salt;
    const int64_t reg10base = 132897987541 + sconf.salt;
    const int64_t reg11base = 341873128712 + 132897987541 + sconf.salt;

    // p1 = nextInt(range); p2 = nextInt(range); pos = (p1+p2)>>1
    const int range = sconf.chunkRange;
    const int rmax  = (qual << 1) + 1;         // p1 <= rmax && p1+p2 <= rmax
    const int rmin2 = (range - qual - 1) << 1; // p1+p2 >= rmin2
    const int rmin1 = rmin2 - range + 1;       // p1 >= rmin1

    int64_t s;
    int p;

    int incomplete = 0;

    s = (reg00base + seed) ^ 0x5deece66dLL; // & 0xffffffffffff;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p = (int)(s >> 17) % range;
    if (p < rmin1) goto incomp11;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p += (int)(s >> 17) % range;
    if (p < rmin2) goto incomp11;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p = (int)(s >> 17) % range;
    if (p < rmin1) goto incomp11;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p += (int)(s >> 17) % range;
    if (p < rmin2) goto incomp11;

    if (0)
    {
        incomp11:
        incomplete = 1;
    }

    s = (reg01base + seed) ^ 0x5deece66dLL; // & 0xffffffffffff;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p = (int)(s >> 17) % range;
    if (p > rmax) goto incomp01;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p += (int)(s >> 17) % range;
    if (p > rmax) goto incomp01;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p = (int)(s >> 17) % range;
    if (p < rmin1) goto incomp01;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p += (int)(s >> 17) % range;
    if (p < rmin2) goto incomp01;

    if (0)
    {
        incomp01:
        if (incomplete) return 0;
        incomplete = 2;
    }

    s = (reg10base + seed) ^ 0x5deece66dLL; // & 0xffffffffffff;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p = (int)(s >> 17) % range;
    if (p < rmin1) goto incomp10;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p += (int)(s >> 17) % range;
    if (p < rmin2) goto incomp10;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p = (int)(s >> 17) % range;
    if (p > rmax) goto incomp10;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p += (int)(s >> 17) % range;
    if (p > rmax) goto incomp10;

    if (0)
    {
        incomp10:
        if (incomplete) return 0;
        incomplete = 3;
    }

    s = (reg11base + seed) ^ 0x5deece66dLL; // & 0xffffffffffff;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p = (int)(s >> 17) % range;
    if (p > rmax) goto incomp00;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p += (int)(s >> 17) % range;
    if (p > rmax) goto incomp00;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p = (int)(s >> 17) % range;
    if (p > rmax) goto incomp00;
    s = (s * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    p += (int)(s >> 17) % range;
    if (p > rmax) goto incomp00;

    if (0)
    {
        incomp00:
        if (incomplete) return 0;
        incomplete = 4;
    }

    return incomplete ? incomplete : -1;
}


/* Calls the correct quad-base finder for the structure config, if available.
 * (Exits program otherwise.)
 */
int isQuadBase(const StructureConfig sconf, const int64_t seed, const int64_t qual)
{
    if ((sconf.chunkRange & (sconf.chunkRange-1)) == 0)
    {
        fprintf(stderr,
                "Quad-finder using power of 2 RNG is not implemented yet.\n");
        exit(-1);
    }

    switch(sconf.properties)
    {
    case 0:
        return isQuadFeatureBase(sconf, seed, qual);
    case LARGE_STRUCT:
        return isLargeQuadBase(sconf, seed, qual);
    default:
        fprintf(stderr,
                "Unknown properties field for structure: 0x%04X\n",
                sconf.properties);
        exit(-1);
    }
}

/* Calls the correct triple-base finder for the structure config, if available.
 * (Exits program otherwise.)
 */
int isTriBase(const StructureConfig sconf, const int64_t seed, const int64_t qual)
{
    if ((sconf.chunkRange & (sconf.chunkRange-1)) == 0)
    {
        fprintf(stderr,
                "Tri-finder using power of 2 RNG is not implemented yet.\n");
        exit(-1);
    }

    switch(sconf.properties)
    {
    case 0:
        return isTriFeatureBase(sconf, seed, qual);
    case LARGE_STRUCT:
        return isLargeTriBase(sconf, seed, qual);
    default:
        fprintf(stderr,
                "Unknown properties field for structure: 0x%04X\n",
                sconf.properties);
        exit(-1);
    }
}



/* Searches for the optimal AFK position given four structures at positions 'p',
 * each of volume (ax,ay,az).
 *
 * Returned is the number of spawning spaces within reach.
 */
int countBlocksInSpawnRange(Pos p[4], const int ax, const int ay, const int az)
{
    int minX = 3e7, minZ = 3e7, maxX = -3e7, maxZ = -3e7;
    int best, i, x, z, px, pz;


    // Find corners
    for (i = 0; i < 4; i++)
    {
        if (p[i].x < minX) minX = p[i].x;
        if (p[i].z < minZ) minZ = p[i].z;
        if (p[i].x > maxX) maxX = p[i].x;
        if (p[i].z > maxZ) maxZ = p[i].z;
    }


    // assume that the search area is bound by the inner corners
    maxX += ax;
    maxZ += az;
    best = 0;

    double thsq = 128.0*128.0 - az*az/4.0;

    for (x = minX; x < maxX; x++)
    {
        for (z = minZ; z < maxZ; z++)
        {
            int inrange = 0;

            for (i = 0; i < 4; i++)
            {
                double dx = p[i].x - (x+0.5);
                double dz = p[i].z - (z+0.5);

                for (px = 0; px < ax; px++)
                {
                    for (pz = 0; pz < az; pz++)
                    {
                        double ddx = px + dx;
                        double ddz = pz + dz;
                        inrange += (ddx*ddx + ddz*ddz <= thsq);
                    }
                }
            }

            if (inrange > best)
            {
                best = inrange;
            }
        }
    }

    return best;
}

#ifdef USE_PTHREAD
static void *search4QuadBasesThread(void *data)
#else
static DWORD WINAPI search4QuadBasesThread(LPVOID data)
#endif
{
    quad_threadinfo_t info = *(quad_threadinfo_t*)data;

    const int64_t start       = info.start;
    const int64_t end         = info.end;
    const StructureConfig stc = info.sconf;

    int64_t seed;

    int64_t *lowerBits;
    int lowerBitsCnt;
    int lowerBitsIdx = 0;
    int i;

    lowerBits = (int64_t *) malloc(0x10000 * sizeof(int64_t));

    if (stc.properties == 0 && stc.chunkRange == 24 && info.quality == 1)
    {
        lowerBitsCnt = sizeof(lowerBaseBitsQ1) / sizeof(lowerBaseBitsQ1[0]);
        for (i = 0; i < lowerBitsCnt; i++)
        {
            lowerBits[i] = (lowerBaseBitsQ1[i] - stc.salt) & 0xffff;
        }
    }
    else if (stc.properties == 0 && stc.chunkRange == 24 && info.quality == 2)
    {
        lowerBitsCnt = sizeof(lowerBaseBitsQ2) / sizeof(lowerBaseBitsQ2[0]);
        for (i = 0; i < lowerBitsCnt; i++)
        {
            lowerBits[i] = (lowerBaseBitsQ2[i] - stc.salt) & 0xffff;
        }
    }
    else if (stc.properties == 0 && stc.chunkRange == 24 && info.quality == -1)
    {
        lowerBitsCnt = 0x100;
        for (i = 0; i < lowerBitsCnt; i++) lowerBits[i] = i << 8;
    }
    else
    {
        printf("WARN search4QuadBasesThread: "
               "Lower bits for this quality and property have not been defined"
               " => trying all combinations.\n");

        lowerBitsCnt = 0x10000;
        for (i = 0; i < lowerBitsCnt; i++) lowerBits[i] = i;
    }

    char fnam[256];
    sprintf(fnam, "%s.part%d", info.fnam, info.threadID);

    FILE *fp = fopen(fnam, "a+");
    if (fp == NULL)
    {
        fprintf(stderr, "Could not open \"%s\" for writing.\n", fnam);
        free(lowerBits);
        exit(-1);
    }

    seed = start;

    // Check the last entry in the file and use it as a starting point if it
    // exists. (I.e. loading the saved progress.)
    int c, nnl = 0;
    char buf[32];
    for (i = 1; i < 32; i++)
    {
        if (fseek(fp, -i, SEEK_END)) break;
        c = fgetc(fp);
        if (c <= 0 || (nnl && c == '\n')) break;
        nnl |= (c != '\n');
    }

    if (i < 32 && !fseek(fp, 1-i, SEEK_END) && fread(buf, i-1, 1, fp) > 0)
    {
        if (sscanf(buf, "%" PRId64, &seed) == 1)
        {
            while (lowerBits[lowerBitsIdx] <= (seed & 0xffff))
                lowerBitsIdx++;

            seed = (seed & 0x0000ffffffff0000) + lowerBits[lowerBitsIdx];

            printf("Thread %d starting from: %" PRId64"\n", info.threadID, seed);
        }
        else
        {
            seed = start;
        }
    }


    fseek(fp, 0, SEEK_END);


    while (seed < end)
    {
        if (info.quality >= 0)
        {
            if (isQuadBase(info.sconf, seed, info.quality))
            {
                fprintf(fp, "%" PRId64"\n", seed);
                fflush(fp);
            }
        }
        else
        {
            int64_t st[0x100];
            int i;
            for (i = 0; i < 0x100; i++)
                st[i] = seed + i;
            checkVec4QuadBases(info.sconf, st);
            for (i = 0; i < 0x100; i++)
            {
                int64_t base = st[i];
                if (base)
                {
                    fprintf(fp, "%" PRId64"\n", base);
                    fflush(fp);
                }
            }
        }

        lowerBitsIdx++;
        if (lowerBitsIdx >= lowerBitsCnt)
        {
            lowerBitsIdx = 0;
            seed += 0x10000;
        }
        seed = (seed & 0x0000ffffffff0000) + lowerBits[lowerBitsIdx];
    }

    fclose(fp);
    free(lowerBits);

#ifdef USE_PTHREAD
    pthread_exit(NULL);
#endif
    return 0;
}


void search4QuadBases(const char *fnam, const int threads,
        const StructureConfig structureConfig, const int quality)
{
    thread_id_t threadID[threads];
    quad_threadinfo_t info[threads];
    int64_t t;

    for (t = 0; t < threads; t++)
    {
        info[t].threadID = t;
        info[t].start = (t * SEED_BASE_MAX / threads) & 0x0000ffffffff0000;
        info[t].end = ((info[t].start + (SEED_BASE_MAX-1) / threads) & 0x0000ffffffff0000) + 1;
        info[t].fnam = fnam;
        info[t].quality = quality;
        info[t].sconf = structureConfig;
    }


#ifdef USE_PTHREAD

    for (t = 0; t < threads; t++)
    {
        pthread_create(&threadID[t], NULL, search4QuadBasesThread, (void*)&info[t]);
    }

    for (t = 0; t < threads; t++)
    {
        pthread_join(threadID[t], NULL);
    }

#else

    for (t = 0; t < threads; t++)
    {
        threadID[t] = CreateThread(NULL, 0, search4QuadBasesThread, (LPVOID)&info[t], 0, NULL);
    }

    WaitForMultipleObjects(threads, threadID, TRUE, INFINITE);

#endif



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

    for (t = 0; t < threads; t++)
    {
        sprintf(fnamThread, "%s.part%d", info[t].fnam, info[t].threadID);

        fpart = fopen(fnamThread, "r");

        if (fpart == NULL)
        {
            perror("ERR search4QuadBases: ");
            break;
        }

        while ((n = fread(buffer, sizeof(char), 4096, fpart)))
        {
            if (!fwrite(buffer, sizeof(char), n, fp))
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


//==============================================================================
// Finding Structure Positions
//==============================================================================


Pos getStructurePos(const StructureConfig config, int64_t seed,
        const int regionX, const int regionZ)
{
    Pos pos;

    // set seed
    seed = regionX*341873128712 + regionZ*132897987541 + seed + config.salt;
    seed = (seed ^ 0x5deece66dLL);// & ((1LL << 48) - 1);

    seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;

    if (config.chunkRange & (config.chunkRange-1))
    {
        pos.x = (int)(seed >> 17) % config.chunkRange;

        seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
        pos.z = (int)(seed >> 17) % config.chunkRange;
    }
    else
    {
        // Java RNG treats powers of 2 as a special case.
        pos.x = (config.chunkRange * (seed >> 17)) >> 31;

        seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
        pos.z = (config.chunkRange * (seed >> 17)) >> 31;
    }

    // Structure is positioned at chunk origin but the biome check is performed
    // at block position (9,9) within chunk. [CHECK: maybe (8,8) in 1.7]
    pos.x = ((regionX*config.regionSize + pos.x) << 4) + 9;
    pos.z = ((regionZ*config.regionSize + pos.z) << 4) + 9;
    return pos;
}

Pos getStructureChunkInRegion(const StructureConfig config, int64_t seed,
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

    seed = regionX*341873128712 + regionZ*132897987541 + seed + config.salt;
    seed = (seed ^ 0x5deece66dLL);// & ((1LL << 48) - 1);

    seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;

    if (config.chunkRange & (config.chunkRange-1))
    {
        pos.x = (int)(seed >> 17) % config.chunkRange;

        seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
        pos.z = (int)(seed >> 17) % config.chunkRange;
    }
    else
    {
        // Java RNG treats powers of 2 as a special case.
        pos.x = (config.chunkRange * (seed >> 17)) >> 31;

        seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
        pos.z = (config.chunkRange * (seed >> 17)) >> 31;
    }

    return pos;
}


Pos getLargeStructurePos(StructureConfig config, int64_t seed,
        const int regionX, const int regionZ)
{
    Pos pos;

    //TODO: power of two chunk ranges...

    // set seed
    seed = regionX*341873128712 + regionZ*132897987541 + seed + config.salt;
    seed = (seed ^ 0x5deece66dLL) & ((1LL << 48) - 1);

    seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    pos.x = (seed >> 17) % config.chunkRange;
    seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    pos.x += (seed >> 17) % config.chunkRange;

    seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    pos.z = (seed >> 17) % config.chunkRange;
    seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    pos.z += (seed >> 17) % config.chunkRange;

    pos.x = regionX*config.regionSize + (pos.x >> 1);
    pos.z = regionZ*config.regionSize + (pos.z >> 1);
    pos.x = pos.x*16 + 9;
    pos.z = pos.z*16 + 9;
    return pos;
}


Pos getLargeStructureChunkInRegion(StructureConfig config, int64_t seed,
        const int regionX, const int regionZ)
{
    Pos pos;

    //TODO: power of two chunk ranges...

    // set seed
    seed = regionX*341873128712 + regionZ*132897987541 + seed + config.salt;
    seed = (seed ^ 0x5deece66dLL) & ((1LL << 48) - 1);

    seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    pos.x = (seed >> 17) % config.chunkRange;
    seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    pos.x += (seed >> 17) % config.chunkRange;

    seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    pos.z = (seed >> 17) % config.chunkRange;
    seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    pos.z += (seed >> 17) % config.chunkRange;

    pos.x >>= 1;
    pos.z >>= 1;

    return pos;
}


int isMineshaftChunk(int64_t seed, const int chunkX, const int chunkZ)
{
    int64_t s = seed;
    setSeed(&s);
    int64_t i = nextLong(&s);
    int64_t j = nextLong(&s);
    s = chunkX * i ^ chunkZ * j ^ seed;
    setSeed(&s);
    return nextDouble(&s) < 0.004;
}

int isTreasureChunk(int64_t seed, const int chunkX, const int chunkZ)
{
    seed = chunkX*341873128712 + chunkZ*132897987541 + seed + TREASURE_CONFIG.salt;
    setSeed(&seed);
    return nextFloat(&seed) < 0.01;
}


//==============================================================================
// Checking Biomes & Biome Helper Functions
//==============================================================================


int getBiomeAtPos(const LayerStack g, const Pos pos)
{
    int *map = allocCache(&g.layers[L_VORONOI_ZOOM_1], 1, 1);
    genArea(&g.layers[L_VORONOI_ZOOM_1], map, pos.x, pos.z, 1, 1);
    int biomeID = map[0];
    free(map);
    return biomeID;
}

Pos findBiomePosition(
        const int mcversion,
        const Layer *l,
        int *cache,
        const int centerX,
        const int centerZ,
        const int range,
        const char *isValid,
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
    int i, j, found;

    Pos out;

    if (l->scale != 4)
    {
        printf("WARN findBiomePosition: require scale = 4, but have %d.\n",
                l->scale);
    }

    map = cache ? cache : allocCache(l, width, height);

    genArea(l, map, x1, z1, width, height);

    out.x = centerX;
    out.z = centerZ;
    found = 0;

    if (mcversion >= MC_1_13)
    {
        for (i = 0, j = 2; i < width*height; i++)
        {
            if (!biomeExists(map[i]) || !isValid[map[i]]) continue;
            if ((found == 0 || nextInt(seed, j++) == 0))
            {
                out.x = (x1 + i%width) << 2;
                out.z = (z1 + i/width) << 2;
                found = 1;
            }
        }
        found = j - 2;
    }
    else
    {
        for (i = 0; i < width*height; i++)
        {
            if (biomeExists(map[i]) && isValid[map[i]] &&
                (found == 0 || nextInt(seed, found + 1) == 0))
            {
                out.x = (x1 + i%width) << 2;
                out.z = (z1 + i/width) << 2;
                ++found;
            }
        }
    }


    if (cache == NULL)
    {
        free(map);
    }

    if (passes != NULL)
    {
        *passes = found;
    }

    return out;
}


int areBiomesViable(
        const Layer *       l,
        int *               cache,
        const int           posX,
        const int           posZ,
        const int           radius,
        const char *        isValid
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

    if (l->scale != 4)
    {
        printf("WARN areBiomesViable: require scale = 4, but have %d.\n",
                l->scale);
    }

    map = cache ? cache : allocCache(l, width, height);
    genArea(l, map, x1, z1, width, height);

    for (i = 0; i < width*height; i++)
    {
        if (!biomeExists(map[i]) || !isValid[ map[i] ])
        {
            if (cache == NULL) free(map);
            return 0;
        }
    }

    if (cache == NULL) free(map);
    return 1;
}


int getBiomeRadius(
        const int *         map,
        const int           mapSide,
        const int *         biomes,
        const int           bnum,
        const int           ignoreMutations)
{
    int r, i, b;
    int blist[0x100];
    int mask = ignoreMutations ? 0x7f : 0xff;
    int radiusMax = mapSide / 2;

    if ((mapSide & 1) == 0)
    {
        printf("WARN getBiomeRadius: Side length of the square map should be an odd integer.\n");
    }

    memset(blist, 0, sizeof(blist));

    for (r = 1; r < radiusMax; r++)
    {
        for (i = radiusMax-r; i <= radiusMax+r; i++)
        {
            blist[ map[(radiusMax-r) * mapSide+ i]    & mask ] = 1;
            blist[ map[(radiusMax+r-1) * mapSide + i] & mask ] = 1;
            blist[ map[mapSide*i + (radiusMax-r)]     & mask ] = 1;
            blist[ map[mapSide*i + (radiusMax+r-1)]   & mask ] = 1;
        }

        for (b = 0; b < bnum && blist[biomes[b] & mask]; b++);
        if (b >= bnum)
        {
            break;
        }
    }

    return r != radiusMax ? r : -1;
}



//==============================================================================
// Finding Strongholds and Spawn
//==============================================================================


const char* getValidStrongholdBiomes()
{
    static char validStrongholdBiomes[256];

    if (!validStrongholdBiomes[plains])
    {
        int id;
        for (id = 0; id < 256; id++)
        {
            if (biomeExists(id) && biomes[id].height > 0.0)
                validStrongholdBiomes[id] = 1;
        }
    }

    return validStrongholdBiomes;
}


int findStrongholds(const int mcversion, const LayerStack *g, int *cache,
        Pos *locations, int64_t worldSeed, int maxSH, const int maxRadius)
{
    const char *validStrongholdBiomes = getValidStrongholdBiomes();
    int i, x, z;
    double distance;

    int currentRing = 0;
    int currentCount = 0;
    int perRing = 3;

    setSeed(&worldSeed);
    double angle = nextDouble(&worldSeed) * PI * 2.0;

    const Layer *l = &g->layers[L_RIVER_MIX_4];

    if (mcversion >= MC_1_9)
    {
        if (maxSH <= 0) maxSH = 128;

        for (i = 0; i < maxSH; i++)
        {
            distance = (4.0 * 32.0) + (6.0 * currentRing * 32.0) +
                (nextDouble(&worldSeed) - 0.5) * 32 * 2.5;

            if (maxRadius && distance*16 > maxRadius)
                return i;

            x = (int)round(cos(angle) * distance);
            z = (int)round(sin(angle) * distance);

            locations[i] = findBiomePosition(mcversion, l, cache,
                    (x << 4) + 8, (z << 4) + 8, 112, validStrongholdBiomes,
                    &worldSeed, NULL);

            angle += 2 * PI / perRing;

            currentCount++;
            if (currentCount == perRing)
            {
                // Current ring is complete, move to next ring.
                currentRing++;
                currentCount = 0;
                perRing = perRing + 2*perRing/(currentRing+1);
                if (perRing > 128-i)
                    perRing = 128-i;
                angle = angle + nextDouble(&worldSeed) * PI * 2.0;
            }
        }
    }
    else
    {
        if (maxSH <= 0) maxSH = 3;

        for (i = 0; i < maxSH; i++)
        {
            distance = (1.25 + nextDouble(&worldSeed)) * 32.0;

            if (maxRadius && distance*16 > maxRadius)
                return i;

            x = (int)round(cos(angle) * distance);
            z = (int)round(sin(angle) * distance);

            locations[i] = findBiomePosition(mcversion, l, cache,
                    (x << 4) + 8, (z << 4) + 8, 112, validStrongholdBiomes,
                    &worldSeed, NULL);

            angle += 2 * PI / 3.0;
        }
    }

    return maxSH;
}


static double getGrassProbability(int64_t seed, int biome, int x, int z)
{
    // TODO: Use ChunkGeneratorOverworld.generateHeightmap for better estimate.
    // TODO: Try to determine the actual probabilities and build a statistic.
    switch (biome)
    {
    case plains:                        return 1.0;
    case mountains:                     return 0.8; // height dependent
    case forest:                        return 1.0;
    case taiga:                         return 1.0;
    case swamp:                         return 0.6; // height dependent
    case river:                         return 0.5;
    case beach:                         return 0.1;
    case wooded_hills:                  return 1.0;
    case taiga_hills:                   return 1.0;
    case mountain_edge:                 return 1.0; // height dependent
    case jungle:                        return 1.0;
    case jungle_hills:                  return 1.0;
    case jungle_edge:                   return 1.0;
    case birch_forest:                  return 1.0;
    case birch_forest_hills:            return 1.0;
    case dark_forest:                   return 0.9;
    case snowy_taiga:                   return 0.2; // below trees
    case snowy_taiga_hills:             return 0.2; // below trees
    case giant_tree_taiga:              return 0.6;
    case giant_tree_taiga_hills:        return 0.6;
    case wooded_mountains:              return 0.2; // height dependent
    case savanna:                       return 1.0;
    case savanna_plateau:               return 1.0;
    case wooded_badlands_plateau:       return 0.1; // height dependent
    case badlands_plateau:              return 0.1; // height dependent

    case sunflower_plains:              return 1.0;
    case gravelly_mountains:            return 0.2;
    case flower_forest:                 return 1.0;
    case taiga_mountains:               return 1.0;
    case swamp_hills:                   return 0.9;
    case modified_jungle:               return 1.0;
    case modified_jungle_edge:          return 1.0;
    case tall_birch_forest:             return 1.0;
    case tall_birch_hills:              return 1.0;
    case dark_forest_hills:             return 0.9;
    case snowy_taiga_mountains:         return 0.2;
    case giant_spruce_taiga:            return 0.6;
    case giant_spruce_taiga_hills:      return 0.6;
    case modified_gravelly_mountains:   return 0.2;
    case shattered_savanna:             return 1.0;
    case shattered_savanna_plateau:     return 1.0;
    case bamboo_jungle:                 return 0.4;
    case bamboo_jungle_hills:           return 0.4;
    // NOTE: in rare circumstances you can get also get grassy islands that are
    // completely in ocean variants...
    default: return 0;
    }
}

static int canCoordinateBeSpawn(const int64_t seed, const LayerStack *g, int *cache, Pos pos)
{
    int biome = getBiomeAtPos(*g, pos);
    return getGrassProbability(seed, biome, pos.x, pos.z) >= 0.5;
}


static const char* getValidSpawnBiomes()
{
    static const int biomesToSpawnIn[] = {forest, plains, taiga, taiga_hills, wooded_hills, jungle, jungle_hills};
    static char isValid[256];
    unsigned int i;

    if (!isValid[biomesToSpawnIn[0]])
        for (i = 0; i < sizeof(biomesToSpawnIn) / sizeof(int); i++)
            isValid[ biomesToSpawnIn[i] ] = 1;

    return isValid;
}


Pos getSpawn(const int mcversion, const LayerStack *g, int *cache, int64_t worldSeed)
{
    const char *isSpawnBiome = getValidSpawnBiomes();
    Pos spawn;
    int found;
    int i;

    const Layer *l = &g->layers[L_RIVER_MIX_4];

    setSeed(&worldSeed);
    spawn = findBiomePosition(mcversion, l, cache, 0, 0, 256, isSpawnBiome,
            &worldSeed, &found);

    if (!found)
    {
        //printf("Unable to find spawn biome.\n");
        spawn.x = spawn.z = 8;
    }

    if (mcversion >= MC_1_13)
    {
        // TODO: The 1.13 section may need further checking!
        int n2 = 0;
        int n3 = 0;
        int n4 = 0;
        int n5 = -1;

        for (i = 0; i < 1024; i++)
        {
            if (n2 > -16 && n2 <= 16 && n3 > -16 && n3 <= 16)
            {
                int cx = ((spawn.x >> 4) + n2) << 4;
                int cz = ((spawn.z >> 4) + n3) << 4;
                int i2, i3;

                for (i2 = cx; i2 <= cx+15; i2++)
                {
                    for (i3 = cz; i3 <= cz+15; i3++)
                    {
                        Pos pos = {i2, i3};
                        if (canCoordinateBeSpawn(worldSeed, g, cache, pos))
                        {
                            return pos;
                        }
                    }
                }
            }

            if (n2 == n3 || (n2 < 0 && n2 == - n3) || (n2 > 0 && n2 == 1 - n3))
            {
                int n7 = n4;
                n4 = - n5;
                n5 = n7;
            }
            n2 += n4;
            n3 += n5;
        }
    }
    else
    {
        for (i = 0; i < 1000 && !canCoordinateBeSpawn(worldSeed, g, cache, spawn); i++)
        {
            spawn.x += nextInt(&worldSeed, 64) - nextInt(&worldSeed, 64);
            spawn.z += nextInt(&worldSeed, 64) - nextInt(&worldSeed, 64);
        }
    }

    return spawn;
}


Pos estimateSpawn(const int mcversion, const LayerStack *g, int *cache, int64_t worldSeed)
{
    const char *isSpawnBiome = getValidSpawnBiomes();
    Pos spawn;
    int found;

    Layer *l = &g->layers[L_RIVER_MIX_4];

    setSeed(&worldSeed);
    spawn = findBiomePosition(mcversion, l, cache, 0, 0, 256, isSpawnBiome,
            &worldSeed, &found);

    if (!found)
    {
        spawn.x = spawn.z = 8;
    }

    return spawn;
}



//==============================================================================
// Validating Structure Positions
//==============================================================================


int isViableFeatureBiome(int structureType, int biomeID)
{
    switch (structureType)
    {
    case Desert_Pyramid:
        return biomeID == desert || biomeID == desert_hills;
    case Igloo:
        return biomeID == snowy_tundra || biomeID == snowy_taiga;
    case Jungle_Pyramid:
        return biomeID == jungle || biomeID == jungle_hills || biomeID == bamboo_jungle || biomeID == bamboo_jungle_hills;
    case Swamp_Hut:
        return biomeID == swamp;
    case Ocean_Ruin:
        return isOceanic(biomeID);
    case Shipwreck:
        return isOceanic(biomeID) || biomeID == beach || biomeID == snowy_beach;
    case Ruined_Portal:
        return 1;
    case Treasure:
        return biomeID == beach || biomeID == snowy_beach || biomeID == stone_shore || biomeID == mushroom_field_shore;
    default:
        fprintf(stderr, "ERR isViableFeatureBiome: not implemented for structure type.\n");
        exit(1);
    }
    return 0;
}

static const char *getValidMonumentBiomes1()
{
    static const int oceanMonumentBiomeList1[] =
    {
            ocean, deep_ocean, river, frozen_river,
            frozen_ocean, deep_frozen_ocean, cold_ocean, deep_cold_ocean,
            lukewarm_ocean, deep_lukewarm_ocean, warm_ocean, deep_warm_ocean
    };
    static char isValid[256];
    unsigned int i;

    if (!isValid[oceanMonumentBiomeList1[0]])
        for (i = 0; i < sizeof(oceanMonumentBiomeList1) / sizeof(int); i++)
            isValid[ oceanMonumentBiomeList1[i] ] = 1;

    return isValid;
}

static const char *getValidMonumentBiomes2()
{
    static const int oceanMonumentBiomeList2[] =
    {
            deep_frozen_ocean, deep_cold_ocean, deep_ocean,
            deep_lukewarm_ocean, deep_warm_ocean
    };
    static char isValid[256];
    unsigned int i;

    if (!isValid[oceanMonumentBiomeList2[0]])
        for (i = 0; i < sizeof(oceanMonumentBiomeList2) / sizeof(int); i++)
            isValid[ oceanMonumentBiomeList2[i] ] = 1;

    return isValid;
}

static const char *getValidMansionBiomes()
{
    static const int mansionBiomeList[] = {dark_forest, dark_forest+128};
    static char isValid[256];
    unsigned int i;

    if (!isValid[mansionBiomeList[0]])
        for (i = 0; i < sizeof(mansionBiomeList) / sizeof(int); i++)
            isValid[ mansionBiomeList[i] ] = 1;

    return isValid;
}


int isViableStructurePos(const StructureConfig sconf, int mcversion,
        LayerStack *g, int64_t seed, int blockX, int blockZ)
{
    int biomeID;
    int *map = NULL;
    Layer *l;

    int64_t chunkX = blockX >> 4;
    int64_t chunkZ = blockZ >> 4;

    // TODO: where possible perform cheaper biome checks before full validation

    switch (sconf.structType)
    {
    case Desert_Pyramid:
    case Igloo:
    case Jungle_Pyramid:
    case Swamp_Hut:
    case Ocean_Ruin:
    case Shipwreck:
    case Ruined_Portal:
    case Treasure:
        if (mcversion < MC_1_16)
        {
            l = &g->layers[L_VORONOI_ZOOM_1];
            setWorldSeed(l, seed);
            map = allocCache(l, 1, 1);
            genArea(l, map, blockX, blockZ, 1, 1);
        }
        else // position and layer check changed position in 1.16
        {
            l = &g->layers[L_RIVER_MIX_4]; // oceanic checks should be fine here
            setWorldSeed(l, seed);
            map = allocCache(l, 1, 1);
            genArea(l, map, (chunkX << 2) + 2, (chunkZ << 2) + 2, 1, 1);
        }
        biomeID = map[0];
        free(map);
        return isViableFeatureBiome(sconf.structType, biomeID);

    case Village:
        l = &g->layers[L_RIVER_MIX_4];
        setWorldSeed(l, seed);
        map = allocCache(l, 1, 1);
        if (mcversion < MC_1_16) // TODO: check this (and if it makes a difference)
            genArea(l, map, blockX >> 2, blockZ >> 2, 1, 1);
        else
            genArea(l, map, (chunkX << 2) + 2, (chunkZ << 2) + 2, 1, 1);
        biomeID = map[0];
        free(map);
        if (biomeID == plains || biomeID == desert || biomeID == savanna || biomeID == taiga)
            return 1;
        if (mcversion >= MC_1_14 && biomeID == snowy_tundra)
            return 1;
        if (mcversion == MC_BE && biomeID == snowy_taiga)
            return 1;
        return 0;

    case Outpost:
    {
        int64_t rnds = seed;
        rnds ^= ((chunkX >> 4) ^ (chunkZ >> 4)) << 4;
        setSeed(&rnds);
        next(&rnds, 32);
        if (nextInt(&rnds, 5) != 0)
            return 0;
        if (mcversion < MC_1_16)
        {
            l = &g->layers[L_VORONOI_ZOOM_1];
            setWorldSeed(l, seed);
            map = allocCache(l, 1, 1);
            genArea(l, map, blockX, blockZ, 1, 1);
        }
        else
        {
            l = &g->layers[L_RIVER_MIX_4]; // oceanic checks should be fine here
            setWorldSeed(l, seed);
            map = allocCache(l, 1, 1);
            genArea(l, map, (chunkX << 2) + 2, (chunkZ << 2) + 2, 1, 1);
        }
        biomeID = map[0];
        free(map);
        // TODO: support for MC_BE
        if (biomeID != plains && biomeID != desert && biomeID != taiga && biomeID != snowy_tundra && biomeID != savanna)
            return 0;
        // look for villages within 10 chunks
        int cx0 = (chunkX-10), cx1 = (chunkX+10);
        int cz0 = (chunkZ-10), cz1 = (chunkZ+10);
        int rx, rz;
        for (rz = cz0 >> 5; rz <= cz1 >> 5; rz++)
        {
            for (rx = cx0 >> 5; rx <= cx1 >> 5; rx++)
            {
                Pos p = getStructurePos(VILLAGE_CONFIG, seed, rx, rz);
                int cx = p.x >> 4, cz = p.z >> 4;
                if (cx >= cx0 && cx <= cx1 && cz >= cz0 && cz <= cz1)
                {
                    if (mcversion >= MC_1_16)
                        return 0;
                    if (isViableStructurePos(VILLAGE_CONFIG, mcversion, g, seed, p.x, p.z))
                        return 0;
                    return 1;
                }
            }
        }
        return 1;
    }

    case Monument:
        if (mcversion >= MC_1_13)
            l = &g->layers[L13_OCEAN_MIX_4];
        else
            l = &g->layers[L_RIVER_MIX_4];
        setWorldSeed(l, seed);
        if (areBiomesViable(l, NULL, blockX, blockZ, 16, getValidMonumentBiomes1()))
            if (areBiomesViable(l, NULL, blockX, blockZ, 29, getValidMonumentBiomes2()))
                return 1;
        return 0;

    case Mansion:
        l = &g->layers[L_RIVER_MIX_4];
        setWorldSeed(l, seed);
        return areBiomesViable(l, NULL, blockX, blockZ, 32, getValidMansionBiomes());

    default:
        fprintf(stderr, "ERR isViableStructurePos: validation for structure type not implemented");
        return 0;
    }
}


//==============================================================================
// Finding Properties of Structures
//==============================================================================


int isZombieVillage(const int mcversion, const int64_t worldSeed,
        const int regionX, const int regionZ)
{
    Pos pos;
    int64_t seed = worldSeed;

    if (mcversion < MC_1_10)
    {
        printf("Warning: Zombie villages were only introduced in MC 1.10.\n");
    }

    // get the chunk position of the village
    seed = regionX*341873128712 + regionZ*132897987541 + seed + VILLAGE_CONFIG.salt;
    seed = (seed ^ 0x5deece66dLL);// & ((1LL << 48) - 1);

    seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    pos.x = (seed >> 17) % VILLAGE_CONFIG.chunkRange;

    seed = (seed * 0x5deece66dLL + 0xbLL) & 0xffffffffffff;
    pos.z = (seed >> 17) % VILLAGE_CONFIG.chunkRange;

    pos.x += regionX * VILLAGE_CONFIG.regionSize;
    pos.z += regionZ * VILLAGE_CONFIG.regionSize;

    // jump to the random number check that determines whether this is village
    // is zombie infested
    int64_t rnd = chunkGenerateRnd(worldSeed, pos.x , pos.z);
    skipNextN(&rnd, mcversion == MC_1_13 ? 10 : 11);

    return nextInt(&rnd, 50) == 0;
}


int64_t getHouseList(const int64_t worldSeed, const int chunkX, const int chunkZ,
        int *out)
{
    int64_t rnd = chunkGenerateRnd(worldSeed, chunkX, chunkZ);
    skipNextN(&rnd, 1);

    out[HouseSmall] = nextInt(&rnd, 4 - 2 + 1) + 2;
    out[Church]     = nextInt(&rnd, 1 - 0 + 1) + 0;
    out[Library]    = nextInt(&rnd, 2 - 0 + 1) + 0;
    out[WoodHut]    = nextInt(&rnd, 5 - 2 + 1) + 2;
    out[Butcher]    = nextInt(&rnd, 2 - 0 + 1) + 0;
    out[FarmLarge]  = nextInt(&rnd, 4 - 1 + 1) + 1;
    out[FarmSmall]  = nextInt(&rnd, 4 - 2 + 1) + 2;
    out[Blacksmith] = nextInt(&rnd, 1 - 0 + 1) + 0;
    out[HouseLarge] = nextInt(&rnd, 3 - 0 + 1) + 0;

    return rnd;
}

//==============================================================================
// Seed Filters
//==============================================================================


BiomeFilter setupBiomeFilter(const int *biomeList, int listLen)
{
    BiomeFilter bf;
    int i, id;

    memset(&bf, 0, sizeof(bf));

    for (i = 0; i < listLen; i++)
    {
        id = biomeList[i] & 0x7f;
        switch (id)
        {
        case mushroom_fields:
        case mushroom_field_shore:
            bf.requireMushroom = 1;
            bf.tempCat |= (1ULL << Oceanic);
            bf.biomesToFind |= (1ULL << id);
        case badlands:
        case wooded_badlands_plateau:
        case badlands_plateau:
            bf.tempCat |= (1ULL << (Warm+Special));
            bf.biomesToFind |= (1ULL << id);
            break;
        case savanna:
        case savanna_plateau:
            bf.tempCat |= (1ULL << Warm);
            bf.biomesToFind |= (1ULL << id);
            break;
        case dark_forest:
        case birch_forest:
        case birch_forest_hills:
        case swamp:
            bf.tempCat |= (1ULL << Lush);
            bf.biomesToFind |= (1ULL << id);
            break;
        case jungle:
        case jungle_hills:
            bf.tempCat |= (1ULL << (Lush+Special));
            bf.biomesToFind |= (1ULL << id);
            break;
        /*case jungleEdge:
            bf.tempCat |= (1ULL << Lush) | (1ULL << Lush+4);
            bf.biomesToFind |= (1ULL << id);
            break;*/
        case giant_tree_taiga:
        case giant_tree_taiga_hills:
            bf.tempCat |= (1ULL << (Cold+Special));
            bf.biomesToFind |= (1ULL << id);
            break;
        case snowy_tundra:
        case snowy_mountains:
        case snowy_taiga:
        case snowy_taiga_hills:
            bf.tempCat |= (1ULL << Freezing);
            bf.biomesToFind |= (1ULL << id);
            break;
        default:
            bf.biomesToFind |= (1ULL << id);

            if (isOceanic(id))
            {
                if (id != ocean && id != deep_ocean)
                    bf.doOceanTypeCheck = 1;

                if (isShallowOcean(id))
                {
                    bf.oceansToFind |= (1ULL << id);
                }
                else
                {
                    if (id == deep_warm_ocean)
                        bf.oceansToFind |= (1ULL << warm_ocean);
                    else if (id == deep_lukewarm_ocean)
                        bf.oceansToFind |= (1ULL << lukewarm_ocean);
                    else if (id == deep_ocean)
                        bf.oceansToFind |= (1ULL << ocean);
                    else if (id == deep_cold_ocean)
                        bf.oceansToFind |= (1ULL << cold_ocean);
                    else if (id == deep_frozen_ocean)
                        bf.oceansToFind |= (1ULL << frozen_ocean);
                }
                bf.tempCat |= (1ULL << Oceanic);
            }
            else
            {
                bf.biomesToFind |= (1ULL << id);
            }
            break;
        }
    }

    for (i = 0; i < Special; i++)
    {
        if (bf.tempCat & (1ULL << i)) bf.tempNormal++;
    }
    for (i = Special; i < Freezing+Special; i++)
    {
        if (bf.tempCat & (1ULL << i)) bf.tempSpecial++;
    }

    bf.doTempCheck = (bf.tempSpecial + bf.tempNormal) >= 6;
    bf.doShroomAndTempCheck = bf.requireMushroom && (bf.tempSpecial >= 1 || bf.tempNormal >= 4);
    bf.doMajorBiomeCheck = 1;
    bf.checkBiomePotential = 1;
    bf.doScale4Check = 1;

    return bf;
}



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
        const int           minscale)
{
    Layer *lspecial = &g->layers[L_SPECIAL_1024];
    Layer *lmushroom = &g->layers[L_ADD_MUSHROOM_256];
    Layer *lbiomes = &g->layers[L_BIOME_256];
    Layer *loceantemp = NULL;

    int *map = cache ? cache : allocCache(&g->layers[L_VORONOI_ZOOM_1], width, height);

    uint64_t potential, required, modified;
    int64_t ss, cs;
    int id, types[0x100];
    int i, x, z;
    int areaX1024, areaZ1024, areaWidth1024, areaHeight1024;
    int areaX256, areaZ256, areaWidth256, areaHeight256;
    int areaX4, areaZ4, areaWidth4, areaHeight4;

    // 1:1024 scale
    areaX1024 = blockX >> 10;
    areaZ1024 = blockZ >> 10;
    areaWidth1024 = ((width-1) >> 10) + 2;
    areaHeight1024 = ((height-1) >> 10) + 2;

    // 1:256 scale
    areaX256 = blockX >> 8;
    areaZ256 = blockZ >> 8;
    areaWidth256 = ((width-1) >> 8) + 2;
    areaHeight256 = ((height-1) >> 8) + 2;


    /*** BIOME CHECKS THAT DON'T NEED OTHER LAYERS ***/

    // Check that there is the necessary minimum of both special and normal
    // temperature categories present.
    if (filter.tempNormal || filter.tempSpecial)
    {
        ss = getStartSeed(seed, lspecial->layerSeed);

        types[0] = types[1] = 0;
        for (z = 0; z < areaHeight1024; z++)
        {
            for (x = 0; x < areaWidth1024; x++)
            {
                cs = getChunkSeed(ss, x + areaX1024, z + areaZ1024);
                types[mcFirstInt(cs, 13) == 0]++;
            }
        }

        if (types[0] < filter.tempNormal || types[1] < filter.tempSpecial)
        {
            goto return_zero;
        }
    }

    // Check there is a mushroom island, provided there is an ocean.
    if (filter.requireMushroom)
    {
        ss = getStartSeed(seed, lmushroom->layerSeed);

        for (z = 0; z < areaHeight256; z++)
        {
            for (x = 0; x < areaWidth256; x++)
            {
                cs = getChunkSeed(ss, x + areaX256, z + areaZ256);
                if (mcFirstInt(cs, 100) == 0)
                {
                    goto after_protomushroom;
                }
            }
        }

        goto return_zero;
    }
    after_protomushroom:

    if (filter.checkBiomePotential)
    {
        ss = getStartSeed(seed, lbiomes->layerSeed);

        potential = 0;
        required = filter.biomesToFind & (
                (1ULL << badlands_plateau) | (1ULL << wooded_badlands_plateau) |
                (1ULL << savanna)          | (1ULL << dark_forest) |
                (1ULL << birch_forest)     | (1ULL << swamp));

        for (z = 0; z < areaHeight256; z++)
        {
            for (x = 0; x < areaWidth256; x++)
            {
                cs = getChunkSeed(ss, x + areaX256, z + areaZ256);
                cs >>= 24;

                int cs6 = cs % 6;
                int cs3 = cs6 & 3;

                if (cs3 == 0) potential |= (1ULL << badlands_plateau);
                else if (cs3 == 1 || cs3 == 2) potential |= (1ULL << wooded_badlands_plateau);

                if (cs6 == 1) potential |= (1ULL << dark_forest);
                else if (cs6 == 3) potential |= (1ULL << savanna);
                else if (cs6 == 4) potential |= (1ULL << savanna) | (1ULL << birch_forest);
                else if (cs6 == 5) potential |= (1ULL << swamp);

                if (!((potential & required) ^ required))
                {
                    goto after_protobiome;
                }
            }
        }

        goto return_zero;
    }
    after_protobiome:


    /*** BIOME CHECKS ***/

    if (filter.doTempCheck)
    {
        setWorldSeed(lspecial, seed);
        genArea(lspecial, map, areaX1024, areaZ1024, areaWidth1024, areaHeight1024);

        potential = 0;

        for (i = 0; i < areaWidth1024 * areaHeight1024; i++)
        {
            id = map[i];
            if (id >= Special) id = (id & 0xf) + Special;
            potential |= (1ULL << id);
        }

        if ((potential & filter.tempCat) ^ filter.tempCat)
        {
            goto return_zero;
        }
    }

    if (minscale > 256) goto return_one;

    if (filter.doShroomAndTempCheck)
    {
        setWorldSeed(lmushroom, seed);
        genArea(lmushroom, map, areaX256, areaZ256, areaWidth256, areaHeight256);

        potential = 0;

        for (i = 0; i < areaWidth256 * areaHeight256; i++)
        {
            id = map[i];
            if (id >= BIOME_NUM) id = (id & 0xf) + Special;
            potential |= (1ULL << id);
        }

        required = filter.tempCat | (1ULL << mushroom_fields);

        if ((potential & required) ^ required)
        {
            goto return_zero;
        }
    }

    if (filter.doOceanTypeCheck)
    {
        loceantemp = &g->layers[L13_OCEAN_TEMP_256];
        setWorldSeed(loceantemp, seed);
        genArea(loceantemp, map, areaX256, areaZ256, areaWidth256, areaHeight256);

        potential = 0; // ocean potential

        for (i = 0; i < areaWidth256 * areaHeight256; i++)
        {
            id = map[i];
            if (id == warm_ocean)     potential |= (1ULL << warm_ocean) | (1ULL << lukewarm_ocean);
            if (id == lukewarm_ocean) potential |= (1ULL << lukewarm_ocean);
            if (id == ocean)          potential |= (1ULL << ocean);
            if (id == cold_ocean)     potential |= (1ULL << cold_ocean);
            if (id == frozen_ocean)   potential |= (1ULL << frozen_ocean) | (1ULL << cold_ocean);
        }

        if ((potential & filter.oceansToFind) ^ filter.oceansToFind)
        {
            goto return_zero;
        }
    }

    if (filter.doMajorBiomeCheck)
    {
        setWorldSeed(lbiomes, seed);
        genArea(lbiomes, map, areaX256, areaZ256, areaWidth256, areaHeight256);

        // get biomes out of the way that we cannot check for at this layer
        potential = (1ULL << beach) | (1ULL << stone_shore) |
                (1ULL << snowy_beach) | (1ULL << river) | (1ULL << frozen_river);

        for (i = 0; i < areaWidth256 * areaHeight256; i++)
        {
            id = map[i];
            switch (id)
            {
            case wooded_badlands_plateau:
            case badlands_plateau:
                potential |= (1ULL << id) | (1ULL << badlands) | (1ULL << desert); break;
            case giant_tree_taiga:
                potential |= (1ULL << id) | (1ULL << taiga) | (1ULL << taiga_hills) | (1ULL << giant_tree_taiga_hills); break;
            case desert:
                potential |= (1ULL << id) | (1ULL << wooded_mountains) | (1ULL << desert_hills); break;
            case swamp:
                potential |= (1ULL << id) | (1ULL << jungleEdge) | (1ULL << plains); break;
            case forest:
                potential |= (1ULL << id) | (1ULL << wooded_hills); break;
            case birch_forest:
                potential |= (1ULL << id) | (1ULL << birch_forest_hills); break;
            case dark_forest:
                potential |= (1ULL << id) | (1ULL << plains); break;
            case taiga:
                potential |= (1ULL << id) | (1ULL << taiga_hills); break;
            case snowy_taiga:
                potential |= (1ULL << id) | (1ULL << snowy_taiga_hills); break;
            case plains:
                potential |= (1ULL << id) | (1ULL << wooded_hills) | (1ULL << forest); break;
            case snowy_tundra:
                potential |= (1ULL << id) | (1ULL << snowy_mountains); break;
            case jungle:
                potential |= (1ULL << id) | (1ULL << jungle_hills); break;
            case ocean:
                potential |= (1ULL << id) | (1ULL << deep_ocean);
                // TODO: buffer possible ocean types at this location
                potential |= (1ULL << frozen_ocean) | (1ULL << cold_ocean) | (1ULL << lukewarm_ocean) | (1ULL << warm_ocean); break;
            case mountains:
                potential |= (1ULL << id) | (1ULL << wooded_mountains); break;
            case savanna:
                potential |= (1ULL << id) | (1ULL << savanna_plateau); break;
            case deep_ocean:
                potential |= (1ULL << id) | (1ULL << plains) | (1ULL << forest);
                // TODO: buffer possible ocean types at this location
                potential |= (1ULL << deep_frozen_ocean) | (1ULL << deep_cold_ocean) | (1ULL << deep_lukewarm_ocean); break;
            case mushroom_fields:
                potential |= (1ULL << id) | (1ULL << mushroom_field_shore);
            default:
                potential |= (1ULL << id);
            }
        }

        if ((potential & filter.biomesToFind) ^ filter.biomesToFind)
        {
            goto return_zero;
        }
    }

    // TODO: Do a check at the HILLS layer, scale 1:64

    if (minscale > 4) goto return_one;

    if (filter.doScale4Check)
    {
        // 1:4 scale
        areaX4 = blockX >> 2;
        areaZ4 = blockZ >> 2;
        areaWidth4 = ((width-1) >> 2) + 2;
        areaHeight4 = ((height-1) >> 2) + 2;

        applySeed(g, seed);
        if (filter.doOceanTypeCheck)
            genArea(&g->layers[L13_OCEAN_MIX_4], map, areaX4, areaZ4, areaWidth4, areaHeight4);
        else
            genArea(&g->layers[L_RIVER_MIX_4], map, areaX4, areaZ4, areaWidth4, areaHeight4);

        potential = modified = 0;

        for (i = 0; i < areaWidth4 * areaHeight4; i++)
        {
            id = map[i];
            if (id >= 128) modified |= (1ULL << (id & 0x7f));
            else potential |= (1ULL << id);
        }

        required = filter.biomesToFind;
        if ((potential & required) ^ required)
        {
            goto return_zero;
        }
        if ((modified & filter.modifiedToFind) ^ filter.modifiedToFind)
        {
            goto return_zero;
        }
    }


    int ret;

    // clean up and return
    return_one:
    ret = 1;

    if (0)
    {
        return_zero:
        ret = 0;
    }

    if (cache == NULL) free(map);

    return ret;
}












