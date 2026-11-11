/* DevilDoomModeCtrlUnit (class name from RTTI): the mode control unit of the DEVIL_DOOM boss, and
 * the units it creates for each mode, DevilDoomModeCtrlUnit_WAIT to _DEATH (see
 * DevilDoomModeCtrlUnit.h). File name inferred from the classes. Definitions are in reverse
 * address order (deferred inlining emits functions last-defined first). */
#include "Game/DevilDoomModeCtrlUnit.h"
#include "Game/BossCtrlUnit.h"
#include "Game/DebugMissionClearCollision.h"
#include "Game/DevilDoomCtrlUnit.h"
#include "Game/DevilDoomVarParam.h"
#include "Game/SharedConstants.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8000A950_near_zero.h"
#include "Game/fn_8040_random.h"
#include "Game/fn_8040_vector_types.h"
#include "Game/fn_8041116C_matrix_stack.h"
#include "Game/fn_804157E8_motion_control.h"
#include "Game/fn_804157E8_motion_range.h"
#include "boost/shared_ptr.hpp"

DevilDoomModeCtrlUnit_DEATH::~DevilDoomModeCtrlUnit_DEATH() {}

bool DevilDoomModeCtrlUnit_DEATH::Update(float arg) {
    u16 motionId = m_devilDoomParam->field_0x44 + 14;
    u16 current = fn_80415CDC(m_devilDoomParam->field_0x3C, 0);
    if (current == motionId) {
        if (stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 3, 1) != 0) {
            fn_80417568(m_devilDoomParam->field_0x3C, m_devilDoomParam->field_0x44 + 15);
            if (m_devilDoomParam->field_0x191[1]) {
                stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 7, 0);
            }
        }
    } else {
        motionId = m_devilDoomParam->field_0x44 + 15;
        current = fn_80415CDC(m_devilDoomParam->field_0x3C, 0);
        if (current == motionId) {
            if (stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 3, 1) != 0) {
                fn_80417568(m_devilDoomParam->field_0x3C, m_devilDoomParam->field_0x44 + 16);
                if (m_devilDoomParam->field_0x191[1]) {
                    stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 7, 0);
                }
            }
        } else {
            motionId = m_devilDoomParam->field_0x44 + 16;
            current = fn_80415CDC(m_devilDoomParam->field_0x3C, 0);
            if (current == motionId && !field_0xC) {
                MotionRange range;
                fn_80415D40(m_devilDoomParam->field_0x3C, 0, &range);
                if (range.currentFrame > range.endFrame - 60.0f) {
                    fn_80175EEC(fn_80074470(), 6, false);
                    field_0xC = 1;
                }
            }
        }
    }
    return false;
}

DevilDoomModeCtrlUnit_BELOW_HALF::~DevilDoomModeCtrlUnit_BELOW_HALF() {
    m_devilDoomParam->field_0x18 = 0;
    m_devilDoomParam->field_0x194 = 1;
    m_devilDoomParam->field_0x195 = 1;
    m_devilDoomParam->field_0x1A = 1;
    /* Sets field_0x18F to field_0x191[0] != 0; the original tests both cases before storing, so
     * the form is kept. */
    if (m_devilDoomParam->field_0x191[0] && !m_devilDoomParam->field_0x18F) {
        m_devilDoomParam->field_0x18F = true;
    } else if (!m_devilDoomParam->field_0x191[0] && m_devilDoomParam->field_0x18F) {
        m_devilDoomParam->field_0x18F = false;
    }
}

bool DevilDoomModeCtrlUnit_BELOW_HALF::Update(float arg) {
    if (m_devilDoomParam->field_0x634) {
        switch (field_0x10) {
        case 0:
            m_devilDoomParam->field_0x19A = 0;
            m_devilDoomParam->field_0x191[field_0xC] = 0;
            break;
        case 1:
            m_devilDoomParam->field_0x19A = 1;
            m_devilDoomParam->field_0x191[1 - field_0xC] = 1;
            m_devilDoomParam->field_0x190 = !m_devilDoomParam->field_0x190;
            break;
        }
        field_0x10++;
        m_devilDoomParam->field_0x634 = 0;
    }
    if (stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 3, 1) != 0) {
        m_devilDoomParam->field_0x14 = 2; /* mode 2: WAIT */
    }
    return false;
}

