# cubiomes

Cubiomes is a standalone library, written in C, that mimics the Minecraft biome and feature generation. 
It is intended as a powerful tool to devise very fast, custom seed finding applications and large scale map viewers.


#### Audience

You should be familiar with the C programming language, also a basic understanding of the Minecraft biome generation process would be helpful.


## Getting Started

This section is meant to give you a quick starting point if you want to use this library to find your own biome dependent features.

### Biome Generator

Let's create a simple program called `find_jedge.c` which tests seeds for a Junge Edge biome at a predefined location.

```C
#include "finders.h"
#include <stdio.h>

int main()
{
    // First initialize the global biome table 'int biomes[256]'. This sets up
    // properties such as the category and temperature of each biome.
    initBiomes();

    // Initialize a stack of biome layers that reflects the biome generation of
    // Minecraft 1.16
    LayerStack g;
    setupGenerator(&g, MC_1_16);

    int64_t seed;
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
        "(%d, %d).\n", seed, pos.x, pos.z);

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
$ gcc find_jedge.c libcubiomes.a -lm   # static
$ gcc find_jedge.c -L. -lcubiomes -lm  # dynamic
```
Both options assume that your source code is saved as `find_jedge.c` in the cubiomes working directory. If your makefile is configured to use pthreads you also may need to add the `-lpthread` option to the compiler. Running the program should output:
```
$ ./a.out
Seed 615 has a Junge Edge biome at block position (0, 0).
```

We can also generate the biomes for a rectangular region using `getArea()` which also offers control over the entry layer, see the layer documentation for more information.

```C
#include "generator.h"
#include "util.h"

int main()
{
    unsigned char biomeColours[256][3];

    // Initialize global biome table.
    initBiomes();
    // Initialize a colour map for biomes.
    initBiomeColours(biomeColours);

    // Initialize a stack of biome layers.
    LayerStack g;
    setupGenerator(&g, MC_1_16);
    // Extract the desired layer.
    Layer *layer = &g.layers[L_SHORE_16];

    int64_t seed = 1661454332289;
    int areaX = -60, areaZ = -60;
    unsigned int areaWidth = 120, areaHeight = 120;
    unsigned int scale = 4;
    unsigned int imgWidth = areaWidth*scale, imgHeight = areaHeight*scale;

    // Allocate a sufficient buffer for the biomes and for the image pixels.
    int *biomeIds = allocCache(layer, areaWidth, areaHeight);
    unsigned char *rgb = (unsigned char *) malloc(3*imgWidth*imgHeight);

    // Apply the seed only for the required layers and generate the area.
    setWorldSeed(layer, seed);
    genArea(layer, biomeIds, areaX, areaZ, areaWidth, areaHeight);

    // Map the biomes to a color buffer and save to an image.
    biomesToImage(rgb, biomeColours, biomeIds, areaWidth, areaHeight, scale, 2);
    savePPM("biomes_at_layer.ppm", rgb, imgWidth, imgHeight);

    // Clean up.
    free(biomeIds);
    free(rgb);

    return 0;
}
```


### Layer Documentation

There is a reference document for the generator layers which contains a summary for most generator layers and their function within the generation process.


### Examples

There are two example programs in this repository which can be compiled using the makefile.


#### Finding Quad-Witch-Huts at a Specific Location

This classic type of finder uses several optimisations reguarding positioning of temples in the world. One of which allows you to specify the exact region (512x512) position about which the quad-hut should generate, without affecting the performance. For example:

`./find_quadhut 0 0`

will start a search with a regional positioning around the origin. (Actually the huts will be positioned in regions (-1,-1) to (0,0) this way.)

To my knowlege, as of the time of writing, this is fastest single-thread quad-hut-finder out there. However, note that the current implementation of the biome finding optimisations causes the finder to miss some seeds (< 2%) in favour for speed.


#### Finding Compact Biome Seeds

This finder searches for seeds that contain all major biome types within 1024 blocks of the origin. These seeds are very rare and it might take a moment for the finder to yield any. The commandline arguments are:

`./find_compactbiomes [starting_seed] [end_seed] [threads]`


# Cool Seeds

## All Biomes Near the Origin

