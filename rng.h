#ifndef RNG_H_
#define RNG_H_

#define __STDC_FORMAT_MACROS 1

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>


///=============================================================================
///                      Compiler and Platform Features
///=============================================================================

#ifndef NULL
#define NULL ((void*)0)
#endif

#define STRUCT(S) typedef struct S S; struct S

#if __GNUC__
#define IABS(X)                 __builtin_abs(X)
#define PREFETCH(PTR,RW,LOC)    __builtin_prefetch(PTR,RW,LOC)
#define L(COND)                 (__builtin_expect(!!(COND),1))  // [[likely]]
#define U(COND)                 (__builtin_expect((COND),0))    // [[unlikely]
#define ATTR(...)               __attribute__((__VA_ARGS__))
#define BSWAP32(X)              __builtin_bswap32(X)
#else
#define IABS(X)                 ((int)abs(X))
#define PREFETCH(PTR,RW,LOC)
#define L(COND)                 (COND)
#define U(COND)                 (COND)
#define ATTR(...)
static inline uint32_t BSWAP32(uint32_t x) {
    x = ((x & 0x000000ff) << 24) | ((x & 0x0000ff00) <<  8) |
        ((x & 0x00ff0000) >>  8) | ((x & 0xff000000) >> 24);
    return x;
}
#endif

/// imitate amd64/x64 rotate instructions

static inline ATTR(const, always_inline, artificial)
uint64_t rotl64(uint64_t x, uint8_t b)
{
    return (x << b) | (x >> (64-b));
}

static inline ATTR(const, always_inline, artificial)
uint32_t rotr32(uint32_t a, uint8_t b)
{
    return (a >> b) | (a << (32-b));
}


///=============================================================================
///                    C implementation of Java Random
///=============================================================================

static inline void setSeed(uint64_t *seed, uint64_t value)
{
    *seed = (value ^ 0x5deece66d) & ((1ULL << 48) - 1);
}

static inline int next(uint64_t *seed, const int bits)
{
    *seed = (*seed * 0x5deece66d + 0xb) & ((1ULL << 48) - 1);
    return (int) ((int64_t)*seed >> (48 - bits));
}

static inline int nextInt(uint64_t *seed, const int n)
{
    int bits, val;
    const int m = n - 1;

    if ((m & n) == 0) {
        uint64_t x = n * (uint64_t)next(seed, 31);
        return (int) ((int64_t) x >> 31);
    }

    do {
        bits = next(seed, 31);
        val = bits % n;
    }
    while (bits - val + m < 0);
    return val;
}

static inline uint64_t nextLong(uint64_t *seed)
{
    return ((uint64_t) next(seed, 32) << 32) + next(seed, 32);
}

static inline float nextFloat(uint64_t *seed)
{
    return next(seed, 24) / (float) (1 << 24);
}

static inline double nextDouble(uint64_t *seed)
{
    uint64_t x = (uint64_t)next(seed, 26);
    x <<= 27;
    x += next(seed, 27);
    return (int64_t) x / (double) (1ULL << 53);
}

/* A macro to generate the ideal assembly for X = nextInt(S, 24)
 * This is a macro and not an inline function, as many compilers can make use
 * of the additional optimisation passes for the surrounding code.
 */
#define JAVA_NEXT_INT24(S,X)                \
    do {                                    \
        uint64_t a = (1ULL << 48) - 1;      \
        uint64_t c = 0x5deece66dULL * (S);  \
        c += 11; a &= c;                    \
        (S) = a;                            \
        a = (uint64_t) ((int64_t)a >> 17);  \
        c = 0xaaaaaaab * a;                 \
        c = (uint64_t) ((int64_t)c >> 36);  \
        (X) = (int)a - (int)(c << 3) * 3;   \
    } while (0)


/* Jumps forwards in the random number sequence by simulating 'n' calls to next.
 */
static inline void skipNextN(uint64_t *seed, uint64_t n)
{
    uint64_t m = 1;
    uint64_t a = 0;
    uint64_t im = 0x5deece66dULL;
    uint64_t ia = 0xb;
    uint64_t k;

    for (k = n; k; k >>= 1)
    {
        if (k & 1)
        {
            m *= im;
            a = im * a + ia;
        }
        ia = (im + 1) * ia;
        im *= im;
    }

    *seed = *seed * m + a;
    *seed &= 0xffffffffffffULL;
}


///=============================================================================
///                               Xoroshiro 128
///=============================================================================

STRUCT(Xoroshiro)
{
    uint64_t lo, hi;
};

static inline void xSetSeed(Xoroshiro *xr, uint64_t value)
{
    const uint64_t XL = 0x9e3779b97f4a7c15ULL;
    const uint64_t XH = 0x6a09e667f3bcc909ULL;
    const uint64_t A = 0xbf58476d1ce4e5b9ULL;
    const uint64_t B = 0x94d049bb133111ebULL;
    uint64_t l = value ^ XH;
    uint64_t h = l + XL;
    l = (l ^ (l >> 30)) * A;
    h = (h ^ (h >> 30)) * A;
    l = (l ^ (l >> 27)) * B;
    h = (h ^ (h >> 27)) * B;
    l = l ^ (l >> 31);
    h = h ^ (h >> 31);
    xr->lo = l;
    xr->hi = h;
}

