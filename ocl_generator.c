#include "ocl_generator.h"
#include <stdio.h>

static int LAYER_BUFFER[L_NUM] = {0};
typedef cl_int (*kernel_runner)(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event);
typedef cl_int4 (*size_alteration)(cl_int4 dims);

// Kernel runners are responcible to assigning buffers to layers.
// Each layer has associated buffer id. Normaly, as buffers lineary depend on each other,
// only single buffer is needed (apart from tmp buffer). 
// But in some cases multiple layers depend on the same layer, and thus 
// it is necesary to store data of an old layer and compute new layers.
// 
// For example: 
// Layer `Deep Ocean` needs to be alive until `River Init` is computed,
// so it is stored in buffer[1], while dependent layers as `Biome` can live
// in buffer[0].
// Layer `River Init` can then live in buffer[1], since 
// `Biome Edge` already occupying buffer[0], but previous occupant of buffer[1]
// no longer needed.
static kernel_runner KERNEL_RUNNERS[L_NUM] = {NULL};
static size_alteration PARENT_SIZES[L_NUM] = {NULL};
static size_alteration LAYER_WORK_DIMENTIONS[L_NUM] = {NULL};

cl_int4 zooming_layer(cl_int4 dims) {
    int x = dims.s0;
    int z = dims.s1;
    int w = dims.s2;
    int h = dims.s3;
    int px = x >> 1;
    int pz = z >> 1;
    dims.s2 = ((x + w) >> 1) - px + 1;
    dims.s3 = ((z + h) >> 1) - pz + 1;
    dims.s0 = px;
    dims.s1 = pz;
    return dims;
}

cl_int4 add_brim_layer(cl_int4 dims) {
    dims.s0 -= 1;
    dims.s1 -= 1;
    dims.s2 += 2;
    dims.s3 += 2;
    return dims;
}

cl_int4 zooming_brim_layer(cl_int4 dims) {
    return zooming_layer(add_brim_layer(dims));
}

cl_int4 layer_size_identity(cl_int4 dims) {
    return dims;
}

static inline cl_int run_layer_kernel_with_dims(int kernel_id, size_t* sdims, cl_mem* in, cl_mem* out, struct GeneratorContext* context, cl_int layer, cl_int4 dims, const cl_event* prev, cl_event* event) {
    cl_kernel kernel = context->kernels[kernel_id];
    cl_int err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &context->layersBuffer);
    if (err < 0) return err;
    err = clSetKernelArg(kernel, 1, sizeof(cl_int), &layer);
    if (err < 0) return err;
    err = clSetKernelArg(kernel, 2, sizeof(cl_int4), &dims);
    if (err < 0) return err;
    err = clSetKernelArg(kernel, 3, sizeof(cl_mem), in);
    if (err < 0) return err;
    err = clSetKernelArg(kernel, 4, sizeof(cl_mem), out);
    if (err < 0) return err;

    return clEnqueueNDRangeKernel(
        context->queue, kernel,
        3, NULL, sdims, NULL, 
        1, prev, event
    );
}

static inline cl_int run_layer_kernel(int kernel_id, cl_mem* in, cl_mem* out, struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    size_t sdims[3];
    cl_int4 parent_dims = LAYER_WORK_DIMENTIONS[layer](dims);
    sdims[0] = parent_dims.s2;
    sdims[1] = parent_dims.s3;
    sdims[2] = seed_range;
    return run_layer_kernel_with_dims(kernel_id, sdims, in, out, context, layer, dims, prev, event);
}

