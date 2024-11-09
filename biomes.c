#include "biomes.h"
#include <inttypes.h>


int biomeExists(int mc, int id)
{
    if (mc >= MC_1_18)
    {
        if (id >= soul_sand_valley && id <= basalt_deltas)
            return 1;
        if (id >= small_end_islands && id <= end_barrens)
            return 1;

        if (id == pale_garden)
            return mc >= MC_1_21_WD;

        if (id == cherry_grove)
            return mc >= MC_1_20;

        if (id == deep_dark || id == mangrove_swamp)
            return mc >= MC_1_19_2;

        switch (id)
        {
        case ocean:
        case plains:
        case desert:
        case mountains:                 // windswept_hills
        case forest:
        case taiga:
        case swamp:
        case river:
        case nether_wastes:
        case the_end:
        case frozen_ocean:
        case frozen_river:
        case snowy_tundra:              // snowy_plains
        case mushroom_fields:
        case beach:
        case jungle:
        case jungle_edge:               // sparse_jungle
        case deep_ocean:
        case stone_shore:               // stony_shore
        case snowy_beach:
        case birch_forest:
        case dark_forest:
        case snowy_taiga:
        case giant_tree_taiga:          // old_growth_pine_taiga
        case wooded_mountains:          // windswept_forest
        case savanna:
        case savanna_plateau:
        case badlands:
        case wooded_badlands_plateau:   // wooded_badlands
        case warm_ocean:
        case lukewarm_ocean:
        case cold_ocean:
        case deep_warm_ocean:
        case deep_lukewarm_ocean:
        case deep_cold_ocean:
        case deep_frozen_ocean:
        case sunflower_plains:
        case gravelly_mountains:        // windswept_gravelly_hills
        case flower_forest:
        case ice_spikes:
        case tall_birch_forest:         // old_growth_birch_forest
        case giant_spruce_taiga:        // old_growth_spruce_taiga
        case shattered_savanna:         // windswept_savanna
        case eroded_badlands:
        case bamboo_jungle:
        case dripstone_caves:
        case lush_caves:
        case meadow:
        case grove:
        case snowy_slopes:
        case stony_peaks:
        case jagged_peaks:
        case frozen_peaks:
            return 1;
        default:
            return 0;
        }
    }

    if (mc <= MC_B1_7)
    {
        switch(id)
        {
        case plains:
        case desert:
        case forest:
        case taiga:
        case swamp:
        case snowy_tundra:
        case savanna:
        case seasonal_forest:
        case rainforest:
        case shrubland:
        // we treat areas below the sea level as oceans
        case ocean:
        case frozen_ocean:
            return 1;
        default:
            return 0;
        }
    }

    if (mc <= MC_B1_8)
    {
        switch (id)
        {
        case frozen_ocean:
        case frozen_river:
        case snowy_tundra:
        case mushroom_fields:
        case mushroom_field_shore:
        case the_end:
            return 0;
        }
    }
    if (mc <= MC_1_0)
    {
        switch (id)
        {
        case snowy_mountains:
        case beach:
        case desert_hills:
        case wooded_hills:
        case taiga_hills:
        case mountain_edge:
            return 0;
        }
    }

    if (id >= ocean             && id <= mountain_edge)     return 1;
    if (id >= jungle            && id <= jungle_hills)      return mc >= MC_1_2;
    if (id >= jungle_edge       && id <= badlands_plateau)  return mc >= MC_1_7;
    if (id >= small_end_islands && id <= end_barrens)       return mc >= MC_1_9;
    if (id >= warm_ocean        && id <= deep_frozen_ocean) return mc >= MC_1_13;

    switch (id)
    {
    case the_void:
        return mc >= MC_1_9;
    case sunflower_plains:
    case desert_lakes:
    case gravelly_mountains:
    case flower_forest:
    case taiga_mountains:
    case swamp_hills:
    case ice_spikes:
    case modified_jungle:
    case modified_jungle_edge:
    case tall_birch_forest:
    case tall_birch_hills:
    case dark_forest_hills:
    case snowy_taiga_mountains:
    case giant_spruce_taiga:
    case giant_spruce_taiga_hills:
    case modified_gravelly_mountains:
    case shattered_savanna:
    case shattered_savanna_plateau:
    case eroded_badlands:
    case modified_wooded_badlands_plateau:
    case modified_badlands_plateau:
        return mc >= MC_1_7;
    case bamboo_jungle:
    case bamboo_jungle_hills:
        return mc >= MC_1_14;
    case soul_sand_valley:
    case crimson_forest:
    case warped_forest:
    case basalt_deltas:
        return mc >= MC_1_16_1;
    case dripstone_caves:
    case lush_caves:
        return mc >= MC_1_17;
    default:
        return 0;
    }
}

