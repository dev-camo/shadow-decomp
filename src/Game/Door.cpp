/* The DOOR and MORPHDOOR gadgets: Door::SetDoor and Door::SetMorphDoor (behaviour in the class
 * comments). File name inferred from the namespace. Definitions are in reverse address order
 * (deferred inlining emits functions last-defined first).
 *
 * SetDoor binds OnCommand to its character info's command unit with
 * boost::bind(&SetDoor::OnCommand, this, _1), and SetMorphDoor binds OnDraw to its draw layer with
 * boost::bind(&SetMorphDoor::OnDraw, this, _1); their managers, invokers and copy functions come
 * from the boost headers. */
#include <cstring>
#include "Game/Door.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_80037A80.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_800E9CEC.h"
#include "Game/fn_80109AF8.h"
#include "Game/fn_8040_vector_types.h"
#include "rw/rpworld.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::CharaColliBase;
using SonicteamUSA::System::Collision::CollisionResultList;

namespace Door {

const char *SetDoor::s_name = "SetDoor";

SetDoor::SetDoor(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x22, 4);
}

SetDoor::~SetDoor() {
    if (m_probe.get() != 0) {
        fn_800C78B0(fn_80021B08(), &m_probe);
    }
}

void SetDoor::Init() {
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&SetDoor::OnCommand, this, _1));
    ReadOwner(m_owner);

    m_resource = static_cast<UnkResource_80012700 *>(fn_80012700(fn_800122D4(), 1, "DOOR.BIN"));
    m_modelIndex = m_params.model;
    m_model = fn_8015F800(1, "DOOR.BIN");
    m_model->fn_80162A68(m_modelIndex, 1);
    m_model->fn_80160C28(m_modelIndex);
    m_model->fn_8015FF60();
    m_model->fn_80161BD8(m_modelIndex, boost::weak_ptr<CharInfo>(m_charInfo), m_modelIndex);

    Fn8040Vector3 center(m_params.x, m_params.y + m_params.height, m_params.z);
    Fn8040Vector3 size(m_params.width, m_params.height, m_params.depth);

    m_probe = fn_800C7C34(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), size, 0x40000, 10,
                          0, &center, &lbl_805E07B8, 0);
    ReadOwner(m_owner);
    UpdateModelMatrix();
}

void SetDoor::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }

    switch (m_state) {
    case 0:
        if (m_params.type == 1) {
            SetState(1);
        } else {
            SetState(2);
        }

        m_target.reset();
        m_target = fn_80166AF4();
        {
            UnkWeakHolder_800CC6C8 weakTarget;

            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        }
        if (m_target.get() != 0) {
            /* 0x1F5-0x1F8 are entry selectors for fn_80167010 and fn_80167448 (meaning unknown;
             * Fan compares a global with 0x1F5 as well). */
            if (m_state == 1) {
                m_target->fn_80167010(0x1F6);
            } else {
                m_target->fn_80167010(0x1F5);
            }
        }
        break;

    case 1:
        if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
            SetState(4);
            PlayMotion(1);
            m_target->fn_80166D14();
            m_target->fn_80167010(0x1F5);
            m_target->fn_80167448(0x1F8);
        }
        break;

    case 2:
        if (IsTouched()) {
            SetState(5);
            PlayMotion(1);
            PlayStateSound(1);
            m_target->fn_80167448(0x1F8);
        }
        break;

    case 5:
        if (IsMotionFinished()) {
            SetState(3);
            PlayStateSound(3);
        }
        break;

    case 3:
        if (!IsTouched()) {
            SetState(6);
            PlayMotion(2);
            PlayStateSound(4);
            m_target->fn_80167448(0x1F7);
        }
        break;

    case 6:
        if (IsMotionFinished()) {
            PlayStateSound(2);
            SetState(2);
        }
        break;
    }

    UpdateModelMatrix();
}

