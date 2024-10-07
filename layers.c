#include "layers.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <float.h>


//==============================================================================
// Essentials
//==============================================================================

void setLayerSeed(Layer *layer, uint64_t worldSeed)
{
    if (layer->p2 != NULL)
        setLayerSeed(layer->p2, worldSeed);

    if (layer->p != NULL)
        setLayerSeed(layer->p, worldSeed);

    if (layer->noise != NULL)
    {
        uint64_t s;
        setSeed(&s, worldSeed);
        perlinInit((PerlinNoise*)layer->noise, &s);
    }

    uint64_t ls = layer->layerSalt;
    if (ls == 0)
    {   // Pre 1.13 the Hills branch stays zero-initialized
        layer->startSalt = 0;
        layer->startSeed = 0;
    }
    else if (ls == LAYER_INIT_SHA)
    {   // Post 1.14 Voronoi uses SHA256 for initialization
        layer->startSalt = getVoronoiSHA(worldSeed);
        layer->startSeed = 0;
    }
    else
    {
        uint64_t st = worldSeed;
        st = mcStepSeed(st, ls);
        st = mcStepSeed(st, ls);
        st = mcStepSeed(st, ls);

        layer->startSalt = st;
        layer->startSeed = mcStepSeed(st, 0);
    }
}


//==============================================================================
// Layers
//==============================================================================

// convenience function used in several layers
static inline int isAny4(int id, int a, int b, int c, int d)
{
    return id == a || id == b || id == c || id == d;
}

int mapContinent(const Layer * l, int * out, int x, int z, int w, int h)
{
    uint64_t ss = l->startSeed;
    uint64_t cs;
    int64_t i, j;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            cs = getChunkSeed(ss, i + x, j + z);
            out[j*w + i] = mcFirstIsZero(cs, 10);
        }
    }

    if (x > -w && x <= 0 && z > -h && z <= 0)
    {
        out[-z * (int64_t)w - x] = 1;
    }

    return 0;
}

int mapZoomFuzzy(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x >> 1;
    int pZ = z >> 1;
    int64_t pW = ((x + w) >> 1) - pX + 1;
    int64_t pH = ((z + h) >> 1) - pZ + 1;
    int64_t i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    int64_t newW = pW * 2;
    //int64_t newH = pH * 2;
    int64_t idx;
    int v00, v01, v10, v11;
    int *buf = out + pW * pH; //(int*) malloc((newW+1)*(newH+1)*sizeof(*buf));

    const uint32_t st = (uint32_t)l->startSalt;
    const uint32_t ss = (uint32_t)l->startSeed;

    for (j = 0; j < pH; j++)
    {
        idx = (j * 2) * newW;

        v00 = out[(j+0)*pW];
        v01 = out[(j+1)*pW];

        for (i = 0; i < pW; i++, v00 = v10, v01 = v11)
        {
            v10 = out[i+1 + (j+0)*pW];
            v11 = out[i+1 + (j+1)*pW];

            if (v00 == v01 && v00 == v10 && v00 == v11)
            {
                buf[idx] = v00;
                buf[idx + 1] = v00;
                buf[idx + newW] = v00;
                buf[idx + newW + 1] = v00;
                idx += 2;
                continue;
            }

            int chunkX = (i + pX) * 2;
            int chunkZ = (j + pZ) * 2;

            uint32_t cs = ss;
            cs += chunkX;
            cs *= cs * 1284865837 + 4150755663;
            cs += chunkZ;
            cs *= cs * 1284865837 + 4150755663;
            cs += chunkX;
            cs *= cs * 1284865837 + 4150755663;
            cs += chunkZ;

            buf[idx] = v00;
            buf[idx + newW] = (cs >> 24) & 1 ? v01 : v00;
            idx++;

            cs *= cs * 1284865837 + 4150755663;
            cs += st;
            buf[idx] = (cs >> 24) & 1 ? v10 : v00;

            cs *= cs * 1284865837 + 4150755663;
            cs += st;
            int r = (cs >> 24) & 3;
            buf[idx + newW] = r==0 ? v00 : r==1 ? v10 : r==2 ? v01 : v11;
            idx++;
        }
    }

    for (j = 0; j < h; j++)
    {
        memmove(&out[j*w], &buf[(j + (z & 1))*newW + (x & 1)], w*sizeof(int));
    }
    //free(buf);

    return 0;
}


static inline int select4(uint32_t cs, uint32_t st, int v00, int v01, int v10, int v11)
{
    int v;
    int cv00 = (v00 == v10) + (v00 == v01) + (v00 == v11);
    int cv10 = (v10 == v01) + (v10 == v11);
    int cv01 = (v01 == v11);
    if (cv00 > cv10 && cv00 > cv01) {
        v = v00;
    } else if (cv10 > cv00) {
        v = v10;
    } else if (cv01 > cv00) {
        v = v01;
    } else {
        cs *= cs * 1284865837 + 4150755663;
        cs += st;
        int r = (cs >> 24) & 3;
        v = r==0 ? v00 : r==1 ? v10 : r==2 ? v01 : v11;
    }
    return v;
}

/// This is the most common layer, and generally the second most performance
/// critical after mapAddIsland.
int mapZoom(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x >> 1;
    int pZ = z >> 1;
    int64_t pW = ((x + w) >> 1) - pX + 1; // (w >> 1) + 2;
    int64_t pH = ((z + h) >> 1) - pZ + 1; // (h >> 1) + 2;
    int64_t i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    int64_t newW = pW * 2;
    //int64_t newH = pH * 2;
    int64_t idx;
    int v00, v01, v10, v11;
    int *buf = out + pW * pH; //(int*) malloc((newW+1)*(newH+1)*sizeof(*buf));

    const uint32_t st = (uint32_t)l->startSalt;
    const uint32_t ss = (uint32_t)l->startSeed;

    for (j = 0; j < pH; j++)
    {
        idx = (j * 2) * newW;

        v00 = out[(j+0)*pW];
        v01 = out[(j+1)*pW];

        for (i = 0; i < pW; i++, v00 = v10, v01 = v11)
        {
            v10 = out[i+1 + (j+0)*pW];
            v11 = out[i+1 + (j+1)*pW];

            if (v00 == v01 && v00 == v10 && v00 == v11)
            {
                buf[idx] = v00;
                buf[idx + 1] = v00;
                buf[idx + newW] = v00;
                buf[idx + newW + 1] = v00;
                idx += 2;
                continue;
            }

            int chunkX = (i + pX) * 2;
            int chunkZ = (j + pZ) * 2;

            uint32_t cs = ss;
            cs += chunkX;
            cs *= cs * 1284865837 + 4150755663;
            cs += chunkZ;
            cs *= cs * 1284865837 + 4150755663;
            cs += chunkX;
            cs *= cs * 1284865837 + 4150755663;
            cs += chunkZ;

            buf[idx] = v00;
            buf[idx + newW] = (cs >> 24) & 1 ? v01 : v00;
            idx++;

            cs *= cs * 1284865837 + 4150755663;
            cs += st;
            buf[idx] = (cs >> 24) & 1 ? v10 : v00;

            buf[idx + newW] = select4(cs, st, v00, v01, v10, v11);

            idx++;
        }
    }

    for (j = 0; j < h; j++)
    {
        memmove(&out[j*w], &buf[(j + (z & 1))*newW + (x & 1)], w*sizeof(int));
    }
    //free(buf);

    return 0;
}

