#include "generator.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define NUM_THREADS 4 // Adjust this based on the number of CPU cores

typedef struct {
    unsigned char *rgb;
    unsigned char (*biomeColors)[3];
    int *biomeIds;
    int startRow;
    int endRow;
    int pix4cell;
    int sx;
    int sz;
} BiomeToImageArgs;

// Function to recursively create directories
int createDir(const char *path) {
    char tmp[256];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0777) && errno != EEXIST) {
                perror("Error creating directory");
                return -1;
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, 0777) && errno != EEXIST) {
        perror("Error creating directory");
        return -1;
    }
    return 0;
}

// Function to convert biomes to image (parallelized)
void* biomesToImageParallel(void *args) {
    BiomeToImageArgs *data = (BiomeToImageArgs *)args;
    int i, j, k;
    for (i = data->startRow; i < data->endRow; i++) {
        for (j = 0; j < data->sx; j++) {
            int biomeId = data->biomeIds[i * data->sx + j];
            for (k = 0; k < data->pix4cell; k++) {
                int pixIdx = (i * data->pix4cell + k) * (data->sx * data->pix4cell) + (j * data->pix4cell);
                for (int m = 0; m < data->pix4cell; m++) {
                    int idx = 3 * (pixIdx + m);
                    data->rgb[idx] = data->biomeColors[biomeId][0];
                    data->rgb[idx + 1] = data->biomeColors[biomeId][1];
                    data->rgb[idx + 2] = data->biomeColors[biomeId][2];
                }
            }
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <seed>\n", argv[0]);
        return 1;
    }

    uint64_t seed = strtoull(argv[1], NULL, 10);

    Generator g;
    setupGenerator(&g, MC_1_18, LARGE_BIOMES);
    applySeed(&g, DIM_OVERWORLD, seed);

    Range r = { .scale = 16, .x = -60, .z = -60, .sx = 400, .sz = 320, .y = 15, .sy = 1 };

    int *biomeIds = allocCache(&g, r);
    genBiomes(&g, biomeIds, r);

    int pix4cell = 4;
    int imgWidth = pix4cell * r.sx;
    int imgHeight = pix4cell * r.sz;

    unsigned char biomeColors[256][3];
    initBiomeColors(biomeColors);

    unsigned char *rgb = (unsigned char *)malloc(3 * imgWidth * imgHeight);

    // Parallelize biomesToImage using pthreads
    pthread_t threads[NUM_THREADS];
    BiomeToImageArgs threadArgs[NUM_THREADS];
    int rowsPerThread = r.sz / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++) {
        threadArgs[i].rgb = rgb;
        threadArgs[i].biomeColors = biomeColors;
        threadArgs[i].biomeIds = biomeIds;
        threadArgs[i].startRow = i * rowsPerThread;
        threadArgs[i].endRow = (i == NUM_THREADS - 1) ? r.sz : (i + 1) * rowsPerThread;
        threadArgs[i].pix4cell = pix4cell;
        threadArgs[i].sx = r.sx;
        threadArgs[i].sz = r.sz;
        pthread_create(&threads[i], NULL, biomesToImageParallel, (void *)&threadArgs[i]);
    }

    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    const char *dirUrl = "/var/www/gme-backend/storage/app/public/images/seeds";

    if (createDir(dirUrl) != 0) {
        free(biomeIds);
        free(rgb);
        return 1;
    }

    char outputFile[256];
    snprintf(outputFile, sizeof(outputFile), "%s/seed_%lu.ppm", dirUrl, (unsigned long)seed);

    if (savePPM(outputFile, rgb, imgWidth, imgHeight) != 0) {
        fprintf(stderr, "Error saving PPM file\n");
        free(biomeIds);
        free(rgb);
        return 1;
    }

    free(biomeIds);
    free(rgb);

    printf("Biome map generated and saved to %s\n", outputFile);

    return 0;
}