void SetDoor::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        if (m_params.model != m_modelIndex) {
            int count = fn_8015FF44(m_model.get());

            m_modelIndex = m_params.model;
            if (m_modelIndex < 0) {
                m_modelIndex = 0;
            }
            if (m_modelIndex > count - 1) {
                m_modelIndex = count - 1;
            }
            ChangeModel(m_modelIndex);
            m_params.model = m_modelIndex;
            fn_800C9D68(owner, &m_params, sizeof(m_params));
        }
        m_model->fn_80162EF4();
        m_model->fn_8016108C();
    }

    if (m_probe.get() != 0) {
        Fn8040Vector3 center(m_params.x, m_params.y + m_params.height, m_params.z);

        m_probe->UnkSlot17(&center, -2);
        m_probe->SetSize(m_params.width, -2, m_params.height, m_params.depth);
        m_probe->UnkSlot26();
    }
}

void SetDoor::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E60(owner, &m_comId);
    m_state = 0;
}

void SetDoor::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }
}

bool SetDoor::IsTouched() {
    if (m_probe.get() != 0) {
        CollisionResultList *hits = m_probe->GetHits();

        if (hits != 0 && hits->field_0x4 > 0) {
            ColliDetectArgs args;

            args.m_hits = hits;
            return fn_8015AC84(args);
        }
    }
    return false;
}

void SetDoor::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x17: {
        SetTransparentCommand &transparent = static_cast<SetTransparentCommand &>(command);

        if (transparent.m_alpha < 1.0f) {
            if (m_model.get() != 0) {
                fn_8015FDD8(m_model.get(), transparent.m_alpha);

                boost::shared_ptr<CharaColliBase> probe = m_model->fn_80161614(m_modelIndex).lock();

                if (probe.get() != 0) {
                    int flags = probe->UnkSlot10(0);

                    flags |= s_transparentFlag;
                    probe->UnkSlot20(flags, -1);
                }
            }
        } else if (m_model.get() != 0) {
            fn_8015FD10(m_model.get());

            boost::shared_ptr<CharaColliBase> probe = m_model->fn_80161614(m_modelIndex).lock();

            if (probe.get() != 0) {
                int flags = probe->UnkSlot10(0);

                probe->UnkSlot20(flags & ~s_transparentFlag, -1);
            }
        }
        break;
    }
    }
    command.m_flags |= 2;
}

/* Each sound repeats the record lookup with its own name buffer, as the original does: its frame
 * holds four 64-byte buffers, and one helper for the lookup stays out of line at this inline
 * level. */
void SetDoor::PlayStateSound(int sound) {
    switch (sound) {
    case 1: {
        const char *modelName = fn_801626D8(m_model.get());
        char name[64];

        memset(name, 0, sizeof(name));
        strcpy(name, modelName);
        strcat(name, "_open");

        UnkResourceRecord_80208848 *record = fn_80208888(m_resource, name);
        if (record != 0) {
            Fn8040Vector3 position = m_charInfo->m_position;
            position.x += record->field_0x10;
            position.y += record->field_0x14;
            position.z += record->field_0x18;
            fn_801D54F4(fn_80053164(), record->field_0x8, &position, 0, 1);
        }
        break;
    }
    case 2: {
        const char *modelName = fn_801626D8(m_model.get());
        char name[64];

        memset(name, 0, sizeof(name));
        strcpy(name, modelName);
        strcat(name, "_close");

        UnkResourceRecord_80208848 *record = fn_80208888(m_resource, name);
        if (record != 0) {
            Fn8040Vector3 position = m_charInfo->m_position;
            position.x += record->field_0x10;
            position.y += record->field_0x14;
            position.z += record->field_0x18;
            fn_801D54F4(fn_80053164(), record->field_0x8, &position, 0, 1);
        }
        break;
    }
    case 3: {
        const char *modelName = fn_801626D8(m_model.get());
        char name[64];

        memset(name, 0, sizeof(name));
        strcpy(name, modelName);
        strcat(name, "_openend");

        UnkResourceRecord_80208848 *record = fn_80208888(m_resource, name);
        if (record != 0) {
            Fn8040Vector3 position = m_charInfo->m_position;
            position.x += record->field_0x10;
            position.y += record->field_0x14;
            position.z += record->field_0x18;
            fn_801D54F4(fn_80053164(), record->field_0x8, &position, 0, 1);
        }
        break;
    }
    case 4: {
        const char *modelName = fn_801626D8(m_model.get());
        char name[64];

        memset(name, 0, sizeof(name));
        strcpy(name, modelName);
        strcat(name, "_closestart");

        UnkResourceRecord_80208848 *record = fn_80208888(m_resource, name);
        if (record != 0) {
            Fn8040Vector3 position = m_charInfo->m_position;
            position.x += record->field_0x10;
            position.y += record->field_0x14;
            position.z += record->field_0x18;
            fn_801D54F4(fn_80053164(), record->field_0x8, &position, 0, 1);
        }
        break;
    }
    }
}

