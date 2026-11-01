/* ElecRollHexa (class name from RTTI); original filename unknown. */
#include "Game/ElecRollHexa.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SharedConstants.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "boost/bind.hpp"
#include <cmath>

const char *ElecRollHexa::s_name = "ElecRollHexa";

ElecRollHexa::ElecRollHexa(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_rotateSpeed, 4);
    m_charInfo = fn_8015CB88(0x58, 4, "ElecRollHexa");
    m_model = new GadgetModelInfo(1, "ROLLHEXA.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col", m_charInfo);
    boost::weak_ptr<UnkAttack_8005A840> weakAttack = m_model->fn_80161428(0);
    {
        boost::shared_ptr<UnkAttack_8005A840> attack(weakAttack.lock());
        if (attack.get() != 0) {
            attack->fn_80096B64(1,
                                AttackCallback(boost::bind(&ElecRollHexa::fn_80267264, this, _1)));
        }
    }
    fn_802673E8(m_owner);
    /* The original applies the initial transform before the first height-offset update. */
    fn_80267334();
    field_0x48 = 0;
    m_timer = 0.0f;
}

ElecRollHexa::~ElecRollHexa() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

void ElecRollHexa::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }
    switch (field_0x48) {
    case 0:
        fn_80267278();
        break;
    case 1:
        fn_80267164();
        fn_8015A794(this);
        m_flags |= 1;
        return;
    }
    fn_80267334();
}

void ElecRollHexa::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_802673E8(owner);
    fn_80267278();
    fn_80267334();
    if (m_model != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();
    }
}

void ElecRollHexa::fn_802673E8(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_rotateSpeed, 4);
}

void ElecRollHexa::fn_80267334() {
    if (m_model != 0) {
        Fn8040Vector3 position;
        fn_80008E1C(&position, (*m_charInfo).m_position.x,
                    m_heightOffset + (*m_charInfo).m_position.y, (*m_charInfo).m_position.z);
        Fn8040Matrix34 translation;
        fn_8040B034(&translation, &position);
        Fn8040Matrix34 matrix(translation);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }
}

void ElecRollHexa::fn_80267278() {
    m_timer += lbl_8057D6A0.field_0x28;
    m_charInfo->m_rotation.y = m_timer * (m_rotateSpeed * (lbl_805FAD38 / 180.0f));
    const float phase = (m_timer - static_cast<int>(m_timer / 5.0f) * 5.0f) / 5.0f;
    m_heightOffset = 2.5f * std::sin((2.0f * lbl_805FAD38) * phase);
}

void ElecRollHexa::fn_80267264(SonicteamUSA::System::CharaColli::AttackCallbackParam &param) {
    param.field_0x54 = 3;
    field_0x48 = 1;
}

void ElecRollHexa::fn_80267164() {
    fn_801D560C(fn_80053164(), 0x7241, 0, 0);
    if (m_model != 0) {
        UnkModelEntry_8015F7BC *entry = m_model->fn_8015F7BC(1);
        if (entry != 0) {
            UnkResourceManager_800122D4 *resources = fn_800122D4();
            UnkEffectParams_8015F0B8 parameters(m_charInfo, 0);
            parameters.field_0x34 = 1.5f;
            parameters.field_0x44 = static_cast<RpClump *>(fn_80012700(resources, 1, entry->name));
            parameters.field_0x54 = 1;
            parameters.field_0x4C = 150;
            fn_8015D93C(&parameters);
        }
    }
}

extern "C" void fn_80267138() {
    fn_8006B474("ROLLHEXA.BIN", 1);
}

extern "C" void fn_80267134() {}

void ElecRollHexa::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecRollHexa(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo lbl_8054C25C[] = {
    {4, "RotateSpeed", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8054C2AC = {
    "ELEC_ROLLHEXA", fn_80267138, fn_80267134, ElecRollHexa::Create, 0, 0, 0x7E8, 1, 0, 10,
    lbl_8054C25C,
};
