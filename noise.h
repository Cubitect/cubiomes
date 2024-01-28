#ifndef NOISE_H_
#define NOISE_H_

#include "rng.h"
#include <math.h>

STRUCT(PerlinNoise)
{
    uint8_t d[256+1];
    uint8_t h2;
    double a, b, c;
    double amplitude;
    double lacunarity;
    double d2;
    double t2;
};

STRUCT(OctaveNoise)
{
    int octcnt;
    PerlinNoise *octaves;
};

STRUCT(DoublePerlinNoise)
{
    double amplitude;
    OctaveNoise octA;
    OctaveNoise octB;
};

#ifdef __cplusplus
extern "C"
{
#endif

/// Helper
static inline ATTR(hot, const)
double maintainPrecision(double x)
{   // This is a highly performance critical function that is used to correct
    // progressing errors from float-maths. However, since cubiomes uses
    // doubles anyway, this seems useless in practice.

    //return x - round(x / 33554432.0) * 33554432.0;
    return x;
}

/// Perlin noise
void perlinInit(PerlinNoise *noise, uint64_t *seed);
void xPerlinInit(PerlinNoise *noise, Xoroshiro *xr);

double samplePerlin(const PerlinNoise *noise, double x, double y, double z,
        double yamp, double ymin);
double sampleSimplex2D(const PerlinNoise *noise, double x, double y);

/// Perlin Octaves
void octaveInit(OctaveNoise *noise, uint64_t *seed, PerlinNoise *octaves,
        int omin, int len);
void octaveInitBeta(OctaveNoise *noise, uint64_t *seed, PerlinNoise *octaves,
        int octcnt, double lac, double lacMul, double persist, double persistMul);
int xOctaveInit(OctaveNoise *noise, Xoroshiro *xr, PerlinNoise *octaves,
        const double *amplitudes, int omin, int len, int nmax);

double sampleOctave(const OctaveNoise *noise, double x, double y, double z);
double sampleOctaveAmp(const OctaveNoise *noise, double x, double y, double z,
        double yamp, double ymin, int ydefault);
double sampleOctave2D(const OctaveNoise *noise, double x, double z);
double sampleOctaveBeta17Biome(const OctaveNoise *noise, double x, double z);
void sampleOctaveBeta17Terrain(const OctaveNoise *noise, double *v,
        double x, double z, int yLacFlag, double lacmin);

/// Double Perlin
void doublePerlinInit(DoublePerlinNoise *noise, uint64_t *seed,
        PerlinNoise *octavesA, PerlinNoise *octavesB, int omin, int len);
int xDoublePerlinInit(DoublePerlinNoise *noise, Xoroshiro *xr,
        PerlinNoise *octaves, const double *amplitudes, int omin, int len, int nmax);

double sampleDoublePerlin(const DoublePerlinNoise *noise,
        double x, double y, double z);


#ifdef __cplusplus
}
#endif

#endif /* NOISE_H_ */



