/* The control units of the HEAVY_DOG_0510 boss other than its mode units (class names from RTTI):
 * HeavyDog0510CtrlUnit_Move, HeavyDog0510CtrlUnit_Bomb and
 * HeavyDog0510CtrlUnit_FixLookCharCamera. File name inferred from the classes. Definitions are
 * in reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/BossCtrlUnit.h"
#include "Game/HeavyDog0510.h"
#include "Game/HeavyDog0510CtrlUnit.h"
#include "Game/HeavyDog0510VarParam.h"
#include "Game/HeavyDogCtrlUnit.h"
#include "Game/SharedConstants.h"
#include "Game/ThrowBomb.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_80209B58.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <cmath>

HeavyDog0510CtrlUnit_Move::HeavyDog0510CtrlUnit_Move(HeavyDog0510VarParam *param)
    : BossCtrlUnit(param), HeavyDog0510CtrlUnit(param) {
    field_0x4 = 2;
}

HeavyDog0510CtrlUnit_Move::~HeavyDog0510CtrlUnit_Move() {}

/* While HeavyDogVarParam::field_0x114 is zero, moves the boss for the step arg (behaviour from
 * the body; the meanings of the values are inferred):
 * - The vertical velocity field_0x11C.y gains (field_0xF8.y - height) * m_params.field_0x0 -
 *   velocity * m_params.field_0x4 per unit time, and the height follows it.
 * - field_0x1F8, set to the horizontal distance of the boss from the origin, takes the value of
 *   field_0x1F4 when the magnitude of their difference is below the step arg * field_0x1D8, and
 *   otherwise moves by the step towards field_0x1F4. The step's sign is not checked: a negative
 *   step moves it away from field_0x1F4.
 * - The angular velocity field_0x1FC gains angle * m_params.field_0x8 - field_0x1FC *
 *   m_params.field_0xC per unit time, where angle is field_0x104 - field_0x110 brought to a
 *   magnitude of at most pi by turns of 2 * pi; field_0x110 follows it and is wrapped the same
 *   way.
 * - The boss is placed at the distance field_0x1F8 and the angle field_0x110 around the origin,
 *   and the horizontal velocity is set to the angular part of that motion only (field_0x1FC
 *   times the tangent at the distance field_0x1F8); the change of field_0x1F8 is not included.
 * - field_0x128 gets the change of the velocity divided by arg, and field_0x118 grows by the
 *   length of the velocity scaled by arg (a length, so it grows even for a negative arg).
 * - While the length of the velocity is greater than m_params.field_0x14, sound 0x7D43 is passed
 *   to fn_801D53C4 at the position of the boss with this unit as its owner.
 * While field_0x114 is not zero, field_0x11C and field_0x128 are set to the zero vector instead.
 * Either way field_0x108 gets the magnitude of field_0xF8.y minus the height, and the result is
 * false. */