/// This is the most performance crittical layer, especially for getBiomeAtPos.
int mapLand(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int64_t pW = w + 2;
    int64_t pH = h + 2;
    int64_t i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    uint64_t st = l->startSalt;
    uint64_t ss = l->startSeed;
    uint64_t cs;

    for (j = 0; j < h; j++)
    {
        int *vz0 = out + (j+0)*pW;
        int *vz1 = out + (j+1)*pW;
        int *vz2 = out + (j+2)*pW;

        int v00 = vz0[0], vt0 = vz0[1];
        int v02 = vz2[0], vt2 = vz2[1];
        int v20, v22;
        int v11, v;

        for (i = 0; i < w; i++)
        {
            v11 = vz1[i+1];
            v20 = vz0[i+2];
            v22 = vz2[i+2];
            v = v11;

            switch (v11)
            {
            case ocean:
                if (v00 || v20 || v02 || v22) // corners have non-ocean
                {
                    /*
                    setChunkSeed(l,x+i,z+j);
                    int inc = 1;
                    if(v00 != 0 && mcNextInt(l,inc++) == 0) v = v00;
                    if(v20 != 0 && mcNextInt(l,inc++) == 0) v = v20;
                    if(v02 != 0 && mcNextInt(l,inc++) == 0) v = v02;
                    if(v22 != 0 && mcNextInt(l,inc++) == 0) v = v22;
                    if(mcNextInt(l,3) == 0) out[x + z*areaWidth] = v;
                    else if(v == 4)         out[x + z*areaWidth] = 4;
                    else                    out[x + z*areaWidth] = 0;
                    */

                    cs = getChunkSeed(ss, i+x, j+z);
                    int inc = 0;
                    v = 1;

                    if (v00 != ocean)
                    {
                        ++inc; v = v00;
                        cs = mcStepSeed(cs, st);
                    }
                    if (v20 != ocean)
                    {
                        if (++inc == 1 || mcFirstIsZero(cs, 2)) v = v20;
                        cs = mcStepSeed(cs, st);
                    }
                    if (v02 != ocean)
                    {
                        switch (++inc)
                        {
                        case 1:     v = v02; break;
                        case 2:     if (mcFirstIsZero(cs, 2)) v = v02; break;
                        default:    if (mcFirstIsZero(cs, 3)) v = v02;
                        }
                        cs = mcStepSeed(cs, st);
                    }
                    if (v22 != ocean)
                    {
                        switch (++inc)
                        {
                        case 1:     v = v22; break;
                        case 2:     if (mcFirstIsZero(cs, 2)) v = v22; break;
                        case 3:     if (mcFirstIsZero(cs, 3)) v = v22; break;
                        default:    if (mcFirstIsZero(cs, 4)) v = v22;
                        }
                        cs = mcStepSeed(cs, st);
                    }

                    if (v != forest)
                    {
                        if (!mcFirstIsZero(cs, 3))
                            v = ocean;
                    }
                }
                break;

            case forest:
                break;

            default:
                if (v00 == 0 || v20 == 0 || v02 == 0 || v22 == 0)
                {
                    cs = getChunkSeed(ss, i+x, j+z);
                    if (mcFirstIsZero(cs, 5))
                        v = 0;
                }
            }

            out[i + j*w] = v;
            v00 = vt0; vt0 = v20;
            v02 = vt2; vt2 = v22;
        }
    }

    return 0;
}

int mapLand16(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int64_t pW = w + 2;
    int64_t pH = h + 2;
    int64_t i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    uint64_t st = l->startSalt;
    uint64_t ss = l->startSeed;
    uint64_t cs;

    for (j = 0; j < h; j++)
    {
        int *vz0 = out + (j+0)*pW;
        int *vz1 = out + (j+1)*pW;
        int *vz2 = out + (j+2)*pW;

        int v00 = vz0[0], vt0 = vz0[1];
        int v02 = vz2[0], vt2 = vz2[1];
        int v20, v22;
        int v11, v;

        for (i = 0; i < w; i++)
        {
            v11 = vz1[i+1];
            v20 = vz0[i+2];
            v22 = vz2[i+2];
            v = v11;


            if (v11 != 0 || (v00 == 0 && v20 == 0 && v02 == 0 && v22 == 0))
            {
                if (v11 != 0 && (v00 == 0 || v20 == 0 || v02 == 0 || v22 == 0))
                {
                    cs = getChunkSeed(ss, i+x, j+z);
                    if (mcFirstIsZero(cs, 5))
                        v = (v == snowy_tundra) ? frozen_ocean : ocean;
                }
            }
            else
            {
                cs = getChunkSeed(ss, i+x, j+z);
                int inc = 0;
                v = 1;

                if (v00 != ocean)
                {
                    ++inc; v = v00;
                    cs = mcStepSeed(cs, st);
                }
                if (v20 != ocean)
                {
                    if (++inc == 1 || mcFirstIsZero(cs, 2)) v = v20;
                    cs = mcStepSeed(cs, st);
                }
                if (v02 != ocean)
                {
                    switch (++inc)
                    {
                    case 1:     v = v02; break;
                    case 2:     if (mcFirstIsZero(cs, 2)) v = v02; break;
                    default:    if (mcFirstIsZero(cs, 3)) v = v02;
                    }
                    cs = mcStepSeed(cs, st);
                }
                if (v22 != ocean)
                {
                    switch (++inc)
                    {
                    case 1:     v = v22; break;
                    case 2:     if (mcFirstIsZero(cs, 2)) v = v22; break;
                    case 3:     if (mcFirstIsZero(cs, 3)) v = v22; break;
                    default:    if (mcFirstIsZero(cs, 4)) v = v22;
                    }
                    cs = mcStepSeed(cs, st);
                }

                if (!mcFirstIsZero(cs, 3))
                    v = (v == snowy_tundra) ? frozen_ocean : ocean;
            }

            out[i + j*w] = v;
            v00 = vt0; vt0 = v20;
            v02 = vt2; vt2 = v22;
        }
    }

    return 0;
}

int mapLandB18(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int64_t pW = w + 2;
    int64_t pH = h + 2;
    int64_t i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, (int)pW, (int)pH);
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

    for (j = 0; j < h; j++)
    {
        int *vz0 = out + (j+0)*pW;
        int *vz1 = out + (j+1)*pW;
        int *vz2 = out + (j+2)*pW;

        int v00 = vz0[0], vt0 = vz0[1];
        int v02 = vz2[0], vt2 = vz2[1];
        int v20, v22;
        int v11, v;

        for (i = 0; i < w; i++)
        {
            v11 = vz1[i+1];
            v20 = vz0[i+2];
            v22 = vz2[i+2];
            v = v11;

            if (v11 == 0 && (v00 != 0 || v02 != 0 || v20 != 0 || v22 != 0))
            {
                cs = getChunkSeed(ss, i+x, j+z);
                v = mcFirstInt(cs, 3) / 2;
            }
            else if (v11 == 1 && (v00 != 1 || v02 != 1 || v20 != 1 || v22 != 1))
            {
                cs = getChunkSeed(ss, i+x, j+z);
                v = 1 - mcFirstInt(cs, 5) / 4;
            }

            out[i + j*w] = v;
            v00 = vt0; vt0 = v20;
            v02 = vt2; vt2 = v22;
        }
    }

    return 0;
}