DevilDoomModeCtrlUnit_START::~DevilDoomModeCtrlUnit_START() {
    m_devilDoomParam->field_0x18 = 0;
    m_devilDoomParam->field_0x1A = 1;
    m_devilDoomParam->field_0x19A = 1;
    for (int i = 0; i < 3; i++) {
        m_devilDoomParam->AddCtrlUnit(new DevilDoomCtrlUnit_BitGroupControl(m_devilDoomParam, i));
    }
}

bool DevilDoomModeCtrlUnit_START::Update(float arg) {
    if (stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 3, 1) != 0) {
        m_devilDoomParam->field_0x14 = 2; /* mode 2: WAIT */
    }
    return false;
}

DevilDoomModeCtrlUnit_ATK_BIT3::~DevilDoomModeCtrlUnit_ATK_BIT3() {}

bool DevilDoomModeCtrlUnit_ATK_BIT3::Update(float arg) {
    m_devilDoomParam->field_0x14 = 2; /* mode 2: WAIT */
    return false;
}

DevilDoomModeCtrlUnit_ATK_BIT2::~DevilDoomModeCtrlUnit_ATK_BIT2() {}

bool DevilDoomModeCtrlUnit_ATK_BIT2::Update(float arg) {
    m_devilDoomParam->field_0x14 = 2; /* mode 2: WAIT */
    return false;
}

DevilDoomModeCtrlUnit_ATK_BIT1::~DevilDoomModeCtrlUnit_ATK_BIT1() {}

bool DevilDoomModeCtrlUnit_ATK_BIT1::Update(float arg) {
    m_devilDoomParam->field_0x14 = 2; /* mode 2: WAIT */
    return false;
}

DevilDoomModeCtrlUnit_ATK_PSYCHO_THROW::~DevilDoomModeCtrlUnit_ATK_PSYCHO_THROW() {
    if (field_0xC > 0.0f) {
        m_devilDoomParam->field_0x1A8 = field_0xC;
    }
}

bool DevilDoomModeCtrlUnit_ATK_PSYCHO_THROW::Update(float arg) {
    if (m_devilDoomParam->field_0x634) {
        m_devilDoomParam->field_0x198 = 1;
        m_devilDoomParam->field_0x634 = 0;
    }
    if (m_devilDoomParam->field_0x1A4 > 0.0f && m_devilDoomParam->field_0x196 && field_0xC < 0.0f) {
        MotionRange range;
        fn_80415D40(m_devilDoomParam->field_0x3C, 0, &range);
        if (range.currentFrame > range.endFrame + 60.0f * field_0xC) {
            m_devilDoomParam->field_0x14 = 8; /* mode 8: ATK_PSYCHO_THROW */
            return false;
        }
    }
    if (stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 3, 1) != 0) {
        m_devilDoomParam->field_0x14 = 2; /* mode 2: WAIT */
    }
    return false;
}

DevilDoomModeCtrlUnit_ATK_PSYCHO_PREPARE::~DevilDoomModeCtrlUnit_ATK_PSYCHO_PREPARE() {}

bool DevilDoomModeCtrlUnit_ATK_PSYCHO_PREPARE::Update(float arg) {
    u16 motionId = m_devilDoomParam->field_0x44 + 5;
    u16 current = fn_80415CDC(m_devilDoomParam->field_0x3C, 0);
    if (current == motionId) {
        if (m_devilDoomParam->field_0x634) {
            m_devilDoomParam->AddCtrlUnit(
                new DevilDoomCtrlUnit_PsychoObjectControl(m_devilDoomParam));
            m_devilDoomParam->field_0x634 = 0;
            fn_801FAF00(fn_80109D64(), 426, 0, 0, 0);
            fn_8034DE74(8, -2, 3.0f);
        }
        if (stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 3, 1) != 0) {
            fn_80417568(m_devilDoomParam->field_0x3C, m_devilDoomParam->field_0x44 + 6);
            if (m_devilDoomParam->field_0x18F) {
                stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 7, 0);
            }
            m_timer = 0.0f;
        }
    } else {
        motionId = m_devilDoomParam->field_0x44 + 6;
        current = fn_80415CDC(m_devilDoomParam->field_0x3C, 0);
        if (current == motionId) {
            m_timer += arg;
            if (m_timer > m_devilDoomParam->m_params.field_0x2C) {
                m_devilDoomParam->field_0x14 = 8; /* mode 8: ATK_PSYCHO_THROW */
            }
        } else {
            motionId = m_devilDoomParam->field_0x44 + 7;
            current = fn_80415CDC(m_devilDoomParam->field_0x3C, 0);
            if (current == motionId) {
                if (stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 3, 1) != 0) {
                    m_devilDoomParam->field_0x14 = 8; /* mode 8: ATK_PSYCHO_THROW */
                }
            }
        }
    }
    return false;
}

