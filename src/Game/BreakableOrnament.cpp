/* The ArtWork::BreakableOrnament gadget (class described in Game/BreakableOrnament.h): the
 * descriptors "ART GADGET" (ARTISTGADGET.BIN) and "ART GADGET ZONE" (ARTGADGETZONE.BIN). File name
 * inferred from the class. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first).
 *
 * The model attack object's callback is a boost::function object built with
 * boost::bind(&BreakableOrnament::OnAttack, this, _1); its manager, invoker and copy functions
 * come from the boost headers. */
#include "Game/BreakableOrnament.h"
#include "Game/CharInfo.h"
#include "Game/CrashInfoCtrl.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_804157E8_motion_control.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

namespace ArtWork {

const char *BreakableOrnament::s_name = "BreakableOrnament";

BreakableOrnament::BreakableOrnament(Task *parent, UnkGadgetOwner_800C9CD0 *owner,
                                     const char *fileName)
    : Gadget(parent, owner), m_modelIndex(0) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_modelIndex, sizeof(m_modelIndex));
    m_charInfo = fn_8015CD90(0x76, 4);
    m_loadedModelIndex = m_modelIndex;
    m_model = fn_8015F800(1, fileName);
    m_resource = static_cast<UnkResource_80012700 *>(fn_80012700(fn_800122D4(), 1, fileName));
    if (!GadgetBuilder::fn_80208BB8(m_resource, m_modelIndex)) {
        fn_8015A7EC(this);
        fn_8015A794(this);
        m_flags |= 1;
        return;
    }

    m_crashInfo = GadgetBuilder::fn_80208978(m_resource, m_modelIndex);
    m_model->fn_801629D8(m_crashInfo->fn_802087D8(), 1);

    const char *recordName = m_crashInfo->fn_802087D8();
    const char *nodeName = m_crashInfo->fn_802087BC();
    bool attached = false;

    if (m_model->fn_80160BA0(recordName)) {
        m_model->fn_8015FF60();
        m_model->fn_80160960(recordName, 1);
        m_stateFlags.Set(0);
        if (m_model->fn_80161AB8(nodeName, boost::weak_ptr<CharInfo>(m_charInfo), recordName)) {
            attached = true;
        }
    } else {
        m_stateFlags.Reset(0);
        if (m_model->fn_80161F08(nodeName, boost::weak_ptr<CharInfo>(m_charInfo))) {
            attached = true;
        }
    }

    if (attached) {
        boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161334(nodeName);

        if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
            attack->field_0x54 = 0;
            attack->field_0x55 = 1;
            attack->field_0x1C.field_0x0 =
                AttackCallback(boost::bind(&BreakableOrnament::OnAttack, this, _1));
        }
    }

    ReadOwner(m_owner);
    UpdateModelMatrix();
    m_name = fn_801626D8(m_model.get());
    m_health = m_crashInfo->fn_802085FC();
    m_model->fn_8015FC20(-1.0f);
}

BreakableOrnament::~BreakableOrnament() {
    m_sound.fn_801D455C();
}

void BreakableOrnament::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }

    if (m_stateFlags.Test(2)) {
        s32 comId;

        fn_800C9E50(m_owner, &comId);
        if (fn_8016904C(fn_800CB69C(), comId) == 1) {
            if (!m_model->fn_801626F4()) {
                m_model->fn_80162790();
                if (m_target.get() != 0) {
                    m_target->fn_80166C70();
                }
            }
        } else if (m_model->fn_801626F4()) {
            m_model->fn_8016271C();
            if (m_target.get() != 0) {
                m_target->fn_80166B70();
            }
        }
    }

    switch (m_state) {
    case 0: {
        m_state = 1;
        if (m_model.get() != 0) {
            m_target = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;
            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        }
        if (m_target.get() != 0) {
            m_target->fn_80167010(-1);
        }

        UnkResourceRecord_80208848 *record = fn_80208888(m_resource, m_crashInfo->fn_802087D8());
        if (record != 0) {
            Fn8040Vector3 position(record->field_0x10, record->field_0x14, record->field_0x18);

            AddVector(&position, m_charInfo->m_position);
            m_sound = fn_801D3E58(static_cast<u16>(record->field_0x8), &position, this);
        }
        break;
    }

    case 1:
        if (m_stateFlags.Test(0)) {
            boost::shared_ptr<_stMtnPlayWork> motion =
                m_model->fn_801601A0(m_crashInfo->fn_802087D8()).lock();

            if (motion.get() != 0) {
                _stMtnPlayWork *control = motion.get();
                unsigned short eventId;
                void *payload;

                while (fn_80415854(control, 0, &eventId, &payload, 0)) {
                    if (eventId == 0x5001) {
                        u32 *sound = static_cast<u32 *>(payload);

                        fn_801D54F4(fn_80053164(), static_cast<u16>(*sound),
                                    &m_charInfo->m_position, 0, 1);
                    }
                    control = 0;
                }
            }
        }
        break;

    case 2:
        StartCrashEffect();
        m_target.reset();
        m_sound.fn_801D455C();
        if (!ShowBrokenModel()) {
            fn_8015A794(this);
            m_flags |= 1;
        }
        break;

    case 3: {
        boost::shared_ptr<_stMtnPlayWork> motion =
            m_model->fn_801601A0(m_crashInfo->fn_802087A0()).lock();

        if (motion.get() != 0) {
            _stMtnPlayWork *control = motion.get();
            unsigned short eventId;
            void *payload;

            while (fn_80415854(control, 0, &eventId, &payload, 0)) {
                if (eventId == 0x5001) {
                    u32 *sound = static_cast<u32 *>(payload);

                    fn_801D54F4(fn_80053164(), static_cast<u16>(*sound), &m_charInfo->m_position, 0,
                                1);
                }
                control = 0;
            }

            if (!stMtnControlWorkFlag(motion.get(), 0, 1, 1) &&
                stMtnControlWorkFlag(motion.get(), 0, 3, 1)) {
                m_model->fn_80160400(m_crashInfo->fn_802087A0());
                m_state = 4;
            }
        }
        break;
    }

    case 4:
        /* Matching workaround: the broken state (set by case 3 and ShowBrokenModel) has nothing
         * to do, but its empty case is listed: the original's switch compares with 4 and branches
         * to the end (cmpwi and beq at USA (GUPE8P) 0x800E4F84); without the case UpdateBody is
         * 266 words against 267. Revisit if the original's handling of this state is found. */
        break;
    }

    UpdateModelMatrix();
}

