#ifndef GAME_DEVILDOOMMODECTRLUNIT_H
#define GAME_DEVILDOOMMODECTRLUNIT_H

#include "Game/BossCtrlUnit.h"
#include "Game/CsdOneShot.h"
#include "Game/DevilDoomCtrlUnit.h"
#include "Game/DevilDoomVarParam.h"
#include "Game/SharedConstants.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8040_vector_types.h"
#include "Game/fn_804157E8_motion_control.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* The mode control unit of the DEVIL_DOOM boss and the units it creates, one for each mode (class
 * names and bases from RTTI; see DevilDoomCtrlUnit.h). The units request the next mode by storing
 * it in BossVarParam::field_0x14, which BossModeCtrlUnit::Update moves to field_0xC and passes to
 * CreateUnit (see BossVarParam; the word "mode" is inferred from the class names). The mode units'
 * constructors are inline: the copies the linker keeps are those of DevilDoomModeCtrlUnit.cpp,
 * which follow CreateUnit. Each unit uses the variables through
 * DevilDoomCtrlUnit::m_devilDoomParam; the parameter types are inferred from the boss task, which
 * passes its DevilDoomVarParam. The motions are numbered from the base id at
 * BossVarParam::field_0x44: a unit passes that word plus the motion's number as a 16-bit motion
 * id. Where a unit compares fn_80415CDC's result with a motion id, both are locals and the id is
 * set before the call, as the original computes it there and compares the result with it (a
 * later motion request, when made, reads field_0x44 again). */

/* The constructor is inline: the boss task's copy (0x8030C27C) is the one the linker keeps. Its
 * vtable (0x8055B84C-0x8055B884, USA) points to the destructor, Update and CreateUnit declared
 * below; its other function entries are adjustment thunks. The words after the vtable are
 * CreateUnit's switch table. */
class DevilDoomModeCtrlUnit : public DevilDoomCtrlUnit, public BossModeCtrlUnit {
  public:
    DevilDoomModeCtrlUnit(DevilDoomVarParam *param)
        : BossCtrlUnit(param), DevilDoomCtrlUnit(param), BossModeCtrlUnit(param) {}
    virtual ~DevilDoomModeCtrlUnit();
    virtual bool Update(float arg);
    virtual BossCtrlUnit *CreateUnit(int mode);
};
CHECK_SIZEOF(DevilDoomModeCtrlUnit, 0x28);

class DevilDoomModeCtrlUnit_WAIT : public DevilDoomCtrlUnit {
  public:
    /* Blends to motion 4 unless fn_80415CDC already reports it for player 0. */
    DevilDoomModeCtrlUnit_WAIT(DevilDoomVarParam *param)
        : BossCtrlUnit(param), DevilDoomCtrlUnit(param) {
        u16 motionId = m_devilDoomParam->field_0x44 + 4;
        u16 current = fn_80415CDC(m_devilDoomParam->field_0x3C, 0);
        if (current != motionId) {
            stMtnPrepareBlendWithTarget(m_devilDoomParam->field_0x3C, 0, -1, -1);
            fn_80417568(m_devilDoomParam->field_0x3C, m_devilDoomParam->field_0x44 + 4);
            stMtnRequestBlendFrame(m_devilDoomParam->field_0x3C, 0, 30.0f);
            if (m_devilDoomParam->field_0x18F) {
                stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 7, 0);
            }
        }
    }
    virtual ~DevilDoomModeCtrlUnit_WAIT();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(DevilDoomModeCtrlUnit_WAIT, 0x1C);

