#ifndef GAME_ENEMYREFBASE_H
#define GAME_ENEMYREFBASE_H

#include "Game/Enemy.h"
#include "Game/EnemyReferer.h"
#include "layout_check.h"

/* Original RTTI name. Adds the shared EnemyReferer virtual base (link at 0x100) to Enemy. */
class EnemyRefBase : public Enemy, public virtual EnemyReferer {
  public:
    virtual ~EnemyRefBase();
};
CHECK_SIZEOF(EnemyRefBase, 0x108);

#endif
