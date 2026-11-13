/* HeavyDog0410ModeCtrlUnit (class name from RTTI): the mode control unit of the HEAVY_DOG_0410
 * boss, and the units it creates for each mode, HeavyDog0410ModeCtrlUnit_WAIT to _MOVETEST (see
 * HeavyDog0410ModeCtrlUnit.h). File name inferred from the classes. Definitions are in reverse
 * address order (deferred inlining emits functions last-defined first). */
#include "Game/HeavyDog0410ModeCtrlUnit.h"
#include "Game/BossCtrlUnit.h"
#include "Game/DebugMissionClearCollision.h"
#include "Game/EffectElement.h"
#include "Game/HeavyDog0410CtrlUnit.h"
#include "Game/HeavyDogVarParam.h"
#include "Game/SharedConstants.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8040_vector_types.h"
#include "Game/fn_804157E8_motion_control.h"
#include "Game/fn_804157E8_motion_range.h"
#include "boost/shared_ptr.hpp"

using Effect::System::EffectElement;

HeavyDog0410ModeCtrlUnit_MOVETEST::~HeavyDog0410ModeCtrlUnit_MOVETEST() {}

bool HeavyDog0410ModeCtrlUnit_MOVETEST::Update(float arg) {
    m_heavyDog0410Param->field_0xF8 = m_heavyDog0410Param->field_0x17C;
    m_heavyDog0410Param->field_0xF8.y = 0.0f;
    m_heavyDog0410Param->field_0x14 = 2; /* mode 2: WAIT */
    return false;
}

HeavyDog0410ModeCtrlUnit_ATK_MINE::~HeavyDog0410ModeCtrlUnit_ATK_MINE() {}

bool HeavyDog0410ModeCtrlUnit_ATK_MINE::Update(float arg) {
    m_heavyDog0410Param->field_0x14 = 2; /* mode 2: WAIT */
    return false;
}

HeavyDog0410ModeCtrlUnit_ATK_BARRIER::~HeavyDog0410ModeCtrlUnit_ATK_BARRIER() {
    m_heavyDog0410Param->field_0x1C0 = 0.0f;
}

bool HeavyDog0410ModeCtrlUnit_ATK_BARRIER::Update(float arg) {
    m_heavyDog0410Param->field_0xF8 = m_heavyDog0410Param->m_charInfo->m_position;
    m_timer += arg;

    switch (m_state) {
    case 0:
        if (m_timer > 1.5f && fn_8040CF50(m_heavyDog0410Param->field_0x11C) <
                                  m_heavyDog0410Param->m_params.field_0x14) {
            Fn8040Vector3 position(m_heavyDog0410Param->m_charInfo->m_position);
            if (m_heavyDog0410Param->field_0x194.y > 40.0f) {
                position.y = 0.5f * m_heavyDog0410Param->m_params.field_0x60 + 40.0f;
            } else {
                position.y = 0.5f * m_heavyDog0410Param->m_params.field_0x60;
            }
            m_heavyDog0410Param->AddCtrlUnit(
                new HeavyDogCtrlUnit_Barrier(m_heavyDog0410Param, &position, true));
            m_heavyDog0410Param->field_0x1C0 = -1.0f;
            m_state = 1;
            m_timer = 0.0f;
        }
        break;
    case 1:
        m_heavyDog0410Param->field_0x118 = 0.0f;
        if (m_timer > m_heavyDog0410Param->m_params.field_0x64) {
            fn_801FAF00(fn_80109D64(), 227, 0, 0, 0);
            m_heavyDog0410Param->field_0x14 = 2; /* mode 2: WAIT */
        }
        break;
    }
    return false;
}

HeavyDog0410ModeCtrlUnit_ATK_ROCKET::~HeavyDog0410ModeCtrlUnit_ATK_ROCKET() {}

bool HeavyDog0410ModeCtrlUnit_ATK_ROCKET::Update(float arg) {
    m_heavyDog0410Param->field_0x14 = 2; /* mode 2: WAIT */
    return false;
}

HeavyDog0410ModeCtrlUnit_DESTROY::~HeavyDog0410ModeCtrlUnit_DESTROY() {
    boost::shared_ptr<EffectElement> effect = field_0x10.lock();
    if (effect) {
        effect->RequestStop();
        effect.reset();
    }
}

