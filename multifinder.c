/**
 * A quad hut finder with lots of fancy options.
 */

#include "finders.h"
#include "generator.h"
#include "layers.h"

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


int DEBUG = 0;

void debug(char *msg) {
    if (DEBUG)
        fprintf(stderr, " --- %s\n", msg);
}

typedef struct {
    int numMonuments;
    Pos monuments[4];
} Monuments;

enum BiomeConfigs {
    noneCfg = -1,
    oceanCfg = 0,
    jungleCfg,
    megaTaigaCfg,
    mesaCfg,
    mushroomIslandCfg,
    flowerForestCfg,
    iceSpikesCfg,
    mesaBryceCfg,
    sunflowerPlainsCfg,
};
#define NUM_BIOME_SEARCH_CONFIGS 9

typedef struct {
    char name[20];
    float spawnFraction;
    float naturalFraction;
    int lookup[256];
} BiomeSearchConfig;
BiomeSearchConfig biomeSearchConfigs[NUM_BIOME_SEARCH_CONFIGS];

typedef struct {
    // Configuration options
    int disableOptimizations;
    int64_t startSeed;
    int64_t endSeed;
    int threads;
    char outputDir[256];
    int append;
    char baseSeedsFile[256];

    // Additional search critera
    int allBiomes;
    BiomeSearchConfig *plentifulBiome;
    int plentifulness;
    BiomeSearchConfig *spawnBiomes;
    int monumentDistance;
    int strongholdDistance;
    int woodlandMansions;

    // Search radius options
    int radius;
    int biomeRadius;
    int hutRadius;
    int mansionRadius;
} SearchOptions;

typedef struct {
    int thread;
    int startIndex;
    const int64_t *qhcandidates;
    int64_t qhcount;
    const SearchOptions *opts;
    char filename[256];
} ThreadInfo;

typedef struct {
    int *filter;
    int *lastLayer;
    int *structure;
    int *spawnArea;
    int *shore;
} SearchCaches;

#define INT_ERROR "An integer argument is required with --%s\n"


// Adventuring time 1.12
// Beach, Birch Forest, Birch Forest Hills, Cold Beach, Cold Taiga, Cold Taiga
// Hills, Deep Ocean, Desert, Desert Hills, Extreme Hills, Extreme Hills+,
// Forest, Forest Hills, Frozen River, Ice Mountains, Ice Plains, Jungle, Jungle
// Edge, Jungle Hills, Mega Taiga, Mega Taiga Hills, Mesa, Mesa Plateau, Mesa
// Plateau F, Mushroom Island, Mushroom Island Shore, Ocean, Plains, River,
// Roofed Forest, Savanna, Savanna Plateau, Stone Beach, Swampland, Taiga, Taiga
// Hills

// Adventuring time 1.13
// Beach, Birch Forest, Birch Forest Hills, Cold Beach, Cold Taiga, Cold Taiga
// Hills, Deep Ocean, Desert, Desert Hills, Extreme Hills, Extreme Hills+,
// Forest, Forest Hills, Frozen River, Ice Mountains, Ice Plains, Jungle, Jungle
// Edge, Jungle Hills, Mega Taiga, Mega Taiga Hills, Mesa, Mesa Plateau, Mesa
// Plateau F, Mushroom Island, Mushroom Island Shore, Ocean, Plains, River,
// Roofed Forest, Savanna, Savanna Plateau, Stone Beach, Swampland, Taiga, Taiga
// Hills, Cold Ocean, Cold Deep Ocean, Frozen Deep Ocean, Lukewarm Ocean,
// Lukewarm Deep Ocean, Warm Ocean


void initSearchConfig(
        char *name, BiomeSearchConfig *config,
        float spawnFraction, float naturalFraction,
        int includedCount, int *includedBiomes,
        int ignoredCount, int *ignoredBiomes) {
    snprintf(config->name, 20, "%s", name);
    config->spawnFraction = spawnFraction;
    config->naturalFraction = naturalFraction;

    memset(config->lookup, 0, 256*sizeof(int));
    for (int i=0; i<includedCount; i++) {
        assert(includedBiomes[i] < 256);
        config->lookup[includedBiomes[i]] = 1;
    }
    for (int i=0; i<ignoredCount; i++) {
        assert(ignoredBiomes[i] < 256);
        config->lookup[ignoredBiomes[i]] = -1;
    }
}


