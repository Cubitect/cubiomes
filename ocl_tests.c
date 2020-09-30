#include "ocl_generator.h"
#include "generator.h"
#include <stdio.h>

#define W 256
#define H 256

#define MAX_ERRORS 128
#define SEED_RANGE 32
#define START_SEED 1

struct Error {
    int layer;
    int seed;
    int x;
    int z;
    int ocl;
    int expected;
};

cl_int test_layer(struct GeneratorContext* context, LayerStack* stack, struct Error* errors, int* error_count, int* bufferB, int layer) {
    cl_int err;
    cl_event event;

    cl_int4 dims = {{0, 0, W, H}};
    err = generate_layer(context, layer, dims, SEED_RANGE, bufferB, NULL, &event);
    if (err < 0) {
        printf("layer=%d err=%d", layer, err);
        return err;
    }
    err = clWaitForEvents(1, &event);
    if (err < 0) {
        printf("layer=%d err=%d", layer, err);
        return err;
    }
    int* bufferA = allocCache(&stack->layers[layer], W, H);
    for (int s = 0; s < SEED_RANGE; ++s) {
        applySeed(stack, START_SEED + s);
        genArea(&stack->layers[layer], bufferA, dims.s0, dims.s1, W, H);
        for (int j = 0; j < H && *error_count <= MAX_ERRORS; ++j) {
            for (int i = 0; i < W && *error_count <= MAX_ERRORS; ++i) {
                if (bufferA[i + j*W] != bufferB[i + j*W + s*W*H]) {
                    errors[*error_count].seed = START_SEED + s;
                    errors[*error_count].layer = layer;
                    errors[*error_count].x = i;
                    errors[*error_count].z = j;
                    errors[*error_count].expected = bufferA[i + j*W + s*W*H];
                    errors[*error_count].ocl = bufferB[i + j*W + s*W*H];
                    (*error_count) += 1;
                }
            }
        }
    }
    free(bufferA);
    if (*error_count == 0) {
        printf("Layer %d is good. :)\n", layer);
    }
    return CL_SUCCESS;
}

cl_int run_tests(struct Error* errors, int* error_count) {
    initBiomes();
    LayerStack stack;
    setupGenerator(&stack, MC_1_16);

    struct GeneratorContext context;
    cl_int err = init_generator_context(&context, MC_1_16, SEED_RANGE, W, H);
    if (err < 0) return err;
    cl_event event0;
    set_world_seed(&context, START_SEED, &event0);
    clWaitForEvents(1, &event0);

    cl_int* bufferB = (cl_int*) malloc(SEED_RANGE * W * H * sizeof(cl_int));

    *error_count = 0;
    for (int layer=0; layer <= L_BIOME_256; ++layer) {
        test_layer(&context, &stack, errors, error_count, bufferB, layer);
    }
    test_layer(&context, &stack, errors, error_count, bufferB, L14_BAMBOO_256);
    for (int layer=L_ZOOM_128; layer <= L_SHORE_16; ++layer) {
        test_layer(&context, &stack, errors, error_count, bufferB, layer);
    }

    free(bufferB);
    release_generator_context(&context);
    return err;
}

int main() {
    struct Error errors[MAX_ERRORS];
    int error_count = 0;
    cl_int err = run_tests(errors, &error_count);
    if (err < 0) {
        printf("CL Error: %d\n", err);
    }
    if (error_count > 0) {
        printf("At least %d value errors encountered:\n", error_count);
        for (int i = 0; i < error_count; ++i) {
            printf("seed=%d x=%d z=%d \t| expected=%d \treal=%d\n", errors[i].seed, errors[i].x, errors[i].z, errors[i].expected, errors[i].ocl);
        }
    } else if (err == 0) {
        printf("No errors.\n :)\n");
    }
}