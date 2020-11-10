typedef long int64_t;
typedef unsigned long uint64_t;

enum {
    L_ISLAND_4096 = 0,
    L_ZOOM_2048,
    L_ADD_ISLAND_2048,
    L_ZOOM_1024,
    L_ADD_ISLAND_1024A,
    L_ADD_ISLAND_1024B,
    L_ADD_ISLAND_1024C,
    L_REMOVE_OCEAN_1024,
    L_ADD_SNOW_1024,
    L_ADD_ISLAND_1024D,
    L_COOL_WARM_1024,
    L_HEAT_ICE_1024,
    L_SPECIAL_1024,  /* Good entry for: temperature categories */
    L_ZOOM_512,
    L_ZOOM_256,
    L_ADD_ISLAND_256,
    L_ADD_MUSHROOM_256, /* Good entry for: mushroom biomes */
    L_DEEP_OCEAN_256,
    L_BIOME_256, /* Good entry for: major biome types */
    L_ZOOM_128,
    L_ZOOM_64,
    L_BIOME_EDGE_64,
    L_RIVER_INIT_256,
    L_ZOOM_128_HILLS,
    L_ZOOM_64_HILLS,
    L_HILLS_64, /* Good entry for: minor biome types */
    L_RARE_BIOME_64,
    L_ZOOM_32,
    L_ADD_ISLAND_32,
    L_ZOOM_16,
    L_SHORE_16,
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
    L_VORONOI_ZOOM_1,

    // 1.13 layers
    L13_OCEAN_TEMP_256,
    L13_ZOOM_128,
    L13_ZOOM_64,
    L13_ZOOM_32,
    L13_ZOOM_16,
    L13_ZOOM_8,
    L13_ZOOM_4,
    L13_OCEAN_MIX_4,

    // 1.14 layers
    L14_BAMBOO_256,

    // largeBiomes layers
    L_ZOOM_LARGE_BIOME_A,
    L_ZOOM_LARGE_BIOME_B,

    L_NUM
};

struct Biome {
    int id;
    int type;
    double height;
    double temp;
    int tempCat;
    int mutated;
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
};

enum BiomeType
{
    Void = -1,
    Ocean, Plains, Desert, Hills, Forest, Taiga, Swamp, River, Nether, Sky, Snow, MushroomIsland, Beach, Jungle, StoneBeach, Savanna, Mesa,
    BTYPE_NUM
};

enum BiomeTempCategory
{
    Oceanic, Warm, Lush, Cold, Freezing, Special
};


struct Layer {
    int64_t startSeed;
    int64_t startSalt;
};

static inline int mcFirstIsZero(int64_t s, int mod) {
    return (int)((s >> 24) % mod) == 0;
}

static inline int64_t mcStepSeed(int64_t s, int64_t salt) {
    return s * (s * 6364136223846793005LL + 1442695040888963407LL) + salt;
}

static inline int mcFirstInt(int64_t s, int mod)
{
    int ret = (int)((s >> 24) % mod);
    if (ret < 0)
        ret += mod;
    return ret;
}

static inline int64_t getChunkSeed(int64_t ss, int x, int z) {
    int64_t cs = ss + x;
    cs = mcStepSeed(cs, z);
    cs = mcStepSeed(cs, x);
    cs = mcStepSeed(cs, z);
    return cs;
}

static inline int isShallowOcean(int id)
{
    const uint64_t shallow_bits =
            (1ULL << ocean) |
            (1ULL << frozen_ocean) |
            (1ULL << warm_ocean) |
            (1ULL << lukewarm_ocean) |
            (1ULL << cold_ocean);
    return id < 64 && ((1ULL << id) & shallow_bits);
}

static inline int isDeepOcean(int id)
{
    const uint64_t deep_bits =
            (1ULL << deep_ocean) |
            (1ULL << deep_warm_ocean) |
            (1ULL << deep_lukewarm_ocean) |
            (1ULL << deep_cold_ocean) |
            (1ULL << deep_frozen_ocean);
    return id < 64 && ((1ULL << id) & deep_bits);
}

static inline int getBiomeType(__constant struct Biome* biomes, int id)
{
    return (id & (~0xff)) ? Void : biomes[id].type;
}

static inline int biomeExists(__constant struct Biome* biomes, int id)
{
    return !(id & (~0xff)) && !(biomes[id].id & (~0xff));
}

static inline int areSimilar113(__constant struct Biome* biomes, int id1, int id2)
{
    if (id1 == id2) return 1;
    if (id1 == wooded_badlands_plateau || id1 == badlands_plateau)
        return id2 == wooded_badlands_plateau || id2 == badlands_plateau;
    if (!biomeExists(biomes, id1) || !biomeExists(biomes, id2)) return 0;
    return getBiomeType(biomes, id1) == getBiomeType(biomes, id2);
}