void initSearchConfigs() {
    // Ocean spawns are interesting for survival island situations, or for
    // spawn-proofing the spawn chunks for passive mob farms.
    initSearchConfig(
            "ocean", &biomeSearchConfigs[oceanCfg],
            0.85f, (15.7+15.7)/100.0,
            3, (int[]){ocean, frozenOcean, deepOcean},
            0, (int[]){});

    // Jungle, mega taiga and mesa biomes are considered "special" by the biome
    // generator, making them more rare (even though they tend to be large),
    // and have unique items.
    initSearchConfig(
            "jungle", &biomeSearchConfigs[jungleCfg],
            0.95f, (1.03+0.359+0.0853+0.0492+0.000451)/100.0,
            5, (int[]){jungle, jungleHills, jungleEdge,
                       jungle+128, jungleEdge+128},
            3, (int[]){river, ocean, deepOcean});

    initSearchConfig(
            "mega taiga", &biomeSearchConfigs[megaTaigaCfg],
            0.90f, (0.691+0.313+0.0358+0.0354)/100.0,
            4, (int[]){megaTaiga, megaTaigaHills,
                       megaTaiga+128, megaTaigaHills+128},
            3, (int[]){river, ocean, deepOcean});

    initSearchConfig(
            "mesa", &biomeSearchConfigs[mesaCfg],
            0.90f, (0.469+0.242+0.103+0.0236+0.0121+0.00566)/100.0,
            6, (int[]){mesa, mesaPlateau_F, mesaPlateau,
                       mesa+128, mesaPlateau_F+128, mesaPlateau+128},
            3, (int[]){river, ocean, deepOcean});

    // Mushroom islands are treated uniquely by the biome generator.
    initSearchConfig(
            "mushroom island", &biomeSearchConfigs[mushroomIslandCfg],
            0.50f, (0.0370+0.0208)/100.0,
            2, (int[]){mushroomIsland, mushroomIslandShore},
            3, (int[]){river, ocean, deepOcean});

    // These are the interesting M (mutated) biomes. Some other M or "hills"
    // biomes are rare, but are not interesting at all; "Jungle Edge M" is the
    // rarest biome, but who cares?
    initSearchConfig(
            "flower forest", &biomeSearchConfigs[flowerForestCfg],
            0.65f, (0.430)/100.0,
            1, (int[]){forest+128},
            3, (int[]){river, ocean, deepOcean});

    initSearchConfig(
            "ice spikes", &biomeSearchConfigs[iceSpikesCfg],
            0.75f, (0.157)/100.0,
            1, (int[]){icePlains+128},
            7, (int[]){icePlains, iceMountains, frozenRiver,
                       river, frozenOcean, ocean, deepOcean});

    initSearchConfig(
            "mesa bryce", &biomeSearchConfigs[mesaBryceCfg],
            0.75f, (0.0236)/100.0,
            1, (int[]){mesa+128},
            8, (int[]){mesa, mesaPlateau_F, mesaPlateau,
                       mesaPlateau_F+128, mesaPlateau+128,
                       river, ocean, deepOcean});

    initSearchConfig(
            "sunflower plains", &biomeSearchConfigs[sunflowerPlainsCfg],
            0.65f, (0.571)/100.0,
            1, (int[]){plains+128},
            3, (int[]){river, ocean, deepOcean});

}


void usage() {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  multifinder [options]\n");
    fprintf(stderr, "    --help\n");
    fprintf(stderr, "    --disable_optimizations\n");
    fprintf(stderr, "      Disable speed optimizations which may cause\n");
    fprintf(stderr, "      search to miss some quad hut seeds. Will find up\n");
    fprintf(stderr, "      to 10%% more seeds, but will run about 13x\n");
    fprintf(stderr, "      slower.\n");
    fprintf(stderr, "    --start_seed=<integer> or \"random\"\n");
    fprintf(stderr, "    --end_seed=<integer>\n");
    fprintf(stderr, "    --threads=<integer>\n");
    fprintf(stderr, "    --output_dir=<string>\n");
    fprintf(stderr, "    --append\n");
    fprintf(stderr, "      append to output files instead of overwriting.\n");
    fprintf(stderr, "    --base_seeds_file=<string>\n");
    fprintf(stderr, "    --all_biomes\n");
    fprintf(stderr, "      Search for all biomes within search radius.\n");
    fprintf(stderr, "    --plentiful_biome=<string>\n");
    fprintf(stderr, "      Find seeds with lots of a particular biome.\n");
    fprintf(stderr, "    --plentifulness=<integer>\n");
    fprintf(stderr, "      Biome is N times more common than usual.\n");
    fprintf(stderr, "    --spawn_biomes=<string>\n");
    fprintf(stderr, "      ocean, jungle, mega_taiga, mesa, mushroom_island,\n");
    fprintf(stderr, "      flower_forest, ice_spikes, mesa_bryce or\n");
    fprintf(stderr, "      sunflower_plains.\n");
    fprintf(stderr, "    --monument_distance=<integer>\n");
    fprintf(stderr, "      Search for an ocean monument within a number of\n");
    fprintf(stderr, "      chunks of the quad hut perimeter.\n");
    fprintf(stderr, "    --stronghold_distance=<integer>\n");
    fprintf(stderr, "      Search for a stronghold within a number of\n");
    fprintf(stderr, "      blocks of the quad hut perimeter.\n");
    fprintf(stderr, "    --woodland_mansions=<integer>\n");
    fprintf(stderr, "      Search for a number of nearby woodland mansions.\n");
    fprintf(stderr, "    --radius=<integer>\n");
    fprintf(stderr, "      Search radius, in blocks (rounded to nearest\n");
    fprintf(stderr, "      structure region).\n");
    fprintf(stderr, "    --biome_radius=<integer>\n");
    fprintf(stderr, "      Search radius, in blocks, for --all_biomes\n");
    fprintf(stderr, "      option. Defaults to --search_radius.\n");
    fprintf(stderr, "    --hut_radius=<integer>\n");
    fprintf(stderr, "      Search radius, in blocks, for quad witch huts.\n");
    fprintf(stderr, "      Defaults to --search_radius.\n");
    fprintf(stderr, "    --mansion_radius=<integer>\n");
    fprintf(stderr, "      Search radius, in blocks, for --wodland_mansions\n");
    fprintf(stderr, "      option. Defaults to --search_radius.\n");
}


