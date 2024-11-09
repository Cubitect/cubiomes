#ifndef BIOMENOISE_H_
#define BIOMENOISE_H_

#include "noise.h"
#include "layers.h"


STRUCT(Range)
{
    // Defines an area or volume for the biome generation. It is given by a
    // position, size and scaling in the horizontal axes, and an optional
    // vertical range. The vertical scaling is equal to 1:1 iff scale == 1,
    // and 1:4 (default biome scale) in all other cases!
    //
    // @scale:  Horizontal scale factor, should be one of 1, 4, 16, 64, or 256
    //          additionally a value of zero bypasses scaling and expects a
    //          manual generation entry layer.
    // @x,z:    Horizontal position, i.e. coordinates of north-west corner.
    // @sx,sz:  Horizontal size (width and height for 2D), should be positive.
    // @y       Vertical position, 1:1 iff scale==1, 1:4 otherwise.
    // @sy      Vertical size. Values <= 0 are treated equivalent to 1.
    //
    // Volumes generated with a range are generally indexed as:
    //  out [ i_y*sx*sz + i_z*sx + i_x ]
    // where i_x, i_y, i_z are indecies in their respective directions.
    //
    // EXAMPLES
    // Area at normal biome scale (1:4):
    //  Range r_2d = {4, x,z, sx,sz};
    // (C99 syntax allows ommission of the trailing zero-initialization.)
    //
    // Area at block scale (1:1) at sea level:
    //  Range r_surf = {1, x,z, sx,sz, 63};
    // (Block level scale uses voronoi sampling with 1:1 vertical scaling.)
    //
    // Area at chunk scale (1:16) near sea level:
    //  Range r_surf16 = {16, x,z, sx,sz, 15};
    // (Note that the vertical scaling is always 1:4 for non-voronoi scales.)
    //
    // Volume at scale (1:4):
    //  Range r_vol = {4, x,z, sx,sz, y,sy};

    int scale;
    int x, z, sx, sz;
    int y, sy;
};


// Nether biome generator 1.16+
STRUCT(NetherNoise)
{   // altitude and wierdness don't affect nether biomes
    // and the weight is a 5th noise parameter which is constant
    DoublePerlinNoise temperature;
    DoublePerlinNoise humidity;
    PerlinNoise oct[8]; // buffer for octaves in double perlin noise
};

// End biome generator 1.9+
STRUCT(EndNoise)
{
    PerlinNoise perlin;
    int mc;
};

STRUCT(SurfaceNoise)
{
    double xzScale, yScale;
    double xzFactor, yFactor;
    OctaveNoise octmin;
    OctaveNoise octmax;
    OctaveNoise octmain;
    OctaveNoise octsurf;
    OctaveNoise octdepth;
    PerlinNoise oct[16+16+8+4+16];
};

STRUCT(SurfaceNoiseBeta)
{
    OctaveNoise octmin;
    OctaveNoise octmax;
    OctaveNoise octmain;
    OctaveNoise octcontA;
    OctaveNoise octcontB;
    PerlinNoise oct[16+16+8+10+16];
};

STRUCT(SeaLevelColumnNoiseBeta)
{
    double contASample;
    double contBSample;
    double minSample[2];
    double maxSample[2];
    double mainSample[2];
};

STRUCT(Spline)
{
    int len, typ;
    float loc[12];
    float der[12];
    Spline *val[12];
};

STRUCT(FixSpline)
{
    int len;
    float val;
};

STRUCT(SplineStack)
{   // the stack size here is just sufficient for overworld generation
    Spline stack[42];
    FixSpline fstack[151];
    int len, flen;
};