static inline cl_int run_biome_layer_kernel(int kernel_id, cl_mem* in, cl_mem* out, struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    size_t sdims[3];
    cl_int4 parent_dims = LAYER_WORK_DIMENTIONS[layer](dims);
    sdims[0] = parent_dims.s2;
    sdims[1] = parent_dims.s3;
    sdims[2] = seed_range;
    cl_kernel kernel = context->kernels[kernel_id];
    cl_int err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &context->layersBuffer);
    if (err < 0) return err;
    err = clSetKernelArg(kernel, 1, sizeof(cl_int), &layer);
    if (err < 0) return err;
    err = clSetKernelArg(kernel, 2, sizeof(cl_int4), &dims);
    if (err < 0) return err;
    err = clSetKernelArg(kernel, 3, sizeof(cl_mem), &context->biomesBuffer);
    if (err < 0) return err;
    err = clSetKernelArg(kernel, 4, sizeof(cl_mem), in);
    if (err < 0) return err;
    err = clSetKernelArg(kernel, 5, sizeof(cl_mem), out);
    if (err < 0) return err;

    return clEnqueueNDRangeKernel(
        context->queue, kernel,
        3, NULL, sdims, NULL, 
        1, prev, event
    );
}

static inline void swap_buffers(cl_mem** a, cl_mem** b) {
    cl_mem* tmp = *a;
    *a = *b;
    *b = tmp;
}

// =============================================
// KERNEL RUNNERS
//
cl_int island_4096_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_PRIMARY];
    cl_mem** out = &context->buffer_layout[BUF_TMP];
    cl_int error = run_layer_kernel(KER_MAP_ISLAND, *in, *out, context, layer, dims, seed_range, prev, event);
    swap_buffers(in, out);
    return error;
}

cl_int zoom_island_2048_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_PRIMARY];
    cl_mem** out = &context->buffer_layout[BUF_TMP];
    cl_int error = run_layer_kernel(KER_MAP_ZOOM_ISLAND, *in, *out, context, layer, dims, seed_range, prev, event);
    swap_buffers(in, out);
    return error;
}

cl_int add_island_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_PRIMARY];
    cl_mem** out = &context->buffer_layout[BUF_TMP];
    cl_int error = run_layer_kernel(KER_MAP_ADD_ISLAND, *in, *out, context, layer, dims, seed_range, prev, event);
    swap_buffers(in, out);
    return error;
}

cl_int zoom_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_PRIMARY];
    cl_mem** out = &context->buffer_layout[BUF_TMP];
    cl_int error = run_layer_kernel(KER_MAP_ZOOM, *in, *out, context, layer, dims, seed_range, prev, event);
    swap_buffers(in, out);
    return error;
}

// Same as zoom_runner, but user secondary buffer
cl_int zoom_snd_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_SECONDARY];
    cl_mem** out = &context->buffer_layout[BUF_TMP];
    cl_int error = run_layer_kernel(KER_MAP_ZOOM, *in, *out, context, layer, dims, seed_range, prev, event);
    swap_buffers(in, out);
    return error;
}

// Zoom and remove brim. 
// Brim is added to make sizes of L_DEEP_OCEAN_256 required by L_RIVER_INIT_256 and L_BIOME_256 same.
cl_int zoom_64_hills_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_SECONDARY];
    cl_mem** out = &context->buffer_layout[BUF_TMP];
    cl_event e;

    cl_int4 d0 = zooming_brim_layer(dims);
    size_t sdims[3] = {d0.s2, d0.s3, seed_range};
    cl_int err = run_layer_kernel_with_dims(KER_MAP_ZOOM, sdims, *in, *out, context, layer, add_brim_layer(dims), prev, &e);
    swap_buffers(in, out);
    if (err < 0) return err;

    sdims[0] = dims.s2;
    sdims[1] = dims.s3;
    sdims[2] = seed_range;

    cl_kernel kernel = context->kernels[KER_RM_BRIM];
    err = clSetKernelArg(kernel, 0, sizeof(cl_int4), &dims);
    if (err < 0) return err;
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), *in);
    if (err < 0) return err;
    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), *out);
    if (err < 0) return err;

    err = clEnqueueNDRangeKernel(
        context->queue, kernel,
        3, NULL, sdims, NULL, 
        1, prev, event
    );
    swap_buffers(in, out);
    return err;
}