static inline int replaceEdge(__constant struct Biome* biomes, __global int *out, int idx, int v10, int v21, int v01, int v12, int id, int baseID, int edgeID) {
    if (id != baseID) return 0;

    // areSimilar() has not changed behaviour for ids < 128, so use the faster variant
    if (areSimilar113(biomes, v10, baseID) && areSimilar113(biomes, v21, baseID) &&
        areSimilar113(biomes, v01, baseID) && areSimilar113(biomes, v12, baseID))
        out[idx] = id;
    else
        out[idx] = edgeID;

    return 1;
}

static inline int isOceanic(int id)
{
    const uint64_t ocean_bits =
            (1ULL << ocean) |
            (1ULL << frozen_ocean) |
            (1ULL << warm_ocean) |
            (1ULL << lukewarm_ocean) |
            (1ULL << cold_ocean) |
            (1ULL << deep_ocean) |
            (1ULL << deep_warm_ocean) |
            (1ULL << deep_lukewarm_ocean) |
            (1ULL << deep_cold_ocean) |
            (1ULL << deep_frozen_ocean);
    return id < 64 && ((1ULL << id) & ocean_bits);
}

static inline int64_t getLayerSeed(int64_t salt)
{
    int64_t ls = mcStepSeed(salt, salt);
    ls = mcStepSeed(ls, salt);
    ls = mcStepSeed(ls, salt);
    return ls;
}

void set_layer_seed(__global struct Layer* layer, int64_t salt, int64_t worldSeed) {
    salt = getLayerSeed(salt);
    int64_t st = worldSeed;
    st = mcStepSeed(st, salt);
    st = mcStepSeed(st, salt);
    st = mcStepSeed(st, salt);

    layer->startSalt = st;
    layer->startSeed = mcStepSeed(st, 0);
}

__kernel void setSeed(int64_t seed_start, __global struct Layer* layers) {
    int offset = get_global_id(0) * L_NUM;
    int seed = seed_start + get_global_id(0);
    set_layer_seed(&layers[offset + L_ISLAND_4096], 1, seed);
    set_layer_seed(&layers[offset + L_ZOOM_2048], 2000, seed);
    set_layer_seed(&layers[offset + L_ADD_ISLAND_2048], 1, seed);
    set_layer_seed(&layers[offset + L_ZOOM_1024], 2001, seed);
    set_layer_seed(&layers[offset + L_ADD_ISLAND_1024A], 2, seed);
    set_layer_seed(&layers[offset + L_ADD_ISLAND_1024B], 50, seed);
    set_layer_seed(&layers[offset + L_ADD_ISLAND_1024C], 70, seed);
    set_layer_seed(&layers[offset + L_REMOVE_OCEAN_1024], 2, seed);
    set_layer_seed(&layers[offset + L_ADD_SNOW_1024], 2, seed);
    set_layer_seed(&layers[offset + L_ADD_ISLAND_1024D], 3, seed);
    set_layer_seed(&layers[offset + L_COOL_WARM_1024], 2, seed);
    set_layer_seed(&layers[offset + L_HEAT_ICE_1024], 2, seed);
    set_layer_seed(&layers[offset + L_SPECIAL_1024], 3, seed);
    set_layer_seed(&layers[offset + L_ZOOM_512], 2002, seed);
    set_layer_seed(&layers[offset + L_ZOOM_256], 2003, seed);
    set_layer_seed(&layers[offset + L_ADD_ISLAND_256], 4, seed);
    set_layer_seed(&layers[offset + L_ADD_MUSHROOM_256], 5, seed);
    set_layer_seed(&layers[offset + L_DEEP_OCEAN_256], 4, seed);
    set_layer_seed(&layers[offset + L_BIOME_256], 200, seed);
    set_layer_seed(&layers[offset + L14_BAMBOO_256], 1001, seed);
    set_layer_seed(&layers[offset + L_ZOOM_128], 1000, seed);
    set_layer_seed(&layers[offset + L_ZOOM_64], 1001, seed);
    set_layer_seed(&layers[offset + L_BIOME_EDGE_64], 1000, seed);
    set_layer_seed(&layers[offset + L_RIVER_INIT_256], 100, seed);
    set_layer_seed(&layers[offset + L_ZOOM_128_HILLS], 1000, seed);
    set_layer_seed(&layers[offset + L_ZOOM_64_HILLS], 1001, seed);
    set_layer_seed(&layers[offset + L_HILLS_64], 1000, seed);
    set_layer_seed(&layers[offset + L_RARE_BIOME_64], 1001, seed);
    set_layer_seed(&layers[offset + L_ZOOM_32], 1000, seed);
    set_layer_seed(&layers[offset + L_ADD_ISLAND_32], 3, seed);
    set_layer_seed(&layers[offset + L_ZOOM_16], 1001, seed);
    set_layer_seed(&layers[offset + L_SHORE_16], 1000, seed);
}

