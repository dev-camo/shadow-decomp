/* ElecBlock (class name from RTTI; the gadget's descriptor is "ELEC_BIGBLOCK"): a block of the
 * electric circuit area. File name inferred from the class. Method names marked "(name inferred)"
 * are not original. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first). */
#include "Game/ElecBlock.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetAccelerate.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8000A950_near_zero.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"
#include <cmath>

const char *ElecBlock::s_name = "ElecBlock";

/* Three factors that ApplyScale multiplies by the "Scale" parameters before passing them to the
 * model (placeholder name; their meaning is unknown). */
static Fn8040Vector3 lbl_80580150(86.5f, 195.0f, 86.5f);

ElecBlock::ElecBlock(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_charInfo = fn_8015CB88(0x56, 4, "ElecBlock");
    m_model = new GadgetModelInfo(1, "BIGBLOCK.BIN");
    m_loadedBlockType = m_params.blockType;
    m_model->fn_80162A68(m_loadedBlockType, 0);
    CopyFromOwner(m_owner);

    if (m_params.collisionFlag != 0) {
        m_model->fn_80161F08("col", m_charInfo);
        ApplyScale();
    }

    m_height = m_params.initialY;
    if (m_params.initialY == 0.0f) {
        m_state = 3;
    } else {
        m_state = 0;
    }
    m_speed = 0.0f;
    UpdateModelMatrix();
}

ElecBlock::~ElecBlock() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

void ElecBlock::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case 0:
        if (IsCharInfoInRange()) {
            m_state = 1;
            fn_801D54F4(fn_80053164(), 0x723D, &m_charInfo->m_position, 0, 1);
        }
        break;

    case 1:
        if (m_params.initialY < 0.0f) {
            Accelerate(&m_height, &m_speed, 200.0f, 2000.0f, dt);
            if (m_height > 0.0f) {
                m_height = 0.0f;
                m_state = 3;
            }
        } else {
            Accelerate(&m_height, &m_speed, -200.0f, 2000.0f, dt);
            if (m_height < 0.0f) {
                m_height = 0.0f;
                m_state = 3;
            }
        }
        break;

    case 2:
        Integrate(&m_height, &m_speed, -1.0f * m_height, 10.0f, dt);
        if (fn_8000A980(m_speed) < 0.001667f) {
            m_height = 0.0f;
            m_state = 3;
        }
        break;

    case 3: /* at rest */
        break;
    }

    UpdateModelMatrix();
}

void ElecBlock::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    m_state = 0;
    m_height = m_params.initialY;
    UpdateModelMatrix();

    if (m_model != 0) {
        m_model->fn_80162EF4();

        if (m_params.blockType != m_loadedBlockType) {
            m_model->fn_80161880();
            m_model->fn_801604FC();
            m_model->fn_80162804();
            m_loadedBlockType = m_params.blockType;
            m_model->fn_80162A68(m_loadedBlockType, 0);
            m_model->fn_80161FC8(m_loadedBlockType, m_charInfo);
            m_model->fn_80160C28(m_loadedBlockType);
            m_params.blockType = m_loadedBlockType;
            fn_800C9D68(owner, &m_params, sizeof(m_params));
        }

        if (m_params.collisionFlag != 0) {
            ApplyScale();
            m_model->fn_8016108C();
        }

        fn_802E0E84(&m_charInfo->m_position, lbl_805F06D8, m_params.searchRange);
    }
}

void ElecBlock::ApplyScale() {
    float scaledY = lbl_80580150.y * m_params.scaleY;

    m_model->fn_801617D0(0, -2, lbl_80580150.x * m_params.scaleX, scaledY,
                         lbl_80580150.z * m_params.scaleZ);

    Fn8040Vector3 offset(0.0f, -scaledY, 0.0f);

    m_model->fn_80161740(0, &offset, -2);
}

bool ElecBlock::IsCharInfoInRange() {
    boost::shared_ptr<CharInfo> nearest;

    nearest = fn_800A8A7C(fn_80074194(), &m_charInfo->m_position).lock();

    if (nearest.get() != 0) {
        float rangeSquared = m_params.searchRange * m_params.searchRange;

        const Fn8040Vector3 &offset = m_charInfo->m_position - nearest->m_position;
        return fn_8040CF2C(offset) <= rangeSquared;
    }

    return false;
}

void ElecBlock::Integrate(float *position, float *velocity, float acceleration, float drag,
                          float duration) {
    if (drag <= 0.0f) {
        float accelerationStep = acceleration * duration;

        *position = *position + *velocity * duration + duration * accelerationStep / 2.0f;
        *velocity = *velocity + accelerationStep;
    } else {
        float growth = std::pow(2.7182817f, drag * duration);
        float decay = 1.0f / growth;
        float inverseDrag = 1.0f / drag;

        *position =
            *position + duration * (acceleration * inverseDrag) +
            (1.0f - decay) * (inverseDrag * (inverseDrag * (drag * *velocity - acceleration)));
        *velocity = acceleration * inverseDrag + decay * (*velocity - acceleration * inverseDrag);
    }
}

void ElecBlock::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
}

void ElecBlock::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);

        Fn8040Vector3 offset(0.0f, m_height, 0.0f);

        fn_8040B59C(&matrix, offset);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B720(&matrix, m_params.scaleX, m_params.scaleY, m_params.scaleZ);
        m_model->fn_801633DC(&matrix);
    }
}

extern "C" void fn_80158684(void) {
    fn_8006B474("BIGBLOCK.BIN", 1);
}

extern "C" void fn_80158680(void) {}

void ElecBlock::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecBlock(fn_801EB304(fn_8001C4F8(), 11), owner);
}

/* The names of the values of the "BlockType" and "CollisionFlag" parameters. */
static const char *s_blockTypeLabels[10] = {
    "BlockType0", "BlockType1", "BlockType2", "BlockType3", "BlockType4",
    "BlockType5", "BlockType6", "BlockType7", "BlockType8", "BlockType9",
};
static const char *s_collisionFlagLabels[2] = {"CollisionFlag< FALSE >", "CollisionFlag< TRUE  >"};

/* The label callback of the descriptor's "BlockType" and "CollisionFlag" entries (behaviour
 * inferred). */
static const char *fn_801585DC(UnkParamCursor_800CBC20 *cursor) {
    switch (cursor->m_index) {
    case 0:
        return s_blockTypeLabels[cursor->m_value];
    case 6:
        return s_collisionFlagLabels[cursor->m_value];
    }

    return 0;
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "BlockType", 0, 0, 9, 0.0f, 0.0f, 0.0f, fn_801585DC, 0},
    {4, "Scale X", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "Scale Y", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "Scale Z", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "SearchRange", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "Initial Y", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {1, "CollisionFlag", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_801585DC, 0},
    {0},
};

GadgetTypeInfo lbl_8052ACC0 = {
    "ELEC_BIGBLOCK", fn_80158684, fn_80158680, ElecBlock::Create, 0, 0, 0x7D5, 1, 0, 10,
    s_paramInfo,
};
