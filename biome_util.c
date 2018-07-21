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

const char *biomeNames[256] = {
    "Ocean", "Plains", "Desert", "Extreme Hills", "Forest", "Taiga", "Swampland", "River", "Hell", "Sky", // 0-9
    "Frozen Ocean", "Frozen River", "Ice Plains", "Ice Mountains", "Mushroom Island", "Mushroom Island Shore", "Beach", "Desert Hills", "Forest Hills", "Taiga Hills",  // 10-19
    "Extreme Hills Edge", "Jungle", "Jungle Hills", "Jungle Edge", "Deep Ocean", "Stone Beach", "Cold Beach", "Birch Forest", "Birch Forest Hills", "Roofed Forest", // 20-29
    "Cold Taiga", "Cold Taiga Hills", "Mega Taiga", "Mega Taiga Hills", "Extreme Hills+", "Savanna", "Savanna Plateau", "Mesa", "Mesa Plateau F", "Mesa Plateau", // 30-39
    // 1.13
    "Sky Island Low", "Sky Island Medium", "Sky Island High", "Sky Island Barren", "Warm Ocean", "Lukewarm Ocean", "Cold Ocean", "Warm Deep Ocean", "Lukewarm Deep Ocean", "Cold Deep Ocean", // 40-49
    "Frozen Deep Ocean",  // 50
    "UNKNOWN #51", "UNKNOWN #52", "UNKNOWN #53", "UNKNOWN #54", "UNKNOWN #55",
    "UNKNOWN #56", "UNKNOWN #57", "UNKNOWN #58", "UNKNOWN #59", "UNKNOWN #60", "UNKNOWN #61", "UNKNOWN #62", "UNKNOWN #63",
    "UNKNOWN #64", "UNKNOWN #65", "UNKNOWN #66", "UNKNOWN #67", "UNKNOWN #68", "UNKNOWN #69", "UNKNOWN #70", "UNKNOWN #71",
    "UNKNOWN #72", "UNKNOWN #73", "UNKNOWN #74", "UNKNOWN #75", "UNKNOWN #76", "UNKNOWN #77", "UNKNOWN #78", "UNKNOWN #79",
    "UNKNOWN #80", "UNKNOWN #81", "UNKNOWN #82", "UNKNOWN #83", "UNKNOWN #84", "UNKNOWN #85", "UNKNOWN #86", "UNKNOWN #87",
    "UNKNOWN #88", "UNKNOWN #89", "UNKNOWN #90", "UNKNOWN #91", "UNKNOWN #92", "UNKNOWN #93", "UNKNOWN #94", "UNKNOWN #95",
    "UNKNOWN #96", "UNKNOWN #97", "UNKNOWN #98", "UNKNOWN #99", "UNKNOWN #100", "UNKNOWN #101", "UNKNOWN #102", "UNKNOWN #103",
    "UNKNOWN #104", "UNKNOWN #105", "UNKNOWN #106", "UNKNOWN #107", "UNKNOWN #108", "UNKNOWN #109", "UNKNOWN #110", "UNKNOWN #111",
    "UNKNOWN #112", "UNKNOWN #113", "UNKNOWN #114", "UNKNOWN #115", "UNKNOWN #116", "UNKNOWN #117", "UNKNOWN #118", "UNKNOWN #119",
    "UNKNOWN #120", "UNKNOWN #121", "UNKNOWN #122", "UNKNOWN #123", "UNKNOWN #124", "UNKNOWN #125", "UNKNOWN #126", "UNKNOWN #127",

    // NOTE: Many of these M biomes don't exist.
    "Ocean M", "Sunflower Plains", "Desert M", "Extreme Hills M", "Flower Forest", "Taiga M", "Swampland M", "River M", "Hell M", "Sky M", // 0-9
    "Frozen Ocean M", "Frozen River M", "Ice Plains Spikes", "Ice Mountains M", "Mushroom Island M", "Mushroom Island Shore M", "Beach M", "Desert Hills M", "Forest Hills M", "Taiga Hills M",  // 10-19
    "Extreme Hills Edge M", "Jungle M", "Jungle Hills M", "Jungle Edge M", "Deep Ocean M", "Stone Beach M", "Cold Beach M", "Birch Forest M", "Birch Forest Hills M", "Roofed Forest M", // 20-29
    "Cold Taiga M", "Cold Taiga Hills M", "Mega Spruce Taiga", "Mega Spruce Taiga Hills", "Extreme Hills+ M", "Savanna M", "Savanna Plateau M", "Mesa Bryce", "Mesa Plateau F M", "Mesa Plateau M", // 30-39
    // 1.13
    "Sky Island Low M", "Sky Island Medium M", "Sky Island High M", "Sky Island Barren M", "Warm Ocean M", "Lukewarm Ocean M", "Cold Ocean M", "Warm Deep Ocean M", "Lukewarm Deep Ocean M", "Cold Deep Ocean M", // 40-49
    "Frozen Deep Ocean M",  // 50
    "UNKNOWN #51 M", "UNKNOWN #52 M", "UNKNOWN #53 M", "UNKNOWN #54 M", "UNKNOWN #55 M",
    "UNKNOWN #56 M", "UNKNOWN #57 M", "UNKNOWN #58 M", "UNKNOWN #59 M", "UNKNOWN #60 M", "UNKNOWN #61 M", "UNKNOWN #62 M", "UNKNOWN #63 M",
    "UNKNOWN #64 M", "UNKNOWN #65 M", "UNKNOWN #66 M", "UNKNOWN #67 M", "UNKNOWN #68 M", "UNKNOWN #69 M", "UNKNOWN #70 M", "UNKNOWN #71 M",
    "UNKNOWN #72 M", "UNKNOWN #73 M", "UNKNOWN #74 M", "UNKNOWN #75 M", "UNKNOWN #76 M", "UNKNOWN #77 M", "UNKNOWN #78 M", "UNKNOWN #79 M",
    "UNKNOWN #80 M", "UNKNOWN #81 M", "UNKNOWN #82 M", "UNKNOWN #83 M", "UNKNOWN #84 M", "UNKNOWN #85 M", "UNKNOWN #86 M", "UNKNOWN #87 M",
    "UNKNOWN #88 M", "UNKNOWN #89 M", "UNKNOWN #90 M", "UNKNOWN #91 M", "UNKNOWN #92 M", "UNKNOWN #93 M", "UNKNOWN #94 M", "UNKNOWN #95 M",
    "UNKNOWN #96 M", "UNKNOWN #97 M", "UNKNOWN #98 M", "UNKNOWN #99 M", "UNKNOWN #100 M", "UNKNOWN #101 M", "UNKNOWN #102 M", "UNKNOWN #103 M",
    "UNKNOWN #104 M", "UNKNOWN #105 M", "UNKNOWN #106 M", "UNKNOWN #107 M", "UNKNOWN #108 M", "UNKNOWN #109 M", "UNKNOWN #110 M", "UNKNOWN #111 M",
    "UNKNOWN #112 M", "UNKNOWN #113 M", "UNKNOWN #114 M", "UNKNOWN #115 M", "UNKNOWN #116 M", "UNKNOWN #117 M", "UNKNOWN #118 M", "UNKNOWN #119 M",
    "UNKNOWN #120 M", "UNKNOWN #121 M", "UNKNOWN #122 M", "UNKNOWN #123 M", "UNKNOWN #124 M", "UNKNOWN #125 M", "UNKNOWN #126 M", "UNKNOWN #127 M",
};


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
