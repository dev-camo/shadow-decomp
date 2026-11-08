#ifndef GAME_HEAVYDOG0410MODECTRLUNIT_H
#define GAME_HEAVYDOG0410MODECTRLUNIT_H

#include "Game/BossCtrlUnit.h"
#include "Game/CsdOneShot.h"
#include "Game/EffectElement.h"
#include "Game/HeavyDog0410CtrlUnit.h"
#include "Game/HeavyDogCtrlUnit.h"
#include "Game/HeavyDogVarParam.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/fn_8040_vector_types.h"
#include "Game/fn_804157E8_motion_control.h"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

/* The mode control unit of the HEAVY_DOG_0410 boss and the units it creates, one for each mode
 * (class names and bases from RTTI; see HeavyDogCtrlUnit.h). The units request the next mode by
 * storing it in BossVarParam::field_0x14, which BossModeCtrlUnit::Update moves to field_0xC and
 * passes to CreateUnit (see BossVarParam; the word "mode" is inferred from the class names). The
 * mode units' constructors are inline: the copies the linker keeps are those of
 * HeavyDog0410ModeCtrlUnit.cpp, which follow CreateUnit. Each unit uses the variables through
 * m_heavyDog0410Param; the parameter types are inferred from the boss task, which passes its
 * HeavyDog0410VarParam. */

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

class HeavyDog0410ModeCtrlUnit_WAIT : public HeavyDog0410CtrlUnit {
  public:
    HeavyDog0410ModeCtrlUnit_WAIT(HeavyDog0410VarParam *param)
        : BossCtrlUnit(param), HeavyDog0410CtrlUnit(param) {
        m_heavyDog0410Param->field_0x114 = 0;
        m_heavyDog0410Param->field_0x115 = 0;
    }
    virtual ~HeavyDog0410ModeCtrlUnit_WAIT();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(HeavyDog0410ModeCtrlUnit_WAIT, 0x20);

class HeavyDog0410ModeCtrlUnit_EDIT : public HeavyDog0410CtrlUnit {
  public:
    HeavyDog0410ModeCtrlUnit_EDIT(HeavyDog0410VarParam *param)
        : BossCtrlUnit(param), HeavyDog0410CtrlUnit(param) {
        m_heavyDog0410Param->field_0x114 = 1;
        m_heavyDog0410Param->field_0x115 = 1;
    }
    virtual ~HeavyDog0410ModeCtrlUnit_EDIT();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(HeavyDog0410ModeCtrlUnit_EDIT, 0x20);

class HeavyDog0410ModeCtrlUnit_CHASE : public HeavyDog0410CtrlUnit {
  public:
    HeavyDog0410ModeCtrlUnit_CHASE(HeavyDog0410VarParam *param)
        : BossCtrlUnit(param), HeavyDog0410CtrlUnit(param) {
        m_heavyDog0410Param->field_0x114 = 0;
        m_heavyDog0410Param->field_0x115 = 0;
        m_timer = 0.0f;
    }
    virtual ~HeavyDog0410ModeCtrlUnit_CHASE();
    virtual bool Update(float arg);

    /* Update adds its argument to it and compares the sum with 1.5 (name inferred). */
    /* 0x10 */ float m_timer;
};
CHECK_SIZEOF(HeavyDog0410ModeCtrlUnit_CHASE, 0x24);

class HeavyDog0410ModeCtrlUnit_ESCAPE : public HeavyDog0410CtrlUnit {
  public:
    HeavyDog0410ModeCtrlUnit_ESCAPE(HeavyDog0410VarParam *param)
        : BossCtrlUnit(param), HeavyDog0410CtrlUnit(param) {
        m_heavyDog0410Param->field_0x114 = 0;
        m_heavyDog0410Param->field_0x115 = 0;
    }
    virtual ~HeavyDog0410ModeCtrlUnit_ESCAPE();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(HeavyDog0410ModeCtrlUnit_ESCAPE, 0x20);

class HeavyDog0410ModeCtrlUnit_GET_ATK : public HeavyDog0410CtrlUnit {
  public:
    HeavyDog0410ModeCtrlUnit_GET_ATK(HeavyDog0410VarParam *param)
        : BossCtrlUnit(param), HeavyDog0410CtrlUnit(param) {
        m_heavyDog0410Param->field_0x18 = 1;
        m_heavyDog0410Param->field_0x114 = 1;
        m_heavyDog0410Param->field_0x1D0++;
        field_0x10 = 0.0f;
    }
    virtual ~HeavyDog0410ModeCtrlUnit_GET_ATK();
    virtual bool Update(float arg);

