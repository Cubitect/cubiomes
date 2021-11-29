#ifndef LAYER_H_
#define LAYER_H_

#include "noise.h"


#define LAYER_INIT_SHA          (~0ULL)


/* Minecraft versions */
enum MCversion
{
    MC_1_0, // <=1.0 Experimental!
    MC_1_1,  MC_1_2,  MC_1_3,  MC_1_4,  MC_1_5,  MC_1_6,
    MC_1_7,  MC_1_8,  MC_1_9,  MC_1_10, MC_1_11, MC_1_12,
    MC_1_13, MC_1_14, MC_1_15, MC_1_16, MC_1_17, MC_1_18,
    MC_NEWEST = MC_1_18,
};

enum BiomeID
{
    none = -1,
    // 0
    ocean = 0,
    plains,
    desert,
    mountains,                  extremeHills = mountains,
    forest,
    taiga,
    swamp,                      swampland = swamp,
    river,
    nether_wastes,              hell = nether_wastes,
    the_end,                    sky = the_end,
    // 10
    frozen_ocean,               frozenOcean = frozen_ocean,
    frozen_river,               frozenRiver = frozen_river,
    snowy_tundra,               icePlains = snowy_tundra,
    snowy_mountains,            iceMountains = snowy_mountains,
    mushroom_fields,            mushroomIsland = mushroom_fields,
    mushroom_field_shore,       mushroomIslandShore = mushroom_field_shore,
    beach,
    desert_hills,               desertHills = desert_hills,
    wooded_hills,               forestHills = wooded_hills,
    taiga_hills,                taigaHills = taiga_hills,
    // 20
    mountain_edge,              extremeHillsEdge = mountain_edge,
    jungle,
    jungle_hills,               jungleHills = jungle_hills,
    jungle_edge,                jungleEdge = jungle_edge,
    deep_ocean,                 deepOcean = deep_ocean,
    stone_shore,                stoneBeach = stone_shore,
    snowy_beach,                coldBeach = snowy_beach,
    birch_forest,               birchForest = birch_forest,
    birch_forest_hills,         birchForestHills = birch_forest_hills,
    dark_forest,                roofedForest = dark_forest,
    // 30
    snowy_taiga,                coldTaiga = snowy_taiga,
    snowy_taiga_hills,          coldTaigaHills = snowy_taiga_hills,
    giant_tree_taiga,           megaTaiga = giant_tree_taiga,
    giant_tree_taiga_hills,     megaTaigaHills = giant_tree_taiga_hills,
    wooded_mountains,           extremeHillsPlus = wooded_mountains,
    savanna,
    savanna_plateau,            savannaPlateau = savanna_plateau,
    badlands,                   mesa = badlands,
    wooded_badlands_plateau,    mesaPlateau_F = wooded_badlands_plateau,
    badlands_plateau,           mesaPlateau = badlands_plateau,
    // 40  --  1.13
    small_end_islands,
    end_midlands,
    end_highlands,
    end_barrens,
    warm_ocean,                 warmOcean = warm_ocean,
    lukewarm_ocean,             lukewarmOcean = lukewarm_ocean,
    cold_ocean,                 coldOcean = cold_ocean,
    deep_warm_ocean,            warmDeepOcean = deep_warm_ocean,
    deep_lukewarm_ocean,        lukewarmDeepOcean = deep_lukewarm_ocean,
    deep_cold_ocean,            coldDeepOcean = deep_cold_ocean,
    // 50
    deep_frozen_ocean,          frozenDeepOcean = deep_frozen_ocean,
    BIOME_NUM,

    the_void = 127,

