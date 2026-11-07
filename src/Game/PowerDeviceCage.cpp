/* The POWER_DEVICE_CAGE gadget (classes described in Game/PowerDeviceCage.h). File name inferred
 * from the class. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first).
 *
 * The model's collision callback and the attack object's callback are boost::function objects
 * built with boost::bind(&PowerDeviceCage::method, this, _1); their managers, invokers and copy
 * functions come from the boost headers. */
/* Matching workaround: TNumEdit.h and CharaColliBaseCharInfo.h are included only for the RTTI
 * records that their never-called stand-in functions emit (see the notes there); no code in this
 * file uses them. TNumEdit.h comes first, in the order their names appear in the unit's .rodata
 * (alphabetical order would reverse them). Revisit when the original inline functions or their
 * real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/PowerDeviceCage.h"
#include "Game/CharInfo.h"
#include "Game/CharaColliAttack.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkManager_80009548.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkObject_802B4254.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* Values that the original builds at startup (names inferred from their uses): the height of
 * m_object above the character info and the value passed to its fn_802B4498. */
static UnkStartupFloat_800E2570 s_objectHeight(58.0f);
static UnkStartupFloat_800E2570 s_objectValue(200.0f);

const char *PowerDeviceCage::s_name = "PowerDeviceCage";

/* The callback that fn_8015AA84 calls for each hit whose collision object and character info still
 * lock and whose character info's fn_80078B4C returns true (placeholder name). context is the
 * gadget that OnCollision passes; when it is not null, sets bit 0 of its m_flags (behaviour
 * inferred). */
static void fn_800D3488(boost::shared_ptr<CharInfo> hit, void *context) {
    PowerDeviceCage *cage = static_cast<PowerDeviceCage *>(context);

    if (cage != 0) {
        cage->m_flags.Set(0);
    }
}

extern "C" void fn_800D345C(void) {
    fn_8006B474("POWERDEVICECAGE.BIN", 1);
}

extern "C" void fn_800D3458(void) {}

void PowerDeviceCage::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new PowerDeviceCage(fn_801EB304(fn_8001C4F8(), 11), owner);
}

PowerDeviceCage::PowerDeviceCage(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_state(0), m_flags(0) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x2B, 4);
    ReadOwner(owner);
    m_model = fn_8015F800(1, "POWERDEVICECAGE.BIN");
    m_model->fn_80162A68(m_params.model, 0);
    m_model->fn_80162424(0);
    m_model->fn_80162424(1);
    m_model->fn_80161F08("AllColli", boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->m_collisionCallback =
        ColliDetectCallback(boost::bind(&PowerDeviceCage::OnCollision, this, _1));

    {
        boost::shared_ptr<CharaColliAttack> attack = m_model->fn_80161334("AllColli").lock();

        if (attack.get() != 0) {
            attack->fn_80096B64(1,
                                AttackCallback(boost::bind(&PowerDeviceCage::OnAttack, this, _1)));
        }
    }

    UpdateModelMatrix();

    if (owner->m_adapterState == 0) {
        fn_800C9930(owner, new PowerDeviceCageSetState);
    }

    /* The original reads the flag here without a dynamic_cast (there is no __dynamic_cast call). */
    if (static_cast<PowerDeviceCageSetState *>(owner->m_adapterState)->m_triggered) {
        m_state = 1;
    } else {
        m_state = 0;
    }

    EnterState(m_state);
    UpdateModelMatrix();
}

PowerDeviceCage::~PowerDeviceCage() {
    m_state0Sound.fn_801D455C();
    m_state1Sound.fn_801D455C();
}

