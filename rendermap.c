#include "biome_util.h"
#include "finders.h"
#include "generator.h"
#include "layers.h"

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_WIDTH 3840*3
#define DEFAULT_HEIGHT DEFAULT_WIDTH*9/16
#define QUAD_SEARCH_RADIUS 10240/16/32

typedef struct {
    int64_t seed;
    char ppmfn[256];
    char pngfn[256];
    int width;
    int height;
    int imageScale;
    int iconScale;
    int hutScale;
    int desertScale;
    int iglooScale;
    int jungleScale;
    int mansionScale;
    int monumentScale;
    int spawnScale;
    int strongholdScale;
    int villageScale;
    int oceanRuinScale;
    int shipwreckScale;
    int use_1_12;
    int oneBiome;
    int highlightSpecial;
    int highlightMutated;
    int highlightSearched;
    int highlightNewOceans;
    int highlightIcons;
    int chunkGrid;
    int centerAtHuts;
} MapOptions;


void usage() {
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    --help\n");
    fprintf(stderr, "    --seed=<integer>\n");
    fprintf(stderr, "    --filename=<string>\n");
    fprintf(stderr, "    --width=<integer>\n");
    fprintf(stderr, "    --height=<integer>\n");
    fprintf(stderr, "    --image_scale=<integer>\n");
    fprintf(stderr, "    --icon_scale=<integer>\n");
    fprintf(stderr, "    --hut_scale=<integer>\n");
    fprintf(stderr, "    --mansion_scale=<integer>\n");
    fprintf(stderr, "    --monument_scale=<integer>\n");
    fprintf(stderr, "    --spawn_scale=<integer>\n");
    fprintf(stderr, "    --stronghold_scale=<integer>\n");
    fprintf(stderr, "    --village_scale=<integer>\n");
    fprintf(stderr, "    --ocean_ruin_scale=<integer>\n");
    fprintf(stderr, "    --shipwreck_scale=<integer>\n");
    fprintf(stderr, "    --use_1_12\n");
    fprintf(stderr, "    --one_biome=<integer>\n");
    fprintf(stderr, "    --highlight_special\n");
    fprintf(stderr, "    --highlight_mutated\n");
    fprintf(stderr, "    --highlight_searched\n");
    fprintf(stderr, "    --highlight_new_oceans\n");
    fprintf(stderr, "    --highlight_icons\n");
    fprintf(stderr, "    --chunk_grid\n");
    fprintf(stderr, "    --center_at_huts\n");
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
        .seed               = 0,
        .ppmfn              = "",
        .pngfn              = "",
        .width              = DEFAULT_WIDTH,
        .height             = DEFAULT_HEIGHT,
        .imageScale         = 1,
        .iconScale          = 1,
        .desertScale        = -1,
        .iglooScale         = -1,
        .jungleScale        = -1,
        .hutScale           = -1,
        .mansionScale       = -1,
        .monumentScale      = -1,
        .spawnScale         = -1,
        .strongholdScale    = -1,
        .villageScale       = -1,
        .oceanRuinScale     = -1,
        .shipwreckScale     = -1,
        .use_1_12           = 0,
        .oneBiome           = -1,
        .highlightSpecial   = 0,
        .highlightMutated   = 0,
        .highlightSearched  = 0,
        .highlightNewOceans = 0,
        .highlightIcons     = 0,
        .chunkGrid          = 0,
        .centerAtHuts       = 0,
    };

    while (1) {
        static struct option longOptions[] = {
            {"help",                 no_argument,       NULL, 'h'},
            {"seed",                 required_argument, NULL, 's'},
            {"filename",             required_argument, NULL, 'f'},
            {"width",                required_argument, NULL, 'x'},
            {"height",               required_argument, NULL, 'z'},
            {"image_scale",          required_argument, NULL, 'X'},
            {"icon_scale",           required_argument, NULL, 'i'},
            {"desert_scale",         required_argument, NULL, 'D'},
            {"igloo_scale",          required_argument, NULL, 'I'},
            {"jungle_scale",         required_argument, NULL, 'J'},
            {"hut_scale",            required_argument, NULL, 'H'},
            {"mansion_scale",        required_argument, NULL, 'W'},
            {"monument_scale",       required_argument, NULL, 'M'},
            {"spawn_scale",          required_argument, NULL, 'S'},
            {"stronghold_scale",     required_argument, NULL, 'T'},
            {"village_scale",        required_argument, NULL, 'V'},
            {"ocean_ruin_scale",     required_argument, NULL, 'O'},
            {"shipwreck_scale",      required_argument, NULL, 'K'},
            {"use_1_12",             no_argument,       NULL, '2'},
            {"one_biome",            required_argument, NULL, '1'},
            {"highlight_special",    no_argument,       NULL, '5'},
            {"highlight_mutated",    no_argument,       NULL, '6'},
            {"highlight_searched",   no_argument,       NULL, '7'},
            {"highlight_new_oceans", no_argument,       NULL, '8'},
            {"highlight_icons",      no_argument,       NULL, '9'},
            {"chunk_grid",           no_argument,       NULL, 'g'},
            {"center_at_huts",       no_argument,       NULL, 'c'},
        };
        int index = 0;
        c = getopt_long(argc, argv,
                "hs:f:x:z:X:i:D:I:H:W:M:S:T:V:O:K:3256789gc", longOptions, &index);
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
            case 'X':
                opts.imageScale = intArg(optarg, longOptions[index].name);
                break;
            case 'i':
                opts.iconScale = intArg(optarg, longOptions[index].name);
                break;
            case 'D':
                opts.desertScale = intArg(optarg, longOptions[index].name);
                break;
            case 'I':
                opts.iglooScale = intArg(optarg, longOptions[index].name);
                break;
            case 'J':
                opts.jungleScale = intArg(optarg, longOptions[index].name);
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
            case 'V':
                opts.villageScale = intArg(optarg, longOptions[index].name);
                break;
            case 'O':
                opts.oceanRuinScale = intArg(optarg, longOptions[index].name);
                break;
            case 'K':
                opts.shipwreckScale = intArg(optarg, longOptions[index].name);
                break;
            case '2':
                opts.use_1_12 = 1;
                break;
            case '1':
                opts.oneBiome = intArg(optarg, longOptions[index].name);
                break;
            case '5':
                opts.highlightSpecial = 1;
                break;
            case '6':
                opts.highlightMutated = 1;
                break;
            case '7':
                opts.highlightSearched = 1;
                break;
            case '8':
                opts.highlightNewOceans = 1;
                break;
            case '9':
                opts.highlightIcons = 1;
                break;
            case 'g':
                opts.chunkGrid = 1;
                break;
            case 'c':
                opts.centerAtHuts = 1;
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

    if (opts.desertScale == -1)
        opts.desertScale = opts.iconScale*2;
    if (opts.iglooScale == -1)
        opts.iglooScale = opts.iconScale*2;
    if (opts.jungleScale == -1)
        opts.jungleScale = opts.iconScale*2;
    if (opts.hutScale == -1)
        opts.hutScale = opts.iconScale*2;
    if (opts.mansionScale == -1)
        opts.mansionScale = opts.iconScale*4;
    if (opts.monumentScale == -1)
        opts.monumentScale = opts.iconScale*2;
    if (opts.spawnScale == -1)
        opts.spawnScale = opts.iconScale*3;
    if (opts.strongholdScale == -1)
        opts.strongholdScale = opts.iconScale*3;
    if (opts.villageScale == -1)
        opts.villageScale = opts.iconScale*2;
    if (opts.oceanRuinScale == -1)
        opts.oceanRuinScale = opts.iconScale*1;
    if (opts.shipwreckScale == -1)
        opts.shipwreckScale = opts.iconScale*1;

    return opts;
}