__kernel void mapIsland(__constant struct Layer* layer, int layer_id, int4 dims, __constant int* in, __global int* out) {
    int seed_offset = layer_id + (int)get_global_id(2) * L_NUM;

    int i = (int)get_global_id(0);
    int j = (int)get_global_id(1);
    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;
    int64_t ss = layer[seed_offset].startSeed;
    // Frame in seed range
    in = in + w * h * get_global_id(2);
    out = out + w * h * get_global_id(2);

    int64_t cs = getChunkSeed(ss, x + i, z + j);
    out[i + j*w] = mcFirstIsZero(cs, 10);

    if (x > -w && x <= 0 && z > -h && z <= 0)
    {
        out[-x + -z * w] = 1;
    }
}

__kernel void mapZoomIsland(__constant struct Layer* layer, int layer_id, int4 dims, __constant int* in, __global int* out) {
    int seed_offset = layer_id + (int)get_global_id(2) * L_NUM;
    int i = (int)get_global_id(0);
    int j = (int)get_global_id(1);
    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;
    int st = (int)layer[seed_offset].startSalt;
    int ss = (int)layer[seed_offset].startSeed;

    int pX = x >> 1;
    int pZ = z >> 1;
    int pW = ((x + w) >> 1) - pX + 1;
    int pH = ((z + h) >> 1) - pZ + 1;

    // Frame in seed range
    in = in + pW * pH * get_global_id(2);
    out = out + w * h * get_global_id(2);

    int newW = pW << 1;
    int newH = pH << 1;

    int v00 = in[(j+0)*pW + i];
    int v01 = in[(j+1)*pW + i];
    int v10 = in[(j+0)*pW + i+1];
    int v11 = in[(j+1)*pW + i+1];

    int chunkX = (i + pX) << 1;
    int chunkZ = (j + pZ) << 1;

    int cs = ss;
    cs += chunkX;
    cs *= cs * 1284865837 + 4150755663;
    cs += chunkZ;
    cs *= cs * 1284865837 + 4150755663;
    cs += chunkX;
    cs *= cs * 1284865837 + 4150755663;
    cs += chunkZ;

    int zz = 2 * j - (z & 1);
    int xx = 2 * i - (x & 1);

    bool z0 = zz >= 0 && zz < h;
    bool z1 = zz + 1 < h;
    bool x0 = xx >= 0 && xx < w;
    bool x1 = xx + 1 < w;

    if (z0 && x0) {
        out[xx + zz * w] = v00;
    }
    if (z1 && x0) {
        out[xx + (zz + 1) * w] = (cs >> 24) & 1 ? v01 : v00;
    }

    cs *= cs * 1284865837 + 4150755663;
    cs += st;

    if (z0 && x1) {
        out[xx + 1 + zz * w] = (cs >> 24) & 1 ? v10 : v00;
    }

    cs *= cs * 1284865837 + 4150755663;
    cs += st;
    int r = (cs >> 24) & 3;

    if (z1 && x1) {
        out[xx + 1 + (zz + 1) * w] = r==0 ? v00 : r==1 ? v10 : r==2 ? v01 : v11;
    }
}

__kernel void mapAddIsland(__constant struct Layer* layer, int layer_id, int4 dims, __constant int* in, __global int* out) {
    int seed_offset = layer_id + (int)get_global_id(2) * L_NUM;

    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;

    int pX = dims.s0 - 1;
    int pZ = dims.s1 - 1;
    int pW = dims.s2 + 2;
    int pH = dims.s3 + 2;

    int64_t st = layer[seed_offset].startSalt;
    int64_t ss = layer[seed_offset].startSeed;

    // Frame in seed range
    in = in + pW * pH * get_global_id(2);
    out = out + w * h * get_global_id(2);


    int i = (int)get_global_id(0);
    int j = (int)get_global_id(1);

    int v00 = in[i + j * pW];
    int v02 = in[i + (j + 2) * pW];
    int v20 = in[(i + 2) + j * pW];
    int v22 = in[(i + 2) + (j + 2) * pW];
    int v11 = in[(i + 1) + (j + 1) * pW];

    int v = v11;

    switch (v11) {
        case 0:
            if (v00 != 0 || v20 != 0 || v02 != 0 || v22 != 0) {
                int64_t cs = getChunkSeed(ss, i+x, j+z);
                int inc = 0;
                v = 1;
                if (v00 != 0) {
                    ++inc; v = v00;
                    cs = mcStepSeed(cs, st);
                }
                if (v20 != 0) {
                    if (++inc == 1 || mcFirstIsZero(cs, 2)) v = v20;
                    cs = mcStepSeed(cs, st);
                }
                if (v02 != 0) {
                    switch (++inc) {
                        case 1: v = v02; break;
                        case 2: if (mcFirstIsZero(cs, 2)) v = v02; break;
                        default: if (mcFirstIsZero(cs, 3)) v = v02;
                    }
                    cs = mcStepSeed(cs, st);
                }
                if (v22 != 0) {
                    switch (++inc) {
                        case 1: v = v22; break;
                        case 2: if (mcFirstIsZero(cs, 2)) v = v22; break;
                        case 3: if (mcFirstIsZero(cs, 3)) v = v22; break;
                        default: if (mcFirstIsZero(cs, 4)) v = v22;
                    }
                    cs = mcStepSeed(cs, st);
                }

                if (v != 4 && !mcFirstIsZero(cs, 3)) {
                    v = 0;
                }
            }
            break;
        case 4:
            break;
        default:
            if (v00 == 0 || v20 == 0 || v02 == 0 || v22 == 0) {
                int64_t cs = getChunkSeed(ss, i+x, j+z);
                if (mcFirstIsZero(cs, 5)) {
                    v = 0;
                }
            }
    }

    out[i + j*w] = v;
}