    // mutated variants
    sunflower_plains                = plains+128,
    desert_lakes                    = desert+128,
    gravelly_mountains              = mountains+128,
    flower_forest                   = forest+128,
    taiga_mountains                 = taiga+128,
    swamp_hills                     = swamp+128,
    ice_spikes                      = snowy_tundra+128,
    modified_jungle                 = jungle+128,
    modified_jungle_edge            = jungle_edge+128,
    tall_birch_forest               = birch_forest+128,
    tall_birch_hills                = birch_forest_hills+128,
    dark_forest_hills               = dark_forest+128,
    snowy_taiga_mountains           = snowy_taiga+128,
    giant_spruce_taiga              = giant_tree_taiga+128,
    giant_spruce_taiga_hills        = giant_tree_taiga_hills+128,
    modified_gravelly_mountains     = wooded_mountains+128,
    shattered_savanna               = savanna+128,
    shattered_savanna_plateau       = savanna_plateau+128,
    eroded_badlands                 = badlands+128,
    modified_wooded_badlands_plateau = wooded_badlands_plateau+128,
    modified_badlands_plateau       = badlands_plateau+128,
    // 1.14
    bamboo_jungle                   = 168,
    bamboo_jungle_hills             = 169,
    // 1.16
    soul_sand_valley                = 170,
    crimson_forest                  = 171,
    warped_forest                   = 172,
    basalt_deltas                   = 173,
    // 1.17
    dripstone_caves                 = 174,
    lush_caves                      = 175,
    // 1.18
    meadow                          = 177,
    grove                           = 178,
    snowy_slopes                    = 179,
    jagged_peaks                    = 180,
    frozen_peaks                    = 181,
    stony_peaks                     = 182,
    old_growth_birch_forest         = tall_birch_forest,
    old_growth_pine_taiga           = giant_tree_taiga,
    old_growth_spruce_taiga         = giant_spruce_taiga,
    snowy_plains                    = snowy_tundra,
    sparse_jungle                   = jungle_edge,
    stony_shore                     = stone_shore,
    windswept_hills                 = mountains,
    windswept_forest                = wooded_mountains,
    windswept_gravelly_hills        = gravelly_mountains,
    windswept_savanna               = shattered_savanna,
    wooded_badlands                 = wooded_badlands_plateau,
};


enum BiomeTempCategory
{
    Oceanic, Warm, Lush, Cold, Freezing, Special
};


/* Enumeration of the layer indices in the layer stack. */
enum LayerId
{
    // new                  [[deprecated]]
    L_CONTINENT_4096 = 0,   L_ISLAND_4096 = L_CONTINENT_4096,
    L_ZOOM_2048,
    L_LAND_2048,            L_ADD_ISLAND_2048 = L_LAND_2048,
    L_ZOOM_1024,
    L_LAND_1024_A,          L_ADD_ISLAND_1024A = L_LAND_1024_A,
    L_LAND_1024_B,          L_ADD_ISLAND_1024B = L_LAND_1024_B,     // 1.7+
    L_LAND_1024_C,          L_ADD_ISLAND_1024C = L_LAND_1024_C,     // 1.7+
    L_ISLAND_1024,          L_REMOVE_OCEAN_1024 = L_ISLAND_1024,    // 1.7+
    L_SNOW_1024,            L_ADD_SNOW_1024 = L_SNOW_1024,
    L_LAND_1024_D,          L_ADD_ISLAND_1024D = L_LAND_1024_D,     // 1.7+
    L_COOL_1024,            L_COOL_WARM_1024 = L_COOL_1024,         // 1.7+
    L_HEAT_1024,            L_HEAT_ICE_1024 = L_HEAT_1024,          // 1.7+
    L_SPECIAL_1024,                                                 // 1.7+
    L_ZOOM_512,
    L_LAND_512,                                                     // 1.6-
    L_ZOOM_256,
    L_LAND_256,             L_ADD_ISLAND_256 = L_LAND_256,
    L_MUSHROOM_256,         L_ADD_MUSHROOM_256 = L_MUSHROOM_256,
    L_DEEP_OCEAN_256,                                               // 1.7+
    L_BIOME_256,
    L_BAMBOO_256,           L14_BAMBOO_256 = L_BAMBOO_256,          // 1.14+
    L_ZOOM_128,
    L_ZOOM_64,
    L_BIOME_EDGE_64,
    L_NOISE_256,            L_RIVER_INIT_256 = L_NOISE_256,
    L_ZOOM_128_HILLS,
    L_ZOOM_64_HILLS,
    L_HILLS_64,
    L_SUNFLOWER_64,         L_RARE_BIOME_64 = L_SUNFLOWER_64,       // 1.7+
    L_ZOOM_32,
    L_LAND_32,              L_ADD_ISLAND_32 = L_LAND_32,
    L_ZOOM_16,
    L_SHORE_16,             // NOTE: in 1.0 this slot is scale 1:32
    L_SWAMP_RIVER_16,                                               // 1.6-
    L_ZOOM_8,
    L_ZOOM_4,
    L_SMOOTH_4,
    L_ZOOM_128_RIVER,
    L_ZOOM_64_RIVER,
    L_ZOOM_32_RIVER,
    L_ZOOM_16_RIVER,
    L_ZOOM_8_RIVER,
    L_ZOOM_4_RIVER,
    L_RIVER_4,
    L_SMOOTH_4_RIVER,
    L_RIVER_MIX_4,
    L_OCEAN_TEMP_256,       L13_OCEAN_TEMP_256 = L_OCEAN_TEMP_256,  // 1.13+
    L_ZOOM_128_OCEAN,       L13_ZOOM_128 = L_ZOOM_128_OCEAN,        // 1.13+
    L_ZOOM_64_OCEAN,        L13_ZOOM_64 = L_ZOOM_64_OCEAN,          // 1.13+
    L_ZOOM_32_OCEAN,        L13_ZOOM_32 = L_ZOOM_32_OCEAN,          // 1.13+
    L_ZOOM_16_OCEAN,        L13_ZOOM_16 = L_ZOOM_16_OCEAN,          // 1.13+
    L_ZOOM_8_OCEAN,         L13_ZOOM_8 = L_ZOOM_8_OCEAN,            // 1.13+
    L_ZOOM_4_OCEAN,         L13_ZOOM_4 = L_ZOOM_4_OCEAN,            // 1.13+
    L_OCEAN_MIX_4,          L13_OCEAN_MIX_4 = L_OCEAN_MIX_4,        // 1.13+