bool HeavyDog0510CtrlUnit_Move::Update(float arg) {
    if (!m_heavyDog0510Param->field_0x114) {
        Fn8040Vector3 previousVelocity(m_heavyDog0510Param->field_0x11C);

        /* The acceleration takes two statements here and for the angle below: the original
         * subtracts the damping product from the finished first product (fnmsubs), where a
         * single expression computes the damping product first and folds the other product
         * into the subtraction (fmsubs). */
        float heightDifference =
            m_heavyDog0510Param->field_0xF8.y - m_heavyDog0510Param->m_charInfo->m_position.y;
        float acceleration = heightDifference * m_heavyDog0510Param->m_params.field_0x0;
        acceleration -=
            m_heavyDog0510Param->field_0x11C.y * m_heavyDog0510Param->m_params.field_0x4;
        m_heavyDog0510Param->field_0x11C.y += acceleration * arg;
        m_heavyDog0510Param->m_charInfo->m_position.y += m_heavyDog0510Param->field_0x11C.y * arg;

        Fn8040Vector3 horizontal(m_heavyDog0510Param->m_charInfo->m_position);
        horizontal.y = 0.0f;
        m_heavyDog0510Param->field_0x1F8 = fn_8040CF50(horizontal);

        float difference = m_heavyDog0510Param->field_0x1F4 - m_heavyDog0510Param->field_0x1F8;
        float step = arg * m_heavyDog0510Param->field_0x1D8;
        if (fn_8000A980(difference) < step) {
            m_heavyDog0510Param->field_0x1F8 = m_heavyDog0510Param->field_0x1F4;
        } else {
            /* The original compares with a double zero here (lfd from an eight-byte constant);
             * 0.0f changes the code and the constants. */
            m_heavyDog0510Param->field_0x1F8 += step * (difference > 0.0 ? 1.0f : -1.0f);
        }

        float angle = m_heavyDog0510Param->field_0x104 - m_heavyDog0510Param->field_0x110;
        while (fn_8000A980(angle) > lbl_805FAD38) {
            angle += angle > 0.0f ? -2.0f * lbl_805FAD38 : 2.0f * lbl_805FAD38;
        }
        float angularAcceleration = angle * m_heavyDog0510Param->m_params.field_0x8;
        angularAcceleration -=
            m_heavyDog0510Param->field_0x1FC * m_heavyDog0510Param->m_params.field_0xC;
        m_heavyDog0510Param->field_0x1FC += angularAcceleration * arg;
        m_heavyDog0510Param->field_0x110 += m_heavyDog0510Param->field_0x1FC * arg;
        while (fn_8000A980(m_heavyDog0510Param->field_0x110) > lbl_805FAD38) {
            m_heavyDog0510Param->field_0x110 += m_heavyDog0510Param->field_0x110 > 0.0f
                                                    ? -2.0f * lbl_805FAD38
                                                    : 2.0f * lbl_805FAD38;
        }

        m_heavyDog0510Param->m_charInfo->m_position.x =
            -m_heavyDog0510Param->field_0x1F8 * std::sin(m_heavyDog0510Param->field_0x110);
        m_heavyDog0510Param->m_charInfo->m_position.z =
            -m_heavyDog0510Param->field_0x1F8 * std::cos(m_heavyDog0510Param->field_0x110);
        m_heavyDog0510Param->field_0x11C.x = -m_heavyDog0510Param->field_0x1F8 *
                                             m_heavyDog0510Param->field_0x1FC *
                                             std::cos(m_heavyDog0510Param->field_0x110);
        m_heavyDog0510Param->field_0x11C.z = m_heavyDog0510Param->field_0x1F8 *
                                             m_heavyDog0510Param->field_0x1FC *
                                             std::sin(m_heavyDog0510Param->field_0x110);

        m_heavyDog0510Param->field_0x128 =
            (m_heavyDog0510Param->field_0x11C - previousVelocity) * (1.0f / arg);
        m_heavyDog0510Param->field_0x118 += fn_8040CF50(m_heavyDog0510Param->field_0x11C * arg);

        if (fn_8040CF50(m_heavyDog0510Param->field_0x11C) >
            m_heavyDog0510Param->m_params.field_0x14) {
            fn_801D53C4(fn_80053164(), 0x7D43, &m_heavyDog0510Param->m_charInfo->m_position, this,
                        3, 0, 3);
        }
    } else {
        m_heavyDog0510Param->field_0x11C = lbl_805E07A0;
        m_heavyDog0510Param->field_0x128 = lbl_805E07A0;
    }

    m_heavyDog0510Param->field_0x108 = fn_8000A980(m_heavyDog0510Param->field_0xF8.y -
                                                   m_heavyDog0510Param->m_charInfo->m_position.y);
    return false;
}

HeavyDog0510CtrlUnit_Bomb::HeavyDog0510CtrlUnit_Bomb(HeavyDog0510VarParam *param,
                                                     const Fn8040Vector3 *position,
                                                     const Fn8040Vector3 *velocity)
    : BossCtrlUnit(param), HeavyDog0510CtrlUnit(param) {
    field_0x4 = 2;
    m_bomb = fn_80209908(m_heavyDog0510Param->field_0x200, 0);

    boost::shared_ptr<ThrowBomb> bomb(m_bomb.lock());
    if (bomb) {
        UnkParams_80145B58 params(*position, *velocity, m_heavyDog0510Param->field_0x1DC, true);
        bomb->fn_8020BFBC(&params);
    }
}