__kernel void removeBrim(int4 dims, __constant int* in, __global int* out) {
    int w = dims.s2;
    int h = dims.s3;
    int pW = w + 2;
    int pH = h + 2;

    // Frame in seed range
    in = in + pW * pH * get_global_id(2);
    out = out + w * h * get_global_id(2);

    int i = (int)get_global_id(0);
    int j = (int)get_global_id(1);
    out[i + j*w] = in[(i+1) + (j+1)*pW];
}

__kernel void mapZoom(__constant struct Layer* layer, int layer_id, int4 dims, __constant int* in, __global int* out) {
    int seed_offset = layer_id + (int)get_global_id(2) * L_NUM;
    int i = (int)get_global_id(0);
    int j = (int)get_global_id(1);
    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;
    int st = (int)layer[seed_offset].startSalt;
    int ss = (int)layer[seed_offset].startSeed;

    int pX = x >> 1;
    int pZ = z >> 1;
    int pW = ((x + w) >> 1) - pX + 1;
    int pH = ((z + h) >> 1) - pZ + 1;

    // Frame in seed range
    in = in + pW * pH * get_global_id(2);
    out = out + w * h * get_global_id(2);

    int newW = pW << 1;
    int newH = pH << 1;

    int v00 = in[(j+0)*pW + i];
    int v01 = in[(j+1)*pW + i];
    int v10 = in[(j+0)*pW + i+1];
    int v11 = in[(j+1)*pW + i+1];

    int chunkX = (i + pX) << 1;
    int chunkZ = (j + pZ) << 1;

    int cs = ss;
    cs += chunkX;
    cs *= cs * 1284865837 + 4150755663;
    cs += chunkZ;
    cs *= cs * 1284865837 + 4150755663;
    cs += chunkX;
    cs *= cs * 1284865837 + 4150755663;
    cs += chunkZ;

    int zz = 2 * j - (z & 1);
    int xx = 2 * i - (x & 1);

    bool z0 = zz >= 0 && zz < h;
    bool z1 = zz + 1 < h;
    bool x0 = xx >= 0 && xx < w;
    bool x1 = xx + 1 < w;

    if (z0 && x0) {
        out[xx + zz * w] = v00;
    }
    if (z1 && x0) {
        out[xx + (zz + 1) * w] = (cs >> 24) & 1 ? v01 : v00;
    }
    cs *= cs * 1284865837 + 4150755663;
    cs += st;
    if (z0 && x1) {
        out[xx + 1 + zz * w] = (cs >> 24) & 1 ? v10 : v00;
    }
    int v;
    if      (v10 == v01 && v01 == v11) v = v10;
    else if (v00 == v10 && v00 == v01) v = v00;
    else if (v00 == v10 && v00 == v11) v = v00;
    else if (v00 == v01 && v00 == v11) v = v00;
    else if (v00 == v10 && v01 != v11) v = v00;
    else if (v00 == v01 && v10 != v11) v = v00;
    else if (v00 == v11 && v10 != v01) v = v00;
    else if (v10 == v01 && v00 != v11) v = v10;
    else if (v10 == v11 && v00 != v01) v = v10;
    else if (v01 == v11 && v00 != v10) v = v01;
    else
    {
        cs *= cs * 1284865837 + 4150755663;
        cs += st;
        int r = (cs >> 24) & 3;
        v = r==0 ? v00 : r==1 ? v10 : r==2 ? v01 : v11;
    }
    if (z1 && x1) {
        out[xx + 1 + (zz + 1) * w] = v;
    }
}

__kernel void mapRemoveTooMuchOcean(__constant struct Layer* layers, int layer_id, int4 dims, __constant int* in, __global int* out) {
    int seed_offset = layer_id + (int)get_global_id(2) * L_NUM;
    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;

    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;

    // Frame in seed range
    in = in + pW * pH * get_global_id(2);
    out = out + w * h * get_global_id(2);

    int i = get_global_id(0);
    int j = get_global_id(1);

    int v10 = in[i+1 + (j+0)*pW];
    int v01 = in[i+0 + (j+1)*pW];
    int v11 = in[i+1 + (j+1)*pW];
    int v21 = in[i+2 + (j+1)*pW];
    int v12 = in[i+1 + (j+2)*pW];

    int v = v11;
    if (v10 == 0 && v01 == 0 && v11 == 0 && v21 == 0 && v12 == 0) {
        int64_t ss = layers[seed_offset].startSeed;
        int64_t cs = getChunkSeed(ss, i+x, j+z);
        if (mcFirstIsZero(cs, 2)) {
            v = 1;
        }
    }
    out[i + j*w] = v;
}

