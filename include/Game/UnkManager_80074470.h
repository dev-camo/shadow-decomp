#ifndef GAME_UNKMANAGER_80074470_H
#define GAME_UNKMANAGER_80074470_H

#include "Game/StageAction.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* The manager that fn_80074470 returns (placeholder name; it is declared with fn_80074470 in
 * Game/DebugMissionClearCollision.h). This is a view of its first 0xC bytes: the current stage
 * action at 0x0 (inferred: the ShootingGame manager task calls the action's second virtual function
 * through it) and a word at 0x8, which that task compares with 3 and the .CEN reader's Update
 * (Game/PjsEventCen.cpp) with 6 and 14 (meaning unknown). fn_80175EEC also uses a word at 0xC and
 * bytes at 0x14 and from 0x17, so the object is at least 0x18 bytes; those members are not
 * declared here, and the complete layout is unknown. */
class UnkManager_80074470 {
  public:
    /* 0x0 */ StageAction::IAction *m_action;
    /* 0x4 */ u8 field_0x4[0x8 - 0x4];
    /* 0x8 */ s32 field_0x8;
};
/* Checks only the declared view, not the object's size (at least 0x18 bytes). */
CHECK_SIZEOF(UnkManager_80074470, 0xC);

#endif
