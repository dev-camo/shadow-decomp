/* BombingWall (class name from RTTI; the gadget's descriptor is "BombingWall"). File name inferred
 * from the class. Method names marked "(name inferred)" in the header are not original.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/BombingWall.h"
#include "Game/CharaColliAttack.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The ids of the model entries whose files the break effect shows, by type (name inferred). */
static const int s_breakEntries[2] = {1000, 1001};

const char *BombingWall::s_name = "BombingWall";

BombingWall::BombingWall(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_state(0) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x5B, 4);
    ReadOwner(owner);
    m_model = fn_8015F800(1, "BOMBINGWALL.BIN");
    m_object = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0xA000);
    SetupModel();
    UpdateModelMatrix();
}

BombingWall::~BombingWall() {}

extern "C" void fn_80314EEC(void) {
    fn_8006B474("BOMBINGWALL.BIN", 1);
}

extern "C" void fn_80314EE8(void) {}

void BombingWall::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new BombingWall(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void BombingWall::UpdateBody(float dt) {
    switch (m_state) {
    case 1:
    case 2: {
        UnkEffectParams_8015F0B8 effect(m_charInfo, 0);

        effect.field_0x44 = static_cast<RpClump *>(fn_80012700(
            fn_800122D4(), 1, m_model->fn_8015F7BC(s_breakEntries[m_params.type])->name));
        effect.field_0x40 = m_charInfo->m_position.y;
        effect.field_0x34 = 1.5f;
        effect.field_0x4A = 0;
        effect.field_0x54 = 1;
        fn_8015D93C(&effect);

        fn_801D54F4(fn_80053164(), 0x764A, &m_charInfo->m_position, 0, 1);
        m_model->fn_80162790();
        m_model->fn_80161880();

        if (m_switchId >= 1) {
            fn_8016907C(fn_800CB69C(), m_switchId, 1);
        }

        fn_800C9B04(m_owner);
        m_flags |= 1;
        break;
    }

    case 0:
        if (m_switchId >= 1 && fn_8016904C(fn_800CB69C(), m_switchId) == 1) {
            m_state = 2;
            break;
        }

        if (m_params.type == 0) {
            boost::shared_ptr<CharInfo> nearest =
                fn_800A8A7C(fn_80074194(), &m_charInfo->m_position).lock();

            if (nearest.get() != 0) {
                Fn8040Vector3 distance = nearest->m_position - m_charInfo->m_position;

                if (fn_8040CF50(&distance) < m_params.range) {
                    m_state = 2;
                    break;
                }
            }
        }
        break;
    }

    m_model->fn_80162FB0(dt);
}

void BombingWall::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    BombingWallParams previous = m_params;

    ReadOwner(owner);

    if (m_params.type != previous.type) {
        m_model->fn_80162804();
        m_model->fn_80161880();
        m_state = 0;
        SetupModel();
    }

    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();
    fn_802E0E84(&m_charInfo->m_position, lbl_805F06D8, m_params.range);
}

void BombingWall::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E50(owner, &m_switchId);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
}

void BombingWall::SetupModel() {
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo));

    boost::shared_ptr<CharaColliAttack> attack = m_model->fn_80161428(0).lock();

    if (attack.get() != 0) {
        attack->fn_80096B64(1, AttackCallback(boost::bind(&BombingWall::OnAttack, this, _1)));
    }
}

void BombingWall::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
    m_model->fn_801633DC(&matrix);
    fn_800CCE38(m_object.get(), &m_charInfo->m_position);
}

void BombingWall::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;

    if (m_state == 0) {
        boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock();

        if (attack.get() != 0 && (attack->field_0x40 & 0x200)) {
            param.field_0x54 = 3;

            boost::shared_ptr<CharInfo> attacker = fn_8015D570(&param);

            if (attacker.get() != 0) {
                fn_8020642C(attacker, 0x24);
            }

            m_state = 1;
        }
    }
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "Type 0:Out 1:In", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "Range (point)", 0, 0, 0, 50.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8055BEF4 = {
    "BombingWall", fn_80314EEC, fn_80314EE8, BombingWall::Create, 0, 0, 0x1004, 1, 0, 10,
    s_paramInfo,
};