DevilDoomModeCtrlUnit_ATK_FIRE::~DevilDoomModeCtrlUnit_ATK_FIRE() {
    m_devilDoomParam->field_0x18 = 0;
    m_devilDoomParam->field_0x195 = 1;
    m_devilDoomParam->field_0x630 = m_devilDoomParam->m_params.field_0xF0;
}

bool DevilDoomModeCtrlUnit_ATK_FIRE::Update(float arg) {
    u16 motionId = m_devilDoomParam->field_0x44 + 1;
    u16 current = fn_80415CDC(m_devilDoomParam->field_0x3C, 0);
    if (current == motionId) {
        if (stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 3, 1) != 0) {
            fn_80417568(m_devilDoomParam->field_0x3C, m_devilDoomParam->field_0x44 + 2);
            if (m_devilDoomParam->field_0x18F) {
                stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 7, 0);
            }
            m_timer = 0.0f;
            m_devilDoomParam->field_0x18C = 1;

            /* The breath unit gets the translation of node 1018 (when field_0x18F is set) or
             * 1028 with y = 0, moved 30 units along the horizontal direction from the character's
             * position to it, and that direction scaled by m_params.field_0xEC (fn_8040CE74 makes
             * it the zero vector when it is too short to normalise). */
            MotionTransform *transform;
            if (m_devilDoomParam->field_0x18F) {
                transform = fn_80415E70(m_devilDoomParam->field_0x3C, 1018);
            } else {
                transform = fn_80415E70(m_devilDoomParam->field_0x3C, 1028);
            }
            Fn8040Vector3 translation;
            fn_804114D0(&translation, &transform->matrix);
            Fn8040Vector3 position;
            fn_80016350(&position, &translation);
            position.y = 0.0f;
            Fn8040Vector3 direction = position - m_devilDoomParam->m_charInfo->m_position;
            direction.y = 0.0f;
            fn_8040CE74(&direction);
            AddVector(&position, direction * 30.0f);
            fn_8040CE38(&direction, m_devilDoomParam->m_params.field_0xEC);
            m_devilDoomParam->AddCtrlUnit(
                new DevilDoomCtrlUnit_FireBreath(m_devilDoomParam, &position, &direction));
        }
    } else {
        motionId = m_devilDoomParam->field_0x44 + 2;
        current = fn_80415CDC(m_devilDoomParam->field_0x3C, 0);
        if (current == motionId) {
            m_timer += arg;
            if (m_timer > 800.0f / m_devilDoomParam->m_params.field_0xEC) {
                stMtnPrepareBlendWithTarget(m_devilDoomParam->field_0x3C, 0, -1, -1);
                fn_80417568(m_devilDoomParam->field_0x3C, m_devilDoomParam->field_0x44 + 3);
                stMtnRequestBlendFrame(m_devilDoomParam->field_0x3C, 0, 30.0f);
                if (m_devilDoomParam->field_0x18F) {
                    stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 7, 0);
                }
            }
        } else {
            motionId = m_devilDoomParam->field_0x44 + 3;
            current = fn_80415CDC(m_devilDoomParam->field_0x3C, 0);
            if (current == motionId) {
                if (stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 3, 1) != 0) {
                    m_devilDoomParam->field_0x14 = 2; /* mode 2: WAIT */
                }
            }
        }
    }
    return false;
}

DevilDoomModeCtrlUnit_CHAOS_CONTROL::~DevilDoomModeCtrlUnit_CHAOS_CONTROL() {
    m_devilDoomParam->field_0x18 = 0;
    m_devilDoomParam->field_0x1B = 0;
    m_devilDoomParam->field_0x19A = 1;
    fn_801FAF00(fn_80109D64(), 430, 0, 0, 0);
}

