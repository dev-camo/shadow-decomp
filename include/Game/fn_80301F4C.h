#ifndef GAME_FN_80301F4C_H
#define GAME_FN_80301F4C_H

#include <dolphin/types.h>

/* Returns the on state of the switch record of id in a table of four records (0x80584CE0), or
 * false when id has no record (behaviour inferred from fn_80301E1C, which it calls). The table is
 * constructed by the static initialiser at 0x80303704 in the BASwitch gadget's code, which the
 * table's functions directly precede. The original names are unknown. */
extern "C" bool fn_80301F4C(s32 id);

#endif