bool HeavyDog0410ModeCtrlUnit_DESTROY::Update(float arg) {
    if (field_0x18 == 0) {
        MotionRange range;
        fn_80415D40(m_heavyDog0410Param->field_0x3C, 0, &range);
        if (range.currentFrame > range.endFrame - 60.0f) {
            fn_80175EEC(fn_80074470(), 6, false);
            field_0x18 = 1;
        }
    }
    return false;
}

HeavyDog0410ModeCtrlUnit_BELOW_HALF::~HeavyDog0410ModeCtrlUnit_BELOW_HALF() {
    fn_80417568(m_heavyDog0410Param->field_0x3C, m_heavyDog0410Param->field_0x44 + 1);
    m_heavyDog0410Param->field_0x18 = 0;
    m_heavyDog0410Param->field_0x1A = 1;
    m_heavyDog0410Param->field_0x1C4 *= 0.5f;
    m_heavyDog0410Param->field_0x1C0 = m_heavyDog0410Param->field_0x1C4;
}

bool HeavyDog0410ModeCtrlUnit_BELOW_HALF::Update(float arg) {
    if (stMtnControlWorkFlag(m_heavyDog0410Param->field_0x3C, 0, 3, 1) != 0) {
        m_heavyDog0410Param->field_0x14 = 2; /* mode 2: WAIT */
    }
    return false;
}

HeavyDog0410ModeCtrlUnit_START::~HeavyDog0410ModeCtrlUnit_START() {
    fn_80417568(m_heavyDog0410Param->field_0x3C, m_heavyDog0410Param->field_0x44 + 1);
    m_heavyDog0410Param->field_0x18 = 0;
    m_heavyDog0410Param->field_0x1A = 1;
}

bool HeavyDog0410ModeCtrlUnit_START::Update(float arg) {
    if (stMtnControlWorkFlag(m_heavyDog0410Param->field_0x3C, 0, 3, 1) != 0) {
        m_heavyDog0410Param->field_0x14 = 2; /* mode 2: WAIT */
    }
    return false;
}

HeavyDog0410ModeCtrlUnit_GET_ATK::~HeavyDog0410ModeCtrlUnit_GET_ATK() {
    m_heavyDog0410Param->field_0x18 = 0;
}

bool HeavyDog0410ModeCtrlUnit_GET_ATK::Update(float arg) {
    if (m_heavyDog0410Param->field_0x18 != 0) {
        field_0x10 += lbl_8057D6A0.field_0x18;
        if (field_0x10 > m_heavyDog0410Param->m_params.field_0x24) {
            m_heavyDog0410Param->field_0x14 = 2; /* mode 2: WAIT */
        }
    }
    return false;
}

HeavyDog0410ModeCtrlUnit_ESCAPE::~HeavyDog0410ModeCtrlUnit_ESCAPE() {}

