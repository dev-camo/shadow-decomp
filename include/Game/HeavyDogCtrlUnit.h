#ifndef GAME_HEAVYDOGCTRLUNIT_H
#define GAME_HEAVYDOGCTRLUNIT_H

#include "Game/AttackCallbackContainer.h"
#include "Game/BossCtrlUnit.h"
#include "Game/HeavyDogVarParam.h"
#include "Game/MotionCallbackContainer.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* The control units of the Heavy Dog bosses (class names and bases from RTTI; see BossCtrlUnit.h).
 * They are defined at 0x802498AC-0x8024E104 apart from the constructors marked inline, whose
 * copies the linker keeps from the HEAVY_DOG_0410 unit. Only the classes that the HEAVY_DOG_0410
 * and HEAVY_DOG_0510 boss tasks create, and their bases, are declared. Each declares its destructor
 * and, where its vtable shows an override, Update; HeavyDogCtrlUnit_AttackCallbackCanopy and
 * HeavyDogCtrlUnit_AttackCallbackBody also declare their overrides of container slots 0 and 2.
 * Other overrides and further virtual functions are not declared yet. The parameter types are
 * inferred from the boss tasks, which pass their HeavyDog0410VarParam or HeavyDog0510VarParam, and
 * the parameter names (index, arg, resourceIndex) are inferred. Regions marked unknown are set up
 * by the out-of-line constructors and not mapped yet. */

class HeavyDogCtrlUnit : public virtual BossCtrlUnit {
  public:
    HeavyDogCtrlUnit(HeavyDogVarParam *param);
    virtual ~HeavyDogCtrlUnit();

    /* 0x08 */ HeavyDogVarParam *m_heavyDogParam;
};
CHECK_SIZEOF(HeavyDogCtrlUnit, 0x1C);

