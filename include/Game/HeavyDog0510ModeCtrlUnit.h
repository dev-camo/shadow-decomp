#ifndef GAME_HEAVYDOG0510MODECTRLUNIT_H
#define GAME_HEAVYDOG0510MODECTRLUNIT_H

#include "Game/BossCtrlUnit.h"
#include "Game/CsdOneShot.h"
#include "Game/DebugMissionClearCollision.h"
#include "Game/HeavyDog0510CtrlUnit.h"
#include "Game/HeavyDog0510VarParam.h"
#include "Game/HeavyDogCtrlUnit.h"
#include "Game/SharedConstants.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/fn_8040_random.h"
#include "Game/fn_8040_vector_types.h"
#include "Game/fn_804157E8_motion_control.h"
#include "Game/fn_804157E8_motion_range.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* The mode units of the HEAVY_DOG_0510 boss (class names and bases from RTTI): the units that
 * HeavyDog0510ModeCtrlUnit::CreateUnit (HeavyDog0510ModeCtrlUnit.cpp) creates for the mode in
 * BossVarParam::field_0xC. Their Update bodies request the next mode by storing its number in
 * BossVarParam::field_0x14, which BossModeCtrlUnit::Update moves to field_0xC and passes to
 * CreateUnit (see BossVarParam). The number selects a CreateUnit case or the default: mode 2,
 * which several units request, has no case and gives WAIT. All their methods are inline; the
 * linker keeps the copies and vtables of that file.
 * The meanings of the modes are unknown beyond the class names, and the behaviour descriptions
 * below are inferred. */

class HeavyDog0510ModeCtrlUnit_WAIT : public HeavyDog0510CtrlUnit {
  public:
    HeavyDog0510ModeCtrlUnit_WAIT(HeavyDog0510VarParam *param)
        : BossCtrlUnit(param), HeavyDog0510CtrlUnit(param) {
        m_heavyDog0510Param->field_0x114 = 0;
        m_heavyDog0510Param->field_0x115 = 0;
    }

    virtual ~HeavyDog0510ModeCtrlUnit_WAIT() {}

    virtual bool Update(float arg) {
        if (m_heavyDog0510Param->field_0x1C0 >= m_heavyDog0510Param->field_0x1C4) {
            m_heavyDog0510Param->field_0x14 = 3;
        } else {
            m_heavyDog0510Param->field_0x14 = 4;
        }
        return false;
    }
};
CHECK_SIZEOF(HeavyDog0510ModeCtrlUnit_WAIT, 0x20);

class HeavyDog0510ModeCtrlUnit_EDIT : public HeavyDog0510CtrlUnit {
  public:
    HeavyDog0510ModeCtrlUnit_EDIT(HeavyDog0510VarParam *param)
        : BossCtrlUnit(param), HeavyDog0510CtrlUnit(param) {
        m_heavyDog0510Param->field_0x114 = 1;
    }

    virtual ~HeavyDog0510ModeCtrlUnit_EDIT() {
        m_heavyDog0510Param->field_0x114 = 0;
    }

    virtual bool Update(float arg) {
        return false;
    }
};
CHECK_SIZEOF(HeavyDog0510ModeCtrlUnit_EDIT, 0x20);

class HeavyDog0510ModeCtrlUnit_CHASE : public HeavyDog0510CtrlUnit {
  public:
    HeavyDog0510ModeCtrlUnit_CHASE(HeavyDog0510VarParam *param)
        : BossCtrlUnit(param), HeavyDog0510CtrlUnit(param) {
        m_heavyDog0510Param->field_0x114 = 0;
        m_heavyDog0510Param->field_0x115 = 0;
        field_0x10 = 0.0f;
        field_0x14 = 0;
    }

    virtual ~HeavyDog0510ModeCtrlUnit_CHASE() {}

