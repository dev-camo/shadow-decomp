#ifndef GAME_UNKMANAGER_80117578_H
#define GAME_UNKMANAGER_80117578_H

#include "Game/Task.h"
#include "layout_check.h"

/* The object that the singleton accessor Sonicteam::System::Singleton<UnkManager_80117578, ...>::
 * Instance (USA (GUPE8P) 0x80117578, kept in Game/SavePoint.cpp with its creating function at
 * 0x801175AC and called from other units) returns (placeholder name after that address; the
 * original class is unknown). Only the task pointer at 0x0 is known: fn_8020261C reads it. */
class UnkManager_80117578 {
  public:
    UnkManager_80117578() : field_0x0(0) {}

    /* 0x0 */ Task *field_0x0;
};
CHECK_SIZEOF(UnkManager_80117578, 0x4);

extern "C" {
/* Returns true when the task at 0x0 is null or is not a ResultTask (class name from RTTI);
 * otherwise whether the ResultTask's word at 0xA0 is 5 or 7 (behaviour from its body; the result
 * type is inferred). */
bool fn_8020261C(UnkManager_80117578 *manager);
}

#endif