bool HeavyDog0410ModeCtrlUnit_ESCAPE::Update(float arg) {
    Fn8040Vector3 corners[4] = {
        Fn8040Vector3(300.0f, 0.0f, 300.0f),
        Fn8040Vector3(300.0f, 0.0f, -300.0f),
        Fn8040Vector3(-300.0f, 0.0f, -300.0f),
        Fn8040Vector3(-300.0f, 0.0f, 300.0f),
    };
    Fn8040Vector3 target;
    Fn8040Vector3 offset;
    Fn8040Vector3 cornerOffset;
    int chosen = -1;
    int nearest = -1;
    float bestDot;
    float nearestDistance = 10000.0f;
    int i;

    for (i = 0; i < 4; i++) {
        offset = corners[i] - m_heavyDog0410Param->field_0x194;
        offset.y = 0.0f;
        float distance = fn_8040CF50(offset);
        if (distance < nearestDistance) {
            nearest = i;
            nearestDistance = distance;
        }
    }

    bestDot = -1.0f;
    for (i = 0; i < 4; i++) {
        if (i != nearest) {
            offset = m_heavyDog0410Param->m_charInfo->m_position - m_heavyDog0410Param->field_0x194;
            offset.y = 0.0f;
            fn_8040CE74(&offset);
            cornerOffset = corners[i] - m_heavyDog0410Param->field_0x194;
            cornerOffset.y = 0.0f;
            fn_8040CE74(&cornerOffset);
            float dot = fn_8040CF90(offset, cornerOffset);
            if (dot > bestDot) {
                chosen = i;
                bestDot = dot;
            }
        }
    }

    if (chosen != -1) {
        target = corners[chosen];
        offset = target - m_heavyDog0410Param->m_charInfo->m_position;
        offset.y = 0.0f;
        float angle = fn_80016260(-offset.x, -offset.z);

        /* From here to the store in field_0xF8, the steps of CHASE's Update with another angle and
         * distance. They are written out in both places, as is the creation of the mine below
         * (the steps of ATK_MINE's constructor): an inline function holding either block, which
         * uses the inline vector operators, stays an out-of-line call under the one-level
         * inlining this code is built with, and the original makes no call there. */
        Fn8040Vector3 direction(lbl_805E07A0);
        if (angle > 0.5f * lbl_805FAD38) {
            direction.x = -1.0f;
            direction.z = 1.0f;
        } else if (angle > 0.0f) {
            direction.x = -1.0f;
            direction.z = -1.0f;
        } else if (angle > 0.5f * -lbl_805FAD38) {
            direction.x = 1.0f;
            direction.z = -1.0f;
        } else {
            direction.x = 1.0f;
            direction.z = 1.0f;
        }
        fn_8040CE38(&direction, m_heavyDog0410Param->m_params.field_0x6C);
        m_heavyDog0410Param->field_0xF8 = m_heavyDog0410Param->m_charInfo->m_position + direction;
    }

    if (m_heavyDog0410Param->field_0x1C0 >= m_heavyDog0410Param->field_0x1C4) {
        m_heavyDog0410Param->field_0x14 = 2; /* mode 2: WAIT */
        fn_801FAF00(fn_80109D64(), 228, 0, 0, 0);
    }

    if (m_heavyDog0410Param->field_0x1B0 < m_heavyDog0410Param->m_params.field_0x5C) {
        if (m_heavyDog0410Param->field_0x118 > m_heavyDog0410Param->field_0x1D8) {
            Fn8040Matrix34 matrix;
            MotionTransform *transform = fn_80415E70(m_heavyDog0410Param->field_0x3C, 4500);
            fn_80016370(&matrix, &transform->matrix);
            Fn8040Vector3 nodePosition4500 = fn_8040FB08(&matrix);
            transform = fn_80415E70(m_heavyDog0410Param->field_0x3C, 4010);
            fn_80016370(&matrix, &transform->matrix);
            Fn8040Vector3 nodePosition4010 = fn_8040FB08(&matrix);
            Fn8040Vector3 minePoint = m_heavyDog0410Param->field_0x194 - nodePosition4010;
            minePoint =
                fn_8040CEE8(minePoint) * m_heavyDog0410Param->field_0x1DC + nodePosition4010;
            Fn8040Vector3 mineVector = m_heavyDog0410Param->field_0x194 - nodePosition4010;
            mineVector.y = 0.0f;
            mineVector = fn_8040CEE8(mineVector) * m_heavyDog0410Param->field_0x1E0;
            mineVector.y = m_heavyDog0410Param->field_0x1E4;
            fn_8040CFF8(&mineVector, &m_heavyDog0410Param->field_0x11C);
            m_heavyDog0410Param->AddCtrlUnit(new HeavyDog0410CtrlUnit_Mine(
                m_heavyDog0410Param, &nodePosition4500, &mineVector, &minePoint));
            m_heavyDog0410Param->field_0x118 = 0.0f;
        }
    } else {
        m_heavyDog0410Param->field_0x144[m_heavyDog0410Param->field_0x138].field_0x2 = 1;
    }
    return false;
}

HeavyDog0410ModeCtrlUnit_CHASE::~HeavyDog0410ModeCtrlUnit_CHASE() {}