class HeavyDogCtrlUnit_AttackCallback : public HeavyDogCtrlUnit, public AttackCallbackContainer {
  public:
    HeavyDogCtrlUnit_AttackCallback(HeavyDogVarParam *param, int index);
    virtual ~HeavyDogCtrlUnit_AttackCallback();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(HeavyDogCtrlUnit_AttackCallback, 0x20);

class HeavyDogCtrlUnit_AttackCallbackCanopy : public HeavyDogCtrlUnit_AttackCallback {
  public:
    HeavyDogCtrlUnit_AttackCallbackCanopy(HeavyDogVarParam *param, int index)
        : BossCtrlUnit(param), HeavyDogCtrlUnit_AttackCallback(param, index) {}
    virtual ~HeavyDogCtrlUnit_AttackCallbackCanopy();
    virtual void fn_80233574(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    virtual void fn_8023357C(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
};
CHECK_SIZEOF(HeavyDogCtrlUnit_AttackCallbackCanopy, 0x20);

class HeavyDogCtrlUnit_AttackCallbackBody : public HeavyDogCtrlUnit_AttackCallback {
  public:
    HeavyDogCtrlUnit_AttackCallbackBody(HeavyDogVarParam *param, int index)
        : BossCtrlUnit(param), HeavyDogCtrlUnit_AttackCallback(param, index) {}
    virtual ~HeavyDogCtrlUnit_AttackCallbackBody();
    virtual void fn_80233574(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    virtual void fn_8023357C(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
};
CHECK_SIZEOF(HeavyDogCtrlUnit_AttackCallbackBody, 0x20);

class HeavyDogCtrlUnit_GetStatus : public HeavyDogCtrlUnit {
  public:
    HeavyDogCtrlUnit_GetStatus(HeavyDogVarParam *param);
    virtual ~HeavyDogCtrlUnit_GetStatus();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(HeavyDogCtrlUnit_GetStatus, 0x1C);

class HeavyDogCtrlUnit_Rotate : public HeavyDogCtrlUnit {
  public:
    HeavyDogCtrlUnit_Rotate(HeavyDogVarParam *param);
    virtual ~HeavyDogCtrlUnit_Rotate();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(HeavyDogCtrlUnit_Rotate, 0x1C);

class HeavyDogCtrlUnit_RocketLauncher : public HeavyDogCtrlUnit,
                                        public AttackCallbackContainer,
                                        public MotionCallbackContainer {
  public:
    HeavyDogCtrlUnit_RocketLauncher(HeavyDogVarParam *param, int arg);
    virtual ~HeavyDogCtrlUnit_RocketLauncher();
    virtual bool Update(float arg);

    /* 0x14 */ u8 field_0x14[0x50 - 0x14]; /* unknown */
};
CHECK_SIZEOF(HeavyDogCtrlUnit_RocketLauncher, 0x60);

class HeavyDogCtrlUnit_Engine : public HeavyDogCtrlUnit {
  public:
    HeavyDogCtrlUnit_Engine(HeavyDogVarParam *param, int arg);
    virtual ~HeavyDogCtrlUnit_Engine();
    virtual bool Update(float arg);

    /* 0x0C */ u8 field_0xC[0x84 - 0xC]; /* unknown */
};
CHECK_SIZEOF(HeavyDogCtrlUnit_Engine, 0x94);

class HeavyDogCtrlUnit_CheckLauncherNum : public HeavyDogCtrlUnit {
  public:
    HeavyDogCtrlUnit_CheckLauncherNum(HeavyDogVarParam *param, int arg);
    virtual ~HeavyDogCtrlUnit_CheckLauncherNum();
    virtual bool Update(float arg);

    /* 0x0C */ s32 field_0xC;  /* the constructor's arg */
    /* 0x10 */ s32 field_0x10; /* copied from HeavyDogVarParam::field_0x140 by the constructor */
};
CHECK_SIZEOF(HeavyDogCtrlUnit_CheckLauncherNum, 0x24);

class HeavyDogCtrlUnit_CheckMotionAttribute : public HeavyDogCtrlUnit,
                                              public BossCtrlUnit_CheckMotionAttribute {
  public:
    HeavyDogCtrlUnit_CheckMotionAttribute(HeavyDogVarParam *param)
        : BossCtrlUnit(param), HeavyDogCtrlUnit(param), BossCtrlUnit_CheckMotionAttribute(param) {}
    virtual ~HeavyDogCtrlUnit_CheckMotionAttribute();
    virtual bool Update(float arg);
    /* Stores the payload of event 0x5000 at HeavyDogVarParam::field_0x1D4 (0x8024A590). */
    virtual void OnMotionEvent(u16 id, void *payload);
};
CHECK_SIZEOF(HeavyDogCtrlUnit_CheckMotionAttribute, 0x24);

/* resourceIndex is passed with resource key 0x16 to a lookup in the resource manager (behaviour
 * inferred), as in EggMecaCtrlUnit_DispCanopy. */
class HeavyDogCtrlUnit_DispCanopy : public HeavyDogCtrlUnit {
  public:
    HeavyDogCtrlUnit_DispCanopy(HeavyDogVarParam *param, int resourceIndex);
    virtual ~HeavyDogCtrlUnit_DispCanopy();
    virtual bool Update(float arg);

    /* 0x0C */ u8 field_0xC[0x14 - 0xC]; /* unknown */
};
CHECK_SIZEOF(HeavyDogCtrlUnit_DispCanopy, 0x24);

/* resourceIndex as in HeavyDogCtrlUnit_DispCanopy. */
class HeavyDogCtrlUnit_DispEnergy : public HeavyDogCtrlUnit {
  public:
    HeavyDogCtrlUnit_DispEnergy(HeavyDogVarParam *param, int resourceIndex);
    virtual ~HeavyDogCtrlUnit_DispEnergy();
    virtual bool Update(float arg);

    /* 0x0C */ u8 field_0xC[0x20 - 0xC]; /* unknown */
};
CHECK_SIZEOF(HeavyDogCtrlUnit_DispEnergy, 0x30);

#endif
