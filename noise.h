#ifndef NOISE_H_
#define NOISE_H_

#include "rng.h"

STRUCT(PerlinNoise)
{
    uint8_t d[512];
    double a, b, c;
    double amplitude;
    double lacunarity;
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
double maintainPrecision(double x);

/// Perlin noise
void perlinInit(PerlinNoise *noise, uint64_t *seed);
void xPerlinInit(PerlinNoise *noise, Xoroshiro *xr);

double samplePerlin(const PerlinNoise *noise, double x, double y, double z,
        double yamp, double ymin);
double sampleSimplex2D(const PerlinNoise *noise, double x, double y);

/// Perlin Octaves
void octaveInit(OctaveNoise *noise, uint64_t *seed, PerlinNoise *octaves,
        int omin, int len);
int xOctaveInit(OctaveNoise *noise, Xoroshiro *xr, PerlinNoise *octaves,
        const double *amplitudes, int omin, int len);

double sampleOctave(const OctaveNoise *noise, double x, double y, double z);

/// Double Perlin
void doublePerlinInit(DoublePerlinNoise *noise, uint64_t *seed,
        PerlinNoise *octavesA, PerlinNoise *octavesB, int omin, int len);
int xDoublePerlinInit(DoublePerlinNoise *noise, Xoroshiro *xr,
        PerlinNoise *octaves, const double *amplitudes, int omin, int len);

double sampleDoublePerlin(const DoublePerlinNoise *noise,
        double x, double y, double z);


#ifdef __cplusplus
}
#endif

#endif /* NOISE_H_ */