int mapIsland(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int64_t pW = w + 2;
    int64_t pH = h + 2;
    int64_t i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[i+1 + (j+1)*pW];
            out[i + j*w] = v11;

            if (v11 == Oceanic)
            {
                if (out[i+1 + (j+0)*pW] != Oceanic) continue;
                if (out[i+2 + (j+1)*pW] != Oceanic) continue;
                if (out[i+0 + (j+1)*pW] != Oceanic) continue;
                if (out[i+1 + (j+2)*pW] != Oceanic) continue;

                cs = getChunkSeed(ss, i+x, j+z);
                if (mcFirstIsZero(cs, 2))
                {
                    out[i + j*w] = 1;
                }
            }
        }
    }

    return 0;
}

int mapSnow16(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int64_t pW = w + 2;
    int64_t pH = h + 2;
    int64_t i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[i+1 + (j+1)*pW];
            if (v11 != ocean)
            {
                cs = getChunkSeed(ss, i+x, j+z);
                v11 = mcFirstIsZero(cs, 5) ? snowy_tundra : plains;
            }
            out[i + j*w] = v11;
        }
    }

    return 0;
}

int mapSnow(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int64_t pW = w + 2;
    int64_t pH = h + 2;
    int64_t i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[i+1 + (j+1)*pW];
            if (!isShallowOcean(v11))
            {
                cs = getChunkSeed(ss, i+x, j+z);
                int r = mcFirstInt(cs, 6);
                if      (r == 0) v11 = Freezing;
                else if (r <= 1) v11 = Cold;
                else             v11 = Warm;
            }
            out[i + j*w] = v11;
        }
    }

    return 0;
}


int mapCool(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int64_t pW = w + 2;
    int64_t pH = h + 2;
    int64_t i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[i+1 + (j+1)*pW];

            if (v11 == Warm)
            {
                int v10 = out[i+1 + (j+0)*pW];
                int v21 = out[i+2 + (j+1)*pW];
                int v01 = out[i+0 + (j+1)*pW];
                int v12 = out[i+1 + (j+2)*pW];

                if (isAny4(Cold, v10, v21, v01, v12) || isAny4(Freezing, v10, v21, v01, v12))
                {
                    v11 = Lush;
                }
            }

            out[i + j*w] = v11;
        }
    }

    return 0;
}


int mapHeat(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int64_t pW = w + 2;
    int64_t pH = h + 2;
    int64_t i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[i+1 + (j+1)*pW];

            if (v11 == Freezing)
            {
                int v10 = out[i+1 + (j+0)*pW];
                int v21 = out[i+2 + (j+1)*pW];
                int v01 = out[i+0 + (j+1)*pW];
                int v12 = out[i+1 + (j+2)*pW];

                if (isAny4(Warm, v10, v21, v01, v12) || isAny4(Lush, v10, v21, v01, v12))
                {
                    v11 = Cold;
                }
            }

            out[i + j*w] = v11;
        }
    }

    return 0;
}


int mapSpecial(const Layer * l, int * out, int x, int z, int w, int h)
{
    int64_t i, j;
    int err = l->p->getMap(l->p, out, x, z, w, h);
    if unlikely(err != 0)
        return err;

    uint64_t st = l->startSalt;
    uint64_t ss = l->startSeed;
    uint64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v = out[i + j*w];
            if (v == Oceanic) continue;

            cs = getChunkSeed(ss, i+x, j+z);

            if (mcFirstIsZero(cs, 13))
            {
                cs = mcStepSeed(cs, st);
                v |= (uint32_t)(1 + mcFirstInt(cs, 15)) << 8 & 0xf00;
                // 1 to 1 mapping so 'out' can be overwritten immediately
                out[i + j*w] = v;
            }
        }
    }

    return 0;
}


int mapMushroom(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int64_t pW = w + 2;
    int64_t pH = h + 2;
    int64_t i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[i+1 + (j+1)*pW];

            // surrounded by ocean?
            if (v11 == 0 &&
                !out[i+0 + (j+0)*pW] && !out[i+2 + (j+0)*pW] &&
                !out[i+0 + (j+2)*pW] && !out[i+2 + (j+2)*pW])
            {
                cs = getChunkSeed(ss, i+x, j+z);
                if (mcFirstIsZero(cs, 100))
                    v11 = mushroom_fields;
            }

            out[i + j*w] = v11;
        }
    }

    return 0;
}


int mapDeepOcean(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int64_t pW = w + 2;
    int64_t pH = h + 2;
    int64_t i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v11 = out[(i+1) + (j+1)*pW];

            if (isShallowOcean(v11))
            {
                // count adjacent oceans
                int oceans = 0;
                if (isShallowOcean(out[(i+1) + (j+0)*pW])) oceans++;
                if (isShallowOcean(out[(i+2) + (j+1)*pW])) oceans++;
                if (isShallowOcean(out[(i+0) + (j+1)*pW])) oceans++;
                if (isShallowOcean(out[(i+1) + (j+2)*pW])) oceans++;

                if (oceans >= 4)
                {
                    switch (v11)
                    {
                    case warm_ocean:
                        v11 = deep_warm_ocean;
                        break;
                    case lukewarm_ocean:
                        v11 = deep_lukewarm_ocean;
                        break;
                    case ocean:
                        v11 = deep_ocean;
                        break;
                    case cold_ocean:
                        v11 = deep_cold_ocean;
                        break;
                    case frozen_ocean:
                        v11 = deep_frozen_ocean;
                        break;
                    default:
                        v11 = deep_ocean;
                    }
                }
            }

            out[i + j*w] = v11;
        }
    }

    return 0;
}


const int warmBiomes[] = {desert, desert, desert, savanna, savanna, plains};
const int lushBiomes[] = {forest, dark_forest, mountains, plains, birch_forest, swamp};
const int coldBiomes[] = {forest, mountains, taiga, plains};
const int snowBiomes[] = {snowy_tundra, snowy_tundra, snowy_tundra, snowy_taiga};

const int oldBiomes[] = { desert, forest, mountains, swamp, plains, taiga, jungle };
const int oldBiomes11[] = { desert, forest, mountains, swamp, plains, taiga };
//const int lushBiomesBE[] = {forest, dark_forest, mountains, plains, plains, plains, birch_forest, swamp};

