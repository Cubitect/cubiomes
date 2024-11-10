#ifndef BIOMES_H_
#define BIOMES_H_

/* Minecraft versions */
enum MCVersion
{   // MC_1_X refers to the latest patch of the respective 1.X release.
    // NOTE: Development effort focuses on just the newest patch for each major
    // release. Minor releases and major versions <= 1.0 are experimental.
    MC_UNDEF,
    MC_B1_7,
    MC_B1_8,
    MC_1_0_0,  MC_1_0  = MC_1_0_0,
    MC_1_1_0,  MC_1_1  = MC_1_1_0,
    MC_1_2_5,  MC_1_2  = MC_1_2_5,
    MC_1_3_2,  MC_1_3  = MC_1_3_2,
    MC_1_4_7,  MC_1_4  = MC_1_4_7,
    MC_1_5_2,  MC_1_5  = MC_1_5_2,
    MC_1_6_4,  MC_1_6  = MC_1_6_4,
    MC_1_7_10, MC_1_7  = MC_1_7_10,
    MC_1_8_9,  MC_1_8  = MC_1_8_9,
    MC_1_9_4,  MC_1_9  = MC_1_9_4,
    MC_1_10_2, MC_1_10 = MC_1_10_2,
    MC_1_11_2, MC_1_11 = MC_1_11_2,
    MC_1_12_2, MC_1_12 = MC_1_12_2,
    MC_1_13_2, MC_1_13 = MC_1_13_2,
    MC_1_14_4, MC_1_14 = MC_1_14_4,
    MC_1_15_2, MC_1_15 = MC_1_15_2,
    MC_1_16_1,
    MC_1_16_5, MC_1_16 = MC_1_16_5,
    MC_1_17_1, MC_1_17 = MC_1_17_1,
    MC_1_18_2, MC_1_18 = MC_1_18_2,
    MC_1_19_2,
    MC_1_19_4, MC_1_19 = MC_1_19_4,
    MC_1_20_6, MC_1_20 = MC_1_20_6,
    MC_1_21_1,
    MC_1_21_3,
    MC_1_21_WD, // Winter Drop, version TBA
    MC_1_21 = MC_1_21_WD,
    MC_NEWEST = MC_1_21,
};

enum Dimension
{
    DIM_NETHER      =   -1,
    DIM_OVERWORLD   =    0,
    DIM_END         =   +1,
    DIM_UNDEF       = 1000,
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
    // Alpha 1.2 - Beta 1.7
    seasonal_forest,
    rainforest,
    shrubland,


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
    // 1.19
    deep_dark                       = 183,
    mangrove_swamp                  = 184,
    // 1.20
    cherry_grove                    = 185,
    // 1.21 Winter Drop
    pale_garden                     = 186,
};


#ifdef __cplusplus
extern "C"
{
#endif

//==============================================================================
// BiomeID Helper Functions
//==============================================================================

int biomeExists(int mc, int id);
int isOverworld(int mc, int id); // false for biomes that don't generate
int getDimension(int id);
int getMutated(int mc, int id);
int getCategory(int mc, int id);
int areSimilar(int mc, int id1, int id2);
int isMesa(int id);
int isShallowOcean(int id);
int isDeepOcean(int id);
int isOceanic(int id);
int isSnowy(int id);

#ifdef __cplusplus
}
#endif

#endif /* BIOMES_H_ */

