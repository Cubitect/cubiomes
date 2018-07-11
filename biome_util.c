#include <string.h>

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


/* Global biome colour table. */
void setBiomeColour(unsigned char biomeColour[256][3], int biome,
        unsigned char r, unsigned char g, unsigned char b)
{
    biomeColour[biome][0] = r;
    biomeColour[biome][1] = g;
    biomeColour[biome][2] = b;
}


void initBiomeColours(unsigned char biomeColours[256][3]) {
    // This colouring scheme is taken from the AMIDST program:
    // https://github.com/toolbox4minecraft/amidst
    // https://sourceforge.net/projects/amidst.mirror/

    memset(biomeColours, 0, 256*3);

    setBiomeColour(biomeColours, ocean, 0, 0, 112);
    setBiomeColour(biomeColours, plains,141, 179, 96);
    setBiomeColour(biomeColours, desert, 250, 148, 24);
    setBiomeColour(biomeColours, extremeHills, 96, 96, 96);
    setBiomeColour(biomeColours, forest, 5, 102, 33);
    setBiomeColour(biomeColours, taiga, 11, 102, 89);
    setBiomeColour(biomeColours, swampland, 7, 249, 178);
    setBiomeColour(biomeColours, river, 0, 0, 255);
    setBiomeColour(biomeColours, hell, 255, 0, 0);
    setBiomeColour(biomeColours, sky, 128, 128, 255);
    setBiomeColour(biomeColours, frozenOcean, 112, 112, 214);
    setBiomeColour(biomeColours, frozenRiver, 160, 160, 255);
    setBiomeColour(biomeColours, icePlains, 255, 255, 255);
    setBiomeColour(biomeColours, iceMountains, 160, 160, 160);
    setBiomeColour(biomeColours, mushroomIsland, 255, 0, 255);
    setBiomeColour(biomeColours, mushroomIslandShore, 160, 0, 255);
    setBiomeColour(biomeColours, beach, 250, 222, 85);
    setBiomeColour(biomeColours, desertHills, 210, 95, 18);
    setBiomeColour(biomeColours, forestHills, 34, 85, 28);
    setBiomeColour(biomeColours, taigaHills, 22, 57, 51);
    setBiomeColour(biomeColours, extremeHillsEdge, 114, 120, 154);
    setBiomeColour(biomeColours, jungle, 83, 123, 9);
    setBiomeColour(biomeColours, jungleHills, 44, 66, 5);
    setBiomeColour(biomeColours, jungleEdge, 98, 139, 23);
    setBiomeColour(biomeColours, deepOcean, 0, 0, 48);
    setBiomeColour(biomeColours, stoneBeach, 162, 162, 132);
    setBiomeColour(biomeColours, coldBeach, 250, 240, 192);
    setBiomeColour(biomeColours, birchForest, 48, 116, 68);
    setBiomeColour(biomeColours, birchForestHills, 31, 95, 50);
    setBiomeColour(biomeColours, roofedForest, 64, 81, 26);
    setBiomeColour(biomeColours, coldTaiga, 49, 85, 74);
    setBiomeColour(biomeColours, coldTaigaHills, 36, 63, 54);
    setBiomeColour(biomeColours, megaTaiga, 89, 102, 81);
    setBiomeColour(biomeColours, megaTaigaHills, 69, 79, 62);
    setBiomeColour(biomeColours, extremeHillsPlus, 80, 112, 80);
    setBiomeColour(biomeColours, savanna, 189, 178, 95);
    setBiomeColour(biomeColours, savannaPlateau, 167, 157, 100);
    setBiomeColour(biomeColours, mesa, 217, 69, 21);
    setBiomeColour(biomeColours, mesaPlateau_F, 176, 151, 101);
    setBiomeColour(biomeColours, mesaPlateau, 202, 140, 101);

    setBiomeColour(biomeColours, warmOcean, 0, 0, 172);
    setBiomeColour(biomeColours, lukewarmOcean, 0, 0, 144);
    setBiomeColour(biomeColours, coldOcean, 32, 32, 112);
    setBiomeColour(biomeColours, warmDeepOcean, 0, 0, 80);
    setBiomeColour(biomeColours, lukewarmDeepOcean, 0, 0, 64);
    setBiomeColour(biomeColours, coldDeepOcean, 32, 32, 56);
    setBiomeColour(biomeColours, frozenDeepOcean, 64, 64, 144);

    setBiomeColour(biomeColours, ocean+128, 0, 0, 112);
    setBiomeColour(biomeColours, plains+128, 141, 179, 96);
    setBiomeColour(biomeColours, desert+128, 250, 148, 24);
    setBiomeColour(biomeColours, extremeHills+128, 96, 96, 96);
    setBiomeColour(biomeColours, forest+128, 5, 102, 33);
    setBiomeColour(biomeColours, taiga+128, 11, 102, 89);
    setBiomeColour(biomeColours, swampland+128, 7, 249, 178);
    setBiomeColour(biomeColours, river+128, 0, 0, 255);
    setBiomeColour(biomeColours, hell+128, 255, 0, 0);
    setBiomeColour(biomeColours, sky+128, 128, 128, 255);
    setBiomeColour(biomeColours, frozenOcean+128, 144, 144, 160);
    setBiomeColour(biomeColours, frozenRiver+128, 160, 160, 255);
    setBiomeColour(biomeColours, icePlains+128, 140, 180, 180);
    setBiomeColour(biomeColours, iceMountains+128, 160, 160, 160);
    setBiomeColour(biomeColours, mushroomIsland+128, 255, 0, 255);
    setBiomeColour(biomeColours, mushroomIslandShore+128, 160, 0, 255);
    setBiomeColour(biomeColours, beach+128, 250, 222, 85);
    setBiomeColour(biomeColours, desertHills+128, 210, 95, 18);
    setBiomeColour(biomeColours, forestHills+128, 34, 85, 28);
    setBiomeColour(biomeColours, taigaHills+128, 22, 57, 51);
    setBiomeColour(biomeColours, extremeHillsEdge+128, 114, 120, 154);
    setBiomeColour(biomeColours, jungle+128, 83, 123, 9);
    setBiomeColour(biomeColours, jungleHills+128, 44, 66, 5);
    setBiomeColour(biomeColours, jungleEdge+128, 98, 139, 23);
    setBiomeColour(biomeColours, deepOcean+128, 0, 0, 48);
    setBiomeColour(biomeColours, stoneBeach+128, 162, 162, 132);
    setBiomeColour(biomeColours, coldBeach+128, 250, 240, 192);
    setBiomeColour(biomeColours, birchForest+128, 48, 116, 68);
    setBiomeColour(biomeColours, birchForestHills+128, 31, 95, 50);
    setBiomeColour(biomeColours, roofedForest+128, 64, 81, 26);
    setBiomeColour(biomeColours, coldTaiga+128, 49, 85, 74);
    setBiomeColour(biomeColours, coldTaigaHills+128, 36, 63, 54);
    setBiomeColour(biomeColours, megaTaiga+128, 89, 102, 81);
    setBiomeColour(biomeColours, megaTaigaHills+128, 69, 79, 62);
    setBiomeColour(biomeColours, extremeHillsPlus+128, 80, 112, 80);
    setBiomeColour(biomeColours, savanna+128, 189, 178, 95);
    setBiomeColour(biomeColours, savannaPlateau+128, 167, 157, 100);
    setBiomeColour(biomeColours, mesa+128, 217, 69, 21);
    setBiomeColour(biomeColours, mesaPlateau_F+128, 176, 151, 101);
    setBiomeColour(biomeColours, mesaPlateau+128, 202, 140, 101);
}


void initBiomeTypeColours(unsigned char biomeColours[256][3]) {
    memset(biomeColours, 0, 256*3);

    setBiomeColour(biomeColours, Oceanic,  0x00, 0x00, 0xa0);
    setBiomeColour(biomeColours, Warm,     0xff, 0xc0, 0x00);
    setBiomeColour(biomeColours, Lush,     0x00, 0xa0, 0x00);
    setBiomeColour(biomeColours, Cold,     0x60, 0x60, 0x60);
    setBiomeColour(biomeColours, Freezing, 0xff, 0xff, 0xff);
}