bool DevilDoomModeCtrlUnit_CHAOS_CONTROL::Update(float arg) {
    float previousTimer = m_timer;
    m_timer += arg;
    if (m_timer > m_devilDoomParam->m_params.field_0x20) {
        /* When field_0x1D4 with y = 0 is longer than 0.0001, the character's position becomes
         * that vector scaled to length m_params.field_0x4, negated when field_0x1F4 is below
         * m_params.field_0x4. */
        Fn8040Vector3 position(m_devilDoomParam->field_0x1D4);
        position.y = 0.0f;
        if (fn_8040CF50(position) > 0.0001) {
            position = fn_8040CEE8(position) * m_devilDoomParam->m_params.field_0x4;
            if (m_devilDoomParam->field_0x1F4 < m_devilDoomParam->m_params.field_0x4) {
                position = fn_80016284(position);
            }
            m_devilDoomParam->m_charInfo->m_position = position;
        }

        Fn8040Vector3 offset =
            m_devilDoomParam->field_0x1D4 - m_devilDoomParam->m_charInfo->m_position;
        m_devilDoomParam->field_0x1F0 = fn_80016260(-offset.x, -offset.z);
        m_devilDoomParam->field_0x1EC = m_devilDoomParam->field_0x1F0;
        if (m_devilDoomParam->field_0x18F) {
            m_devilDoomParam->m_charInfo->m_rotation.y =
                m_devilDoomParam->field_0x1EC - 0.5f * lbl_805FAD38;
        } else {
            m_devilDoomParam->m_charInfo->m_rotation.y =
                m_devilDoomParam->field_0x1EC + 0.5f * lbl_805FAD38;
        }
        m_devilDoomParam->field_0x1F8 = 0.0f;
        offset.y = 0.0f;
        m_devilDoomParam->field_0x1F4 = fn_8040CF50(offset);
        if (field_0x10) {
            m_devilDoomParam->field_0x14 = 7; /* mode 7: ATK_PSYCHO_PREPARE */
        } else {
            m_devilDoomParam->field_0x14 = 2; /* mode 2: WAIT */
        }
        if (previousTimer <= m_devilDoomParam->m_params.field_0x20) {
            fn_801D560C(fn_80053164(), 0x8135, 0, 0);
        }
    }
    return false;
}

DevilDoomModeCtrlUnit_INVERSE::~DevilDoomModeCtrlUnit_INVERSE() {}

bool DevilDoomModeCtrlUnit_INVERSE::Update(float arg) {
    if (m_devilDoomParam->field_0x634) {
        /* Toggles field_0x18F, adds pi to field_0x1EC and sets field_0x1F8 to field_0x1F0 -
         * field_0x1EC; while the absolute value of either angle is greater than pi, it adds
         * -2 pi to it when it is greater than 0, otherwise 2 pi. */
        m_devilDoomParam->field_0x18F = !m_devilDoomParam->field_0x18F;
        m_devilDoomParam->field_0x1EC += lbl_805FAD38;
        while (fn_8000A980(m_devilDoomParam->field_0x1EC) > lbl_805FAD38) {
            m_devilDoomParam->field_0x1EC +=
                m_devilDoomParam->field_0x1EC > 0.0f ? -2.0f * lbl_805FAD38 : 2.0f * lbl_805FAD38;
        }
        m_devilDoomParam->field_0x1F8 =
            m_devilDoomParam->field_0x1F0 - m_devilDoomParam->field_0x1EC;
        while (fn_8000A980(m_devilDoomParam->field_0x1F8) > lbl_805FAD38) {
            m_devilDoomParam->field_0x1F8 +=
                m_devilDoomParam->field_0x1F8 > 0.0f ? -2.0f * lbl_805FAD38 : 2.0f * lbl_805FAD38;
        }
        m_devilDoomParam->field_0x634 = 0;
    }
    if (stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 3, 1) != 0) {
        m_devilDoomParam->field_0x14 = 2; /* mode 2: WAIT */
    }
    return false;
}