int mapBiome(const Layer * l, int * out, int x, int z, int w, int h)
{
    int64_t i, j;
    int err = l->p->getMap(l->p, out, x, z, w, h);
    if unlikely(err != 0)
        return err;

    int mc = l->mc;
    uint64_t ss = l->startSeed;
    uint64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int64_t idx = i + j*w;
            int id = out[idx];
            int v;
            int hasHighBit = (id & 0xf00);
            id &= ~0xf00;

            if (mc <= MC_1_6)
            {
                if (id == ocean || id == mushroom_fields)
                {
                    out[idx] = id;
                    continue;
                }

                cs = getChunkSeed(ss, i + x, j + z);

                if (mc <= MC_1_1)
                    v = oldBiomes11[mcFirstInt(cs, 6)];
                else
                    v = oldBiomes[mcFirstInt(cs, 7)];

                if (id != plains && (v != taiga || mc <= MC_1_2))
                    v = snowy_tundra;
            }
            else
            {
                if (isOceanic(id) || id == mushroom_fields)
                {
                    out[idx] = id;
                    continue;
                }

                cs = getChunkSeed(ss, i + x, j + z);

                switch (id)
                {
                case Warm:
                    if (hasHighBit) v = mcFirstIsZero(cs, 3) ? badlands_plateau : wooded_badlands_plateau;
                    else v = warmBiomes[mcFirstInt(cs, 6)];
                    break;
                case Lush:
                    if (hasHighBit) v = jungle;
                    else v = lushBiomes[mcFirstInt(cs, 6)];
                    break;
                case Cold:
                    if (hasHighBit) v = giant_tree_taiga;
                    else v = coldBiomes[mcFirstInt(cs, 4)];
                    break;
                case Freezing:
                    v = snowBiomes[mcFirstInt(cs, 4)];
                    break;
                default:
                    v = mushroom_fields;
                }
            }

            out[idx] = v;
        }
    }

    return 0;
}


int mapNoise(const Layer * l, int * out, int x, int z, int w, int h)
{
    int64_t i, j;
    int err = l->p->getMap(l->p, out, x, z, w, h);
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

    int mod = (l->mc <= MC_1_6) ? 2 : 299999;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            if (out[i + j*w] > 0)
            {
                cs = getChunkSeed(ss, i + x, j + z);
                out[i + j*w] = mcFirstInt(cs, mod)+2;
            }
            else
            {
                out[i + j*w] = 0;
            }
        }
    }

    return 0;
}


int mapBamboo(const Layer * l, int * out, int x, int z, int w, int h)
{
    int64_t i, j;
    int err = l->p->getMap(l->p, out, x, z, w, h);
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int64_t idx = i + j*w;
            if (out[idx] != jungle) continue;

            cs = getChunkSeed(ss, i + x, j + z);
            if (mcFirstIsZero(cs, 10))
            {
                out[idx] = bamboo_jungle;
            }
        }
    }

    return 0;
}


static inline int replaceEdge(int *out, int idx, int mc, int v10, int v21, int v01, int v12, int id, int baseID, int edgeID)
{
    if (id != baseID) return 0;

    if (areSimilar(mc, v10, baseID) && areSimilar(mc, v21, baseID) &&
        areSimilar(mc, v01, baseID) && areSimilar(mc, v12, baseID))
        out[idx] = id;
    else
        out[idx] = edgeID;

    return 1;
}


int mapBiomeEdge(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int64_t pW = w + 2;
    int64_t pH = h + 2;
    int64_t i, j;
    int mc = l->mc;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    for (j = 0; j < h; j++)
    {
        int *vz0 = out + (j+0)*pW;
        int *vz1 = out + (j+1)*pW;
        int *vz2 = out + (j+2)*pW;

        for (i = 0; i < w; i++)
        {
            int v11 = vz1[i+1];
            int v10 = vz0[i+1];
            int v21 = vz1[i+2];
            int v01 = vz1[i+0];
            int v12 = vz2[i+1];

            if (!replaceEdge(out, i + j*w, mc, v10, v21, v01, v12, v11, wooded_badlands_plateau, badlands) &&
                !replaceEdge(out, i + j*w, mc, v10, v21, v01, v12, v11, badlands_plateau, badlands) &&
                !replaceEdge(out, i + j*w, mc, v10, v21, v01, v12, v11, giant_tree_taiga, taiga))
            {
                if (v11 == desert)
                {
                    if (!isAny4(snowy_tundra, v10, v21, v01, v12))
                    {
                        out[i + j*w] = v11;
                    }
                    else
                    {
                        out[i + j*w] = wooded_mountains;
                    }
                }
                else if (v11 == swamp)
                {
                    if (!isAny4(desert, v10, v21, v01, v12) &&
                        !isAny4(snowy_taiga, v10, v21, v01, v12) &&
                        !isAny4(snowy_tundra, v10, v21, v01, v12))
                    {
                        if (!isAny4(jungle, v10, v21, v01, v12) &&
                            !isAny4(bamboo_jungle, v10, v21, v01, v12))
                            out[i + j*w] = v11;
                        else
                            out[i + j*w] = jungle_edge;
                    }
                    else
                    {
                        out[i + j*w] = plains;
                    }
                }
                else
                {
                    out[i + j*w] = v11;
                }
            }
        }
    }

    return 0;
}


int mapHills(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int64_t pW = w + 2;
    int64_t pH = h + 2;
    int64_t i, j;

    if unlikely(l->p2 == NULL)
    {
        printf("mapHills() requires two parents! Use setupMultiLayer()\n");
        exit(1);
    }

    int err;
    err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    int *riv = out + pW * pH;
    err = l->p2->getMap(l->p2, riv, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    int mc = l->mc;
    uint64_t st = l->startSalt;
    uint64_t ss = l->startSeed;
    uint64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int a11 = out[i+1 + (j+1)*pW]; // biome branch
            int b11 = riv[i+1 + (j+1)*pW]; // river branch
            int64_t idx = i + j*w;
            int bn = -1;

            if (mc >= MC_1_7)
                bn = (b11 - 2) % 29;

            if (bn == 1 && b11 >= 2 && !isShallowOcean(a11))
            {
                int m = getMutated(mc, a11);
                if (m > 0)
                    out[idx] = m;
                else
                    out[idx] = a11;
            }
            else
            {
                cs = getChunkSeed(ss, i + x, j + z);
                if (bn == 0 || mcFirstIsZero(cs, 3))
                {
                    int hillID = a11;

                    switch (a11)
                    {
                    case desert:
                        hillID = desert_hills;
                        break;
                    case forest:
                        hillID = wooded_hills;
                        break;
                    case birch_forest:
                        hillID = birch_forest_hills;
                        break;
                    case dark_forest:
                        hillID = plains;
                        break;
                    case taiga:
                        hillID = taiga_hills;
                        break;
                    case giant_tree_taiga:
                        hillID = giant_tree_taiga_hills;
                        break;
                    case snowy_taiga:
                        hillID = snowy_taiga_hills;
                        break;
                    case plains:
                        if (mc <= MC_1_6) {
                            hillID = forest;
                            break;
                        }
                        cs = mcStepSeed(cs, st);
                        hillID = mcFirstIsZero(cs, 3) ? wooded_hills : forest;
                        break;
                    case snowy_tundra:
                        hillID = snowy_mountains;
                        break;
                    case jungle:
                        hillID = jungle_hills;
                        break;
                    case bamboo_jungle:
                        hillID = bamboo_jungle_hills;
                        break;
                    case ocean:
                        if (mc >= MC_1_7)
                            hillID = deep_ocean;
                        break;
                    case mountains:
                        if (mc >= MC_1_7)
                            hillID = wooded_mountains;
                        break;
                    case savanna:
                        hillID = savanna_plateau;
                        break;
                    default:
                        if (areSimilar(mc, a11, wooded_badlands_plateau))
                            hillID = badlands;
                        else if (isDeepOcean(a11))
                        {
                            cs = mcStepSeed(cs, st);
                            if (mcFirstIsZero(cs, 3))
                            {
                                cs = mcStepSeed(cs, st);
                                hillID = mcFirstIsZero(cs, 2) ? plains : forest;
                            }
                        }
                        break;
                    }

                    if (bn == 0 && hillID != a11)
                    {
                        hillID = getMutated(mc, hillID);
                        if (hillID < 0)
                            hillID = a11;
                    }

                    if (hillID != a11)
                    {
                        int a10 = out[i+1 + (j+0)*pW];
                        int a21 = out[i+2 + (j+1)*pW];
                        int a01 = out[i+0 + (j+1)*pW];
                        int a12 = out[i+1 + (j+2)*pW];
                        int equals = 0;

                        if (areSimilar(mc, a10, a11)) equals++;
                        if (areSimilar(mc, a21, a11)) equals++;
                        if (areSimilar(mc, a01, a11)) equals++;
                        if (areSimilar(mc, a12, a11)) equals++;

                        if (equals >= 3 + (mc <= MC_1_6))
                            out[idx] = hillID;
                        else
                            out[idx] = a11;
                    }
                    else
                    {
                        out[idx] = a11;
                    }
                }
                else
                {
                    out[idx] = a11;
                }
            }
        }
    }

    return 0;
}