void PowerDeviceCage::UpdateBody(float dt) {
    switch (m_state) {
    case 0:
        if (m_flags.Test(0)) {
            ExitState(m_state);
            m_state = 1;
            EnterState(m_state);
            fn_8000D664(fn_80009548(), 0, 4.0f, 1.0f, 20.0f);
            /* As in the constructor, no dynamic_cast. */
            static_cast<PowerDeviceCageSetState *>(m_owner->m_adapterState)->m_triggered = true;
            fn_800C985C(m_owner);
            fn_8016907C(fn_800CB69C(), m_tag, 1);

            {
                boost::shared_ptr<UnkActor_800740E4> actor = fn_800740E4(fn_8006D6A8(), 0).lock();

                if (actor.get() != 0) {
                    actor->UnkSlot7(1, 0);
                }
            }

            m_flags.Reset(0);
        }
        break;

    /* Matching workaround: state 1 has no work of its own here, but the original's compare tree
     * tests it first (cmpwi 1); without this arm UpdateBody is 0xC bytes shorter, and with an if
     * on state 0 instead of the switch 0x10 bytes shorter. Revisit when the state 1 arm's body or
     * the state enumeration is found. */
    case 1:
        break;
    }

    UpdateModelMatrix();
    m_model->fn_80162FB0(dt);

    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }
}

void PowerDeviceCage::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    PowerDeviceCageParams previous = m_params;

    ReadOwner(owner);

    if (m_params.model != previous.model) {
        m_model->fn_80162804();
        m_model->fn_80162A68(m_params.model, 0);
        ExitState(m_state);
        EnterState(m_state);
    }

    UpdateModelMatrix();
    m_model->fn_8016108C();
    m_model->fn_80162EF4();
}

void PowerDeviceCage::ExitState(int state) {
    switch (state) {
    case 1:
        m_model->fn_801604FC();
        m_target.reset();
        m_state1Sound.fn_801D455C();
        break;

    case 0:
        m_state0Sound.fn_801D455C();
        break;
    }
}

void PowerDeviceCage::EnterState(int state) {
    switch (state) {
    case 1:
        m_model->fn_80160BA0("CoreMtn");
        m_model->fn_801607A8("CoreMtn");
        m_model->fn_8015FF60();
        m_model->fn_801609F8("CoreMtn");
        m_model->fn_80160C28(m_params.model);
        m_model->fn_8016082C(m_params.model);
        m_model->fn_8015FF60();
        m_model->fn_80160B18(m_params.model);
        m_target = fn_80166AF4();

        {
            UnkWeakHolder_800CC6C8 weakTarget;

            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
            m_target->fn_80167010(0);
        }

        m_state1Sound = fn_801D3E58(0x733A, &m_charInfo->m_position, this);
        m_object.reset();
        break;

    case 0: {
        m_state0Sound = fn_801D3E58(0x7338, &m_charInfo->m_position, this);
        m_object = fn_802B4254(0, 0);
        fn_802B4498(m_object.get(), s_objectValue.value);
        fn_802B4510(m_object.get(), 1);

        Fn8040Vector3 above(0.0f, s_objectHeight.value, 0.0f);

        const Fn8040Vector3 &objectPosition = above + m_charInfo->m_position;
        fn_802B44C8(m_object.get(), &objectPosition);
        break;
    }
    }
}

void PowerDeviceCage::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E60(owner, &m_tag);
}

void PowerDeviceCage::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }
}

void PowerDeviceCage::OnCollision(ColliDetectArgs &args) {
    if (m_state == 0) {
        fn_8015AA84(&args, fn_800D3488, this);
    }
}

void PowerDeviceCage::OnAttack(AttackCallbackParam &param) {
    if (fn_8015D4E4(&param)) {
        boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock();

        if (attack.get() != 0 && !(attack->field_0x40 & 2)) {
            fn_801D54F4(fn_80053164(), 0x733C, &m_charInfo->m_position, 0, 1);
        }
    }

    param.field_0x54 = 5;
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "MODEL 0:Out 1:In", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80523090 = {
    "POWER_DEVICE_CAGE", fn_800D345C, fn_800D3458, PowerDeviceCage::Create, 0, 0, 0x839, 1, 0, 10,
    s_paramInfo,
};
