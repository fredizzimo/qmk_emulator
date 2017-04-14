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

#ifndef EMULATOR_EMULATOR_DRIVER_H_
#define EMULATOR_EMULATOR_DRIVER_H_

#include "src/gdisp/gdisp_driver.h"

typedef struct pixmap {
	color_t			pixels[1];			// We really want pixels[0] but some compilers don't allow that even though it is C standard.
} pixmap;

#if !defined(GDISP_DRIVER_VMT)
static pixel_t	*getEmulatorPixmap(GDisplay *g) {
	return ((pixmap *)g->priv)->pixels;
}
#endif

#endif /* EMULATOR_EMULATOR_DRIVER_H_ */