bool HeavyDog0410ModeCtrlUnit_CHASE::Update(float arg) {
    m_timer += arg;

    Fn8040Vector3 direction(lbl_805E07A0);
    if (m_heavyDog0410Param->field_0x1BC > 0.5f * lbl_805FAD38) {
        direction.x = -1.0f;
        direction.z = 1.0f;
    } else if (m_heavyDog0410Param->field_0x1BC > 0.0f) {
        direction.x = -1.0f;
        direction.z = -1.0f;
    } else if (m_heavyDog0410Param->field_0x1BC > 0.5f * -lbl_805FAD38) {
        direction.x = 1.0f;
        direction.z = -1.0f;
    } else {
        direction.x = 1.0f;
        direction.z = 1.0f;
    }
    fn_8040CE38(&direction, m_heavyDog0410Param->m_params.field_0x68);
    m_heavyDog0410Param->field_0xF8 = m_heavyDog0410Param->m_charInfo->m_position + direction;

    if (m_timer > 1.5f &&
        m_heavyDog0410Param->field_0x1B0 < m_heavyDog0410Param->m_params.field_0x5C - 30.0f) {
        m_heavyDog0410Param->field_0x14 = 10; /* mode 10: ATK_BARRIER */
    }
    m_heavyDog0410Param->field_0x144[m_heavyDog0410Param->field_0x138].field_0x2 = 1;
    return false;
}

HeavyDog0410ModeCtrlUnit_EDIT::~HeavyDog0410ModeCtrlUnit_EDIT() {
    m_heavyDog0410Param->field_0x114 = 0;
    m_heavyDog0410Param->field_0x115 = 0;
}

bool HeavyDog0410ModeCtrlUnit_EDIT::Update(float arg) {
    return false;
}

HeavyDog0410ModeCtrlUnit_WAIT::~HeavyDog0410ModeCtrlUnit_WAIT() {}

bool HeavyDog0410ModeCtrlUnit_WAIT::Update(float arg) {
    if (m_heavyDog0410Param->field_0x1C0 >= m_heavyDog0410Param->field_0x1C4) {
        m_heavyDog0410Param->field_0x14 = 3; /* mode 3: CHASE */
    } else {
        m_heavyDog0410Param->field_0x14 = 4; /* mode 4: ESCAPE */
    }
    fn_801D54F4(fn_80053164(), 0x7D3F, &m_heavyDog0410Param->m_charInfo->m_position, 0, 1);
    return false;
}

/* Modes 1 and 3 to 12 select a unit; any other value, including 0 and 2, gives WAIT. */
BossCtrlUnit *HeavyDog0410ModeCtrlUnit::CreateUnit(int mode) {
    switch (mode) {
    default:
        return new HeavyDog0410ModeCtrlUnit_WAIT(m_heavyDog0410Param);
    case 1:
        return new HeavyDog0410ModeCtrlUnit_EDIT(m_heavyDog0410Param);
    case 3:
        return new HeavyDog0410ModeCtrlUnit_CHASE(m_heavyDog0410Param);
    case 4:
        return new HeavyDog0410ModeCtrlUnit_ESCAPE(m_heavyDog0410Param);
    case 5:
        return new HeavyDog0410ModeCtrlUnit_GET_ATK(m_heavyDog0410Param);
    case 6:
        return new HeavyDog0410ModeCtrlUnit_START(m_heavyDog0410Param);
    case 7:
        return new HeavyDog0410ModeCtrlUnit_BELOW_HALF(m_heavyDog0410Param);
    case 8:
        return new HeavyDog0410ModeCtrlUnit_DESTROY(m_heavyDog0410Param);
    case 9:
        return new HeavyDog0410ModeCtrlUnit_ATK_ROCKET(m_heavyDog0410Param);
    case 10:
        return new HeavyDog0410ModeCtrlUnit_ATK_BARRIER(m_heavyDog0410Param);
    case 11:
        return new HeavyDog0410ModeCtrlUnit_ATK_MINE(m_heavyDog0410Param);
    case 12:
        return new HeavyDog0410ModeCtrlUnit_MOVETEST(m_heavyDog0410Param);
    }
}

HeavyDog0410ModeCtrlUnit::~HeavyDog0410ModeCtrlUnit() {}

bool HeavyDog0410ModeCtrlUnit::Update(float arg) {
    return BossModeCtrlUnit::Update(arg);
}