DevilDoomModeCtrlUnit_GET_ATK::~DevilDoomModeCtrlUnit_GET_ATK() {
    m_devilDoomParam->field_0x18 = 0;
    m_devilDoomParam->field_0x194 = 1;
    m_devilDoomParam->field_0x195 = 1;
    /* The original tests each of the next two bytes and stores only when the store changes it;
     * the form is kept. */
    if (!m_devilDoomParam->field_0x1A) {
        m_devilDoomParam->field_0x1A = 1;
    }
    if (m_devilDoomParam->field_0x191[field_0xC]) {
        m_devilDoomParam->field_0x191[field_0xC] = 0;
    }
    if (!m_devilDoomParam->field_0x191[1 - field_0xC]) {
        m_devilDoomParam->field_0x191[1 - field_0xC] = 1;
        m_devilDoomParam->field_0x190 = !m_devilDoomParam->field_0x190;
        m_devilDoomParam->field_0x19A = 1;
    }
    /* Sets field_0x18F to field_0x191[0] != 0; the original tests both cases before storing, so
     * the form is kept. */
    if (m_devilDoomParam->field_0x191[0] && !m_devilDoomParam->field_0x18F) {
        m_devilDoomParam->field_0x18F = true;
    } else if (!m_devilDoomParam->field_0x191[0] && m_devilDoomParam->field_0x18F) {
        m_devilDoomParam->field_0x18F = false;
    }
}

bool DevilDoomModeCtrlUnit_GET_ATK::Update(float arg) {
    if (m_devilDoomParam->field_0x634) {
        switch (field_0x10) {
        case 0:
            m_devilDoomParam->field_0x19A = 0;
            m_devilDoomParam->field_0x191[field_0xC] = 0;
            break;
        case 1:
            m_devilDoomParam->field_0x19A = 1;
            m_devilDoomParam->field_0x191[1 - field_0xC] = 1;
            m_devilDoomParam->field_0x190 = !m_devilDoomParam->field_0x190;
            break;
        }
        field_0x10++;
        m_devilDoomParam->field_0x634 = 0;
    }
    if (stMtnControlWorkFlag(m_devilDoomParam->field_0x3C, 0, 3, 1) != 0) {
        m_devilDoomParam->field_0x14 = 2; /* mode 2: WAIT */
    }
    return false;
}

DevilDoomModeCtrlUnit_EDIT::~DevilDoomModeCtrlUnit_EDIT() {}

bool DevilDoomModeCtrlUnit_EDIT::Update(float arg) {
    return false;
}

DevilDoomModeCtrlUnit_WAIT::~DevilDoomModeCtrlUnit_WAIT() {}

