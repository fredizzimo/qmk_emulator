/*
Copyright 2016 Fred Sundvik

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef EMULATOR_EMULATOR_DRIVER_IMPL_H_
#define EMULATOR_EMULATOR_DRIVER_IMPL_H_

#include "gfx.h"
#include "src/gdisp/gdisp_driver.h"
#include "emulator_driver.h"

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

LLDSPEC bool_t gdisp_lld_init(GDisplay *g) {
    unsigned width = GDISP_SCREEN_WIDTH;
    unsigned height = GDISP_SCREEN_HEIGHT;
    // Calculate the size of the display surface in bytes
    unsigned i = width*height*sizeof(color_t);
    if (i < 2*sizeof(coord_t))
        i = 2*sizeof(coord_t);

    pixmap* p;

    // Allocate the pixmap
    if (!(p = gfxAlloc(i+sizeof(pixmap)-sizeof(p->pixels))))
        return 0;

    // Save the width and height so the driver can retrieve it.
    ((coord_t *)p->pixels)[0] = width;
    ((coord_t *)p->pixels)[1] = height;

    g->priv = p;

    // Initialize the GDISP structure
    //	Width and height were saved into the start of the framebuffer.
    g->g.Width = ((coord_t *)((pixmap *)g->priv)->pixels)[0];
    g->g.Height = ((coord_t *)((pixmap *)g->priv)->pixels)[1];
    g->g.Backlight = 100;
    g->g.Contrast = 50;
    g->g.Orientation = GDISP_ROTATE_0;
    g->g.Powermode = powerOn;
    g->board = 0;

    return TRUE;
}

LLDSPEC void gdisp_lld_draw_pixel(GDisplay *g) {
    unsigned	pos;

#if GDISP_NEED_CONTROL
    switch(g->g.Orientation) {
    case GDISP_ROTATE_0:
    default:
        pos = g->p.y * g->g.Width + g->p.x;
        break;
    case GDISP_ROTATE_90:
        pos = (g->g.Width-g->p.x-1) * g->g.Height + g->p.y;
        break;
    case GDISP_ROTATE_180:
#ifdef ROTATE_180_IS_FLIP
        pos = g->p.y * g->g.Width + g->g.Width - g->p.x - 1;
#else
        pos = (g->g.Height-g->p.y-1) * g->g.Width + g->g.Width-g->p.x-1;
#endif
        break;
    case GDISP_ROTATE_270:
        pos = g->p.x * g->g.Height + g->g.Height-g->p.y-1;
        break;
    }
#else
    pos = g->p.y * g->g.Width + g->p.x;
#endif
    // Make sure that the color is converted to the right format
    color_t color = gdispColor2Native(g->p.color);
#if GDISP_LLD_PIXELFORMAT == GDISP_PIXELFORMAT_MONO
    color = color ? White : Black;
#endif

    ((pixmap *)(g)->priv)->pixels[pos] = color;
}

LLDSPEC	color_t gdisp_lld_get_pixel_color(GDisplay *g) {
    unsigned		pos;

#if GDISP_NEED_CONTROL
    switch(g->g.Orientation) {
    case GDISP_ROTATE_0:
    default:
        pos = g->p.y * g->g.Width + g->p.x;
        break;
    case GDISP_ROTATE_90:
        pos = (g->g.Width-g->p.x-1) * g->g.Height + g->p.y;
        break;
    case GDISP_ROTATE_180:
#ifdef ROTATE_180_IS_FLIP
        pos = g->p.y * g->g.Width + g->g.Width - g->p.x - 1;
#else
        pos = (g->g.Height-g->p.y-1) * g->g.Width + g->g.Width-g->p.x-1;
#endif
        break;
    case GDISP_ROTATE_270:
        pos = g->p.x * g->g.Height + g->g.Height-g->p.y-1;
        break;
        }
#else
    pos = g->p.y * g->g.Width + g->p.x;
#endif

    return ((pixmap *)(g)->priv)->pixels[pos];
}

#if GDISP_NEED_CONTROL
    LLDSPEC void gdisp_lld_control(GDisplay *g) {
        switch(g->p.x) {
        case GDISP_CONTROL_ORIENTATION:
            if (g->g.Orientation == (orientation_t)g->p.ptr)
                return;
            switch((orientation_t)g->p.ptr) {
                case GDISP_ROTATE_0:
                case GDISP_ROTATE_180:
                    if (g->g.Orientation == GDISP_ROTATE_90 || g->g.Orientation == GDISP_ROTATE_270) {
                        coord_t		tmp;

                        tmp = g->g.Width;
                        g->g.Width = g->g.Height;
                        g->g.Height = tmp;
                    }
                    break;
                case GDISP_ROTATE_90:
                case GDISP_ROTATE_270:
                    if (g->g.Orientation == GDISP_ROTATE_0 || g->g.Orientation == GDISP_ROTATE_180) {
                        coord_t		tmp;

                        tmp = g->g.Width;
                        g->g.Width = g->g.Height;
                        g->g.Height = tmp;
                    }
                    break;
                default:
                    return;
            }
            g->g.Orientation = (orientation_t)g->p.ptr;
            return;
        }
    }
#endif

#ifdef GDISP_HARDWARE_BITFILLS
#ifdef GDISP_PIXELFORMAT_MONO
LLDSPEC void gdisp_lld_blit_area(GDisplay *g) {
    uint8_t* buffer = (uint8_t*)g->p.ptr;
    int linelength = g->p.cx;
    for (int i = 0; i < g->p.cy; i++) {
        unsigned dstx = g->p.x;
        unsigned dsty = g->p.y + i;
        unsigned srcx = g->p.x1;
        unsigned srcy = g->p.y1 + i;
        unsigned srcbit = srcy * g->p.x2 + srcx;
        for(int j=0; j < linelength; j++) {
            uint8_t src = buffer[srcbit / 8];
            uint8_t bit = 7-(srcbit % 8);
            uint8_t bitset = (src >> bit) & 1;
            color_t color;
            if (bitset) {
                color = RGB2COLOR(255, 255, 255);
            }
            else {
                color = RGB2COLOR(0, 0, 0);
            }
            ((pixmap *)(g)->priv)->pixels[i * g->g.Width + j] = color;
			dstx++;
            srcbit++;
        }
    }
}
#else
# Blitting not implemented for this pixel format
#endif
#endif


#endif /* EMULATOR_EMULATOR_DRIVER_IMPL_H_ */
