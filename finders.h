#ifndef FINDERS_H_
#define FINDERS_H_

#include "generator.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>


#define THREADS 6
#define SEEDMAX (1L << 48)


static const int oceanMonumentBiomeList[] = {ocean, deepOcean, river, frozenOcean, frozenRiver};

static const int biomesToSpawnIn[] = {forest, plains, taiga, taigaHills, forestHills, jungle, jungleHills};

STRUCT(Pos)
{
    int x, z;
};


extern Biome biomes[256];

extern const int achievementBiomes[256];


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



/***************************** Quad-Temple Checks ******************************
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
 *  seed2 = seed1 - 14357617 - dregX * 341873128712 - dregZ * 132897987541;
 *
 *  Here seed1 and seed2 have the same temple positioning, but moved by a
 *  region offset of (dregX,dregZ). [a region is 32x32 chunks]
 *
 *  For a quad-temple, we mainly care about relative positioning, so we can get
 *  away with just checking the regions near the origin: (0,0),(0,1),(1,0),(1,1)
 *  and then move the temples to the desired position.
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
 *  found this way describes an entire set of possible quad-witch-huts
 *  (with degrees of freedom for region-transposition, and the top 16-bit bits).
 */


long moveTemple(const long baseSeed, const int regX, const int regZ);

long *loadSavedSeeds(const char *fnam, long *scnt);

void baseQuadTempleSearch(const char *fnam, int threads, int quality);


/**************************** General Biome Checks *****************************
 */


/* getBiomeAtPos
 * ----------------
 * Returns the biome for the position specified.
 */
int getBiomeAtPos(Generator *g, Pos pos);


/* getTemplePos
 * ------------
 * Faster implementation for finding the block position at which the temple
 * generation attempt will occur in the specified region.
 */
Pos getTemplePos(long seed, const long regionX, const long regionZ);


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
long filterAllTempCats(long *seedsIn, long *seedsOut, long seedCnt, int centX, int centZ);

/* filterAllMajorBiomes
 * --------------------
 * Looks through the list of seeds in 'seedsIn' and copies those that have all
 * major overworld biomes in the specified area into 'seedsOut'. These checks
 * are done at a scale of 1:256.
 *
 * Returns the number of seeds found.
 */
long filterAllMajorBiomes(long *seedsIn, long *seedsOut, long seedCnt,
        int pX, int pZ, uint sX, uint sZ);

/********************** C copy of the Java Random methods **********************
 */

static inline void setSeed(long *seed)
{
    *seed = (*seed ^ 0x5DEECE66DL) & ((1L << 48) - 1);
}

static inline int next(long *seed, const int bits)
{
    *seed = (*seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
    return (int) (*seed >> (48 - bits));
}

static inline int nextInt(long *seed, const int n)
{
    int bits, val;
    do {
        bits = next(seed, 31);
        val = bits % n;
    }
    while(bits - val + (n - 1) < 0);
    return val;
}

static inline long nextLong(long *seed)
{
    return ((long) next(seed, 32) << 32) + next(seed, 32);
}

static inline float nextFloat(long *seed)
{
    return next(seed, 24) / (float) (1 << 24);
}

static inline double nextDouble(long *seed)
{
    return (((long) next(seed, 26) << 27) + next(seed, 27)) / (double) (1L << 53);
}



// Custom, faster alternative for the first and second call to nextInt(24)

static inline int firstInt24(long seed)
{
    seed ^= 0x5deece66d;
    seed = (seed * 0x5deece66d) & 0xffffffffffff;
    seed >>= 17;
    return seed % 24;
}

static inline int secondInt24(long seed)
{
    seed ^= 0x5deece66d;
    seed = (seed * 0x5deece66d + 0xB) & 0xffffffffffff;
    seed = (seed * 0x5deece66d) & 0xffffffffffff;
    seed >>= 17;
    return seed % 24;
}

/**
 * invSeed48()
 * -----------
 * Returns the previous 48-bit seed which will generate 'nseed'.
 * The upper 16 bits are ignored, both here and in the generator.
 */
static inline long invSeed48(long nseed)
{
    const long x = 0x5deece66d;
    const long xinv = 0xdfe05bcb1365L;
    const long y = 0xbL;
    const long m48 = 0xffffffffffffL;

    long a = nseed >> 32;
    long b = nseed & 0xffffffffL;
    if(b & 0x80000000L) a++;

    long q = ((b << 16) - y - (a << 16)*x) & m48;
    for(long k = 0; k <= 5; k++)
    {
        long d = (x - (q + (k << 48))) % x;
        d = (d + x) % x; // force the modulo and keep it positive
        if(d < 65536)
        {
            long c = ((q + d) * xinv) & m48;
            if(c < 65536)
            {
                return ((((a << 16) + c) - y) * xinv) & m48;
            }
        }
    }
    return -1;
}


static inline const char *int2binstr(int x, int bit)
{
    static char str[33];
    str[0] = '\0';
    for(bit = (1 << bit); bit > 0; bit >>= 1)
        strcat(str, ((x & bit) == bit) ? "1" : "0");

    return str;
}

#endif /* FINDERS_H_ */