// Standard values from IEC 61966-2-1
// NOTE: A gamma of 2.2 approximates sRGB, but the actual sRGB curve is a
// piecewise function of a linear and power component. The power component of
// that curve has an expontent of 2.4.
#define SRGB_GAMMA 2.4
#define SRGB_A 0.055
#define SRGB_PHI 12.92
#define SRGB_K0 0.04045

float sRGBToLinear(int c) {
    float c1 = (float)c/255.0;
    float ret;

    if (c1 <= SRGB_K0)
        ret = c1 / SRGB_PHI;
    else
        ret = pow((c1 + SRGB_A) / (1 + SRGB_A), SRGB_GAMMA);

    if (ret < 0.0)
        return 0.0;
    if (ret > 1.0)
        return 1.0;

    return ret;
}


int linearTosRGB(float c) {
    float c1;

    if (c <= SRGB_K0/SRGB_PHI)
        c1 = c*SRGB_PHI;
    else
        c1 = (1 + SRGB_A) * pow(c, 1.0/SRGB_GAMMA) - SRGB_A;

    int ret = (int)round(255.0*c1);

    if (ret < 0)
        return 0;
    if (ret > 255)
        return 255;
    return ret;
}


void biomesToColors(
        MapOptions opts, unsigned char biomeColors[256][3],
        int *biomes, unsigned char *pixels, int left, int z) {
    for (int i=0; i<opts.width; i++) {
        if (biomes[i] > 255) {
            fprintf(stderr, "Invalid biome.\n");
            exit(-1);
        }

        int r, g, b;
        int x = left + i;
        int id = biomes[i];
        if (id < 128) {
            r = biomeColors[id][0];
            g = biomeColors[id][1];
            b = biomeColors[id][2];
        } else {
            r = biomeColors[id][0] + 40; r = (r>0xff) ? 0xff : r;
            g = biomeColors[id][1] + 40; g = (g>0xff) ? 0xff : g;
            b = biomeColors[id][2] + 40; b = (b>0xff) ? 0xff : b;
        }

        if (opts.highlightSpecial || opts.highlightSearched ||
                opts.highlightMutated || opts.highlightNewOceans ||
                opts.highlightIcons) {
            int highlighted = 0;
            if ((opts.highlightSpecial || opts.highlightSearched) && (
                        id == jungle || id == jungleHills || id == jungleEdge ||
                        id == jungle+128 || id == jungleEdge+128 ||
                        id == megaTaiga || id == megaTaigaHills ||
                        id == megaTaiga+128 || id == megaTaigaHills+128 ||
                        id == mesa || id == mesaPlateau_F ||
                        id == mesaPlateau || id == mesa+128 ||
                        id == mesaPlateau_F+128 || id == mesaPlateau+128 ||
                        id == mushroomIsland || id == mushroomIslandShore))
                highlighted = 1;

            if (opts.highlightMutated && id >= 128)
                highlighted = 1;

            if (opts.highlightSearched && (
                        id == forest+128 || id == plains+128 ||
                        id == icePlains+128 || id == mesa+128))
                highlighted = 1;

            if (opts.highlightNewOceans && (
                        id == frozenOcean || id == frozenDeepOcean ||
                        id == coldOcean || id == coldDeepOcean ||
                        id == lukewarmOcean || id == lukewarmDeepOcean ||
                        id == warmOcean || id == warmDeepOcean))
                highlighted = 1;

            if (!highlighted) {
                // I think I'm probably a tool for making this colometrically
                // correct. I should probably make the multiplier a command
                // line option.
                float fr = sRGBToLinear(r);
                float fg = sRGBToLinear(g);
                float fb = sRGBToLinear(b);
                float a = fr + fg + fb;
                r = linearTosRGB((fr + 2*a) * 0.005);
                g = linearTosRGB((fg + 2*a) * 0.005);
                b = linearTosRGB((fb + 2*a) * 0.005);
            }
        }

        if (opts.chunkGrid) {
            int chunkx = x >> 4;  // Acts like a flooring div by 16. Regular
            int chunkz = z >> 4;  // divide is toward zero. Bit shift is signed.
            int offx = chunkx & 31;
            int offz = chunkz & 31;
            float offr, offg, offb;

            if (offx < 24 && offz < 24) {
                // Witch hut colors
                offr = 0.35; offg = 0.20; offb = 0.40;
            } else if (offx < 27 && offz < 27) {
                // Ocean monument colors
                offr = 0.25; offg = 0.45; offb = 0.55;
            } else {
                // Nothing colors
                offr = 0.15; offg = 0.15; offb = 0.15;
            }

            if ((chunkx + chunkz) & 1) {
                offr *= 0.5; offg *= 0.5; offb *= 0.5;
            }

            r = linearTosRGB(sRGBToLinear(r) * 0.1 + offr * 0.9);
            g = linearTosRGB(sRGBToLinear(g) * 0.1 + offg * 0.9);
            b = linearTosRGB(sRGBToLinear(b) * 0.1 + offb * 0.9);
        }

        pixels[i*3+0] = r;
        pixels[i*3+1] = g;
        pixels[i*3+2] = b;
    }
}