HeavyDog0510CtrlUnit_Bomb::~HeavyDog0510CtrlUnit_Bomb() {
    boost::shared_ptr<ThrowBomb> bomb(m_bomb.lock());
    if (bomb) {
        bomb->fn_8020BF9C();
    }
}

/* Returns true once the bomb no longer exists. An if with two returns: the original converts
 * the comparison to bool a second time after it has destroyed lock()'s temporary, which
 * returning the comparison itself does not do. */
bool HeavyDog0510CtrlUnit_Bomb::Update(float arg) {
    if (m_bomb.lock().get() == 0) {
        return true;
    }
    return false;
}

HeavyDog0510CtrlUnit_FixLookCharCamera::HeavyDog0510CtrlUnit_FixLookCharCamera(
    HeavyDog0510VarParam *param)
    : BossCtrlUnit(param), HeavyDog0510CtrlUnit(param), BossCtrlUnit_FixLookCharCamera(param) {
    field_0x40 = 0;
}

/* The override of BossCtrlUnit_FixLookCharCamera::fn_80291514 (behaviour from the body; the
 * meanings of the values are unknown). It clears field_0x40 and returns false when field_0x1B0
 * is below m_params.field_0x3C and field_0x194.y is below the height of the boss, or when the
 * boss is above field_0x1F0 and field_0x194.y is below -52. Otherwise state 0 sets the base's
 * two floats and vector from field_0x194.y and returns true: below -52 they are 120, 30 and
 * (0, -80, 0) and the state becomes 3, above 200 they are 120, 30 and the zero vector (state
 * 1), and otherwise 60, 16 and the zero vector (state 2). States 1 to 3 return true until
 * field_0x194.y is below 200 (state 1), above 200 or below -52 (state 2) or above -52 (state
 * 3); they then clear the state and return false. The two tests at the top read the variables
 * through m_heavyDog0510Param and the states through m_heavyDogParam, as the original does. */
bool HeavyDog0510CtrlUnit_FixLookCharCamera::fn_80291514() {
    if (m_heavyDog0510Param->field_0x1B0 < m_heavyDog0510Param->m_params.field_0x3C &&
        m_heavyDog0510Param->field_0x194.y < m_heavyDog0510Param->m_charInfo->m_position.y) {
        field_0x40 = 0;
        return false;
    }
    if (m_heavyDog0510Param->m_charInfo->m_position.y > m_heavyDog0510Param->field_0x1F0 &&
        m_heavyDog0510Param->field_0x194.y < -52.0f) {
        field_0x40 = 0;
        return false;
    }

    switch (field_0x40) {
    case 0:
        if (m_heavyDogParam->field_0x194.y < -52.0f) {
            field_0x8 = 120.0f;
            field_0xC = 30.0f;
            field_0x18 = lbl_805E07A0;
            field_0x18.y = -80.0f;
            field_0x40 = 3;
        } else if (m_heavyDogParam->field_0x194.y > 200.0f) {
            field_0x8 = 120.0f;
            field_0xC = 30.0f;
            field_0x18 = lbl_805E07A0;
            field_0x40 = 1;
        } else {
            field_0x8 = 60.0f;
            field_0xC = 16.0f;
            field_0x18 = lbl_805E07A0;
            field_0x40 = 2;
        }
        break;
    case 1:
        if (m_heavyDogParam->field_0x194.y < 200.0f) {
            field_0x40 = 0;
            return false;
        }
        break;
    case 2:
        if (m_heavyDogParam->field_0x194.y > 200.0f || m_heavyDogParam->field_0x194.y < -52.0f) {
            field_0x40 = 0;
            return false;
        }
        break;
    case 3:
        if (m_heavyDogParam->field_0x194.y > -52.0f) {
            field_0x40 = 0;
            return false;
        }
        break;
    }
    return true;
}

HeavyDog0510CtrlUnit_FixLookCharCamera::~HeavyDog0510CtrlUnit_FixLookCharCamera() {}

/* The original defines this override although it only forwards to the second base's Update
 * (0x8026B718 in USA (GUPE8P), 0x24 bytes). */
bool HeavyDog0510CtrlUnit_FixLookCharCamera::Update(float arg) {
    return BossCtrlUnit_FixLookCharCamera::Update(arg);
}
