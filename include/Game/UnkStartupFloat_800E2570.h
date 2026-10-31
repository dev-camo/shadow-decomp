#ifndef GAME_UNKSTARTUPFLOAT_800E2570_H
#define GAME_UNKSTARTUPFLOAT_800E2570_H

#include "layout_check.h"

/* Matching workaround: a stand-in class gives each float a constructor, so that it is initialised
 * at startup by code like CityLaserHolder's static initialiser at 0x800E2570. The original
 * mechanism is unknown (a dynamically initialised float, possibly a tunable wrapper). Revisit
 * when the original wrapper type is found.
 *
 * A float that the original builds at startup; the names of the variables say what the code does
 * with them (inferred). */
struct UnkStartupFloat_800E2570 {
    UnkStartupFloat_800E2570(float v) : value(v) {}

    /* 0x0 */ float value;
};
CHECK_SIZEOF(UnkStartupFloat_800E2570, 0x4);

#endif
