#ifndef GAME_ENEMYWALKERCOMMON_H
#define GAME_ENEMYWALKERCOMMON_H

#include "Game/TEnemyCommonBase.h"
#include "Game/EnemyDispCommon.h"
#include "Game/EnemyMoveWalker.h"
#include "Game/TEnemySetBase.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* Original RTTI name (from the RTTI of the counted block that holds it); not recovered here. */
class CollisionEffect;

/* Original RTTI name. Adds the display (0x1A8), walking movement (0x1E8) and set (0x250)
 * components and the CollisionEffect at 0x268. Its vtable replaces EnemyDispCommon's slot 0x10
 * (fn_8019B3A8) and adds no slots of its own. */
class EnemyWalkerCommon : public TEnemyCommonBase,
                          public EnemyDispCommon,
                          public EnemyMoveWalker,
                          public TEnemySetBase {
  public:
    /* The original (0x8018EE90) also creates the CollisionEffect with new and stores it at
     * 0x268. */
    EnemyWalkerCommon(int kind);
    virtual ~EnemyWalkerCommon();
    /* The original (0x8017CC7C) runs EnemyDispCommon's slot first; then, when status slot 0x40
     * returns false and field_0x268 holds an object, it passes dt to that object (0x80344528). */
    virtual void fn_8019B3A8(float dt);

    /* 0x268 */ boost::shared_ptr<CollisionEffect> field_0x268;
};
CHECK_SIZEOF(EnemyWalkerCommon, 0x274);

#endif
