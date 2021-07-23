#include "generator.h"
#include "layers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Layer *setupLayer(LayerStack *g, int layerId, mapfunc_t *map, int mc,
    int8_t zoom, int8_t edge, uint64_t saltbase, Layer *p, Layer *p2)
{
    Layer *l = g->layers + layerId;
    l->getMap = map;
    l->mc = mc;
    l->zoom = zoom;
    l->edge = edge;
    l->scale = 0;
    if (saltbase == 0 || saltbase == LAYER_INIT_SHA)
        l->layerSalt = saltbase;
    else
        l->layerSalt = getLayerSalt(saltbase);
    l->startSalt = 0;
    l->startSeed = 0;
    l->noise = NULL;
    l->data = NULL;
    l->p = p;
    l->p2 = p2;
    return l;
}

static void setupScale(Layer *l, int scale)
{
    l->scale = scale;
    if (l->p)
        setupScale(l->p, scale * l->zoom);
    if (l->p2)
        setupScale(l->p2, scale * l->zoom);
}

void setupGeneratorLargeBiomes(LayerStack *g, int mc, int largeBiomes)
{
    if (mc < MC_1_3)
        largeBiomes = 0;

    memset(g, 0, sizeof(LayerStack));
    Layer *p;

    // G: generator layer stack
    // L: layer ID
    // M: mapping function
    // V: minecraft version
    // Z: zoom
    // E: edge
    // S: salt base
    // P1: parent 1
    // P2: parent 2

    //             G, L,                    M               V   Z  E  S     P1 P2
    p = setupLayer(g, L_CONTINENT_4096,     mapContinent,   mc, 1, 0, 1,    0, 0);
    p = setupLayer(g, L_ZOOM_2048,          mapZoomFuzzy,   mc, 2, 3, 2000, p, 0);
    p = setupLayer(g, L_LAND_2048,          mapLand,        mc, 1, 2, 1,    p, 0);
    p = setupLayer(g, L_ZOOM_1024,          mapZoom,        mc, 2, 3, 2001, p, 0);
    p = setupLayer(g, L_LAND_1024_A,        mapLand,        mc, 1, 2, 2,    p, 0);

    if (mc <= MC_1_6)
    {   //             G  L                 M               V   Z  E  S     P1 P2
        p = setupLayer(g, L_SNOW_1024,      mapSnow16,      mc, 1, 2, 2,    p, 0);
        p = setupLayer(g, L_ZOOM_512,       mapZoom,        mc, 2, 3, 2002, p, 0);
        p = setupLayer(g, L_LAND_512,       mapLand16,      mc, 1, 2, 3,    p, 0);
        p = setupLayer(g, L_ZOOM_256,       mapZoom,        mc, 2, 3, 2003, p, 0);
        p = setupLayer(g, L_LAND_256,       mapLand16,      mc, 1, 2, 4,    p, 0);
        p = setupLayer(g, L_MUSHROOM_256,   mapMushroom,    mc, 1, 2, 5,    p, 0);
        p = setupLayer(g, L_BIOME_256,      mapBiome,       mc, 1, 0, 200,  p, 0);
        p = setupLayer(g, L_ZOOM_128,       mapZoom,        mc, 2, 3, 1000, p, 0);
        p = setupLayer(g, L_ZOOM_64,        mapZoom,        mc, 2, 3, 1001, p, 0);
        // river noise layer chain, also used to determine where hills generate
        p = setupLayer(g, L_NOISE_256,      mapNoise,       mc, 1, 0, 100,
                       g->layers+L_MUSHROOM_256, 0);
    }
    else
    {   //             G  L                 M               V   Z  E  S     P1 P2
        p = setupLayer(g, L_LAND_1024_B,    mapLand,        mc, 1, 2, 50,   p, 0);
        p = setupLayer(g, L_LAND_1024_C,    mapLand,        mc, 1, 2, 70,   p, 0);
        p = setupLayer(g, L_ISLAND_1024,    mapIsland,      mc, 1, 2, 2,    p, 0);
        p = setupLayer(g, L_SNOW_1024,      mapSnow,        mc, 1, 2, 2,    p, 0);
        p = setupLayer(g, L_LAND_1024_D,    mapLand,        mc, 1, 2, 3,    p, 0);
        p = setupLayer(g, L_COOL_1024,      mapCool,        mc, 1, 2, 2,    p, 0);
        p = setupLayer(g, L_HEAT_1024,      mapHeat,        mc, 1, 2, 2,    p, 0);
        p = setupLayer(g, L_SPECIAL_1024,   mapSpecial,     mc, 1, 2, 3,    p, 0);
        p = setupLayer(g, L_ZOOM_512,       mapZoom,        mc, 2, 3, 2002, p, 0);
        p = setupLayer(g, L_ZOOM_256,       mapZoom,        mc, 2, 3, 2003, p, 0);
        p = setupLayer(g, L_LAND_256,       mapLand,        mc, 1, 2, 4,    p, 0);
        p = setupLayer(g, L_MUSHROOM_256,   mapMushroom,    mc, 1, 2, 5,    p, 0);
        p = setupLayer(g, L_DEEP_OCEAN_256, mapDeepOcean,   mc, 1, 2, 4,    p, 0);
        p = setupLayer(g, L_BIOME_256,      mapBiome,       mc, 1, 0, 200,  p, 0);
        if (mc >= MC_1_14)
            p = setupLayer(g, L_BAMBOO_256, mapBamboo,      mc, 1, 0, 1001, p, 0);
        p = setupLayer(g, L_ZOOM_128,       mapZoom,        mc, 2, 3, 1000, p, 0);
        p = setupLayer(g, L_ZOOM_64,        mapZoom,        mc, 2, 3, 1001, p, 0);
        p = setupLayer(g, L_BIOME_EDGE_64,  mapBiomeEdge,   mc, 1, 2, 1000, p, 0);
        // river noise layer chain, also used to determine where hills generate
        p = setupLayer(g, L_RIVER_INIT_256, mapNoise,       mc, 1, 0, 100,
                       g->layers+L_DEEP_OCEAN_256, 0);
    }

    if (mc <= MC_1_12)
    {
        p = setupLayer(g, L_ZOOM_128_HILLS, mapZoom,        mc, 2, 3, 0,    p, 0);
        p = setupLayer(g, L_ZOOM_64_HILLS,  mapZoom,        mc, 2, 3, 0,    p, 0);
    }
    else if (mc >= MC_1_1)
    {
        p = setupLayer(g, L_ZOOM_128_HILLS, mapZoom,        mc, 2, 3, 1000, p, 0);
        p = setupLayer(g, L_ZOOM_64_HILLS,  mapZoom,        mc, 2, 3, 1001, p, 0);
    }

    if (mc <= MC_1_0)
    {   //             G  L                 M               V   Z  E  S     P1 P2
        p = setupLayer(g, L_ZOOM_32,        mapZoom,        mc, 2, 3, 1000,
                g->layers+L_ZOOM_64, 0);
        p = setupLayer(g, L_LAND_32,        mapLand16,      mc, 1, 2, 3,    p, 0);
        // NOTE: reusing slot for shore:16, but scale is 1:32
        p = setupLayer(g, L_SHORE_16,       mapShore,       mc, 1, 2, 1000, p, 0);
        p = setupLayer(g, L_ZOOM_16,        mapZoom,        mc, 2, 3, 1001, p, 0);
        p = setupLayer(g, L_ZOOM_8,         mapZoom,        mc, 2, 3, 1002, p, 0);
        p = setupLayer(g, L_ZOOM_4,         mapZoom,        mc, 2, 3, 1003, p, 0);

        p = setupLayer(g, L_SMOOTH_4,       mapSmooth,      mc, 1, 2, 1000, p, 0);

        // river layer chain
        p = setupLayer(g, L_ZOOM_128_RIVER, mapZoom,        mc, 2, 3, 1000,
                       g->layers+L_NOISE_256, 0);
        p = setupLayer(g, L_ZOOM_64_RIVER,  mapZoom,        mc, 2, 3, 1001, p, 0);
        p = setupLayer(g, L_ZOOM_32_RIVER,  mapZoom,        mc, 2, 3, 1002, p, 0);
        p = setupLayer(g, L_ZOOM_16_RIVER,  mapZoom,        mc, 2, 3, 1003, p, 0);
        p = setupLayer(g, L_ZOOM_8_RIVER,   mapZoom,        mc, 2, 3, 1004, p, 0);
        p = setupLayer(g, L_ZOOM_4_RIVER,   mapZoom,        mc, 2, 3, 1005, p, 0);

        p = setupLayer(g, L_RIVER_4,        mapRiver,       mc, 1, 2, 1,    p, 0);
        p = setupLayer(g, L_SMOOTH_4_RIVER, mapSmooth,      mc, 1, 2, 1000, p, 0);
    }
    else if (mc <= MC_1_6)
    {
        p = setupLayer(g, L_HILLS_64,       mapHills,       mc, 1, 2, 1000,
                g->layers+L_ZOOM_64, g->layers+L_ZOOM_64_HILLS);

        p = setupLayer(g, L_ZOOM_32,        mapZoom,        mc, 2, 3, 1000, p, 0);
        p = setupLayer(g, L_LAND_32,        mapLand16,      mc, 1, 2, 3,    p, 0);
        p = setupLayer(g, L_ZOOM_16,        mapZoom,        mc, 2, 3, 1001, p, 0);
        p = setupLayer(g, L_SHORE_16,       mapShore,       mc, 1, 2, 1000, p, 0);
        p = setupLayer(g, L_SWAMP_RIVER_16, mapSwampRiver,  mc, 1, 0, 1000, p, 0);
        p = setupLayer(g, L_ZOOM_8,         mapZoom,        mc, 2, 3, 1002, p, 0);
        p = setupLayer(g, L_ZOOM_4,         mapZoom,        mc, 2, 3, 1003, p, 0);

        if (largeBiomes)
        {
            p = setupLayer(g, L_ZOOM_LARGE_A, mapZoom,      mc, 2, 3, 1004, p, 0);
            p = setupLayer(g, L_ZOOM_LARGE_B, mapZoom,      mc, 2, 3, 1005, p, 0);
        }

        p = setupLayer(g, L_SMOOTH_4,       mapSmooth,      mc, 1, 2, 1000, p, 0);

        // river layer chain
        p = setupLayer(g, L_ZOOM_128_RIVER, mapZoom,        mc, 2, 3, 1000,
                g->layers+L_NOISE_256, 0);
        p = setupLayer(g, L_ZOOM_64_RIVER,  mapZoom,        mc, 2, 3, 1001, p, 0);
        p = setupLayer(g, L_ZOOM_32_RIVER,  mapZoom,        mc, 2, 3, 1002, p, 0);
        p = setupLayer(g, L_ZOOM_16_RIVER,  mapZoom,        mc, 2, 3, 1003, p, 0);
        p = setupLayer(g, L_ZOOM_8_RIVER,   mapZoom,        mc, 2, 3, 1004, p, 0);
        p = setupLayer(g, L_ZOOM_4_RIVER,   mapZoom,        mc, 2, 3, 1005, p, 0);

        if (largeBiomes)
        {
            p = setupLayer(g, L_ZOOM_L_RIVER_A, mapZoom,    mc, 2, 3, 1006, p, 0);
            p = setupLayer(g, L_ZOOM_L_RIVER_B, mapZoom,    mc, 2, 3, 1007, p, 0);
        }

        p = setupLayer(g, L_RIVER_4,        mapRiver,       mc, 1, 2, 1,    p, 0);
        p = setupLayer(g, L_SMOOTH_4_RIVER, mapSmooth,      mc, 1, 2, 1000, p, 0);
    }
    else
    {
        p = setupLayer(g, L_HILLS_64,       mapHills,       mc, 1, 2, 1000,
                g->layers+L_BIOME_EDGE_64, g->layers+L_ZOOM_64_HILLS);

        p = setupLayer(g, L_SUNFLOWER_64,   mapSunflower,   mc, 1, 0, 1001, p, 0);
        p = setupLayer(g, L_ZOOM_32,        mapZoom,        mc, 2, 3, 1000, p, 0);
        p = setupLayer(g, L_LAND_32,        mapLand,        mc, 1, 2, 3,    p, 0);
        p = setupLayer(g, L_ZOOM_16,        mapZoom,        mc, 2, 3, 1001, p, 0);
        p = setupLayer(g, L_SHORE_16,       mapShore,       mc, 1, 2, 1000, p, 0);
        p = setupLayer(g, L_ZOOM_8,         mapZoom,        mc, 2, 3, 1002, p, 0);
        p = setupLayer(g, L_ZOOM_4,         mapZoom,        mc, 2, 3, 1003, p, 0);

        if (largeBiomes)
        {
            p = setupLayer(g, L_ZOOM_LARGE_A, mapZoom,      mc, 2, 3, 1004, p, 0);
            p = setupLayer(g, L_ZOOM_LARGE_B, mapZoom,      mc, 2, 3, 1005, p, 0);
        }

        p = setupLayer(g, L_SMOOTH_4,       mapSmooth,      mc, 1, 2, 1000, p, 0);

        // river layer chain
        p = setupLayer(g, L_ZOOM_128_RIVER, mapZoom,        mc, 2, 3, 1000,
                g->layers+L_RIVER_INIT_256, 0);
        p = setupLayer(g, L_ZOOM_64_RIVER,  mapZoom,        mc, 2, 3, 1001, p, 0);
        p = setupLayer(g, L_ZOOM_32_RIVER,  mapZoom,        mc, 2, 3, 1000, p, 0);
        p = setupLayer(g, L_ZOOM_16_RIVER,  mapZoom,        mc, 2, 3, 1001, p, 0);
        p = setupLayer(g, L_ZOOM_8_RIVER,   mapZoom,        mc, 2, 3, 1002, p, 0);
        p = setupLayer(g, L_ZOOM_4_RIVER,   mapZoom,        mc, 2, 3, 1003, p, 0);

        if (largeBiomes && mc == MC_1_7)
        {
            p = setupLayer(g, L_ZOOM_L_RIVER_A, mapZoom,    mc, 2, 3, 1004, p, 0);
            p = setupLayer(g, L_ZOOM_L_RIVER_B, mapZoom,    mc, 2, 3, 1005, p, 0);
        }

        p = setupLayer(g, L_RIVER_4,        mapRiver,       mc, 1, 2, 1,    p, 0);
        p = setupLayer(g, L_SMOOTH_4_RIVER, mapSmooth,      mc, 1, 2, 1000, p, 0);
    }

    p = setupLayer(g, L_RIVER_MIX_4, mapRiverMix, mc, 1, 0, 100,
            g->layers+L_SMOOTH_4, g->layers+L_SMOOTH_4_RIVER);


    if (mc <= MC_1_12)
    {
        p = setupLayer(g, L_VORONOI_1, mapVoronoi114, mc, 4, 7, 10, p, 0);
    }
    else
    {
        // ocean variants
        p = setupLayer(g, L_OCEAN_TEMP_256, mapOceanTemp,   mc, 1, 0, 2,    0, 0);
        p->noise = &g->oceanRnd;
        p = setupLayer(g, L_ZOOM_128_OCEAN, mapZoom,        mc, 2, 3, 2001, p, 0);
        p = setupLayer(g, L_ZOOM_64_OCEAN,  mapZoom,        mc, 2, 3, 2002, p, 0);
        p = setupLayer(g, L_ZOOM_32_OCEAN,  mapZoom,        mc, 2, 3, 2003, p, 0);
        p = setupLayer(g, L_ZOOM_16_OCEAN,  mapZoom,        mc, 2, 3, 2004, p, 0);
        p = setupLayer(g, L_ZOOM_8_OCEAN,   mapZoom,        mc, 2, 3, 2005, p, 0);
        p = setupLayer(g, L_ZOOM_4_OCEAN,   mapZoom,        mc, 2, 3, 2006, p, 0);
        p = setupLayer(g, L_OCEAN_MIX_4,    mapOceanMix,    mc, 1, 17, 100,
                g->layers+L_RIVER_MIX_4, g->layers+L_ZOOM_4_OCEAN);

        if (mc <= MC_1_14)
            p = setupLayer(g, L_VORONOI_1, mapVoronoi114, mc, 4, 7, 10, p, 0);
        else
            p = setupLayer(g, L_VORONOI_1, mapVoronoi, mc, 4, 7, LAYER_INIT_SHA, p, 0);
    }

    g->entry_1 = p;
    g->entry_4 = g->layers + (mc <= MC_1_12 ? L_RIVER_MIX_4 : L_OCEAN_MIX_4);
    if (largeBiomes)
    {
        g->entry_16 = g->layers + L_ZOOM_4;
        g->entry_64 = g->layers + (mc <= MC_1_6 ? L_SWAMP_RIVER_16 : L_SHORE_16);
        g->entry_256 = g->layers + (mc <= MC_1_7 ? L_HILLS_64 : L_SUNFLOWER_64);
    }
    else if (mc >= MC_1_1)
    {
        g->entry_16 = g->layers + (mc <= MC_1_6 ? L_SWAMP_RIVER_16 : L_SHORE_16);
        g->entry_64 = g->layers + (mc <= MC_1_7 ? L_HILLS_64 : L_SUNFLOWER_64);
        g->entry_256 = g->layers + (mc <= MC_1_14 ? L_BIOME_256 : L_BAMBOO_256);
    }
    else
    {
        g->entry_16 = g->layers + L_ZOOM_16;
        g->entry_64 = g->layers + L_ZOOM_64;
        g->entry_256 = g->layers + L_BIOME_256;
    }
    setupScale(g->entry_1, 1);
}

