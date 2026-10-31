#ifndef GAME_UNKSTARTUPFLOAT_800E2570_H
#define GAME_UNKSTARTUPFLOAT_800E2570_H

#include "layout_check.h"

/* A float that the original builds at startup (as in CityLaserHolder's static initialiser at
 * 0x800E2570); the names of the variables say what the code does with them (inferred). The original
 * mechanism is unknown (a dynamically initialised float, possibly a tunable wrapper): this
 * placeholder class reproduces the startup function. */
struct UnkStartupFloat_800E2570 {
    UnkStartupFloat_800E2570(float v) : value(v) {}

    float value;
};
CHECK_SIZEOF(UnkStartupFloat_800E2570, 0x4);

#endif
