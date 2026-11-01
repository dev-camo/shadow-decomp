#ifndef GAME_ENEMYREFERER_H
#define GAME_ENEMYREFERER_H

#include "layout_check.h"

class Enemy;

/* Original RTTI spelling. Shared virtual base containing a borrowed Enemy*. */
class EnemyReferer {
  public:
    EnemyReferer() : m_enemy(0) {}

  protected:
    /* 0x00 */ Enemy *m_enemy;
};
CHECK_SIZEOF(EnemyReferer, 0x4);

#endif
