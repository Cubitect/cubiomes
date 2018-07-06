#include "finders.h"
#include "generator.h"
#include "layers.h"

#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_WIDTH 3840*2
#define DEFAULT_HEIGHT DEFAULT_WIDTH*9/16
#define DEFAULT_ICON_SCALE 2

typedef struct {
    int64_t seed;
    char ppmfn[256];
    char pngfn[256];
    int width;
    int height;
    int iconScale;
    int hutScale;
    int mansionScale;
    int monumentScale;
    int spawnScale;
    int strongholdScale;
    int use_1_13;
} MapOptions;

void setBiomeColour(unsigned char biomeColour[256][3], int biome,
        unsigned char r, unsigned char g, unsigned char b)
{
    biomeColour[biome][0] = r;
    biomeColour[biome][1] = g;
    biomeColour[biome][2] = b;
}


void initBiomeColours(unsigned char biomeColours[256][3])
{
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
    setBiomeColour(biomeColours, frozenOcean, 144, 144, 160);
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

    setBiomeColour(biomeColours, warmOcean, 0, 50, 92);
    setBiomeColour(biomeColours, lukewarmOcean, 0, 30, 100);
    setBiomeColour(biomeColours, coldOcean, 20, 20, 80);
    setBiomeColour(biomeColours, warmDeepOcean, 0, 24, 38);
    setBiomeColour(biomeColours, lukewarmDeepOcean, 0, 16, 42);
    setBiomeColour(biomeColours, coldDeepOcean, 16, 16, 40);
    setBiomeColour(biomeColours, frozenDeepOcean, 100, 100, 112);


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


void usage() {
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    --help\n");
    fprintf(stderr, "    --seed=<integer>\n");
    fprintf(stderr, "    --filename=<string>\n");
    fprintf(stderr, "    --width=<integer>\n");
    fprintf(stderr, "    --height=<integer>\n");
    fprintf(stderr, "    --icon_scale=<integer>\n");
    fprintf(stderr, "    --hut_scale=<integer>\n");
    fprintf(stderr, "    --mansion_scale=<integer>\n");
    fprintf(stderr, "    --monument_scale=<integer>\n");
    fprintf(stderr, "    --spawn_scale=<integer>\n");
    fprintf(stderr, "    --stronghold_scale=<integer>\n");
    fprintf(stderr, "    --use_1_13\n");
}


int64_t int64Arg(const char *val, const char *name) {
    char *endptr;
    int64_t ret = strtol(val, &endptr, 10);
    if (errno != 0) {
        fprintf(stderr, "%s must be an integer\n", name);
        usage();
        exit(-1);
    }
    return ret;
}


int intArg(const char *val, const char *name) {
    return (int)int64Arg(val, name);
}


MapOptions parseOptions(int argc, char *argv[]) {
    int c;
    MapOptions opts = {
        .seed          = 0,
        .ppmfn         = "",
        .pngfn         = "",
        .width         = DEFAULT_WIDTH,
        .height        = DEFAULT_HEIGHT,
        .iconScale     = DEFAULT_ICON_SCALE,
        .hutScale      = 0,
        .mansionScale  = 0,
        .monumentScale = 0,
        .spawnScale    = 0,
        .use_1_13      = 0,
    };

    while (1) {
        static struct option longOptions[] = {
            {"help",             no_argument,       NULL, 'h'},
            {"seed",             required_argument, NULL, 's'},
            {"filename",         required_argument, NULL, 'f'},
            {"width",            required_argument, NULL, 'x'},
            {"height",           required_argument, NULL, 'z'},
            {"icon_scale",       required_argument, NULL, 'i'},
            {"hut_scale",        required_argument, NULL, 'H'},
            {"mansion_scale",    required_argument, NULL, 'W'},
            {"monument_scale",   required_argument, NULL, 'M'},
            {"spawn_scale",      required_argument, NULL, 'S'},
            {"stronghold_scale", required_argument, NULL, 'T'},
            {"use_1_13",         no_argument,       NULL, '3'},
        };
        int index = 0;
        c = getopt_long(argc, argv,
                "hs:f:x:z:i:H:W:M:S:T:3", longOptions, &index);
        if (c == -1)
            break;

        switch (c) {
            case 'h':
                usage();
                exit(0);
                break;
            case 's':
                opts.seed = int64Arg(optarg, longOptions[index].name);
                break;
            case 'f':
                if (strlen(optarg) > 250) {
                    fprintf(stderr, "Output filename too long.");
                    exit(-1);
                }
                snprintf(opts.ppmfn, 256, "%s.ppm", optarg);
                snprintf(opts.pngfn, 256, "%s.png", optarg);
                break;
            case 'x':
                opts.width = intArg(optarg, longOptions[index].name);
                break;
            case 'z':
                opts.height = intArg(optarg, longOptions[index].name);
                break;
            case 'i':
                opts.iconScale = intArg(optarg, longOptions[index].name);
                break;
            case 'H':
                opts.hutScale = intArg(optarg, longOptions[index].name);
                break;
            case 'W':
                opts.mansionScale = intArg(optarg, longOptions[index].name);
                break;
            case 'M':
                opts.monumentScale = intArg(optarg, longOptions[index].name);
                break;
            case 'S':
                opts.spawnScale = intArg(optarg, longOptions[index].name);
                break;
            case 'T':
                opts.strongholdScale = intArg(optarg, longOptions[index].name);
                break;
            case '3':
                opts.use_1_13 = 1;
                break;
            default:
                exit(-1);
        }
    }

    if (!opts.seed) {
        fprintf(stderr, "Seed is required (0 is not a valid MC seed).\n");
        usage();
        exit(-1);
    }

    if (!strlen(opts.ppmfn)) {
        fprintf(stderr, "Filename is required.\n");
        usage();
        exit(-1);
    }

    if (!opts.hutScale)
        opts.hutScale = opts.iconScale*2;
    if (!opts.mansionScale)
        opts.mansionScale = opts.iconScale*3;
    if (!opts.monumentScale)
        opts.monumentScale = opts.iconScale*3;
    if (!opts.spawnScale)
        opts.spawnScale = opts.iconScale*2;
    if (!opts.strongholdScale)
        opts.strongholdScale = opts.iconScale*3;

    return opts;
}


void biomesToColors(
        unsigned char biomeColors[256][3],
        int *biomes, unsigned char *pixels, int count) {
    for (int i=0; i<count; i++) {
        if (biomes[i] > 255) {
            fprintf(stderr, "Invalid biome.\n");
            exit(-1);
        }
        int r, g, b;
        if (biomes[i] < 128) {
            r = biomeColors[biomes[i]][0];
            g = biomeColors[biomes[i]][1];
            b = biomeColors[biomes[i]][2];
        } else {
            r = biomeColors[biomes[i]][0] + 40; r = (r>0xff) ? 0xff : r;
            g = biomeColors[biomes[i]][1] + 40; g = (g>0xff) ? 0xff : g;
            b = biomeColors[biomes[i]][2] + 40; b = (b>0xff) ? 0xff : b;
        }
        pixels[i*3+0] = r;
        pixels[i*3+1] = g;
        pixels[i*3+2] = b;
    }
}


void writePPMHeader(FILE *fp, int width, int height) {
    fprintf(fp, "P6\n%d %d\n255\n", width, height);
}


void writeMap(MapOptions opts, LayerStack *g, FILE *fp) {
    unsigned char biomeColors[256][3];
    initBiomeColours(biomeColors);

    Layer *fullRes = &g->layers[g->layerNum-1];
    int *cache = allocCache(fullRes, opts.width, 256);
    unsigned char pixelBuf[256*3];

    writePPMHeader(fp, opts.width, opts.height);

    int left = -opts.width/2;
    for (int top=-opts.height/2; top<opts.height/2; top+=256) {

        int rows = opts.height/2 - top;
        rows = rows > 256 ? 256 : rows;
        genArea(fullRes, cache, left, top, opts.width, rows);

        int pixels = 0;
        while (pixels < opts.width*rows) {
            int toWrite = opts.width*rows - pixels;
            toWrite = toWrite > 256 ? 256 : toWrite;
            biomesToColors(biomeColors, cache+pixels, pixelBuf, toWrite);
            fwrite(pixelBuf, 3, 256, fp);
            pixels += toWrite;
        }
    }

    free(cache);
}


void addIcon(char *icon, int width, int height, Pos pos, int iconWidth, int iconHeight, int scale) {
    int iconW = iconWidth*scale;
    int iconH = iconHeight*scale;
    int realX = pos.x + width/2 - iconW/2;
    int realZ = pos.z + height/2 - iconH/2;

    // Just ignore icons that are off the edge of the map.
    if (realX < 0 || realZ < 0 ||
            realX > width-iconW || realZ > height-iconH)
        return;

    printf("    \\( \"icon/%s.png\" -resize %d00%% \\) -geometry +%d+%d -composite \\\n",
            icon, scale, realX, realZ);
}


int regionify(int val, int chunks) {
    return (int)ceil((float)val / 16 / chunks);
}


int getBiomeAt(const LayerStack *g, const Pos pos, int *buf) {
    genArea(&g->layers[g->layerNum-1], buf, pos.x, pos.z, 1, 1);
    return buf[0];
}


void printCompositeCommand(MapOptions opts, LayerStack *g) {
    Layer *fullRes = &g->layers[g->layerNum-1];
    int *cache = allocCache(fullRes, 256, 256);

    printf("convert \"%s\" -filter Point \\\n", opts.ppmfn);
    Pos pos = getSpawn(g, cache, opts.seed);
    addIcon("spawn", opts.width, opts.height, pos,
            20, 20, opts.spawnScale);

    int rX = regionify(opts.width/2, 32);
    int rZ = regionify(opts.height/2, 32);
    for (int z=-rZ; z<rZ; z++) {
        for (int x=-rX; x<rX; x++) {
            int biomeAt;

            pos = getStructurePos(DESERT_PYRAMID_SEED, opts.seed, x, z);
            biomeAt = getBiomeAt(g, pos, cache);
            if (biomeAt == desert || biomeAt == desertHills)
                addIcon("desert", opts.width, opts.height, pos,
                        19, 20, opts.iconScale);

            pos = getStructurePos(IGLOO_SEED, opts.seed, x, z);
            biomeAt = getBiomeAt(g, pos, cache);
            if (biomeAt == icePlains || biomeAt == coldTaiga)
                addIcon("igloo", opts.width, opts.height, pos,
                        20, 20, opts.iconScale);

            pos = getStructurePos(JUNGLE_PYRAMID_SEED, opts.seed, x, z);
            biomeAt = getBiomeAt(g, pos, cache);
            if (biomeAt == jungle || biomeAt == jungleHills)
                addIcon("jungle", opts.width, opts.height, pos,
                        19, 20, opts.iconScale);

            pos = getStructurePos(SWAMP_HUT_SEED, opts.seed, x, z);
            biomeAt = getBiomeAt(g, pos, cache);
            if (biomeAt == swampland)
                addIcon("witch", opts.width, opts.height, pos,
                        20, 27, opts.hutScale);

            pos = getOceanMonumentPos(opts.seed, x, z);
            if (isViableOceanMonumentPos(*g, cache, pos.x, pos.z))
                addIcon("ocean_monument", opts.width, opts.height, pos,
                        20, 20, opts.monumentScale);
        }
    }

    rX = regionify(opts.width/2, 80);
    rZ = regionify(opts.height/2, 80);
    for (int z=-rZ; z<rZ; z++) {
        for (int x=-rX; x<rX; x++) {
            pos = getMansionPos(opts.seed, x, z);
            if (isViableMansionPos(*g, cache, pos.x, pos.z))
                addIcon("woodland_mansion", opts.width, opts.height, pos,
                        20, 26, opts.mansionScale);
        }
    }

    Pos strongholds[128];
    findStrongholds(g, cache, strongholds, opts.seed, 0);
    for (int i=0; i<128; i++) {
        addIcon("stronghold", opts.width, opts.height, strongholds[i],
                19, 20, opts.strongholdScale);
    }
    // TODO: villages
    printf("    \"%s\"\n", opts.pngfn);

    free(cache);
}


int main(int argc, char *argv[]) {
    MapOptions opts = parseOptions(argc, argv);

    fprintf(stderr, "===========================================================================\n");
    fprintf(stderr, "Writing map for seed %ld...\n", opts.seed);
    fprintf(stderr, "===========================================================================\n");

    FILE *fp = fopen(opts.ppmfn, "w");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file %s for writing.\n", opts.ppmfn);
        exit(-1);
    }

    initBiomes();
    LayerStack g;
    if (opts.use_1_13) {
        g = setupGeneratorMC113();
    } else {
        g = setupGenerator();
    }
    applySeed(&g, opts.seed);

    // Write the base map as a PPM file
    writeMap(opts, &g, fp);

    fclose(fp);

    // Write a imagemagick command to compose the map with icons and convert
    // to a .png file.
    printCompositeCommand(opts, &g);

    return 0;
}
