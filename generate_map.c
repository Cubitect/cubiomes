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
#include <pthread.h>

int totalTiles = 0;
int completedTiles = 0;
time_t startTime;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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

void generateTile(Generator *g, uint64_t seed, int tileX, int tileY, int tileSize, const char *outputDir, int zoomLevel, int scale) {
    setupGenerator(g, MC_1_18, LARGE_BIOMES);
    applySeed(g, DIM_OVERWORLD, seed);

    Range r;
    r.scale = scale;
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
        pthread_mutex_lock(&mutex);
        completedTiles++;
        printf("Tile %d of %d generated and saved to %s\n", completedTiles, totalTiles, outputFile);

        time_t currentTime = time(NULL);
        double elapsedSeconds = difftime(currentTime, startTime);
        double estimatedTotalTime = (elapsedSeconds / completedTiles) * totalTiles;
        double timeRemaining = estimatedTotalTime - elapsedSeconds;

        printf("Estimated time remaining: %.2f seconds\n", timeRemaining);
        pthread_mutex_unlock(&mutex);
    }

    free(biomeIds);
    free(rgb);
}

typedef struct {
    uint64_t seed;
    const char *outputDir;
    int zoomLevel;
    int scale;
    int base_tile_size;
    int tile_count;
} ZoomLevelParams;

void *generateTilesForZoomLevel(void *arg) {
    ZoomLevelParams *params = (ZoomLevelParams *)arg;
    int zoomLevel = params->zoomLevel;
    int scale = params->scale;
    int base_tile_size = params->base_tile_size;
    int tile_count = params->tile_count;
    uint64_t seed = params->seed;
    const char *outputDir = params->outputDir;

    int tileSize = base_tile_size;

    printf("Generating tiles for zoom level %d with tile size %d, scale %d, and tile count %d\n", zoomLevel, tileSize, scale, tile_count);

    Generator g;
    setupGenerator(&g, MC_1_18, LARGE_BIOMES);

    // Initialize spiral parameters
    int centerX = tile_count / 2;
    int centerY = tile_count / 2;
    int x = centerX;
    int y = centerY;
    int dx = 0;
    int dy = -1;
    int segmentLength = 1;
    int segmentPassed = 0;
    int turnsMade = 0;

    for (int i = 0; i < (2 * centerX + 1) * (2 * centerY + 1); ++i) {
        // Generate the tile only if within the defined range
        if (x >= 0 && x < tile_count && y >= 0 && y < tile_count) {
            generateTile(&g, seed, x, y, tileSize, outputDir, zoomLevel, scale);
        }

        // Move to the next point in the spiral
        x += dx;
        y += dy;
        segmentPassed++;

        if (segmentPassed == segmentLength) {
            // Change direction
            int temp = dx;
            dx = -dy;
            dy = temp;

            segmentPassed = 0;
            turnsMade++;

            if (turnsMade % 2 == 0) {
                segmentLength++;
            }
        }
    }

    pthread_exit(NULL);
}

void generateTilesForZoomLevels(uint64_t seed, const char *outputDir) {
    // Array of zoom level parameters
    ZoomLevelParams zoomLevels[] = {
        {seed, outputDir, 3, 96, 128, 8},
        {seed, outputDir, 4, 48, 128, 16},
        {seed, outputDir, 5, 24, 128, 32},
        {seed, outputDir, 6, 12, 128, 32},
    };

    int numZoomLevels = sizeof(zoomLevels) / sizeof(zoomLevels[0]);

    totalTiles = 0;
    for (int i = 0; i < numZoomLevels; i++) {
        totalTiles += zoomLevels[i].tile_count * zoomLevels[i].tile_count;
    }

    pthread_t threads[numZoomLevels];
    for (int i = 0; i < numZoomLevels; i++) {
        if (pthread_create(&threads[i], NULL, generateTilesForZoomLevel, (void *)&zoomLevels[i]) != 0) {
            fprintf(stderr, "Error creating thread for zoom level %d\n", zoomLevels[i].zoomLevel);
        }
    }

    for (int i = 0; i < numZoomLevels; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <seed>\n", argv[0]);
        return 1;
    }

    uint64_t seed = strtoull(argv[1], NULL, 10);

    printf("Starting tile generation for seed: %lu\n", seed);

    startTime = time(NULL);

    char outputDir[2048];
    snprintf(outputDir, sizeof(outputDir), "/var/www/gme-backend/storage/app/public/tiles");

    if (createDir(outputDir) != 0) {
        return 1;
    }

    generateTilesForZoomLevels(seed, outputDir);

    time_t endTime = time(NULL);
    double totalTime = difftime(endTime, startTime);
    printf("All tiles generated. Total time taken: %.2f seconds\n", totalTime);

    return 0;
}
