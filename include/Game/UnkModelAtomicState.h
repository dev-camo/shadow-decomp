#ifndef GAME_UNKMODELATOMICSTATE_H
#define GAME_UNKMODELATOMICSTATE_H

#include "Game/DebugDisp.h"
#include "Game/UnkBitFlags_80014E04.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* An entry of the 0x14-byte array at 0x20 of the object at 0x30 of a GadgetModelInfo
 * (placeholder name). fn_8016248C finds the model part whose name equals the name at 0x4 and
 * either converts the colour to floats or hands the entry to fn_801640A4, depending on the word
 * at 0x8. fn_80162424 looks an entry up by the id at 0x10, and fn_80162298 a part by the id of
 * its state's entry. */
struct UnkModelAtomicEntry {
    /* 0x00 */ u8 field_0x0[0x4]; /* Unrecovered. */
    /* 0x04 */ const char *name;
    /* 0x08 */ int field_0x8; /* 0 or 1 selects how fn_8016248C applies the entry */
    /* 0x0C */ UnkDebugColor_802E0E84 color;
    /* 0x10 */ int id;
};
CHECK_SIZEOF(UnkModelAtomicEntry, 0x14);

/* The 0x10-byte object that a model part keeps at 0x1C of its data (placeholder name, as in
 * GadgetModelInfoStorage.h). The model setup allocates it (0x80162C8C), constructs the flags and
 * zeroes the other members; fn_801640A4 stores the entry and copies its colour. The gadgets find
 * it with GadgetModelInfo::fn_80162298 and replace the colour. */
struct UnkModelAtomicState {
    /* 0x0 */ const UnkModelAtomicEntry *entry;
    /* 0x4 */ UnkBitFlags_80014E04 flags; /* fn_801640A4 sets bit 0 */
    /* 0x8 */ UnkDebugColor_802E0E84 color;
    /* 0xC */ u8 field_0xC[0x10 - 0xC]; /* Unrecovered: four bytes that the setup zeroes. */
};
CHECK_SIZEOF(UnkModelAtomicState, 0x10);

extern "C" {
/* Sets each of the four bytes at 0x8 of state to from + (to - from) * (t - t0) / (t1 - t0), byte
 * by byte (behaviour inferred from its body). */
void fn_80154C6C(UnkModelAtomicState *state, float t, const UnkDebugColor_802E0E84 &from,
                 const UnkDebugColor_802E0E84 &to, float t0, float t1);
}

#endif
