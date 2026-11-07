/* The CITYGADGET gadget and its owner state (classes described in Game/CityGadget.h). File name
 * inferred from the class. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first).
 *
 * The attack callback is a boost::function object built with
 * boost::bind(&CityGadget::OnAttack, this, _1); its manager, invoker and copy functions come from
 * the boost headers. */
#include "Game/CityGadget.h"
#include "Game/CharInfo.h"
#include "Game/CharaColliBase.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8000A950_near_zero.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;
using SonicteamUSA::System::CharaColli::CharaColliBase;

/* The five types that the "Type" parameter selects (name inferred). Types 0 and 1 share model id 0
 * and only type 1 has a broken model (2000); types 3 and 4 accept more attacks (mask 0xBF0). */
static const CityGadgetType s_types[] = {
    {0, 1000, -1, 0x7135, 0x200, 1.0f}, {0, 1000, 2000, 0x7135, 0x200, 1.0f},
    {1, 1001, -1, 0x7136, 0x200, 1.0f}, {2, 1002, -1, 0x7138, 0xBF0, 1.0f},
    {3, 1003, -1, 0x713A, 0xBF0, 1.0f},
};

const char *CityGadget::s_name = "CityGadget";

CityGadget::CityGadget(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_type(0), m_data(0), m_health(0.0f) {
    m_name = s_name;

    if (owner->m_adapterState == 0) {
        fn_800C9930(owner, new CityGadgetState);
    }

    m_data = static_cast<CityGadgetState *>(owner->m_adapterState);
    m_charInfo = fn_8015CD90(0x52, 4);
    ReadOwner(owner);
    m_model = fn_8015F800(1, "CITYGADGET.BIN");
    SetupModel();
    UpdateModelMatrix();
    ApplyScale();
}

CityGadget::~CityGadget() {}

extern "C" void fn_802F6F88(void) {
    fn_8006B474("CITYGADGET.BIN", 1);
}

extern "C" void fn_802F6F84(void) {}

void CityGadget::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new CityGadget(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void CityGadget::UpdateBody(float dt) {
    switch (m_data->m_state) {
    case CityGadgetState::STATE_BREAKING: {
        UnkEffectParams_8015F0B8 parameters(m_charInfo, 0);
        UnkModelEntry_8015F7BC *entry = m_model->fn_8015F7BC(m_type->effectEntry);
        parameters.field_0x44 = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, entry->name));
        parameters.field_0x40 = m_charInfo->m_position.y;
        parameters.field_0x34 = 1.0f;
        parameters.field_0x4A = m_ownerFlag;
        parameters.field_0x54 = 1;
        fn_8015D93C(&parameters);

        fn_801D54F4(fn_80053164(), m_type->sound, &m_charInfo->m_position, 0, 1);

        if (m_type->brokenModelId == -1) {
            m_model->fn_80162790();
            m_model->fn_80161880();
            fn_800C9B04(m_owner);
            m_flags |= 1;
        } else {
            m_model->fn_80162804();
            m_model->fn_80161880();
            m_model->fn_80162A68(m_type->brokenModelId, 0);
            m_model->fn_80161FC8(m_type->brokenModelId, m_charInfo);
            m_data->m_state = CityGadgetState::STATE_BROKEN;
            UpdateModelMatrix();
            ApplyScale();
        }
        break;
    }
    }

    m_model->fn_80162FB0(dt);
}

void CityGadget::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CityGadgetParams previous = m_params;

    ReadOwner(owner);
    if (m_params.type != previous.type) {
        m_model->fn_80162804();
        m_model->fn_80161880();
        m_data->m_state = CityGadgetState::STATE_INTACT;
        SetupModel();
    }

    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();
}

void CityGadget::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    u32 ownerFlags;

    fn_800C9E70(owner, &ownerFlags);
    m_ownerFlag = (ownerFlags & 0x400000) != 0;
    m_type = &s_types[m_params.type];
    m_health = m_type->health;
}

void CityGadget::SetupModel() {
    switch (m_data->m_state) {
    case CityGadgetState::STATE_INTACT: {
        m_model->fn_80162A68(m_type->modelId, 0);
        m_model->fn_80161FC8(m_type->modelId, m_charInfo);

        boost::shared_ptr<CharaColliAttack> attack(m_model->fn_80161428(m_type->modelId).lock());

        if (attack.get() != 0) {
            attack->fn_80096B64(1, AttackCallback(boost::bind(&CityGadget::OnAttack, this, _1)));
        }
        break;
    }
    case CityGadgetState::STATE_BROKEN:
        m_model->fn_80162A68(m_type->brokenModelId, 0);
        m_model->fn_80161FC8(m_type->brokenModelId, m_charInfo);
        break;
    }
}

void CityGadget::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);

    Fn8040Vector3 scale(m_params.scaleX, m_params.scaleY, m_params.scaleZ);

    fn_8040B790(&matrix, scale);
    m_model->fn_801633DC(&matrix);
}

void CityGadget::ApplyScale() {
    s32 modelId =
        m_data->m_state == CityGadgetState::STATE_INTACT ? m_type->modelId : m_type->brokenModelId;

    boost::shared_ptr<CharaColliBase> collision(m_model->fn_80161614(modelId).lock());

    if (collision.get() != 0) {
        Fn8040Vector3 scale(m_params.scaleX, m_params.scaleY, m_params.scaleZ);

        collision->UnkSlot15(&scale);
    }
}

void CityGadget::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;
    if (m_data->m_state == CityGadgetState::STATE_INTACT) {
        boost::shared_ptr<CharaColliAttack> attack(param.field_0x0.lock());

        if (attack.get() != 0 && (m_type->attackMask & attack->field_0x40)) {
            param.field_0x54 = 1;
            m_health -= param.field_0x50;
            param.field_0x40 = param.field_0x24;
            if (m_health < 0.0f || fn_8000A950(m_health)) {
                param.field_0x54 = 3;

                boost::shared_ptr<CharInfo> attacker = fn_8015D570(&param);
                if (attacker.get() != 0) {
                    fn_8020642C(attacker, 0x23);
                }
                m_data->m_state = CityGadgetState::STATE_BREAKING;
            }
        }
    }
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "Type", 0, 0, 4, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "ScaleX", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "ScaleY", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "ScaleZ", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8055A314 = {
    "CITYGADGET", fn_802F6F88, fn_802F6F84, CityGadget::Create, 0, 0, 0xFA4, 1, 0, 10, s_paramInfo,
};