int isOverworld(int mc, int id)
{
    if (!biomeExists(mc, id))
        return 0;

    if (id >= small_end_islands && id <= end_barrens) return 0;
    if (id >= soul_sand_valley && id <= basalt_deltas) return 0;

    switch (id)
    {
    case nether_wastes:
    case the_end:
        return 0;
    case frozen_ocean:
        return mc <= MC_1_6 || mc >= MC_1_13;
    case mountain_edge:
        return mc <= MC_1_6;
    case deep_warm_ocean:
    case the_void:
        return 0;
    case tall_birch_forest:
        return mc <= MC_1_8 || mc >= MC_1_11;
    case dripstone_caves:
    case lush_caves:
        return mc >= MC_1_18;
    }
    return 1;
}

int getDimension(int id)
{
    if (id >= small_end_islands && id <= end_barrens) return DIM_END;
    if (id >= soul_sand_valley && id <= basalt_deltas) return DIM_NETHER;
    if (id == the_end) return DIM_END;
    if (id == nether_wastes) return DIM_NETHER;
    return DIM_OVERWORLD;
}

int getMutated(int mc, int id)
{
    switch (id)
    {
    case plains:                    return sunflower_plains;
    case desert:                    return desert_lakes;
    case mountains:                 return gravelly_mountains;
    case forest:                    return flower_forest;
    case taiga:                     return taiga_mountains;
    case swamp:                     return swamp_hills;
    case snowy_tundra:              return ice_spikes;
    case jungle:                    return modified_jungle;
    case jungle_edge:               return modified_jungle_edge;
    // emulate MC-98995
    case birch_forest:
        return (mc >= MC_1_9 && mc <= MC_1_10) ? tall_birch_hills : tall_birch_forest;
    case birch_forest_hills:
        return (mc >= MC_1_9 && mc <= MC_1_10) ? none : tall_birch_hills;
    case dark_forest:               return dark_forest_hills;
    case snowy_taiga:               return snowy_taiga_mountains;
    case giant_tree_taiga:          return giant_spruce_taiga;
    case giant_tree_taiga_hills:    return giant_spruce_taiga_hills;
    case wooded_mountains:          return modified_gravelly_mountains;
    case savanna:                   return shattered_savanna;
    case savanna_plateau:           return shattered_savanna_plateau;
    case badlands:                  return eroded_badlands;
    case wooded_badlands_plateau:   return modified_wooded_badlands_plateau;
    case badlands_plateau:          return modified_badlands_plateau;
    default:
        return none;
    }
}

