/* ElecSmallCube (class name from RTTI); original filename unknown. */
#include "Game/ElecSmallCube.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkResource_80012700.h"
#include "Game/SharedConstants.h"
#include "Game/fn_8040_random.h"
#include "Game/fn_8006_resource_list.h"
#include "boost/bind.hpp"

const char *ElecSmallCube::s_name = "ElecSmallCube";

/* Selects the next rotation; helper name inferred from its repeated operation. */
inline void ElecSmallCube::PickNextRotation() {
    m_rotationTo.x = fn_8040EC80(0.1f * -lbl_805FAD38, 0.1f * lbl_805FAD38);
    m_rotationTo.y = fn_8040EC80(0.1f * -lbl_805FAD38, 0.1f * lbl_805FAD38);
    m_rotationTo.z = fn_8040EC80(0.1f * -lbl_805FAD38, 0.1f * lbl_805FAD38);
    switch (fn_8040EA90(3)) {
    case 0:
        m_rotationTo.x +=
            m_rotationTo.x < 0.0f ? 0.66667f * -lbl_805FAD38 : 0.66667f * lbl_805FAD38;
        break;
    case 1:
        m_rotationTo.y +=
            m_rotationTo.y < 0.0f ? 0.66667f * -lbl_805FAD38 : 0.66667f * lbl_805FAD38;
        break;
    case 2:
        m_rotationTo.z +=
            m_rotationTo.z < 0.0f ? 0.66667f * -lbl_805FAD38 : 0.66667f * lbl_805FAD38;
        break;
    }
    m_rotationTo.x += m_rotationFrom.x;
    m_rotationTo.y += m_rotationFrom.y;
    m_rotationTo.z += m_rotationFrom.z;
}

ElecSmallCube::ElecSmallCube(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_rotationFrom.x = 0.0f;
    m_rotationFrom.y = 0.0f;
    m_rotationFrom.z = 0.0f;
    m_rotationTo.x = 0.0f;
    m_rotationTo.y = 0.0f;
    m_rotationTo.z = 0.0f;
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x58, 4, "ElecSmallCube");
    m_model = new GadgetModelInfo(1, "SMALLROLLCUBE.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col", m_charInfo);
    boost::weak_ptr<UnkAttack_8005A840> weakAttack = m_model->fn_80161428(0);
    {
        boost::shared_ptr<UnkAttack_8005A840> attack(weakAttack.lock());
        if (attack.get() != 0) {
            attack->fn_80096B64(1,
                                AttackCallback(boost::bind(&ElecSmallCube::fn_8021AEAC, this, _1)));
        }
    }

    fn_8021B1B0(m_owner);
    switch (fn_8040EA90(3)) {
    case 0:
        m_charInfo->field_0x30 = 0;
        break;
    case 1:
        m_charInfo->field_0x30 = 1;
        break;
    case 2:
        m_charInfo->field_0x30 = 2;
        break;
    }

    field_0x3C = 0;
    m_timer = 0.0f;
    m_rotationFrom.x = fn_8040EC80(-lbl_805FAD38, lbl_805FAD38);
    m_rotationFrom.y = fn_8040EC80(-lbl_805FAD38, lbl_805FAD38);
    m_rotationFrom.z = fn_8040EC80(-lbl_805FAD38, lbl_805FAD38);
    PickNextRotation();
    fn_8021B140();
}

ElecSmallCube::~ElecSmallCube() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

void ElecSmallCube::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_8021B1B0(owner);
    fn_8021B140();
    if (m_model != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();
    }
}

void ElecSmallCube::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }
    switch (field_0x3C) {
    case 0:
        fn_8021AEC0(dt);
        break;
    case 1:
        fn_8021ADC0();
        fn_8015A794(this);
        m_flags |= 1;
        return;
    }
    fn_8021B140();
}

void ElecSmallCube::fn_8021B1B0(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
}

void ElecSmallCube::fn_8021B140() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix;
        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8001E070(&matrix, &m_charInfo->m_rotation, m_charInfo->field_0x30);
        m_model->fn_801633DC(&matrix);
    }
}

void ElecSmallCube::fn_8021AEC0(float dt) {
    float t;
    m_timer += dt;
    if (m_timer < 0.25f) {
        t = 0.75f * (m_timer / 0.25f);
    } else {
        t = 0.25f * ((m_timer - 0.25f) / 0.85f) + 0.75f;
        if (t > 1.0f) {
            t = 1.0f;
        }
    }

    const float totalWeight = t + (1.0f - t);
    m_charInfo->m_rotation.x =
        m_rotationFrom.x + (m_rotationTo.x - m_rotationFrom.x) * t / totalWeight;
    m_charInfo->m_rotation.y =
        m_rotationFrom.y + (m_rotationTo.y - m_rotationFrom.y) * t / totalWeight;
    m_charInfo->m_rotation.z =
        m_rotationFrom.z + (m_rotationTo.z - m_rotationFrom.z) * t / totalWeight;

    if (m_timer > 1.1f) {
        m_timer = 0.0f;
        m_rotationFrom = m_rotationTo;
        PickNextRotation();
    }
}

void ElecSmallCube::fn_8021AEAC(SonicteamUSA::System::CharaColli::AttackCallbackParam &param) {
    param.field_0x54 = 3;
    field_0x3C = 1;
}

void ElecSmallCube::fn_8021ADC0() {
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

extern "C" void fn_8021AD94() {
    fn_8006B474("SMALLROLLCUBE.BIN", 1);
}

extern "C" void fn_8021AD90() {}

void ElecSmallCube::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecSmallCube(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_8054588C = {
    "ELEC_SMALLCUBE", fn_8021AD94, fn_8021AD90, ElecSmallCube::Create, 0, 0, 0x7E4, 1, 0, 10, 0,
};
