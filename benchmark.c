#include "finders.h"
#include "generator.h"
#include "layers.h"

#include <stdio.h>
#include <time.h>

#define COUNT 100
#define RADIUS 1024


void bench(LayerStack g, char *ver) {
    Layer *quarterRes = &g.layers[g.layerNum-2];
    int *cache = allocCache(quarterRes, RADIUS*2, RADIUS*2);
    time_t start = time(NULL);
    for (int64_t x=0; x<COUNT; x++) {
        applySeed(&g, x);
        genArea(quarterRes, cache, -RADIUS, -RADIUS, RADIUS*2, RADIUS*2);
    }
    printf("%s - %d in %ld seconds.\n", ver, COUNT, time(NULL)-start);
    free(cache);
}


int main(int argc, char *argv[]) {
    initBiomes();

    LayerStack g_1_7 = setupGeneratorMC17();
    LayerStack g_1_13 = setupGeneratorMC113();

    bench(g_1_7, "1.7");
    bench(g_1_13, "1.13");

    return 0;
}