    virtual bool Update(float arg) {
        field_0x10 += arg;

        m_heavyDog0510Param->field_0xF8.y = m_heavyDog0510Param->field_0x17C.y;
        if (m_heavyDog0510Param->field_0xF8.y < 0.0f) {
            m_heavyDog0510Param->field_0xF8.y = 0.0f;
        } else if (m_heavyDog0510Param->field_0xF8.y > 200.0f) {
            m_heavyDog0510Param->field_0xF8.y = 200.0f;
        }

        if (fn_8040CF50(&m_heavyDog0510Param->field_0x17C) < 10.0f) {
            m_heavyDog0510Param->field_0x104 = m_heavyDog0510Param->field_0x1B8 + lbl_805FAD38;
        } else {
            m_heavyDog0510Param->field_0x104 = m_heavyDog0510Param->field_0x1B8;
        }

        if (m_heavyDog0510Param->field_0x1B0 < m_heavyDog0510Param->m_params.field_0x5C - 30.0f &&
            m_heavyDog0510Param->field_0x1B4 < m_heavyDog0510Param->m_params.field_0x10) {
            m_heavyDog0510Param->field_0x14 = 10;
        }

        if (m_heavyDog0510Param->field_0x17C.y < -52.0f) {
            if (field_0x10 > m_heavyDog0510Param->field_0x1E0) {
                /* The same bomb as in HeavyDog0510ModeCtrlUnit_ATK_MINE's constructor. */
                Fn8040Matrix34 matrix;
                MotionTransform *transform = fn_80415E70(m_heavyDog0510Param->field_0x3C, 0x1194);

                fn_80016370(&matrix, &transform->matrix);
                Fn8040Vector3 position(fn_8040FB08(&matrix));
                Fn8040Vector3 velocity(lbl_805E07A0 - m_heavyDog0510Param->m_charInfo->m_position);
                velocity.y = 0.0f;
                velocity = fn_8040CEE8(velocity) * m_heavyDog0510Param->field_0x1E8;

                Fn8040Vector3 offset(m_heavyDog0510Param->field_0x1EC * fn_8040EE90(), 0.0f, 0.0f);
                fn_8040C148(&matrix, &lbl_805E0720);
                fn_8040B848(&matrix, lbl_805FAD38 * fn_8040EE90());
                offset = matrix * offset;
                fn_8040CFF8(&velocity, &offset);

                m_heavyDog0510Param->AddCtrlUnit(
                    new HeavyDog0510CtrlUnit_Bomb(m_heavyDog0510Param, &position, &velocity));

                if (++field_0x14 >= m_heavyDog0510Param->field_0x1E4) {
                    field_0x14 = 0;
                    field_0x10 = 0.0f;
                } else {
                    field_0x10 = m_heavyDog0510Param->field_0x1E0 - 0.1f;
                }
            }
        } else {
            m_heavyDog0510Param->field_0x144[m_heavyDog0510Param->field_0x138].field_0x2 = 1;
        }
        return false;
    }

    /* 0x10 */ float field_0x10; /* cleared by the constructor; Update adds its argument */
    /* Cleared by the constructor. Update's count of bomb creation attempts in the current batch:
     * it is incremented after each attempt, even when the Bomb allocation fails, and cleared
     * (with field_0x10) when it reaches or exceeds field_0x1E4. */
    /* 0x14 */ s32 field_0x14;
};
CHECK_SIZEOF(HeavyDog0510ModeCtrlUnit_CHASE, 0x28);

class HeavyDog0510ModeCtrlUnit_ESCAPE : public HeavyDog0510CtrlUnit {
  public:
    HeavyDog0510ModeCtrlUnit_ESCAPE(HeavyDog0510VarParam *param)
        : BossCtrlUnit(param), HeavyDog0510CtrlUnit(param) {
        m_heavyDog0510Param->field_0x114 = 0;
        m_heavyDog0510Param->field_0x115 = 0;
        m_heavyDog0510Param->field_0x104 = m_heavyDog0510Param->field_0x1B8 + lbl_805FAD38;
        m_heavyDog0510Param->field_0x117 = 0;
        field_0x10 = 0.0f;
        field_0x14 = 0;
    }

    virtual ~HeavyDog0510ModeCtrlUnit_ESCAPE() {}

