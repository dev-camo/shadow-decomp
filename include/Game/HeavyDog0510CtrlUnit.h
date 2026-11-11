#ifndef GAME_HEAVYDOG0510CTRLUNIT_H
#define GAME_HEAVYDOG0510CTRLUNIT_H

#include "Game/BossCtrlUnit.h"
#include "Game/HeavyDogCtrlUnit.h"
#include "Game/HeavyDog0510VarParam.h"
#include "Game/ThrowBomb.h"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

/* The control units of the HEAVY_DOG_0510 boss that its boss task and its mode units create
 * (class names and bases from RTTI; see HeavyDogCtrlUnit.h). The mode units are declared in
 * HeavyDog0510ModeCtrlUnit.h; of the other units only what their users call is declared. The
 * parameter types are inferred from the boss task, which passes its HeavyDog0510VarParam. */

/* The constructor and the destructor are inline; the copies the linker keeps (constructor
 * 0x80251B5C, destructor 0x80251790) and the vtable (0x8054994C) are those of the mode control
 * unit file. The constructor keeps param at 0xC as well. */
class HeavyDog0510CtrlUnit : public HeavyDogCtrlUnit {
  public:
    HeavyDog0510CtrlUnit(HeavyDog0510VarParam *param)
        : BossCtrlUnit(param), HeavyDogCtrlUnit(param), m_heavyDog0510Param(param) {}
    virtual ~HeavyDog0510CtrlUnit() {}

    /* 0x0C */ HeavyDog0510VarParam *m_heavyDog0510Param;
};
CHECK_SIZEOF(HeavyDog0510CtrlUnit, 0x20);

/* The constructor is inline: the boss task's copy (0x80254048) is the one the linker keeps. Its
 * vtable (0x80549610) and its other methods are in the mode control unit file. */
class HeavyDog0510ModeCtrlUnit : public HeavyDog0510CtrlUnit, public BossModeCtrlUnit {
  public:
    HeavyDog0510ModeCtrlUnit(HeavyDog0510VarParam *param)
        : BossCtrlUnit(param), HeavyDog0510CtrlUnit(param), BossModeCtrlUnit(param) {}
    virtual ~HeavyDog0510ModeCtrlUnit();
    virtual bool Update(float arg);
    virtual BossCtrlUnit *CreateUnit(int mode);
};
CHECK_SIZEOF(HeavyDog0510ModeCtrlUnit, 0x2C);

/* The three classes below are defined in HeavyDog0510CtrlUnit.cpp. */

/* Moves the boss (see Update in HeavyDog0510CtrlUnit.cpp). */
class HeavyDog0510CtrlUnit_Move : public HeavyDog0510CtrlUnit {
  public:
    HeavyDog0510CtrlUnit_Move(HeavyDog0510VarParam *param);
    virtual ~HeavyDog0510CtrlUnit_Move();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(HeavyDog0510CtrlUnit_Move, 0x20);

class HeavyDog0510CtrlUnit_FixLookCharCamera : public HeavyDog0510CtrlUnit,
                                               public BossCtrlUnit_FixLookCharCamera {
  public:
    HeavyDog0510CtrlUnit_FixLookCharCamera(HeavyDog0510VarParam *param);
    virtual ~HeavyDog0510CtrlUnit_FixLookCharCamera();
    virtual bool Update(float arg);
    virtual bool fn_80291514();

    /* The state of fn_80291514, 0 to 3 (see HeavyDog0510CtrlUnit.cpp); the constructor clears
     * it. */
    /* 0x40 */ s32 field_0x40;
};
CHECK_SIZEOF(HeavyDog0510CtrlUnit_FixLookCharCamera, 0x54);

/* Holds a weak pointer to a thrown bomb. The constructor creates the bomb under the boss task
 * (fn_80209908, kind 0) and, when the weak pointer then locks, passes the bomb a record built
 * from position, velocity, HeavyDog0510VarParam::field_0x1DC and true. The destructor calls
 * ThrowBomb::fn_8020BF9C on the bomb when it still exists, and Update returns true once it no
 * longer does (the parameter names are inferred). */
class HeavyDog0510CtrlUnit_Bomb : public HeavyDog0510CtrlUnit {
  public:
    HeavyDog0510CtrlUnit_Bomb(HeavyDog0510VarParam *param, const Fn8040Vector3 *position,
                              const Fn8040Vector3 *velocity);
    virtual ~HeavyDog0510CtrlUnit_Bomb();
    virtual bool Update(float arg);

    /* The result of fn_80209908 (name inferred; pointee type as that function's). */
    /* 0x10 */ boost::weak_ptr<ThrowBomb> m_bomb;
};
CHECK_SIZEOF(HeavyDog0510CtrlUnit_Bomb, 0x28);

#endif
