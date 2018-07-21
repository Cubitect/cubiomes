#ifndef MORE_FINDERS_H_
#define MORE_FINDERS_H_

#include "layers.h"

enum BiomeGroup {
    Other_G = 0, Ocean_G, Jungle_G, MegaTaiga_G, Mesa_G, MushroomIsland_G,
    FlowerForest_G, IceSpikes_G, MesaBryce_G, SunflowerPlains_G,
    FrozenOcean_G, ColdOcean_G, LukewarmOcean_G, WarmOcean_G};

#define NUM_ALL_BIOMES 14

int getBiomeGroup(int biome);
const char *biomeGroupNames[NUM_ALL_BIOMES];
const char *biomeNames[256];

void initBiomeColours(unsigned char biomeColours[256][3]);
void initBiomeTypeColours(unsigned char biomeColours[256][3]);

#endif