    virtual bool Update(float arg) {
        field_0x10 += arg;

        m_heavyDog0510Param->field_0xF8.y = m_heavyDog0510Param->field_0x17C.y;
        if (m_heavyDog0510Param->field_0xF8.y < 0.0f) {
            m_heavyDog0510Param->field_0xF8.y = 0.0f;
        } else if (m_heavyDog0510Param->field_0xF8.y > 180.0f) {
            m_heavyDog0510Param->field_0xF8.y = 180.0f;
        }

        if (m_heavyDog0510Param->field_0x117) {
            m_heavyDog0510Param->field_0x104 = m_heavyDog0510Param->field_0x1B8 + lbl_805FAD38;
            m_heavyDog0510Param->field_0x117 = 0;
        }

        if (m_heavyDog0510Param->field_0x1C0 >= m_heavyDog0510Param->field_0x1C4) {
            m_heavyDog0510Param->field_0x14 = 2;
            fn_801FAF00(fn_80109D64(), 0xF1, 0, 0, 0);
        }

        if (m_heavyDog0510Param->field_0x17C.y < -52.0f) {
            if (field_0x10 > m_heavyDog0510Param->field_0x1E0) {
                /* The same bomb as in HeavyDog0510ModeCtrlUnit_ATK_MINE's constructor. */
                Fn8040Matrix34 matrix;
                MotionTransform *transform = fn_80415E70(m_heavyDog0510Param->field_0x3C, 0x1194);

                fn_80016370(&matrix, &transform->matrix);
                Fn8040Vector3 position(fn_8040FB08(&matrix));
                Fn8040Vector3 velocity(lbl_805E07A0 - m_heavyDog0510Param->m_charInfo->m_position);
                velocity.y = 0.0f;
                velocity = fn_8040CEE8(velocity) * m_heavyDog0510Param->field_0x1E8;

                Fn8040Vector3 offset(m_heavyDog0510Param->field_0x1EC * fn_8040EE90(), 0.0f, 0.0f);
                fn_8040C148(&matrix, &lbl_805E0720);
                fn_8040B848(&matrix, lbl_805FAD38 * fn_8040EE90());
                offset = matrix * offset;
                fn_8040CFF8(&velocity, &offset);

                m_heavyDog0510Param->AddCtrlUnit(
                    new HeavyDog0510CtrlUnit_Bomb(m_heavyDog0510Param, &position, &velocity));

                if (++field_0x14 >= m_heavyDog0510Param->field_0x1E4) {
                    field_0x14 = 0;
                    field_0x10 = 0.0f;
                } else {
                    field_0x10 = m_heavyDog0510Param->field_0x1E0 - 0.1f;
                }
            }
        } else {
            m_heavyDog0510Param->field_0x144[m_heavyDog0510Param->field_0x138].field_0x2 = 1;
        }
        return false;
    }

    /* 0x10 */ float field_0x10; /* as in HeavyDog0510ModeCtrlUnit_CHASE */
    /* 0x14 */ s32 field_0x14;   /* as in HeavyDog0510ModeCtrlUnit_CHASE */
};
CHECK_SIZEOF(HeavyDog0510ModeCtrlUnit_ESCAPE, 0x28);

class HeavyDog0510ModeCtrlUnit_GET_ATK : public HeavyDog0510CtrlUnit {
  public:
    HeavyDog0510ModeCtrlUnit_GET_ATK(HeavyDog0510VarParam *param)
        : BossCtrlUnit(param), HeavyDog0510CtrlUnit(param) {
        m_heavyDog0510Param->field_0x18 = 1;
        m_heavyDog0510Param->field_0x114 = 1;
        m_heavyDog0510Param->field_0x1D0++;
        field_0x10 = 0.0f;
    }

    virtual ~HeavyDog0510ModeCtrlUnit_GET_ATK() {
        m_heavyDog0510Param->field_0x18 = 0;
    }

    virtual bool Update(float arg) {
        if (m_heavyDog0510Param->field_0x18) {
            field_0x10 += lbl_8057D6A0.field_0x18;
            if (field_0x10 > m_heavyDog0510Param->m_params.field_0x24) {
                m_heavyDog0510Param->field_0x14 = 2;
            }
        }
        return false;
    }

    /* Cleared by the constructor; Update adds the float at 0x18 of lbl_8057D6A0 while
     * BossVarParam::field_0x18 is set. */
    /* 0x10 */ float field_0x10;
};
CHECK_SIZEOF(HeavyDog0510ModeCtrlUnit_GET_ATK, 0x24);

