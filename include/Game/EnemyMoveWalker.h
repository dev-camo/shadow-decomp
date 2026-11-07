#ifndef GAME_ENEMYMOVEWALKER_H
#define GAME_ENEMYMOVEWALKER_H

#include "Game/EnemyMoveCommon.h"
#include "Game/EnemyReferer.h"
#include "layout_check.h"

/* Original RTTI name. Replaces slot 0x20 of EnemyMoveCommon's vtable (0x8017B570, which returns
 * false) and adds no members; its own virtual-base link follows EnemyMoveCommon at 0x64. */
class EnemyMoveWalker : public EnemyMoveCommon, public virtual EnemyReferer {
  public:
    /* The original is at 0x8018F178. */
    EnemyMoveWalker();
    virtual bool fn_8019B450();
};
CHECK_SIZEOF(EnemyMoveWalker, 0x6C);

#endif
