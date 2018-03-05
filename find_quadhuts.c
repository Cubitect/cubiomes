/**
 * This is an example program that demonstrates how to find seeds with a
 * quad-witch-hut located around the specified region (512x512 area).
 *
 * It uses some optimisations that cause it miss a small number of seeds, in
 * exchange for a major speed upgrade. (~99% accuracy, ~1200% speed)
 */

#include "finders.h"
#include "generator.h"
#include "layers.h"

#include <unistd.h>


int main(int argc, char *argv[])
{
    // Always initialize the biome list before starting any seed finder or
    // biome generator.
    initBiomes();

    const char *seedFileName = "quadbases_Q1.txt";

    if(access(seedFileName, F_OK))
    {
        printf("Seed base file does not exist: Creating new one.\n"
               "This may take a few minutes...\n");
        int threads = 6;
        int quality = 1;
        baseQuadTempleSearch(seedFileName, threads, quality);
    }

    long i, j, qhcnt;
    long base, seed;
    long *qhcandidates = loadSavedSeeds(seedFileName, &qhcnt);

    Generator g = setupGenerator();
    Generator gFilterBiome = setupGenerator();

    gFilterBiome.topLayerIndex = 19; // Biome Layer with scale 1:256
    int *biomeCache = allocCache(&gFilterBiome, 3, 3);


    // Load the positions of the four structures that make up the quad-structure
    // so we can test the biome at these positions.
    Pos qhpos[4];


    // Translate the positions to the desired regions.
    int regPosX;
    int regPosZ;

    if(argc > 2)
    {
        if(sscanf(argv[1], "%d", &regPosX) != 1) regPosX = 0;
        if(sscanf(argv[2], "%d", &regPosZ) != 1) regPosZ = 0;
    }

    regPosX -= 1;
    regPosZ -= 1;

    // Setup a dummy layer for Layer 19: Biome.
    Layer layerBiome;
    setupLayer(&layerBiome, NULL, 200, NULL);


    int areaX = (regPosX << 1) + 1;
    int areaZ = (regPosZ << 1) + 1;


    // Search for a swamp at the structure positions
    for(i = 0; i < qhcnt; i++)
    {
        base = moveTemple(qhcandidates[i], regPosX, regPosZ);

        qhpos[0] = getTemplePos(base, 0+regPosX, 0+regPosZ);
        qhpos[1] = getTemplePos(base, 0+regPosX, 1+regPosZ);
        qhpos[2] = getTemplePos(base, 1+regPosX, 0+regPosZ);
        qhpos[3] = getTemplePos(base, 1+regPosX, 1+regPosZ);

        /*
        for(j = 0; j < 4; j++)
        {
            printf("(%d,%d) ", qhpos[j].x, qhpos[j].z);
        }
        printf("\n");
        //*/

        // This little magic code checks if there is a meaningful chance for
        // this seed base to generate swamps in the area.
        // The idea is that the conversion from Lush temperature to swampland is
        // independent of surroundings, so we can test the conversion
        // beforehand. Furthermore biomes tend to leak into the negative
        // coordinates because of the Zoom layers, so the majority of hits will
        // occur when SouthEast corner (at a 1:256 scale) of the quad-hut has a
        // swampland. (This assumption misses about 1 in 500 quad-hut seeds.)
        // Finally, here we also exploit that the minecraft random number
        // generator is quite bad, such that for the "mcNextRand() mod 6" check
        // it has a period pattern of ~3 on the high seed-bits.
        for(j = 0; j < 5; j++)
        {
            seed = base + ((j+0x53) << 48);
            setWorldSeed(&layerBiome, seed);
            setChunkSeed(&layerBiome, areaX+1, areaZ+1);
            if(mcNextInt(&layerBiome, 6) == 5)
                break;
        }
        if(j >= 5) continue;


        long hits = 0, swpc;

        for(j = 0; j < 0x10000; j++)
        {
            seed = base + (j << 48);

            /** Pre-Generation Checks **/
            // We can check that at least one swamp could generate in this area
            // before doing the biome generator checks.
            setWorldSeed(&layerBiome, seed);

            setChunkSeed(&layerBiome, areaX+1, areaZ+1);
            if(mcNextInt(&layerBiome, 6) != 5)
                continue;

            // This seed base does not seem to contain many quad huts, so make
            // a more detailed analysis of the surroundings and see if there is
            // enough potential for more swamps to justify searching fruther.
            if(hits == 0 && (j & 0xfff) == 0xfff)
            {
                swpc = 0;
                setChunkSeed(&layerBiome, areaX, areaZ+1);
                swpc += mcNextInt(&layerBiome, 6) == 5;
                setChunkSeed(&layerBiome, areaX+1, areaZ);
                swpc += mcNextInt(&layerBiome, 6) == 5;
                setChunkSeed(&layerBiome, areaX, areaZ);
                swpc += mcNextInt(&layerBiome, 6) == 5;

                if(swpc < (j > 0x1000 ? 2 : 1)) break;
            }

            // Dismiss seeds that don't have a swamp near the quad temple.
            applySeed(&gFilterBiome, seed);
            genArea(&gFilterBiome, biomeCache, (regPosX<<1)+2, (regPosZ<<1)+2, 1, 1);

            if(biomeCache[0] != swampland)
                continue;

            applySeed(&g, seed);
            if(getBiomeAtPos(&g, qhpos[0]) != swampland) continue;
            if(getBiomeAtPos(&g, qhpos[1]) != swampland) continue;
            if(getBiomeAtPos(&g, qhpos[2]) != swampland) continue;
            if(getBiomeAtPos(&g, qhpos[3]) != swampland) continue;

            printf("%ld\n", seed);
            hits++;
        }
    }

    free(biomeCache);
    freeGenerator(&g);
    freeGenerator(&gFilterBiome);

    return 0;
}
