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
    long startSeed;
    long endSeed;
} SearchOptions;


void usage() {
    printf("Usage:\n");
    printf("  multifinder [options]\n");
    printf("    --help\n");
    printf("    --radius=<integer>\n");
    printf("    --start_seed=<integer>\n");
    printf("    --end_seed=<integer>\n");
}


long parseHumanArgument(const char *flagName) {
    char *endptr;

    int len = strlen(optarg);
    if (len < 1) {
        printf("An integer argument is required wit --%s", flagName);
        exit(-1);
    }

    long mult = 1;
    switch (optarg[len-1]) {
        case 'K': mult = 1024L; break;
        case 'M': mult = 1024L*1024L; break;
        case 'B': mult = 1024L*1024L*1024L; break;
        case 'G': mult = 1024L*1024L*1024L; break;
        case 'T': mult = 1024L*1024L*1024L*1024L; break;
    }

    if (mult != 1)
        optarg[len-1] = 0;
    long val = strtol(optarg, &endptr, 10);
    if (errno != 0) {
        printf("An integer argument is required with --%s.", flagName);
        exit(-1);
    }

    return val*mult;
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
    SearchOptions opts = {2048, 4, 0, 1L<<48};

    while (1) {
        static struct option longOptions[] = {
            {"radius",     required_argument, NULL, 'r'},
            {"start_seed", required_argument, NULL, 's'},
            {"end_seed",   required_argument, NULL, 'e'},
            {"help",       no_argument,       NULL, 'h'},
        };
        int index = 0;
        c = getopt_long(argc, argv, "r:s:e:h", longOptions, &index);

        if (c == -1)
            break;

        switch (c) {
            case 'r':
                opts.radius = parseIntArgument(longOptions[index].name);
                opts.hutRadius = (int)ceil((double)opts.radius / 512.0);
                break;
            case 's':
                opts.startSeed = parseHumanArgument(longOptions[index].name);
                break;
            case 'e':
                opts.endSeed = parseHumanArgument(longOptions[index].name);
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

    printf("Searching base seeds %ld-%ld, radius %d...\n",
            opts.startSeed, opts.endSeed, opts.radius);

    // Always initialize the biome list before starting any seed finder or
    // biome generator.
    initBiomes();

    LayerStack g = setupGenerator();

    Layer *lFilterBiome = &g.layers[L_BIOME_256];
    int *biomeCache = allocCache(lFilterBiome, 3, 3);
    long j, qhcnt, base, seed;
    const long const *qhcandidates = getBaseSeeds(&qhcnt);


    // Load the positions of the four structures that make up the quad-structure
    // so we can test the biome at these positions.
    Pos qhpos[4];

    // Setup a dummy layer for Layer 19: Biome.
    Layer layerBiomeDummy;
    setupLayer(256, &layerBiomeDummy, NULL, 200, NULL);

    // Search for a swamp at the structure positions
    int startIndex = 0;
    while (qhcandidates[startIndex] < opts.startSeed && startIndex < qhcnt) {
        startIndex++;
    }
    for(int i=startIndex; i < qhcnt && qhcandidates[i] < opts.endSeed; i++) {
        for (int rZ = -opts.hutRadius-1; rZ < opts.hutRadius; rZ++) {
            for (int rX = -opts.hutRadius-1; rX < opts.hutRadius; rX++) {

                // rZ, rX is the hut region in the upper left of the potential
                // quad hut. Hut regions are 32 chunks/512 blocks. The biome
                // generation layers we're looking at are 1:256 zoom. So
                // the biome area is 2* the hut region. Also, we want the area
                // at the center of the quad-hut regions, so +1.
                int areaX = (rX << 1) + 1;
                int areaZ = (rZ << 1) + 1;

                base = moveTemple(qhcandidates[i], rX, rZ);

                // This little magic code checks if there is a meaningful chance
                // for this seed base to generate swamps in the area.

                // The idea is that the conversion from Lush temperature to
                // swampland is independent of surroundings, so we can test the
                // conversion beforehand. Furthermore biomes tend to leak into
                // the negative coordinates because of the Zoom layers, so the
                // majority of hits will occur when SouthEast corner (at a 1:256
                // scale) of the quad-hut has a swampland. (This assumption
                // misses about 1 in 500 quad-hut seeds.) Finally, here we also
                // exploit that the minecraft random number generator is quite
                // bad, such that for the "mcNextRand() mod 6" check it has a
                // period pattern of ~3 on the high seed-bits.
                for (j = 0; j < 5; j++) {
                    seed = base + ((j+0x53) << 48);
                    setWorldSeed(&layerBiomeDummy, seed);
                    setChunkSeed(&layerBiomeDummy, areaX+1, areaZ+1);
                    if(mcNextInt(&layerBiomeDummy, 6) == 5)
                        break;
                }
                if (j >= 5) continue;

                qhpos[0] = getWitchHutPos(base, 0+rX, 0+rZ);
                qhpos[1] = getWitchHutPos(base, 0+rX, 1+rZ);
                qhpos[2] = getWitchHutPos(base, 1+rX, 0+rZ);
                qhpos[3] = getWitchHutPos(base, 1+rX, 1+rZ);


                long hits = 0, swpc;

                for(j = 0; j < 0x10000; j++)
                {
                    seed = base + (j << 48);

                    /** Pre-Generation Checks **/
                    // We can check that at least one swamp could generate in
                    // this area before doing the biome generator checks.
                    setWorldSeed(&layerBiomeDummy, seed);

                    setChunkSeed(&layerBiomeDummy, areaX+1, areaZ+1);
                    if(mcNextInt(&layerBiomeDummy, 6) != 5)
                        continue;

                    // This seed base does not seem to contain many quad huts,
                    // so make a more detailed analysis of the surroundings and
                    // see if there is enough potential for more swamps to
                    // justify searching fruther.
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

                    // Dismiss seeds that don't have a swamp near the quad
                    // temple.
                    setWorldSeed(lFilterBiome, seed);
                    genArea(lFilterBiome, biomeCache, areaX+1, areaZ+1, 1, 1);

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
