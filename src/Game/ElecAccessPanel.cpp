/* ElecAccessPanel (class name from RTTI; the gadget's descriptor is "ELEC_ACCESS"): a panel of the
 * electric circuit area that shows ACCESS.BIN with an object (fn_802B4254) above it. When a
 * character hits it while the state of its com id is not 1, it sets that state to 1, switches
 * the model's parts and fades the colour of the model's "mat" atomic state from 0 to FF over half
 * a second; it then calls fn_8016B224(1, 0) on the mission that fn_800740E4 returns when it locks
 * to a non-null pointer, releases the object and keeps passing sound 0x7249 to fn_801D53C4. The
 * owner's ElecAccessPanelData keeps the state between creations (behaviour inferred). File name
 * inferred from the class. Method names marked "(name inferred)" are not original. Definitions are
 * in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The model's collision callback is a boost::function object built with
 * boost::bind(&ElecAccessPanel::OnCollision, this, _1); its manager, invoker and copy functions
 * come from the boost headers. */
#include "Game/ElecAccessPanel.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkModelAtomicState.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_80038E84.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

/* The two colours of the fade (names inferred). */
static UnkDebugColor_802E0E84 s_fadeColorStart(0x00, 0x00, 0x00, 0x00);
static UnkDebugColor_802E0E84 s_fadeColorEnd(0xFF, 0xFF, 0xFF, 0xFF);

const char *ElecAccessPanel::s_name = "ElecAccessPanel";

/* The part callback (placeholder name): calls fn_80038F94 on the part's clump when userData is
 * not null and fn_80038FC0 otherwise (behaviour inferred). */
static UnkModelPart_8015FA78 *fn_80248AE0(UnkModelPart_8015FA78 *part, void *userData);

/* The callback that fn_8015AA84 calls for each character info in the hits (placeholder name).
 * arg is the address of the panel pointer that OnCollision passes. Unless the state of the
 * panel's com id is 1, sets it to 1 and activates the panel (behaviour inferred). */
static void fn_802496C0(boost::shared_ptr<CharInfo> hit, void *arg) {
    ElecAccessPanel **panel = static_cast<ElecAccessPanel **>(arg);

    if (panel != 0) {
        if (fn_8016904C(fn_800CB69C(), (*panel)->m_comId) != 1) {
            fn_801690D0(fn_800CB69C(), (*panel)->m_comId, 1);
            (*panel)->Activate();
        }
    }
}

ElecAccessPanel::ElecAccessPanel(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x59, 4, "ElecAccessPanel");
    ReadOwner(owner);

    if (owner->m_adapterState == 0) {
        ElecAccessPanelData *data = new ElecAccessPanelData;

        fn_800C9930(owner, data);
        data->m_state = 0;
        m_state = 0;
    } else if (dynamic_cast<ElecAccessPanelData *>(m_owner->m_adapterState)->m_state == 3) {
        fn_801690D0(fn_800CB69C(), m_comId, 1);
    }

    m_model = new GadgetModelInfo(1, "ACCESS.BIN");
    m_model->fn_80162A68(0, 0);
    m_object = fn_802B4254(2, 0);
    fn_802B4498(m_object.get(), 1200.0f);
    fn_802B4510(m_object.get(), 1);
    m_model->fn_8016239C("mat"); /* applies the "mat" entry; the result is unused */

    ElecAccessPanelData *data = dynamic_cast<ElecAccessPanelData *>(m_owner->m_adapterState);

    if (fn_8016904C(fn_800CB69C(), m_comId) != 1) {
        m_state = 0;
        data->m_state = 0;
        m_model->fn_8015FA78(fn_80248AE0, 0);
        m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo));
        m_model->m_collisionCallback =
            ColliDetectCallback(boost::bind(&ElecAccessPanel::OnCollision, this, _1));
    } else {
        m_state = 3;
        data->m_state = 3;

        UnkModelAtomicState *state = m_model->fn_801621F0("mat");

        state->color.field_0x0 = 0xFF;
        state->color.field_0x1 = 0xFF;
        state->color.field_0x2 = 0xFF;
        state->color.field_0x3 = 0xFF;
        m_model->fn_8015FA78(fn_80248AE0, reinterpret_cast<void *>(1));
    }
}

ElecAccessPanel::~ElecAccessPanel() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

void ElecAccessPanel::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case 0:
        break;
    case 1:
        m_model->fn_80161880();
        m_state = 2;
        break;
    case 2:
        m_timer += dt;
        if (m_timer >= 0.5f) {
            m_timer = 0.5f;
            /* The original repeats the lookup in each branch (one lookup with the id chosen
             * first compiles to different code). */
            if (lbl_8057D6A0.field_0x125 != 0) {
                boost::shared_ptr<Mission::Mission> mission = fn_800740E4(fn_8006D6A8(), 1).lock();

                if (mission.get() != 0) {
                    mission->fn_8016B224(1, 0);
                }
            } else {
                boost::shared_ptr<Mission::Mission> mission = fn_800740E4(fn_8006D6A8(), 2).lock();

                if (mission.get() != 0) {
                    mission->fn_8016B224(1, 0);
                }
            }
            m_object.reset();
            m_state = 3;
        }
        fn_80154C6C(m_model->fn_801621F0("mat"), m_timer, s_fadeColorStart, s_fadeColorEnd, 0.0f,
                    0.5f);
        break;
    case 3:
        fn_801D53C4(fn_80053164(), 0x7249, &m_charInfo->m_position, this, 3, 0, 5);
        break;
    }

    UpdateModelMatrix();
}

void ElecAccessPanel::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
}

void ElecAccessPanel::fn_800CBC1C(float) {
    UpdateModelMatrix();
    if (m_model != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();
    }
}

void ElecAccessPanel::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix(fn_8040B034(m_charInfo->m_position));
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }

    if (m_object.get() != 0) {
        Fn8040Vector3 above(0.0f, 34.0f, 0.0f);

        const Fn8040Vector3 &position = above + m_charInfo->m_position;
        fn_802B44C8(m_object.get(), &position);
    }
}

void ElecAccessPanel::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E60(owner, &m_comId);
}

static UnkModelPart_8015FA78 *fn_80248AE0(UnkModelPart_8015FA78 *part, void *userData) {
    if (userData != 0) {
        fn_80038F94(part->clump);
    } else {
        fn_80038FC0(part->clump);
    }
    return part;
}

void ElecAccessPanel::Activate() {
    m_state = 1;
    m_timer = 0.0f;
    m_model->fn_8015FA78(fn_80248AE0, reinterpret_cast<void *>(1));

    UnkModelAtomicState *state = m_model->fn_801621F0("mat");

    state->color.field_0x0 = 0;
    state->color.field_0x1 = 0;
    state->color.field_0x2 = 0;
    state->color.field_0x3 = 0;

    ElecAccessPanelData *data = dynamic_cast<ElecAccessPanelData *>(m_owner->m_adapterState);

    data->m_state = 3;
    fn_800C985C(m_owner);
}

void ElecAccessPanel::OnCollision(ColliDetectArgs &args) {
    ElecAccessPanel *panel = this;

    fn_8015AA84(&args, fn_802496C0, &panel);
}

extern "C" void fn_802489E4(void) {
    fn_8006B474("ACCESS.BIN", 1);
}

extern "C" void fn_802489E0(void) {}

void ElecAccessPanel::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecAccessPanel(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_80548B70 = {
    "ELEC_ACCESS", fn_802489E4, fn_802489E0, ElecAccessPanel::Create, 0, 0, 0x7DD, 1, 0, 10, 0,
};