bool DevilDoomModeCtrlUnit_WAIT::Update(float arg) {
    /* The ATK_FIRE test is written out in both cases below: an inline function holding it stays
     * an out-of-line call (it calls the inline fn_8000A980, see fn_8000A950_near_zero.h), and the
     * original makes no call there. */
    float limit = m_devilDoomParam->m_params.field_0x10 * (lbl_805FAD38 / 180.0f);
    if (fn_8000A980(m_devilDoomParam->field_0x1F8) > limit) {
        m_devilDoomParam->field_0x14 = 4; /* mode 4: INVERSE */
        return false;
    }

    switch (m_devilDoomParam->field_0x19C) {
    case 1:
        m_devilDoomParam->field_0x18C = 0;
        if (!m_devilDoomParam->field_0x196) {
            m_devilDoomParam->field_0x19C = 0;
        }
        if (m_devilDoomParam->field_0x1A4 < 0.0f) {
            m_devilDoomParam->field_0x19C = 0;
        }
        if (m_devilDoomParam->field_0x1A8 < 0.0f) {
            m_devilDoomParam->field_0x14 = 8; /* mode 8: ATK_PSYCHO_THROW */
        }
        break;
    case 2:
        m_devilDoomParam->field_0x18C = 1;
        if (m_devilDoomParam->field_0x630 < 0.0f &&
            fn_8000A980(m_devilDoomParam->field_0x1F8) <
                m_devilDoomParam->m_params.field_0xF8 * (lbl_805FAD38 / 180.0f) &&
            m_devilDoomParam->field_0x1F4 < m_devilDoomParam->m_params.field_0xF4) {
            m_devilDoomParam->field_0x14 = 6; /* mode 6: ATK_FIRE */
            return false;
        }
        break;
    default:
        if (m_devilDoomParam->field_0x630 < 0.0f &&
            fn_8000A980(m_devilDoomParam->field_0x1F8) <
                m_devilDoomParam->m_params.field_0xF8 * (lbl_805FAD38 / 180.0f) &&
            m_devilDoomParam->field_0x1F4 < m_devilDoomParam->m_params.field_0xF4) {
            m_devilDoomParam->field_0x14 = 6; /* mode 6: ATK_FIRE */
            return false;
        }

        m_devilDoomParam->field_0x1A4 = 0.0f;
        float ratio = m_devilDoomParam->field_0x24 / m_devilDoomParam->field_0x8;
        switch (m_devilDoomParam->field_0x1A0) {
        case 1:
            if (ratio > 0.75f) {
                m_devilDoomParam->field_0x14 = 9; /* mode 9: ATK_BIT1 */
            } else if (ratio > 0.5f) {
                m_devilDoomParam->field_0x14 = 10; /* mode 10: ATK_BIT2 */
            } else if (ratio > 0.25f) {
                switch (static_cast<int>(2.0f * fn_8040EE90())) {
                case 0:
                    m_devilDoomParam->field_0x14 = 10; /* mode 10: ATK_BIT2 */
                    break;
                case 1:
                    m_devilDoomParam->field_0x14 = 11; /* mode 11: ATK_BIT3 */
                    break;
                }
            } else {
                switch (static_cast<int>(3.0f * fn_8040EE90())) {
                case 0:
                    m_devilDoomParam->field_0x14 = 9; /* mode 9: ATK_BIT1 */
                    break;
                case 1:
                    m_devilDoomParam->field_0x14 = 10; /* mode 10: ATK_BIT2 */
                    break;
                case 2:
                    m_devilDoomParam->field_0x14 = 11; /* mode 11: ATK_BIT3 */
                    break;
                }
            }
            break;
        case 2:
            if (!fn_802039E8(&lbl_8057D6A0.field_0xA8)) {
                m_devilDoomParam->field_0x14 = 7; /* mode 7: ATK_PSYCHO_PREPARE */
            }
            break;
        default:
            if (fn_8040EE90() < 0.5f) {
                m_devilDoomParam->field_0x1A0 = 1;
            } else {
                m_devilDoomParam->field_0x1A0 = 2;
            }
            break;
        }
        break;
    }
    return false;
}

/* Modes 1 and 3 to 14 select a unit; any other value, including 0 and 2, gives WAIT. */
BossCtrlUnit *DevilDoomModeCtrlUnit::CreateUnit(int mode) {
    switch (mode) {
    default:
        return new DevilDoomModeCtrlUnit_WAIT(m_devilDoomParam);
    case 1:
        return new DevilDoomModeCtrlUnit_EDIT(m_devilDoomParam);
    case 3:
        return new DevilDoomModeCtrlUnit_GET_ATK(m_devilDoomParam);
    case 4:
        return new DevilDoomModeCtrlUnit_INVERSE(m_devilDoomParam);
    case 5:
        return new DevilDoomModeCtrlUnit_CHAOS_CONTROL(m_devilDoomParam);
    case 6:
        return new DevilDoomModeCtrlUnit_ATK_FIRE(m_devilDoomParam);
    case 7:
        return new DevilDoomModeCtrlUnit_ATK_PSYCHO_PREPARE(m_devilDoomParam);
    case 8:
        return new DevilDoomModeCtrlUnit_ATK_PSYCHO_THROW(m_devilDoomParam);
    case 9:
        return new DevilDoomModeCtrlUnit_ATK_BIT1(m_devilDoomParam);
    case 10:
        return new DevilDoomModeCtrlUnit_ATK_BIT2(m_devilDoomParam);
    case 11:
        return new DevilDoomModeCtrlUnit_ATK_BIT3(m_devilDoomParam);
    case 12:
        return new DevilDoomModeCtrlUnit_START(m_devilDoomParam);
    case 13:
        return new DevilDoomModeCtrlUnit_BELOW_HALF(m_devilDoomParam);
    case 14:
        return new DevilDoomModeCtrlUnit_DEATH(m_devilDoomParam);
    }
}

DevilDoomModeCtrlUnit::~DevilDoomModeCtrlUnit() {}

bool DevilDoomModeCtrlUnit::Update(float arg) {
    return BossModeCtrlUnit::Update(arg);
}
