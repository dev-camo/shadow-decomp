#ifndef GAME_FN_800396D0_H
#define GAME_FN_800396D0_H

#include "rw/rwcore.h"

/* Draws the rectangle as a four-vertex immediate-mode triangle strip in the given colour at the
 * device's near screen depth, with vertex alpha enabled for the call and raster as the texture
 * raster; when raster is not null the vertices also get texture coordinates from 0 to 1 (behaviour
 * inferred from its body; constness inferred). */
extern "C" void fn_800396D0(const RwRect *rect, const RwRGBA *color, RwRaster *raster);

#endif