__kernel void mapAddSnow(__constant struct Layer* layers, int layer_id, int4 dims, __constant int* in, __global int* out) {
    int seed_offset = layer_id + (int)get_global_id(2) * L_NUM;
    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;

    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;

    // Frame in seed range
    in = in + pW * pH * get_global_id(2);
    out = out + w * h * get_global_id(2);

    int i = get_global_id(0);
    int j = get_global_id(1);

    int v11 = in[i+1 + (j+1)*pW];
    
    int64_t ss = layers[seed_offset].startSeed;
    int64_t cs;

    if (isShallowOcean(v11)) {
        out[i + j*w] = v11;
    } else {
        cs = getChunkSeed(ss, i+x, j+z);
        int r = mcFirstInt(cs, 6);
        int v;

        if (r == 0) v = 4;
        else if (r <= 1) v = 3;
        else v = 1;

        out[i + j*w] = v;
    }
}

__kernel void mapCoolWarm(__constant struct Layer* layers, int layer_id, int4 dims, __constant int* in, __global int* out) {
    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;

    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;

    // Frame in seed range
    in = in + pW * pH * get_global_id(2);
    out = out + w * h * get_global_id(2);

    int i = get_global_id(0);
    int j = get_global_id(1);

    int v11 = in[i+1 + (j+1)*pW];

    if (v11 == 1)
    {
        int v10 = in[i+1 + (j+0)*pW];
        int v21 = in[i+2 + (j+1)*pW];
        int v01 = in[i+0 + (j+1)*pW];
        int v12 = in[i+1 + (j+2)*pW];

        if (v10 == 3 || v10 == 4 || v21 == 3 || v21 == 4 || v01 == 3 || v01 == 4 || v12 == 3 || v12 == 4)
        {
            v11 = 2;
        }
    }

    out[i + j*w] = v11;
}

__kernel void mapHeatIce(__constant struct Layer* layer, int layer_id, int4 dims, __constant int* in, __global int* out) {
    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;

    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;

    // Frame in seed range
    in = in + pW * pH * get_global_id(2);
    out = out + w * h * get_global_id(2);

    int i = get_global_id(0);
    int j = get_global_id(1);

    int v11 = in[i+1 + (j+1)*pW];

    if (v11 == 4) {
        int v10 = in[i+1 + (j+0)*pW];
        int v21 = in[i+2 + (j+1)*pW];
        int v01 = in[i+0 + (j+1)*pW];
        int v12 = in[i+1 + (j+2)*pW];

        if (v10 == 1 || v10 == 2 || v21 == 1 || v21 == 2 || v01 == 1 || v01 == 2 || v12 == 1 || v12 == 2)
        {
            v11 = 3;
        }
    }

    out[i + j*w] = v11;
}

__kernel void mapSpecial(__constant struct Layer* layers, int layer_id, int4 dims, __constant int* in, __global int* out) {
    int seed_offset = layer_id + (int)get_global_id(2) * L_NUM;
    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;

    // Frame in seed range
    in = in + w * h * get_global_id(2);
    out = out + w * h * get_global_id(2);

    int i = get_global_id(0);
    int j = get_global_id(1);
    
    int64_t st = layers[seed_offset].startSalt;
    int64_t ss = layers[seed_offset].startSeed;

    int v = in[i + j*w];
    if (v != 0) {
        int64_t cs = getChunkSeed(ss, i+x, j+z);
        if (mcFirstIsZero(cs, 13)) {
            cs = mcStepSeed(cs, st);
            v |= (1 + mcFirstInt(cs, 15)) << 8 & 0xf00;
            // 1 to 1 mapping so 'out' can be overwritten immediately
        }
    }
    out[i + j*w] = v;
}

__kernel void mapAddMushroomIsland(__constant struct Layer * layers, int layer_id, int4 dims, __constant int* in, __global int* out) {
    int seed_offset = layer_id + (int)get_global_id(2) * L_NUM;
    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;

    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;

    // Frame in seed range
    in = in + pW * pH * get_global_id(2);
    out = out + w * h * get_global_id(2);

    int i = get_global_id(0);
    int j = get_global_id(1);

    int64_t ss = layers[seed_offset].startSeed;
    int64_t cs;

    int v11 = in[i+1 + (j+1)*pW];
    // surrounded by ocean?
    if (v11 == 0 &&
        !in[i+0 + (j+0)*pW] && !in[i+2 + (j+0)*pW] &&
        !in[i+0 + (j+2)*pW] && !in[i+2 + (j+2)*pW])
    {
        cs = getChunkSeed(ss, i+x, j+z);
        if (mcFirstIsZero(cs, 100))
            v11 = mushroom_fields;
    }

    out[i + j*w] = v11;
}