class DevilDoomModeCtrlUnit_EDIT : public DevilDoomCtrlUnit {
  public:
    DevilDoomModeCtrlUnit_EDIT(DevilDoomVarParam *param)
        : BossCtrlUnit(param), DevilDoomCtrlUnit(param) {
        fn_80417568(m_devilDoomParam->field_0x3C, m_devilDoomParam->field_0x44 + 4);
        m_devilDoomParam->field_0x18C = 1;
    }
    virtual ~DevilDoomModeCtrlUnit_EDIT();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(DevilDoomModeCtrlUnit_EDIT, 0x1C);

class DevilDoomModeCtrlUnit_GET_ATK : public DevilDoomCtrlUnit {
  public:
    DevilDoomModeCtrlUnit_GET_ATK(DevilDoomVarParam *param)
        : BossCtrlUnit(param), DevilDoomCtrlUnit(param) {
        stMtnPrepareBlendWithTarget(m_devilDoomParam->field_0x3C, 0, -1, -1);
        stMtnRequestBlendFrame(m_devilDoomParam->field_0x3C, 0, 30.0f);
        fn_80417568(m_devilDoomParam->field_0x3C, m_devilDoomParam->field_0x44 + 13);
        if (m_devilDoomParam->field_0x191[1]) {
            stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 7, 0);
        }
        m_devilDoomParam->field_0x18 = 1;
        m_devilDoomParam->field_0x18C = 1;
        m_devilDoomParam->field_0x194 = 0;
        m_devilDoomParam->field_0x195 = 0;
        if (!m_devilDoomParam->field_0x18E) {
            m_devilDoomParam->AddCtrlUnit(
                new BossCtrlUnit_PlayCameraMotion(m_devilDoomParam, 16, 0.0f, 0.0f));
            m_devilDoomParam->field_0x1A = 0;
            m_devilDoomParam->field_0x18E = 1;
        }
        if (m_devilDoomParam->field_0x191[0]) {
            field_0xC = 0;
        } else {
            field_0xC = 1;
        }
        field_0x10 = 0;
        m_devilDoomParam->field_0x634 = 0;
        m_devilDoomParam->AddCtrlUnit(
            new DevilDoomCtrlUnit_DoomsEye(m_devilDoomParam, m_devilDoomParam->field_0x44 + 25));
    }
    virtual ~DevilDoomModeCtrlUnit_GET_ATK();
    virtual bool Update(float arg);

    /* 0 when the variables' field_0x191[0] was set at construction, otherwise 1. Update clears
     * field_0x191[field_0xC] on the first update that finds field_0x634 set and sets
     * field_0x191[1 - field_0xC] on the second; the destructor does either that is still
     * needed. The signedness is inferred. */
    /* 0x0C */ s32 field_0xC;
    /* Counts the updates that find field_0x634 set (only the first two act). */
    /* 0x10 */ s32 field_0x10;
};
CHECK_SIZEOF(DevilDoomModeCtrlUnit_GET_ATK, 0x24);

class DevilDoomModeCtrlUnit_INVERSE : public DevilDoomCtrlUnit {
  public:
    DevilDoomModeCtrlUnit_INVERSE(DevilDoomVarParam *param)
        : BossCtrlUnit(param), DevilDoomCtrlUnit(param) {
        stMtnPrepareBlendWithTarget(m_devilDoomParam->field_0x3C, 0, -1, -1);
        fn_80417568(m_devilDoomParam->field_0x3C, m_devilDoomParam->field_0x44 + 11);
        stMtnRequestBlendFrame(m_devilDoomParam->field_0x3C, 0, 30.0f);
        if (m_devilDoomParam->field_0x18F) {
            stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 7, 0);
        }
        m_devilDoomParam->field_0x18C = 1;
        m_devilDoomParam->field_0x634 = 0;
    }
    virtual ~DevilDoomModeCtrlUnit_INVERSE();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(DevilDoomModeCtrlUnit_INVERSE, 0x1C);

