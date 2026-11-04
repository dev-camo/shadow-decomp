#ifndef GAME_EGGMECACTRLUNIT_H
#define GAME_EGGMECACTRLUNIT_H

#include "Game/BossCtrlUnit.h"
#include <dolphin/types.h>
#include "layout_check.h"

class EggMecaVarParam;

/* The control units of the Egg Mecha bosses (class names and bases from RTTI; see BossCtrlUnit.h).
 * The constructors pass their parameter to every base and keep it at 0x8 as well. EggMecaCtrlUnit
 * leaves BossCtrlUnit::Update pure; every unit below overrides it. The overrides of the units with
 * a second Boss base call that base's Update. Regions marked as unknown are set up by the
 * constructors and not mapped yet. */
class EggMecaCtrlUnit : public virtual BossCtrlUnit {
  public:
    EggMecaCtrlUnit(EggMecaVarParam *param);
    virtual ~EggMecaCtrlUnit();

    /* 0x08 */ EggMecaVarParam *m_eggMecaParam;
};
CHECK_SIZEOF(EggMecaCtrlUnit, 0x1C);

class EggMecaCtrlUnit_AttackCallback : public EggMecaCtrlUnit {
  public:
    EggMecaCtrlUnit_AttackCallback(EggMecaVarParam *param);
    virtual ~EggMecaCtrlUnit_AttackCallback();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(EggMecaCtrlUnit_AttackCallback, 0x1C);

class EggMecaCtrlUnit_GetStatus : public EggMecaCtrlUnit {
  public:
    EggMecaCtrlUnit_GetStatus(EggMecaVarParam *param);
    virtual ~EggMecaCtrlUnit_GetStatus();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(EggMecaCtrlUnit_GetStatus, 0x1C);

class EggMecaModeCtrlUnit : public EggMecaCtrlUnit, public BossModeCtrlUnit {
  public:
    EggMecaModeCtrlUnit(EggMecaVarParam *param);
    virtual ~EggMecaModeCtrlUnit();
    virtual bool Update(float arg);
    virtual BossCtrlUnit *CreateUnit(int mode);
};
CHECK_SIZEOF(EggMecaModeCtrlUnit, 0x28);

class EggMecaCtrlUnit_Move : public EggMecaCtrlUnit {
  public:
    EggMecaCtrlUnit_Move(EggMecaVarParam *param);
    virtual ~EggMecaCtrlUnit_Move();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(EggMecaCtrlUnit_Move, 0x1C);

class EggMecaCtrlUnit_CheckMotionAttribute : public EggMecaCtrlUnit,
                                             public BossCtrlUnit_CheckMotionAttribute {
  public:
    EggMecaCtrlUnit_CheckMotionAttribute(EggMecaVarParam *param);
    virtual ~EggMecaCtrlUnit_CheckMotionAttribute();
    virtual bool Update(float arg);
    /* Stores the payload of event 0x5000 at EggMecaVarParam::field_0x21C. */
    virtual void OnMotionEvent(u16 id, void *payload);
};
CHECK_SIZEOF(EggMecaCtrlUnit_CheckMotionAttribute, 0x24);

class EggMecaCtrlUnit_FixLookCharCamera : public EggMecaCtrlUnit,
                                          public BossCtrlUnit_FixLookCharCamera {
  public:
    EggMecaCtrlUnit_FixLookCharCamera(EggMecaVarParam *param);
    virtual ~EggMecaCtrlUnit_FixLookCharCamera();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(EggMecaCtrlUnit_FixLookCharCamera, 0x4C);

/* resourceIndex is passed with resource key 0x16 to a lookup in the resource manager (behaviour
 * inferred). */
class EggMecaCtrlUnit_DispCanopy : public EggMecaCtrlUnit {
  public:
    EggMecaCtrlUnit_DispCanopy(EggMecaVarParam *param, int resourceIndex);
    virtual ~EggMecaCtrlUnit_DispCanopy();
    virtual bool Update(float arg);

    /* 0x0C */ u8 field_0xC[0x14 - 0xC]; /* unknown */
};
CHECK_SIZEOF(EggMecaCtrlUnit_DispCanopy, 0x24);

class EggMecaCtrlUnit_DispEggman : public EggMecaCtrlUnit {
  public:
    EggMecaCtrlUnit_DispEggman(EggMecaVarParam *param, int resourceIndex);
    virtual ~EggMecaCtrlUnit_DispEggman();
    virtual bool Update(float arg);

    /* 0x0C */ u8 field_0xC[0x14 - 0xC]; /* unknown */
};
CHECK_SIZEOF(EggMecaCtrlUnit_DispEggman, 0x24);

class EggMecaCtrlUnit_DispEye : public EggMecaCtrlUnit {
  public:
    EggMecaCtrlUnit_DispEye(EggMecaVarParam *param);
    virtual ~EggMecaCtrlUnit_DispEye();
    virtual bool Update(float arg);

    /* 0x0C */ u8 field_0xC[0x20 - 0xC]; /* unknown */
};
CHECK_SIZEOF(EggMecaCtrlUnit_DispEye, 0x30);

class EggMecaCtrlUnit_IronBall : public EggMecaCtrlUnit {
  public:
    EggMecaCtrlUnit_IronBall(EggMecaVarParam *param);
    virtual ~EggMecaCtrlUnit_IronBall();
    virtual bool Update(float arg);

    /* 0x0C */ u8 field_0xC[0x64 - 0xC]; /* unknown */
};
CHECK_SIZEOF(EggMecaCtrlUnit_IronBall, 0x74);

class EggMecaCtrlUnit_RollUpper : public EggMecaCtrlUnit {
  public:
    EggMecaCtrlUnit_RollUpper(EggMecaVarParam *param);
    virtual ~EggMecaCtrlUnit_RollUpper();
    virtual bool Update(float arg);

    /* 0x0C */ u8 field_0xC[0x2C - 0xC]; /* unknown */
};
CHECK_SIZEOF(EggMecaCtrlUnit_RollUpper, 0x3C);

class EggMecaCtrlUnit_DispAura : public EggMecaCtrlUnit {
  public:
    EggMecaCtrlUnit_DispAura(EggMecaVarParam *param);
    virtual ~EggMecaCtrlUnit_DispAura();
    virtual bool Update(float arg);

    /* 0x0C */ u8 field_0xC[0x1C - 0xC]; /* unknown */
};
CHECK_SIZEOF(EggMecaCtrlUnit_DispAura, 0x2C);

class EggMecaCtrlUnit_AdjustCircleArea : public EggMecaCtrlUnit {
  public:
    EggMecaCtrlUnit_AdjustCircleArea(EggMecaVarParam *param);
    virtual ~EggMecaCtrlUnit_AdjustCircleArea();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(EggMecaCtrlUnit_AdjustCircleArea, 0x1C);

#endif