int64_t parseHumanArgument(char *arg, const char *flagName) {
    char *endptr;

    int len = strlen(arg);
    if (len < 1) {
        fprintf(stderr, INT_ERROR, flagName);
        exit(-1);
    }

    int64_t mult = 1;
    switch (arg[len-1]) {
        case 'K': mult = 1024LL; break;
        case 'M': mult = 1024LL*1024LL; break;
        case 'B': mult = 1024LL*1024LL*1024LL; break;
        case 'G': mult = 1024LL*1024LL*1024LL; break;
        case 'T': mult = 1024LL*1024LL*1024LL*1024LL; break;
    }

    if (mult != 1)
        arg[len-1] = 0;
    int64_t val = strtol(arg, &endptr, 10);
    if (errno != 0) {
        fprintf(stderr, INT_ERROR, flagName);
        exit(-1);
    }

    return val*mult;
}


int parseIntArgument(const char *arg, const char *flagName) {
    char *endptr;

    int val = strtol(arg, &endptr, 10);
    if (errno != 0) {
        fprintf(stderr, INT_ERROR, flagName);
        exit(-1);
    }

    return val;
}


BiomeSearchConfig* parseBiome(const char *arg) {
    if (strcmp(arg, "ocean")                == 0)
        return &biomeSearchConfigs[oceanCfg];

    if (strcmp(arg, "jungle")               == 0)
        return &biomeSearchConfigs[jungleCfg];

    if (strcmp(arg, "mega_taiga")           == 0 ||
            strcmp(arg, "megaTaiga")        == 0)
        return &biomeSearchConfigs[megaTaigaCfg];

    if (strcmp(arg, "mesa")                 == 0)
        return &biomeSearchConfigs[mesaCfg];

    if (strcmp(arg, "mushroom_island")      == 0 ||
            strcmp(arg, "mushroom")         == 0 ||
            strcmp(arg, "mushroomIsland")   == 0)
        return &biomeSearchConfigs[mushroomIslandCfg];

    if (strcmp(arg, "flower_forest")        == 0 ||
            strcmp(arg, "flower")           == 0 ||
            strcmp(arg, "flowerForest")     == 0)
        return &biomeSearchConfigs[flowerForestCfg];

    if (strcmp(arg, "ice_spikes")           == 0 ||
            strcmp(arg, "iceSpikes")        == 0)
        return &biomeSearchConfigs[iceSpikesCfg];

    if (strcmp(arg, "mesa_bryce")           == 0 ||
            strcmp(arg, "mesaBryce")        == 0 ||
            strcmp(arg, "bryce")            == 0)
        return &biomeSearchConfigs[mesaBryceCfg];

    if (strcmp(arg, "sunflower")            == 0 ||
            strcmp(arg, "sunflower_plains") == 0 ||
            strcmp(arg, "sunflowerPlains")  == 0)
        return &biomeSearchConfigs[sunflowerPlainsCfg];

    fprintf(stderr, "Unknown biome group \"%s\".\n", arg);
    exit(-1);
}


int blockToRegion(int val, int regionSize) {
    return (int)ceil((double)val / (regionSize*16));
}