class DevilDoomModeCtrlUnit_CHAOS_CONTROL : public DevilDoomCtrlUnit {
  public:
    DevilDoomModeCtrlUnit_CHAOS_CONTROL(DevilDoomVarParam *param)
        : BossCtrlUnit(param), DevilDoomCtrlUnit(param) {
        m_timer = 0.0f;
        u16 motionId = m_devilDoomParam->field_0x44 + 4;
        u16 current = fn_80415CDC(m_devilDoomParam->field_0x3C, 0);
        if (current != motionId) {
            stMtnPrepareBlendWithTarget(m_devilDoomParam->field_0x3C, 0, -1, -1);
            fn_80417568(m_devilDoomParam->field_0x3C, m_devilDoomParam->field_0x44 + 4);
            stMtnRequestBlendFrame(m_devilDoomParam->field_0x3C, 0, 30.0f);
            if (m_devilDoomParam->field_0x18F) {
                stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 7, 0);
            }
        }
        m_devilDoomParam->field_0x18 = 1;
        m_devilDoomParam->field_0x18C = 1;
        m_devilDoomParam->field_0x1B = 1;
        m_devilDoomParam->field_0x19C = 0;
        m_devilDoomParam->field_0x19A = 0;
        if (m_devilDoomParam->field_0x199) {
            field_0x10 = 1;
            m_devilDoomParam->field_0x199 = 0;
        } else {
            field_0x10 = 0;
        }
        fn_801FAF00(fn_80109D64(), 429, 0, 0, 0);
        fn_801D560C(fn_80053164(), 0x8134, 0, 0);
        fn_8034DF48(8, -2);
    }
    virtual ~DevilDoomModeCtrlUnit_CHAOS_CONTROL();
    virtual bool Update(float arg);

    /* Update adds its argument to it and compares the sum with m_params.field_0x20 (name
     * inferred). */
    /* 0x0C */ float m_timer;
    /* Set when the variables' field_0x199 was set at construction (the constructor then clears
     * field_0x199). Once m_timer exceeds m_params.field_0x20, Update requests mode 7 when it is
     * set, otherwise mode 2. */
    /* 0x10 */ u8 field_0x10;
};
CHECK_SIZEOF(DevilDoomModeCtrlUnit_CHAOS_CONTROL, 0x24);

class DevilDoomModeCtrlUnit_ATK_FIRE : public DevilDoomCtrlUnit {
  public:
    DevilDoomModeCtrlUnit_ATK_FIRE(DevilDoomVarParam *param)
        : BossCtrlUnit(param), DevilDoomCtrlUnit(param) {
        m_timer = 0.0f;
        stMtnPrepareBlendWithTarget(m_devilDoomParam->field_0x3C, 0, -1, -1);
        fn_80417568(m_devilDoomParam->field_0x3C, m_devilDoomParam->field_0x44 + 1);
        stMtnRequestBlendFrame(m_devilDoomParam->field_0x3C, 0, 30.0f);
        if (m_devilDoomParam->field_0x18F) {
            stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 7, 0);
        }
        m_devilDoomParam->field_0x18 = 1;
        m_devilDoomParam->field_0x18C = 0;
        m_devilDoomParam->field_0x195 = 0;
        fn_801FAF00(fn_80109D64(), 428, 0, 0, 0);
    }
    virtual ~DevilDoomModeCtrlUnit_ATK_FIRE();
    virtual bool Update(float arg);

    /* Update clears it when it switches from motion 1 to motion 2 and, while motion 2 plays,
     * adds its argument to it and compares the sum with 800 / m_params.field_0xEC (name
     * inferred). */
    /* 0x0C */ float m_timer;
};
CHECK_SIZEOF(DevilDoomModeCtrlUnit_ATK_FIRE, 0x20);

