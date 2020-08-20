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

#if defined(_WIN32)
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

int main(int argc, char *argv[])
{
    // Always initialize the biome list before starting any seed finder or
    // biome generator.
    initBiomes();
    LayerStack g;

    // Translate the positions to the desired regions.
    int regPosX = 0;
    int regPosZ = 0;

    int mcversion = MC_1_16;
    StructureConfig featureConfig;

    char seedFileName[256] = {};
    enum LowBitSet lbitset = LBIT_ALL;
    int radius = 128;

    if (argc > 2)
    {
        if (sscanf(argv[1], "%d", &regPosX) != 1) regPosX = 0;
        if (sscanf(argv[2], "%d", &regPosZ) != 1) regPosZ = 0;

        if (argc > 3)
        {
            int mcarg1 = 0, mcarg2 = 0;
            int ac = sscanf(argv[3], "%d.%d", &mcarg1, &mcarg2);
            if (ac < 1)
            {
                printf("Bad version format\n");
                exit(1);
            }
            if (ac > 1)
                mcarg1 = 100 * mcarg1 + mcarg2;
            if (mcarg1 < 113)
                mcversion = MC_1_7;
            else if (mcarg1 < 116)
                mcversion = MC_1_13;
            else
                mcversion = MC_1_16;
        }
        else
        {
            printf("MC version not specified. Defaulting to 1.16\n");
        }
    }
    else
    {
        printf("Usage:\n"
               "find_quadhuts [regionX] [regionZ] [mcversion]\n"
               "Defaulting to origin.\n\n");
    }

    regPosX -= 1;
    regPosZ -= 1;

    if (mcversion >= MC_1_13)
    {
        featureConfig = SWAMP_HUT_CONFIG;
    }
    else
    {
        featureConfig = FEATURE_CONFIG;
    }

    setupGenerator(&g, mcversion);
    
#if defined(_WIN32)
    _mkdir("./seeds");
#else 
    mkdir("./seeds", 0773);
#endif

#if 0
    sprintf(seedFileName, "./seeds/quadbase_ideal_%d.txt", featureConfig.salt);
    lbitset = LBIT_IDEAL;
    radius = 128;
#elif 0
    sprintf(seedFileName, "./seeds/quadbase_classic_%d.txt", featureConfig.salt);
    lbitset = LBIT_CLASSIC;
    radius = 128;
#elif 1
    sprintf(seedFileName, "./seeds/quadbase_normal_%d.txt", featureConfig.salt);
    lbitset = LBIT_HUT_NORMAL;
    radius = 128;
#else
    sprintf(seedFileName, "./seeds/quadbase_barely_%d.txt", featureConfig.salt);
    lbitset = LBIT_HUT_BARELY;
    radius = 128;
#endif

    if (access(seedFileName, F_OK))
    {
        printf("Seed base file does not exist: Creating new one.\n"
               "This can take a while...\n");
        int threads = 6;
        search4QuadBases(seedFileName, threads, featureConfig, radius, lbitset);
    }

    int64_t i, j, qhcnt;
    int64_t base, seed;
    int64_t *qhcandidates = loadSavedSeeds(seedFileName, &qhcnt);


    Layer *lFilterBiome = &g.layers[L_BIOME_256];
    int *biomeCache = allocCache(lFilterBiome, 3, 3);


    // Load the positions of the four structures that make up the quad-structure
    // so we can test the biome at these positions.
    Pos qpos[4];

    // layerSeed for Layer 19: Biome, to make preliminary seed tests.
    int64_t lsBiome = g.layers[L_BIOME_256].layerSeed;


    int areaX = (regPosX << 1) + 1;
    int areaZ = (regPosZ << 1) + 1;

    int check_swamp_hut_specifics = 1;
    int64_t ss, cs;

    // Search for a swamp at the structure positions
    for (i = 0; i < qhcnt; i++)
    {
        base = moveStructure(qhcandidates[i], regPosX, regPosZ);

        qpos[0] = getStructurePos(featureConfig, base, 0+regPosX, 0+regPosZ, NULL);
        qpos[1] = getStructurePos(featureConfig, base, 0+regPosX, 1+regPosZ, NULL);
        qpos[2] = getStructurePos(featureConfig, base, 1+regPosX, 0+regPosZ, NULL);
        qpos[3] = getStructurePos(featureConfig, base, 1+regPosX, 1+regPosZ, NULL);

        /*
        for (j = 0; j < 4; j++)
        {
            printf("(%d,%d) ", qpos[j].x, qpos[j].z);
        }
        printf("\n");
        */

        if (check_swamp_hut_specifics)
        {
            // This little magic code checks if there is a meaningful chance for
            // this seed base to generate swamps in the area.
            // The idea is, that the conversion from Lush temperature to swamp
            // is independent of surroundings, so we can test for this
            // conversion beforehand. Furthermore, biomes tend to leak into the
            // negative coordinates because of the Zoom layers, so the majority
            // of hits will occur when SouthEast corner (at a 1:256 scale) of
            // the quad-hut has a swamp. (This assumption misses about 1 in 500
            // quad-hut seeds.) Finally, here we also exploit that the minecraft
            // random number generator is quite bad, the "mcNextRand() mod 6"
            // check has a period pattern of ~3 on the high seed-bits, which
            // means we can avoid checking all 16 high-bit combinations.
            for (j = 0; j < 5; j++)
            {
                seed = base + ((j+0x53) << 48);
                ss = getStartSeed(seed, lsBiome);
                cs = getChunkSeed(ss, areaX+1, areaZ+1);
                if (mcFirstInt(cs, 6) == 5)
                    break;
            }
            if (j >= 5)
                continue;
        }

        int64_t hits = 0, swpc;

        for (j = 0; j < 0x10000; j++)
        {
            seed = base + (j << 48);

            if (check_swamp_hut_specifics)
            {
                /** Pre-Generation Checks **/
                // We can check that at least one swamp could generate in this
                // area before doing the biome generator checks.
                ss = getStartSeed(seed, lsBiome);
                cs = getChunkSeed(ss, areaX+1, areaZ+1);
                if (mcFirstInt(cs, 6) != 5)
                    continue;

                // This seed base does not seem to contain many quad huts, so
                // make a more detailed analysis of the surroundings and see if
                // there is enough potential for more swamps to justify
                // searching further.
                if (hits == 0 && (j & 0xfff) == 0xfff)
                {
                    swpc = 0;
                    cs = getChunkSeed(ss, areaX, areaZ+1);
                    swpc += mcFirstInt(cs, 6) == 5;
                    cs = getChunkSeed(ss, areaX+1, areaZ);
                    swpc += mcFirstInt(cs, 6) == 5;
                    cs = getChunkSeed(ss, areaX, areaZ);
                    swpc += mcFirstInt(cs, 6) == 5;

                    if (swpc < (j > 0x1000 ? 2 : 1))
                        break;
                }

                // Dismiss seeds that don't have a swamp near the quad temple.
                setWorldSeed(lFilterBiome, seed);
                genArea(lFilterBiome, biomeCache, (regPosX<<1)+2, (regPosZ<<1)+2, 1, 1);

                if (biomeCache[0] != swamp)
                    continue;
            }

            // we need to use config here for pre 1.13 as that would not
            // specify that we are only interested in swamp huts
            if (!isViableStructurePos(Swamp_Hut, mcversion, &g, seed, qpos[0].x, qpos[0].z)) continue;
            if (!isViableStructurePos(Swamp_Hut, mcversion, &g, seed, qpos[1].x, qpos[1].z)) continue;
            if (!isViableStructurePos(Swamp_Hut, mcversion, &g, seed, qpos[2].x, qpos[2].z)) continue;
            if (!isViableStructurePos(Swamp_Hut, mcversion, &g, seed, qpos[3].x, qpos[3].z)) continue;

            printf("%" PRId64 "\n", seed);
            hits++;
        }
    }

    free(biomeCache);

    return 0;
}
