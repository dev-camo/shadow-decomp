/* BASwitch::BASwitch (class name from RTTI; the gadget's descriptor is "BA Switch") and the table
 * of switch records that it keeps (behaviour in Game/BASwitch.h). File name inferred from the
 * class. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first).
 *
 * The command handler and the base model attack object's callback are boost::function objects
 * built with boost::bind(&BASwitch::OnCommand, this, _1) and boost::bind(&BASwitch::OnAttack,
 * this, _1); their managers, invokers and copy functions come from the boost headers. */
#include "Game/BASwitch.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkManager_8006D6A8.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

static const char *s_modelFile = "BASWITCH.BIN";

/* The heights of the target object and of the second object above the character info (names
 * inferred).
 * Matching workaround: the original sets these floats with a start-up function in .ctors (a
 * constant initialiser would be emitted as initialised .sdata data instead), and that function
 * loads both values first, as it does for const floats. UnkStartupFloat_800E2570 stands in for
 * the original initialiser expression and gives the start-up function, and the const qualifier
 * gives that load order: without it, or with UnkStartupFloat_800E2570 objects as other
 * gadget files keep their start-up floats, it loads them after forming the table's address (4 of
 * its 19 words differ). Revisit when the original initialiser is found. */
static const float s_objectHeight = UnkStartupFloat_800E2570(10.0f).value;
static const float s_object2Height = UnkStartupFloat_800E2570(50.0f).value;

/* The table of switch records (name inferred). */
static UnkSwitchRecord_80301F10 s_switchRecords[4];

namespace BASwitch {

const char *BASwitch::s_name = "BASwitch";

BASwitch::BASwitch(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x8B, 4);
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&BASwitch::OnCommand, this, _1));
    CopyFromOwner(owner);
    SetupModels();

    m_object2 = fn_802B4254(0, 0);
    fn_802B4510(m_object2.get(), 1);
    m_object = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0x80);

    if (fn_80301E1C(m_comId)) {
        fn_801690D0(fn_800CB69C(), m_comId, 1);
        m_models[0]->fn_801608AC(0, 2);
        ShowState(2);
    } else {
        fn_801690D0(fn_800CB69C(), m_comId, 0);
        m_models[0]->fn_801608AC(0, 1);
        ShowState(1);
    }

    UpdateModelMatrix();
}

BASwitch::~BASwitch() {
    fn_801E7108(m_object);
}

void BASwitch::UpdateBody(float dt) {
    if (m_flags.Test(1) && m_models[0]->fn_80160040(0)) {
        fn_801D54F4(fn_80053164(), 0x793A, &m_charInfo->m_position, 0, 1);
        m_flags.Reset(1);
    }

    for (int i = 0; i < 3; i++) {
        if (m_models[i].get() != 0) {
            m_models[i]->fn_80162FB0(dt);
        }
    }
    for (int i = 0; i < 3; i++) {
        if (m_targets[i].get() != 0) {
            m_targets[i]->fn_8016761C(dt);
        }
    }

    m_recoveryTime -= dt;
    if (m_recoveryTime <= 0.0f) {
        m_recoveryTime = 0.0f;
        m_flags.Reset(0);
    }

    /* The original compares the state with 0 only (0x80302DC4); for any other state its branches
     * at 0x80302DCC and 0x80302DD0 both reach 0x80302DDC, the shape MWCC gives a switch with
     * further case labels above 0 whose bodies are empty. Nothing in the file sets a state other
     * than 0 and 1, so those labels are unknown and only case 0 is written (unit note in
     * configure.py). */
    switch (m_state) {
    case 0:
        m_state = 1;
        break;
    }

    UpdateModelMatrix();
}

void BASwitch::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();
}

void BASwitch::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E50(owner, &m_comId);
    m_state = 0;
    m_recoveryTime = 0.0f;
    m_flags.Clear();
}

void BASwitch::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);

    for (int i = 0; i < 3; i++) {
        if (m_models[i].get() != 0) {
            m_models[i]->fn_801633DC(&matrix);
        }
    }

    if (m_object2.get() != 0) {
        Fn8040Vector3 position;

        fn_800091EC(&position, &m_charInfo->m_position);
        position.y += s_object2Height;
        fn_802B44C8(m_object2.get(), &position);
    }

    if (m_object.get() != 0) {
        Fn8040Vector3 position;

        fn_800091EC(&position, &m_charInfo->m_position);
        position.y += s_objectHeight;
        fn_800CCE38(m_object.get(), &position);
    }
}

void BASwitch::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;
    if (m_recoveryTime > 0.0f) {
        return;
    }
    if (m_state != 1) {
        return;
    }

    if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
        if ((attack->field_0x40 & 0x3D1) == 0) {
            return;
        }
        m_recoveryTime = 2.0f;
        m_flags.Set(0);
        param.field_0x54 = 1;
        Toggle();
    }
}

void BASwitch::Toggle() {
    if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
        fn_801690D0(fn_800CB69C(), m_comId, 0);
        m_models[0]->fn_80160A84(0, 1);
        ShowState(1);

        boost::shared_ptr<UnkActor_800740E4> actor = fn_800740E4(fn_8006D6A8(), 0).lock();

        if (actor.get() != 0) {
            actor->UnkSlot7(-1, 0);
        }
        fn_80301D04(m_comId);
    } else {
        fn_801690D0(fn_800CB69C(), m_comId, 1);
        m_models[0]->fn_80160A84(0, 2);
        ShowState(2);

        boost::shared_ptr<UnkActor_800740E4> actor = fn_800740E4(fn_8006D6A8(), 0).lock();

        if (actor.get() != 0) {
            actor->UnkSlot7(1, 0);
        }
        m_flags.Set(1);
        fn_80301D90(m_comId);
    }

    fn_801D54F4(fn_80053164(), 0x7939, &m_charInfo->m_position, 0, 1);
}