static inline int reduceID(int id)
{
    return id >= 2 ? 2 + (id & 1) : id;
}

int mapRiver(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int64_t pW = w + 2;
    int64_t pH = h + 2;
    int64_t i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    int mc = l->mc;

    for (j = 0; j < h; j++)
    {
        int *vz0 = out + (j+0)*pW;
        int *vz1 = out + (j+1)*pW;
        int *vz2 = out + (j+2)*pW;

        for (i = 0; i < w; i++)
        {
            int v01 = vz1[i+0];
            int v11 = vz1[i+1];
            int v21 = vz1[i+2];
            int v10 = vz0[i+1];
            int v12 = vz2[i+1];

            if (mc >= MC_1_7)
            {
                v01 = reduceID(v01);
                v11 = reduceID(v11);
                v21 = reduceID(v21);
                v10 = reduceID(v10);
                v12 = reduceID(v12);
            }
            else if (v11 == 0)
            {
                out[i + j * w] = river;
                continue;
            }

            if (v11 == v01 && v11 == v10 && v11 == v12 && v11 == v21)
            {
                out[i + j * w] = -1;
            }
            else
            {
                out[i + j * w] = river;
            }
        }
    }

    return 0;
}


int mapSmooth(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int64_t pW = w + 2;
    int64_t pH = h + 2;
    int64_t i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

    for (j = 0; j < h; j++)
    {
        int *vz0 = out + (j+0)*pW;
        int *vz1 = out + (j+1)*pW;
        int *vz2 = out + (j+2)*pW;

        for (i = 0; i < w; i++)
        {
            int v11 = vz1[i+1];
            int v01 = vz1[i+0];
            int v10 = vz0[i+1];

            if (v11 != v01 || v11 != v10)
            {
                int v21 = vz1[i+2];
                int v12 = vz2[i+1];
                if (v01 == v21 && v10 == v12)
                {
                    cs = getChunkSeed(ss, i+x, j+z);
                    if (cs & (1ULL << 24))
                        v11 = v10;
                    else
                        v11 = v01;
                }
                else
                {
                    if (v01 == v21) v11 = v01;
                    if (v10 == v12) v11 = v10;
                }
            }

            out[i + j * w] = v11;
        }
    }

    return 0;
}


int mapSunflower(const Layer * l, int * out, int x, int z, int w, int h)
{
    int64_t i, j;
    int err = l->p->getMap(l->p, out, x, z, w, h);
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v = out[i + j * w];

            if (v == plains)
            {
                cs = getChunkSeed(ss, i + x, j + z);
                if (mcFirstIsZero(cs, 57))
                {
                    out[i + j*w] = sunflower_plains;
                }
            }
        }
    }

    return 0;
}


inline static int replaceOcean(int *out, int idx, int v10, int v21, int v01, int v12, int id, int replaceID)
{
    if (isOceanic(id)) return 0;

    if (isOceanic(v10) || isOceanic(v21) || isOceanic(v01) || isOceanic(v12))
        out[idx] = replaceID;
    else
        out[idx] = id;

    return 1;
}

inline static int isAll4JFTO(int mc, int a, int b, int c, int d)
{
    return
        (getCategory(mc, a) == jungle || a == forest || a == taiga || isOceanic(a)) &&
        (getCategory(mc, b) == jungle || b == forest || b == taiga || isOceanic(b)) &&
        (getCategory(mc, c) == jungle || c == forest || c == taiga || isOceanic(c)) &&
        (getCategory(mc, d) == jungle || d == forest || d == taiga || isOceanic(d));
}

inline static int isAny4Oceanic(int a, int b, int c, int d)
{
    return isOceanic(a) || isOceanic(b) || isOceanic(c) || isOceanic(d);
}

int mapShore(const Layer * l, int * out, int x, int z, int w, int h)
{
    int pX = x - 1;
    int pZ = z - 1;
    int64_t pW = w + 2;
    int64_t pH = h + 2;
    int64_t i, j;

    int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
    if unlikely(err != 0)
        return err;

    int mc = l->mc;

    for (j = 0; j < h; j++)
    {
        int *vz0 = out + (j+0)*pW;
        int *vz1 = out + (j+1)*pW;
        int *vz2 = out + (j+2)*pW;

        for (i = 0; i < w; i++)
        {
            int v11 = vz1[i+1];
            int v10 = vz0[i+1];
            int v21 = vz1[i+2];
            int v01 = vz1[i+0];
            int v12 = vz2[i+1];

            if (v11 == mushroom_fields)
            {
                if (isAny4(ocean, v10, v21, v01, v12))
                    out[i + j*w] = mushroom_field_shore;
                else
                    out[i + j*w] = v11;
                continue;
            }
            if (mc <= MC_1_0)
            {
                out[i + j*w] = v11;
                continue;
            }

            if (mc <= MC_1_6)
            {
                if (v11 == mountains)
                {
                    if (v10 != mountains || v21 != mountains || v01 != mountains || v12 != mountains)
                        v11 = mountain_edge;
                }
                else if (v11 != ocean && v11 != river && v11 != swamp)
                {
                    if (isAny4(ocean, v10, v21, v01, v12))
                        v11 = beach;
                }
                out[i + j*w] = v11;
            }
            else if (getCategory(mc, v11) == jungle)
            {
                if (isAll4JFTO(mc, v10, v21, v01, v12))
                {
                    if (isAny4Oceanic(v10, v21, v01, v12))
                        out[i + j*w] = beach;
                    else
                        out[i + j*w] = v11;
                }
                else
                {
                    out[i + j*w] = jungle_edge;
                }
            }
            else if (v11 == mountains || v11 == wooded_mountains /* || v11 == mountain_edge*/)
            {
                replaceOcean(out, i + j*w, v10, v21, v01, v12, v11, stone_shore);
            }
            else if (isSnowy(v11))
            {
                replaceOcean(out, i + j*w, v10, v21, v01, v12, v11, snowy_beach);
            }
            else if (v11 == badlands || v11 == wooded_badlands_plateau)
            {
                if (!isAny4Oceanic(v10, v21, v01, v12))
                {
                    if (isMesa(v10) && isMesa(v21) && isMesa(v01) && isMesa(v12))
                        out[i + j*w] = v11;
                    else
                        out[i + j*w] = desert;
                }
                else
                {
                    out[i + j*w] = v11;
                }
            }
            else
            {
                if (v11 != ocean && v11 != deep_ocean && v11 != river && v11 != swamp)
                {
                    if (isAny4Oceanic(v10, v21, v01, v12))
                        out[i + j*w] = beach;
                    else
                        out[i + j*w] = v11;
                }
                else
                {
                    out[i + j*w] = v11;
                }
            }
        }
    }

    return 0;
}