void BreakableOrnament::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    /* The model parameter before ReadOwner rereads it. With a model, a change drops the crash
     * information and loads it again only when the model file has crash information for the new
     * parameter. The original compares with this earlier value, not with m_loadedModelIndex. */
    s32 previousModelIndex = m_modelIndex;

    ReadOwner(owner);
    UpdateModelMatrix();
    if (m_model.get() == 0) {
        return;
    }

    if (m_modelIndex != previousModelIndex) {
        m_target.reset();
        m_sound.fn_801D455C();
        m_model->fn_80161880();
        m_model->fn_801604FC();
        m_model->fn_80162804();
        m_crashInfo.reset();
        if (!GadgetBuilder::fn_80208BB8(m_resource, m_modelIndex)) {
            return;
        }

        m_crashInfo = GadgetBuilder::fn_80208978(m_resource, m_modelIndex);
        m_loadedModelIndex = m_modelIndex;
        m_model->fn_801629D8(m_crashInfo->fn_802087D8(), 1);

        const char *recordName = m_crashInfo->fn_802087D8();
        const char *nodeName = m_crashInfo->fn_802087BC();
        bool attached = false;

        if (m_model->fn_80160BA0(recordName)) {
            m_model->fn_8015FF60();
            m_model->fn_80160960(recordName, 1);
            if (m_model->fn_80161AB8(nodeName, boost::weak_ptr<CharInfo>(m_charInfo), recordName)) {
                attached = true;
            }
        } else if (m_model->fn_80161F08(nodeName, boost::weak_ptr<CharInfo>(m_charInfo))) {
            attached = true;
        }

        if (attached) {
            boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161334(nodeName);

            if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
                attack->field_0x54 = 0;
                attack->field_0x55 = 1;
                attack->field_0x1C.field_0x0 =
                    AttackCallback(boost::bind(&BreakableOrnament::OnAttack, this, _1));
            }
        }

        m_health = m_crashInfo->fn_802085FC();
    }

    m_model->fn_80162EF4();
    m_model->fn_8016108C();
}

void BreakableOrnament::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_modelIndex, sizeof(m_modelIndex));
    m_state = 0;

    u32 ownerFlags;

    fn_800C9E70(owner, &ownerFlags);
    if (ownerFlags & 0x400000) {
        m_stateFlags.Set(1);
    } else {
        m_stateFlags.Reset(1);
    }

    s32 comId;

    fn_800C9E50(owner, &comId);
    if (comId > 0) {
        m_stateFlags.Set(2);
    } else {
        m_stateFlags.Reset(2);
    }
    m_stateFlags.Reset(3);
}

void BreakableOrnament::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        m_model->fn_801633DC(&matrix);
    }
}

void BreakableOrnament::OnAttack(AttackCallbackParam &param) {
    if (m_state == 1) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if ((attack->field_0x40 & 2) != 0) {
                param.field_0x54 = 5;
                return;
            }

            m_health -= param.field_0x50;
            if (m_health <= 0.0f) {
                param.field_0x54 = 3;
                m_state = 2;
                if (boost::shared_ptr<CharInfo> attacker = attack->m_owner.lock()) {
                    fn_8020642C(attacker, 0x23);
                }
            } else if (param.field_0x50 > 0.9f) {
                param.field_0x54 = 1;
                if (m_model.get() != 0) {
                    m_model->fn_8016268C(0.25f);
                }
            } else {
                param.field_0x54 = 5;
            }
            return;
        }
    }

    param.field_0x54 = 5;
}

