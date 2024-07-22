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

// Function to perform bilinear interpolation
void bilinearInterpolate(unsigned char *rgb, int width, int height, unsigned char *smoothRgb, int smoothWidth, int smoothHeight) {
    for (int y = 0; y < smoothHeight; ++y) {
        for (int x = 0; x < smoothWidth; ++x) {
            float gx = ((float)x / (float)(smoothWidth - 1)) * (width - 1);
            float gy = ((float)y / (float)(smoothHeight - 1)) * (height - 1);
            int gxi = (int)gx;
            int gyi = (int)gy;

            // Ensure gxi and gyi are within bounds
            gxi = (gxi < 0) ? 0 : (gxi >= width - 1) ? width - 2 : gxi;
            gyi = (gyi < 0) ? 0 : (gyi >= height - 1) ? height - 2 : gyi;

            // Bilinear interpolation weights
            float wx = gx - gxi;
            float wy = gy - gyi;
            float wx1 = 1.0f - wx;
            float wy1 = 1.0f - wy;

            // Interpolate each channel separately
            float r = wy1 * (wx1 * rgb[(gyi * width + gxi) * 3 + 0] + wx * rgb[(gyi * width + (gxi + 1)) * 3 + 0]) +
                      wy * (wx1 * rgb[((gyi + 1) * width + gxi) * 3 + 0] + wx * rgb[((gyi + 1) * width + (gxi + 1)) * 3 + 0]);

            float g = wy1 * (wx1 * rgb[(gyi * width + gxi) * 3 + 1] + wx * rgb[(gyi * width + (gxi + 1)) * 3 + 1]) +
                      wy * (wx1 * rgb[((gyi + 1) * width + gxi) * 3 + 1] + wx * rgb[((gyi + 1) * width + (gxi + 1)) * 3 + 1]);

            float b = wy1 * (wx1 * rgb[(gyi * width + gxi) * 3 + 2] + wx * rgb[(gyi * width + (gxi + 1)) * 3 + 2]) +
                      wy * (wx1 * rgb[((gyi + 1) * width + gxi) * 3 + 2] + wx * rgb[((gyi + 1) * width + (gxi + 1)) * 3 + 2]);

            // Store interpolated values
            smoothRgb[(y * smoothWidth + x) * 3 + 0] = (unsigned char)r;
            smoothRgb[(y * smoothWidth + x) * 3 + 1] = (unsigned char)g;
            smoothRgb[(y * smoothWidth + x) * 3 + 2] = (unsigned char)b;
        }
    }
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
    r.sx = 800; r.sz = 480; // Size of the area to generate
    r.y = 15; r.sy = 1;     // y and sy are typically not used in 2D generation

    // Allocate memory for storing biome IDs
    int *biomeIds = allocCache(&g, r);
    genBiomes(&g, biomeIds, r);
    
    // Parameters for image generation
    int pix4cell = 16; // Pixels per cell
    int imgWidth = pix4cell * r.sx;
    int imgHeight = pix4cell * r.sz;

    // Initialize colors for biomes
    unsigned char biomeColors[256][3];
    initBiomeColors(biomeColors);

    // Allocate memory for the image
    unsigned char *rgb = (unsigned char *)malloc(3 * imgWidth * imgHeight);

    // Convert biomes to image
    biomesToImage(rgb, biomeColors, biomeIds, r.sx, r.sz, pix4cell, 2);

    // Apply bilinear interpolation to smooth the image
    int smoothWidth = imgWidth * 2;
    int smoothHeight = imgHeight * 2;
    unsigned char *smoothRgb = (unsigned char *)malloc(3 * smoothWidth * smoothHeight);
    bilinearInterpolate(rgb, imgWidth, imgHeight, smoothRgb, smoothWidth, smoothHeight);

    // Define the output directory relative to the container's working directory
    const char *dirUrl = "/var/www/storage/app/public/images/seeds";

    // Ensure the directory exists
    if (createDir(dirUrl) != 0) {
        return 1;
    }

    // Construct the output file path using the seed
    char outputFile[256];
    snprintf(outputFile, sizeof(outputFile), "%s/seed_%lu.ppm", dirUrl, (unsigned long)seed);

    // Debug: Print the output file path
    printf("Attempting to save PPM file to %s\n", outputFile);

    // Save the smooth image to a PPM file
    if (savePPM(outputFile, smoothRgb, smoothWidth, smoothHeight) != 0) {
        fprintf(stderr, "Error saving PPM file\n");
        free(biomeIds);
        free(rgb);
        free(smoothRgb);
        return 1;
    }

    // Free allocated memory
    free(biomeIds);
    free(rgb);
    free(smoothRgb);

    printf("Biome map generated and saved to %s\n", outputFile);

    return 0;
}