int mapSwampRiver(const Layer * l, int * out, int x, int z, int w, int h)
{
    int64_t i, j;
    int err = l->p->getMap(l->p, out, x, z, w, h);
    if unlikely(err != 0)
        return err;

    uint64_t ss = l->startSeed;
    uint64_t cs;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int v = out[i + j*w];
            if (v != swamp && v != jungle && v != jungle_hills)
                continue;

            cs = getChunkSeed(ss, i + x, j + z);
            if (mcFirstIsZero(cs, (v == swamp) ? 6 : 8))
                v = river;

            out[i + j*w] = v;
        }
    }

    return 0;
}


int mapRiverMix(const Layer * l, int * out, int x, int z, int w, int h)
{
    if unlikely(l->p2 == NULL)
    {
        printf("mapRiverMix() requires two parents! Use setupMultiLayer()\n");
        exit(1);
    }

    int err = l->p->getMap(l->p, out, x, z, w, h); // biome chain
    if unlikely(err != 0)
        return err;

    int64_t len = w*(int64_t)h;
    int64_t idx;
    int mc = l->mc;
    int *buf = out + len;

    err = l->p2->getMap(l->p2, buf, x, z, w, h); // rivers
    if unlikely(err != 0)
        return err;


    for (idx = 0; idx < len; idx++)
    {
        int v = out[idx];

        if (buf[idx] == river && v != ocean && (mc <= MC_1_6 || !isOceanic(v)))
        {
            if (v == snowy_tundra)
                v = frozen_river;
            else if (v == mushroom_fields || v == mushroom_field_shore)
                v = mushroom_field_shore;
            else
                v = river;
        }

        out[idx] = v;
    }

    return 0;
}


int mapOceanTemp(const Layer * l, int * out, int x, int z, int w, int h)
{
    int64_t i, j;
    const PerlinNoise *rnd = (const PerlinNoise*) l->noise;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            double tmp = samplePerlin(rnd, (i + x) / 8.0, (j + z) / 8.0, 0, 0, 0);

            if (tmp > 0.4)
                out[i + j*w] = warm_ocean;
            else if (tmp > 0.2)
                out[i + j*w] = lukewarm_ocean;
            else if (tmp < -0.4)
                out[i + j*w] = frozen_ocean;
            else if (tmp < -0.2)
                out[i + j*w] = cold_ocean;
            else
                out[i + j*w] = ocean;
        }
    }

    return 0;
}


int mapOceanMix(const Layer * l, int * out, int x, int z, int w, int h)
{
    int64_t i, j;
    int lx0, lx1, lz0, lz1, lw, lh;

    if unlikely(l->p2 == NULL)
    {
        printf("mapOceanMix() requires two parents! Use setupMultiLayer()\n");
        exit(1);
    }

    int err = l->p2->getMap(l->p2, out, x, z, w, h);
    if unlikely(err != 0)
        return err;

    // determine the minimum required land area: (x+lx0, z+lz0), (lw, lh)
    // (the extra border is only required if there is warm or frozen ocean)
    lx0 = 0; lx1 = w;
    lz0 = 0; lz1 = h;

    for (j = 0; j < h; j++)
    {
        int jcentre = (j-8 > 0 && j+9 < h);
        for (i = 0; i < w; i++)
        {
            if (jcentre && i-8 > 0 && i+9 < w)
                continue;
            int oceanID = out[i + j*w];
            if (oceanID == warm_ocean || oceanID == frozen_ocean)
            {
                if (i-8 < lx0) lx0 = i-8;
                if (i+9 > lx1) lx1 = i+9;
                if (j-8 < lz0) lz0 = j-8;
                if (j+9 > lz1) lz1 = j+9;
            }
        }
    }

    int *land = out + w*h;
    lw = lx1 - lx0;
    lh = lz1 - lz0;
    err = l->p->getMap(l->p, land, x+lx0, z+lz0, lw, lh);
    if unlikely(err != 0)
        return err;

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int landID = land[(i-lx0) + (j-lz0)*lw];
            int oceanID = out[i + j*w];
            int replaceID = 0;
            int ii, jj;

            if (!isOceanic(landID))
            {
                out[i + j*w] = landID;
                continue;
            }

            if (oceanID == warm_ocean  ) replaceID = lukewarm_ocean;
            if (oceanID == frozen_ocean) replaceID = cold_ocean;
            if (replaceID)
            {
                for (ii = -8; ii <= 8; ii += 4)
                {
                    for (jj = -8; jj <= 8; jj += 4)
                    {
                        int id = land[(i+ii-lx0) + (j+jj-lz0)*lw];
                        if (!isOceanic(id))
                        {
                            out[i + j*w] = replaceID;
                            goto loop_x;
                        }
                    }
                }
            }

            if (landID == deep_ocean)
            {
                switch (oceanID)
                {
                case lukewarm_ocean:
                    oceanID = deep_lukewarm_ocean;
                    break;
                case ocean:
                    oceanID = deep_ocean;
                    break;
                case cold_ocean:
                    oceanID = deep_cold_ocean;
                    break;
                case frozen_ocean:
                    oceanID = deep_frozen_ocean;
                    break;
                }
            }

            out[i + j*w] = oceanID;

            loop_x:;
        }
    }

    return 0;
}


static inline void getVoronoiCell(uint64_t sha, int a, int b, int c,
        int *x, int *y, int *z)
{
    uint64_t s = sha;
    s = mcStepSeed(s, a);
    s = mcStepSeed(s, b);
    s = mcStepSeed(s, c);
    s = mcStepSeed(s, a);
    s = mcStepSeed(s, b);
    s = mcStepSeed(s, c);

    *x = (((s >> 24) & 1023) - 512) * 36;
    s = mcStepSeed(s, sha);
    *y = (((s >> 24) & 1023) - 512) * 36;
    s = mcStepSeed(s, sha);
    *z = (((s >> 24) & 1023) - 512) * 36;
}

