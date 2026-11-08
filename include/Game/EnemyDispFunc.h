#ifndef GAME_ENEMY_DISP_FUNC_H
#define GAME_ENEMY_DISP_FUNC_H

#include "Game/EnemyMotionEventArg.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class Unk801A4430;
struct Unk801A35ACDescriptor;
/* Opaque model wrapper and its field0 pointee; original names not recovered.
 * These declarations imply no layout and must be unified with later recovery. */
class Unk802CD448;
struct Unk802CD448Data;

/* Creates a model wrapper from two resource entries and a parts descriptor (behaviour inferred from
 * BkWingSmallBase's use; the descriptor is passed on to Unk801A35AC::fn_801A3CE4). */
extern "C" boost::shared_ptr<Unk802CD448> fn_802CDB08(int key, const Unk801A35ACDescriptor *parts,
                                                      void *resourceData,
                                                      unsigned int resourceValue);

/* Argument of slot 0x44 (placeholder name); its implementations switch on the word at 0x0. */
struct UnkEnemyDispEvent_8019B40C {
    /* 0x00 */ int field_0x0;
};
CHECK_SIZEOF(UnkEnemyDispEvent_8019B40C, 0x4);

/* RTTI-proven class with 21 virtual slots (0x08-0x58). Slots 0x40-0x58 return nothing in the
 * base (except 0x44, which returns 0); their parameter lists are not recovered unless noted.
 * EnemyDispCommon binds slot 0x40 as the callback of its model's Unk801A4430
 * (0x801A1798-0x801A1814), so slot 0x40 receives that object's motion events.
 * The original constructor stores the vptr at 0xC, after the vector at 0.
 */
class EnemyDispFunc {
  public:
    /* 0x00 */ Fn8040Vector3 field_0x0;

    virtual void fn_8019B3A0();                           // 0x08
    virtual void fn_8019B3A4(float);                      // 0x0C
    virtual void fn_8019B3A8(float);                      // 0x10
    virtual const Fn8040Matrix34 *fn_8019B3AC();          // 0x14
    virtual Unk802CD448Data *fn_8019B3B4();               // 0x18
    virtual Unk801A4430 *fn_8017CC74();                   // 0x1C
    virtual boost::shared_ptr<Unk802CD448> fn_80181038(); // 0x20
    virtual Fn8040Matrix34 *fn_8019448C(unsigned int);    // 0x24
    virtual Fn8040Vector3 fn_8019B3BC();                  // 0x28
    virtual Fn8040Vector3 fn_8019B3DC();                  // 0x2C
    virtual void fn_80188E0C(float);                      // 0x30
    virtual float fn_8019B3FC();                          // 0x34
    // 0x38; the second vector is taken by value: BkWingSmallBase copies it before the call.
    virtual void fn_8019B404(const Fn8040Vector3 &, UnkRotation3);
    virtual void fn_8017F234(bool);                              // 0x3C
    virtual void fn_8019B408(EnemyMotionEventArg &event);        // 0x40
    virtual bool fn_8019B40C(UnkEnemyDispEvent_8019B40C *event); // 0x44
    virtual void fn_8019B414();                                  // 0x48
    virtual void fn_801834D4();                                  // 0x4C
    virtual void fn_801812C8();                                  // 0x50
    virtual void fn_8019B418();                                  // 0x54
    virtual void fn_8019B41C(bool); // 0x58; its override stores a byte like 0x3C's (bool inferred)
};
CHECK_SIZEOF(EnemyDispFunc, 0x10);

#endif
