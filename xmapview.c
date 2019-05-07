#include "xmapview.h"
#include <string.h>
#include <stdio.h>


/* Global biome colour table. */


void setBiomeColour(unsigned char biomeColour[256][3], int biome,
        unsigned char r, unsigned char g, unsigned char b)
{
    biomeColour[biome][0] = r;
    biomeColour[biome][1] = g;
    biomeColour[biome][2] = b;
}

void initBiomeColours(unsigned char biomeColours[256][3])
{
    // This colouring scheme is taken from the AMIDST program:
    // https://github.com/toolbox4minecraft/amidst
    // https://sourceforge.net/projects/amidst.mirror/

    memset(biomeColours, 0, 256*3);

    setBiomeColour(biomeColours, ocean, 0, 0, 112);
    setBiomeColour(biomeColours, plains,141, 179, 96);
    setBiomeColour(biomeColours, desert, 250, 148, 24);
    setBiomeColour(biomeColours, mountains, 96, 96, 96);
    setBiomeColour(biomeColours, forest, 5, 102, 33);
    setBiomeColour(biomeColours, taiga, 11, 102, 89);
    setBiomeColour(biomeColours, swamp, 7, 249, 178);
    setBiomeColour(biomeColours, river, 0, 0, 255);
    setBiomeColour(biomeColours, hell, 255, 0, 0);
    setBiomeColour(biomeColours, sky, 128, 128, 255);
    setBiomeColour(biomeColours, frozen_ocean, 112, 112, 214);
    setBiomeColour(biomeColours, frozen_river, 160, 160, 255);
    setBiomeColour(biomeColours, snowy_tundra, 255, 255, 255);
    setBiomeColour(biomeColours, snowy_mountains, 160, 160, 160);
    setBiomeColour(biomeColours, mushroom_fields, 255, 0, 255);
    setBiomeColour(biomeColours, mushroom_field_shore, 160, 0, 255);
    setBiomeColour(biomeColours, beach, 250, 222, 85);
    setBiomeColour(biomeColours, desert_hills, 210, 95, 18);
    setBiomeColour(biomeColours, wooded_hills, 34, 85, 28);
    setBiomeColour(biomeColours, taiga_hills, 22, 57, 51);
    setBiomeColour(biomeColours, mountain_edge, 114, 120, 154);
    setBiomeColour(biomeColours, jungle, 83, 123, 9);
    setBiomeColour(biomeColours, jungle_hills, 44, 66, 5);
    setBiomeColour(biomeColours, jungleEdge, 98, 139, 23);
    setBiomeColour(biomeColours, deep_ocean, 0, 0, 48);
    setBiomeColour(biomeColours, stone_shore, 162, 162, 132);
    setBiomeColour(biomeColours, snowy_beach, 250, 240, 192);
    setBiomeColour(biomeColours, birch_forest, 48, 116, 68);
    setBiomeColour(biomeColours, birch_forest_hills, 31, 95, 50);
    setBiomeColour(biomeColours, dark_forest, 64, 81, 26);
    setBiomeColour(biomeColours, snowy_taiga, 49, 85, 74);
    setBiomeColour(biomeColours, snowy_taiga_hills, 36, 63, 54);
    setBiomeColour(biomeColours, giant_tree_taiga, 89, 102, 81);
    setBiomeColour(biomeColours, giant_tree_taiga_hills, 69, 79, 62);
    setBiomeColour(biomeColours, wooded_mountains, 80, 112, 80);
    setBiomeColour(biomeColours, savanna, 189, 178, 95);
    setBiomeColour(biomeColours, savanna_plateau, 167, 157, 100);
    setBiomeColour(biomeColours, badlands, 217, 69, 21);
    setBiomeColour(biomeColours, wooded_badlands_plateau, 176, 151, 101);
    setBiomeColour(biomeColours, badlands_plateau, 202, 140, 101);

    setBiomeColour(biomeColours, warm_ocean, 0, 0, 172);
    setBiomeColour(biomeColours, lukewarm_ocean, 0, 0, 144);
    setBiomeColour(biomeColours, cold_ocean, 32, 32, 112);
    setBiomeColour(biomeColours, deep_warm_ocean, 0, 0, 80);
    setBiomeColour(biomeColours, deep_lukewarm_ocean, 0, 0, 64);
    setBiomeColour(biomeColours, deep_cold_ocean, 32, 32, 56);
    setBiomeColour(biomeColours, deep_frozen_ocean, 64, 64, 144);

    setBiomeColour(biomeColours, ocean+128, 0, 0, 112);
    setBiomeColour(biomeColours, plains+128, 141, 179, 96);
    setBiomeColour(biomeColours, desert+128, 250, 148, 24);
    setBiomeColour(biomeColours, mountains+128, 96, 96, 96);
    setBiomeColour(biomeColours, forest+128, 5, 102, 33);
    setBiomeColour(biomeColours, taiga+128, 11, 102, 89);
    setBiomeColour(biomeColours, swamp+128, 7, 249, 178);
    setBiomeColour(biomeColours, river+128, 0, 0, 255);
    setBiomeColour(biomeColours, hell+128, 255, 0, 0);
    setBiomeColour(biomeColours, sky+128, 128, 128, 255);
    setBiomeColour(biomeColours, frozen_ocean+128, 144, 144, 160);
    setBiomeColour(biomeColours, frozen_river+128, 160, 160, 255);
    setBiomeColour(biomeColours, snowy_tundra+128, 140, 180, 180);
    setBiomeColour(biomeColours, snowy_mountains+128, 160, 160, 160);
    setBiomeColour(biomeColours, mushroom_fields+128, 255, 0, 255);
    setBiomeColour(biomeColours, mushroom_field_shore+128, 160, 0, 255);
    setBiomeColour(biomeColours, beach+128, 250, 222, 85);
    setBiomeColour(biomeColours, desert_hills+128, 210, 95, 18);
    setBiomeColour(biomeColours, wooded_hills+128, 34, 85, 28);
    setBiomeColour(biomeColours, taiga_hills+128, 22, 57, 51);
    setBiomeColour(biomeColours, mountain_edge+128, 114, 120, 154);
    setBiomeColour(biomeColours, jungle+128, 83, 123, 9);
    setBiomeColour(biomeColours, jungle_hills+128, 44, 66, 5);
    setBiomeColour(biomeColours, jungleEdge+128, 98, 139, 23);
    setBiomeColour(biomeColours, deep_ocean+128, 0, 0, 48);
    setBiomeColour(biomeColours, stone_shore+128, 162, 162, 132);
    setBiomeColour(biomeColours, snowy_beach+128, 250, 240, 192);
    setBiomeColour(biomeColours, birch_forest+128, 48, 116, 68);
    setBiomeColour(biomeColours, birch_forest_hills+128, 31, 95, 50);
    setBiomeColour(biomeColours, dark_forest+128, 64, 81, 26);
    setBiomeColour(biomeColours, snowy_taiga+128, 49, 85, 74);
    setBiomeColour(biomeColours, snowy_taiga_hills+128, 36, 63, 54);
    setBiomeColour(biomeColours, giant_tree_taiga+128, 89, 102, 81);
    setBiomeColour(biomeColours, giant_tree_taiga_hills+128, 69, 79, 62);
    setBiomeColour(biomeColours, wooded_mountains+128, 80, 112, 80);
    setBiomeColour(biomeColours, savanna+128, 189, 178, 95);
    setBiomeColour(biomeColours, savanna_plateau+128, 167, 157, 100);
    setBiomeColour(biomeColours, badlands+128, 217, 69, 21);
    setBiomeColour(biomeColours, wooded_badlands_plateau+128, 176, 151, 101);
    setBiomeColour(biomeColours, badlands_plateau+128, 202, 140, 101);

    setBiomeColour(biomeColours, bamboo_jungle, 118, 142, 20);
    setBiomeColour(biomeColours, bamboo_jungle_hills, 59, 71, 10);
}

