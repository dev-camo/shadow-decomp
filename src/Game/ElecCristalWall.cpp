/* The ElecCristalWall gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The model attack object's callback is a boost::function object built with
 * boost::bind(&ElecCristalWall::OnAttack, this, _1); its manager, invoker and copy functions come
 * from the boost headers. */
#include "Game/ElecCristalWall.h"
#include "Game/AngleConversion.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkModelAtomicState.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_random.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include <cmath>

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The colours of the "mat" part (names inferred), each applied only when the part is found: during
 * the appearance the colour is set along a fade from the first to the second, and while the flash
 * timer is not running its last three bytes are then overwritten with the health colour, which goes
 * from the second (6 health) through the third (3 health) towards the fourth. */
static UnkDebugColor_802E0E84 s_appearColorStart(0xFF, 0x00, 0x00, 0x00);
static UnkDebugColor_802E0E84 s_appearColorEnd(0xFF, 0x00, 0x33, 0x00);
static UnkDebugColor_802E0E84 s_damageColorMiddle(0xFF, 0x26, 0x26, 0x00);
static UnkDebugColor_802E0E84 s_damageColorEnd(0xFF, 0x33, 0x00, 0x00);

/* Sets the last three bytes of color to the blend to * t + from * (1 - t) (name inferred). With
 * t above 1 (below 2 health) the blend extrapolates: at 1 health, byte 2 is 0 * 2 + 0x26 * -1 =
 * -38, which the original converts and stores as its low byte, 0xDA (USA 0x801F94B0-0x801F94BC). */
static inline void BlendColor(UnkDebugColor_802E0E84 &color, const UnkDebugColor_802E0E84 &from,
                              const UnkDebugColor_802E0E84 &to, float t) {
    color.field_0x1 = to.field_0x1 * t + from.field_0x1 * (1.0f - t);
    color.field_0x2 = to.field_0x2 * t + from.field_0x2 * (1.0f - t);
    color.field_0x3 = to.field_0x3 * t + from.field_0x3 * (1.0f - t);
}

const char *ElecCristalWall::s_name = "ElecCristalWall";

ElecCristalWall::ElecCristalWall(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_appearRotation(0.0f, 0.0f, 0.0f), m_rotation(0.0f, 0.0f, 0.0f) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_charInfo = fn_8015CB88(0x58, 4, "ElecCristalWall");
    m_model = new GadgetModelInfo(1, "WALL.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_8016239C("mat");
    m_model->fn_80161F08("col", m_charInfo);

    boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161428(0);
    if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
        attack->fn_80096B64(1, AttackCallback(boost::bind(&ElecCristalWall::OnAttack, this, _1)));
    }

    CopyFromOwner(m_owner);
    m_state = STATE_RESET;
    m_health = 6;

    switch (fn_8040EA90(3)) {
    case 0:
        m_rotationOrder = 0;
        break;
    case 1:
        m_rotationOrder = 1;
        break;
    case 2:
        m_rotationOrder = 2;
        break;
    }

    m_appearRotation.x = lbl_805FAD38 * (fn_8040EA90(2) != 0 ? 1.0f : -1.0f);
    m_appearRotation.y = lbl_805FAD38 * (fn_8040EA90(2) != 0 ? 1.0f : -1.0f);
    m_appearRotation.z = lbl_805FAD38 * (fn_8040EA90(2) != 0 ? 1.0f : -1.0f);
    UpdateModelMatrix();
}

ElecCristalWall::~ElecCristalWall() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

void ElecCristalWall::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case STATE_RESET:
        m_state = STATE_WAITING;
        /* The result is not used. */
        m_model->fn_801621F0("mat");
        break;

    case STATE_WAITING:
        if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
            m_state = STATE_APPEARING;
            m_appearTime = 0.0f;
            m_appearOffset.x = m_appearOffset.y = 0.0f;
            m_appearOffset.z = -75.0f;

            m_appearOffset = fn_8040D118(m_appearOffset, DegreesToRadians(m_params.appearAngleX));
            m_appearOffset = fn_8040D0AC(m_appearOffset, DegreesToRadians(m_params.appearAngleY));

            fn_800C9F14(m_owner, &m_position);
            fn_8040CFF8(&m_position, &m_appearOffset);
            fn_801D54F4(fn_80053164(), 0x723F, &m_position, 0, 1);
        }
        break;

    case STATE_APPEARING: {
        m_appearTime += dt;
        if (m_appearTime > 1.0f) {
            m_appearTime = 1.0f;
            m_state = STATE_APPEARED;

            UnkModelAtomicState *flash = m_model->fn_801621F0("mat");
            if (flash != 0) {
                flash->color.field_0x1 = 0xC8;
                flash->color.field_0x2 = 0xFF;
                flash->color.field_0x3 = 0xFF;
            }
            m_flashTime = 0.25f;
        }

        UnkModelAtomicState *state = m_model->fn_801621F0("mat");
        if (state != 0) {
            fn_80154C6C(state, m_appearTime, s_appearColorStart, s_appearColorEnd, 0.0f, 1.0f);
        }

        /* The remaining part of the offset and of the spin: cos^3 of a quarter turn times the
         * appear time, from 1 down to 0. */
        float cosine = std::cos(m_appearTime * lbl_805FAD38 * 0.5f);
        float remaining = cosine * cosine * cosine;

        fn_800C9F14(m_owner, &m_position);
        AddVector(&m_position, m_appearOffset * remaining);
        m_rotation.x = m_appearRotation.x * remaining;
        m_rotation.y = m_appearRotation.y * remaining;
        m_rotation.z = m_appearRotation.z * remaining;
        break;
    }

    case STATE_HEALTH_USED_UP:
        if (m_health <= 0) {
            ShowBreakEffect();
            fn_8015A794(this);
            m_flags |= 1;
            return;
        }
        break;
    }

    UpdateModelMatrix();

    if (m_flashTime > 0.0f) {
        m_flashTime -= dt;
        if (m_flashTime < 0.0f) {
            m_flashTime = 0.0f;
        }
    } else {
        UnkModelAtomicState *state = m_model->fn_801621F0("mat");
        if (state != 0) {
            /* The original divides by the health in the lower half, so t passes 1 below 2
             * health. */
            if (m_health <= 3) {
                float t = (float)(3 - m_health) / m_health;

                BlendColor(state->color, s_damageColorMiddle, s_damageColorEnd, t);
            } else {
                float t = (float)(6 - m_health) / 3.0f;

                BlendColor(state->color, s_appearColorEnd, s_damageColorMiddle, t);
            }
        }
    }
}