enum
{
    NP_TEMPERATURE      = 0,
    NP_HUMIDITY         = 1,
    NP_CONTINENTALNESS  = 2,
    NP_EROSION          = 3,
    NP_SHIFT            = 4, NP_DEPTH = NP_SHIFT, // not a real climate
    NP_WEIRDNESS        = 5,
    NP_MAX
};
// Overworld biome generator for 1.18+
STRUCT(BiomeNoise)
{
    DoublePerlinNoise climate[NP_MAX];
    PerlinNoise oct[2*23]; // buffer for octaves in double perlin noise
    Spline *sp;
    SplineStack ss;
    int nptype;
    int mc;
};
// Overworld biome generator for pre-Beta 1.8
STRUCT(BiomeNoiseBeta)
{
    OctaveNoise climate[3];
    PerlinNoise oct[10];
    int nptype;
    int mc;
};


STRUCT(BiomeTree)
{
    const uint32_t *steps;
    const int32_t  *param;
    const uint64_t *nodes;
    uint32_t order;
    uint32_t len;
};

#ifdef __cplusplus
extern "C"
{
#endif

//==============================================================================
// Noise
//==============================================================================

void initSurfaceNoise(SurfaceNoise *sn, int dim, uint64_t seed);
void initSurfaceNoiseBeta(SurfaceNoiseBeta *snb, uint64_t seed);
double sampleSurfaceNoise(const SurfaceNoise *sn, int x, int y, int z);
double sampleSurfaceNoiseBetween(const SurfaceNoise *sn, int x, int y, int z,
    double noiseMin, double noiseMax);


//==============================================================================
// End (1.9+), Nether (1.16+) and Overworld (1.18+) Biome Noise Generation
//==============================================================================

/**
 * Nether biomes are 3D, and generated at scale 1:4. Use voronoiAccess3D() to
 * convert coordinates at 1:1 scale to their 1:4 access. Biome checks for
 * structures are generally done at y=0.
 *
 * The function getNetherBiome() determines the nether biome at a given
 * coordinate at scale 1:4. The parameter 'ndel' is an output noise delta for
 * optimization purposes and can be ignored (nullable).
 *
 * Use mapNether2D() to get a 2D area of nether biomes at y=0, scale 1:4.
 *
 * The mapNether3D() function attempts to optimize the generation of a volume
 * at scale 1:4. The output is indexed as:
 * out[i_y*(r.sx*r.sz) + i_z*r.sx + i_x].
 * If the optimization parameter 'confidence' has a value less than 1.0, the
 * generation will generally be faster, but can yield incorrect results in some
 * circumstances.
 *
 * The output buffer for the map-functions need only be of sufficient size to
 * hold the generated area (i.e. w*h or r.sx*r.sy*r.sz).
 */
void setNetherSeed(NetherNoise *nn, uint64_t seed);
int getNetherBiome(const NetherNoise *nn, int x, int y, int z, float *ndel);
int mapNether2D(const NetherNoise *nn, int *out, int x, int z, int w, int h);
int mapNether3D(const NetherNoise *nn, int *out, Range r, float confidence);
/**
 * The scaled Nether generation supports scales 1, 4, 16, 64, and 256.
 * It is similar to mapNether3D(), but applies voronoi zoom if necessary, and
 * fills the output buffer with nether_wastes for versions older than 1.16.
 */
int genNetherScaled(const NetherNoise *nn, int *out, Range r, int mc, uint64_t sha);

/**
 * End biome generation is based on simplex noise and varies only at a 1:16
 * chunk scale which can be generated with mapEndBiome(). The function mapEnd()
 * is a variation which also scales this up on a regular grid to 1:4. The final
 * access at a 1:1 scale uses voronoi.
 */
void setEndSeed(EndNoise *en, int mc, uint64_t seed);
int mapEndBiome(const EndNoise *en, int *out, int x, int z, int w, int h);
int mapEnd(const EndNoise *en, int *out, int x, int z, int w, int h);
int getEndSurfaceHeight(int mc, uint64_t seed, int x, int z);
int mapEndSurfaceHeight(float *y, const EndNoise *en, const SurfaceNoise *sn,
    int x, int z, int w, int h, int scale, int ymin);

/**
 * The scaled End generation supports scales 1, 4, 16, and 64.
 * The End biomes are usually 2D, but in 1.15+ there is 3D voronoi noise, which
 * is controlled by the 'sha' hash of the seed. For scales higher than 1:1, and
 * versions up to 1.14, 'sha' is ignored.
 */
int genEndScaled(const EndNoise *en, int *out, Range r, int mc, uint64_t sha);

/**
 * In 1.18 the Overworld uses a new noise map system for the biome generation.
 * The random number generation also has updated to a Xiroshiro128 algorithm.
 * The scale is 1:4, and is sampled at each point individually as there is
 * currently not much benefit from generating a volume as a whole.
 *
 * The 1.18 End generation remains similar to 1.17 and does NOT use the
 * biome noise.
 */
enum {
    SAMPLE_NO_SHIFT = 0x1,  // skip local distortions
    SAMPLE_NO_DEPTH = 0x2,  // skip depth sampling for vertical biomes
    SAMPLE_NO_BIOME = 0x4,  // do not apply climate noise to biome mapping
};
void initBiomeNoise(BiomeNoise *bn, int mc);
void setBiomeSeed(BiomeNoise *bn, uint64_t seed, int large);
void setBetaBiomeSeed(BiomeNoiseBeta *bnb, uint64_t seed);
int sampleBiomeNoise(const BiomeNoise *bn, int64_t *np, int x, int y, int z,
    uint64_t *dat, uint32_t sample_flags);
int sampleBiomeNoiseBeta(const BiomeNoiseBeta *bnb, int64_t *np, double *nv,
    int x, int z);
double approxSurfaceBeta(const BiomeNoiseBeta *bnb, const SurfaceNoiseBeta *snb,
    int x, int z); // doesn't really work yet

/**
 * (Alpha 1.2 - Beta 1.7) 
 * Temperature and humidity values to biome.
 */
int getOldBetaBiome(float t, float h);

/**
 * Uses the global biome tree definitions (see tables/btreeXX.h)
 * to map a noise point (i.e. climate) to the corresponding overworld biome.
 */
int climateToBiome(int mc, const uint64_t np[6], uint64_t *dat);

/**
 * Initialize BiomeNoise for only a single climate parameter.
 * If nptype == NP_DEPTH, the value is sampled at y=0. Note that this value
 * changes linearly with the height (i.e. -= y/128).
 * A maximum of nmax octaves is set, initializing only the most contributing
 * octaves up to that point. Use -1 for a full initialization.
 */
void setClimateParaSeed(BiomeNoise *bn, uint64_t seed, int large, int nptype, int nmax);
double sampleClimatePara(const BiomeNoise *bn, int64_t *np, double x, double z);

/**
 * Currently, in 1.18, we have to generate biomes one chunk at a time to get an
 * accurate mapping of the biomes in the level storage, as there is no longer a
 * unique mapping from noise points to biomes (MC-241546). Note that the results
 * from this are not suitable for chunk population/structure generation.
 * The output is in the form out[x][y][z] for the 64 biome points in the chunk
 * section. The coordinates {cx,cy,cz} are all at scale 1:16 and the 'dat'
 * argument should be the previous noise sampling and can be left NULL.
 */
void genBiomeNoiseChunkSection(const BiomeNoise *bn, int out[4][4][4],
    int cx, int cy, int cz, uint64_t *dat);

/**
 * The scaled biome noise generation applies for the Overworld version 1.18+.
 * The 'sha' hash of the seed is only required for voronoi at scale 1:1.
 * A scale of zero is interpreted as the default 1:4 scale.
 */
int genBiomeNoiseScaled(const BiomeNoise *bn, int *out, Range r, uint64_t sha);

/**
 * Generates the biomes for Beta 1.7, the surface noise is optional and enables
 * ocean mapping in areas that fall below the sea level.
 */
int genBiomeNoiseBetaScaled(const BiomeNoiseBeta *bnb, const SurfaceNoiseBeta *snb,
    int *out, Range r);


int getBiomeDepthAndScale(int id, double *depth, double *scale, int *grass);

// Gets the range in the parent/source layer which may be accessed by voronoi.
Range getVoronoiSrcRange(Range r);


#ifdef __cplusplus
}
#endif

#endif /* BIOMENOISE_H_ */