void writePPMHeader(FILE *fp, int width, int height) {
    fprintf(fp, "P6\n%d %d\n255\n", width, height);
}


static inline int min(int a, int b) {
    return a < b ? a : b;
}


static inline int max(int a, int b) {
    return a > b ? a : b;
}


static inline int dist(Pos spawn, int x, int z) {
    int dx = spawn.x - x;
    int dz = spawn.z - z;
    return (int)round(sqrt(dx*dx + dz*dz));
}


Pos findQuadRegions(int64_t seed, int radius) {
    int rX, rZ;
    for (rZ = -radius-1; rZ < radius; rZ++) {
        for (rX = -radius-1; rX < radius; rX++) {
            int64_t translated = moveStructure(seed, -rX, -rZ);
            if (isQuadBase(
                    SWAMP_HUT_CONFIG, translated, 1))
                return (Pos){rX, rZ};
        }
    }
    return (Pos){0, 0};
}


Pos findCenterFromRegion(Pos quad) {
    return (Pos){((quad.x+1) * 32 - 4) * 16, ((quad.z+1) * 32 - 4) * 16};
}


Pos findQuadCenter(int64_t seed, int radius) {
    Pos quad = findQuadRegions(seed, radius);
    return findCenterFromRegion(quad);
}


void writeMap(MapOptions opts, LayerStack *g, FILE *fp) {
    unsigned char biomeColors[256][3];
    initBiomeColours(biomeColors);

    Layer *fullRes = &g->layers[g->layerNum-1];
    int *cache = allocCache(fullRes, opts.width, 256);
    unsigned char pixelBuf[opts.width*3];
    Pos spawn = getSpawn((opts.use_1_12 ? MC_1_12 : MC_1_13), g, cache, opts.seed);

    int distances[256];
    for (int i=0; i<256; i++) distances[i] = INT_MAX;

    writePPMHeader(fp, opts.width, opts.height);

    Pos center = {0, 0};
    Pos distfrom = spawn;
    if (opts.centerAtHuts) {
        center = findQuadCenter(opts.seed, QUAD_SEARCH_RADIUS);
        distfrom = center;
    }

    int left = center.x - opts.width/2;
    int minZ = center.z - opts.height/2;
    int maxZ = center.z + opts.height/2;
    for (int top=minZ; top<maxZ; top+=256) {
        int rows = maxZ - top;
        rows = rows > 256 ? 256 : rows;
        genArea(fullRes, cache, left, top, opts.width, rows);

        for (int row=0; row < rows; row++) {
            int z = top + row;
            for (int i=0; i<opts.width; i++) {
                int b = cache[row*opts.width+i];
                if (b < 256)
                    distances[b] = min(distances[b], dist(distfrom, i+left, z));
                else
                    fprintf(stderr, "INVALID BIOME!");
            }

            biomesToColors(opts, biomeColors, cache+row*opts.width, pixelBuf, left, z);
            fwrite(pixelBuf, 3, opts.width, fp);
        }
    }

    free(cache);

    fprintf(stderr, "Distances to biomes:\n");
    for (int i=0; i<256; i++) {
        if (distances[i] < INT_MAX)
            fprintf(stderr, "    %23s: %5d\n", biomeNames[i], distances[i]);
    }
    fprintf(stderr, "======================================="
            "======================================\n");
}


