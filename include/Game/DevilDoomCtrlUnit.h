#ifndef GAME_DEVILDOOMCTRLUNIT_H
#define GAME_DEVILDOOMCTRLUNIT_H

#include "Game/AttackCallbackContainer.h"
#include "Game/BossCtrlUnit.h"
#include "Game/DevilDoomVarParam.h"
#include "Game/MotionCallbackContainer.h"
#include "Game/fn_8040_vector_types.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* The control units of the DEVIL_DOOM boss (class names and bases from RTTI; see BossCtrlUnit.h).
 * They are defined at 0x803060C4-0x8030BCD4 (USA). The constructors marked inline are those whose
 * copies the linker keeps from the DEVIL_DOOM unit. Only the classes that the boss task creates,
 * and their bases, are declared. Each declares its destructor and, where its vtable shows an
 * override, Update; DevilDoomCtrlUnit_CheckMotionAttribute also declares its override of
 * OnMotionEvent, and the four attack callback classes below DevilDoomCtrlUnit_AttackCallback their
 * overrides of container slot 2. Other overrides and further virtual functions are not declared
 * yet. The parameter types are inferred from the caller, which passes its DevilDoomVarParam, and
 * the parameter name index is inferred. Regions marked unknown are set up by the out-of-line
 * constructors and not mapped yet. */

/* Constructor at 0x80306554. */
class DevilDoomCtrlUnit : public virtual BossCtrlUnit {
  public:
    DevilDoomCtrlUnit(DevilDoomVarParam *param);
    virtual ~DevilDoomCtrlUnit();

    /* 0x08 */ DevilDoomVarParam *m_devilDoomParam;
};
CHECK_SIZEOF(DevilDoomCtrlUnit, 0x1C);