void SetDoor::SetState(int state) {
    m_state = state;
}

void SetDoor::PlayMotion(int motion) {
    m_model->fn_80160A84(m_modelIndex, motion);
}

bool SetDoor::IsMotionFinished() {
    return m_model->fn_80160040(m_modelIndex);
}

void SetDoor::ChangeModel(int index) {
    m_model->fn_80161880();
    m_model->fn_801604FC();
    m_model->fn_80162804();
    m_model->fn_80162A68(index, 1);
    m_model->fn_80160C28(index);
    m_model->fn_8015FF60();
    m_model->fn_80161BD8(index, boost::weak_ptr<CharInfo>(m_charInfo), index);
}

} // namespace Door

extern "C" void fn_80143FE0(void) {
    fn_8006B474("DOOR.BIN", 1);
}

extern "C" void fn_80143FDC(void) {}

void Door::SetDoor::Create(UnkGadgetOwner_800C9CD0 *owner) {
    SetDoor *door = new SetDoor(fn_801EB304(fn_8001C4F8(), 11), owner);

    door->Init();
}

/* The names of the values of the "type" parameter. */
static const char *s_typeLabels[2] = {"type : normal", "type : key"};

/* The label callback of the descriptor's first parameter (behaviour inferred). */
static const char *fn_80143F54(UnkParamCursor_800CBC20 *cursor) {
    if (cursor->m_index == 0) {
        return s_typeLabels[cursor->m_value];
    }

    return 0;
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "type", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_80143F54, 0},
    {1, "model", 0, 0, 0x100, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "width", 0, 0, 0, 100.0f, 1.0f, 10000.0f, fn_800CBC20, 0},
    {4, "height", 0, 0, 0, 100.0f, 1.0f, 10000.0f, fn_800CBC20, 0},
    {4, "depth", 0, 0, 0, 100.0f, 1.0f, 10000.0f, fn_800CBC20, 0},
    {4, "X", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "Y", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "Z", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_805299D4 = {
    "DOOR", fn_80143FE0, fn_80143FDC, Door::SetDoor::Create, 0, 0, 0x1133, 1, 0, 10, s_paramInfo,
};

namespace Door {

SetMorphDoor::SetMorphDoor(Task *parent, UnkGadgetOwner_800C9CD0 *owner, const char *clumpFile,
                           const char *dmaFile, float morphTime, float drawRadius)
    : SetDoor(parent, owner), m_clumpFile(clumpFile), m_dmaFile(dmaFile), m_morphTime(morphTime),
      m_drawRadius(drawRadius), m_clump(0), m_motion(2), m_motionFinished(1), m_morphTimer(0.0f) {}

SetMorphDoor::~SetMorphDoor() {}

void SetMorphDoor::Init() {
    ReadOwner(m_owner);

    m_resource =
        static_cast<UnkResource_80012700 *>(fn_80012700(fn_800122D4(), 1, "MORPHDOOR.BIN"));
    m_modelIndex = m_params.model;

    /* fn_80037A80 hands the .DMA resource to each atomic of the clump. The resource loader returns
     * the clump of the .DFF file as an opaque pointer (type inferred from its uses). */
    m_clump = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, m_clumpFile));

    void *dmaResource = fn_80012700(fn_800122D4(), 1, m_dmaFile);

    fn_80037A80(m_clump, dmaResource);
    m_layer = fn_8004DFB0(fn_80046178()->field_0x8, "MorphDoor");
    m_layer->m_drawCallback = RenderDrawCallback(boost::bind(&SetMorphDoor::OnDraw, this, _1));

    m_model = fn_8015F800(1, "MORPHDOOR.BIN");
    m_model->fn_80161FC8(m_modelIndex, boost::weak_ptr<CharInfo>(m_charInfo));

    Fn8040Vector3 center(m_params.x, m_params.y + m_params.height, m_params.z);
    Fn8040Vector3 size(m_params.width, m_params.height, m_params.depth);

    m_probe = fn_800C7C34(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), size, 0x40000, 10,
                          0, &center, &lbl_805E07B8, 0);
    ReadOwner(m_owner);
    UpdateModelMatrix();
}