class DevilDoomModeCtrlUnit_ATK_PSYCHO_PREPARE : public DevilDoomCtrlUnit {
  public:
    /* When field_0x1F4 is below m_params.field_0x58 it only requests mode 5 and sets
     * field_0x199; otherwise it sets field_0x1A0, field_0x19C and field_0x1A4 and blends to
     * motion 5. */
    DevilDoomModeCtrlUnit_ATK_PSYCHO_PREPARE(DevilDoomVarParam *param)
        : BossCtrlUnit(param), DevilDoomCtrlUnit(param) {
        m_timer = 0.0f;
        if (m_devilDoomParam->field_0x1F4 < m_devilDoomParam->m_params.field_0x58) {
            m_devilDoomParam->field_0x14 = 5; /* mode 5: CHAOS_CONTROL */
            m_devilDoomParam->field_0x199 = 1;
            return;
        }
        m_devilDoomParam->field_0x1A0 = 1;
        m_devilDoomParam->field_0x19C = 1;
        m_devilDoomParam->field_0x1A4 = m_devilDoomParam->m_params.field_0x30;
        stMtnPrepareBlendWithTarget(m_devilDoomParam->field_0x3C, 0, -1, -1);
        fn_80417568(m_devilDoomParam->field_0x3C, m_devilDoomParam->field_0x44 + 5);
        stMtnRequestBlendFrame(m_devilDoomParam->field_0x3C, 0, 30.0f);
        if (m_devilDoomParam->field_0x18F) {
            stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 7, 0);
        }
        m_devilDoomParam->field_0x18C = 0;
        m_devilDoomParam->field_0x634 = 0;
    }
    virtual ~DevilDoomModeCtrlUnit_ATK_PSYCHO_PREPARE();
    virtual bool Update(float arg);

    /* Update clears it when it switches from motion 5 to motion 6 and, while motion 6 plays,
     * adds its argument to it and compares the sum with m_params.field_0x2C (name inferred). */
    /* 0x0C */ float m_timer;
};
CHECK_SIZEOF(DevilDoomModeCtrlUnit_ATK_PSYCHO_PREPARE, 0x20);

class DevilDoomModeCtrlUnit_ATK_PSYCHO_THROW : public DevilDoomCtrlUnit {
  public:
    /* Blends to motion 9 when field_0x193 is set and clears it, otherwise to motion 8 and sets
     * it. */
    DevilDoomModeCtrlUnit_ATK_PSYCHO_THROW(DevilDoomVarParam *param)
        : BossCtrlUnit(param), DevilDoomCtrlUnit(param) {
        stMtnPrepareBlendWithTarget(m_devilDoomParam->field_0x3C, 0, -1, -1);
        if (m_devilDoomParam->field_0x193) {
            fn_80417568(m_devilDoomParam->field_0x3C, m_devilDoomParam->field_0x44 + 9);
            m_devilDoomParam->field_0x193 = 0;
        } else {
            fn_80417568(m_devilDoomParam->field_0x3C, m_devilDoomParam->field_0x44 + 8);
            m_devilDoomParam->field_0x193 = 1;
        }
        stMtnRequestBlendFrame(m_devilDoomParam->field_0x3C, 0, 30.0f);
        if (m_devilDoomParam->field_0x18F) {
            stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 7, 0);
        }
        m_devilDoomParam->field_0x197 = 1;
        m_devilDoomParam->field_0x634 = 0;
        field_0xC = m_devilDoomParam->m_params.field_0x38 +
                    m_devilDoomParam->field_0x1AC * (m_devilDoomParam->m_params.field_0x34 -
                                                     m_devilDoomParam->m_params.field_0x38);
    }
    virtual ~DevilDoomModeCtrlUnit_ATK_PSYCHO_THROW();
    virtual bool Update(float arg);

    /* m_params.field_0x38 plus field_0x1AC times (m_params.field_0x34 - m_params.field_0x38), set
     * by the constructor; the destructor copies it to the variables' field_0x1A8 when it is
     * greater than 0. */
    /* 0x0C */ float field_0xC;
};
CHECK_SIZEOF(DevilDoomModeCtrlUnit_ATK_PSYCHO_THROW, 0x20);