__kernel void mapDeepOcean(__constant struct Layer* layers, int layer_id, int4 dims, __constant int* in, __global int* out) {
    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;

    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;

    // Frame in seed range
    in = in + pW * pH * get_global_id(2);
    out = out + w * h * get_global_id(2);

    int i = get_global_id(0);
    int j = get_global_id(1);

    int v11 = in[(i+1) + (j+1)*pW];

    if (isShallowOcean(v11))
    {
        // count adjacent oceans
        int oceans = 0;
        if (isShallowOcean(in[(i+1) + (j+0)*pW])) oceans++;
        if (isShallowOcean(in[(i+2) + (j+1)*pW])) oceans++;
        if (isShallowOcean(in[(i+0) + (j+1)*pW])) oceans++;
        if (isShallowOcean(in[(i+1) + (j+2)*pW])) oceans++;

        if (oceans >= 4)
        {
            switch (v11)
            {
            case warm_ocean:
                v11 = deep_warm_ocean;
                break;
            case lukewarm_ocean:
                v11 = deep_lukewarm_ocean;
                break;
            case ocean:
                v11 = deep_ocean;
                break;
            case cold_ocean:
                v11 = deep_cold_ocean;
                break;
            case frozen_ocean:
                v11 = deep_frozen_ocean;
                break;
            default:
                v11 = deep_ocean;
            }
        }
    }
    out[i + j*w] = v11;
}

__constant int warmBiomes[] = {desert, desert, desert, savanna, savanna, plains};
__constant int lushBiomes[] = {forest, dark_forest, mountains, plains, birch_forest, swamp};
__constant int coldBiomes[] = {forest, mountains, taiga, plains};
__constant int snowBiomes[] = {snowy_tundra, snowy_tundra, snowy_tundra, snowy_taiga};

__kernel void mapBiomes(__constant struct Layer* layers, int layer_id, int4 dims, __constant struct Biome* biomes, __constant int* in, __global int* out) {
    int seed_offset = layer_id + (int)get_global_id(2) * L_NUM;
    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;

    // Frame in seed range
    in = in + w * h * get_global_id(2);
    out = out + w * h * get_global_id(2);

    int i = get_global_id(0);
    int j = get_global_id(1);

    int64_t ss = layers[seed_offset].startSeed;
    int64_t cs;

    int idx = i + j*w;
    int id = in[idx];
    int hasHighBit = (id & 0xf00);
    id &= ~0xf00;

    if (getBiomeType(biomes, id) == Ocean || id == mushroom_fields) {
        out[idx] = id;
    } else {
        cs = getChunkSeed(ss, i + x, j + z);
        switch (id)
        {
        case Warm:
            if (hasHighBit) out[idx] = mcFirstIsZero(cs, 3) ? badlands_plateau : wooded_badlands_plateau;
            else out[idx] = warmBiomes[mcFirstInt(cs, 6)];
            break;
        case Lush:
            if (hasHighBit) out[idx] = jungle;
            else out[idx] = lushBiomes[mcFirstInt(cs, 6)];
            break;
        case Cold:
            if (hasHighBit) out[idx] = giant_tree_taiga;
            else out[idx] = coldBiomes[mcFirstInt(cs, 4)];
            break;
        case Freezing:
            out[idx] = snowBiomes[mcFirstInt(cs, 4)];
            break;
        default:
            out[idx] = mushroom_fields;
        }
    }
}

__kernel void mapAddBamboo(__constant struct Layer* layers, int layer_id, int4 dims, __constant int* in, __global int* out) {
    int seed_offset = layer_id + (int)get_global_id(2) * L_NUM;
    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;

    // Frame in seed range
    in = in + w * h * get_global_id(2);
    out = out + w * h * get_global_id(2);

    int i = get_global_id(0);
    int j = get_global_id(1);

    int64_t ss = layers[seed_offset].startSeed;
    int64_t cs;

    int idx = i + j*w;
    int v = in[idx];
    if (v == jungle) {
        cs = getChunkSeed(ss, i + x, j + z);
        if (mcFirstIsZero(cs, 10)) {
            v = bamboo_jungle;
        } 
    }
    out[idx] = v;
}

