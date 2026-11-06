#ifndef GAME_UNKMANAGER_8006D6A8_H
#define GAME_UNKMANAGER_8006D6A8_H

#include <dolphin/types.h>
#include "layout_check.h"

/* The manager that fn_8006D6A8 returns (placeholder name: the original class name is unknown;
 * fn_8006D6A8 is declared in UnkActor_800740E4.h). Only the word at 0x20 is mapped, and the size
 * is not known. */
class UnkManager_8006D6A8 {
  public:
    /* 0x00 */ u8 field_0x0[0x20];
    /* 0x20 */ int field_0x20; /* the BDHologram gadget passes it to fn_801FAAC4 */
};
CHECK_OFFSETOF(UnkManager_8006D6A8, field_0x20, 0x20);

/* Passes each non-null word at 0x8, 0x10 and 0x18 to fn_8016B170, calls fn_80336FB4 for the
 * address 0x88 bytes into lbl_8057D6A0 when the byte at 0x28 is set, and copies the word at 0x20
 * to 0x24 (behaviour from its body). */
extern "C" void fn_8016DFF8(UnkManager_8006D6A8 *manager);

#endif