void mapVoronoiPlane(uint64_t sha, int *out, int *src,
    int x, int z, int w, int h, int y, int px, int pz, int pw, int ph)
{
    x -= 2;
    y -= 2;
    z -= 2;
    int x000, x001, x010, x011, x100, x101, x110, x111;
    int y000, y001, y010, y011, y100, y101, y110, y111;
    int z000, z001, z010, z011, z100, z101, z110, z111;
    int pi, pj, ii, jj, dx, dz, pjz, pix, i4, j4;
    int v00, v01, v10, v11, v;
    int prev_skip;
    int64_t r;
    uint64_t d, dmin;
    int i, j;

    for (pj = 0; pj < ph-1; pj++)
    {
        v00 = src[(pj+0)*(int64_t)pw];
        v10 = src[(pj+1)*(int64_t)pw];
        pjz = pz + pj;
        j4 = pjz * 4 - z;
        prev_skip = 1;

        for (pi = 0; pi < pw-1; pi++)
        {
            PREFETCH( out + (pjz * 4 + 0) * (int64_t)w + pi, 1, 1 );
            PREFETCH( out + (pjz * 4 + 1) * (int64_t)w + pi, 1, 1 );
            PREFETCH( out + (pjz * 4 + 2) * (int64_t)w + pi, 1, 1 );
            PREFETCH( out + (pjz * 4 + 3) * (int64_t)w + pi, 1, 1 );

            v01 = src[(pj+0)*(int64_t)pw + (pi+1)];
            v11 = src[(pj+1)*(int64_t)pw + (pi+1)];
            pix = px + pi;
            i4 = pix * 4 - x;

            if (v00 == v01 && v00 == v10 && v00 == v11)
            {
                for (jj = 0; jj < 4; jj++)
                {
                    j = j4 + jj;
                    if (j < 0 || j >= h) continue;
                    for (ii = 0; ii < 4; ii++)
                    {
                        i = i4 + ii;
                        if (i < 0 || i >= w) continue;
                        out[j*(int64_t)w + i] = v00;
                    }
                }
                prev_skip = 1;
                continue;
            }
            if (prev_skip)
            {
                getVoronoiCell(sha, pix, y-1, pjz+0, &x000, &y000, &z000);
                getVoronoiCell(sha, pix, y+0, pjz+0, &x001, &y001, &z001);
                getVoronoiCell(sha, pix, y-1, pjz+1, &x100, &y100, &z100);
                getVoronoiCell(sha, pix, y+0, pjz+1, &x101, &y101, &z101);
                prev_skip = 0;
            }
            getVoronoiCell(sha, pix+1, y-1, pjz+0, &x010, &y010, &z010);
            getVoronoiCell(sha, pix+1, y+0, pjz+0, &x011, &y011, &z011);
            getVoronoiCell(sha, pix+1, y-1, pjz+1, &x110, &y110, &z110);
            getVoronoiCell(sha, pix+1, y+0, pjz+1, &x111, &y111, &z111);


            for (jj = 0; jj < 4; jj++)
            {
                j = j4 + jj;
                if (j < 0 || j >= h) continue;
                for (ii = 0; ii < 4; ii++)
                {
                    i = i4 + ii;
                    if (i < 0 || i >= w) continue;

                    const int A = 40*1024;
                    const int B = 20*1024;
                    dx = ii * 10*1024;
                    dz = jj * 10*1024;
                    dmin = (uint64_t)-1;

                    v = v00;
                    d = 0;
                    r = x000 - 0 + dx;  d += r*r;
                    r = y000 + B;       d += r*r;
                    r = z000 - 0 + dz;  d += r*r;
                    if (d < dmin) { dmin = d; }
                    d = 0;
                    r = x001 - 0 + dx;  d += r*r;
                    r = y001 - B;       d += r*r;
                    r = z001 - 0 + dz;  d += r*r;
                    if (d < dmin) { dmin = d; }

                    d = 0;
                    r = x010 - A + dx;  d += r*r;
                    r = y010 + B;       d += r*r;
                    r = z010 - 0 + dz;  d += r*r;
                    if (d < dmin) { dmin = d; v = v01; }
                    d = 0;
                    r = x011 - A + dx;  d += r*r;
                    r = y011 - B;       d += r*r;
                    r = z011 - 0 + dz;  d += r*r;
                    if (d < dmin) { dmin = d; v = v01; }

                    d = 0;
                    r = x100 - 0 + dx;  d += r*r;
                    r = y100 + B;       d += r*r;
                    r = z100 - A + dz;  d += r*r;
                    if (d < dmin) { dmin = d; v = v10; }
                    d = 0;
                    r = x101 - 0 + dx;  d += r*r;
                    r = y101 - B;       d += r*r;
                    r = z101 - A + dz;  d += r*r;
                    if (d < dmin) { dmin = d; v = v10; }

                    d = 0;
                    r = x110 - A + dx;  d += r*r;
                    r = y110 + B;       d += r*r;
                    r = z110 - A + dz;  d += r*r;
                    if (d < dmin) { dmin = d; v = v11; }
                    d = 0;
                    r = x111 - A + dx;  d += r*r;
                    r = y111 - B;       d += r*r;
                    r = z111 - A + dz;  d += r*r;
                    if (d < dmin) { dmin = d; v = v11; }

                    out[j*(int64_t)w + i] = v;
                }
            }

            x000 = x010;
            y000 = y010;
            z000 = z010;
            x100 = x110;
            y100 = y110;
            z100 = z110;
            x001 = x011;
            y001 = y011;
            z001 = z011;
            x101 = x111;
            y101 = y111;
            z101 = z111;
            v00 = v01;
            v10 = v11;
        }
    }
}

int mapVoronoi(const Layer * l, int * out, int x, int z, int w, int h)
{
    x -= 2;
    z -= 2;
    int px = x >> 2;
    int pz = z >> 2;
    int pw = ((x + w) >> 2) - px + 2;
    int ph = ((z + h) >> 2) - pz + 2;

    if (l->p)
    {
        int err = l->p->getMap(l->p, out, px, pz, pw, ph);
        if (err != 0)
            return err;
    }

    int *src = out + (int64_t)w*h;
    memmove(src, out, sizeof(int)*pw*ph);
    mapVoronoiPlane(l->startSalt, out, src, x,z,w,h, 0, px,pz,pw,ph);

    return 0;
}


