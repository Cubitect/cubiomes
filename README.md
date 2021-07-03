# cubiomes

Cubiomes is a standalone library, written in C, that mimics the biome and feature generation of Minecraft Java Edition.
It is intended as a powerful tool to devise very fast, custom seed finding applications and large scale map viewers with minimal memory usage.


#### Cubiomes-Viewer

If you want to get started without coding there is now also a [graphical application](https://github.com/Cubitect/cubiomes-viewer) based on this library.


#### Audience

You should be familiar with the C programming language, also a basic understanding of the Minecraft biome generation process would be helpful.


## Getting Started

This section is meant to give you a quick starting point with small example programs if you want to use this library to find your own biome dependent features.

### Biome Generator

Let's create a simple program called `find_jedge.c` which tests seeds for a Junge Edge biome at a predefined location.

```C
// check the biome at a block position
#include "finders.h"
#include <stdio.h>

int main()
{
    // Initialize a stack of biome layers that reflects the biome generation of
    // Minecraft 1.17
    LayerStack g;
    setupGenerator(&g, MC_1_17);

    // seeds are internally represented as unsigned 64-bit integers
    uint64_t seed;
    Pos pos = {0,0}; // block position to be checked

    for (seed = 0; ; seed++)
    {
        // Go through the layers in the layer stack and initialize the seed
        // dependent aspects of the generator.
        applySeed(&g, seed);

        // To get the biome at single block position we can use getBiomeAtPos().
        int biomeID = getBiomeAtPos(&g, pos);
        if (biomeID == jungle_edge)
            break;
    }

    printf("Seed %" PRId64 " has a Junge Edge biome at block position "
        "(%d, %d).\n", (int64_t) seed, pos.x, pos.z);

    return 0;
}
```

You can compile this code either by directly adding a target to the makefile, or you can compile and link to a cubiomes archive:
```
$ cd cubiomes
$ make libcubiomes
```
To compile, and link the cubiomes library you can use one of
```
$ gcc find_jedge.c libcubiomes.a -fwrapv -lm   # static
$ gcc find_jedge.c -L. -lcubiomes -fwrapv -lm  # dynamic
```
Both commands assume that your source code is saved as `find_jedge.c` in the cubiomes working directory. If your makefile is configured to use pthreads you also may need to add the `-lpthread` option to the compiler.
The option `-fwrapv` enforces two's complement for signed integer overflow, which this library relies on. It is not strictly necessary for this example as the library should already be compiled with this flag, but it is good practice to prevent undefined behaviour.
Running the program should output:
```
$ ./a.out
Seed 615 has a Junge Edge biome at block position (0, 0).
```

We can also generate the biomes for a rectangular region using `genArea()` which also offers control over the entry layer, see the layer documentation for more information.

```C
// generate an image of the world
#include "generator.h"
#include "util.h"

int main()
{
    unsigned char biomeColors[256][3];

    // Initialize a color map for biomes.
    initBiomeColors(biomeColors);

    // Initialize a stack of biome layers.
    LayerStack g;
    setupGenerator(&g, MC_1_17);
    // Extract the desired layer.
    Layer *layer = &g.layers[L_SHORE_16];

    uint64_t seed = 1661454332289LL;
    int areaX = -60, areaZ = -60;
    unsigned int areaWidth = 120, areaHeight = 120;
    unsigned int scale = 4;
    unsigned int imgWidth = areaWidth*scale, imgHeight = areaHeight*scale;

    // Allocate a sufficient buffer for the biomes and for the image pixels.
    int *biomeIds = allocCache(layer, areaWidth, areaHeight);
    unsigned char *rgb = (unsigned char *) malloc(3*imgWidth*imgHeight);

    // Apply the seed only for the required layers and generate the area.
    setLayerSeed(layer, seed);
    genArea(layer, biomeIds, areaX, areaZ, areaWidth, areaHeight);

    // Map the biomes to a color buffer and save to an image.
    biomesToImage(rgb, biomeColors, biomeIds, areaWidth, areaHeight, scale, 2);
    savePPM("biomes_at_layer.ppm", rgb, imgWidth, imgHeight);

    // Clean up.
    free(biomeIds);
    free(rgb);

    return 0;
}
```


#### Layer Documentation

There is a reference document for the generator layers which contains a summary for most generator layers and their function within the generation process (a little out of date, since 1.13).


#### Biome Filters

Biome filters provide a way of generating an area, but only if that area contains certain biomes. Rather than generating an area first and then checking that it contains what we want, the requirements are tested during the generation process. This can be a dramatic speed up, particularly if we require several wildly different biomes.

```C
// find seeds that have certain biomes near the origin
#include "finders.h"
#include <stdio.h>

int main()
{
    int mc = MC_1_17;
    LayerStack g;
    BiomeFilter filter;

    setupGenerator(&g, mc);

    // Define the required biomes.
    int wanted[] = {
        dark_forest,
        ice_spikes,
        mushroom_fields,
    };
    filter = setupBiomeFilter(wanted, sizeof(wanted) / sizeof(int));

    int x = -200, z = -200, w = 400, h = 400;
    int entry = L_VORONOI_1;
    int *area = allocCache(&g.layers[entry], w, h);

    printf("Searching...\n");
    uint64_t seed;
    for (seed = 0; ; seed++)
        if (checkForBiomes(&g, entry, area, seed, x, z, w, h, filter, 1) > 0)
            break;

    printf("Seed %" PRId64 " has the required biomes in (%d, %d) - (%d, %d).\n",
        (int64_t) seed, x, z, x+w, z+h);

    free(area);
    return 0;
}
```


### Structure Generation

The generation of structures can usually be regarded as a two stage process: generation attempts and biome checks. For most structures, Minecraft divides the world into a grid of regions (usually 32x32 chunks) and performs one generation attempt in each. We can use `getStructurePos` to get the position of such a generation attempt and then test whether a structure will actually generate there with `isViableStructurePos`, however, this is more expensive to compute (a few µsec rather than nsec).

```C
// find a seed with a certain structure at the origin chunk
#include "finders.h"
#include <stdio.h>

int main()
{
    int structType = Outpost;
    int mc = MC_1_17;

    LayerStack g;
    setupGenerator(&g, mc);

    uint64_t lower48;
    for (lower48 = 0; ; lower48++)
    {
        // The structure position depends only on the region coordinates and
        // the lower 48-bits of the world seed.
        Pos p;
        if (!getStructurePos(structType, mc, lower48, 0, 0, &p))
            continue;

        // Look for a seed with the structure at the origin chunk.
        if (p.x >= 16 || p.z >= 16)
            continue;

        // Look for a full 64-bit seed with viable biomes.
        uint64_t upper16;
        for (upper16 = 0; upper16 < 0x10000; upper16++)
        {
            uint64_t seed = lower48 | (upper16 << 48);
            if (isViableStructurePos(structType, mc, &g, seed, p.x, p.z))
            {
                printf("Seed %" PRId64 " has a Pillager Outpost at (%d, %d).\n",
                    (int64_t) seed, p.x, p.z);
                return 0;
            }
        }
    }
}
```

#### Quad-Witch-Huts

A commonly desired feature are Quad-Witch-Huts or similar multi-structure clusters. To test for these types of seeds we can look a little deeper into how the generation attemps are determined. Notice that the positions depend only on the structure type, region coordinates and the lower 48 bits of the seed. Also, once we have found a seed with the desired generation attemps, we can move them around by transforming the 48-bit seed using `moveStructure`. This means there is a set of seed bases which can function as a starting point to generate all other seeds with similar structure placement.

The function `searchAll48` can be used to find a complete set of 48-bit seed bases for a custom criterion. Given that in general it can take a very long time to check all 2^48 seeds (days or weeks), the function provides some functionality to save the results to disk which can be loaded again using `loadSavedSeeds`. Luckly, in some cases it is possible to reduce the search space even further. For Swamp Huts and structures with a similar structure configuration there are only a handfull of constellations where the structures are close enough together to run simultaneously. Conveniently, these constellations differ uniquely at the lower 20 bits. (This is hard to prove, or at least I haven't found a riggerous proof that doesn't rely on brute forcing.) By specifying a list of lower 20-bit values we can reduce the search space to the order of 2^28, which can be checked in a reasonable amount of time.


```C
// find seeds with a quad-witch-hut about the origin
#include "finders.h"
#include <stdio.h>

int check(uint64_t s48, void *data)
{
    const StructureConfig sconf = *(const StructureConfig*) data;
    return isQuadBase(sconf, s48 - sconf.salt, 128);
}

int main()
{
    int styp = Swamp_Hut;
    int mc = MC_1_17;
    uint64_t basecnt = 0;
    uint64_t *bases = NULL;
    int threads = 8;
    LayerStack g;

    StructureConfig sconf;
    getStructureConfig(styp, mc, &sconf);

    printf("Preparing seed bases...\n");
    // Get all 48-bit quad-witch-hut bases, but consider only the best 20-bit
    // constellations where the structures are the closest together.
    int err = searchAll48(
        &bases, &basecnt, NULL, threads,
        low20QuadIdeal, sizeof(low20QuadIdeal) / sizeof(uint64_t), 20,
        check, &sconf
        );

    if (err || !bases)
    {
        printf("Failed to generate seed bases.\n");
        exit(1);
    }

    setupGenerator(&g, mc);

    uint64_t i;
    for (i = 0; i < basecnt; i++)
    {
        // The quad bases by themselves have structures in regions (0,0)-(1,1)
        // so we can move them by -1 regions to have them around the origin.
        uint64_t s48 = moveStructure(bases[i] - sconf.salt, -1, -1);

        Pos pos[4];
        getStructurePos(styp, mc, s48, -1, -1, &pos[0]);
        getStructurePos(styp, mc, s48, -1,  0, &pos[1]);
        getStructurePos(styp, mc, s48,  0, -1, &pos[2]);
        getStructurePos(styp, mc, s48,  0,  0, &pos[3]);

        uint64_t high;
        for (high = 0; high < 0x10000; high++)
        {
            uint64_t seed = s48 | (high << 48);

            if (isViableStructurePos(styp, mc, &g, seed, pos[0].x, pos[0].z) &&
                isViableStructurePos(styp, mc, &g, seed, pos[1].x, pos[1].z) &&
                isViableStructurePos(styp, mc, &g, seed, pos[2].x, pos[2].z) &&
                isViableStructurePos(styp, mc, &g, seed, pos[3].x, pos[3].z))
            {
                printf("%" PRId64 "\n", (int64_t) seed);
            }
        }
    }

    free(bases);
    return 0;
}
```

#### Strongholds and Spawn

Strongholds as well as the world spawn point actually search until they find a suitable location, rather than checking a single spot like most other structures. This causes them to be particularly performance expensive to find. Furthermore, the positions of stongholds have to be generated in a certain order, which can be done in an iterator fashion with `initFirstStronghold` and `nextStronghold`. For the world spawn, the generation starts with a search for a suitable biome near the origin, but will continue until a grass or podzol block is found. There is no reliable way of checking actual blocks, which means the search relies on a statistic, matching grass presence to biomes. Alternatively, we can simply use `estimateSpawn` and terminate the search after the first biome check and assume that grass is near by.


```C
// find spawn and the first N strongholds
#include "finders.h"
#include <stdio.h>

int main()
{
    int mc = MC_1_17;
    uint64_t seed = 3055141959546LL;

    // Only the first stronghold has a position which can be estimated
    // (+/-112 blocks) without biome check.
    StrongholdIter sh;
    Pos pos = initFirstStronghold(&sh, mc, seed);

    printf("Seed: %" PRId64 "\n", (int64_t) seed);
    printf("Estimated position of first stronghold: (%d, %d)\n", pos.x, pos.z);

    // The finders for the strongholds and spawn require that the seed is
    // applied to the generator beforehand.
    LayerStack g;
    setupGenerator(&g, mc);
    applySeed(&g, seed);

    pos = getSpawn(mc, &g, NULL, seed);
    printf("Spawn: (%d, %d)\n", pos.x, pos.z);

    int i, N = 12;
    for (i = 1; i <= N; i++)
    {
        if (nextStronghold(&sh, &g, NULL) <= 0)
            break;
        printf("Stronghold #%-3d: (%6d, %6d)\n", i, sh.pos.x, sh.pos.z);
    }

    return 0;
}
```





