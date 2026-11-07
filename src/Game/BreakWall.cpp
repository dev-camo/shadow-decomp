/* The BreakWall gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The character info's command handler and the model attack object's callback are boost::function
 * objects built with boost::bind(&BreakWall::OnCommand, this, _1) and
 * boost::bind(&BreakWall::OnAttack, this, _1); their managers, invokers and copy functions come
 * from the boost headers. */
#include <cstdio>
/* TNumEdit.h comes before CharaColliBaseCharInfo.h, in the order their names appear in the unit's
 * .rodata (alphabetical order would reverse them).
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit that RTTI
 * data; revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/BreakWall.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

static const char *s_modelFile = "BREAKWALL.BIN";
/* Appended to the crash information's model entry name to name the cracked entry. */
static const char *s_crackSuffix = "_crack";

namespace BreakWall {

const char *BreakWall::s_name = "BreakWall";

/* The cracked model's part callback: creates a part target for the part's clump and calls its
 * fn_80167448 with -1 (name inferred). */
static UnkModelPart_8015FA78 *CrackedPartCallback(UnkModelPart_8015FA78 *part, void *userData) {
    if (part->clump != 0) {
        boost::shared_ptr<UnkTarget_80166AF4> target = fn_8016690C(part->clump);

        target->fn_80167448(-1);
    }
    return part;
}

BreakWall::BreakWall(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_modelIndex(0) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_modelIndex, sizeof(m_modelIndex));

    m_charInfo = fn_8015CD90(0x23, 4);
    m_charInfo->m_commandUnit = boost::shared_ptr<CharCommandUnit>(new CharCommandUnit);
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&BreakWall::OnCommand, this, _1));

    m_loadedModelIndex = m_modelIndex;
    m_model = fn_8015F800(1, s_modelFile);
    m_resource = static_cast<UnkResource_80012700 *>(fn_80012700(fn_800122D4(), 1, s_modelFile));
    if (!GadgetBuilder::fn_80208BB8(m_resource, m_modelIndex)) {
        fn_8015A794(this);
        m_flags |= 1;
        return;
    } else {
        /* The original destroys weakAttack and attack before it calls ReadOwner: they live in
         * this block. */
        m_crashInfo = GadgetBuilder::fn_80208978(m_resource, m_modelIndex);
        m_model->fn_801629D8(m_crashInfo->fn_802087D8(), 0);

        const char *nodeName = m_crashInfo->fn_802087BC();

        m_model->fn_80161F08(nodeName, boost::weak_ptr<CharInfo>(m_charInfo));

        boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161334(nodeName);
        if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
            attack->field_0x54 = 0;
            attack->field_0x55 = 1;
            attack->field_0x1C.field_0x0 =
                AttackCallback(boost::bind(&BreakWall::OnAttack, this, _1));
        }
    }

    ReadOwner(owner);
    UpdateModelMatrix();
}

BreakWall::~BreakWall() {}

void BreakWall::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }

    switch (m_state) {
    case 0:
        if (m_model.get() != 0) {
            m_target = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;
            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        }
        if (m_target.get() != 0) {
            m_target->fn_80167010(-1);
        }
        m_state = 1;
        break;

    case 3:
        StartCrashEffect();
        if (boost::shared_ptr<CharInfo> breaker = m_breaker.lock()) {
            fn_8020642C(breaker, 0x23);
        }
        if (!ShowBrokenModel()) {
            fn_8015A794(this);
            m_flags |= 1;
        }
        break;
    }
}

void BreakWall::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    if (m_model.get() == 0) {
        return;
    }

    if (m_modelIndex != m_loadedModelIndex) {
        m_target.reset();
        m_model->fn_80161880();
        m_model->fn_80162804();
        m_crashInfo.reset();
        if (!GadgetBuilder::fn_80208BB8(m_resource, m_modelIndex)) {
            return;
        }

        m_crashInfo = GadgetBuilder::fn_80208978(m_resource, m_modelIndex);
        m_loadedModelIndex = m_modelIndex;
        m_model->fn_801629D8(m_crashInfo->fn_802087D8(), 0);

        const char *nodeName = m_crashInfo->fn_802087BC();

        m_model->fn_80161F08(nodeName, boost::weak_ptr<CharInfo>(m_charInfo));

        boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161334(nodeName);
        if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
            attack->field_0x54 = 0;
            attack->field_0x55 = 1;
            attack->field_0x1C.field_0x0 =
                AttackCallback(boost::bind(&BreakWall::OnAttack, this, _1));
        }
    }

    UpdateModelMatrix();
    m_model->fn_8016108C();
}

