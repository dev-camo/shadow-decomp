#ifndef GAME_HEAVYDOG0510CTRLUNIT_H
#define GAME_HEAVYDOG0510CTRLUNIT_H

#include "Game/BossCtrlUnit.h"
#include "Game/HeavyDogCtrlUnit.h"
#include "Game/HeavyDog0510VarParam.h"
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

/* Constructor at 0x8026BDC4. When the object that 0x8026BBC8 gives it is not null, it passes
 * position, velocity, HeavyDog0510VarParam::field_0x1DC and 1 to 0x80194C1C, which builds a
 * record from them on the stack, and then calls the virtual function at 0xC of that object's
 * vtable with the record (behaviour inferred; the parameter names and types are inferred). */
class HeavyDog0510CtrlUnit_Bomb : public HeavyDog0510CtrlUnit {
  public:
    HeavyDog0510CtrlUnit_Bomb(HeavyDog0510VarParam *param, const Fn8040Vector3 *position,
                              const Fn8040Vector3 *velocity);
    virtual ~HeavyDog0510CtrlUnit_Bomb();
    virtual bool Update(float arg);

    /* 0x10 */ u8 field_0x10[0x18 - 0x10]; /* unknown */

  private:
    /* Copying is disabled in this reconstructed interface because the copy contract is
     * unrecovered: the eight bytes at 0x10 are a pointer and a weak count. The constructor
     * assigns them through 0x8026BF04, which stores the pointer word and calls
     * boost::detail::weak_count::operator= on the count; the destructor releases them through
     * 0x8026BD74, which calls weak_count's destructor. The bytes here would copy them without
     * taking a reference. Declared and not defined. */
    HeavyDog0510CtrlUnit_Bomb(const HeavyDog0510CtrlUnit_Bomb &);
    HeavyDog0510CtrlUnit_Bomb &operator=(const HeavyDog0510CtrlUnit_Bomb &);
};
CHECK_SIZEOF(HeavyDog0510CtrlUnit_Bomb, 0x28);

#endif
