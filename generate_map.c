#include "generator.h"
#include "util.h"
#include "image_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
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
    char *p = tmp;
    snprintf(tmp, sizeof(tmp), "%s", path);

    // Handle absolute and relative paths consistently
    if (tmp[0] == '/') {
        p = tmp + 1;
    } else {
        p = tmp;
    }

    for (; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            // Create the directory if it doesn't exist
            if (mkdir(tmp, 0777) && errno != EEXIST) {
                fprintf(stderr, "Error creating directory %s: %s\n", tmp, strerror(errno));
                return -1;
            }
            *p = '/';
        }
    }

    // Final directory creation
    if (mkdir(tmp, 0777) && errno != EEXIST) {
        fprintf(stderr, "Error creating directory %s: %s\n", tmp, strerror(errno));
        return -1;
    }

    return 0;
}


void generateTile(Generator *g, uint64_t seed, int tileX, int tileY, int tileSize, const char *outputDir, int zoomLevel, int scale) {
    setupGenerator(g, MC_1_18, LARGE_BIOMES);
    applySeed(g, DIM_OVERWORLD, seed);

    Range r = {
        .scale = scale,
        .x = tileX * tileSize,
        .z = tileY * tileSize,
        .sx = tileSize,
        .sz = tileSize,
        .y = 15,
        .sy = 1
    };

    int *biomeIds = allocCache(g, r);
    if (!biomeIds) {
        fprintf(stderr, "Error allocating memory for biomes\n");
        return;
    }

    genBiomes(g, biomeIds, r);

    int pix4cell = 4;
    int imgWidth = pix4cell * r.sx;
    int imgHeight = pix4cell * r.sz;

    unsigned char *rgb = (unsigned char *)malloc(3 * imgWidth * imgHeight);
    if (!rgb) {
        fprintf(stderr, "Error allocating memory for image\n");
        free(biomeIds);
        return;
    }

    unsigned char biomeColors[256][3];
    initBiomeColors(biomeColors);

    biomesToImage(rgb, biomeColors, biomeIds, r.sx, r.sz, pix4cell, 2);

    char tileDir[4096], outputFile[8192];
    snprintf(tileDir, sizeof(tileDir), "%s/%lu/%d/%d", outputDir, seed, zoomLevel, tileX);
    snprintf(outputFile, sizeof(outputFile), "%s/%d.png", tileDir, tileY);

    if (createDir(tileDir) != 0 || savePNG(outputFile, rgb, imgWidth, imgHeight) != 0) {
        fprintf(stderr, "Error saving image file for tile %d_%d at zoom level %d\n", tileX, tileY, zoomLevel);
    } else {
        pthread_mutex_lock(&mutex);
        completedTiles++;
        double elapsedSeconds = difftime(time(NULL), startTime);
        double estimatedTotalTime = (elapsedSeconds / completedTiles) * totalTiles;
        printf("Tile %d of %d generated and saved to %s\nEstimated time remaining: %.2f seconds\n", 
                completedTiles, totalTiles, outputFile, estimatedTotalTime - elapsedSeconds);
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
    int tileSize = params->base_tile_size;
    Generator g;

    // Initialize spiral parameters
    int x = params->tile_count / 2;
    int y = x;
    int dx = 0, dy = -1;
    int segmentLength = 1, segmentPassed = 0, turnsMade = 0;

    setupGenerator(&g, MC_1_18, LARGE_BIOMES);

    for (int i = 0; i < params->tile_count * params->tile_count; ++i) {
        if (x >= 0 && x < params->tile_count && y >= 0 && y < params->tile_count) {
            generateTile(&g, params->seed, x, y, tileSize, params->outputDir, params->zoomLevel, params->scale);
        }

        // Spiral logic
        x += dx;
        y += dy;
        segmentPassed++;
        if (segmentPassed == segmentLength) {
            int temp = dx;
            dx = -dy;
            dy = temp;
            segmentPassed = 0;
            if (++turnsMade % 2 == 0) segmentLength++;
        }
    }

    pthread_exit(NULL);
}

void generateTilesForZoomLevels(uint64_t seed, const char *outputDir) {
    ZoomLevelParams zoomLevels[] = {
        // {seed, outputDir, 3, 96, 128, 1},
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
    startTime = time(NULL);

    char outputDir[2048];
    snprintf(outputDir, sizeof(outputDir), "/var/www/gme-backend/storage/app/public/tiles");

    if (createDir(outputDir) != 0) {
        return 1;
    }

    generateTilesForZoomLevels(seed, outputDir);

    printf("All tiles generated. Total time taken: %.2f seconds\n", difftime(time(NULL), startTime));
    return 0;
}