SearchOptions parseOptions(int argc, char *argv[]) {
    int c;
    SearchOptions opts = {
        .disableOptimizations = 0,
        .startSeed            = 0,
        .endSeed              = 1LL<<48,
        .threads              = 1,
        .outputDir            = "",
        .append               = 0,
        .baseSeedsFile        = "./seeds/quadhutbases_1_13_Q1.txt",
        .allBiomes            = 0,
        .plentifulBiome       = NULL,
        .plentifulness        = 25,
        .spawnBiomes          = NULL,
        .monumentDistance     = 0,
        .strongholdDistance   = 0,
        .woodlandMansions     = 0,
        .radius               = 2048,
        .biomeRadius          = 0,
        .hutRadius            = 0,
        .mansionRadius        = 0,
    };

    while (1) {
        static struct option longOptions[] = {
            {"help",                  no_argument,       NULL, 'h'},
            {"debug",                 no_argument,       NULL, 'D'},
            {"disable_optimizations", no_argument,       NULL, 'X'},
            {"start_seed",            required_argument, NULL, 's'},
            {"end_seed",              required_argument, NULL, 'e'},
            {"threads",               required_argument, NULL, 't'},
            {"output_dir",            required_argument, NULL, 'o'},
            {"append",                no_argument,       NULL, 'A'},
            {"base_seeds_file",       required_argument, NULL, 'S'},
            {"all_biomes",            no_argument,       NULL, 'a'},
            {"plentiful_biome",       required_argument, NULL, 'p'},
            {"plentifulness",         required_argument, NULL, 'P'},
            {"spawn_biomes",          required_argument, NULL, 'b'},
            {"monument_distance",     required_argument, NULL, 'm'},
            {"stronghold_distance",   required_argument, NULL, 'z'},
            {"woodland_mansions",     required_argument, NULL, 'w'},
            {"radius",                required_argument, NULL, 'r'},
            {"biome_radius",          required_argument, NULL, 'B'},
            {"hut_radius",            required_argument, NULL, 'H'},
            {"mansion_radius",        required_argument, NULL, 'M'},
        };
        int index = 0;
        c = getopt_long(argc, argv,
                "hDXs:e:t:o:AS:ap:P:b:m:z:w:r:B:H:M:", longOptions, &index);

        if (c == -1)
            break;

        switch (c) {
            case 'h':
                usage();
                exit(0);
                break;
            case 'D':
                DEBUG = 1;
                break;
            case 'X':
                opts.disableOptimizations = 1;
                break;
            case 's':
                if (strcmp(optarg, "random") == 0) {
                    int64_t lower = rand() & 0xffffffff;
                    int64_t upper = rand() & 0xffff;
                    opts.startSeed = (upper << 32) + lower;
                } else {
                    opts.startSeed = parseHumanArgument(
                            optarg, longOptions[index].name);
                }
                break;
            case 'e':
                opts.endSeed = parseHumanArgument(
                        optarg, longOptions[index].name);
                break;
            case 't':
                opts.threads = parseIntArgument(
                        optarg, longOptions[index].name);
                break;
            case 'o':
                if (strlen(optarg) > 255-13) {
                    fprintf(stderr, "Output path too long.");
                    exit(-1);
                }
                strncpy(opts.outputDir, optarg, 256);
                int len = strlen(opts.outputDir);
                if (opts.outputDir[len-1] == '/')
                    opts.outputDir[len-1] = 0;
                break;
            case 'A':
                opts.append = 1;
                break;
            case 'S':
                if (strlen(optarg) > 255) {
                    fprintf(stderr, "Base seeds filename too long.");
                    exit(-1);
                }
                strncpy(opts.baseSeedsFile, optarg, 256);
                break;
            case 'a':
                opts.allBiomes = 1;
                break;
            case 'p':
                opts.plentifulBiome = parseBiome(optarg);
                break;
            case 'P':
                opts.plentifulness = parseIntArgument(
                        optarg, longOptions[index].name);
                break;
            case 'b':
                opts.spawnBiomes = parseBiome(optarg);
                break;
            case 'm':
                opts.monumentDistance = parseIntArgument(
                        optarg, longOptions[index].name);
                break;
            case 'z':
                opts.strongholdDistance = parseIntArgument(
                        optarg, longOptions[index].name);
                break;
            case 'w':
                opts.woodlandMansions = parseIntArgument(
                        optarg, longOptions[index].name);
                break;
            case 'r':
                opts.radius = parseIntArgument(
                        optarg, longOptions[index].name);
                break;
            case 'B':
                opts.biomeRadius = parseIntArgument(
                        optarg, longOptions[index].name);
                break;
            case 'H':
                opts.hutRadius = blockToRegion(
                        parseIntArgument(optarg, longOptions[index].name), 32);
                break;
            case 'M':
                opts.mansionRadius = blockToRegion(
                        parseIntArgument(optarg, longOptions[index].name), 80);
                break;
            default:
                exit(-1);
        }

        if (!opts.biomeRadius)
            opts.biomeRadius = opts.radius;
        if (!opts.hutRadius)
            opts.hutRadius = blockToRegion(opts.radius, 32);
        if (!opts.mansionRadius)
            opts.mansionRadius = blockToRegion(opts.radius, 80);

    }
    return opts;
}


int64_t* getBaseSeeds(int64_t *qhcount, int threads, const char *seedFileName) {
    if (access(seedFileName, F_OK)) {
        fprintf(stderr, "Seed base file does not exist: Creating new one.\n"
                "This may take a few minutes...\n");
        int quality = 1;
        search4QuadBases(seedFileName, threads, SWAMP_HUT_SEED, quality);
    }

    return loadSavedSeeds(seedFileName, qhcount);
}


int getBiomeAt(const LayerStack g, const Pos pos, int *buf) {
    genArea(&g.layers[g.layerNum-1], buf, pos.x, pos.z, 1, 1);
    return buf[0];
}


