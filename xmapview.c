#include "xmapview.h"
#include "util.h"

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


void viewmap(Layer *layer, unsigned char biomeColour[256][3], int areaX, int areaZ, uint areaWidth, uint areaHeight, uint pixscale)
{
    int *ints = allocCache(layer, areaWidth, areaHeight);

    // generate the biome ints
    genArea(layer, ints, areaX, areaZ, areaWidth, areaHeight);

    // Calculate a hash for the area (useful to verify the accuracy of the map)
    uint i, hash = 0;
    for (i = 0; i < areaWidth*areaHeight; i++) hash = hash ^ (i*(ints[i]+1));
    printf("Hash:%3X\n", hash&0xfff);

    // construct the X11 window
    xwin_t w = init_x(areaWidth*pixscale, areaHeight*pixscale, "XMapViewer");

    XEvent event;
    KeySym key;
    char text[255];


    // convert the biome ints to a colour image
    uint *colbuf = (uint *) malloc(sizeof(uint) *
            areaWidth*areaHeight*pixscale*pixscale);

    biomesToImage(colbuf, biomeColour, ints, areaWidth, areaHeight, pixscale, 0);

    XImage *ximg = XCreateImage(w.dis, DefaultVisual(w.dis,0), 24, ZPixmap, 0,
            (char*)colbuf, areaWidth*pixscale, areaHeight*pixscale, 24, 0);

    XSetForeground(w.dis, w.gc, 0xf00020);

    // enter the event loop
    while (1)
    {
        XNextEvent(w.dis, &event);

        if (event.type == ClientMessage)
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