cl_int remove_ocean_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_PRIMARY];
    cl_mem** out = &context->buffer_layout[BUF_TMP];
    cl_int error = run_layer_kernel(KER_REMOVE_OCEAN, *in, *out, context, layer, dims, seed_range, prev, event);
    swap_buffers(in, out);
    return error;
}

cl_int add_snow_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_PRIMARY];
    cl_mem** out = &context->buffer_layout[BUF_TMP];
    cl_int error = run_layer_kernel(KER_ADD_SNOW, *in, *out, context, layer, dims, seed_range, prev, event);
    swap_buffers(in, out);
    return error;
}

cl_int cool_warm_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_PRIMARY];
    cl_mem** out = &context->buffer_layout[BUF_TMP];
    cl_int error = run_layer_kernel(KER_COOL_WARM, *in, *out, context, layer, dims, seed_range, prev, event);
    swap_buffers(in, out);
    return error;
}

cl_int heat_ice_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_PRIMARY];
    cl_mem** out = &context->buffer_layout[BUF_TMP];
    cl_int error = run_layer_kernel(KER_HEAT_ICE, *in, *out, context, layer, dims, seed_range, prev, event);
    swap_buffers(in, out);
    return error;
}

cl_int special_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_PRIMARY];
    cl_mem** out = &context->buffer_layout[BUF_TMP];
    cl_int error = run_layer_kernel(KER_SPECIAL, *in, *out, context, layer, dims, seed_range, prev, event);
    swap_buffers(in, out);
    return error;
}

cl_int add_mashroom_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_PRIMARY];
    cl_mem** out = &context->buffer_layout[BUF_TMP];
    cl_int error = run_layer_kernel(KER_ADD_MUSHROOM, *in, *out, context, layer, dims, seed_range,  prev, event);
    swap_buffers(in, out);
    return error;
}

cl_int deep_ocean_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_PRIMARY];
    cl_mem** out = &context->buffer_layout[BUF_SECONDARY];
    cl_int error = run_layer_kernel(KER_DEEP_OCEAN, *in, *out, context, layer, dims, seed_range, prev, event);
    return error;
}

cl_int biomes_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_SECONDARY];
    cl_mem** out = &context->buffer_layout[BUF_PRIMARY];
    cl_int error = run_biome_layer_kernel(KER_BIOMES, *in, *out, context, layer, dims, seed_range, prev, event);
    return error;
}

cl_int add_bamboo_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_PRIMARY];
    cl_mem** out = &context->buffer_layout[BUF_TMP];
    cl_int error = run_layer_kernel(KER_ADD_BAMBOO, *in, *out, context, layer, dims, seed_range, prev, event);
    swap_buffers(in, out);
    return error;
}

cl_int biome_edge_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_PRIMARY];
    cl_mem** out = &context->buffer_layout[BUF_TMP];
    cl_int error = run_biome_layer_kernel(KER_BIOME_EDGE, *in, *out, context, layer, dims, seed_range, prev, event);
    swap_buffers(in, out);
    return error;
}

cl_int river_init_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_SECONDARY];
    cl_mem** out = &context->buffer_layout[BUF_TMP];
    cl_int error = run_layer_kernel(KER_RIVER_INIT, *in, *out, context, layer, dims, seed_range, prev, event);
    swap_buffers(in, out);
    return error;
}

cl_int hills_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in1 = &context->buffer_layout[BUF_PRIMARY];
    cl_mem** in2 = &context->buffer_layout[BUF_SECONDARY];
    cl_mem** out = &context->buffer_layout[BUF_TMP];

    size_t sdims[3];
    cl_int4 parent_dims = LAYER_WORK_DIMENTIONS[layer](dims);
    sdims[0] = parent_dims.s2;
    sdims[1] = parent_dims.s3;
    sdims[2] = seed_range;
    cl_kernel kernel = context->kernels[KER_HILLS];
    cl_int err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &context->layersBuffer);
    if (err < 0) return err;
    err = clSetKernelArg(kernel, 1, sizeof(cl_int), &layer);
    if (err < 0) return err;
    err = clSetKernelArg(kernel, 2, sizeof(cl_int4), &dims);
    if (err < 0) return err;
    err = clSetKernelArg(kernel, 3, sizeof(cl_mem), &context->biomesBuffer);
    if (err < 0) return err;
    err = clSetKernelArg(kernel, 4, sizeof(cl_mem), *in1);
    if (err < 0) return err;
    err = clSetKernelArg(kernel, 5, sizeof(cl_mem), *in2);
    if (err < 0) return err;
    err = clSetKernelArg(kernel, 6, sizeof(cl_mem), *out);
    if (err < 0) return err;

    err = clEnqueueNDRangeKernel(
        context->queue, kernel,
        3, NULL, sdims, NULL, 
        1, prev, event
    );
    swap_buffers(in1, out);
    return err;
}

