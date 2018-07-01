/**
 * A quad hut finder with lots of fancy options.
 */

#include "finders.h"
#include "generator.h"
#include "layers.h"

#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <unistd.h>


typedef struct {
    int radius;  /* Search radius in blocks. */
    int hutRadius;
} SearchOptions;


void usage() {
    printf("Usage:\n");
    printf("  multifinder [options]\n");
    printf("    --help\n");
    printf("    --radius=2048  search radius\n");
}


int parseIntArgument(const char *flagName) {
    char *endptr;

    int val = strtol(optarg, &endptr, 10);
    if (errno != 0) {
        printf("An integer argument is required with --%s.", flagName);
        exit(-1);
    }

    return val;
}


SearchOptions parseOptions(int argc, char *argv[]) {
    int c;
    SearchOptions opts = {2048, 4};

    while (1) {
        static struct option longOptions[] = {
            {"radius", required_argument, NULL, 'r'},
            {"help", no_argument, NULL, 'h'},
        };
        int index = 0;
        c = getopt_long(argc, argv, "r:", longOptions, &index);

        if (c == -1)
            break;

        switch (c) {
            case 'r':
                opts.radius = parseIntArgument(longOptions[index].name);
                opts.hutRadius = (int)ceil((double)opts.radius / 512.0);
                break;
            case 'h':
                usage();
                exit(0);
                break;
            default:
                exit(-1);
        }
    }
    return opts;
}


long* getBaseSeeds(long *qhcnt) {
    const char *seedFileName = "./seeds/quadbases_Q1.txt";

    if (access(seedFileName, F_OK)) {
        printf("Seed base file does not exist: Creating new one.\n"
               "This may take a few minutes...\n");
        int threads = 6;
        int quality = 1;
        baseQuadWitchHutSearch(seedFileName, threads, quality);
    }

    return loadSavedSeeds(seedFileName, qhcnt);
}


int main(int argc, char *argv[])
{
    SearchOptions opts = parseOptions(argc, argv);

    // Always initialize the biome list before starting any seed finder or
    // biome generator.
    initBiomes();

    LayerStack g = setupGenerator();

    Layer *lFilterBiome = &g.layers[L_BIOME_256];
    int *biomeCache = allocCache(lFilterBiome, 3, 3);
    long i, j, qhcnt;
    long base, seed;
    const long const *qhcandidates = getBaseSeeds(&qhcnt);


    // Load the positions of the four structures that make up the quad-structure
    // so we can test the biome at these positions.
    Pos qhpos[4];

    // Setup a dummy layer for Layer 19: Biome.
    Layer layerBiomeDummy;
    setupLayer(256, &layerBiomeDummy, NULL, 200, NULL);


    // Search for a swamp at the structure positions
    for(i = 0; i < qhcnt; i++) {
        for (int regPosZ = -opts.hutRadius-1; regPosZ < opts.hutRadius; regPosZ++) {
            for (int regPosX = -opts.hutRadius-1; regPosX < opts.hutRadius; regPosX++) {

                int areaX = (regPosX << 1) + 1;
                int areaZ = (regPosZ << 1) + 1;

                base = moveTemple(qhcandidates[i], regPosX, regPosZ);

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
                    setWorldSeed(&layerBiomeDummy, seed);
                    setChunkSeed(&layerBiomeDummy, areaX+1, areaZ+1);
                    if(mcNextInt(&layerBiomeDummy, 6) == 5)
                        break;
                }
                if(j >= 5) continue;

                qhpos[0] = getWitchHutPos(base, 0+regPosX, 0+regPosZ);
                qhpos[1] = getWitchHutPos(base, 0+regPosX, 1+regPosZ);
                qhpos[2] = getWitchHutPos(base, 1+regPosX, 0+regPosZ);
                qhpos[3] = getWitchHutPos(base, 1+regPosX, 1+regPosZ);


                long hits = 0, swpc;

                for(j = 0; j < 0x10000; j++)
                {
                    seed = base + (j << 48);

                    /** Pre-Generation Checks **/
                    // We can check that at least one swamp could generate in this area
                    // before doing the biome generator checks.
                    setWorldSeed(&layerBiomeDummy, seed);

                    setChunkSeed(&layerBiomeDummy, areaX+1, areaZ+1);
                    if(mcNextInt(&layerBiomeDummy, 6) != 5)
                        continue;

                    // This seed base does not seem to contain many quad huts, so make
                    // a more detailed analysis of the surroundings and see if there is
                    // enough potential for more swamps to justify searching fruther.
                    if(hits == 0 && (j & 0xfff) == 0xfff)
                    {
                        swpc = 0;
                        setChunkSeed(&layerBiomeDummy, areaX, areaZ+1);
                        swpc += mcNextInt(&layerBiomeDummy, 6) == 5;
                        setChunkSeed(&layerBiomeDummy, areaX+1, areaZ);
                        swpc += mcNextInt(&layerBiomeDummy, 6) == 5;
                        setChunkSeed(&layerBiomeDummy, areaX, areaZ);
                        swpc += mcNextInt(&layerBiomeDummy, 6) == 5;

                        if(swpc < (j > 0x1000 ? 2 : 1)) break;
                    }

                    // Dismiss seeds that don't have a swamp near the quad temple.
                    setWorldSeed(lFilterBiome, seed);
                    genArea(lFilterBiome, biomeCache, (regPosX<<1)+2, (regPosZ<<1)+2, 1, 1);

                    if(biomeCache[0] != swampland)
                        continue;

                    applySeed(&g, seed);
                    if(getBiomeAtPos(g, qhpos[0]) != swampland) continue;
                    if(getBiomeAtPos(g, qhpos[1]) != swampland) continue;
                    if(getBiomeAtPos(g, qhpos[2]) != swampland) continue;
                    if(getBiomeAtPos(g, qhpos[3]) != swampland) continue;

                    printf("%ld\n", seed);
                    hits++;
                }
            }
        }
    }

    free(biomeCache);
    freeGenerator(g);

    return 0;
}