class HeavyDog0510ModeCtrlUnit_START : public HeavyDog0510CtrlUnit {
  public:
    HeavyDog0510ModeCtrlUnit_START(HeavyDog0510VarParam *param)
        : BossCtrlUnit(param), HeavyDog0510CtrlUnit(param) {
        fn_80417568(m_heavyDog0510Param->field_0x3C, m_heavyDog0510Param->field_0x44 + 3);
        m_heavyDog0510Param->m_charInfo->m_position = lbl_805E07A0;
        m_heavyDog0510Param->m_charInfo->m_rotation = lbl_805E07B8;
        m_heavyDog0510Param->field_0x18 = 1;
        m_heavyDog0510Param->field_0x114 = 1;
        m_heavyDog0510Param->field_0x115 = 1;
        m_heavyDog0510Param->AddCtrlUnit(new BossCtrlUnit_PlayCameraMotion(
            m_heavyDog0510Param, 10, &lbl_805E07A0, &lbl_805E07B8, 0.0f, 0.0f));
        m_heavyDog0510Param->field_0x1A = 0;
        fn_801FAF00(fn_80109D64(), 0xEC, 0, 0, 0);
        fn_8035367C("CsdFiles/BChara_BFalcon", "bchara", "open");
    }

    virtual ~HeavyDog0510ModeCtrlUnit_START() {
        stMtnPrepareBlendWithTarget(m_heavyDog0510Param->field_0x3C, 0, -1, -1);
        stMtnRequestBlendFrame(m_heavyDog0510Param->field_0x3C, 0, 20.0f);
        fn_80417568(m_heavyDog0510Param->field_0x3C, m_heavyDog0510Param->field_0x44 + 1);
        m_heavyDog0510Param->field_0x18 = 0;
        m_heavyDog0510Param->field_0x1A = 1;
    }

    virtual bool Update(float arg) {
        if (stMtnControlWorkFlag(m_heavyDog0510Param->field_0x3C, 0, 3, 1)) {
            m_heavyDog0510Param->field_0x14 = 2;
        }
        return false;
    }
};
CHECK_SIZEOF(HeavyDog0510ModeCtrlUnit_START, 0x20);

class HeavyDog0510ModeCtrlUnit_BELOW_HALF : public HeavyDog0510CtrlUnit {
  public:
    HeavyDog0510ModeCtrlUnit_BELOW_HALF(HeavyDog0510VarParam *param)
        : BossCtrlUnit(param), HeavyDog0510CtrlUnit(param) {
        fn_80417568(m_heavyDog0510Param->field_0x3C, m_heavyDog0510Param->field_0x44 + 4);
        m_heavyDog0510Param->field_0x18 = 1;
        m_heavyDog0510Param->field_0x114 = 1;
        m_heavyDog0510Param->field_0x115 = 1;
        m_heavyDog0510Param->AddCtrlUnit(
            new BossCtrlUnit_PlayCameraMotion(m_heavyDog0510Param, 8, 0.0f, 0.0f));
        m_heavyDog0510Param->field_0x1A = 0;
        fn_801FAF00(fn_80109D64(), 0xEE, 0, 0, 0);
        fn_8034DF48(12, -2);
    }

    virtual ~HeavyDog0510ModeCtrlUnit_BELOW_HALF() {
        stMtnPrepareBlendWithTarget(m_heavyDog0510Param->field_0x3C, 0, -1, -1);
        stMtnRequestBlendFrame(m_heavyDog0510Param->field_0x3C, 0, 40.0f);
        fn_80417568(m_heavyDog0510Param->field_0x3C, m_heavyDog0510Param->field_0x44 + 1);
        m_heavyDog0510Param->field_0x18 = 0;
        m_heavyDog0510Param->field_0x1A = 1;
    }

    virtual bool Update(float arg) {
        if (stMtnControlWorkFlag(m_heavyDog0510Param->field_0x3C, 0, 3, 1)) {
            m_heavyDog0510Param->field_0x14 = 2;
        }
        return false;
    }
};
CHECK_SIZEOF(HeavyDog0510ModeCtrlUnit_BELOW_HALF, 0x20);