Monuments potentialMonuments(int64_t baseSeed, int distance) {
    const int upper = 23 - distance;
    const int lower = distance;
    Monuments potential;
    potential.numMonuments = 0;
    Pos pos;

    pos = getOceanMonumentChunk(baseSeed, 0, 0);
    if (pos.x >= upper && pos.z >= upper) {
        pos.x = (pos.x +  0) * 16 + 8;
        pos.z = (pos.z +  0) * 16 + 8;
        potential.monuments[potential.numMonuments++] = pos;
    }

    pos = getOceanMonumentChunk(baseSeed, 1, 0);
    if (pos.x <= lower && pos.z >= upper) {
        pos.x = (pos.x + 32) * 16 + 8;
        pos.z = (pos.z +  0) * 16 + 8;
        potential.monuments[potential.numMonuments++] = pos;
    }

    pos = getOceanMonumentChunk(baseSeed, 0, 1);
    if (pos.x >= upper && pos.z <= lower) {
        pos.x = (pos.x +  0) * 16 + 8;
        pos.z = (pos.z + 32) * 16 + 8;
        potential.monuments[potential.numMonuments++] = pos;
    }

    pos = getOceanMonumentChunk(baseSeed, 1, 1);
    if (pos.x <= lower && pos.z <= lower) {
        pos.x = (pos.x + 32) * 16 + 8;
        pos.z = (pos.z + 32) * 16 + 8;
        potential.monuments[potential.numMonuments++] = pos;
    }

    return potential;
}


int verifyMonuments(LayerStack *g, int *cache, Monuments *mon, int rX, int rZ) {
    for (int m = 0; m < mon->numMonuments; m++) {
        // Translate monument coordintes from the origin-relative coordinates
        // from the base seed family.
        int monX = mon->monuments[m].x + rX*32*16;
        int monZ = mon->monuments[m].z + rZ*32*16;
        if (isViableOceanMonumentPos(*g, cache, monX, monZ)) {
            return 1;
        }
    }
    return 0;
}


int hasStronghold(LayerStack *g, int *cache, int64_t seed, int maxDistance, Pos qhpos[4]) {
    Pos strongholds[128];
    // Approximateish center of quad hut formation.
    int cx = (qhpos[0].x + qhpos[1].x + qhpos[2].x + qhpos[3].x) / 4;
    int cz = (qhpos[0].z + qhpos[1].z + qhpos[2].z + qhpos[3].z) / 4;

    // Quit searching strongholds in outer rings; include a fudge factor.
    int maxRadius = (int)round(sqrt(cx*cx + cz*cz)) + maxDistance + 16;

    int count = findStrongholds(g, cache, strongholds, seed, maxRadius);

    for (int i=0; i<count; i++) {
        int dx = strongholds[i].x - cx;
        int dz = strongholds[i].z - cz;
        if (dx*dx + dz*dz <= maxDistance*maxDistance)
            return 1;
    }

    return 0;
}


int hasMansions(const LayerStack *g, int *cache, int64_t seed, int radius, int minCount) {
    int count = 0;

    for (int rZ=-radius; rZ<radius; rZ++) {
        for (int rX=-radius; rX<radius; rX++) {
            Pos mansion = getMansionPos(seed, rX, rZ);
            if (isViableMansionPos(*g, cache, mansion.x, mansion.z)) {
                count++;
                if (count >= minCount)
                    return 1;
            }
        }
    }
    return 0;
}


int hasSpawnBiome(LayerStack *g, int *cache, Pos spawn, BiomeSearchConfig *config) {
    Layer *lShoreBiome = &g->layers[L_SHORE_16];

    int areaX = spawn.x >> 4;
    int areaZ = spawn.z >> 4;
    float ignoreFraction = 0;
    float includeFraction = 0;

    genArea(lShoreBiome, cache, areaX-9, areaZ-9, 18, 18);

    for (int i=0; i<18*18; i++) {
        switch (config->lookup[cache[i]]) {
            case 1:
                includeFraction += 1;
                break;
            case -1:
                ignoreFraction += 1;
                break;
        }
    }

    includeFraction /= (18*18);
    ignoreFraction /= (18*18);
    if (ignoreFraction > 0.80f) { ignoreFraction = 0.80f; }

    return includeFraction / (1.0 - ignoreFraction) >= config->spawnFraction;
}


void getAreaBiomes(LayerStack *g, int *cache, Pos spawn, int radius) {
    Layer *lShoreBiome = &g->layers[L_SHORE_16];
    int width = radius >> 3;  // 1:16 resolution layer, *2 for a radius.
    int left = (spawn.x - radius) >> 4;
    int top  = (spawn.z - radius) >> 4;
    genArea(lShoreBiome, cache, left, top, width, width);
}


