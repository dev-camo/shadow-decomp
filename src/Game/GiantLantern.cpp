/* The GiantLantern gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The three callbacks of the probe's attack object are boost::function objects built with
 * boost::bind(&GiantLantern::..., this, _1); their manager, invoker and copy functions come from
 * the boost headers. */
/* TNumEdit.h comes before CharaColliBaseCharInfo.h, in the order their names appear in the unit's
 * .rodata (alphabetical order would reverse them). */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/GiantLantern.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SetAdapterState.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "layout_check.h"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The direction passed to fn_80168C48 with the effect's position (0, 0, -1). */
static Fn8040Vector3 s_effectDirection(0.0f, 0.0f, -1.0f);

/* The height of the object above the character info's position and the value passed to
 * its fn_802B4498 (names inferred from their uses).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original type of these
 * variables. The original sets them with a start-up function in .ctors (a constant float
 * initialiser would be emitted as initialised .sdata data instead), and the shared stand-in
 * reproduces those two float stores. The original start-up function (0x80237968, 20 words) also
 * builds and copies the effect direction through fn_800091EC; that part still differs (see the
 * unit's configure.py note). Revisit when the original wrapper type is found. */
static UnkStartupFloat_800E2570 s_objectHeight(13.0f);
static UnkStartupFloat_800E2570 s_objectValue(400.0f);

/* The state that the owner remembers between the creations of a giant lantern (class name from
 * RTTI, derived from SetAdapterState): bit 0 of the flags is set once the lantern has been lit
 * (behaviour inferred). */
class GiantLanternSetState : public SetAdapterState {
  public:
    GiantLanternSetState() {
        m_flags.Clear();
    }

    virtual ~GiantLanternSetState() {}

    /* 0x4 */ UnkBitFlags_80014E04 m_flags;
};
CHECK_SIZEOF(GiantLanternSetState, 0x8);

const char *GiantLantern::s_name = "GiantLantern";

GiantLantern::GiantLantern(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_flags(0) {
    m_name = s_name;
    m_effect.reset();

    if (owner->m_adapterState == 0) {
        fn_800C9930(owner, new GiantLanternSetState);
    }

    m_data = dynamic_cast<GiantLanternSetState *>(owner->m_adapterState);
    m_charInfo = fn_8015CD90(0x4D, 4);
    CopyFromOwner(owner);
    m_probe = fn_800C7DEC(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 0x40100, 7, 0,
                          &lbl_805E07A0, 30.0f);

    {
        boost::shared_ptr<CharaColliAttack> attack(fn_800C4374(fn_80021B08(), m_probe).lock());

        if (attack.get() != 0) {
            attack->fn_80096B64(1, AttackCallback(boost::bind(&GiantLantern::OnAttack, this, _1)));
            attack->fn_80054DD0(
                1, 0x401, AttackCallback(boost::bind(&GiantLantern::fn_8023684C, this, _1)),
                AttackCallback(boost::bind(&GiantLantern::fn_80236848, this, _1)), 1.0f);
        }
    }

    if (m_data->m_flags.Test(0)) {
        m_effect = fn_80168C48(&m_charInfo->m_position, &s_effectDirection, 0x1E);
        m_sound = fn_801D3E58(0x7430, &m_charInfo->m_position, this);

        boost::shared_ptr<CharaColliAttack> attack(fn_800C42D0(fn_80021B08(), m_probe).lock());

        if (attack.get() != 0) {
            attack->field_0x55 = 0;
        }
    } else {
        m_object = fn_802B4254(0, 0);
        fn_802B4498(m_object.get(), s_objectValue.value);
        fn_802B4510(m_object.get(), 1);

        Fn8040Vector3 above(0.0f, s_objectHeight.value, 0.0f);
        Fn8040Vector3 objectPosition;

        fn_8040CDD0(&objectPosition, &above, &m_charInfo->m_position);
        fn_802B44C8(m_object.get(), &objectPosition);

        boost::shared_ptr<CharaColliAttack> attack(fn_800C42D0(fn_80021B08(), m_probe).lock());

        if (attack.get() != 0) {
            attack->field_0x54 = 0;
        }
    }
}

GiantLantern::~GiantLantern() {
    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

        if (effect.get() != 0) {
            effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
        }

        m_effect.reset();
    }

    m_sound.fn_801D455C();
}

extern "C" void fn_80236D10(void) {}

extern "C" void fn_80236D0C(void) {}

void GiantLantern::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new GiantLantern(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void GiantLantern::UpdateBody(float dt) {
    if (m_flags.Test(0)) {
        if (m_effect.expired()) {
            m_effect = fn_80168C48(&m_charInfo->m_position, &s_effectDirection, 0x1E);
        }

        fn_801D54F4(fn_80053164(), 0x7431, &m_charInfo->m_position, 0, 1);
        m_sound = fn_801D3E58(0x7430, &m_charInfo->m_position, this);

        {
            boost::shared_ptr<CharaColliAttack> attack(fn_800C42D0(fn_80021B08(), m_probe).lock());

            if (attack.get() != 0) {
                attack->field_0x55 = 0;
                attack->field_0x54 = 1;
            }
        }

        fn_8016907C(fn_800CB69C(), m_comId, 1);
        m_data->m_flags.Set(0);

        {
            boost::shared_ptr<UnkActor_800740E4> actor = fn_800740E4(fn_8006D6A8(), 0).lock();

            if (actor.get() != 0) {
                actor->UnkSlot7(1, 0);
            }
        }

        fn_800C985C(m_owner);
        m_object.reset();
        m_flags.Reset(0);
    }
}

void GiantLantern::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdatePositions();

    if (m_probe.get() != 0) {
        m_probe->UnkSlot26();
    }
}

void GiantLantern::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E60(owner, &m_comId);
}

void GiantLantern::UpdatePositions() {
    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

        if (effect.get() != 0) {
            fn_80425E4C(effect.get(), &m_charInfo->m_position);
        }
    }

    if (m_object.get() != 0) {
        Fn8040Vector3 above(0.0f, s_objectHeight.value, 0.0f);
        Fn8040Vector3 objectPosition;

        fn_8040CDD0(&objectPosition, &above, &m_charInfo->m_position);
        fn_802B44C8(m_object.get(), &objectPosition);
    }
}

void GiantLantern::OnAttack(AttackCallbackParam &param) {
    if (!m_data->m_flags.Test(0) && fn_8015D4E4(&param)) {
        boost::shared_ptr<CharaColliAttack> attack(param.field_0x0.lock());

        if (attack.get() != 0 && (attack->field_0x40 & 0x400) != 0) {
            m_flags.Set(0);
            param.field_0x54 = 1;
            return;
        }
    }

    param.field_0x54 = 5;
}

void GiantLantern::fn_8023684C(AttackCallbackParam &param) {
    Fn8040Vector3 direction;

    fn_8040CD74(&direction, &param.field_0x18, &m_charInfo->m_position);
    fn_800091D0(&param.field_0x34, &direction);
    fn_8040CE74(&param.field_0x34);
    param.field_0x4C = 10.0f;
    param.field_0x50 = 1.0f;
}

void GiantLantern::fn_80236848(AttackCallbackParam &param) {}

GadgetTypeInfo lbl_80547948 = {
    "GiantLantern", fn_80236D10, fn_80236D0C, GiantLantern::Create, 0, 0, 0xBC9, 1, 0, 10, 0,
};
