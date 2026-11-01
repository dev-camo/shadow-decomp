#ifndef GAME_ENEMYMOVEFUNC_H
#define GAME_ENEMYMOVEFUNC_H

#include "Game/fn_8040_vector_types.h"
#include "layout_check.h"

/* Original RTTI name. Method placeholders identify the original base-vtable implementations.
 * Types and qualifiers are inferred; there is no virtual destructor. */
class EnemyMoveFunc {
  public:
    /* 0x08 */ virtual void fn_8019B438();
    /* 0x0C */ virtual void fn_8019B43C(float deltaTime);
    /* 0x10 */ virtual bool fn_80187CBC();
    /* 0x14 */ virtual bool fn_8019B440();
    /* 0x18 */ virtual bool fn_8019588C();
    /* 0x1C */ virtual bool fn_8019B448();
    /* 0x20 */ virtual bool fn_8019B450();
    /* 0x24 */ virtual float fn_8017B5A0();
    /* 0x28 */ virtual float fn_8019B458();
    /* 0x2C */ virtual unsigned int fn_8017CAC0();
    /* 0x30 */ virtual const Fn8040Vector3 *fn_8017CAC8();
    /* 0x34 */ virtual void fn_8019B460();
};
CHECK_SIZEOF(EnemyMoveFunc, 0x4);

#endif