static inline uint64_t xNextLong(Xoroshiro *xr)
{
    uint64_t l = xr->lo;
    uint64_t h = xr->hi;
    uint64_t n = rotl64(l + h, 17) + l;
    h ^= l;
    xr->lo = rotl64(l, 49) ^ h ^ (h << 21);
    xr->hi = rotl64(h, 28);
    return n;
}

static inline int xNextInt(Xoroshiro *xr, uint32_t n)
{
    uint64_t r = (xNextLong(xr) & 0xFFFFFFFF) * n;
    if ((uint32_t)r < n)
    {
        while ((uint32_t)r < (~n + 1) % n)
        {
            r = (xNextLong(xr) & 0xFFFFFFFF) * n;
        }
    }
    return r >> 32;
}

static inline double xNextDouble(Xoroshiro *xr)
{
    return (xNextLong(xr) >> (64-53)) * 1.1102230246251565E-16;
}

static inline float xNextFloat(Xoroshiro *xr)
{
    return (xNextLong(xr) >> (64-24)) * 5.9604645E-8F;
}

static inline void xSkipN(Xoroshiro *xr, int count)
{
    while (count --> 0)
        xNextLong(xr);
}

static inline uint64_t xNextLongJ(Xoroshiro *xr)
{
    return (xNextLong(xr) & 0xffffffff00000000) | (xNextLong(xr) >> 32);
}

static inline int xNextIntJ(Xoroshiro *xr, uint32_t n)
{
    int bits, val;
    const int m = n - 1;

    if ((m & n) == 0) {
        uint64_t x = n * (xNextLong(xr) >> 33);
        return (int) ((int64_t) x >> 31);
    }

    do {
        bits = (xNextLong(xr) >> 33);
        val = bits % n;
    }
    while (bits - val + m < 0);
    return val;
}


//==============================================================================
//                              MC Seed Helpers
//==============================================================================

/**
 * The seed pipeline:
 *
 * getLayerSalt(n)                -> layerSalt (ls)
 * layerSalt (ls), worldSeed (ws) -> startSalt (st), startSeed (ss)
 * startSeed (ss), coords (x,z)   -> chunkSeed (cs)
 *
 * The chunkSeed alone is enough to generate the first PRNG integer with:
 *   mcFirstInt(cs, mod)
 * subsequent PRNG integers are generated by stepping the chunkSeed forwards,
 * salted with startSalt:
 *   cs_next = mcStepSeed(cs, st)
 */

static inline uint64_t mcStepSeed(uint64_t s, uint64_t salt)
{
    return s * (s * 6364136223846793005ULL + 1442695040888963407ULL) + salt;
}

static inline int mcFirstInt(uint64_t s, int mod)
{
    int ret = (int)(((int64_t)s >> 24) % mod);
    if (ret < 0)
        ret += mod;
    return ret;
}

static inline int mcFirstIsZero(uint64_t s, int mod)
{
    return (int)(((int64_t)s >> 24) % mod) == 0;
}

static inline uint64_t getChunkSeed(uint64_t ss, int x, int z)
{
    uint64_t cs = ss + x;
    cs = mcStepSeed(cs, z);
    cs = mcStepSeed(cs, x);
    cs = mcStepSeed(cs, z);
    return cs;
}

static inline uint64_t getLayerSalt(uint64_t salt)
{
    uint64_t ls = mcStepSeed(salt, salt);
    ls = mcStepSeed(ls, salt);
    ls = mcStepSeed(ls, salt);
    return ls;
}

static inline uint64_t getStartSalt(uint64_t ws, uint64_t ls)
{
    uint64_t st = ws;
    st = mcStepSeed(st, ls);
    st = mcStepSeed(st, ls);
    st = mcStepSeed(st, ls);
    return st;
}

static inline uint64_t getStartSeed(uint64_t ws, uint64_t ls)
{
    uint64_t ss = ws;
    ss = getStartSalt(ss, ls);
    ss = mcStepSeed(ss, 0);
    return ss;
}


///============================================================================
///                               Arithmatic
///============================================================================


/* Linear interpolations
 */
static inline double lerp(double part, double from, double to)
{
    return from + part * (to - from);
}

static inline double lerp2(
        double dx, double dy, double v00, double v10, double v01, double v11)
{
    return lerp(dy, lerp(dx, v00, v10), lerp(dx, v01, v11));
}

static inline double lerp3(
        double dx, double dy, double dz,
        double v000, double v100, double v010, double v110,
        double v001, double v101, double v011, double v111)
{
    v000 = lerp2(dx, dy, v000, v100, v010, v110);
    v001 = lerp2(dx, dy, v001, v101, v011, v111);
    return lerp(dz, v000, v001);
}

static inline double clampedLerp(double part, double from, double to)
{
    if (part <= 0) return from;
    if (part >= 1) return to;
    return lerp(part, from, to);
}

/* Find the modular inverse: (1/x) | mod m.
 * Assumes x and m are positive (less than 2^63), co-prime.
 */
static inline ATTR(const)
uint64_t mulInv(uint64_t x, uint64_t m)
{
    uint64_t t, q, a, b, n;
    if ((int64_t)m <= 1)
        return 0; // no solution

    n = m;
    a = 0; b = 1;

    while ((int64_t)x > 1)
    {
        if (m == 0)
            return 0; // x and m are co-prime
        q = x / m;
        t = m; m = x % m;     x = t;
        t = a; a = b - q * a; b = t;
    }

    if ((int64_t)b < 0)
        b += n;
    return b;
}


#endif /* RNG_H_ */



