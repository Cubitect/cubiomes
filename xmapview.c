#include "xmapview.h"
#include <string.h>
#include <stdio.h>


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
            int id = ints[i*sx+j]; //if(id != swampland) id = 100;
            uint r, g, b;

            if(id < 0 || id >= 256)
            {
                // This may happen for some intermediate layers
                containsInvalidBiomes = 1;
                r = 0; g = 0; b = 0;
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
