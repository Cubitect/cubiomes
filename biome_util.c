#include "biome_util.h"
#include "layers.h"

const char *biomeGroupNames[NUM_ALL_BIOMES] = {
    "Other",
    "Ocean",
    "Jungle",
    "Mega taiga",
    "Mesa",
    "Mushroom island",
    "Flower forest",
    "Ice spikes",
    "Mesa Bryce",
    "Sunflower plains",
    "Frozen ocean",
    "Cold ocean",
    "Lukewarm ocean",
    "Warm ocean"};

int getBiomeGroup(int biome) {
    // Most biomes are basically everywhere, so we only make an effort to
    // count up the ones that have a good chance of being far away. The list
    // also focuses on biomes with items that don't occur elsewhere (packed ice,
    // teracotta, podzol, jungle saplings, cocoa beans, certain flowers, etc.)

    // A list of bomes that completes the Adventuring Time advancement would
    // also be a cool option.
    switch(biome) {
        case ocean:
        case deepOcean:
            return Ocean_G;
        case jungle:
        case jungleHills:
        case jungleEdge:
        case jungle+128:            // Jungle M
        case jungleEdge+128:        // Jungle Edge M
            return Jungle_G;
        case megaTaiga:
        case megaTaigaHills:
        case megaTaiga+128:         // Mega Spruce Taiga
        case megaTaigaHills+128:    // Mega Spruce Taiga Hills
            return MegaTaiga_G;
        case mesa:
        case mesaPlateau_F:
        case mesaPlateau:
        case mesaPlateau_F+128:     // Mesa Plateau F M
        case mesaPlateau+128:       // Mesa Plateau M
            return Mesa_G;
        case mushroomIsland:
        case mushroomIslandShore:
            return MushroomIsland_G;
        case forest+128:            // Flower Forest
            return FlowerForest_G;
        case icePlains+128:         // Ice Spikes
            return IceSpikes_G;
        case mesa+128:              // Mesa Bryce
            return MesaBryce_G;
        case plains+128:            // Sunflower plains
            return SunflowerPlains_G;
        case frozenOcean:
        case frozenDeepOcean:
            return FrozenOcean_G;
        case coldOcean:
        case coldDeepOcean:
            return ColdOcean_G;
        case lukewarmOcean:
        case lukewarmDeepOcean:
            return LukewarmOcean_G;
        case warmOcean:
        case warmDeepOcean:         // Does not occur in the game, nor reality.
            return WarmOcean_G;
    }
    return Other_G;
}
