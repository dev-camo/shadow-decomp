#ifndef GAME_HEAVYDOG0510CTRLUNIT_H
#define GAME_HEAVYDOG0510CTRLUNIT_H

#include "Game/BossCtrlUnit.h"
#include "Game/HeavyDogCtrlUnit.h"
#include "Game/HeavyDog0510VarParam.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* The control units of the HEAVY_DOG_0510 boss that its boss task creates (class names and bases
 * from RTTI; see HeavyDogCtrlUnit.h). Their files are not recovered; only what the boss task uses
 * is declared. The parameter types are inferred from the boss task, which passes its
 * HeavyDog0510VarParam. */

/* Constructor at 0x80251B5C. It keeps param at 0xC as well. */
class HeavyDog0510CtrlUnit : public HeavyDogCtrlUnit {
  public:
    HeavyDog0510CtrlUnit(HeavyDog0510VarParam *param);
    virtual ~HeavyDog0510CtrlUnit();

    /* 0x0C */ HeavyDog0510VarParam *m_heavyDog0510Param;
};
CHECK_SIZEOF(HeavyDog0510CtrlUnit, 0x20);

/* The constructor is inline: the boss task's copy (0x80254048) is the one the linker keeps. Its
 * vtable (0x80549610) is not in the boss task's file. */
class HeavyDog0510ModeCtrlUnit : public HeavyDog0510CtrlUnit, public BossModeCtrlUnit {
  public:
    HeavyDog0510ModeCtrlUnit(HeavyDog0510VarParam *param)
        : BossCtrlUnit(param), HeavyDog0510CtrlUnit(param), BossModeCtrlUnit(param) {}
    virtual ~HeavyDog0510ModeCtrlUnit();
    virtual bool Update(float arg);
    virtual BossCtrlUnit *CreateUnit(int mode);
};
CHECK_SIZEOF(HeavyDog0510ModeCtrlUnit, 0x2C);

/* Constructor at 0x8026C3A4. */
class HeavyDog0510CtrlUnit_Move : public HeavyDog0510CtrlUnit {
  public:
    HeavyDog0510CtrlUnit_Move(HeavyDog0510VarParam *param);
    virtual ~HeavyDog0510CtrlUnit_Move();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(HeavyDog0510CtrlUnit_Move, 0x20);

/* Constructor at 0x8026B9CC. */
class HeavyDog0510CtrlUnit_FixLookCharCamera : public HeavyDog0510CtrlUnit,
                                               public BossCtrlUnit_FixLookCharCamera {
  public:
    HeavyDog0510CtrlUnit_FixLookCharCamera(HeavyDog0510VarParam *param);
    virtual ~HeavyDog0510CtrlUnit_FixLookCharCamera();
    virtual bool Update(float arg);

    /* A signed state value: the constructor clears it, and the virtual method at 0x8026B7E4
     * (USA) compares it with 2 and 0 at 0x8026B864-0x8026B874 and stores 1 at 0x8026B908. */
    /* 0x40 */ s32 field_0x40;
};
CHECK_SIZEOF(HeavyDog0510CtrlUnit_FixLookCharCamera, 0x54);

#endif