cl_int rare_biome_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_PRIMARY];
    cl_mem** out = &context->buffer_layout[BUF_TMP];
    cl_int error = run_layer_kernel(KER_MAP_RARE_BIOMES, *in, *out, context, layer, dims, seed_range, prev, event);
    swap_buffers(in, out);
    return error;
}

cl_int shore_runner(struct GeneratorContext* context, cl_int layer, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_mem** in = &context->buffer_layout[BUF_PRIMARY];
    cl_mem** out = &context->buffer_layout[BUF_TMP];
    cl_int error = run_biome_layer_kernel(KER_SHORE, *in, *out, context, layer, dims, seed_range, prev, event);
    swap_buffers(in, out);
    return error;
}

/// Setup layer global properties.
/// + layer :: id of layer to set up
/// + runner :: function that will execute openCL kernel. Responcible for selecting proper buffers.
/// + buffer_id :: id of the buffer there the result will be available.
/// + size_alter :: function to determine required size of parent layer, given current layer.
/// + work_dimentions :: function to determine dimentions of global workgroup of kernel.
static inline void init_layer(int layer, kernel_runner runner, int buffer_id, size_alteration size_alter, size_alteration work_dimentions) {
    PARENT_SIZES[layer] = size_alter;
    KERNEL_RUNNERS[layer] = runner;
    LAYER_BUFFER[layer] = buffer_id;
    LAYER_WORK_DIMENTIONS[layer] = work_dimentions;
}

