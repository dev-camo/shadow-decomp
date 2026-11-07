/* The Fire gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The character info's command handler and the attack object's two callbacks are boost::function
 * objects built with boost::bind(&Fire::method, this, _1); their managers, invokers and copy
 * functions come from the boost headers. */
/* The RTTI data of TNumEdit.h comes before that of CharaColliBaseCharInfo.h at the start of the
 * unit's .data, .sdata and .rodata, so TNumEdit.h is included before CharaColliBaseCharInfo.h.
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit that RTTI
 * data; revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/Fire.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWorld_80021B08.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;
using SonicteamUSA::System::CharaColli::CharaColliBase;

namespace Fire {

const char *Fire::s_name = "Fire";

/* The value that the constructor stores in the Target's float at 0x20 (name inferred).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original initialiser
 * expression. The original sets the value with a start-up function in .ctors (a constant float
 * initialiser would be emitted as initialised .sdata data instead), and the shared stand-in
 * reproduces that function. The const qualifier lets the constructor load the value after the
 * out-of-line operator-> call, as the original does; with the non-const stand-in object the value
 * is loaded before that call (0x44C bytes instead of 0x43C). Revisit when the original
 * initialiser is found. */
static const float s_targetField20 = UnkStartupFloat_800E2570(40.0f).value;

Fire::Fire(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_scale(1.0f, 1.0f, 1.0f) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x1A, 4, "Fire");
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&Fire::OnCommand, this, _1));
    ReadOwner(owner);
    m_effect = fn_8015D794(&m_charInfo->m_position, 0xA6);

    {
        boost::shared_ptr<CharaColliBase> probe = fn_800C7DEC(
            fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 0, 10, 0, &lbl_805E07A0, 10.0f);

        m_handle = fn_8015B8C4(probe);

        boost::weak_ptr<CharaColliAttack> attackWeak = fn_800C4374(fn_80021B08(), probe);

        if (boost::shared_ptr<CharaColliAttack> attack = attackWeak.lock()) {
            attack->field_0x54 = 1;
            attack->field_0x38 = 1;
            attack->field_0x34 = 1.0f;
            attack->field_0x10.field_0x0 =
                AttackCallback(boost::bind(&Fire::fn_80104040, this, _1));
            attack->field_0x55 = 1;
            attack->field_0x1C.field_0x0 = AttackCallback(boost::bind(&Fire::OnAttack, this, _1));
        }
    }

    m_target = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0x80);
    m_target->SetField20(s_targetField20);
    UpdatePositions();
}

Fire::~Fire() {
    fn_801E7108(m_target);
    StopEffect();
}

void Fire::UpdateBody(float dt) {
    fn_8015BE9C(m_handle.get(), dt);

    switch (m_state) {
    case 0:
        m_state = 1;
        break;
    case 2:
        fn_8015D794(&m_charInfo->m_position, 0x29C);
        fn_8015A794(this);
        m_flags |= 1;
        break;
    }
}

void Fire::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdatePositions();
}

void Fire::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_scale, sizeof(m_scale));
    m_state = 0;
}

void Fire::UpdatePositions() {
    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

        fn_80425E4C(effect.get(), &m_charInfo->m_position);
    }

    if (m_target.get() != 0) {
        fn_800CCE38(m_target.get(), m_charInfo->m_position);
    }
}

void Fire::StopEffect() {
    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

        effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
    }
}

void Fire::OnCommand(CharCommand &command) {
    if (m_state == 1 && command.m_type == 0x14) {
        m_state = 2;
    }
}

void Fire::OnAttack(AttackCallbackParam &param) {
    if (m_state == 1) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if ((attack->field_0x40 & 0x95) == 0) {
                param.field_0x54 = 5;
                return;
            }

            if (boost::shared_ptr<CharInfo> attacker = attack->m_owner.lock()) {
                fn_8020642C(attacker, 0x1B);
            }

            param.field_0x54 = 3;
            m_state = 2;
            return;
        }
    }

    param.field_0x54 = 5;
}

void Fire::fn_80104040(AttackCallbackParam &param) {
    if (m_state == 1) {
        m_state = 2;
    }
}

} // namespace Fire

extern "C" void fn_8010403C(void) {}

extern "C" void fn_80104038(void) {}

void Fire::Fire::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new Fire(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "scale X", 0, 0, 0, 1.0f, 0.5f, 2.0f, fn_800CBC20, 0},
    {4, "scale Y", 0, 0, 0, 1.0f, 0.5f, 2.0f, fn_800CBC20, 0},
    {4, "scale Z", 0, 0, 0, 1.0f, 0.5f, 2.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80525FA4 = {
    "FIRE", fn_8010403C, fn_80104038, Fire::Fire::Create, 0, 0, 0x34, 1, 0, 10, s_paramInfo,
};
