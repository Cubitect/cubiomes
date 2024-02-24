#ifndef QUADBASE_H_
#define QUADBASE_H_


#include "finders.h"

#include <stdio.h>
#include <math.h>


#ifdef __cplusplus
extern "C"
{
#endif

/** Quad-Witch-Huts
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
        0x43f18,0xc751a,0xf520a, 0
};

// lower 20 bits, the classic quad-structure constellations
static const uint64_t low20QuadClassic[] =
{
        0x43f18,0x79a0a,0xc751a,0xf520a, 0
};

// for any valid quad-structure constellation with a structure size:
// (7+1,7+43+1,9+1) which corresponds to a fall-damage based quad-witch-farm,
// but may require a perfect player position
static const uint64_t low20QuadHutNormal[] =
{
        0x43f18,0x65118,0x75618,0x79a0a, 0x89718,0x9371a,0xa5a08,0xb5e18,
        0xc751a,0xf520a, 0
};

// for any valid quad-structure constellation with a structure size:
// (7+1,7+1,9+1) which corresponds to quad-witch-farms without drop chute
static const uint64_t low20QuadHutBarely[] =
{
        0x1272d,0x17908,0x367b9,0x43f18, 0x487c9,0x487ce,0x50aa7,0x647b5,
        0x65118,0x75618,0x79a0a,0x89718, 0x9371a,0x967ec,0xa3d0a,0xa5918,
        0xa591d,0xa5a08,0xb5e18,0xc6749, 0xc6d9a,0xc751a,0xd7108,0xd717a,
        0xe2739,0xe9918,0xee1c4,0xf520a, 0
};


// categorize a constellation
enum { CST_NONE, CST_IDEAL, CST_CLASSIC, CST_NORMAL, CST_BARELY };
int getQuadHutCst(uint64_t low20);


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
static inline ATTR(always_inline, const)
float isQuadBaseFeature24Classic (const StructureConfig sconf, uint64_t seed);

static inline ATTR(always_inline, const)
float isQuadBaseFeature24 (const StructureConfig sconf, uint64_t seed,
        int ax, int ay, int az);

static inline ATTR(always_inline, const)
float isQuadBaseFeature (const StructureConfig sconf, uint64_t seed,
        int ax, int ay, int az, int radius);

static inline ATTR(always_inline, const)
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
 * @lowBitN     number of bits in the subset values
 * @check       the testing function, should return non-zero for desired seeds
 * @data        custom data argument passed to 'check'
 * @stop        occasional check for abort (nullable)
 *
 * Returns zero upon success.
 */
int searchAll48(
        uint64_t **         seedbuf,
        uint64_t *          buflen,
        const char *        path,
        int                 threads,
        const uint64_t *    lowBits,
        int                 lowBitN,
        int (*check)(uint64_t s48, void *data),
        void *              data,
        volatile char *     stop // should be atomic, but is fine as stop flag
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
        const uint64_t *lowBits, int lowBitN, uint64_t salt,
        int x, int z, int w, int h, Pos *qplist, int n);


//==============================================================================
// Implementaions for Functions that Ideally Should be Inlined
//==============================================================================

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
    case Ocean_Ruin:
    case Shipwreck:
    case Ruined_Portal:
        return isQuadBaseFeature(sconf, seed, 0, 0, 0, radius);

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
    JAVA_NEXT_INT24(s00, x0); if likely(x0 < 20) return 0;
    JAVA_NEXT_INT24(s00, z0); if likely(z0 < 20) return 0;

    s11 ^= K;
    JAVA_NEXT_INT24(s11, x1); if likely(x1 > x0-20) return 0;
    JAVA_NEXT_INT24(s11, z1); if likely(z1 > z0-20) return 0;

    x = x1 + 32 - x0;
    z = z1 + 32 - z0;
    if (x*x + z*z > 255)
        return 0;

    uint64_t s01 = 341873128712ULL + seed;
    uint64_t s10 = 132897987541ULL + seed;

    s01 ^= K;
    JAVA_NEXT_INT24(s01, x2); if likely(x2 >= 4) return 0;
    JAVA_NEXT_INT24(s01, z2); if likely(z2 < 20) return 0;

    s10 ^= K;
    JAVA_NEXT_INT24(s10, x3); if likely(x3 < 20) return 0;
    JAVA_NEXT_INT24(s10, z3); if likely(z3 >= 4) return 0;

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
    JAVA_NEXT_INT24(s00, p); if likely(p < 22) return 0;
    JAVA_NEXT_INT24(s00, p); if likely(p < 22) return 0;

    s11 ^= K;
    JAVA_NEXT_INT24(s11, p); if likely(p > 1) return 0;
    JAVA_NEXT_INT24(s11, p); if likely(p > 1) return 0;

    uint64_t s01 = 341873128712ULL + seed;
    uint64_t s10 = 132897987541ULL + seed;

    s01 ^= K;
    JAVA_NEXT_INT24(s01, p); if likely(p > 1) return 0;
    JAVA_NEXT_INT24(s01, p); if likely(p < 22) return 0;

    s10 ^= K;
    JAVA_NEXT_INT24(s10, p); if likely(p < 22) return 0;
    JAVA_NEXT_INT24(s10, p); if likely(p > 1) return 0;

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
    s = (s * K + b) & M; x0 = (int)(s >> 17) % C; if likely(x0 <= rm) return 0;
    s = (s * K + b) & M; z0 = (int)(s >> 17) % C; if likely(z0 <= rm) return 0;

    s = s11 ^ K;
    s = (s * K + b) & M; x1 = (int)(s >> 17) % C; if likely(x1 >= x0-rm) return 0;
    s = (s * K + b) & M; z1 = (int)(s >> 17) % C; if likely(z1 >= z0-rm) return 0;

    // check that the two structures in the opposing diagonal quadrants are
    // close enough together

    x = x1 + R - x0;
    z = z1 + R - z0;
    if likely(x*x + z*z > cd*cd)
        return 0;

    uint64_t s01 = 341873128712ULL + seed;
    uint64_t s10 = 132897987541ULL + seed;

    s = s01 ^ K;
    s = (s * K + b) & M; x2 = (int)(s >> 17) % C; if likely(x2 >= C-rm) return 0;
    s = (s * K + b) & M; z2 = (int)(s >> 17) % C; if likely(z2 <= rm) return 0;

    s = s10 ^ K;
    s = (s * K + b) & M; x3 = (int)(s >> 17) % C; if likely(x3 <= rm) return 0;
    s = (s * K + b) & M; z3 = (int)(s >> 17) % C; if likely(z3 >= C-rm) return 0;

    x = x2 + R - x3;
    z = z3 + R - z2;
    if likely(x*x + z*z > cd*cd)
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
    s = (s * K + b) & M; p += (int)(s >> 17) % C; if likely(p <= rm) return 0;
    x0 = p;
    s = (s * K + b) & M; p =  (int)(s >> 17) % C;
    s = (s * K + b) & M; p += (int)(s >> 17) % C; if likely(p <= rm) return 0;
    z0 = p;

    s = s11 ^ K;
    s = (s * K + b) & M; p =  (int)(s >> 17) % C;
    s = (s * K + b) & M; p += (int)(s >> 17) % C; if likely(p > x0-rm) return 0;
    x1 = p;
    s = (s * K + b) & M; p =  (int)(s >> 17) % C;
    s = (s * K + b) & M; p += (int)(s >> 17) % C; if likely(p > z0-rm) return 0;
    z1 = p;

    s = ((x1-x0)>>1)*((x1-x0)>>1) + ((z1-z0)>>1)*((z1-z0)>>1);
    if (s > (uint64_t)4*radius*radius)
        return 0;

    s = s01 ^ K;
    s = (s * K + b) & M; p =  (int)(s >> 17) % C;
    s = (s * K + b) & M; p += (int)(s >> 17) % C; if likely(p > x0-rm) return 0;
    x2 = p;
    s = (s * K + b) & M; p =  (int)(s >> 17) % C;
    s = (s * K + b) & M; p += (int)(s >> 17) % C; if likely(p <= rm) return 0;
    z2 = p;

    s = s10 ^ K;
    s = (s * K + b) & M; p =  (int)(s >> 17) % C;
    s = (s * K + b) & M; p += (int)(s >> 17) % C; if likely(p <= rm) return 0;
    x3 = p;
    s = (s * K + b) & M; p =  (int)(s >> 17) % C;
    s = (s * K + b) & M; p += (int)(s >> 17) % C; if likely(p > z0-rm) return 0;
    z3 = p;

    float sqrad = getEnclosingRadius(
            x0>>1,z0>>1, x1>>1,z1>>1, x2>>1,z2>>1, x3>>1,z3>>1,
            ax,ay,az, sconf.regionSize, radius);
    return sqrad < radius ? sqrad : 0;
}

#ifdef __cplusplus
}
#endif

#endif // QUADBASE_H_


