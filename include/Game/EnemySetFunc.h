#ifndef GAME_ENEMYSETFUNC_H
#define GAME_ENEMYSETFUNC_H

#include "layout_check.h"

class UnkGadgetOwner_800C9CD0;

/* Original RTTI name. Ten virtual slots and no virtual destructor. Method placeholders name the
 * base implementations in its original vtable; parameter lists are not recovered unless an
 * implementation or caller shows them, and the int/bool returns follow the bodies that set r3. */
class EnemySetFunc {
  public:
    /* 0x08 */ virtual int fn_8018EBA0();
    /* 0x0C */ virtual int fn_8019B350();
    /* 0x10 */ virtual void fn_80180BE4();
    /* 0x14 */ virtual void fn_8019B358();
    /* 0x18 */ virtual void fn_80187CB8();
    /* 0x1C */ virtual void fn_8019B35C(UnkGadgetOwner_800C9CD0 *owner);
    /* 0x20 */ virtual void fn_8019B360(UnkGadgetOwner_800C9CD0 *owner);
    /* 0x24 */ virtual void fn_8019B364(float);
    /* 0x28 */ virtual int fn_8019B368();
    /* Returns null in the base; EnemyTemplate's override returns TEnemySetTask's field_0x2C, the
     * gadget owner (the pointer return is inferred from that member's type). */
    /* 0x2C */ virtual UnkGadgetOwner_800C9CD0 *fn_8017D788();
};
CHECK_SIZEOF(EnemySetFunc, 0x4);

#endif
