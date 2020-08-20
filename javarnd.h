#ifndef JAVARND_H_
#define JAVARND_H_

#include <stdint.h>


/********************** C copy of the Java Random methods **********************
 */

static inline void setSeed(int64_t *seed)
{
    *seed = (*seed ^ 0x5deece66d) & ((1LL << 48) - 1);
}

static inline int next(int64_t *seed, const int bits)
{
    *seed = (*seed * 0x5deece66d + 0xb) & ((1LL << 48) - 1);
    return (int) (*seed >> (48 - bits));
}

static inline int nextInt(int64_t *seed, const int n)
{
    int bits, val;
    const int m = n - 1;

    if((m & n) == 0) return (int) ((n * (int64_t)next(seed, 31)) >> 31);

    do {
        bits = next(seed, 31);
        val = bits % n;
    }
    while (bits - val + m < 0);
    return val;
}

static inline int64_t nextLong(int64_t *seed)
{
    return ((int64_t) next(seed, 32) << 32) + next(seed, 32);
}

static inline float nextFloat(int64_t *seed)
{
    return next(seed, 24) / (float) (1 << 24);
}

static inline double nextDouble(int64_t *seed)
{
    return (((int64_t) next(seed, 26) << 27) + next(seed, 27)) / (double) (1LL << 53);
}

/* A macro to generate the ideal assembly for X = nextInt(S, 24)
 * This is a macro and not an inline function, as many compilers can make use
 * of the additional optimisation passes for the surrounding code.
 */
#define JAVA_NEXT_INT24(S,X)                \
    do {                                    \
        int64_t a = (1ULL << 48) - 1;       \
        int64_t c = 0x5deece66dLL * (S);    \
        c += 11; a &= c;                    \
        (S) = a;                            \
        a >>= 17;                           \
        c = 0xaaaaaaab * a;                 \
        c >>= 36;                           \
        (X) = (int)a - (int)(c << 3) * 3;   \
    } while (0)


/* skipNextN
 * ---------
 * Jumps forwards in the random number sequence by simulating 'n' calls to next.
 */
static inline void skipNextN(int64_t *seed, const int n)
{
    int i;
    for (i = 0; i < n; i++) *seed = (*seed * 0x5deece66d + 0xb);
    *seed &= 0xffffffffffff;
}

/* invSeed48
 * ---------
 * Returns the previous 48-bit seed which will generate 'nseed'.
 * The upper 16 bits are ignored, both here and in the generator.
 */
static inline int64_t invSeed48(int64_t nseed)
{
    const int64_t x = 0x5deece66d;
    const int64_t xinv = 0xdfe05bcb1365LL;
    const int64_t y = 0xbLL;
    const int64_t m48 = 0xffffffffffffLL;

    int64_t a = nseed >> 32;
    int64_t b = nseed & 0xffffffffLL;
    if (b & 0x80000000LL) a++;

    int64_t q = ((b << 16) - y - (a << 16)*x) & m48;
    int64_t k;
    for (k = 0; k <= 5; k++)
    {
        int64_t d = (x - (q + (k << 48))) % x;
        d = (d + x) % x; // force the modulo and keep it positive
        if (d < 65536)
        {
            int64_t c = ((q + d) * xinv) & m48;
            if (c < 65536)
            {
                return ((((a << 16) + c) - y) * xinv) & m48;
            }
        }
    }
    return -1;
}



#endif /* JAVARND_H_ */
