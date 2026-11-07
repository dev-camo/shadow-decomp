#ifndef GAME_HEAVYDOG0410CTRLUNIT_H
#define GAME_HEAVYDOG0410CTRLUNIT_H

#include "Game/BossCtrlUnit.h"
#include "Game/HeavyDogCtrlUnit.h"
#include "Game/HeavyDogVarParam.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* The control units of the HEAVY_DOG_0410 boss (class names and bases from RTTI; see
 * HeavyDogCtrlUnit.h). The parameter types are inferred from the boss task, which passes its
 * HeavyDog0410VarParam. */

/* The constructor and the destructor are inline; the copies the linker keeps (constructor
 * 0x8024EB1C, destructor 0x8024EA8C) and the vtable are those of the HEAVY_DOG_0410 unit. The
 * constructor keeps param at 0xC as well. */
class HeavyDog0410CtrlUnit : public HeavyDogCtrlUnit {
  public:
    HeavyDog0410CtrlUnit(HeavyDog0410VarParam *param)
        : BossCtrlUnit(param), HeavyDogCtrlUnit(param), m_heavyDog0410Param(param) {}
    virtual ~HeavyDog0410CtrlUnit() {}

    /* 0x0C */ HeavyDog0410VarParam *m_heavyDog0410Param;
};
CHECK_SIZEOF(HeavyDog0410CtrlUnit, 0x20);

/* The constructor is inline: the boss task's copy (0x8024E9E0) is the one the linker keeps. Its
 * vtable (0x80549280-0x805492B8, USA) points to the destructor, Update and CreateUnit declared
 * below; its other function entries are adjustment thunks. The words after the vtable are
 * CreateUnit's switch table. */
class HeavyDog0410ModeCtrlUnit : public HeavyDog0410CtrlUnit, public BossModeCtrlUnit {
  public:
    HeavyDog0410ModeCtrlUnit(HeavyDog0410VarParam *param)
        : BossCtrlUnit(param), HeavyDog0410CtrlUnit(param), BossModeCtrlUnit(param) {}
    virtual ~HeavyDog0410ModeCtrlUnit();
    virtual bool Update(float arg);
    virtual BossCtrlUnit *CreateUnit(int mode);
};
CHECK_SIZEOF(HeavyDog0410ModeCtrlUnit, 0x2C);

/* Constructor at 0x8026D428. */
class HeavyDog0410CtrlUnit_Move : public HeavyDog0410CtrlUnit {
  public:
    HeavyDog0410CtrlUnit_Move(HeavyDog0410VarParam *param);
    virtual ~HeavyDog0410CtrlUnit_Move();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(HeavyDog0410CtrlUnit_Move, 0x20);

/* Constructor at 0x8026C620. */
class HeavyDog0410CtrlUnit_Dust : public HeavyDog0410CtrlUnit {
  public:
    HeavyDog0410CtrlUnit_Dust(HeavyDog0410VarParam *param);
    virtual ~HeavyDog0410CtrlUnit_Dust();
    virtual bool Update(float arg);

    /* 0x10 */ u8 field_0x10[0x18 - 0x10]; /* unknown */
};
CHECK_SIZEOF(HeavyDog0410CtrlUnit_Dust, 0x28);

#endif