int getCategory(int mc, int id)
{
    switch (id)
    {
    case beach:
    case snowy_beach:
        return beach;

    case desert:
    case desert_hills:
    case desert_lakes:
        return desert;

    case mountains:
    case mountain_edge:
    case wooded_mountains:
    case gravelly_mountains:
    case modified_gravelly_mountains:
        return mountains;

    case forest:
    case wooded_hills:
    case birch_forest:
    case birch_forest_hills:
    case dark_forest:
    case flower_forest:
    case tall_birch_forest:
    case tall_birch_hills:
    case dark_forest_hills:
        return forest;

    case snowy_tundra:
    case snowy_mountains:
    case ice_spikes:
        return snowy_tundra;

    case jungle:
    case jungle_hills:
    case jungle_edge:
    case modified_jungle:
    case modified_jungle_edge:
    case bamboo_jungle:
    case bamboo_jungle_hills:
        return jungle;

    case badlands:
    case eroded_badlands:
    case modified_wooded_badlands_plateau:
    case modified_badlands_plateau:
        return mesa;

    case wooded_badlands_plateau:
    case badlands_plateau:
        return mc <= MC_1_15 ? mesa : badlands_plateau;

    case mushroom_fields:
    case mushroom_field_shore:
        return mushroom_fields;

    case stone_shore:
        return stone_shore;

    case ocean:
    case frozen_ocean:
    case deep_ocean:
    case warm_ocean:
    case lukewarm_ocean:
    case cold_ocean:
    case deep_warm_ocean:
    case deep_lukewarm_ocean:
    case deep_cold_ocean:
    case deep_frozen_ocean:
        return ocean;

    case plains:
    case sunflower_plains:
        return plains;

    case river:
    case frozen_river:
        return river;

    case savanna:
    case savanna_plateau:
    case shattered_savanna:
    case shattered_savanna_plateau:
        return savanna;

    case swamp:
    case swamp_hills:
        return swamp;

    case taiga:
    case taiga_hills:
    case snowy_taiga:
    case snowy_taiga_hills:
    case giant_tree_taiga:
    case giant_tree_taiga_hills:
    case taiga_mountains:
    case snowy_taiga_mountains:
    case giant_spruce_taiga:
    case giant_spruce_taiga_hills:
        return taiga;

    case nether_wastes:
    case soul_sand_valley:
    case crimson_forest:
    case warped_forest:
    case basalt_deltas:
        return nether_wastes;

    default:
        return none;
    }
}

int areSimilar(int mc, int id1, int id2)
{
    if (id1 == id2) return 1;

    if (mc <= MC_1_15)
    {
        if (id1 == wooded_badlands_plateau || id1 == badlands_plateau)
            return id2 == wooded_badlands_plateau || id2 == badlands_plateau;
    }

    return getCategory(mc, id1) == getCategory(mc, id2);
}

int isMesa(int id)
{
    switch (id)
    {
    case badlands:
    case eroded_badlands:
    case modified_wooded_badlands_plateau:
    case modified_badlands_plateau:
    case wooded_badlands_plateau:
    case badlands_plateau:
        return 1;
    default:
        return 0;
    }
}

int isShallowOcean(int id)
{
    const uint64_t shallow_bits =
            (1ULL << ocean) |
            (1ULL << frozen_ocean) |
            (1ULL << warm_ocean) |
            (1ULL << lukewarm_ocean) |
            (1ULL << cold_ocean);
    return (uint32_t) id < 64 && ((1ULL << id) & shallow_bits);
}

int isDeepOcean(int id)
{
    const uint64_t deep_bits =
            (1ULL << deep_ocean) |
            (1ULL << deep_warm_ocean) |
            (1ULL << deep_lukewarm_ocean) |
            (1ULL << deep_cold_ocean) |
            (1ULL << deep_frozen_ocean);
    return (uint32_t) id < 64 && ((1ULL << id) & deep_bits);
}

int isOceanic(int id)
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
    return (uint32_t) id < 64 && ((1ULL << id) & ocean_bits);
}

int isSnowy(int id)
{
    switch (id)
    {
    case frozen_ocean:
    case frozen_river:
    case snowy_tundra:
    case snowy_mountains:
    case snowy_beach:
    case snowy_taiga:
    case snowy_taiga_hills:
    case ice_spikes:
    case snowy_taiga_mountains:
        return 1;
    default:
        return 0;
    }
}