void setupGenerator(LayerStack *g, int mc)
{
    setupGeneratorLargeBiomes(g, mc, 0);
}

/* Recursively calculates the minimum buffer size required to generate an area
 * of the specified size from the current layer onwards.
 */
static void getMaxArea(const Layer *layer, int areaX, int areaZ, int *maxX, int *maxZ)
{
    if (layer == NULL)
        return;

    if (layer->zoom == 2)
    {
        areaX >>= 1;
        areaZ >>= 1;
    }
    else if (layer->zoom == 4)
    {
        areaX >>= 2;
        areaZ >>= 2;
    }

    areaX += layer->edge;
    areaZ += layer->edge;

    if (areaX > *maxX) *maxX = areaX;
    if (areaZ > *maxZ) *maxZ = areaZ;

    getMaxArea(layer->p, areaX, areaZ, maxX, maxZ);
    getMaxArea(layer->p2, areaX, areaZ, maxX, maxZ);
}

int calcRequiredBuf(const Layer *layer, int areaX, int areaZ)
{
    int maxX = areaX, maxZ = areaZ;
    getMaxArea(layer, areaX, areaZ, &maxX, &maxZ);

    return maxX * maxZ;
}

int *allocCache(const Layer *layer, int sizeX, int sizeZ)
{
    int size = calcRequiredBuf(layer, sizeX, sizeZ);

    int *ret = (int *) malloc(sizeof(*ret)*size);
    memset(ret, 0, sizeof(*ret)*size);

    return ret;
}