void init_layer_decendants() {
    init_layer(L_ISLAND_4096, island_4096_runner, 0, layer_size_identity, layer_size_identity);
    init_layer(L_ZOOM_2048, zoom_island_2048_runner, 0, zooming_layer, zooming_layer);
    init_layer(L_ADD_ISLAND_2048, add_island_runner, 0, add_brim_layer, layer_size_identity);
    init_layer(L_ZOOM_1024, zoom_runner, 0, zooming_layer, zooming_layer);
    init_layer(L_ADD_ISLAND_1024A, add_island_runner, 0, add_brim_layer, layer_size_identity);
    init_layer(L_ADD_ISLAND_1024B, add_island_runner, 0, add_brim_layer, layer_size_identity);
    init_layer(L_ADD_ISLAND_1024C, add_island_runner, 0, add_brim_layer, layer_size_identity);
    init_layer(L_REMOVE_OCEAN_1024, remove_ocean_runner, 0, add_brim_layer, layer_size_identity);
    init_layer(L_ADD_SNOW_1024, add_snow_runner, 0, add_brim_layer, layer_size_identity);
    init_layer(L_ADD_ISLAND_1024D, add_island_runner, 0, add_brim_layer, layer_size_identity);
    init_layer(L_COOL_WARM_1024, cool_warm_runner, 0, add_brim_layer, layer_size_identity);
    init_layer(L_HEAT_ICE_1024, heat_ice_runner, 0, add_brim_layer, layer_size_identity);
    init_layer(L_SPECIAL_1024, special_runner, 0, layer_size_identity, layer_size_identity);
    init_layer(L_ZOOM_512, zoom_runner, 0, zooming_layer, zooming_layer);
    init_layer(L_ZOOM_256, zoom_runner, 0, zooming_layer, zooming_layer);
    init_layer(L_ADD_ISLAND_256, add_island_runner, 0, add_brim_layer, layer_size_identity);
    init_layer(L_ADD_MUSHROOM_256, add_mashroom_runner, 0, add_brim_layer, layer_size_identity);
    init_layer(L_DEEP_OCEAN_256, deep_ocean_runner, 1, add_brim_layer, layer_size_identity);
    init_layer(L_BIOME_256, biomes_runner, 0, layer_size_identity, layer_size_identity);
    init_layer(L14_BAMBOO_256, add_bamboo_runner, 0, layer_size_identity, layer_size_identity);
    init_layer(L_ZOOM_128, zoom_runner, 0, zooming_layer, zooming_layer);
    init_layer(L_ZOOM_64, zoom_runner, 0, zooming_layer, zooming_layer);
    init_layer(L_BIOME_EDGE_64, biome_edge_runner, 0, add_brim_layer, layer_size_identity);
    init_layer(L_RIVER_INIT_256, river_init_runner, 1, layer_size_identity, layer_size_identity);
    init_layer(L_ZOOM_128_HILLS, zoom_snd_runner, 1, zooming_layer, zooming_layer);
    init_layer(L_ZOOM_64_HILLS, zoom_64_hills_runner, 1, zooming_brim_layer, layer_size_identity);
    init_layer(L_HILLS_64, hills_runner, 0, add_brim_layer, layer_size_identity);
    init_layer(L_RARE_BIOME_64, rare_biome_runner, 0, layer_size_identity, layer_size_identity);
    init_layer(L_ZOOM_32, zoom_runner, 0, zooming_layer, zooming_layer);
    init_layer(L_ADD_ISLAND_32, add_island_runner, 0, add_brim_layer, layer_size_identity);
    init_layer(L_ZOOM_16, zoom_runner, 0, zooming_layer, zooming_layer);
    init_layer(L_SHORE_16, shore_runner, 0, add_brim_layer, layer_size_identity);
}

static const cl_int ZERO = 0;
static inline cl_int fill_primary_buffer(struct GeneratorContext* context, cl_int4 dims, size_t seed_range, const cl_event* prev, cl_event* event) {
    cl_int event_count = prev ? 1 : 0;
    return clEnqueueFillBuffer(context->queue, context->buffers[0], &ZERO, sizeof(cl_int), 0, seed_range*dims.s2*dims.s3*sizeof(cl_int), event_count, prev, event);
}

int create_build_plan(struct GeneratorContext* context, int layer, int* plan, cl_int4* sizes, cl_int4 dims);
int create_hills_build_plan(struct GeneratorContext* context, int* plan, cl_int4* sizes, cl_int4 dims) {
    // river branch
    cl_int4 rdims = dims;
    const int RIVER_LAYERS[] = {L_ZOOM_64_HILLS, L_ZOOM_128_HILLS, L_RIVER_INIT_256};
    for (int i=0; i<3; ++i) {
        plan--;
        sizes--;
        *plan = RIVER_LAYERS[i];
        *sizes = rdims;
        rdims = PARENT_SIZES[RIVER_LAYERS[i]](rdims);
    }
    // main branch
    return 4 + create_build_plan(context, L_BIOME_EDGE_64, plan, sizes, dims);
}

void show_build_plan(int* layer, cl_int4* sizes, int len) {
    for(int i=0; i<len; ++i) {
        printf("%d (%d %d %d %d)\n", layer[i], sizes[i].s0, sizes[i].s1, sizes[i].s2, sizes[i].s3);
    }
}

