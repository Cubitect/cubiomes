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
#include <time.h>

const int base_tile_size = 16;
const int min_tile_size = 16;

int totalTiles = 0;
int completedTiles = 0;
time_t startTime;

int getTileSize(int zoomLevel) {
    int tileSize = base_tile_size >> zoomLevel;
    if (tileSize < min_tile_size) {
        return min_tile_size;
    }
    return tileSize;
}

int createDir(const char *path) {
    char tmp[2048];
    char *p;

    snprintf(tmp, sizeof(tmp), "%s", path);

    if (tmp[0] == '/') {
        p = tmp + 1;
    } else {
        p = tmp;
    }

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

    if (mkdir(tmp, 0777) && errno != EEXIST) {
        fprintf(stderr, "Error creating directory %s: %s\n", tmp, strerror(errno));
        return -1;
    }

    return 0;
}

void generateTile(Generator *g, uint64_t seed, int tileX, int tileY, int tileSize, const char *outputDir, int zoomLevel) {
    setupGenerator(g, MC_1_18, LARGE_BIOMES);
    applySeed(g, DIM_OVERWORLD, seed);

    Range r;
    r.scale = 4;
    r.x = tileX * tileSize;
    r.z = tileY * tileSize;
    r.sx = tileSize;
    r.sz = tileSize;
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

    char zoomDir[2048];
    snprintf(zoomDir, sizeof(zoomDir), "%s/%lu/%d", outputDir, seed, zoomLevel);

    if (createDir(zoomDir) != 0) {
        free(biomeIds);
        free(rgb);
        return;
    }

    char tileDir[4096];
    snprintf(tileDir, sizeof(tileDir), "%s/%d", zoomDir, tileX);

    if (createDir(tileDir) != 0) {
        free(biomeIds);
        free(rgb);
        return;
    }

    char outputFile[8096];
    snprintf(outputFile, sizeof(outputFile), "%s/%d.png", tileDir, tileY);

    if (savePNG(outputFile, rgb, imgWidth, imgHeight) != 0) {
        fprintf(stderr, "Error saving image file for tile %d_%d at zoom level %d\n", tileX, tileY, zoomLevel);
    } else {
        completedTiles++;
        printf("Tile %d of %d generated and saved to %s\n", completedTiles, totalTiles, outputFile);

        time_t currentTime = time(NULL);
        double elapsedSeconds = difftime(currentTime, startTime);
        double estimatedTotalTime = (elapsedSeconds / completedTiles) * totalTiles;
        double timeRemaining = estimatedTotalTime - elapsedSeconds;

        printf("Estimated time remaining: %.2f seconds\n", timeRemaining);
    }

    free(biomeIds);
    free(rgb);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <seed> <zoom_level> <tile_count>\n", argv[0]);
        return 1;
    }

    uint64_t seed = strtoull(argv[1], NULL, 10);
    int zoomLevel = atoi(argv[2]);
    int tileCount = atoi(argv[3]);

    if (zoomLevel < 0) {
        fprintf(stderr, "Zoom level must be non-negative\n");
        return 1;
    }

    int tileSize = getTileSize(zoomLevel);

    totalTiles = tileCount * tileCount;
    printf("Total tiles to be generated: %d\n", totalTiles);

    startTime = time(NULL);

    char outputDir[2048];
    snprintf(outputDir, sizeof(outputDir), "/var/www/storage/app/public/tiles");

    if (createDir(outputDir) != 0) {
        return 1;
    }

    Generator g;
    setupGenerator(&g, MC_1_18, LARGE_BIOMES);

    // Initialize spiral parameters
    int x = tileCount;  // Start at specified tileCount as the starting X coordinate
    int y = tileCount;  // Start at specified tileCount as the starting Y coordinate
    int dx = 0;
    int dy = -1;
    int maxIters = tileCount * tileCount;
    int segmentLength = 1;
    int segmentPassed = 0;
    int segmentsToPass = 1;
    int turnsMade = 0;

    for (int i = 0; i < maxIters; ++i) {
        if (x >= 0 && x < totalTiles && y >= 0 && y < totalTiles) {
            generateTile(&g, seed, x, y, tileSize, outputDir, zoomLevel);
        }

        // Move to the next point in the spiral
        x += dx;
        y += dy;
        segmentPassed++;

        if (segmentPassed == segmentsToPass) {
            // Change direction
            int temp = dx;
            dx = -dy;
            dy = temp;

            segmentPassed = 0;
            turnsMade++;

            if (turnsMade % 2 == 0) {
                segmentsToPass++;
            }
        }
    }

    time_t endTime = time(NULL);
    double totalTime = difftime(endTime, startTime);
    printf("All tiles generated. Total time taken: %.2f seconds\n", totalTime);

    return 0;
}
