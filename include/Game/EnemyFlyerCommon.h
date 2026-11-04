#ifndef GAME_ENEMYFLYERCOMMON_H
#define GAME_ENEMYFLYERCOMMON_H

#include "Game/TEnemyCommonBase.h"
#include "Game/EnemyDispCommon.h"
#include "Game/EnemyMoveFlyer.h"
#include "Game/TEnemySetBase.h"
#include "layout_check.h"

/* Original RTTI name. Adds the display (0x1A8), flying movement (0x1E8) and set (0x250) components;
 * it adds no virtual slots or members. */
class EnemyFlyerCommon : public TEnemyCommonBase,
                         public EnemyDispCommon,
                         public EnemyMoveFlyer,
                         public TEnemySetBase {
  public:
    EnemyFlyerCommon(int kind);
    virtual ~EnemyFlyerCommon();
};
CHECK_SIZEOF(EnemyFlyerCommon, 0x26C);

#endif