void BASwitch::SetupModels() {
    m_models[0] = fn_8015F800(1, s_modelFile);
    m_models[0]->fn_80162A68(0, 0);
    m_models[1] = fn_8015F800(1, s_modelFile);
    m_models[1]->fn_801629D8("blue", 0);
    m_models[2] = fn_8015F800(1, s_modelFile);
    m_models[2]->fn_801629D8("red", 0);
    m_models[0]->fn_80160C28(0);
    m_models[0]->fn_8015FF60();
    m_models[0]->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo));

    boost::weak_ptr<CharaColliAttack> weakAttack = m_models[0]->fn_80161428(0);
    if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 = AttackCallback(boost::bind(&BASwitch::OnAttack, this, _1));
    }

    for (int i = 0; i < 3; i++) {
        if (m_models[i].get() != 0) {
            m_targets[i] = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;
            weakTarget.ptr = m_targets[i];
            m_models[i]->fn_8015FA78(fn_80169D00, &weakTarget);
        }
        if (m_targets[i].get() != 0) {
            m_targets[i]->fn_80167010(-1);
        }
    }
}

void BASwitch::ShowState(int state) {
    switch (state) {
    case 1:
        if (m_models[1].get() != 0) {
            m_models[1]->fn_8016271C();
        }
        if (m_models[2].get() != 0) {
            m_models[2]->fn_80162790();
        }
        if (m_targets[1].get() != 0) {
            m_targets[1]->fn_80166B70();
        }
        if (m_targets[2].get() != 0) {
            m_targets[2]->fn_80166C70();
        }
        break;
    case 2:
        if (m_models[2].get() != 0) {
            m_models[2]->fn_8016271C();
        }
        if (m_models[1].get() != 0) {
            m_models[1]->fn_80162790();
        }
        if (m_targets[2].get() != 0) {
            m_targets[2]->fn_80166B70();
        }
        if (m_targets[1].get() != 0) {
            m_targets[1]->fn_80166C70();
        }
        break;
    }
}

void BASwitch::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x14:
        if (!m_flags.Test(0)) {
            Toggle();
            m_recoveryTime = 2.0f;
            m_flags.Set(0);
        }
        break;
    }
    command.m_flags |= 2;
}

} // namespace BASwitch

extern "C" void fn_80301FC8(void) {
    fn_8006B474(s_modelFile, 1);
}

extern "C" void fn_80301FC4(void) {}

void BASwitch::BASwitch::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new BASwitch(parent, owner);
}

bool fn_80301F4C(s32 id) {
    return fn_80301E1C(id);
}

void fn_80301F2C(void) {
    fn_80301CD0();
}

UnkSwitchRecord_80301F10::UnkSwitchRecord_80301F10() {
    id = savedId = SWITCH_RECORD_FREE;
    on = savedOn = false;
}

void fn_80301ED4(void) {
    for (int i = 0; i < 4; i++) {
        s_switchRecords[i].id = s_switchRecords[i].savedId = SWITCH_RECORD_FREE;
        s_switchRecords[i].on = s_switchRecords[i].savedOn = false;
    }
}

void fn_80301EB4(void) {
    fn_80301ED4();
}

void fn_80301E94(void) {
    fn_80301ED4();
}

void fn_80301E60(void) {
    for (int i = 0; i < 4; i++) {
        s_switchRecords[i].id = s_switchRecords[i].savedId;
        s_switchRecords[i].on = s_switchRecords[i].savedOn;
    }
}

bool fn_80301E1C(s32 id) {
    for (int i = 0; i < 4; i++) {
        if (s_switchRecords[i].id == id) {
            return s_switchRecords[i].on;
        }
    }
    return false;
}

void fn_80301D90(s32 id) {
    for (int i = 0; i < 4; i++) {
        if (s_switchRecords[i].id == id) {
            s_switchRecords[i].id = id;
            s_switchRecords[i].on = true;
            return;
        }
    }
    for (int i = 0; i < 4; i++) {
        if (s_switchRecords[i].id == SWITCH_RECORD_FREE) {
            s_switchRecords[i].id = id;
            s_switchRecords[i].on = true;
            return;
        }
    }
}

void fn_80301D04(s32 id) {
    for (int i = 0; i < 4; i++) {
        if (s_switchRecords[i].id == id) {
            s_switchRecords[i].id = id;
            s_switchRecords[i].on = false;
            return;
        }
    }
    for (int i = 0; i < 4; i++) {
        if (s_switchRecords[i].id == SWITCH_RECORD_FREE) {
            s_switchRecords[i].id = id;
            s_switchRecords[i].on = false;
            return;
        }
    }
}

void fn_80301CD0(void) {
    for (int i = 0; i < 4; i++) {
        s_switchRecords[i].savedId = s_switchRecords[i].id;
        s_switchRecords[i].savedOn = s_switchRecords[i].on;
    }
}

int fn_80301CAC(void) {
    fn_80301ED4();
    return 1;
}

int fn_80301C88(void) {
    fn_80301E60();
    return 1;
}

int fn_80301C64(void) {
    fn_80301E94();
    return 1;
}

int fn_80301C40(void) {
    fn_80301EB4();
    return 1;
}

GadgetTypeInfo lbl_8055AD78 = {
    "BA Switch", fn_80301FC8, fn_80301FC4, BASwitch::BASwitch::Create, 0, 0, 0x1900, 1, 0, 10, 0,
};