void ElecCristalWall::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    m_state = STATE_RESET;
    UpdateModelMatrix();
    if (m_model != 0) {
        m_model->fn_80162EF4();
    }
}

void ElecCristalWall::fn_800CBC1C(float) {
    fn_800C9F14(m_owner, &m_position);
    m_rotation.x = m_rotation.y = m_rotation.z = 0.0f;

    Fn8040Vector3 direction(0.0f, 0.0f, -30.0f);

    direction = fn_8040D118(direction, DegreesToRadians(m_params.appearAngleX));
    direction = fn_8040D0AC(direction, DegreesToRadians(m_params.appearAngleY));
    UnkSegment_80020A20 line(m_charInfo->m_position, m_charInfo->m_position + direction);

    fn_802E0AA0(line, lbl_805F06D8, 3.0f);
    if (m_model != 0) {
        m_model->fn_8016108C();
    }
}

void ElecCristalWall::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    if (m_params.type == 1) {
        Fn8040Matrix34 matrix;

        fn_800193B8(&matrix, &m_charInfo->m_rotation, 2);
        fn_8040B884(&matrix, 0.5f * lbl_805FAD38);
        fn_800494F8(&m_charInfo->m_rotation, &matrix);
    }
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E60(owner, &m_comId);
}

void ElecCristalWall::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_position);
        fn_8001E070(&matrix, &m_rotation, m_rotationOrder);
        fn_8001E070(&matrix, &m_charInfo->m_rotation, 2);
        m_model->fn_801633DC(&matrix);
    }
}

void ElecCristalWall::OnAttack(AttackCallbackParam &param) {
    if (m_flashTime == 0.0f && m_state == STATE_APPEARED) {
        UnkModelAtomicState *flash = m_model->fn_801621F0("mat");
        if (flash != 0) {
            flash->color.field_0x1 = 0xC8;
            flash->color.field_0x2 = 0xC8;
            flash->color.field_0x3 = 0;
        }
        m_flashTime = 0.125f;

        m_health--;
        if (m_health <= 0) {
            param.field_0x54 = 3;
            m_state = STATE_HEALTH_USED_UP;
            fn_801D54F4(fn_80053164(), 0x7241, &m_charInfo->m_position, 0, 1);
        } else {
            param.field_0x54 = 1;
            fn_801D54F4(fn_80053164(), 0x7240, &m_charInfo->m_position, 0, 1);
        }
    } else {
        param.field_0x54 = 1;
    }
}

void ElecCristalWall::ShowBreakEffect() {
    if (m_model != 0) {
        UnkModelEntry_8015F7BC *entry = m_model->fn_8015F7BC(1);

        if (entry != 0) {
            UnkResourceManager_800122D4 *resources = fn_800122D4();
            UnkEffectParams_8015F0B8 parameters(m_charInfo, 0);

            parameters.field_0x34 = 1.5f;
            parameters.field_0x44 = static_cast<RpClump *>(fn_80012700(resources, 1, entry->name));
            parameters.field_0x54 = 0;
            parameters.field_0x4C = 150;
            fn_8015D93C(&parameters);
        }
    }
}

extern "C" void fn_801F8CDC(void) {
    fn_8006B474("WALL.BIN", 1);
}

extern "C" void fn_801F8CD8(void) {}

void ElecCristalWall::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecCristalWall(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static const char *s_typeLabels[2] = {"type< Horizontal >", "type< Vertical   >"};

/* The label callback of the descriptor's first parameter. */
static const char *fn_801F8C58(UnkParamCursor_800CBC20 *cursor) {
    switch (cursor->m_index) {
    case 0:
        return s_typeLabels[cursor->m_value];
    }

    return 0;
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_801F8C58, 0},
    {4, "AppearAngleX", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "AppearAngleY", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_805424C4 = {
    "ELEC_WALL", fn_801F8CDC, fn_801F8CD8, ElecCristalWall::Create, 0, 0, 0x7D7, 1,
    0,           10,          s_paramInfo,
};