/* Constructor at 0x8030AB70. */
class DevilDoomCtrlUnit_AttackCallback : public DevilDoomCtrlUnit, public AttackCallbackContainer {
  public:
    DevilDoomCtrlUnit_AttackCallback(DevilDoomVarParam *param, int index);
    virtual ~DevilDoomCtrlUnit_AttackCallback();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(DevilDoomCtrlUnit_AttackCallback, 0x20);

class DevilDoomCtrlUnit_AttackCallbackChest : public DevilDoomCtrlUnit_AttackCallback {
  public:
    DevilDoomCtrlUnit_AttackCallbackChest(DevilDoomVarParam *param, int index)
        : BossCtrlUnit(param), DevilDoomCtrlUnit_AttackCallback(param, index) {}
    virtual ~DevilDoomCtrlUnit_AttackCallbackChest();
    virtual void fn_8023357C(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
};
CHECK_SIZEOF(DevilDoomCtrlUnit_AttackCallbackChest, 0x20);

class DevilDoomCtrlUnit_AttackCallbackEye : public DevilDoomCtrlUnit_AttackCallback {
  public:
    DevilDoomCtrlUnit_AttackCallbackEye(DevilDoomVarParam *param, int index)
        : BossCtrlUnit(param), DevilDoomCtrlUnit_AttackCallback(param, index) {}
    virtual ~DevilDoomCtrlUnit_AttackCallbackEye();
    virtual void fn_8023357C(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
};
CHECK_SIZEOF(DevilDoomCtrlUnit_AttackCallbackEye, 0x20);

class DevilDoomCtrlUnit_AttackCallbackEyeL : public DevilDoomCtrlUnit_AttackCallbackEye {
  public:
    DevilDoomCtrlUnit_AttackCallbackEyeL(DevilDoomVarParam *param, int index)
        : BossCtrlUnit(param), DevilDoomCtrlUnit_AttackCallbackEye(param, index) {}
    virtual ~DevilDoomCtrlUnit_AttackCallbackEyeL();
    virtual void fn_8023357C(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
};
CHECK_SIZEOF(DevilDoomCtrlUnit_AttackCallbackEyeL, 0x20);

class DevilDoomCtrlUnit_AttackCallbackEyeR : public DevilDoomCtrlUnit_AttackCallbackEye {
  public:
    DevilDoomCtrlUnit_AttackCallbackEyeR(DevilDoomVarParam *param, int index)
        : BossCtrlUnit(param), DevilDoomCtrlUnit_AttackCallbackEye(param, index) {}
    virtual ~DevilDoomCtrlUnit_AttackCallbackEyeR();
    virtual void fn_8023357C(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
};
CHECK_SIZEOF(DevilDoomCtrlUnit_AttackCallbackEyeR, 0x20);

/* Constructor at 0x8030B1E4. */
class DevilDoomCtrlUnit_GetStatus : public DevilDoomCtrlUnit {
  public:
    DevilDoomCtrlUnit_GetStatus(DevilDoomVarParam *param);
    virtual ~DevilDoomCtrlUnit_GetStatus();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(DevilDoomCtrlUnit_GetStatus, 0x1C);

/* Constructor at 0x8030A8A0. */
class DevilDoomCtrlUnit_Move : public DevilDoomCtrlUnit {
  public:
    DevilDoomCtrlUnit_Move(DevilDoomVarParam *param);
    virtual ~DevilDoomCtrlUnit_Move();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(DevilDoomCtrlUnit_Move, 0x1C);

/* Constructor at 0x8030A568. */
class DevilDoomCtrlUnit_FixLookCharCamera : public DevilDoomCtrlUnit,
                                            public BossCtrlUnit_FixLookCharCamera {
  public:
    DevilDoomCtrlUnit_FixLookCharCamera(DevilDoomVarParam *param);
    virtual ~DevilDoomCtrlUnit_FixLookCharCamera();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(DevilDoomCtrlUnit_FixLookCharCamera, 0x4C);

/* Constructor at 0x8030A370. */
class DevilDoomCtrlUnit_AdjustEye : public DevilDoomCtrlUnit, public MotionCallbackContainer {
  public:
    DevilDoomCtrlUnit_AdjustEye(DevilDoomVarParam *param);
    virtual ~DevilDoomCtrlUnit_AdjustEye();
    virtual bool Update(float arg);

    /* 0x10 */ u8 field_0x10[0x5C - 0x10]; /* unknown */
};
CHECK_SIZEOF(DevilDoomCtrlUnit_AdjustEye, 0x6C);

/* Constructor at 0x80309D64. */
class DevilDoomCtrlUnit_AdjustNeck : public DevilDoomCtrlUnit, public MotionCallbackContainer {
  public:
    DevilDoomCtrlUnit_AdjustNeck(DevilDoomVarParam *param);
    virtual ~DevilDoomCtrlUnit_AdjustNeck();
    virtual bool Update(float arg);

    /* 0x10 */ u8 field_0x10[0x20 - 0x10]; /* unknown */
};
CHECK_SIZEOF(DevilDoomCtrlUnit_AdjustNeck, 0x30);

/* Constructor at 0x80309AC8. */
class DevilDoomCtrlUnit_CheckInterrupt : public DevilDoomCtrlUnit {
  public:
    DevilDoomCtrlUnit_CheckInterrupt(DevilDoomVarParam *param);
    virtual ~DevilDoomCtrlUnit_CheckInterrupt();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(DevilDoomCtrlUnit_CheckInterrupt, 0x1C);

/* Constructor at 0x80306E24. */
class DevilDoomCtrlUnit_BallControl : public DevilDoomCtrlUnit, public MotionCallbackContainer {
  public:
    DevilDoomCtrlUnit_BallControl(DevilDoomVarParam *param);
    virtual ~DevilDoomCtrlUnit_BallControl();
    virtual bool Update(float arg);

    /* 0x10 */ u8 field_0x10[0x24 - 0x10]; /* unknown */
};
CHECK_SIZEOF(DevilDoomCtrlUnit_BallControl, 0x34);

/* Constructor at 0x80308234. */
class DevilDoomCtrlUnit_WingControl : public DevilDoomCtrlUnit,
                                      public AttackCallbackContainer,
                                      public MotionCallbackContainer {
  public:
    DevilDoomCtrlUnit_WingControl(DevilDoomVarParam *param);
    virtual ~DevilDoomCtrlUnit_WingControl();
    virtual bool Update(float arg);

    /* 0x14 */ u8 field_0x14[0xCC - 0x14]; /* unknown */
};
CHECK_SIZEOF(DevilDoomCtrlUnit_WingControl, 0xDC);

/* Constructor at 0x80306B10. */
class DevilDoomCtrlUnit_Cloud : public DevilDoomCtrlUnit {
  public:
    DevilDoomCtrlUnit_Cloud(DevilDoomVarParam *param);
    virtual ~DevilDoomCtrlUnit_Cloud();
    virtual bool Update(float arg);

    /* 0x0C */ u8 field_0xC[0x30 - 0xC]; /* unknown */
};
CHECK_SIZEOF(DevilDoomCtrlUnit_Cloud, 0x40);

/* Constructor at 0x803068C4. */
class DevilDoomCtrlUnit_AdjustTarget : public DevilDoomCtrlUnit {
  public:
    DevilDoomCtrlUnit_AdjustTarget(DevilDoomVarParam *param);
    virtual ~DevilDoomCtrlUnit_AdjustTarget();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(DevilDoomCtrlUnit_AdjustTarget, 0x1C);

class DevilDoomCtrlUnit_CheckMotionAttribute : public DevilDoomCtrlUnit,
                                               public BossCtrlUnit_CheckMotionAttribute {
  public:
    DevilDoomCtrlUnit_CheckMotionAttribute(DevilDoomVarParam *param)
        : BossCtrlUnit(param), DevilDoomCtrlUnit(param), BossCtrlUnit_CheckMotionAttribute(param) {}
    virtual ~DevilDoomCtrlUnit_CheckMotionAttribute();
    virtual bool Update(float arg);
    virtual void OnMotionEvent(u16 id, void *payload);
};
CHECK_SIZEOF(DevilDoomCtrlUnit_CheckMotionAttribute, 0x24);

#endif
