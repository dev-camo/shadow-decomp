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

#endif
