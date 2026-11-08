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

/* Constructor at 0x8026CD1C. It copies the vectors at arg1, arg2 and arg3 to 0x2C, 0x38 and 0x44
 * (the parameter names are inferred). */
class HeavyDog0410CtrlUnit_Mine : public HeavyDog0410CtrlUnit, public AttackCallbackContainer {
  public:
    HeavyDog0410CtrlUnit_Mine(HeavyDog0410VarParam *param, const Fn8040Vector3 *arg1,
                              const Fn8040Vector3 *arg2, const Fn8040Vector3 *arg3);
    virtual ~HeavyDog0410CtrlUnit_Mine();
    virtual bool Update(float arg);
    /* Overrides container slot 2 (0x8026C7C4; the container's vtable reaches it through the
     * adjustment thunk 0x8026D50C). */
    virtual void fn_8023357C(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    /* 0x14 */ u8 field_0x14[0x68 - 0x14]; /* unknown */

  private:
    /* Copying is disabled in this reconstructed interface because the copy contract is
     * unrecovered: the destructor releases shared counts at 0x60, 0x58 and 0x50
     * (0x8026CC44-0x8026CC5C), which the bytes here would copy without taking a reference.
     * Declared and not defined. */
    HeavyDog0410CtrlUnit_Mine(const HeavyDog0410CtrlUnit_Mine &);
    HeavyDog0410CtrlUnit_Mine &operator=(const HeavyDog0410CtrlUnit_Mine &);
};
CHECK_SIZEOF(HeavyDog0410CtrlUnit_Mine, 0x78);

#endif
