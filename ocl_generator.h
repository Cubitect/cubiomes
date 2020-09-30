#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl.h>
#include "generator.h"

enum Buffers {
    BUF_PRIMARY = 0,
    BUF_SECONDARY,
    BUF_TMP,

    BUFFER_COUNT
};

enum Kernels {
    KER_MAP_ISLAND,
    KER_MAP_ZOOM_ISLAND,
    KER_MAP_ADD_ISLAND,
    KER_MAP_ZOOM,
    KER_REMOVE_OCEAN,
    KER_ADD_SNOW,
    KER_COOL_WARM,
    KER_HEAT_ICE,
    KER_SPECIAL,
    KER_ADD_MUSHROOM,
    KER_DEEP_OCEAN,
    KER_BIOMES,
    KER_ADD_BAMBOO,
    KER_BIOME_EDGE,
    KER_RIVER_INIT,
    KER_HILLS,
    KER_RM_BRIM,
    KER_MAP_RARE_BIOMES,
    KER_SHORE,

    SET_SEED,

    KERNEL_COUNT
};

struct CLLayer {
    int64_t startSeed;
    int64_t startSalt;
};

struct CLLayerStack {
    struct CLLayer layers[L_NUM];
};

struct GeneratorContext {
    cl_context context;
    cl_command_queue queue;
    cl_mem biomesBuffer;
    cl_mem layersBuffer;
    cl_mem buffers[BUFFER_COUNT];
    cl_mem* buffer_layout[BUFFER_COUNT];
    cl_program program;
    size_t seed_range;
    int version;

    cl_kernel kernels[KERNEL_COUNT];

    struct CLLayerStack stack;
};

cl_int init_generator_context(struct GeneratorContext* context, int version, size_t width, size_t height, size_t seed_range);
cl_int set_world_seed(struct GeneratorContext* context, int64_t seed, cl_event* event);
/// Before buffer `target` can be used, user must wait for queue to finish, by 
/// clWaitForEvent for event.
cl_int generate_layer(struct GeneratorContext* context, int layer, cl_int4 dims, size_t seed_range, cl_int* target, const cl_event* prev, cl_event* event);

void release_generator_context(struct GeneratorContext* context);