    L_VORONOI_1,            L_VORONOI_ZOOM_1 = L_VORONOI_1,

    // largeBiomes layers
    L_ZOOM_LARGE_A,
    L_ZOOM_LARGE_B,
    L_ZOOM_L_RIVER_A,
    L_ZOOM_L_RIVER_B,

    L_NUM
};


STRUCT(Range)
{
    // Cuboidal range, given by a position, size and scaling in the horizontal
    // axes, used to define a generation range. The parameters for the vertical
    // control can be left at zero when dealing with versions without 3D volume
    // support. The vertical scaling is equal to 1:1 iff scale == 1, and 1:4
    // (default biome scale) in all other cases!
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


struct Layer;
typedef int (mapfunc_t)(const struct Layer *, int *, int, int, int, int);

STRUCT(Layer)
{
    mapfunc_t *getMap;

    int8_t mc;          // minecraft version
    int8_t zoom;        // zoom factor of layer
    int8_t edge;        // maximum border required from parent layer
    int scale;          // scale of this layer (cell = scale x scale blocks)

    uint64_t layerSalt; // processed salt or initialization mode
    uint64_t startSalt; // (depends on world seed) used to step PRNG forward
    uint64_t startSeed; // (depends on world seed) start for chunk seeds

    void *noise;        // (depends on world seed) noise map data
    void *data;         // generic data for custom layers

    Layer *p, *p2;      // parent layers
};

// Overworld biome generator up to 1.17
STRUCT(LayerStack)
{
    Layer layers[L_NUM];
    Layer *entry_1;     // entry scale (1:1) [L_VORONOI_1]
    Layer *entry_4;     // entry scale (1:4) [L_RIVER_MIX_4|L_OCEAN_MIX_4]
    // unofficial entries for other scales (latest sensible layers):
    Layer *entry_16;    // [L_SWAMP_RIVER_16|L_SHORE_16]
    Layer *entry_64;    // [L_HILLS_64|L_SUNFLOWER_64]
    Layer *entry_256;   // [L_BIOME_256|L_BAMBOO_256]
    PerlinNoise oceanRnd;
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
typedef PerlinNoise EndNoise;

STRUCT(SurfaceNoise)
{
    double xzScale, yScale;
    double xzFactor, yFactor;
    OctaveNoise octmin;
    OctaveNoise octmax;
    OctaveNoise octmain;
    PerlinNoise oct[16+16+8];
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

/// Overworld and Nether biome generator for 1.18
STRUCT(BiomeNoise)
{
    DoublePerlinNoise shift;
    DoublePerlinNoise temperature;
    DoublePerlinNoise humidity;
    DoublePerlinNoise continentalness;
    DoublePerlinNoise erosion;
    DoublePerlinNoise weirdness;
    PerlinNoise oct[2*23]; // buffer for octaves in double perlin noise
    Spline *sp;
    SplineStack ss;
    int previdx;
};


#ifdef __cplusplus
extern "C"
{
#endif

//==============================================================================
// Essentials
//==============================================================================

void initBiomes();

/* Applies the given world seed to the layer and all dependent layers. */
void setLayerSeed(Layer *layer, uint64_t worldSeed);


//==============================================================================
// Noise
//==============================================================================

void initSurfaceNoise(SurfaceNoise *rnd, uint64_t *seed,
        double xzScale, double yScale, double xzFactor, double yFactor);
void initSurfaceNoiseEnd(SurfaceNoise *rnd, uint64_t seed);
double sampleSurfaceNoise(const SurfaceNoise *rnd, int x, int y, int z);


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
void setEndSeed(EndNoise *en, uint64_t seed);
int mapEndBiome(const EndNoise *en, int *out, int x, int z, int w, int h);
int mapEnd(const EndNoise *en, int *out, int x, int z, int w, int h);
int getSurfaceHeightEnd(int mc, uint64_t seed, int x, int z);
/**
 * The scaled End generation supports scales 1, 4, 16, and 64.
 * The End biomes are usually 2D, but in 1.15+ there is 3D voronoi noise, which
 * is controlled by the 'sha' hash of the seed. For scales higher than 1:1, and
 * versions up to 1.14, 'sha' is ignored.
 */
int genEndScaled(const EndNoise *en, int *out, Range r, int mc, uint64_t sha);

/**
 * In 1.18 the Overworld uses a new noise map system for the biome generation.
 * The random number generation has also updated to a Xiroshiro128 algorithm.
 * The scale is 1:4, and is sampled at each point individually as there is
 * currently not much benefit from generating a volume as a whole.
 *
 * The 1.18 End generation remains similar to 1.17 and does NOT use the
 * biome noise.
 */
enum {
    SAMPLE_NO_SHIFT = 0x1,
    SAMPLE_NO_DEPTH = 0x2,
    SAMPLE_NO_BIOME = 0x4,
};
void initBiomeNoise(BiomeNoise *bn, int mc);
void setBiomeSeed(BiomeNoise *bn, uint64_t seed, int large);
int sampleBiomeNoise(const BiomeNoise *bn, int64_t *np, int x, int y, int z,
    uint64_t *dat, uint32_t flags);
/**
 * Currently, in 1.18, we have to generate biomes a chunk at a time to get an
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
 * The scaled biome noise generation applies for the Overworld version 1.18.
 * The 'sha' hash of the seed is only required for voronoi at scale 1:1.
 * A scale of zero is interpreted as the default 1:4 scale.
 */
int genBiomeNoiseScaled(const BiomeNoise *bn, int *out, Range r, int mc, uint64_t sha);


//==============================================================================
// BiomeID Helpers
//==============================================================================

int biomeExists(int mc, int id);
int isOverworld(int mc, int id);
int getMutated(int mc, int id);
int getCategory(int mc, int id);
int areSimilar(int mc, int id1, int id2);
int isMesa(int id);
int isShallowOcean(int id);
int isDeepOcean(int id);
int isOceanic(int id);
int isSnowy(int id);


//==============================================================================
// Layers
//==============================================================================

//                             old names
mapfunc_t mapContinent;     // mapIsland
mapfunc_t mapZoomFuzzy;
mapfunc_t mapZoom;
mapfunc_t mapLand;          // mapAddIsland
mapfunc_t mapLand16;
mapfunc_t mapIsland;        // mapRemoveTooMuchOcean
mapfunc_t mapSnow;          // mapAddSnow
mapfunc_t mapSnow16;
mapfunc_t mapCool;          // mapCoolWarm
mapfunc_t mapHeat;          // mapHeatIce
mapfunc_t mapSpecial;
mapfunc_t mapMushroom;      // mapAddMushroomIsland
mapfunc_t mapDeepOcean;
mapfunc_t mapBiome;
mapfunc_t mapBamboo;        // mapAddBamboo
mapfunc_t mapNoise;         // mapRiverInit
mapfunc_t mapBiomeEdge;
mapfunc_t mapHills;
mapfunc_t mapRiver;
mapfunc_t mapSmooth;
mapfunc_t mapSunflower;     // mapRareBiome
mapfunc_t mapShore;
mapfunc_t mapSwampRiver;
mapfunc_t mapRiverMix;
mapfunc_t mapOceanTemp;
mapfunc_t mapOceanMix;

// final layer 1:1
mapfunc_t mapVoronoi;       // mapVoronoiZoom
mapfunc_t mapVoronoi114;

// With 1.15 voronoi changed in preparation for 3D biome generation.
// Biome generation now stops at scale 1:4 OceanMix and voronoi is just an
// access algorithm, mapping the 1:1 scale onto its 1:4 correspondent.
// It is seeded by the first 8-bytes of the SHA-256 hash of the world seed.
ATTR(const)
uint64_t getVoronoiSHA(uint64_t worldSeed);
void voronoiAccess3D(uint64_t sha, int x, int y, int z, int *x4, int *y4, int *z4);

// Gets the range in the parent/source layer which may be accessed by voronoi.
Range getVoronoiSrcRange(Range r);

// Applies a 2D voronoi mapping at height 'y' to a 'src' plane, where
// src_range [px,pz,pw,ph] -> out_range [x,z,w,h] have to match the scaling.
void mapVoronoiPlane(uint64_t sha, int *out, int *src,
    int x, int z, int w, int h, int y, int px, int pz, int pw, int ph);


#ifdef __cplusplus
}
#endif

#endif /* LAYER_H_ */
