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

const int tileSize = 32; // Fixed tile size (32x32 blocks)
const int numTiles = 100; // Number of tiles to generate

// Function to calculate the number of tiles needed based on viewport size and tile size
void calculateTileDimensions(int viewportWidth, int viewportHeight, int tileSize, int *tilesX, int *tilesY) {
    *tilesX = (viewportWidth + tileSize - 1) / tileSize; // Number of tiles in X direction
    *tilesY = (viewportHeight + tileSize - 1) / tileSize; // Number of tiles in Y direction
}

int createDir(const char *path) {
    char tmp[2048];
    char *p;
    
    // Copy the path to avoid modifying the original string
    snprintf(tmp, sizeof(tmp), "%s", path);

    // Handle the root directory if present
    if (tmp[0] == '/') {
        p = tmp + 1;
    } else {
        p = tmp;
    }

    // Create each directory in the path
    for (; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0777) && errno != EEXIST) {
                fprintf(stderr, "Error creating directory %s: %s\n", tmp, strerror(errno));
                return -1;
            }
            *p = '/';
        }
    }

    // Create the final directory
    if (mkdir(tmp, 0777) && errno != EEXIST) {
        fprintf(stderr, "Error creating directory %s: %s\n", tmp, strerror(errno));
        return -1;
    }

    return 0;
}

void generateTile(Generator *g, uint64_t seed, int tileX, int tileY, int tileSize, const char *outputDir) {
    setupGenerator(g, MC_1_18, LARGE_BIOMES);
    applySeed(g, DIM_OVERWORLD, seed);

    Range r;
    r.scale = 4;
    r.x = tileX * tileSize; // Position of the tile in the world
    r.z = tileY * tileSize; // Position of the tile in the world
    r.sx = tileSize; // Width of the tile in blocks
    r.sz = tileSize; // Height of the tile in blocks
    r.y = 15; // Height is not relevant in 2D maps; you might set it to a constant or ignore
    r.sy = 1;  // Not used in 2D maps, but keep if it affects your implementation

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

    // Construct the directory path
    char tileDir[4096];
    snprintf(tileDir, sizeof(tileDir), "%s/%lu", outputDir, seed);

    printf("Creating tile directory: %s\n", tileDir);

    // Create the tile directory if it does not exist
    if (createDir(tileDir) != 0) {
        free(biomeIds);
        free(rgb);
        return;
    }

    // Construct the output file path
    char outputFile[8096];
    snprintf(outputFile, sizeof(outputFile), "%s/%d_%d.png", tileDir, tileX, tileY);

    printf("Saving file to: %s\n", outputFile);

    if (savePNG(outputFile, rgb, imgWidth, imgHeight) != 0) {
        fprintf(stderr, "Error saving image file for tile %d_%d\n", tileX, tileY);
    } else {
        printf("Tile map generated and saved to %s\n", outputFile);
    }

    free(biomeIds);
    free(rgb);
}

void generateTiles(uint64_t seed, const char *outputDir) {
    Generator g;
    setupGenerator(&g, MC_1_18, LARGE_BIOMES);

    // Example coordinates for generating tiles in all four quadrants
    int startX = -5; // Start generating from -5 to 5 (example)
    int startY = -5; // Start generating from -5 to 5 (example)
    int tileCount = 0;

    for (int x = startX; x <= 5 && tileCount < numTiles; ++x) {
        for (int y = startY; y <= 5 && tileCount < numTiles; ++y) {
            generateTile(&g, seed, x, y, tileSize, outputDir);
            tileCount++;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <seed>\n", argv[0]);
        return 1;
    }

    uint64_t seed = strtoull(argv[1], NULL, 10);

    char outputDir[2048];
    // snprintf(outputDir, sizeof(outputDir), "/var/www/storage/app/public/tiles");
    snprintf(outputDir, sizeof(outputDir), "/var/www/gme-backend/storage/app/public/tiles");

    if (createDir(outputDir) != 0) {
        return 1;
    }

    generateTiles(seed, outputDir);

    return 0;
}