class HeavyDog0510ModeCtrlUnit_DESTROY : public HeavyDog0510CtrlUnit {
  public:
    HeavyDog0510ModeCtrlUnit_DESTROY(HeavyDog0510VarParam *param)
        : BossCtrlUnit(param), HeavyDog0510CtrlUnit(param) {
        field_0x10 = false;
        fn_80417568(m_heavyDog0510Param->field_0x3C, m_heavyDog0510Param->field_0x44 + 2);
        m_heavyDog0510Param->m_charInfo->m_position = lbl_805E07A0;
        m_heavyDog0510Param->m_charInfo->m_rotation = lbl_805E07B8;
        m_heavyDog0510Param->field_0x18 = 1;
        m_heavyDog0510Param->field_0x114 = 1;
        m_heavyDog0510Param->field_0x115 = 1;
        m_heavyDog0510Param->AddCtrlUnit(new BossCtrlUnit_PlayCameraMotion(
            m_heavyDog0510Param, 9, &lbl_805E07A0, &lbl_805E07B8, 0.0f, 0.0f));
        m_heavyDog0510Param->field_0x1A = 0;
        fn_801FAF00(fn_80109D64(), 0xEF, 0, 0, 0);
        m_heavyDog0510Param->AddCtrlUnit(new HeavyDogCtrlUnit_DestroyEffect(m_heavyDog0510Param));
        fn_8034DF48(7, -2);
    }

    virtual ~HeavyDog0510ModeCtrlUnit_DESTROY() {}

    virtual bool Update(float arg) {
        if (!field_0x10) {
            MotionRange range;

            fn_80415D40(m_heavyDog0510Param->field_0x3C, 0, &range);
            if (range.currentFrame > range.endFrame - 60.0f) {
                fn_80175EEC(fn_80074470(), 6, false);
                field_0x10 = true;
            }
        }
        return false;
    }

    /* 0x10 */ bool field_0x10; /* cleared by the constructor; set when Update calls fn_80175EEC */
};
CHECK_SIZEOF(HeavyDog0510ModeCtrlUnit_DESTROY, 0x24);

class HeavyDog0510ModeCtrlUnit_ATK_ROCKET : public HeavyDog0510CtrlUnit {
  public:
    HeavyDog0510ModeCtrlUnit_ATK_ROCKET(HeavyDog0510VarParam *param)
        : BossCtrlUnit(param), HeavyDog0510CtrlUnit(param) {
        m_heavyDog0510Param->field_0x144[m_heavyDog0510Param->field_0x138].field_0x2 = 1;
    }

    virtual ~HeavyDog0510ModeCtrlUnit_ATK_ROCKET() {}

    virtual bool Update(float arg) {
        m_heavyDog0510Param->field_0x14 = 2;
        return false;
    }
};
CHECK_SIZEOF(HeavyDog0510ModeCtrlUnit_ATK_ROCKET, 0x20);

class HeavyDog0510ModeCtrlUnit_ATK_BARRIER : public HeavyDog0510CtrlUnit {
  public:
    HeavyDog0510ModeCtrlUnit_ATK_BARRIER(HeavyDog0510VarParam *param)
        : BossCtrlUnit(param), HeavyDog0510CtrlUnit(param) {
        m_heavyDog0510Param->field_0x114 = 0;
        m_heavyDog0510Param->field_0x115 = 1;
        field_0x10 = 0.0f;
        fn_801FAF00(fn_80109D64(), 0xF2, 0, 0, 0);
        m_heavyDog0510Param->field_0x1C0 = m_heavyDog0510Param->field_0x1C4 + 1.0f;
        field_0x14 = 0;
    }

    virtual ~HeavyDog0510ModeCtrlUnit_ATK_BARRIER() {
        m_heavyDog0510Param->field_0x1C0 = 0.0f;
    }

    virtual bool Update(float arg) {
        m_heavyDog0510Param->field_0xF8.y = m_heavyDog0510Param->m_charInfo->m_position.y;
        m_heavyDog0510Param->field_0x104 = m_heavyDog0510Param->field_0x110;
        field_0x10 += arg;

        switch (field_0x14) {
        case 0:
            if (field_0x10 > 1.5f && fn_8040CF50(&m_heavyDog0510Param->field_0x11C) <
                                         m_heavyDog0510Param->m_params.field_0x14) {
                Fn8040Vector3 position(m_heavyDog0510Param->m_charInfo->m_position);
                position.y += 0.5f * m_heavyDog0510Param->m_params.field_0x60;
                m_heavyDog0510Param->AddCtrlUnit(
                    new HeavyDogCtrlUnit_Barrier(m_heavyDog0510Param, &position, false));
                m_heavyDog0510Param->field_0x1C0 = -1.0f;
                field_0x14 = 1;
                field_0x10 = 0.0f;
            }
            break;
        case 1:
            if (field_0x10 > m_heavyDog0510Param->m_params.field_0x64) {
                fn_801FAF00(fn_80109D64(), 0xF0, 0, 0, 0);
                m_heavyDog0510Param->field_0x14 = 2;
            }
            break;
        }
        return false;
    }