// Create build plan 
// Responsible for setting layers in order they computed.
// `plan` and `sizes` arrays grow backwards.
int create_build_plan(struct GeneratorContext* context, int layer, int* plan, cl_int4* sizes, cl_int4 dims) {
    plan--;
    sizes--;
    *plan = layer;
    *sizes = dims;
    if (layer < L_ZOOM_128) {
        int len = layer + 1;
        for (; layer >= 0; layer--, plan--, sizes--) {
            *plan = layer;
            *sizes = dims;
            dims = PARENT_SIZES[layer](dims);
        }
        return len;
    } else {
        int parent;
        switch (layer) {
            case L_ZOOM_128: parent = L14_BAMBOO_256; break;
            case L14_BAMBOO_256: parent = L_BIOME_256; break;
            case L_ZOOM_64: parent = L_ZOOM_128; break;
            case L_BIOME_EDGE_64: parent = L_ZOOM_64; break;
            case L_RIVER_INIT_256: parent = L_DEEP_OCEAN_256; break;
            case L_ZOOM_128_HILLS: parent = L_RIVER_INIT_256; break;
            case L_ZOOM_64_HILLS: parent = L_ZOOM_128_HILLS; break;
            case L_HILLS_64:
                return create_hills_build_plan(context, plan, sizes, PARENT_SIZES[layer](dims));
            case L_RARE_BIOME_64: parent = L_HILLS_64; break;
            case L_ZOOM_32: parent = L_RARE_BIOME_64; break;
            case L_ADD_ISLAND_32: parent = L_ZOOM_32; break;
            case L_ZOOM_16: parent = L_ADD_ISLAND_32; break;
            case L_SHORE_16: parent = L_ZOOM_16; break;
            default:
                printf("Error: unable to create build plan. Invalid layer id %d\n", layer);
                return 0;
        }
        return 1 + create_build_plan(context, parent, plan, sizes, PARENT_SIZES[layer](dims));
    }
}

cl_int generate_layer(struct GeneratorContext* context, int layer, cl_int4 dims, size_t seed_range, cl_int* target, const cl_event* prev, cl_event* event) {
    int build_plan[L_NUM];
    cl_int4 sz[L_NUM];
    cl_event events[L_NUM + 1];
    cl_int err;

    int* plan_end = build_plan + L_NUM;
    int plan_len = create_build_plan(context, layer, build_plan + L_NUM, sz + L_NUM, dims);

    int* layers = build_plan + L_NUM - plan_len;
    cl_int4* sizes = sz + L_NUM - plan_len;

    err = fill_primary_buffer(context, dims, seed_range, prev, &events[0]);
    if (err < 0) return err;

    for (int i = 0; i < plan_len; ++i) {
        int curr_layer = layers[i];
        err = KERNEL_RUNNERS[curr_layer](context, (cl_int)curr_layer, sizes[i], seed_range, &events[i], &events[i + 1]);
        if (err < 0) return err;
    }
    cl_mem* buffer = context->buffer_layout[LAYER_BUFFER[layer]];
    return clEnqueueReadBuffer(
        context->queue, *buffer, CL_FALSE,
        0, sizeof(cl_int) * dims.s2 * dims.s3 * seed_range, target, 
        1, &events[plan_len], event
    );
}

