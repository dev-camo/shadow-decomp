#ifndef GAME_ENEMYMOVEFUNC_H
#define GAME_ENEMYMOVEFUNC_H

#include "Game/fn_8040_vector_types.h"
#include "layout_check.h"

/* Original RTTI name. Methods are address placeholders, anchored to the Common
 * implementations where they override the interface's default implementation.
 * Their purpose and qualifiers are inferred; there is no virtual destructor. */
class EnemyMoveFunc {
  public:
    /* 0x08 */ virtual void fn_801A5CBC();
    /* 0x0C */ virtual void fn_801A5778(float deltaTime);
    /* 0x10 */ virtual bool fn_8017B578();
    /* 0x14 */ virtual bool fn_8017B580();
    /* 0x18 */ virtual bool fn_801A5760();
    /* 0x1C */ virtual bool fn_801A5734();
    /* 0x20 */ virtual bool fn_8019B450();
    /* 0x24 */ virtual float fn_8017B5A0();
    /* 0x28 */ virtual float fn_8017B588();
    /* 0x2C */ virtual unsigned int fn_8017B590();
    /* 0x30 */ virtual const Fn8040Vector3 *fn_8017B598();
    /* 0x34 */ virtual void fn_801A560C();
};
CHECK_SIZEOF(EnemyMoveFunc, 0x4);

#endif
