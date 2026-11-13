#ifndef GAME_FN_80038E84_H
#define GAME_FN_80038E84_H

#include "rw/rwcore.h"

struct RpClump;

extern "C" void fn_80038FC0(RpClump *clump);
extern "C" void fn_80038F94(RpClump *clump);
extern "C" RwFrame *fn_80038E84(RwFrame *frame, int id);
/* Pass color (fn_80039114) or the two floats (fn_80039B94) with a callback (0x800393E4,
 * 0x80039BD0) to fn_80456C54 for the clump; fn_80039B94 does nothing for a null clump (behaviour
 * from their bodies; the meanings of the values are not known). */
extern "C" void fn_80039114(RpClump *clump, const RwRGBAReal *color);
extern "C" void fn_80039B94(RpClump *clump, float arg1, float arg2);

#endif
