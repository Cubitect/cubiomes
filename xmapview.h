#ifndef XMAPVIEW_H_
#define XMAPVIEW_H_

#include "generator.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

typedef struct xwin_t
{
    Display *dis;
    int screen;
    Window win;
    GC gc;

    uint *colbuf;
    uint sx, sy;

} xwin_t;

xwin_t init_x(uint sx, uint sy, const char *titel);
void close_x(xwin_t w);

void viewmap(Layer *layer, unsigned char biomeColour[256][3],
        int areaX, int areaZ, uint areaWidth, uint areaHeight, uint pixscale);



#endif