int addIcon(char *icon, int width, int height, int imageScale, Pos center, Pos pos,
        int iconWidth, int iconHeight, int scale) {

    // Setting scale to 0 can be used to hide an icon category.
    if (!scale)
        return 0;

    int iconW = iconWidth*scale;
    int iconH = iconHeight*scale;
    int realX = imageScale * (pos.x - center.x + width/2) - iconW/2;
    int realZ = imageScale * (pos.z - center.z + height/2) - iconH/2;

    // Just ignore icons that are off the edge of the map.
    if (realX < -iconW || realZ < -iconH ||
            realX > width*imageScale || realZ > height*imageScale)
        return 0;

    printf("    \\( \"icon/%s.png\" -resize %d00%% \\) "
            "-geometry +%d+%d -composite \\\n",
            icon, scale, realX, realZ);

    return 1;
}


void regionify(Pos center, int width, int height, int regionSize, Pos *tl, Pos *br) {
    tl->x = (int)floor((float)(center.x - width/2) / 16 / regionSize);
    tl->z = (int)floor((float)(center.z - height/2) / 16 / regionSize);
    br->x = (int)ceil((float)(center.x + width/2) / 16 / regionSize);
    br->z = (int)ceil((float)(center.z + height/2) / 16 / regionSize);
}


