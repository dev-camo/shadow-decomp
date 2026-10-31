#ifndef GAME_UNKSTARTUPFLOAT_800E2570_H
#define GAME_UNKSTARTUPFLOAT_800E2570_H

#include "layout_check.h"

/* A float that the original builds at startup (as in CityLaserHolder's static initialiser at
 * 0x800E2570); the names of the variables say what the code does with them (inferred).
 * Matching workaround: the original mechanism is unknown (a dynamically initialised float,
 * possibly a tunable wrapper). This placeholder class gives the float a constructor, so that a
 * static initialiser stores it at startup as in the original; a plain float initialiser would
 * place the value in .sdata instead. Revisit when the original wrapper type is found. */
struct UnkStartupFloat_800E2570 {
    UnkStartupFloat_800E2570(float v) : value(v) {}

    /* 0x0 */ float value;
};
CHECK_SIZEOF(UnkStartupFloat_800E2570, 0x4);

#endif