cl_int init_generator_context(struct GeneratorContext* context, int version, size_t width, size_t height, size_t seed_range) {
    init_layer_decendants();
    context->seed_range = seed_range;
    cl_int err;
    cl_platform_id platform;
    int new_width = width + 2 * L_NUM; // Most commonly, parent layers add 1 cell brim around child area.
    int new_height = height + 2 * L_NUM;
    if ((err = clGetPlatformIDs(1, &platform, NULL)) < 0) {
        return err;
    }
    cl_device_id device;
    if ((err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL)) < 0) {
        return err;
    }
    context->context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (err < 0) {
        return err;
    }
    context->queue = clCreateCommandQueueWithProperties(context->context, device, (cl_queue_properties*) NULL, &err);
    if (err < 0) {
        clReleaseContext(context->context);
        return err;
    }
    // This buffer holds info about layer seeds. Context will be initialized by `set_layer_seed` call.
    context->layersBuffer = clCreateBuffer(context->context, CL_MEM_READ_ONLY, sizeof(struct CLLayer) * L_NUM * seed_range, NULL, &err);
    if (err < 0) {
        clReleaseContext(context->context);
        return err;
    }
    // This buffer holds info about biomes. 
    context->biomesBuffer = clCreateBuffer(context->context, CL_MEM_READ_ONLY, sizeof(Biome) * 256, NULL, &err);
    if (err < 0) {
        clReleaseContext(context->context);
        return err;
    }
    initBiomes();
    clEnqueueWriteBuffer(context->queue, context->biomesBuffer, CL_TRUE, 0, sizeof(Biome) * 256, biomes, 0, NULL, NULL);
    // Init buffers for layer data.
    for (int i = 0; i < BUFFER_COUNT; ++i) {
        context->buffers[i] = clCreateBuffer(context->context, CL_MEM_READ_WRITE, seed_range * new_width * new_height * sizeof(cl_int), NULL, &err);
        if (err < 0) {
            clReleaseContext(context->context);
            return err;
        }
        context->buffer_layout[i] = &context->buffers[i];
    }
    // Load program.
    FILE* program = fopen("ocl_kernels.cl", "r");
    if (!program) {
        clReleaseContext(context->context);
        return -1;
    }
    // 256 KiB should be enough?
    // #TODO: handle file loading properly.
    const int SIZE = 1024 * 256;
    char* buffer = (char*) malloc(SIZE); 
    if (!buffer) {
        fclose(program);
        clReleaseContext(context->context);
        return -1;
    }
    int length = fread(buffer, 1, SIZE, program);
    if (length < 0) {
        fclose(program);
        free(buffer);
        clReleaseContext(context->context);
        return -1;
    }
    fclose(program);
    context->program = clCreateProgramWithSource(context->context, 1, (const char**)&buffer, (size_t*)NULL, &err);
    free(buffer);
    if (err < 0) {
        clReleaseContext(context->context);
        return err;
    }
    err = clBuildProgram(context->program, 1, &device, NULL, NULL, NULL);
    if (err < 0) {
        size_t size = 256*1024*sizeof(char);
        char *buildlog = malloc(size); 
        int err2 = clGetProgramBuildInfo(context->program, device, CL_PROGRAM_BUILD_LOG, size, buildlog, NULL); 
        if (err2 < 0) {
            printf("While handling failed build: error %d\n", err2);
        }
        printf("%s\n", buildlog);
        clReleaseContext(context->context);
        return err;
    }
    // Select available kernels. Order must confirm to `Kernels` enumeration.
    const char* kernel_names[KERNEL_COUNT] = {
        "mapIsland", 
        "mapZoomIsland",
        "mapAddIsland",
        "mapZoom",
        "mapRemoveTooMuchOcean",
        "mapAddSnow",
        "mapCoolWarm",
        "mapHeatIce",
        "mapSpecial",
        "mapAddMushroomIsland",
        "mapDeepOcean",
        "mapBiomes",
        "mapAddBamboo",
        "mapBiomeEdge",
        "mapRiverInit",
        "mapHills13",
        "removeBrim",
        "mapRareBiome",
        "mapShore",

        "setSeed"
    };
    for (int i=0; i<KERNEL_COUNT; ++i) {
        context->kernels[i] = clCreateKernel(context->program, kernel_names[i], &err);
        if (err < 0) {
            clReleaseContext(context->context);
            return err;
        }
    }

    return CL_SUCCESS;
}

void release_generator_context(struct GeneratorContext* context) {
    clReleaseContext(context->context);
}

/// Initialize world seeds in range of (seed..seed+seed_range)
cl_int set_world_seed(struct GeneratorContext* context, int64_t seed, cl_event* event) {
    cl_kernel kernel = context->kernels[SET_SEED];
    cl_int err = clSetKernelArg(kernel, 0, sizeof(int64_t), &seed);
    if (err < 0) return err;
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &context->layersBuffer);
    if (err < 0) return err;
    return clEnqueueNDRangeKernel(
        context->queue, kernel, 1, 
        NULL, &context->seed_range, NULL, 
        0, NULL, event
    );
}