__kernel void mapBiomeEdge(__constant struct Layer* layers, int layer_id, int4 dims,  __constant struct Biome* biomes, __constant int* in, __global int* out) {
    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;

    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;

    // Frame in seed range
    in = in + pW * pH * get_global_id(2);
    out = out + w * h * get_global_id(2);

    int i = get_global_id(0);
    int j = get_global_id(1);

    int v11 = in[(i+1) + (j+1)*pW];
    int v10 = in[(i+1) + j*pW];
    int v21 = in[(i+2) + (j+1)*pW];
    int v01 = in[(i+0) + (j+1)*pW];
    int v12 = in[(i+1) + (j+2)*pW];

    if (!replaceEdge(biomes, out, i + j*w, v10, v21, v01, v12, v11, wooded_badlands_plateau, badlands) &&
        !replaceEdge(biomes, out, i + j*w, v10, v21, v01, v12, v11, badlands_plateau, badlands) &&
        !replaceEdge(biomes, out, i + j*w, v10, v21, v01, v12, v11, giant_tree_taiga, taiga))
    {
        if (v11 == desert)
        {
            if (v10 != snowy_tundra && v21 != snowy_tundra && v01 != snowy_tundra && v12 != snowy_tundra)
            {
                out[i + j*w] = v11;
            }
            else
            {
                out[i + j*w] = wooded_mountains;
            }
        }
        else if (v11 == swamp)
        {
            if (v10 != desert && v21 != desert && v01 != desert && v12 != desert &&
                v10 != snowy_taiga && v21 != snowy_taiga && v01 != snowy_taiga && v12 != snowy_taiga &&
                v10 != snowy_tundra && v21 != snowy_tundra && v01 != snowy_tundra && v12 != snowy_tundra)
            {
                if (v10 != jungle && v12 != jungle && v21 != jungle && v01 != jungle &&
                    v10 != bamboo_jungle && v12 != bamboo_jungle &&
                    v21 != bamboo_jungle && v01 != bamboo_jungle)
                    out[i + j*w] = v11;
                else
                    out[i + j*w] = jungle_edge;
            }
            else
            {
                out[i + j*w] = plains;
            }
        }
        else
        {
            out[i + j*w] = v11;
        }
    }
}

__kernel void mapRiverInit(__constant struct Layer* layers, int layer_id, int4 dims, __constant int* in, __global int* out) {
    int seed_offset = layer_id + (int)get_global_id(2) * L_NUM;
    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;

    // Frame in seed range
    in = in + w * h * get_global_id(2);
    out = out + w * h * get_global_id(2);

    int i = get_global_id(0);
    int j = get_global_id(1);

    int64_t ss = layers[seed_offset].startSeed;
    int64_t cs;

    if (in[i + j*w] > 0) {
        cs = getChunkSeed(ss, i + x, j + z);
        out[i + j*w] = mcFirstInt(cs, 299999)+2;
    } else {
        out[i + j*w] = 0;
    }
}

__kernel void mapHills13(__constant struct Layer* layers, int layer_id, int4 dims, __constant struct Biome* biomes, __constant int* in1, __constant int* in2, __global int* out) {
    int seed_offset = layer_id + (int)get_global_id(2) * L_NUM;
    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;

    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;

    // Frame in seed range
    in2 = in2 + pW * pH * get_global_id(2);
    in1 = in1 + pW * pH * get_global_id(2);
    out = out + w * h * get_global_id(2);

    int i = get_global_id(0);
    int j = get_global_id(1);

    int64_t st = layers[seed_offset].startSalt;
    int64_t ss = layers[seed_offset].startSeed;
    int64_t cs;

    int a11 = in1[i+1 + (j+1)*pW]; // biome branch
    int b11 = in2[i+1 + (j+1)*pW]; // river branch
    int idx = i + j*w;

    int bn = (b11 - 2) % 29;

    if (!isShallowOcean(a11) && b11 >= 2 && bn == 1)
    {
        int m = biomes[a11].mutated;
        if (m > 0)
            out[idx] = m;
        else
            out[idx] = a11;
    }
    else
    {
        cs = getChunkSeed(ss, i + x, j + z);
        if (bn == 0 || mcFirstIsZero(cs, 3))
        {
            int hillID = a11;

            switch(a11)
            {
            case desert:
                hillID = desert_hills; break;
            case forest:
                hillID = wooded_hills; break;
            case birch_forest:
                hillID = birch_forest_hills; break;
            case dark_forest:
                hillID = plains; break;
            case taiga:
                hillID = taiga_hills; break;
            case giant_tree_taiga:
                hillID = giant_tree_taiga_hills; break;
            case snowy_taiga:
                hillID = snowy_taiga_hills; break;
            case plains:
                cs = mcStepSeed(cs, st);
                hillID = mcFirstIsZero(cs, 3) ? wooded_hills : forest; break;
            case snowy_tundra:
                hillID = snowy_mountains; break;
            case jungle:
                hillID = jungle_hills; break;
            case bamboo_jungle:
                hillID = bamboo_jungle_hills; break;
            case ocean:
                hillID = deep_ocean; break;
            case mountains:
                hillID = wooded_mountains; break;
            case savanna:
                hillID = savanna_plateau; break;
            default:
                if (areSimilar113(biomes, a11, wooded_badlands_plateau))
                    hillID = badlands;
                else if (isDeepOcean(a11))
                {
                    cs = mcStepSeed(cs, st);
                    if (mcFirstIsZero(cs, 3))
                    {
                        cs = mcStepSeed(cs, st);
                        hillID = mcFirstIsZero(cs, 2) ? plains : forest;
                    }
                }
                break;
            }

            if (bn == 0 && hillID != a11)
            {
                hillID = biomes[hillID].mutated;
                if (hillID < 0)
                    hillID = a11;
            }

            if (hillID != a11)
            {
                int a10 = in1[i+1 + (j+0)*pW];
                int a21 = in1[i+2 + (j+1)*pW];
                int a01 = in1[i+0 + (j+1)*pW];
                int a12 = in1[i+1 + (j+2)*pW];
                int equals = 0;

                if (areSimilar113(biomes, a10, a11)) equals++;
                if (areSimilar113(biomes, a21, a11)) equals++;
                if (areSimilar113(biomes, a01, a11)) equals++;
                if (areSimilar113(biomes, a12, a11)) equals++;

                if (equals >= 3)
                    out[idx] = hillID;
                else
                    out[idx] = a11;
            }
            else
            {
                out[idx] = a11;
            }
        }
        else
        {
            out[idx] = a11;
        }
    }

}