int getBiomeAt(const LayerStack *g, const Pos pos, int *buf) {
    genArea(&g->layers[g->layerNum-1], buf, pos.x, pos.z, 1, 1);
    return buf[0];
}


void printCompositeCommand(MapOptions opts, LayerStack *g) {
    Layer *fullRes = &g->layers[g->layerNum-1];
    int *cache = allocCache(fullRes, 256, 256);

    Pos center = {0, 0};
    Pos huts = findQuadRegions(opts.seed, QUAD_SEARCH_RADIUS);
    Pos hutCenter = findCenterFromRegion(huts);
    if (opts.centerAtHuts) {
        center = hutCenter;
    }

    fprintf(stderr, "Interesting structures:\n");
    fprintf(stderr, "     Quad hut center: %6d, %6d\n", hutCenter.x, hutCenter.z);

    printf("convert \"%s\" -filter Point \\\n", opts.ppmfn);
    if (opts.imageScale != 1)
        printf("    -resize %d00%% \\\n", opts.imageScale);
    Pos spawn = getSpawn((opts.use_1_12 ? MC_1_12 : MC_1_13), g, cache, opts.seed);
    fprintf(stderr, "               Spawn: %6d, %6d\n", spawn.x, spawn.z);
    addIcon("spawn", opts.width, opts.height, opts.imageScale, center, spawn,
            20, 20, opts.spawnScale);

    StructureConfig desertPyramid, igloo, junglePyramid, swampHut;
    if (opts.use_1_12) {
        desertPyramid = igloo = junglePyramid = swampHut = FEATURE_CONFIG;
    } else {
        desertPyramid = DESERT_PYRAMID_CONFIG;
        igloo = IGLOO_CONFIG;
        junglePyramid = JUNGLE_PYRAMID_CONFIG;
        swampHut = SWAMP_HUT_CONFIG;
    }

    Pos tl, br;
    regionify(center, opts.width, opts.height, FEATURE_CONFIG.regionSize, &tl, &br);
    int biomeAt;
    Pos pos;
    for (int z=tl.z; z<=br.z; z++) {
        for (int x=tl.x; x<=br.x; x++) {
            pos = getLargeStructurePos(MONUMENT_CONFIG, opts.seed, x, z);
            if (isViableOceanMonumentPos(*g, cache, pos.x, pos.z)) {
                addIcon("ocean_monument", opts.width, opts.height, opts.imageScale, center, pos,
                        20, 20, opts.monumentScale);
                if ((z == huts.z || z == huts.z+1) && (x == huts.x || x == huts.x+1)) {
                    fprintf(stderr, "     Nearby monument: %6d, %6d (%d, %2d, %2d)\n",
                         pos.x, pos.z, dist(spawn, pos.x, pos.z),
                         (pos.x>>4) & 31, (pos.z>>4) & 31);
                }
            }

            pos = getStructurePos(VILLAGE_CONFIG, opts.seed, x, z);
            if (isViableVillagePos(*g, cache, pos.x, pos.z))
                addIcon("village", opts.width, opts.height, opts.imageScale, center, pos,
                        20, 26, opts.villageScale);

            pos = getStructurePos(desertPyramid, opts.seed, x, z);
            biomeAt = getBiomeAt(g, pos, cache);
            if (biomeAt == desert || biomeAt == desertHills)
                addIcon("desert", opts.width, opts.height, opts.imageScale, center, pos,
                        20, 20, opts.desertScale);

            pos = getStructurePos(igloo, opts.seed, x, z);
            biomeAt = getBiomeAt(g, pos, cache);
            if (biomeAt == icePlains || biomeAt == coldTaiga)
                addIcon("igloo", opts.width, opts.height, opts.imageScale, center, pos,
                        20, 20, opts.iglooScale);

            pos = getStructurePos(junglePyramid, opts.seed, x, z);
            biomeAt = getBiomeAt(g, pos, cache);
            if (biomeAt == jungle || biomeAt == jungleHills)
                addIcon("jungle", opts.width, opts.height, opts.imageScale, center, pos,
                        20, 20, opts.jungleScale);

            pos = getStructurePos(swampHut, opts.seed, x, z);
            biomeAt = getBiomeAt(g, pos, cache);
            if (biomeAt == swampland) {
                addIcon("witch", opts.width, opts.height, opts.imageScale, center, pos,
                        20, 26, opts.hutScale);
                if ((z == huts.z || z == huts.z+1) && (x == huts.x || x == huts.x+1)) {
                    fprintf(stderr, "      Quad witch hut: %6d, %6d (%d, %2d, %2d)\n",
                         pos.x, pos.z, dist(spawn, pos.x, pos.z),
                         (pos.x>>4) & 31, (pos.z>>4) & 31);
                }
            }
        }
    }

    if (!opts.use_1_12) {
        regionify(center, opts.width, opts.height, OCEAN_RUIN_CONFIG.regionSize, &tl, &br);
        for (int z=tl.z; z<=br.z; z++) {
            for (int x=tl.x; x<=br.x; x++) {
                pos = getStructurePos(OCEAN_RUIN_CONFIG, opts.seed, x, z);
                biomeAt = getBiomeAt(g, pos, cache);
                if (isOceanic(biomeAt))
                    addIcon("ocean_ruins", opts.width, opts.height, opts.imageScale, center, pos,
                            20, 20, opts.oceanRuinScale);
            }
        }

        regionify(center, opts.width, opts.height, SHIPWRECK_CONFIG.regionSize, &tl, &br);
        for (int z=tl.z; z<=br.z; z++) {
            for (int x=tl.x; x<=br.x; x++) {
                pos = getStructurePos(SHIPWRECK_CONFIG, opts.seed, x, z);
                biomeAt = getBiomeAt(g, pos, cache);
                if (isOceanic(biomeAt))
                    addIcon("shipwreck", opts.width, opts.height, opts.imageScale, center, pos,
                            20, 20, opts.shipwreckScale);
            }
        }
    }

    regionify(center, opts.width, opts.height, MANSION_CONFIG.regionSize, &tl, &br);
    for (int z=tl.z; z<=br.z; z++) {
        for (int x=tl.x; x<=br.x; x++) {
            pos = getLargeStructurePos(MANSION_CONFIG, opts.seed, x, z);
            if (isViableMansionPos(*g, cache, pos.x, pos.z)) {
                addIcon("woodland_mansion", opts.width, opts.height, opts.imageScale, center, pos,
                        20, 26, opts.mansionScale);
                fprintf(stderr, "    Woodland mansion: %6d, %6d (%d)\n",
                        pos.x, pos.z, dist(spawn, pos.x, pos.z));
            }
        }
    }

    Pos strongholds[128];
    findStrongholds((opts.use_1_12 ? MC_1_12 : MC_1_13), g, cache, strongholds, opts.seed, 0, 0);
    for (int i=0; i<128; i++) {
        pos = strongholds[i];
        if (addIcon("stronghold", opts.width, opts.height, opts.imageScale, center, pos,
                20, 20, opts.strongholdScale)) {
            fprintf(stderr, "          Stronghold: %6d, %6d (%d)\n",
                    pos.x, pos.z, dist(spawn, pos.x, pos.z));
        }
    }
    printf("    \"%s\"\n", opts.pngfn);

    fprintf(stderr, "======================================="
            "======================================\n");

    free(cache);
}


void mapFake(Layer *l, int* __restrict out, int areaX, int areaZ, int areaWidth, int areaHeight) {
    for (int i=0; i<areaWidth*areaHeight; i++) {
        out[i] = l->baseSeed;
    }
}


LayerStack setupFakeGenerator(int biome) {
    LayerStack g = setupGeneratorMC17();

    for (int i=0; i<g.layerNum; i++) {
        g.layers[i].baseSeed = biome;
        g.layers[i].getMap = mapFake;
    }

    return g;
}


int main(int argc, char *argv[]) {
    MapOptions opts = parseOptions(argc, argv);

    fprintf(stderr, "======================================="
            "======================================\n");
    fprintf(stderr, "Writing %dx%d map for seed %"PRId64"...\n", opts.width, opts.height, opts.seed);
    fprintf(stderr, "======================================="
            "======================================\n");

    FILE *fp = fopen(opts.ppmfn, "w");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file %s for writing.\n", opts.ppmfn);
        exit(-1);
    }

    initBiomes();
    LayerStack g;
    if (opts.oneBiome != -1) {
        g = setupFakeGenerator(opts.oneBiome);
    } else if (opts.use_1_12) {
        g = setupGeneratorMC17();
    } else {
        g = setupGeneratorMC113();
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
