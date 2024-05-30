#include "generator.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

// Function to recursively create directories
int createDir(const char *path) {
    char tmp[256];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0755) && errno != EEXIST) {
                perror("Error creating directory");
                return -1;
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, 0755) && errno != EEXIST) {
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

    // Parse the seed argument
    uint64_t seed = strtoull(argv[1], NULL, 10);
    
    // Set up the biome generator for Minecraft 1.18 with LARGE_BIOMES setting
    Generator g;
    setupGenerator(&g, MC_1_18, LARGE_BIOMES);

    // Apply the specified seed to the generator
    applySeed(&g, DIM_OVERWORLD, seed);

    // Predefined range for generating biomes
    Range r;
    r.scale = 16;         // Scale for biome coordinates
    r.x = -60; r.z = -60; // Starting coordinates
    r.sx = 480; r.sz = 480; // Size of the area to generate
    r.y = 15; r.sy = 1;     // y and sy are typically not used in 2D generation

    // Allocate memory for storing biome IDs
    int *biomeIds = allocCache(&g, r);
    genBiomes(&g, biomeIds, r);

    // Parameters for image generation
    int pix4cell = 4; // Pixels per cell
    int imgWidth = pix4cell * r.sx;
    int imgHeight = pix4cell * r.sz;

    // Initialize colors for biomes
    unsigned char biomeColors[256][3];
    initBiomeColors(biomeColors);

    // Allocate memory for the image
    unsigned char *rgb = (unsigned char *)malloc(3 * imgWidth * imgHeight);

    // Convert biomes to image
    biomesToImage(rgb, biomeColors, biomeIds, r.sx, r.sz, pix4cell, 2);

    // Define the output directory relative to the container's working directory
    const char *dirUrl = "/var/www/storage/app/public/images/seeds";

    // Ensure the directory exists
    if (createDir(dirUrl) != 0) {
        return 1;
    }

    // Construct the output file path using the seed
    char outputFile[256];
    snprintf(outputFile, sizeof(outputFile), "%s/seed_%lu.ppm", dirUrl, (unsigned long) seed);

    // Save the image to a PPM file
    if (savePPM(outputFile, rgb, imgWidth, imgHeight) != 0) {
        fprintf(stderr, "Error saving PPM file\n");
        return 1;
    }

    // Free allocated memory
    free(biomeIds);
    free(rgb);

    printf("Biome map generated and saved to %s\n", outputFile);

    return 0;
}
