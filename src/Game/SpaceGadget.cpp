/* SpaceGadget (class name from RTTI): a gadget that shows the model SPACEGADGET.BIN with the part
 * given by the parameter Model and turns it at the speeds RotDegSpdX, RotDegSpdY and RotDegSpdZ.
 * When the model has the entry Model + 1000, the part's attack object calls OnAttack; once the
 * health is used up, the gadget plays sound 0x774C, passes a parameter block with the clump named
 * by that entry to fn_8015D93C and marks itself finished (behaviour inferred). File name inferred
 * from the class. Method names marked "(name inferred)" are not original. Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/SpaceGadget.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SharedConstants.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8000A950_near_zero.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"

using SonicteamUSA::System::CharaColli::CharaColliAttack;

const char *SpaceGadget::s_name = "SpaceGadget";

SpaceGadget::SpaceGadget(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_state(0) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x65, 4);
    ReadOwner(owner);
    m_model = fn_8015F800(1, "SPACEGADGET.BIN");
    SetupModel();
}

SpaceGadget::~SpaceGadget() {}

extern "C" void fn_8026B164(void) {
    fn_8006B474("SPACEGADGET.BIN", 1);
}

extern "C" void fn_8026B160(void) {}

void SpaceGadget::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new SpaceGadget(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void SpaceGadget::UpdateBody(float dt) {
    switch (m_state) {
    case 0:
        m_charInfo->m_rotation.x += m_params.rotSpdX * dt;
        m_charInfo->m_rotation.y += m_params.rotSpdY * dt;
        m_charInfo->m_rotation.z += m_params.rotSpdZ * dt;
        UpdateModelMatrix();
        break;
    case 1: {
        m_model->fn_80162790();
        m_model->fn_80161880();
        fn_801D54F4(fn_80053164(), 0x774C, &m_charInfo->m_position, 0, 1);

        UnkEffectParams_8015F0B8 parameters(m_charInfo, 0);
        UnkModelEntry_8015F7BC *entry = m_model->fn_8015F7BC(m_params.model + 1000);
        UnkResourceManager_800122D4 *resources = fn_800122D4();
        parameters.field_0x44 = static_cast<RpClump *>(fn_80012700(resources, 1, entry->name));
        parameters.field_0x40 = m_charInfo->m_position.y;
        parameters.field_0x34 = 1.0f;
        parameters.field_0x4A = 0;
        parameters.field_0x54 = 1;
        fn_8015D93C(&parameters);

        fn_800C9AD4(m_owner);
        m_flags |= 1;
        break;
    }
    }
    m_model->fn_80162FB0(dt);
}

void SpaceGadget::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    SpaceGadgetParams previous = m_params;

    ReadOwner(owner);
    if (m_params.model != previous.model) {
        if (m_model->fn_80162EA8(m_params.model)) {
            m_model->fn_80162804();
            m_model->fn_80161880();
            SetupModel();
        } else {
            m_params.model = previous.model;
            /* ReadOwner has already converted the speeds to radians, so they are written back
             * in radians (original behaviour). */
            fn_800C9D68(owner, &m_params, sizeof(m_params));
        }
    }
    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();
}

void SpaceGadget::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_params.rotSpdX *= lbl_805FAD38 / 180.0f;
    m_params.rotSpdY *= lbl_805FAD38 / 180.0f;
    m_params.rotSpdZ *= lbl_805FAD38 / 180.0f;
    m_health = 1.0f;
    m_state = 0;
}

void SpaceGadget::SetupModel() {
    m_model->fn_80162A68(m_params.model, 0);
    m_model->fn_80161FC8(m_params.model, m_charInfo);
    if (m_model->fn_8015F7BC(m_params.model + 1000) != 0) {
        boost::shared_ptr<CharaColliAttack> attack(m_model->fn_80161428(m_params.model).lock());

        if (attack.get() != 0) {
            attack->fn_80096B64(1, AttackCallback(boost::bind(&SpaceGadget::OnAttack, this, _1)));
        }
    }
}

void SpaceGadget::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
    m_model->fn_801633DC(&matrix);
}

void SpaceGadget::OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param) {
    param.field_0x54 = 1;

    if (m_state != 1) {
        m_health -= param.field_0x50;

        if (m_health < 0.0f || fn_8000A950(m_health)) {
            param.field_0x54 = 3;

            boost::shared_ptr<CharInfo> attacker(fn_8015D570(&param));

            if (attacker.get() != 0) {
                fn_8020642C(attacker, 0x23);
            }

            m_state = 1;
        }
    }
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "Model", 0, 0, 100, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "RotDegSpdX", 0, 0, 0, 10.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "RotDegSpdY", 0, 0, 0, 10.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "RotDegSpdZ", 0, 0, 0, 10.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8054C72C = {
    "SPACEGADGET", fn_8026B164, fn_8026B160, SpaceGadget::Create, 0, 0, 0x1392, 1, 0, 10,
    s_paramInfo,
};
