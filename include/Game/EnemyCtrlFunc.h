#ifndef GAME_ENEMYCTRLFUNC_H
#define GAME_ENEMYCTRLFUNC_H

#include "Game/fn_8040_vector_types.h"
#include "layout_check.h"

/* Original RTTI name. Seven virtual slots and no virtual destructor; the vptr follows 0x18 bytes
 * of data. Method placeholders name the base implementations in its original vtable. Slot 0x10's
 * base returns this, i.e. the vector at 0x0, and slot 0x14's returns the vector at 0xC (const
 * inferred for both). Slot 0x1C returns a vector by value (its base copies the vector at 0x0). The
 * float parameters of slots 0x0C and 0x1C come from the callers, which pass f1, and the overrides,
 * which read it; slot 0x18 returns a float in f1. */
class EnemyCtrlFunc {
  public:
    /* The original (0x8019B370) sets the three components of the vector at 0xC to the float slot
     * 0x18's base returns and leaves the vector at 0x0 uninitialised. */
    EnemyCtrlFunc();

    /* 0x00 */ Fn8040Vector3 field_0x0;
    /* 0x0C */ Fn8040Vector3 field_0xC;

    virtual void fn_8019B420();                 // 0x08
    virtual void fn_8019B424(float);            // 0x0C
    virtual const Fn8040Vector3 *fn_8017C3C8(); // 0x10
    virtual const Fn8040Vector3 *fn_8019B428(); // 0x14
    virtual float fn_8019B430();                // 0x18
    virtual Fn8040Vector3 fn_80196628(float);   // 0x1C
    virtual void fn_8017C680();                 // 0x20
};
CHECK_OFFSETOF(EnemyCtrlFunc, field_0xC, 0xC);
CHECK_SIZEOF(EnemyCtrlFunc, 0x1C);

#endif