void BreakWall::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_modelIndex, sizeof(m_modelIndex));
    m_state = 0;
    m_health = 3.0f;
}

void BreakWall::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }
}

void BreakWall::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 11:
        if (m_state == 1) {
            ShowCrackedModel();
            m_state = 2;
        }
        break;
    case 10:
        if (m_state == 2) {
            m_breaker = static_cast<WallJumpCommand &>(command).field_0x14;
            m_state = 3;
        }
        break;
    }
}

void BreakWall::OnAttack(AttackCallbackParam &param) {
    if (m_state == 1 || m_state == 2) {
        boost::weak_ptr<CharInfo> attacker;

        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if ((attack->field_0x40 & 0x250) == 0) {
                param.field_0x54 = 5;
                return;
            }
            attacker = attack->m_owner;
        }

        m_health -= param.field_0x50;
        if (m_health <= 0.0f) {
            m_state = 3;
            param.field_0x54 = 3;
            m_breaker = attacker;
        } else {
            if (m_state == 1) {
                ShowCrackedModel();
                m_state = 2;
            }
            param.field_0x54 = 1;
        }
    } else {
        param.field_0x54 = 5;
    }
}

void BreakWall::StartCrashEffect() {
    if (m_crashInfo.get() != 0) {
        const UnkCrashEntry_802086C0 *entry = m_crashInfo->fn_802086C0();

        if (entry != 0) {
            RpClump *clump = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, entry->fileName));

            if (clump != 0) {
                UnkEffectParams_8015F0B8 parameters(m_charInfo, 0);

                parameters.field_0x4.x += entry->field_0x18.x;
                parameters.field_0x4.y += entry->field_0x18.y;
                parameters.field_0x4.z += entry->field_0x18.z;
                parameters.field_0x34 = 1.3f;
                parameters.field_0x44 = clump;
                parameters.field_0x54 = 1;
                parameters.field_0x4C = 150;
                fn_8015D93C(&parameters);
            }
        }

        const UnkResourceRecord_80208848 *record = m_crashInfo->fn_80208618();

        if (record != 0) {
            Fn8040Vector3 position = m_charInfo->m_position;
            position.x += record->field_0x10;
            position.y += record->field_0x14;
            position.z += record->field_0x18;
            fn_801D54F4(fn_80053164(), record->field_0x8, &position, 0, 1);
        }
    }
}

bool BreakWall::ShowBrokenModel() {
    if (m_crashInfo.get() != 0 && m_crashInfo->fn_802087A0() != 0) {
        m_target.reset();
        m_model->fn_80161880();
        m_model->fn_80162804();
        m_model->fn_801629D8(m_crashInfo->fn_802087A0(), 0);
        UpdateModelMatrix();
        if (m_crashInfo->fn_80208784() != 0) {
            m_model->fn_80161F08(m_crashInfo->fn_80208784(), boost::weak_ptr<CharInfo>(m_charInfo));
        }
        m_state = 4;
        return true;
    }
    return false;
}

bool BreakWall::ShowCrackedModel() {
    if (m_model.get() == 0) {
        return false;
    }

    m_target.reset();

    bool cracked = false;
    char name[64];

    sprintf(name, "%s%s", m_crashInfo->fn_802087D8(), s_crackSuffix);

    UnkResourceEntry_80208900 *entry = fn_80208900(m_resource, name);
    if (entry != 0) {
        m_model->fn_80162804();
        m_model->fn_80162A68(entry->field_0x8, 0);
        UpdateModelMatrix();
        m_model->fn_8015FA78(CrackedPartCallback, 0);
        cracked = true;
    }

    if (cracked) {
        UnkResourceRecord_80208848 *record = fn_80208888(m_resource, name);

        if (record != 0) {
            Fn8040Vector3 position = m_charInfo->m_position;
            position.x += record->field_0x10;
            position.y += record->field_0x14;
            position.z += record->field_0x18;
            fn_801D54F4(fn_80053164(), record->field_0x8, &position, 0, 1);
        }
    }
    return cracked;
}

} // namespace BreakWall

/* Uses the literal: the original takes the string's address here instead of loading s_modelFile. */
extern "C" void fn_801402D4(void) {
    fn_8006B474("BREAKWALL.BIN", 1);
}

extern "C" void fn_801402D0(void) {}

void BreakWall::BreakWall::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new BreakWall(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "model", 0, 0, 0x10000, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80529534 = {
    "BREAKWALL", fn_801402D4, fn_801402D0, BreakWall::BreakWall::Create, 0, 0, 0x1135, 1,
    0,           10,          s_paramInfo,
};
