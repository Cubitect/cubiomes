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
const int MAX_TILES = 200; // Number of tiles to generate

// Function to determine tile size based on zoom level
int getTileSize(int zoomLevel) {
    return chunk_size << zoomLevel; // Tile size increases exponentially with zoom level
}

// Function to calculate the number of tiles needed based on viewport size and tile size
void calculateTileDimensions(int viewportWidth, int viewportHeight, int tileSize, int *tilesX, int *tilesY) {
    *tilesX = (viewportWidth + tileSize - 1) / tileSize; // Number of tiles in X direction
    *tilesY = (viewportHeight + tileSize - 1) / tileSize; // Number of tiles in Y direction
}

// Function to check if a file exists
int fileExists(const char *path) {
    return access(path, F_OK) != -1;
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

void generateTile(Generator *g, uint64_t seed, int tileX, int tileY, int tileSize, int viewportWidth, int viewportHeight, const char *outputDir, int zoomLevel) {
    // Construct the directory path including zoom level
    char zoomDir[2048];
    snprintf(zoomDir, sizeof(zoomDir), "%s/%lu/%d", outputDir, seed, zoomLevel);

    // Construct the final directory path for the tile
    char tileDir[4096];
    snprintf(tileDir, sizeof(tileDir), "%s/%d", zoomDir, tileX);

    // Construct the output file path
    char outputFile[8096];
    snprintf(outputFile, sizeof(outputFile), "%s/%d.png", tileDir, tileY);

    // Check if the tile file already exists
    if (fileExists(outputFile)) {
        printf("Tile file already exists: %s\n", outputFile);
        return;
    }

    setupGenerator(g, MC_1_18, LARGE_BIOMES);
    applySeed(g, DIM_OVERWORLD, seed);

    Range r;
    r.scale = 4;
    r.x = (tileX - viewportWidth / 2) * tileSize; // Adjust for centering
    r.z = (tileY - viewportHeight / 2) * tileSize; // Adjust for centering
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

    printf("Creating directory: %s\n", zoomDir);

    // Create the directory if it does not exist
    if (createDir(zoomDir) != 0) {
        free(biomeIds);
        free(rgb);
        return;
    }

    printf("Creating tile directory: %s\n", tileDir);

    // Create the tile directory if it does not exist
    if (createDir(tileDir) != 0) {
        free(biomeIds);
        free(rgb);
        return;
    }

    printf("Saving file to: %s\n", outputFile);

    if (savePNG(outputFile, rgb, imgWidth, imgHeight) != 0) {
        fprintf(stderr, "Error saving image file for tile %d_%d at zoom level %d\n", tileX, tileY, zoomLevel);
    } else {
        printf("Tile map generated and saved to %s\n", outputFile);
    }

    free(biomeIds);
    free(rgb);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <seed> <zoom_level> <pan_x> <pan_y>\n", argv[0]);
        return 1;
    }

    uint64_t seed = strtoull(argv[1], NULL, 10);
    int zoomLevel = atoi(argv[2]);
    int pan_x = atoi(argv[3]);
    int pan_y = atoi(argv[4]);

    if (zoomLevel < 0) {
        fprintf(stderr, "Zoom level must be non-negative\n");
        return 1;
    }

    int tileSize = getTileSize(zoomLevel);

    const int viewportWidth = 1920;
    const int viewportHeight = 1240;

    int tilesX, tilesY;
    calculateTileDimensions(viewportWidth, viewportHeight, tileSize, &tilesX, &tilesY);

    // Calculate the starting point to center the tiles around pan_x and pan_y
    int startX = pan_x - tilesX / 2;
    int startY = pan_y - tilesY / 2;

    char outputDir[2048];
    snprintf(outputDir, sizeof(outputDir), "/var/www/storage/app/public/tiles");

    if (createDir(outputDir) != 0) {
        return 1;
    }

    Generator g;
    setupGenerator(&g, MC_1_18, LARGE_BIOMES);

    int tileCounter = 0;
    for (int x = startX; tileCounter < MAX_TILES && x < startX + tilesX; ++x) {
        for (int y = startY; tileCounter < MAX_TILES && y < startY + tilesY; ++y) {
            generateTile(&g, seed, x, y, tileSize, viewportWidth, viewportHeight, outputDir, zoomLevel);
            tileCounter++;
        }
    }

    return 0;
}