Below is a list of some very rare seeds that have all the interesing biomes in very close proximity to the origin, offering some unique scenery. TIP: If you are creating a new world you can use the Custom World Generation setting to reduce the biome size by a factor of up to 4, which puts all the biomes even closer together.

<table>
<tr>
  <th>Seed</th>
  <th>Notable biomes within 250 blocks of spawn</th>
  <th>Remarks</th>
</tr>
<tr>
  <td>2458028242930</td>
  <td>Ocean, Plains, Forest, Taiga, Swampland,<br>Ice Plains, Roofed Forest, Cold Taiga,<br>Mega Taiga, Sunflower Plains, Ice Spikes</td>
  <td>
   <ul>
    <li>Decently sized Mushroom Island </li>
   </ul>
  </td>
</tr>
<tr>
  <td>3659308845421</td>
  <td>Ocean, Plains, Extreme Hills, Forest, Taiga,<br>Swampland, Deep Ocean, Mega Taiga,<br>Sunflower Plains</td>
  <td>
  </td>
</tr>
<tr>
  <td>3055141959546</td>
  <td>Ocean, Plains, Extreme Hills, Forest, Taiga,<br>Deep Ocean, Flower Forest</td>
  <td>
   <ul>
    <li>All biomes are arranged in a<br>ring around a central ocean</li>
    <li>Well suited for custom worlds</li>
   </ul>
  </td>
</tr>
<tr>
  <td>1618761219563</td>
  <td>Ocean, Plains, Extreme Hills, Forest, Swampland,<br>Ice Plains, Mushroom Island, Deep Ocean,<br>Birch Forest, Roofed Forest, Savanna</td>
  <td>
   <ul>
    <li>Village at spawn</li>
   </ul>
  </td>
</tr>
<tr>
  <td>1661454332289</td>
  <td>Ocean, Plains, Desert, Mushroom Island,<br>Deep Ocean, Roofed Forest, Savanna</td>
  <td>
   <ul>
    <li>All biomes are arranged in a<br>ring around a central ocean</li>
    <li>Mushroom Island at (0,0)</li>
    <li>Extremely Small Ice Spike Biome</li>
   </ul>
  </td>
</tr>

</table>



## Lots of Witch Huts

No more than four huts can generate close enough together to be operated by a single player. However, if you have a server, or you just want multiple witch farms in your world, then you can consider seeds with more than one multi-hut. 

For seeds that have a quad-hut with 2 additional tri-huts, I present to you (possibly all) seeds that have the multi-huts in the closest proximity to spawn. These seeds have one quad-hut and 2 tri-huts within 40000 blocks.

| Seed                 | Quad Hut      | Tri Hut #1     | Tri Hut #2     |
|----------------------|---------------|----------------|----------------|
|   181201211981019340 | ( 5480, 4984) | (10616, -8344) | (-23688,28520) |
|  2178171917826985089 | (-3736,-3720) | ( 1400,-17048) | (-32904,19816) |
| -2263221739690455935 | (-3736,-3720) | ( 1400,-17048) | (-32904,19816) |
|  3382334921639955859 | (28008, -648) | (33144,-13976) | ( -1160,22888) |
| -8386696804585992813 | (28008, -648) | (33144,-13976) | ( -1160,22888) |
|  4027541812768105332 | (-5784,-7304) | ( -648,-20632) | (-34952,16232) |

For a mostly complete list of double-quad-huts within 32000 blocks see: `./seeds/alldoublequadhuts32k.txt`.



## Advancement Related

If you are looking to get the "Adventuring Time" achievment you might consider one of the following seeds. All of these seeds have all 36 required biomes within less than 650 blocks from the origin: 

| Seed          | All biome radius |
|---------------|------------------|
| -880424771806 | 644              |
| 48382691805   | 633              |
| 480800992945  | 649              |
| 1065757415811 | 612              |
| 1509124018794 | 645              |
| 1550633690354 | 616              |
| 1571479851306 | 631              |
| 1925837979058 | 621              |
| 2082386353360 | 649              |
| 2087339213306 | 632              |
| 2810140768300 | 637              |
| 3053313529066 | 648              |
| 3457626356584 | 649              |
| 3548624619264 | 646              |







