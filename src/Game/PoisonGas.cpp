/* The PoisonGas gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The character info's command handler, the handle target's callback and the attack object's
 * callback are boost::function objects built with boost::bind(&PoisonGas::method, this, _1); their
 * managers, invokers and copy functions come from the boost headers. */
/* The RTTI data of TNumEdit.h comes before that of CharaColliBaseCharInfo.h at the start of the
 * unit's .data, .sdata and .rodata, so TNumEdit.h is included before CharaColliBaseCharInfo.h.
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit that RTTI
 * data; revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/PoisonGas.h"
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

/* What the hit callback receives (placeholder name): the gas's character info, which the callback
 * does not read, and, as output, whether the callback ran for any character info. */
struct UnkHitContext_80110974 {
    UnkHitContext_80110974() : m_hit(false) {}

    /* 0x00 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x08 */ bool m_hit;
};
CHECK_SIZEOF(UnkHitContext_80110974, 0xC);

/* The callback that fn_8015B004 calls for the character infos in the collision results that pass
 * its filter (their fn_80078B2C, vtable offset 0x18, returns true): when the context is not null,
 * passes the character info to fn_8020642C with 0x1C and records the hit. */
static void fn_80111E10(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_80110974 *context = static_cast<UnkHitContext_80110974 *>(arg);

    if (context != 0) {
        fn_8020642C(hit, 0x1C);
        context->m_hit = true;
    }
}

namespace PoisonGas {

const char *PoisonGas::s_name = "PoisonGas";

/* The value that the constructor stores in the Target's float at 0x20 (name inferred).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original initialiser
 * expression. The original sets the value with a start-up function in .ctors (a constant float
 * initialiser would be emitted as initialised .sdata data instead), and the shared stand-in
 * reproduces that function. The const qualifier lets the constructor load the value after the
 * out-of-line operator-> call, as the original does; with the non-const stand-in object the value
 * is loaded before that call and kept in f31 (0x438 bytes instead of 0x428). Revisit when the
 * original initialiser is found. */
static const float s_targetField20 = UnkStartupFloat_800E2570(40.0f).value;

PoisonGas::PoisonGas(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_scale(1.0f, 1.0f, 1.0f) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x19, 4, "PoisonGas");
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&PoisonGas::OnCommand, this, _1));
    ReadOwner(owner);
    m_effect = fn_8015D794(&m_charInfo->m_position, 0x8B);

    {
        boost::shared_ptr<CharaColliBase> probe = fn_800C7DEC(
            fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 0, 10, 0, &lbl_805E07A0, 10.0f);

        m_handle = fn_8015B8C4(probe);
        m_handle->field_0x0.fn_800F5980(
            GadgetColliDetectCallback(boost::bind(&PoisonGas::OnCollision, this, _1)));

        boost::weak_ptr<CharaColliAttack> attackWeak = fn_800C4374(fn_80021B08(), probe);

        if (boost::shared_ptr<CharaColliAttack> attack = attackWeak.lock()) {
            attack->field_0x54 = 0;
            attack->field_0x55 = 1;
            attack->field_0x1C.field_0x0 =
                AttackCallback(boost::bind(&PoisonGas::OnAttack, this, _1));
        }
    }

    m_target = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0x80);
    m_target->SetField20(s_targetField20);
    UpdatePositions();
}

PoisonGas::~PoisonGas() {
    fn_801E7108(m_target);
    StopEffect();
}

void PoisonGas::UpdateBody(float dt) {
    fn_8015BE9C(m_handle.get(), dt);

    switch (m_state) {
    case 0:
        m_state = 1;
        break;
    case 2:
        fn_8015D794(&m_charInfo->m_position, 0x29F);
        fn_8015A794(this);
        m_flags |= 1;
        break;
    }
}

void PoisonGas::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdatePositions();
}

void PoisonGas::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E60(owner, &m_comId);
    fn_800C9DBC(owner, &m_scale, sizeof(m_scale));
    m_state = 0;
}

void PoisonGas::UpdatePositions() {
    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

        fn_80425E4C(effect.get(), &m_charInfo->m_position);
    }

    if (m_target.get() != 0) {
        fn_800CCE38(m_target.get(), &m_charInfo->m_position);
    }
}

void PoisonGas::OnCollision(ColliDetectArgs &args) {
    UnkHitContext_80110974 context;

    context.m_charInfo = m_charInfo;
    fn_8015B004(&args, fn_80111E10, &context);

    if (context.m_hit) {
        m_state = 2;
    }
}

void PoisonGas::StopEffect() {
    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

        effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
    }
}

void PoisonGas::OnCommand(CharCommand &command) {
    if (m_state == 1 && command.m_type == 0x14) {
        m_state = 2;
    }
}

void PoisonGas::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;

    if (m_state == 1) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if ((attack->field_0x40 & 0x10) == 0) {
                /* Matching workaround: 5 is stored again although it was stored on entry, because
                 * the original's bytes contain this second store (USA (GUPE8P) 0x80110894); without
                 * it OnAttack is 44 words instead of 46. Revisit if the original form of this test
                 * is found. */
                param.field_0x54 = 5;
                return;
            }

            param.field_0x54 = 3;
            m_state = 2;
        }
    }
}

} // namespace PoisonGas

extern "C" void fn_80110818(void) {}

extern "C" void fn_80110814(void) {}

void PoisonGas::PoisonGas::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new PoisonGas(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "scale X", 0, 0, 0, 1.0f, 0.5f, 2.0f, fn_800CBC20, 0},
    {4, "scale Y", 0, 0, 0, 1.0f, 0.5f, 2.0f, fn_800CBC20, 0},
    {4, "scale Z", 0, 0, 0, 1.0f, 0.5f, 2.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_805269C4 = {
    "POISON_GAS", fn_80110818, fn_80110814, PoisonGas::PoisonGas::Create, 0, 0, 0x35, 1, 0,
    10,           s_paramInfo,
};