int getBiomeGroup(int biome) {
    // Most biomes are basically everywhere, so we only make an effort to
    // count up the ones that have a good chance of being far away. The list
    // also focuses on biomes with items that don't occur elsewhere (packed ice,
    // teracotta, podzol, jungle saplings, cocoa beans, certain flowers, etc.)

    // A list of bomes that completes the Adventuring Time advancement would
    // also be a cool option.
    switch(biome) {
        case ocean:
        case frozenOcean:
        case deepOcean:
            return 1;
        case jungle:
        case jungleHills:
        case jungleEdge:
        case jungle+128:            // Jungle M
        case jungleEdge+128:        // Jungle Edge M
            return 2;
        case megaTaiga:
        case megaTaigaHills:
        case megaTaiga+128:         // Mega Spruce Taiga
        case megaTaigaHills+128:    // Mega Spruce Taiga Hills
            return 3;
        case mesa:
        case mesaPlateau_F:
        case mesaPlateau:
        case mesaPlateau_F+128:     // Mesa Plateau F M
        case mesaPlateau+128:       // Mesa Plateau M
            return 4;
        case mushroomIsland:
        case mushroomIslandShore:
            return 5;
        case forest+128:            // Flower Forest
            return 6;
        case icePlains+128:         // Ice Spikes
            return 7;
        case mesa+128:              // Mesa Bryce
            return 8;
        case plains+128:            // Sunflower plains
            return 9;
    }
    return 0;
}


#define NUM_ALL_BIOMES 10
int hasAllBiomes(int *cache, int radius) {
    int width = radius >> 3;  // 1:16 resolution layer, *2 for a radius.
    int area = width*width;

    int biomeCounts[NUM_ALL_BIOMES] = {0};
    for (int i=0; i<area; i++) {
        biomeCounts[getBiomeGroup(cache[i])]++;
    }

    for (int i=0; i<NUM_ALL_BIOMES; i++) {
        // Require a non-trivial amount of biome area (e.g. a 4x4 chunk area).
        if (biomeCounts[i] < 16)
            return 0;
    }
    return 1;
}


int hasPlentifulBiome(int *cache, int radius, int plentifulness,
        BiomeSearchConfig *config) {
    int width = radius >> 3;  // 1:16 resolution layer, *2 for a radius.
    int area = width*width;

    float fraction = 0.0;
    for (int i=0; i<area; i++) {
        if (config->lookup[cache[i]] == 1)
            fraction += 1.0;
    }

    return fraction / area >= config->naturalFraction * plentifulness;
}


SearchCaches preallocateCaches(const LayerStack *g, const SearchOptions *opts) {
    SearchCaches cache = {NULL, NULL, NULL, NULL, NULL};

    // 1:256 and 1:1 layer caches for checking a single value.
    cache.filter = allocCache(&g->layers[L_BIOME_256], 3, 3);
    cache.lastLayer = allocCache(&g->layers[g->layerNum-1], 3, 3);

    // Just make a cache big enough to accomodate all structures.
    cache.structure = allocCache(&g->layers[L_RIVER_MIX_4], 256, 256);

    // Shore layer is 16:1, and spawn is 256x256, and we want to include
    // the neighboring areas which blend into it -> 18.
    if (opts->spawnBiomes)
        cache.spawnArea = allocCache(&g->layers[L_SHORE_16], 18, 18);

    // A 1:16 resolution biome, but x2 because it's a radius.
    if (opts->allBiomes || opts->plentifulBiome) {
        int biomeWidth = opts->biomeRadius >> 3;
        cache.shore = allocCache(
                &g->layers[L_SHORE_16], biomeWidth, biomeWidth);
    }

    return cache;
}


void freeCaches(SearchCaches *cache) {
    free(cache->filter);
    free(cache->lastLayer);
    free(cache->structure);
    if (cache->spawnArea)
        free(cache->spawnArea);
    if (cache->shore)
        free(cache->shore);
}