void applySeed(LayerStack *g, uint64_t seed)
{
    // the seed has to be applied recursively
    setLayerSeed(g->entry_1, seed);
}

int genArea(const Layer *layer, int *out, int areaX, int areaZ, int areaWidth, int areaHeight)
{
    memset(out, 0, areaWidth*areaHeight*sizeof(*out));
    return layer->getMap(layer, out, areaX, areaZ, areaWidth, areaHeight);
}



int genNetherScaled(int mc, uint64_t seed, int scale, int *out,
        int x, int z, int w, int h, int y0, int y1)
{
    if (scale != 1 && scale != 4 && scale != 16 && scale != 64)
        return 1; // unsupported scale

    if (mc < MC_1_16)
    {
        int i, siz = w*h*(y1-y0+1);
        for (i = 0; i < siz; i++)
            out[i] = nether_wastes;
        return 0;
    }

    NetherNoise nn;
    setNetherSeed(&nn, seed);

    if (scale == 1)
    {
        if (y0 != 0 || y1 != 0)
        {
            printf("getNetherScaled(): volume voronoi not implemented yet\n");
            return 1;
        }

        int vx = x - 2;
        int vz = z - 2;
        int pX = vx >> 2;
        int pZ = vz >> 2;
        int pW = ((vx + w) >> 2) - pX + 2;
        int pH = ((vz + h) >> 2) - pZ + 2;

        int err = mapNether2D(&nn, out, pX, pZ, pW, pH);
        if (err)
            return err;
        Layer lvoronoi;
        memset(&lvoronoi, 0, sizeof(Layer));
        lvoronoi.startSalt = getVoronoiSHA(seed);
        return mapVoronoi(&lvoronoi, out, x, z, w, h);
    }
    else
    {
        return mapNether3D(&nn, out, x, z, w, h, y0, y1-y0+1, scale, 1.0);
    }
}