class DevilDoomModeCtrlUnit_ATK_BIT1 : public DevilDoomCtrlUnit {
  public:
    DevilDoomModeCtrlUnit_ATK_BIT1(DevilDoomVarParam *param)
        : BossCtrlUnit(param), DevilDoomCtrlUnit(param) {
        m_devilDoomParam->field_0x1A0 = 2;
        m_devilDoomParam->field_0x19C = 2;
        m_devilDoomParam->field_0x628 = 1;
        m_devilDoomParam->AddCtrlUnit(new DevilDoomCtrlUnit_BitAttackControl(m_devilDoomParam));
        m_devilDoomParam->field_0x634 = 0;
    }
    virtual ~DevilDoomModeCtrlUnit_ATK_BIT1();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(DevilDoomModeCtrlUnit_ATK_BIT1, 0x1C);

class DevilDoomModeCtrlUnit_ATK_BIT2 : public DevilDoomCtrlUnit {
  public:
    DevilDoomModeCtrlUnit_ATK_BIT2(DevilDoomVarParam *param)
        : BossCtrlUnit(param), DevilDoomCtrlUnit(param) {
        m_devilDoomParam->field_0x1A0 = 2;
        m_devilDoomParam->field_0x19C = 2;
        m_devilDoomParam->field_0x628 = 2;
        m_devilDoomParam->AddCtrlUnit(new DevilDoomCtrlUnit_BitAttackControl(m_devilDoomParam));
        m_devilDoomParam->field_0x634 = 0;
        fn_801FAF00(fn_80109D64(), 427, 0, 0, 0);
    }
    virtual ~DevilDoomModeCtrlUnit_ATK_BIT2();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(DevilDoomModeCtrlUnit_ATK_BIT2, 0x1C);

class DevilDoomModeCtrlUnit_ATK_BIT3 : public DevilDoomCtrlUnit {
  public:
    DevilDoomModeCtrlUnit_ATK_BIT3(DevilDoomVarParam *param)
        : BossCtrlUnit(param), DevilDoomCtrlUnit(param) {
        m_devilDoomParam->field_0x1A0 = 2;
        m_devilDoomParam->field_0x19C = 2;
        m_devilDoomParam->field_0x628 = 3;
        m_devilDoomParam->AddCtrlUnit(new DevilDoomCtrlUnit_BitAttackControl(m_devilDoomParam));
        m_devilDoomParam->field_0x634 = 0;
    }
    virtual ~DevilDoomModeCtrlUnit_ATK_BIT3();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(DevilDoomModeCtrlUnit_ATK_BIT3, 0x1C);

class DevilDoomModeCtrlUnit_START : public DevilDoomCtrlUnit {
  public:
    DevilDoomModeCtrlUnit_START(DevilDoomVarParam *param)
        : BossCtrlUnit(param), DevilDoomCtrlUnit(param) {
        fn_80417568(m_devilDoomParam->field_0x3C, m_devilDoomParam->field_0x44 + 10);
        if (m_devilDoomParam->field_0x18F) {
            stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 7, 0);
        }
        m_devilDoomParam->field_0x18 = 1;
        m_devilDoomParam->field_0x18C = 1;
        m_devilDoomParam->AddCtrlUnit(
            new BossCtrlUnit_PlayCameraMotion(m_devilDoomParam, 17, 0.0f, 0.0f));
        m_devilDoomParam->field_0x1A = 0;
        m_devilDoomParam->field_0x19C = 0;
        m_devilDoomParam->field_0x19A = 0;
        fn_801FAF00(fn_80109D64(), 420, 0, 0, 0);
        fn_801D560C(fn_80053164(), 0x8144, 0, 0);
        fn_8035367C("CsdFiles/BChara_DevDoom", "bchara", "open");
    }
    virtual ~DevilDoomModeCtrlUnit_START();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(DevilDoomModeCtrlUnit_START, 0x1C);

class DevilDoomModeCtrlUnit_BELOW_HALF : public DevilDoomCtrlUnit {
  public:
    /* When field_0x191[1] is set, the camera motion unit gets the character's position and its
     * rotation with pi added to y; otherwise it is created without them. */
    DevilDoomModeCtrlUnit_BELOW_HALF(DevilDoomVarParam *param)
        : BossCtrlUnit(param), DevilDoomCtrlUnit(param) {
        /* passCharPose keeps the result of the field_0x191[1] test for the camera motion: testing
         * field_0x191[1] again there compiles to other code. */
        bool passCharPose = false;
        stMtnPrepareBlendWithTarget(m_devilDoomParam->field_0x3C, 0, -1, -1);
        stMtnRequestBlendFrame(m_devilDoomParam->field_0x3C, 0, 30.0f);
        fn_80417568(m_devilDoomParam->field_0x3C, m_devilDoomParam->field_0x44 + 13);
        if (m_devilDoomParam->field_0x191[1]) {
            stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 7, 0);
            passCharPose = true;
        }
        m_devilDoomParam->field_0x18 = 1;
        m_devilDoomParam->field_0x18C = 1;
        m_devilDoomParam->field_0x194 = 0;
        m_devilDoomParam->field_0x195 = 0;
        if (passCharPose) {
            Fn8040Vector3 position(m_devilDoomParam->m_charInfo->m_position);
            UnkRotation3 rotation(m_devilDoomParam->m_charInfo->m_rotation);
            rotation.y += lbl_805FAD38;
            m_devilDoomParam->AddCtrlUnit(new BossCtrlUnit_PlayCameraMotion(
                m_devilDoomParam, 16, &position, &rotation, 0.0f, 0.0f));
        } else {
            m_devilDoomParam->AddCtrlUnit(
                new BossCtrlUnit_PlayCameraMotion(m_devilDoomParam, 16, 0.0f, 0.0f));
        }
        m_devilDoomParam->field_0x1A = 0;
        if (m_devilDoomParam->field_0x191[0]) {
            field_0xC = 0;
        } else {
            field_0xC = 1;
        }
        field_0x10 = 0;
        m_devilDoomParam->field_0x634 = 0;
        m_devilDoomParam->AddCtrlUnit(
            new DevilDoomCtrlUnit_DoomsEye(m_devilDoomParam, m_devilDoomParam->field_0x44 + 25));
        m_devilDoomParam->field_0x19C = 0;
        fn_801FAF00(fn_80109D64(), 424, 0, 0, 0);
        fn_801D560C(fn_80053164(), 0x8144, 0, 0);
        fn_8034DF48(12, -2);
    }
    virtual ~DevilDoomModeCtrlUnit_BELOW_HALF();
    virtual bool Update(float arg);