int mapVoronoi114(const Layer * l, int * out, int x, int z, int w, int h)
{
    x -= 2;
    z -= 2;
    int pX = x >> 2;
    int pZ = z >> 2;
    int64_t pW = ((x + w) >> 2) - pX + 2;
    int64_t pH = ((z + h) >> 2) - pZ + 2;

    if (l->p)
    {
        int err = l->p->getMap(l->p, out, pX, pZ, pW, pH);
        if (err != 0)
            return err;
    }

    int i, j, ii, jj, pi, pj, pix, pjz, i4, j4, mi, mj;
    int v00, v01, v10, v11, v;
    int64_t da1, da2, db1, db2, dc1, dc2, dd1, dd2;
    int64_t sja, sjb, sjc, sjd, da, db, dc, dd;
    int *buf = out + pW * pH;

    uint64_t st = l->startSalt;
    uint64_t ss = l->startSeed;
    uint64_t cs;

    for (pj = 0; pj < pH-1; pj++)
    {
        v00 = out[(pj+0)*pW];
        v01 = out[(pj+1)*pW];
        pjz = pZ + pj;
        j4 = pjz * 4 - z;

        for (pi = 0; pi < pW-1; pi++, v00 = v10, v01 = v11)
        {
            pix = pX + pi;
            i4 = pix * 4 - x;

            // try to prefetch the relevant rows to help prevent cache misses
            PREFETCH( buf + (pjz * 4 + 0) * (int64_t)w + pi, 1, 1 );
            PREFETCH( buf + (pjz * 4 + 1) * (int64_t)w + pi, 1, 1 );
            PREFETCH( buf + (pjz * 4 + 2) * (int64_t)w + pi, 1, 1 );
            PREFETCH( buf + (pjz * 4 + 3) * (int64_t)w + pi, 1, 1 );

            v10 = out[pi+1 + (pj+0)*pW];
            v11 = out[pi+1 + (pj+1)*pW];

            if (v00 == v01 && v00 == v10 && v00 == v11)
            {
                for (jj = 0; jj < 4; jj++)
                {
                    j = j4 + jj;
                    if (j < 0 || j >= h) continue;
                    for (ii = 0; ii < 4; ii++)
                    {
                        i = i4 + ii;
                        if (i < 0 || i >= w) continue;
                        buf[j*(int64_t)w + i] = v00;
                    }
                }
                continue;
            }

            cs = getChunkSeed(ss, (pi+pX) * 4, (pj+pZ) * 4);
            da1 = (mcFirstInt(cs, 1024) - 512) * 36;
            cs = mcStepSeed(cs, st);
            da2 = (mcFirstInt(cs, 1024) - 512) * 36;

            cs = getChunkSeed(ss, (pi+pX+1) * 4, (pj+pZ) * 4);
            db1 = (mcFirstInt(cs, 1024) - 512) * 36 + 40*1024;
            cs = mcStepSeed(cs, st);
            db2 = (mcFirstInt(cs, 1024) - 512) * 36;

            cs = getChunkSeed(ss, (pi+pX) * 4, (pj+pZ+1) * 4);
            dc1 = (mcFirstInt(cs, 1024) - 512) * 36;
            cs = mcStepSeed(cs, st);
            dc2 = (mcFirstInt(cs, 1024) - 512) * 36 + 40*1024;

            cs = getChunkSeed(ss, (pi+pX+1) * 4, (pj+pZ+1) * 4);
            dd1 = (mcFirstInt(cs, 1024) - 512) * 36 + 40*1024;
            cs = mcStepSeed(cs, st);
            dd2 = (mcFirstInt(cs, 1024) - 512) * 36 + 40*1024;

            for (jj = 0; jj < 4; jj++)
            {
                j = j4 + jj;
                if (j < 0 || j >= h) continue;

                mj = 10240*jj;
                sja = (mj-da2) * (mj-da2);
                sjb = (mj-db2) * (mj-db2);
                sjc = (mj-dc2) * (mj-dc2);
                sjd = (mj-dd2) * (mj-dd2);

                for (ii = 0; ii < 4; ii++)
                {
                    i = i4 + ii;
                    if (i < 0 || i >= w) continue;

                    mi = 10240*ii;
                    da = (mi-da1) * (mi-da1) + sja;
                    db = (mi-db1) * (mi-db1) + sjb;
                    dc = (mi-dc1) * (mi-dc1) + sjc;
                    dd = (mi-dd1) * (mi-dd1) + sjd;

                    if      unlikely((da < db) && (da < dc) && (da < dd))
                        v = v00;
                    else if unlikely((db < da) && (db < dc) && (db < dd))
                        v = v10;
                    else if unlikely((dc < da) && (dc < db) && (dc < dd))
                        v = v01;
                    else
                        v = v11;

                    buf[j*(int64_t)w + i] = v;
                }
            }
        }
    }

    memmove(out, buf, sizeof(*buf)*w*h);

    return 0;
}


uint64_t getVoronoiSHA(uint64_t seed)
{
    static const uint32_t K[64] = {
        0x428a2f98,0x71374491, 0xb5c0fbcf,0xe9b5dba5,
        0x3956c25b,0x59f111f1, 0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01, 0x243185be,0x550c7dc3,
        0x72be5d74,0x80deb1fe, 0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786, 0x0fc19dc6,0x240ca1cc,
        0x2de92c6f,0x4a7484aa, 0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d, 0xb00327c8,0xbf597fc7,
        0xc6e00bf3,0xd5a79147, 0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138, 0x4d2c6dfc,0x53380d13,
        0x650a7354,0x766a0abb, 0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b, 0xc24b8b70,0xc76c51a3,
        0xd192e819,0xd6990624, 0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08, 0x2748774c,0x34b0bcb5,
        0x391c0cb3,0x4ed8aa4a, 0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f, 0x84c87814,0x8cc70208,
        0x90befffa,0xa4506ceb, 0xbef9a3f7,0xc67178f2,
    };
    static const uint32_t B[8] = {
        0x6a09e667,0xbb67ae85, 0x3c6ef372,0xa54ff53a,
        0x510e527f,0x9b05688c, 0x1f83d9ab,0x5be0cd19,
    };

    uint32_t m[64];
    uint32_t a0,a1,a2,a3,a4,a5,a6,a7;
    uint32_t i, x, y;
    m[0] = BSWAP32((uint32_t)(seed));
    m[1] = BSWAP32((uint32_t)(seed >> 32));
    m[2] = 0x80000000;
    for (i = 3; i < 15; i++)
        m[i] = 0;
    m[15] = 0x00000040;

    for (i = 16; i < 64; ++i)
    {
        m[i] = m[i - 7] + m[i - 16];
        x = m[i - 15];
        m[i] += rotr32(x,7) ^ rotr32(x,18) ^ (x >> 3);
        x = m[i - 2];
        m[i] += rotr32(x,17) ^ rotr32(x,19) ^ (x >> 10);
    }

    a0 = B[0];
    a1 = B[1];
    a2 = B[2];
    a3 = B[3];
    a4 = B[4];
    a5 = B[5];
    a6 = B[6];
    a7 = B[7];

    for (i = 0; i < 64; i++)
    {
        x = a7 + K[i] + m[i];
        x += rotr32(a4,6) ^ rotr32(a4,11) ^ rotr32(a4,25);
        x += (a4 & a5) ^ (~a4 & a6);

        y = rotr32(a0,2) ^ rotr32(a0,13) ^ rotr32(a0,22);
        y += (a0 & a1) ^ (a0 & a2) ^ (a1 & a2);

        a7 = a6;
        a6 = a5;
        a5 = a4;
        a4 = a3 + x;
        a3 = a2;
        a2 = a1;
        a1 = a0;
        a0 = x + y;
    }

    a0 += B[0];
    a1 += B[1];

    return BSWAP32(a0) | ((uint64_t)BSWAP32(a1) << 32);
}

void voronoiAccess3D(uint64_t sha, int x, int y, int z, int *x4, int *y4, int *z4)
{
    x -= 2;
    y -= 2;
    z -= 2;
    int pX = x >> 2;
    int pY = y >> 2;
    int pZ = z >> 2;
    int dx = (x & 3) * 10240;
    int dy = (y & 3) * 10240;
    int dz = (z & 3) * 10240;
    int ax = 0, ay = 0, az = 0;
    uint64_t dmin = (uint64_t)-1;
    int i;

    for (i = 0; i < 8; i++)
    {
        int bx = (i & 4) != 0;
        int by = (i & 2) != 0;
        int bz = (i & 1) != 0;
        int cx = pX + bx;
        int cy = pY + by;
        int cz = pZ + bz;
        int rx, ry, rz;

        getVoronoiCell(sha, cx, cy, cz, &rx, &ry, &rz);

        rx += dx - 40*1024*bx;
        ry += dy - 40*1024*by;
        rz += dz - 40*1024*bz;

        uint64_t d = rx*(uint64_t)rx + ry*(uint64_t)ry + rz*(uint64_t)rz;
        if (d < dmin)
        {
            dmin = d;
            ax = cx;
            ay = cy;
            az = cz;
        }
    }

    if (x4) *x4 = ax;
    if (y4) *y4 = ay;
    if (z4) *z4 = az;
}



