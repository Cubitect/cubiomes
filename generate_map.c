#include "stb_image_write.h"
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

    // Define the number of chunks to generate
    int viewportWidth = 320; // Viewport width in blocks
    int viewportHeight = 420; // Viewport height in blocks
    int chunkSize = 16; // Size of a chunk in blocks

    int chunksX = (viewportWidth + chunkSize - 1) / chunkSize; // Number of chunks in the X direction
    int chunksZ = (viewportHeight + chunkSize - 1) / chunkSize; // Number of chunks in the Z direction

    // Define the output directory relative to the container's working directory
    const char *dirUrl = "/var/www/storage/app/public/images/seeds"; // Local file path

    // Ensure the directory exists
    if (createDir(dirUrl) != 0) {
        return 1;
    }

    // Initialize colors for biomes
    unsigned char biomeColors[256][3];
    initBiomeColors(biomeColors);

    // Parameters for image generation
    int pix4cell = 4; // Pixels per cell

    for (int cx = 0; cx < chunksX; cx++) {
        for (int cz = 0; cz < chunksZ; cz++) {
            // Predefined range for generating biomes
            Range r;
            r.scale = 4; // Scale for biome coordinates, this should match Minecraft's large biome scale
            r.x = (cx * chunkSize) / r.scale; // Starting X coordinate
            r.z = (cz * chunkSize) / r.scale; // Starting Z coordinate
            r.sx = chunkSize / r.scale; // Size of the area to generate in the X direction
            r.sz = chunkSize / r.scale; // Size of the area to generate in the Z direction

            // Allocate memory for storing biome IDs
            int *biomeIds = allocCache(&g, r);
            genBiomes(&g, biomeIds, r);

            // Calculate image dimensions
            int imgWidth = pix4cell * r.sx;
            int imgHeight = pix4cell * r.sz;

            // Allocate memory for the image
            unsigned char *rgb = (unsigned char *)malloc(3 * imgWidth * imgHeight);

            // Convert biomes to image
            biomesToImage(rgb, biomeColors, biomeIds, r.sx, r.sz, pix4cell, 2);

            // Save the image to file
            char outputFile[512];
            snprintf(outputFile, sizeof(outputFile), "%s/biome_map_chunk_%d_%d.png", dirUrl, cx, cz);
            if (stbi_write_png(outputFile, imgWidth, imgHeight, 3, rgb, 3 * imgWidth) == 0) {
                fprintf(stderr, "Error saving image to %s\n", outputFile);
                free(rgb);
                free(biomeIds);
                return 1;
            }

            free(rgb);
            free(biomeIds);

            printf("Chunk (%d, %d) generated and saved to %s\n", cx, cz, outputFile);
        }
    }

    return 0;
}
