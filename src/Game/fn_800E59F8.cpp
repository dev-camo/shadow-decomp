/* ArtWork::Ornament, from RTTI. Original source filename is unknown. */
#include "Game/Ornament.h"
#include "Game/CrashInfoCtrl.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_804157E8_motion_control.h"
#include "boost/weak_ptr.hpp"

const char *ArtWork::Ornament::s_name = "Ornament";

ArtWork::Ornament::Ornament(Task *parent, UnkGadgetOwner_800C9CD0 *owner, const char *fileName)
    : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_charInfo = fn_8015CD90(0x77, 4);
    field_0x50 = m_params.model;
    m_model = fn_8015F800(1, fileName);
    m_resource = static_cast<UnkResource_80012700 *>(fn_80012700(fn_800122D4(), 1, fileName));
    if (!GadgetBuilder::fn_80208BB8(m_resource, m_params.model)) {
        fn_8015A7EC(this);
        fn_8015A794(this);
        m_flags |= 1;
        return;
    }

    m_crashInfo = GadgetBuilder::fn_80208978(m_resource, m_params.model);
    m_model->fn_801629D8(m_crashInfo->fn_802087D8(), 0);
    const char *recordName = m_crashInfo->fn_802087D8();
    const char *secondaryName = m_crashInfo->fn_802087BC();
    if (m_model->fn_80160BA0(recordName)) {
        m_model->fn_8015FF60();
        m_model->fn_80160960(recordName, 1);
        field_0x88 = true;
        if (secondaryName != 0) {
            m_model->fn_80161AB8(secondaryName, boost::weak_ptr<CharInfo>(m_charInfo), recordName);
        }
    } else {
        field_0x88 = false;
        if (secondaryName != 0) {
            m_model->fn_80161F08(secondaryName, boost::weak_ptr<CharInfo>(m_charInfo));
        }
    }
    fn_800E5BB8(m_owner);
    fn_800E5B18();
    m_name = fn_801626D8(m_model.get());
}

ArtWork::Ornament::~Ornament() {
    m_target.reset();
    m_sound.fn_801D455C();
}

void ArtWork::Ornament::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }
    switch (m_state) {
    case 0: {
        m_state = 1;
        if (m_model.get() != 0) {
            m_target = fn_80166AF4();
            UnkWeakHolder_800CC6C8 target;
            target.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &target);
        }
        if (m_target.get() != 0) {
            m_target->fn_80167010(-1);
        }
        const char *recordName = m_crashInfo->fn_802087D8();
        UnkResourceRecord_80208848 *record = fn_80208888(m_resource, recordName);
        if (record != 0) {
            Fn8040Vector3 position(record->field_0x10, record->field_0x14, record->field_0x18);
            AddVector(&position, m_charInfo->m_position);
            m_sound = fn_801D3E58(static_cast<u16>(record->field_0x8), &position, this);
        }
        break;
    }
    case 1:
        if (field_0x88) {
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
    }
    fn_800E5B18();
}

void ArtWork::Ornament::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    int previousModel = m_params.model;
    fn_800E5BB8(owner);
    fn_800E5B18();
    if (m_model.get() != 0) {
        if (m_params.model != previousModel) {
            m_target.reset();
            m_sound.fn_801D455C();
            m_model->fn_80161880();
            m_model->fn_801604FC();
            m_model->fn_80162804();
            m_crashInfo.reset();
            if (!GadgetBuilder::fn_80208BB8(m_resource, m_params.model)) {
                return;
            }
            m_crashInfo = GadgetBuilder::fn_80208978(m_resource, m_params.model);
            field_0x50 = m_params.model;
            m_model->fn_801629D8(m_crashInfo->fn_802087D8(), 0);
            const char *recordName = m_crashInfo->fn_802087D8();
            const char *secondaryName = m_crashInfo->fn_802087BC();
            if (m_model->fn_80160BA0(recordName)) {
                m_model->fn_8015FF60();
                m_model->fn_80160960(recordName, 1);
                if (secondaryName != 0) {
                    m_model->fn_80161AB8(secondaryName, boost::weak_ptr<CharInfo>(m_charInfo),
                                         recordName);
                }
            } else if (secondaryName != 0) {
                m_model->fn_80161F08(secondaryName, boost::weak_ptr<CharInfo>(m_charInfo));
            }
        }
        m_model->fn_80162EF4();
        m_model->fn_8016108C();
    }
}

void ArtWork::Ornament::fn_800E5BB8(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_state = 0;
}

void ArtWork::Ornament::fn_800E5B18() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;
        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        fn_8040B720(&matrix, m_params.scaleX, m_params.scaleY, m_params.scaleZ);
        m_model->fn_801633DC(&matrix);
    }
}

extern "C" void fn_800E5AEC() {
    fn_8006B474("ARTISTWORK.BIN", 1);
}
extern "C" void fn_800E5AE8() {}
extern "C" void fn_800E5A88(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);
    new ArtWork::Ornament(parent, owner, "ARTISTWORK.BIN");
}
extern "C" void fn_800E5A5C() {
    fn_8006B474("ARTWORKZONE.BIN", 1);
}
extern "C" void fn_800E5A58() {}
extern "C" void fn_800E59F8(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);
    new ArtWork::Ornament(parent, owner, "ARTWORKZONE.BIN");
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "model", 0, 0xFFFF0000, 0x10000, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "scale X", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "scale Y", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "scale Z", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0, 0},
};
GadgetTypeInfo lbl_80524490 = {
    "ART WORK", fn_800E5AEC, fn_800E5AE8, fn_800E5A88, 0, 0, 0x2588, 1, 0, 10, s_paramInfo,
};
GadgetTypeInfo lbl_805244B4 = {
    "ART WORK ZONE", fn_800E5A5C, fn_800E5A58, fn_800E59F8, 0, 0, 0x2590, 1, 0, 10, s_paramInfo,
};