    /* Update adds lbl_8057D6A0's float at 0x18 to it while the variables' field_0x18 is set. */
    /* 0x10 */ float field_0x10;
};
CHECK_SIZEOF(HeavyDog0410ModeCtrlUnit_GET_ATK, 0x24);

class HeavyDog0410ModeCtrlUnit_START : public HeavyDog0410CtrlUnit {
  public:
    HeavyDog0410ModeCtrlUnit_START(HeavyDog0410VarParam *param)
        : BossCtrlUnit(param), HeavyDog0410CtrlUnit(param) {
        fn_80417568(m_heavyDog0410Param->field_0x3C, m_heavyDog0410Param->field_0x44 + 4);
        m_heavyDog0410Param->m_charInfo->m_position = lbl_805E07A0;
        m_heavyDog0410Param->m_charInfo->m_rotation = lbl_805E07B8;
        m_heavyDog0410Param->field_0x18 = 1;
        m_heavyDog0410Param->field_0x114 = 1;
        m_heavyDog0410Param->field_0x115 = 1;
        m_heavyDog0410Param->AddCtrlUnit(new BossCtrlUnit_PlayCameraMotion(
            m_heavyDog0410Param, 14, &lbl_805E07A0, &lbl_805E07B8, 0.0f, 0.0f));
        m_heavyDog0410Param->field_0x1A = 0;
        fn_801FAF00(fn_80109D64(), 223, 0, 0, 0);
        fn_8035367C("CsdFiles/BChara_HDog", "bchara", "open");
    }
    virtual ~HeavyDog0410ModeCtrlUnit_START();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(HeavyDog0410ModeCtrlUnit_START, 0x20);

class HeavyDog0410ModeCtrlUnit_BELOW_HALF : public HeavyDog0410CtrlUnit {
  public:
    HeavyDog0410ModeCtrlUnit_BELOW_HALF(HeavyDog0410VarParam *param)
        : BossCtrlUnit(param), HeavyDog0410CtrlUnit(param) {
        fn_80417568(m_heavyDog0410Param->field_0x3C, m_heavyDog0410Param->field_0x44 + 2);
        m_heavyDog0410Param->field_0x18 = 1;
        m_heavyDog0410Param->field_0x114 = 1;
        m_heavyDog0410Param->field_0x115 = 1;
        m_heavyDog0410Param->AddCtrlUnit(
            new BossCtrlUnit_PlayCameraMotion(m_heavyDog0410Param, 12, 0.0f, 0.0f));
        m_heavyDog0410Param->field_0x1A = 0;
        fn_801FAF00(fn_80109D64(), 225, 0, 0, 0);
        fn_8034DF48(12, -2);
    }
    virtual ~HeavyDog0410ModeCtrlUnit_BELOW_HALF();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(HeavyDog0410ModeCtrlUnit_BELOW_HALF, 0x20);

class HeavyDog0410ModeCtrlUnit_DESTROY : public HeavyDog0410CtrlUnit {
  public:
    HeavyDog0410ModeCtrlUnit_DESTROY(HeavyDog0410VarParam *param)
        : BossCtrlUnit(param), HeavyDog0410CtrlUnit(param) {
        field_0x18 = 0;
        fn_80417568(m_heavyDog0410Param->field_0x3C, m_heavyDog0410Param->field_0x44 + 3);
        m_heavyDog0410Param->m_charInfo->m_position = lbl_805E07A0;
        m_heavyDog0410Param->m_charInfo->m_rotation = lbl_805E07B8;
        m_heavyDog0410Param->field_0x18 = 1;
        m_heavyDog0410Param->field_0x114 = 1;
        m_heavyDog0410Param->field_0x115 = 1;
        m_heavyDog0410Param->AddCtrlUnit(new BossCtrlUnit_PlayCameraMotion(
            m_heavyDog0410Param, 13, &m_heavyDog0410Param->m_charInfo->m_position,
            &m_heavyDog0410Param->m_charInfo->m_rotation, 0.0f, 0.0f));
        m_heavyDog0410Param->field_0x1A = 0;
        fn_801FAF00(fn_80109D64(), 226, 0, 0, 0);
        m_heavyDog0410Param->AddCtrlUnit(new HeavyDogCtrlUnit_DestroyEffect(m_heavyDog0410Param));
        fn_8034DF48(7, -2);
    }
    virtual ~HeavyDog0410ModeCtrlUnit_DESTROY();
    virtual bool Update(float arg);