    /* 0x10 */ float field_0x10; /* cleared by the constructor; Update adds its argument */
    /* 0x14 */ s32 field_0x14;   /* Update's state, 0 or 1; cleared by the constructor */
};
CHECK_SIZEOF(HeavyDog0510ModeCtrlUnit_ATK_BARRIER, 0x28);

class HeavyDog0510ModeCtrlUnit_ATK_MINE : public HeavyDog0510CtrlUnit {
  public:
    HeavyDog0510ModeCtrlUnit_ATK_MINE(HeavyDog0510VarParam *param)
        : BossCtrlUnit(param), HeavyDog0510CtrlUnit(param) {
        /* Creates a HeavyDog0510CtrlUnit_Bomb and hands it to AddCtrlUnit (as null when the
         * allocation fails). Its position is the translation of the matrix that fn_80016370 makes
         * from the transform of node 0x1194 of the boss's motion. Its velocity (name inferred) is
         * the direction from the boss's position to the origin without the y component, normalised
         * by fn_8040CEE8 and scaled by field_0x1E8, plus (field_0x1EC times a random factor, 0, 0)
         * turned about the y axis by pi times another random factor (behaviour inferred).
         * HeavyDog0510ModeCtrlUnit_CHASE and HeavyDog0510ModeCtrlUnit_ESCAPE repeat these
         * statements: an inline function holding them is not expanded, as it calls other inline
         * functions, whereas the original has the code in place in all three functions. */
        Fn8040Matrix34 matrix;
        MotionTransform *transform = fn_80415E70(m_heavyDog0510Param->field_0x3C, 0x1194);

        fn_80016370(&matrix, &transform->matrix);
        Fn8040Vector3 position(fn_8040FB08(&matrix));
        Fn8040Vector3 velocity(lbl_805E07A0 - m_heavyDog0510Param->m_charInfo->m_position);
        velocity.y = 0.0f;
        velocity = fn_8040CEE8(velocity) * m_heavyDog0510Param->field_0x1E8;

        Fn8040Vector3 offset(m_heavyDog0510Param->field_0x1EC * fn_8040EE90(), 0.0f, 0.0f);
        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B848(&matrix, lbl_805FAD38 * fn_8040EE90());
        offset = matrix * offset;
        fn_8040CFF8(&velocity, &offset);

        m_heavyDog0510Param->AddCtrlUnit(
            new HeavyDog0510CtrlUnit_Bomb(m_heavyDog0510Param, &position, &velocity));
    }

    virtual ~HeavyDog0510ModeCtrlUnit_ATK_MINE() {}

    virtual bool Update(float arg) {
        m_heavyDog0510Param->field_0x14 = 2;
        return false;
    }
};
CHECK_SIZEOF(HeavyDog0510ModeCtrlUnit_ATK_MINE, 0x20);

class HeavyDog0510ModeCtrlUnit_MOVETEST : public HeavyDog0510CtrlUnit {
  public:
    HeavyDog0510ModeCtrlUnit_MOVETEST(HeavyDog0510VarParam *param)
        : BossCtrlUnit(param), HeavyDog0510CtrlUnit(param) {}

    virtual ~HeavyDog0510ModeCtrlUnit_MOVETEST() {}

    virtual bool Update(float arg) {
        m_heavyDog0510Param->field_0xF8.y = m_heavyDog0510Param->field_0x17C.y;
        m_heavyDog0510Param->field_0x104 = m_heavyDog0510Param->field_0x1B8;
        m_heavyDog0510Param->field_0x14 = 2;
        return false;
    }
};
CHECK_SIZEOF(HeavyDog0510ModeCtrlUnit_MOVETEST, 0x20);

#endif
