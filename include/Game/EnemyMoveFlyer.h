#ifndef GAME_ENEMYMOVEFLYER_H
#define GAME_ENEMYMOVEFLYER_H

#include "Game/EnemyMoveCommon.h"
#include "Game/EnemyReferer.h"
#include "layout_check.h"

/* Original RTTI name. Replaces slot 0x20 of EnemyMoveCommon's vtable and adds no members; its own
 * virtual-base link follows EnemyMoveCommon at 0x64. */
class EnemyMoveFlyer : public EnemyMoveCommon, public virtual EnemyReferer {
  public:
    /* The original (0x801909DC) clears field_0x5C after EnemyMoveCommon's constructor sets it. */
    EnemyMoveFlyer();
    virtual bool fn_8019B450();
};
CHECK_SIZEOF(EnemyMoveFlyer, 0x6C);

#endif