void initBiomeTypeColours(unsigned char biomeColours[256][3])
{
    memset(biomeColours, 0, 256*3);

    setBiomeColour(biomeColours, Oceanic,  0x00, 0x00, 0xa0);
    setBiomeColour(biomeColours, Warm,     0xff, 0xc0, 0x00);
    setBiomeColour(biomeColours, Lush,     0x00, 0xa0, 0x00);
    setBiomeColour(biomeColours, Cold,     0x60, 0x60, 0x60);
    setBiomeColour(biomeColours, Freezing, 0xff, 0xff, 0xff);
}


xwin_t init_x(uint sx, uint sy, const char *titel)
{
    xwin_t w;

    w.dis = XOpenDisplay(NULL);
    w.screen = DefaultScreen(w.dis);

    w.win = XCreateSimpleWindow(w.dis, DefaultRootWindow(w.dis), 0, 0, sx, sy,
            5, 0x000000, 0xffffff);

    w.gc = XCreateGC(w.dis, w.win, 0,0);

    w.sx = sx;
    w.sy = sy;

    XSetStandardProperties(w.dis, w.win, titel, "Cubiomes", None, NULL, 0, NULL);

    Atom WM_DELETE_WINDOW = XInternAtom(w.dis, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(w.dis, w.win, &WM_DELETE_WINDOW, 1);

    XSelectInput(w.dis, w.win, ExposureMask|ButtonPressMask|KeyPressMask);

    XSetBackground(w.dis, w.gc, 0x000000);
    XSetForeground(w.dis, w.gc, 0xffffff);

    XClearWindow(w.dis, w.win);
    XMapRaised(w.dis, w.win);

    return w;
}

void close_x(xwin_t w)
{
    XFreeGC(w.dis, w.gc);
    XDestroyWindow(w.dis, w.win);
    XCloseDisplay(w.dis);
}


void getBiomeColourMap(uint *colbuf, const unsigned char biomeColour[256][3],
        const int *ints, const uint sx, const uint sy, const uint pixscale)
{
    uint i, j;
    int containsInvalidBiomes = 0;

    for(j = 0; j < sy; j++)
    {
        for(i = 0; i < sx; i++)
        {
            int id = ints[i*sx+j]; //if(id != swamp) id = 100;
            uint r, g, b;

            if(id < 0 || id >= 256)
            {
                // This may happen for some intermediate layers
                containsInvalidBiomes = 1;
                r = biomeColour[id&0x7f][0]-40; r = (r>0xff) ? 0x00 : r&0xff;
                g = biomeColour[id&0x7f][1]-40; g = (g>0xff) ? 0x00 : g&0xff;
                b = biomeColour[id&0x7f][2]-40; b = (b>0xff) ? 0x00 : b&0xff;
            }
            else
            {
                if(id < 128) {
                    r = biomeColour[id][0];
                    g = biomeColour[id][1];
                    b = biomeColour[id][2];
                } else {
                    r = biomeColour[id][0]+40; r = (r>0xff) ? 0xff : r&0xff;
                    g = biomeColour[id][1]+40; g = (g>0xff) ? 0xff : g&0xff;
                    b = biomeColour[id][2]+40; b = (b>0xff) ? 0xff : b&0xff;
                }
            }

            uint m, n;
            for(m = 0; m < pixscale; m++){
                for(n = 0; n < pixscale; n++){
                    colbuf[(j*pixscale+n) + sy*pixscale*(i*pixscale+m)] =
                            ((r&0xff) << 16) + ((g&0xff) << 8) + (b&0xff);
                }
            }
        }
    }

    if(containsInvalidBiomes)
    {
        printf("Warning: Ints contain invalid Biome IDs (Is this an intermediate layer?)\n");
    }
}


void viewmap(Layer *layer, unsigned char biomeColour[256][3], int areaX, int areaZ, uint areaWidth, uint areaHeight, uint pixscale)
{
    int *ints = allocCache(layer, areaWidth, areaHeight);

    // generate the biome ints
    genArea(layer, ints, areaX, areaZ, areaWidth, areaHeight);

    // Calculate a hash for the area (useful to verify the accuracy of the map)
    uint i, hash = 0;
    for(i = 0; i < areaWidth*areaHeight; i++) hash = hash ^ (i*(ints[i]+1));
    printf("Hash:%3X\n", hash&0xfff);

    // construct the X11 window
    xwin_t w = init_x(areaWidth*pixscale, areaHeight*pixscale, "XMapViewer");

    XEvent event;
    KeySym key;
    char text[255];


    // convert the biome ints to a colour image
    uint *colbuf = (uint *) malloc(sizeof(uint) *
            areaWidth*areaHeight*pixscale*pixscale);

    getBiomeColourMap(colbuf, biomeColour, ints, areaWidth, areaHeight, pixscale);

    XImage *ximg = XCreateImage(w.dis, DefaultVisual(w.dis,0), 24, ZPixmap, 0,
            (char*)colbuf, areaWidth*pixscale, areaHeight*pixscale, 32, 0);

    XSetForeground(w.dis, w.gc, 0xf00020);

    // enter the event loop
    while(1)
    {
        XNextEvent(w.dis, &event);

        if(event.type == ClientMessage)
        {
            break;
        }

        if (event.type==Expose && event.xexpose.count==0)
        {
            XMapWindow(w.dis, w.win);

            XPutImage(w.dis, w.win, w.gc, ximg, 0, 0, 0, 0,
                    areaWidth*pixscale, areaHeight*pixscale);
            XSetForeground(w.dis, w.gc, 0xf00020);
        }
        if (event.type==KeyPress)
        {
            XLookupString(&event.xkey,text,255,&key,0);
            if (key == XK_Escape)
            {
                break;
            }
            else
            {
            }
        }
        if (event.type==ButtonPress)
        {
        }

        //XResizeWindow(dis, win, sx, sy);
    }

    close_x(w);
    XFree(ximg);

    free(ints);
    free(colbuf);
}










