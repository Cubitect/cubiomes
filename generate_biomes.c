#include "generator.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

int createDir(const char *path) {
    char tmp[256];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0775) && errno != EEXIST) {
                perror("Error creating directory");
                return -1;
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, 0775) && errno != EEXIST) {
        perror("Error creating directory");
        return -1;
    }
    return 0;
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

    Range r;
    r.scale = 16;
    r.x = -60; r.z = -60;
    r.sx = 480; r.sz = 480;
    r.y = 15; r.sy = 1;

    int *biomeIds = allocCache(&g, r);
    genBiomes(&g, biomeIds, r);

    int pix4cell = 16;
    int imgWidth = pix4cell * r.sx;
    int imgHeight = pix4cell * r.sz;

    unsigned char biomeColors[256][3];
    initBiomeColors(biomeColors);

    unsigned char *rgb = (unsigned char *)malloc(3 * imgWidth * imgHeight);
    biomesToImage(rgb, biomeColors, biomeIds, r.sx, r.sz, pix4cell, 2);

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

    if (chmod(outputFile, 0664) != 0) {
        perror("Error changing file permissions");
        free(biomeIds);
        free(rgb);
        return 1;
    }

    free(biomeIds);
    free(rgb);

    printf("Biome map generated and saved to %s\n", outputFile);

    return 0;
}
