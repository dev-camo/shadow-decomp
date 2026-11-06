/* The RAIL SWITCH gadget: RailSwitch::SetRailSwitch (behaviour in Game/RailSwitch.h). File name
 * inferred from the namespace. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first).
 *
 * The model attack object's callback is a boost::function object built with
 * boost::bind(&SetRailSwitch::OnAttack, this, _1); its manager, invoker and copy functions come
 * from the boost headers. */
#include "Game/RailSwitch.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkCharInfoTable_800DDF20.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkRailRoute_8054B8E0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

static const char *s_modelFile = "RAILSWITCH.BIN";

/* The colours of the material "mat": off (index 0) and on (index 1). */
static UnkDebugColor_802E0E84 s_colors[2] = {
    UnkDebugColor_802E0E84(0xFF, 0x00, 0x88, 0x62),
    UnkDebugColor_802E0E84(0xFF, 0x88, 0x01, 0x00),
};

namespace RailSwitch {

const char *SetRailSwitch::s_name = "SetRailSwitch";

SetRailSwitch::SetRailSwitch(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x82, 4);
    ReadOwner(owner);

    m_model = fn_8015F800(1, s_modelFile);
    m_model->fn_80162A68(0, 0);
    m_model->fn_80160C28(0);
    m_model->fn_8015FF60();
    m_model->fn_80161BD8(0, boost::weak_ptr<CharInfo>(m_charInfo), 0);

    boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161428(0);
    if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 =
            AttackCallback(boost::bind(&SetRailSwitch::OnAttack, this, _1));
    }

    UpdateModelMatrix();
    m_model->fn_8016239C("mat");
    m_material = m_model->fn_801621F0("mat");

    if (lbl_8054B8E0[m_params.route].field_0x8 == 1) {
        m_model->fn_80160A84(0, 2);
        m_material->color = s_colors[1];
        m_flags.Set(1);
        m_on = 1;
    } else {
        m_material->color = s_colors[0];
        m_flags.Reset(1);
        m_on = 0;
    }
}

SetRailSwitch::~SetRailSwitch() {}

void SetRailSwitch::UpdateBody(float dt) {
    if (m_flags.Test(0)) {
        m_soundTime -= dt;
        if (m_soundTime <= 0.0f) {
            fn_801D54F4(fn_80053164(), 0x7836, &m_charInfo->m_position, 0, 1);
            m_flags.Reset(0);
        }
    }

    if (m_flags.Test(1)) {
        fn_801D53C4(fn_80053164(), 0x7837, &m_charInfo->m_position, this, 3, 0, 2);
    }

    m_hitDelay -= dt;
    if (m_hitDelay < 0.0f) {
        m_hitDelay = 0.0f;
    }

    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    UpdateModelMatrix();
}

void SetRailSwitch::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
    }
}

void SetRailSwitch::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E50(owner, &m_comId);
    m_on = 0;
    m_hitDelay = 0.0f;
}

void SetRailSwitch::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8015CEA0(m_charInfo.get(), &matrix);
        m_model->fn_801633DC(&matrix);
    }
}

} // namespace RailSwitch

/* Matching workaround: the SwitchCommand constructor is defined here as an ordinary function. The
 * original's only copy is here, after OnAttack, its only caller, while the command's destructor
 * and vtable are elsewhere; it may be the out-of-line copy of an inline from the commands' header,
 * as other gadget files hold for the commands they send. Defined in the class, it is expanded
 * into OnAttack (0x278 bytes instead of 0x25C) and no copy is left. Revisit when the command
 * constructors are modelled in the headers. */
SwitchCommand::SwitchCommand(s32 route) : CharCommand(9, 4), m_route(route) {}

namespace RailSwitch {

void SetRailSwitch::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;
    if (m_hitDelay > 0.0f) {
        return;
    }

    if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
        if ((attack->field_0x40 & 0x3D1) == 0) {
            return;
        }

        if (param.field_0x50 < 0.9f) {
            param.field_0x54 = 5;
            m_hitDelay = 1.0f;
            return;
        }

        m_on = m_on ? 0 : 1;
        if (m_on == 1) {
            m_model->fn_80160A84(0, 2);
            m_material->color = s_colors[1];
            m_flags.Set(1);
        } else {
            m_model->fn_80160A84(0, 1);
            m_material->color = s_colors[0];
            m_flags.Reset(1);
        }

        fn_80169B80(9, &m_charInfo->m_position);
        fn_801D54F4(fn_80053164(), 0x7835, &m_charInfo->m_position, 0, 1);

        UnkCharInfoNode_801DCB14 *node = fn_801DCB14(fn_800DDF20(), 0x80);
        if (node != 0) {
            if (boost::shared_ptr<CharInfo> charInfo = node->charInfo.lock()) {
                charInfo->fn_801DC980(new SwitchCommand(m_params.route));
            }
        }

        param.field_0x54 = 1;
        m_soundTime = 0.2f;
        m_flags.Set(0);
        m_hitDelay = 1.0f;
    }
}

} // namespace RailSwitch

extern "C" void fn_80257A0C(void) {
    fn_8006B474("RAILSWITCH.BIN", 1);
}

extern "C" void fn_80257A08(void) {}

void RailSwitch::SetRailSwitch::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new SetRailSwitch(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "route", 0, 0, 0x10000, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80549E24 = {
    "RAIL SWITCH",
    fn_80257A0C,
    fn_80257A08,
    RailSwitch::SetRailSwitch::Create,
    0,
    0,
    0x13ED,
    1,
    0,
    10,
    s_paramInfo,
};