__kernel void mapRareBiome(__constant struct Layer* layers, int layer_id, int4 dims, __constant int* in, __global int* out) {
    int seed_offset = layer_id + (int)get_global_id(2) * L_NUM;
    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;

    // Frame in seed range
    in = in + w * h * get_global_id(2);
    out = out + w * h * get_global_id(2);

    int i = get_global_id(0);
    int j = get_global_id(1);

    int64_t ss = layers[seed_offset].startSeed;
    int64_t cs;

    int v = in[i + j * w];

    if (v == plains)
    {
        cs = getChunkSeed(ss, i + x, j + z);
        if (mcFirstIsZero(cs, 57))
        {
            // Sunflower Plains
            v = plains + 128;
        }
    }
    out[i + j*w] = v;
}

inline static int isBiomeJFTO(__constant struct Biome* biomes, int id)
{
    return biomeExists(biomes, id) && (getBiomeType(biomes, id) == Jungle || id == forest || id == taiga || isOceanic(id));
}

static inline int isBiomeSnowy(__constant struct Biome* biomes, int id)
{
    return biomeExists(biomes, id) && biomes[id].temp < 0.1;
}

inline static int replaceOcean(__global int *out, int idx, int v10, int v21, int v01, int v12, int id, int replaceID)
{
    if (isOceanic(id)) return 0;

    if (!isOceanic(v10) && !isOceanic(v21) && !isOceanic(v01) && !isOceanic(v12))
        out[idx] = id;
    else
        out[idx] = replaceID;

    return 1;
}

__kernel void mapShore(__constant struct Layer* layers, int layer_id, int4 dims, __constant struct Biome* biomes, __constant int* in, __global int* out) {
    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;

    int i = get_global_id(0);
    int j = get_global_id(1);

    int pX = x - 1;
    int pZ = z - 1;
    int pW = w + 2;
    int pH = h + 2;

    // Frame in seed range
    in = in + pW * pH * get_global_id(2);
    out = out + w * h * get_global_id(2);

    int v11 = in[(j+1) * pW + i+1];
    int v10 = in[j * pW + i+1];
    int v21 = in[(j+1) * pW + i+2];
    int v01 = in[(j+1) * pW + i+0];
    int v12 = in[(j+2) * pW + i+1];

    int biome = biomeExists(biomes, v11) ? v11 : 0;

    if (v11 == mushroom_fields)
    {
        if (v10 != ocean && v21 != ocean && v01 != ocean && v12 != ocean)
            out[i + j*w] = v11;
        else
            out[i + j*w] = mushroom_field_shore;
    }
    else if (/*biome < 128 &&*/ getBiomeType(biomes, biome) == Jungle)
    {
        if (isBiomeJFTO(biomes, v10) && isBiomeJFTO(biomes, v21) && isBiomeJFTO(biomes, v01) && isBiomeJFTO(biomes, v12))
        {
            if (!isOceanic(v10) && !isOceanic(v21) && !isOceanic(v01) && !isOceanic(v12))
                out[i + j*w] = v11;
            else
                out[i + j*w] = beach;
        }
        else
        {
            out[i + j*w] = jungle_edge;
        }
    }
    else if (v11 != mountains && v11 != wooded_mountains && v11 != mountain_edge)
    {
        if (isBiomeSnowy(biomes, biome))
        {
            replaceOcean(out, i + j*w, v10, v21, v01, v12, v11, snowy_beach);
        }
        else if (v11 != badlands && v11 != wooded_badlands_plateau)
        {
            if (v11 != ocean && v11 != deep_ocean && v11 != river && v11 != swamp)
            {
                if (!isOceanic(v10) && !isOceanic(v21) && !isOceanic(v01) && !isOceanic(v12))
                    out[i + j*w] = v11;
                else
                    out[i + j*w] = beach;
            }
            else
            {
                out[i + j*w] = v11;
            }
        }
        else
        {
            if (!isOceanic(v10) && !isOceanic(v21) && !isOceanic(v01) && !isOceanic(v12))
            {
                if (getBiomeType(biomes, v10) == Mesa && getBiomeType(biomes, v21) == Mesa && getBiomeType(biomes, v01) == Mesa && getBiomeType(biomes, v12) == Mesa)
                    out[i + j*w] = v11;
                else
                    out[i + j*w] = desert;
            }
            else
            {
                out[i + j*w] = v11;
            }
        }
    }
    else
    {
        replaceOcean(out, i + j*w, v10, v21, v01, v12, v11, stone_shore);
    }
}