void BreakableOrnament::StartCrashEffect() {
    const UnkRecord_802039C8 *record = &lbl_8057D6A0.field_0xA8;

    if (fn_802039E8(record) && !fn_802039C8(record)) {
        return;
    }

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
                parameters.field_0x4C = 90;
                if (m_stateFlags.Test(1)) {
                    parameters.field_0x40 = m_charInfo->m_position.y;
                    parameters.field_0x4A = 1;
                }
                fn_8015D93C(&parameters);
            }
        }

        const UnkResourceRecord_80208848 *soundRecord = m_crashInfo->fn_80208618();

        if (soundRecord != 0) {
            Fn8040Vector3 position = m_charInfo->m_position;

            position.x += soundRecord->field_0x10;
            position.y += soundRecord->field_0x14;
            position.z += soundRecord->field_0x18;
            fn_801D54F4(fn_80053164(), soundRecord->field_0x8, &position, 0, 1);
        }
    }
}

bool BreakableOrnament::ShowBrokenModel() {
    if (m_crashInfo.get() != 0 && m_crashInfo->fn_802087A0() != 0) {
        m_model->fn_80161880();
        m_model->fn_801604FC();
        m_model->fn_80162804();
        m_model->fn_801629D8(m_crashInfo->fn_802087A0(), 1);
        UpdateModelMatrix();
        if (m_model->fn_80160BA0(m_crashInfo->fn_802087A0())) {
            m_model->fn_8015FF60();
            m_model->fn_80161AB8(m_crashInfo->fn_80208784(), boost::weak_ptr<CharInfo>(m_charInfo),
                                 m_crashInfo->fn_802087A0());
            m_model->fn_801609F8(m_crashInfo->fn_802087A0());
            UpdateModelMatrix();
            m_state = 3;
        } else if (m_crashInfo->fn_80208784() != 0) {
            m_model->fn_80161F08(m_crashInfo->fn_80208784(), boost::weak_ptr<CharInfo>(m_charInfo));
            m_state = 4;
        } else {
            /* Matching workaround: state 4 is written out in this branch and in the one above,
             * as the original stores it twice (stw at USA (GUPE8P) 0x800E4214 and 0x800E4220);
             * one store after the two branches makes ShowBrokenModel 148 words against the
             * original's 151. Revisit if the original code of these branches is found. */
            m_state = 4;
        }

        if (m_model.get() != 0) {
            m_target = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;
            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        }
        if (m_target.get() != 0) {
            m_target->fn_80167010(-1);
        }

        UnkResourceRecord_80208848 *record = fn_80208888(m_resource, m_crashInfo->fn_802087A0());
        if (record != 0) {
            Fn8040Vector3 position(record->field_0x10, record->field_0x14, record->field_0x18);

            AddVector(&position, m_charInfo->m_position);
            m_sound = fn_801D3E58(static_cast<u16>(record->field_0x8), &position, this);
        }
        return true;
    }
    return false;
}

bool BreakableOrnament::IsFinished() {
    if (fn_8015A858(this)) {
        m_flags |= 1;
        return true;
    }

    if (!m_stateFlags.Test(3)) {
        if (fn_8015A818(this)) {
            m_stateFlags.Set(3);
            if (m_model.get() != 0) {
                m_model->fn_8015FC94(-1.0f);
            }
        }
    } else if (m_model.get() != 0 && m_model->fn_8015FBF8()) {
        m_flags |= 1;
        return true;
    }
    return false;
}

} // namespace ArtWork

extern "C" void fn_800E3FDC(void) {
    fn_8006B474("ARTISTGADGET.BIN", 1);
}

extern "C" void fn_800E3FD8(void) {}

extern "C" void fn_800E3F78(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new ArtWork::BreakableOrnament(parent, owner, "ARTISTGADGET.BIN");
}

extern "C" void fn_800E3F4C(void) {
    fn_8006B474("ARTGADGETZONE.BIN", 1);
}

extern "C" void fn_800E3F48(void) {}

extern "C" void fn_800E3EE8(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new ArtWork::BreakableOrnament(parent, owner, "ARTGADGETZONE.BIN");
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "model", 0, 0, 0x10000, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80524340 = {
    "ART GADGET", fn_800E3FDC, fn_800E3FD8, fn_800E3F78, 0, 0, 0x2589, 1, 0, 10, s_paramInfo,
};

GadgetTypeInfo lbl_80524364 = {
    "ART GADGET ZONE", fn_800E3F4C, fn_800E3F48, fn_800E3EE8, 0, 0, 0x2591, 1, 0, 10, s_paramInfo,
};