int genEndScaled(int mc, uint64_t seed, int scale, int *out,
        int x, int z, int w, int h)
{
    if (scale != 1 && scale != 4 && scale != 16 && scale != 64)
        return 1; // unsupported scale

    if (mc < MC_1_9)
    {
        int i, siz = w*h;
        for (i = 0; i < siz; i++)
            out[i] = the_end;
        return 0;
    }

    EndNoise en;
    setEndSeed(&en, seed);

    if (scale == 1)
    {
        int vx = x - 2;
        int vz = z - 2;
        int pX = vx >> 2;
        int pZ = vz >> 2;
        int pW = ((vx + w) >> 2) - pX + 2;
        int pH = ((vz + h) >> 2) - pZ + 2;

        int err = mapEnd(&en, out, pX, pZ, pW, pH);
        if (err)
            return err;
        Layer lvoronoi;
        memset(&lvoronoi, 0, sizeof(Layer));
        if (mc >= MC_1_15)
        {
            lvoronoi.startSalt = getVoronoiSHA(seed);
            return mapVoronoi(&lvoronoi, out, x, z, w, h);
        }
        else
        {
            lvoronoi.startSalt = getLayerSalt(10);
            return mapVoronoi114(&lvoronoi, out, x, z, w, h);
        }
    }
    else if (scale == 4)
    {
        return mapEnd(&en, out, x, z, w, h);
    }
    else if (scale == 16)
    {
        return mapEndBiome(&en, out, x, z, w, h);
    }
    else if (scale == 64)
    {
        int i, j, di, dj;
        int r = 4;
        int hw = (2+w) * r + 1;
        int hh = (2+h) * r + 1;
        int16_t *hmap = (int16_t*) calloc(hw*hh, sizeof(*hmap));

        for (j = 0; j < h; j++)
        {
            for (i = 0; i < w; i++)
            {
                int64_t hx = (i+x) * r;
                int64_t hz = (j+z) * r;
                if (hx*hx + hz*hz <= 4096L)
                {
                    out[j*w+i] = the_end;
                    continue;
                }

                int64_t h = 64*16*16;

                for (dj = -r; dj < r; dj++)
                {
                    for (di = -r; di < r; di++)
                    {
                        int64_t rx = hx + di;
                        int64_t rz = hz + dj;
                        int hi = i*r + di+r;
                        int hj = j*r + dj+r;
                        int16_t *p = &hmap[hj*hw + hi];
                        if (*p == 0)
                        {
                            if (sampleSimplex2D(&en, rx, rz) < -0.9f)
                            {
                                *p = (llabs(rx) * 3439 + llabs(rz) * 147) % 13 + 9;
                                *p *= *p;
                            }
                            else
                            {
                                *p = -1;
                            }
                        }

                        if (*p > 0)
                        {
                            int64_t noise = 4*(di*di + dj*dj) * (*p);
                            if (noise < h)
                                h = noise;
                        }
                    }
                }

                if (h < 3600)
                    out[j*w+i] = end_highlands;
                else if (h <= 10000)
                    out[j*w+i] = end_midlands;
                else if (h <= 14400)
                    out[j*w+i] = end_barrens;
                else
                    out[j*w+i] = small_end_islands;
            }
        }
        free(hmap);
    }
    return 1;
}