void *searchQuadHutsThread(void *data) {
    const ThreadInfo info = *(const ThreadInfo *)data;
    const SearchOptions opts = *info.opts;

    LayerStack g = setupGenerator();
    Layer *lFilterBiome = &g.layers[L_BIOME_256];
    int64_t j, base, seed;

    Monuments monuments = {0};

    // Load the positions of the four structures that make up the quad-structure
    // so we can test the biome at these positions.
    Pos qhpos[4];

    // Setup a dummy layer for Layer 19: Biome.
    Layer layerBiomeDummy;
    setupLayer(256, &layerBiomeDummy, NULL, 200, NULL);

    FILE *fh;
    if (strlen(info.filename)) {
        debug("Opening output files.");
        fh = fopen(info.filename, opts.append ? "a" : "w");
        if (fh == NULL) {
            fprintf(stderr, "Could not open file %s.\n", info.filename);
            return NULL;
        }
    } else {
        fh = stdout;
    }

    SearchCaches cache = preallocateCaches(&g, &opts);

    // Every nth + m base seed is assigned to thread m;
    debug("Starting search.");
    for(int i=info.startIndex;
            i < info.qhcount && info.qhcandidates[i] < opts.endSeed;
            i+=opts.threads) {
        int basehits = 0;

        debug("New base seed.");
        // The ocean monument check is quick and has a high probability
        // of eliminating the base seed, so perform that first.
        if (opts.monumentDistance) {
            debug("Checking monument distance.");
            monuments = potentialMonuments(
                    info.qhcandidates[i], opts.monumentDistance);
            if (monuments.numMonuments == 0)
                continue;
        }

        for (int rZ = -opts.hutRadius-1; rZ < opts.hutRadius; rZ++) {
            for (int rX = -opts.hutRadius-1; rX < opts.hutRadius; rX++) {
                // The base seed has potential monuments around the origin;
                // if we translate it to rX, rZ, it will always have potential
                // huts around that region.
                base = moveStructure(info.qhcandidates[i], rX, rZ);

                // rZ, rX is the hut region in the upper left of the potential
                // quad hut. Hut regions are 32 chunks/512 blocks. The biome
                // generation layers we're looking at are 1:256 zoom. So
                // the biome area is 2* the hut region. Also, we want the area
                // at the center of the quad-hut regions, so +1.
                int areaX = (rX << 1) + 1;
                int areaZ = (rZ << 1) + 1;

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

                // Misses 8-9% of seeds for a 2x speedup.
                if (!opts.disableOptimizations) {
                    debug("Checking lush conversion.");
                    for (j = 0x53; j < 0x58; j++) {
                        seed = base + (j << 48);
                        setWorldSeed(&layerBiomeDummy, seed);
                        setChunkSeed(&layerBiomeDummy, areaX+1, areaZ+1);
                        if(mcNextInt(&layerBiomeDummy, 6) == 5)
                            break;
                    }
                    if (j >= 0x58)
                        continue;
                }

                debug("Getting witch hut positions.");
                qhpos[0] = getStructurePos(SWAMP_HUT_SEED, base, 0+rX, 0+rZ);
                qhpos[1] = getStructurePos(SWAMP_HUT_SEED, base, 0+rX, 1+rZ);
                qhpos[2] = getStructurePos(SWAMP_HUT_SEED, base, 1+rX, 0+rZ);
                qhpos[3] = getStructurePos(SWAMP_HUT_SEED, base, 1+rX, 1+rZ);

                int64_t hits = 0;
                int64_t hutHits = 0;

                for (j = 0; j < 0x10000; j++) {
                    seed = base + (j << 48);
                    setWorldSeed(&layerBiomeDummy, seed);

                    // This seed base does not seem to contain many quad huts,
                    // so make a more detailed analysis of the surroundings and
                    // see if there is enough potential for more swamps to
                    // justify searching further. Misses and additional 1% of
                    // seeds for a 1.4:1 speedup.

                    // This uses a separate counter for all seeds that pass the
                    // quad hut checks, even if they fail the other checks, so
                    // that the other checks don't cause this to fail too early.
                    if (!opts.disableOptimizations) {
                        if (hutHits == 0 && (j & 0xfff) == 0xfff) {
                            debug("Checking area around a weak seed.");
                            int swpc = 0;
                            setChunkSeed(&layerBiomeDummy, areaX, areaZ+1);
                            swpc += mcNextInt(&layerBiomeDummy, 6) == 5;
                            setChunkSeed(&layerBiomeDummy, areaX+1, areaZ);
                            swpc += mcNextInt(&layerBiomeDummy, 6) == 5;
                            setChunkSeed(&layerBiomeDummy, areaX, areaZ);
                            swpc += mcNextInt(&layerBiomeDummy, 6) == 5;

                            if (swpc < (j > 0x1000 ? 2 : 1))
                                break;
                        }

                        // We can check that at least one swamp could generate in
                        // this area before doing the biome generator checks.
                        // Misses an additional 0.2% of seeds for a 2.75:1 speedup.
                        debug("Checking if swamp is possible in the area.");
                        setChunkSeed(&layerBiomeDummy, areaX+1, areaZ+1);
                        if (mcNextInt(&layerBiomeDummy, 6) != 5)
                            continue;

                        // Dismiss seeds that don't have a swamp near the quad
                        // temple. Misses an additional 0.03% of seeds for a 1.7:1
                        // speedup.
                        debug("Checking if swamp is actually in the area.");
                        setWorldSeed(lFilterBiome, seed);
                        genArea(lFilterBiome, cache.filter, areaX+1, areaZ+1, 1, 1);
                        if (cache.filter[0] != swampland)
                            continue;
                    }

                    debug("Checking witch hut swamps.");
                    applySeed(&g, seed);
                    if (getBiomeAt(g, qhpos[0], cache.lastLayer) != swampland)
                        continue;
                    if (getBiomeAt(g, qhpos[1], cache.lastLayer) != swampland)
                        continue;
                    if (getBiomeAt(g, qhpos[2], cache.lastLayer) != swampland)
                        continue;
                    if (getBiomeAt(g, qhpos[3], cache.lastLayer) != swampland)
                        continue;
                    hutHits++;

                    debug("Other structure checks.");
                    // These checks are a tad slow because they have to get full
                    // biomes for a bit of an area.
                    if (opts.monumentDistance &&
                            !verifyMonuments(&g, cache.structure, &monuments, rX, rZ))
                        continue;

                    if (opts.strongholdDistance &&
                            !hasStronghold(&g, cache.structure, seed, opts.strongholdDistance, qhpos))
                        continue;

                    if (opts.woodlandMansions &&
                            !hasMansions(&g, cache.structure, seed, opts.mansionRadius, opts.woodlandMansions))
                        continue;

                    if (opts.spawnBiomes || opts.allBiomes || opts.plentifulBiome) {
                        debug("Biome checks.");
                        Pos spawn = getSpawn(&g, cache.structure, seed);

                        // This has to get more biome area, so is slower.
                        if (opts.spawnBiomes
                                && !hasSpawnBiome(&g, cache.spawnArea, spawn, opts.spawnBiomes))
                            continue;

                        // These have to get a lot of biome area, so very slow.
                        if (opts.allBiomes || opts.plentifulBiome) {
                            getAreaBiomes(&g, cache.shore, spawn, opts.biomeRadius);
                            if (opts.allBiomes
                                    && !hasAllBiomes(cache.shore, opts.biomeRadius))
                                continue;

                            if (opts.plentifulBiome
                                    && !hasPlentifulBiome(cache.shore, opts.biomeRadius, opts.plentifulness, opts.plentifulBiome))
                                continue;
                        }
                    }

                    fprintf(fh, "%ld\n", seed);
                    hits++;
                    basehits++;
                }
                fflush(fh);
            }
        }
        fprintf(stderr, "Base seed %ld (thread %d): %d hits\n",
                info.qhcandidates[i], info.thread, basehits);
    }

    if (fh != stdout) {
        fclose(fh);
        fprintf(stderr, "%s written.\n", info.filename);
    }
    freeCaches(&cache);
    freeGenerator(g);

    return NULL;
}