    /* As in DevilDoomModeCtrlUnit_GET_ATK (the signedness of field_0xC is inferred). */
    /* 0x0C */ s32 field_0xC;
    /* 0x10 */ s32 field_0x10;
};
CHECK_SIZEOF(DevilDoomModeCtrlUnit_BELOW_HALF, 0x24);

class DevilDoomModeCtrlUnit_DEATH : public DevilDoomCtrlUnit {
  public:
    /* The camera motion unit is created as in DevilDoomModeCtrlUnit_BELOW_HALF. */
    DevilDoomModeCtrlUnit_DEATH(DevilDoomVarParam *param)
        : BossCtrlUnit(param), DevilDoomCtrlUnit(param) {
        bool passCharPose = false;
        field_0xC = 0;
        fn_80417568(m_devilDoomParam->field_0x3C, m_devilDoomParam->field_0x44 + 14);
        if (m_devilDoomParam->field_0x191[1]) {
            stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 7, 0);
            passCharPose = true;
        }
        m_devilDoomParam->field_0x18 = 1;
        m_devilDoomParam->field_0x18C = 1;
        if (passCharPose) {
            Fn8040Vector3 position(m_devilDoomParam->m_charInfo->m_position);
            UnkRotation3 rotation(m_devilDoomParam->m_charInfo->m_rotation);
            rotation.y += lbl_805FAD38;
            m_devilDoomParam->AddCtrlUnit(new BossCtrlUnit_PlayCameraMotion(
                m_devilDoomParam, 15, &position, &rotation, 0.0f, 0.0f));
        } else {
            m_devilDoomParam->AddCtrlUnit(
                new BossCtrlUnit_PlayCameraMotion(m_devilDoomParam, 15, 0.0f, 0.0f));
        }
        m_devilDoomParam->field_0x1A = 0;
        m_devilDoomParam->field_0x19C = 0;
        m_devilDoomParam->field_0x19A = 0;
        fn_801FAF00(fn_80109D64(), 425, 0, 0, 0);
        fn_801D560C(fn_80053164(), 0x8145, 0, 0);
        fn_8034DF48(7, -2);
    }
    virtual ~DevilDoomModeCtrlUnit_DEATH();
    virtual bool Update(float arg);

    /* Update sets it when it calls fn_80175EEC, and does not call it again while it is set. */
    /* 0x0C */ u8 field_0xC;
};
CHECK_SIZEOF(DevilDoomModeCtrlUnit_DEATH, 0x20);

#endif