    /* Nothing in this file assigns it; the destructor requests a stop of the effect it refers to,
     * if any. */
    /* 0x10 */ boost::weak_ptr<Effect::System::EffectElement> field_0x10;
    /* Update sets it when it calls fn_80175EEC, and does nothing more while it is set. */
    /* 0x18 */ u8 field_0x18;
};
CHECK_SIZEOF(HeavyDog0410ModeCtrlUnit_DESTROY, 0x2C);

class HeavyDog0410ModeCtrlUnit_ATK_ROCKET : public HeavyDog0410CtrlUnit {
  public:
    HeavyDog0410ModeCtrlUnit_ATK_ROCKET(HeavyDog0410VarParam *param)
        : BossCtrlUnit(param), HeavyDog0410CtrlUnit(param) {
        m_heavyDog0410Param->field_0x144[m_heavyDog0410Param->field_0x138].field_0x2 = 1;
    }
    virtual ~HeavyDog0410ModeCtrlUnit_ATK_ROCKET();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(HeavyDog0410ModeCtrlUnit_ATK_ROCKET, 0x20);

class HeavyDog0410ModeCtrlUnit_ATK_BARRIER : public HeavyDog0410CtrlUnit {
  public:
    HeavyDog0410ModeCtrlUnit_ATK_BARRIER(HeavyDog0410VarParam *param)
        : BossCtrlUnit(param), HeavyDog0410CtrlUnit(param) {
        m_heavyDog0410Param->field_0x114 = 0;
        m_heavyDog0410Param->field_0x115 = 1;
        m_timer = 0.0f;
        fn_801FAF00(fn_80109D64(), 229, 0, 0, 0);
        m_heavyDog0410Param->field_0x1C0 = m_heavyDog0410Param->field_0x1C4 + 1.0f;
        m_state = 0;
    }
    virtual ~HeavyDog0410ModeCtrlUnit_ATK_BARRIER();
    virtual bool Update(float arg);

    /* 0x10 */ s32 m_state; /* the case of Update's switch, 0 or 1 (name inferred) */
    /* Update adds its argument to it and compares the sum with 1.5 and m_params.field_0x64; it
     * restarts from 0 when the state changes to 1 (name inferred). */
    /* 0x14 */ float m_timer;
};
CHECK_SIZEOF(HeavyDog0410ModeCtrlUnit_ATK_BARRIER, 0x28);

class HeavyDog0410ModeCtrlUnit_ATK_MINE : public HeavyDog0410CtrlUnit {
  public:
    HeavyDog0410ModeCtrlUnit_ATK_MINE(HeavyDog0410VarParam *param)
        : BossCtrlUnit(param), HeavyDog0410CtrlUnit(param) {
        /* The mine's arguments (local names inferred): arg1 is nodePosition4500, the translation
         * of the motion's node 4500; arg3 is minePoint, the point field_0x1DC from node 4010's
         * translation towards field_0x194; arg2 is mineVector, the horizontal direction from
         * node 4010 towards field_0x194 scaled by field_0x1E0, with field_0x1E4 as its y, plus
         * field_0x11C. fn_8040CEE8 gives the zero vector for a difference shorter than 1e-6, so
         * minePoint is then node 4010's translation and mineVector has no horizontal part before
         * field_0x11C is added. */
        Fn8040Matrix34 matrix;
        MotionTransform *transform = fn_80415E70(m_heavyDog0410Param->field_0x3C, 4500);
        fn_80016370(&matrix, &transform->matrix);
        Fn8040Vector3 nodePosition4500 = fn_8040FB08(&matrix);
        transform = fn_80415E70(m_heavyDog0410Param->field_0x3C, 4010);
        fn_80016370(&matrix, &transform->matrix);
        Fn8040Vector3 nodePosition4010 = fn_8040FB08(&matrix);
        Fn8040Vector3 minePoint = m_heavyDog0410Param->field_0x194 - nodePosition4010;
        minePoint = fn_8040CEE8(minePoint) * m_heavyDog0410Param->field_0x1DC + nodePosition4010;
        Fn8040Vector3 mineVector = m_heavyDog0410Param->field_0x194 - nodePosition4010;
        mineVector.y = 0.0f;
        mineVector = fn_8040CEE8(mineVector) * m_heavyDog0410Param->field_0x1E0;
        mineVector.y = m_heavyDog0410Param->field_0x1E4;
        fn_8040CFF8(&mineVector, &m_heavyDog0410Param->field_0x11C);
        m_heavyDog0410Param->AddCtrlUnit(new HeavyDog0410CtrlUnit_Mine(
            m_heavyDog0410Param, &nodePosition4500, &mineVector, &minePoint));
    }
    virtual ~HeavyDog0410ModeCtrlUnit_ATK_MINE();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(HeavyDog0410ModeCtrlUnit_ATK_MINE, 0x20);

class HeavyDog0410ModeCtrlUnit_MOVETEST : public HeavyDog0410CtrlUnit {
  public:
    HeavyDog0410ModeCtrlUnit_MOVETEST(HeavyDog0410VarParam *param)
        : BossCtrlUnit(param), HeavyDog0410CtrlUnit(param) {}
    virtual ~HeavyDog0410ModeCtrlUnit_MOVETEST();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(HeavyDog0410ModeCtrlUnit_MOVETEST, 0x20);

#endif