int main(int argc, char *argv[])
{
    // Always initialize the biome list before starting any seed finder or
    // biome generator.
    srand(time(NULL));
    initBiomes();
    initSearchConfigs();

    SearchOptions opts = parseOptions(argc, argv);

    if (opts.threads > 1 && strlen(opts.outputDir) < 1) {
        fprintf(stderr,
                "Must specify --output_dir if using more than one thread.");
        exit(-1);
    }

    fprintf(stderr, "===========================================================================\n");
    fprintf(stderr,
            "Searching base seeds %ld-%ld, radius %d using %d threads...\n",
            opts.startSeed, opts.endSeed, opts.radius, opts.threads);
    if (*opts.outputDir) {
        if (opts.append)
            fprintf(stderr, "Appending to files in \"%s\"...\n", opts.outputDir);
        else
            fprintf(stderr, "Writing output to \"%s\"...\n", opts.outputDir);
    }
    if (opts.monumentDistance) {
        fprintf(stderr, "Looking for an ocean monument within %d chunks of quad hut perimeter.\n",
                opts.monumentDistance);
    }
    if (opts.strongholdDistance) {
        fprintf(stderr, "Looking for a stronghold within %d blocks of quad hut center.\n",
                opts.strongholdDistance);
    }
    if (opts.woodlandMansions) {
        fprintf(stderr, "Looking for %d woodland mansions within %d blocks.\n",
                opts.woodlandMansions, opts.mansionRadius*80*16);
    }
    if (opts.allBiomes) {
        fprintf(stderr, "Looking for all biomes within %d blocks.\n", opts.biomeRadius);
    }
    if (opts.plentifulBiome) {
        fprintf(stderr, "Looking for %dx more than usual %s biomes.\n", opts.plentifulness, opts.plentifulBiome->name);
    }
    if (opts.spawnBiomes) {
        fprintf(stderr, "Looking for world spawn in %s biomes.\n", opts.spawnBiomes->name);
    }
    if (opts.disableOptimizations) {
        fprintf(stderr, "WARNING: Optimizations disabled. Will be slow as snot.\n");
    }
    fprintf(stderr, "===========================================================================\n");

    int64_t qhcount;
    const int64_t *qhcandidates = getBaseSeeds(&qhcount, opts.threads, opts.baseSeedsFile);
    int startIndex = 0;
    while (qhcandidates[startIndex] < opts.startSeed && startIndex < qhcount) {
        startIndex++;
    }

    pthread_t threadID[opts.threads];
    ThreadInfo info[opts.threads];

    for (int t=0; t<opts.threads; t++) {
        info[t].thread = t;
        info[t].startIndex = startIndex + t;
        info[t].qhcandidates = qhcandidates;
        info[t].qhcount = qhcount;
        info[t].opts = &opts;

        if (opts.threads == 1 && !strlen(opts.outputDir)) {
            info[t].filename[0] = 0;
        } else {
            snprintf(info[t].filename, 256,
                    "%s/seeds-%02d.txt", opts.outputDir, t);
        }
    }

    for (int t=0; t<opts.threads; t++) {
        pthread_create(
                &threadID[t], NULL, searchQuadHutsThread, (void*)&info[t]);
    }

    for (int t=0; t<opts.threads; t++) {
        pthread_join(threadID[t], NULL);
    }

    if (strlen(opts.outputDir)) {
        char filename[256];
        // TODO: Remove COMPLETE file if present at start of search.
        snprintf(filename, 256, "%s/COMPLETE", opts.outputDir);
        FILE *fh = fopen(filename, "w");
        if (fh != NULL) {
            fprintf(fh, "Done.\n");
            fclose(fh);
        }
    }
    fprintf(stderr, "Done.\n");

    return 0;
}