void SetMorphDoor::UpdateBody(float dt) {
    if (m_motionFinished == 0) {
        if (m_motion == 1) {
            m_morphTimer += dt;

            float fraction = m_morphTimer / m_morphTime;

            fn_800379E8(m_clump, fraction);
            if (fraction > 1.0f) {
                m_motionFinished = 1;
            }
        } else if (m_motion == 2) {
            m_morphTimer -= dt;

            float fraction = m_morphTimer / m_morphTime;

            fn_800379E8(m_clump, fraction);
            if (fraction < 0.0f) {
                m_motionFinished = 1;
            }
        }
    }
    SetDoor::UpdateBody(dt);
}

void SetMorphDoor::OnDraw(RednerDrawArgs &args) {
    RwSphere sphere;

    sphere.radius = m_drawRadius;
    sphere.center.x = m_charInfo->m_position.x;
    sphere.center.y = m_charInfo->m_position.y;
    sphere.center.z = m_charInfo->m_position.z;

    if (fn_804856C8(lbl_805F15B4->curCamera, &sphere)) {
        RwFrame *frame = static_cast<RwFrame *>(m_clump->object.parent);

        fn_80109AF8(frame, rwCOMBINEREPLACE, m_charInfo->m_rotation.z);
        fn_800E9C64(frame, rwCOMBINEPOSTCONCAT, m_charInfo->m_rotation.x);
        fn_800E9CEC(frame, rwCOMBINEPOSTCONCAT, m_charInfo->m_rotation.y);
        fn_804875B0(frame, &sphere.center, rwCOMBINEPOSTCONCAT);
        fn_80456BC4(m_clump);
    }
}

void SetMorphDoor::PlayMotion(int motion) {
    m_motion = motion;
    m_motionFinished = 0;
}

bool SetMorphDoor::IsMotionFinished() {
    return m_motionFinished == 1;
}

void SetMorphDoor::SetState(int state) {
    switch (state) {
    case 5:
        m_model->fn_80161230(m_modelIndex, -1);
        break;
    case 2:
        m_model->fn_8016111C(m_modelIndex, -1);
        break;
    }
    m_state = state;
}

} // namespace Door

static const char *s_clumpFile = "PR0302DRAPE.DFF";
static const char *s_dmaFile = "PR0302DRAPE.DMA";

extern "C" void fn_80143504(void) {
    fn_8006B474("MORPHDOOR.BIN", 1);
    fn_8006B474(s_dmaFile, 0);
}

extern "C" void fn_80143500(void) {}

void Door::SetMorphDoor::Create(UnkGadgetOwner_800C9CD0 *owner) {
    SetMorphDoor *door = new SetMorphDoor(fn_801EB304(fn_8001C4F8(), 11), owner, s_clumpFile,
                                          s_dmaFile, 0.25f, 80.0f);

    door->Init();
}

GadgetTypeInfo lbl_80529A04 = {
    "MORPHDOOR", fn_80143504, fn_80143500, Door::SetMorphDoor::Create, 0, 0, 0xC89, 1,
    0,           10,          s_paramInfo,
};
