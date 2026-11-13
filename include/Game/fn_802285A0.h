#ifndef GAME_FN_802285A0_H
#define GAME_FN_802285A0_H

#include <dolphin/types.h>
#include "layout_check.h"

/* A 0x2C-byte record of the character table at 0x804F1834 (placeholder name). Only the words that
 * fn_802285A0 and Game/PlayerEffect.cpp's belt read are mapped: the id at 0x0, the belt's effect
 * id at 0x1C and the three components of its colour at 0x20. */
struct UnkCharacterRecord_804F1834 {
    /* 0x00 */ s32 field_0x0;
    /* 0x04 */ u8 field_0x4[0x1C - 0x4];
    /* 0x1C */ s32 field_0x1C;
    /* 0x20 */ s32 field_0x20;
    /* 0x24 */ s32 field_0x24;
    /* 0x28 */ s32 field_0x28;
};
CHECK_SIZEOF(UnkCharacterRecord_804F1834, 0x2C);

/* The character table, indexed by the character (twenty records). */
extern const UnkCharacterRecord_804F1834 lbl_804F1834[20];

/* Lookups in the character tables (original names unknown; behaviour inferred from their bodies).
 * The SET_CHARACTOR gadget uses them with its "Characotr" parameter. */
extern "C" {
/* Returns the first word of the character's 0x2C-byte record in the table at 0x804F1834, except
 * that it returns 2 for character 11 while the word at 0xA8 of lbl_8057D6A0 is 0x192. The gadget
 * passes the result to fn_800740E4 as the id. */
int fn_802285A0(int character);
/* Returns the entry at index in the table of 20 string pointers at 0x80546768, without a range
 * check (the gadget's parameter shows these strings as the character names). */
const char *fn_802285D4(int index);
}

#endif
