#include "generator.h"
#include "util.h"
#include "image_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

const int chunk_size = 16; // Minecraft chunk size (16x16 blocks)
const int viewport_width = 1920;
const int viewport_height = 1240;

int createDir(const char *path) {
    char tmp[2048];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0777) && errno != EEXIST) {
                fprintf(stderr, "Error creating directory %s: %s\n", tmp, strerror(errno));
                return -1;
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, 0777) && errno != EEXIST) {
        fprintf(stderr, "Error creating directory %s: %s\n", tmp, strerror(errno));
        return -1;
    }
    return 0;
}

void generateTile(Generator *g, uint64_t seed, int tileX, int tileZ, int tileSize, const char *outputDir) {
    setupGenerator(g, MC_1_18, LARGE_BIOMES);
    applySeed(g, DIM_OVERWORLD, seed);

    Range r;
    r.scale = 4;
    r.x = tileX * chunk_size; // Multiply by chunk size to get correct position in blocks
    r.z = tileZ * chunk_size;
    r.sx = tileSize * chunk_size;
    r.sz = tileSize * chunk_size;
    r.y = 15;
    r.sy = 1;

    int *biomeIds = allocCache(g, r);
    genBiomes(g, biomeIds, r);

    int pix4cell = 4;
    int imgWidth = pix4cell * r.sx;
    int imgHeight = pix4cell * r.sz;

    unsigned char biomeColors[256][3];
    initBiomeColors(biomeColors);

    unsigned char *rgb = (unsigned char *)malloc(3 * imgWidth * imgHeight);
    if (rgb == NULL) {
        fprintf(stderr, "Error allocating memory for image\n");
        free(biomeIds);
        return;
    }

    biomesToImage(rgb, biomeColors, biomeIds, r.sx, r.sz, pix4cell, 2);

    char seedDir[2048];
    snprintf(seedDir, sizeof(seedDir), "%s/%lu", outputDir, seed);

    if (createDir(seedDir) != 0) {
        free(biomeIds);
        free(rgb);
        return;
    }

    char outputFile[4096];
    snprintf(outputFile, sizeof(outputFile), "%s/%d_%d.png", seedDir, tileX, tileZ);

    if (savePNG(outputFile, rgb, imgWidth, imgHeight) != 0) {
        fprintf(stderr, "Error saving image file for tile %d_%d\n", tileX, tileZ);
    } else {
        printf("Tile map generated and saved to %s\n", outputFile);
    }

    free(biomeIds);
    free(rgb);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <seed>\n", argv[0]);
        return 1;
    }

    uint64_t seed = strtoull(argv[1], NULL, 10);
    
    // Default values based on viewport and chunk size
    int tileSize = 1; // Represents one chunk (16x16 blocks)
    int tilesX = (viewport_width + chunk_size - 1) / chunk_size; // Number of tiles in X direction
    int tilesZ = (viewport_height + chunk_size - 1) / chunk_size; // Number of tiles in Z direction

    // Create output directory based on seed
    char outputDir[2048];
    // snprintf(outputDir, sizeof(outputDir), "/var/www/storage/app/public/tiles");
    snprintf(outputDir, sizeof(outputDir), "/var/www/gme-backend/storage/app/public/tiles");

    if (createDir(outputDir) != 0) {
        return 1;
    }

    Generator g;
    setupGenerator(&g, MC_1_18, LARGE_BIOMES);

    // Generate all tiles necessary to cover the viewport
    for (int x = 0; x < tilesX; ++x) {
        for (int z = 0; z < tilesZ; ++z) {
            generateTile(&g, seed, x, z, tileSize, outputDir);
        }
    }

    return 0;
}
