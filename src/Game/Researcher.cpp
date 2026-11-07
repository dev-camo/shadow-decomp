/* Researcher (class name from RTTI; the gadget's descriptor is "Researcher"), with its character
 * info class ResearcherCharInfo. File name inferred from the class. Method names marked "(name
 * inferred)" in the header are not original. Definitions are in reverse address order (deferred
 * inlining emits functions last-defined first).
 *
 * The character info's command handler is a boost::function object built with
 * boost::bind(&Researcher::OnCommand, this, _1); its manager, invoker and copy functions come from
 * the boost headers. */
#include "Game/Researcher.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/MissionClearCollision.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_8006D6A8.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

/* The height of the second object above the character info and the value passed to its
 * fn_802B4498 (names inferred).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original type of these
 * variables. The original sets them with a start-up function in .ctors (a constant float
 * initialiser would be emitted as initialised .sdata data instead), and the shared stand-in
 * reproduces that function. Revisit when the original wrapper type is found. */
static UnkStartupFloat_800E2570 s_object2Height(10.0f);
static UnkStartupFloat_800E2570 s_object2Value(200.0f);

const char *Researcher::s_name = "Researcher";

/* Builds the gadget's character info, as fn_8015CD90 builds a plain GadgetCharInfo: arg0 goes to
 * the constructor and arg1 is stored at 0x5C (name inferred). */
static boost::shared_ptr<GadgetCharInfo> CreateCharInfo(int arg0, int arg1) {
    boost::shared_ptr<GadgetCharInfo> info =
        boost::shared_ptr<GadgetCharInfo>(new ResearcherCharInfo(arg0));

    info->SetField0x5C(arg1);
    return info;
}

Researcher::Researcher(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_state(0) {
    m_name = s_name;
    m_charInfo = CreateCharInfo(0x5C, 4);
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&Researcher::OnCommand, this, _1));
    ReadOwner(owner);
    m_model = fn_8015F800(1, "RESEARCHER.BIN");
    m_object = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0xA000);

    m_object2 = fn_802B4254(2, 0);
    fn_802B4498(m_object2.get(), s_object2Value.value);
    fn_802B4510(m_object2.get(), 1);

    Fn8040Vector3 above(0.0f, s_object2Height.value, 0.0f);
    const Fn8040Vector3 &position = m_charInfo->m_position + above;

    fn_802B44C8(m_object2.get(), &position);

    SetupModel();
}

Researcher::~Researcher() {}

extern "C" void fn_80318DB4(void) {
    fn_8006B474("RESEARCHER.BIN", 1);
}

extern "C" void fn_80318DB0(void) {}

void Researcher::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new Researcher(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void Researcher::UpdateBody(float dt) {
    switch (m_state) {
    case 1:
        m_timer -= dt;
        if (m_timer < 0.0f) {
            m_timer = 2.0f;
            m_state = 2;
        }
        break;

    case 2:
        m_timer -= dt;
        if (m_timer < 0.0f) {
            m_model->fn_80161880();
            m_model->fn_801604FC();
            m_model->fn_80162804();
            m_timer = 0.5f;
            m_state = 3;
        } else {
            fn_8015FDD8(m_model.get(), m_timer / 2.0f);
        }
        break;

    case 3:
        m_timer -= dt;
        if (m_timer < 0.0f) {
            m_target->fn_80166F88();
            m_flags |= 1;
            return;
        }
        break;
    }

    UpdateModelMatrix();
    m_model->fn_80162FB0(dt);
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }
}

void Researcher::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ResearcherParams previous = m_params;

    ReadOwner(owner);

    if (m_params.type != previous.type) {
        m_model->fn_80162804();
        m_model->fn_80161880();
        m_model->fn_801604FC();
        m_state = 0;
        m_timer = 0.0f;
        SetupModel();
    }

    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();
}

void Researcher::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_timer = 0.0f;
}

void Researcher::SetupModel() {
    m_model->fn_80162A68(m_params.type, 1);
    m_model->fn_80160C28(m_params.type);
    m_model->fn_8015FF60();
    m_model->fn_80161BD8(0, boost::weak_ptr<CharInfo>(m_charInfo), m_params.type);
    m_model->fn_80160A84(m_params.type, 1);
    UpdateModelMatrix();

    m_target = fn_80166AF4();

    UnkWeakHolder_800CC6C8 weakTarget;
    weakTarget.ptr = m_target;
    m_model->fn_8015FA78(fn_80169D00, &weakTarget);
}

void Researcher::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    m_model->fn_801633DC(&matrix);
    fn_800CCE38(m_object.get(), m_charInfo->m_position);
}

void Researcher::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 16:
        if (m_state == 0) {
            int missionId = 2;
            if (lbl_8057D6A0.field_0x125 != 0) {
                missionId = 1;
            }

            {
                boost::shared_ptr<Mission::Mission> mission =
                    fn_800740E4(fn_8006D6A8(), missionId).lock();

                if (mission.get() != 0) {
                    mission->fn_8016B224(1, 0);
                }
            }

            m_object2.reset();

            {
                boost::shared_ptr<CharInfo> character = fn_800A8C54(fn_80074194(), 0).lock();

                if (character.get() != 0) {
                    fn_8020642C(character, 0x28);
                }
            }

            m_model->fn_80160A84(m_params.type, 2);
            m_target->fn_80167448(-1);
            fn_801D54F4(fn_80053164(), 0x7634, &m_charInfo->m_position, 0, 1);
            fn_800C9B04(m_owner);
            m_timer = 0.5f;
            m_state = 1;
        }
        break;
    }

    command.m_flags |= 2;
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "Type 0:FaceUp 1:FaceDown", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8055C1F4 = {
    "Researcher", fn_80318DB4, fn_80318DB0, Researcher::Create, 0, 0, 0x1005, 1, 0, 10, s_paramInfo,
};
