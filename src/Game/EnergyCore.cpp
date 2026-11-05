/* EnergyCore::SetEnergyCoreTask and EnergyCore::EnergyCoreTask (class names from RTTI; the gadget's
 * descriptor is "ENERGY_CORE"): an energy core placed as a gadget and one created at run time. Both
 * own an EnergyCore::EnergyCore (named after the class in the bound callback's RTTI string), which
 * holds a sphere probe of radius 10 around its character info and an effect (0xAD or 0xB1 by type).
 * When a character touches the probe, the core passes item 7 or 8 (by type) to fn_8015F6EC for it
 * once and marks itself touched; its owner then ends, the placed one after playing effect 0x2A1 or
 * 0x2A5 (by type) at its position (behaviour inferred). File name inferred from
 * the namespace. Method names marked "(name inferred)" are not original. Definitions are in reverse
 * address order (deferred inlining emits functions last-defined first).
 *
 * The probe's collision callback is a boost::function object built with
 * boost::bind(&EnergyCore::OnCollision, this, _1); its manager, invoker and copy functions come
 * from the boost headers. */
#include "Game/EnergyCore.h"
#include "Game/CharInfo.h"
#include "Game/EffectElement.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SharedConstants.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_80169BE4.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

using SonicteamUSA::System::CharaColli::CharaColliBase;

namespace EnergyCore {

const char *SetEnergyCoreTask::s_name = "EnergyCore::SetEnergyCoreTask";
const char *EnergyCoreTask::s_name = "EnergyCore::EnergyCoreTask";

/* The effect ids and the item ids of the two core types (indexed by the type). */
static const s32 s_effectIds[2] = {0xAD, 0xB1};
static const s32 s_itemIds[2] = {7, 8};

/* EnergyCore::EnergyCore (class name from the bound callback's RTTI string): the state that both
 * energy core tasks own. Member and method names are inferred. */
class EnergyCore {
  public:
    EnergyCore();
    ~EnergyCore();

    /* Passes dt to the handle target's update, fn_8015BE9C (name inferred). */
    void Update(float dt);
    /* Places the character info and the effect (name inferred). The vectors are not const: the
     * original copies the rotation component by component, each load after the previous store,
     * which a const source would let the compiler reorder. */
    void SetTransform(Fn8040Vector3 *position, Fn8040Vector3 *rotation);
    /* Stores the type and starts the type's effect at the character info (name inferred). */
    void SetType(int type);
    /* Calls fn_8015BE64 on the handle target (purpose unknown). */
    void fn_801018C8();
    /* Ends the effect (name inferred). */
    void StopEffect();
    /* The handle target's callback while its probe has hits: passes each character in the hits to
     * fn_801020B0 and marks the core touched when one of them was given the item (name inferred). */
    void OnCollision(ColliDetectArgs &args);

    /* Whether a character has touched the core (name inferred). */
    bool IsTouched() const {
        return m_flags.Test(0);
    }

    /* 0x00 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x08 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
    /* 0x10 */ boost::weak_ptr<Effect::System::EffectElement> m_effect;
    /* 0x18 */ s32 m_type;
    /* 0x1C */ UnkBitFlags_80014E04 m_flags; /* bit 0 once a character has touched the core */
};
CHECK_SIZEOF(EnergyCore, 0x20);

/* What the hit callback receives (placeholder name). */
struct UnkHitContext_801016FC {
    UnkHitContext_801016FC() : m_type(0), m_given(false) {}

    /* 0x0 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x8 */ s32 m_type;
    /* 0xC */ bool m_given; /* set once the item has been given */
};
CHECK_SIZEOF(UnkHitContext_801016FC, 0x10);

/* The callback that fn_8015AA84 calls for each character info in the hits: gives the first one the
 * type's item (behaviour inferred). */
static void fn_801020B0(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_801016FC *context = static_cast<UnkHitContext_801016FC *>(arg);

    if (context != 0 && !context->m_given) {
        fn_8015F6EC(hit, s_itemIds[context->m_type], lbl_805F4BF0, 0);
        context->m_given = true;
    }
}

EnergyCore::EnergyCore() {
    m_type = 0;
    m_charInfo = fn_8015CB88(0x1C, 4, "EnergyCore");

    boost::shared_ptr<CharaColliBase> probe = fn_800C7DEC(
        fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 0, 10, 0, &lbl_805E07A0, 10.0f);
    m_handle = fn_8015B8C4(probe);

    m_handle->field_0x0.fn_800F5980(
        ColliDetectCallback(boost::bind(&EnergyCore::OnCollision, this, _1)));
}

/* Matching workaround: auto_inline around this destructor. The original keeps checked_delete<T>
 * out of line and emits this destructor at its definition; with the stock GC/2.7 compiler and
 * -inline noauto that needs auto_inline here. A tooling constraint, not a claim about the original
 * source; revisit if a destructor called from checked_delete turns out to be inlined in the
 * original. */
#pragma auto_inline on
EnergyCore::~EnergyCore() {
    StopEffect();
}
#pragma auto_inline reset

void EnergyCore::Update(float dt) {
    fn_8015BE9C(m_handle.get(), dt);
}

void EnergyCore::SetTransform(Fn8040Vector3 *position, Fn8040Vector3 *rotation) {
    fn_800091D0(&m_charInfo->m_position, position);
    m_charInfo->m_rotation = *rotation;

    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

        fn_80425E4C(effect.get(), &m_charInfo->m_position);
    }
}

void EnergyCore::SetType(int type) {
    m_type = type;
    m_effect = fn_8015D794(&m_charInfo->m_position, s_effectIds[m_type]);
}

void EnergyCore::fn_801018C8() {
    fn_8015BE64(m_handle.get());
}

void EnergyCore::StopEffect() {
    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

        effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
    }

    m_effect.reset();
}

void EnergyCore::OnCollision(ColliDetectArgs &args) {
    UnkHitContext_801016FC context;

    context.m_charInfo = m_charInfo;
    context.m_type = m_type;
    fn_8015AA84(&args, fn_801020B0, &context);

    if (context.m_given) {
        m_flags.Set(0);
    }
}

SetEnergyCoreTask::SetEnergyCoreTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_core(new EnergyCore), m_rotation(0.0f, 0.0f, 0.0f), m_type(0) {
    m_name = s_name;
    CopyFromOwner(owner);
    m_core->SetType(m_type);
    UpdateTransform();
}

SetEnergyCoreTask::~SetEnergyCoreTask() {
    m_core->StopEffect();
}

void SetEnergyCoreTask::UpdateBody(float dt) {
    m_core->Update(dt);

    if (m_core->IsTouched()) {
        if (m_type == 0) {
            fn_8015D794(&m_position, 0x2A1);
        } else {
            fn_8015D794(&m_position, 0x2A5);
        }

        fn_8015A794(this);
        m_flags |= 1;
    }
}

void SetEnergyCoreTask::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    s32 type = m_type;

    CopyFromOwner(owner);

    if (m_type != type) {
        m_core->StopEffect();
        m_core->SetType(m_type);
    }

    UpdateTransform();
    m_core->fn_801018C8();
}

void SetEnergyCoreTask::UpdateTransform() {
    m_core->SetTransform(&m_position, &m_rotation);
}

EnergyCoreTask::EnergyCoreTask(int type, Task *parent, Fn8040Vector3 *position,
                               Fn8040Vector3 *rotation, float range)
    : Task(parent), m_core(new EnergyCore), m_rotation(0.0f, 0.0f, 0.0f) {
    m_name = s_name;
    fn_800091D0(&m_position, position);
    m_rotation = *rotation;
    m_rangeSquared = range * range;
    m_core->SetType(type);
    m_core->SetTransform(&m_position, &m_rotation);
}

EnergyCoreTask::~EnergyCoreTask() {
    m_core->StopEffect();
}

void EnergyCoreTask::Update(float dt) {
    m_core->Update(dt);

    if (m_core->IsTouched()) {
        m_flags |= 1;
    } else if (fn_80169BE4(&m_position, m_rangeSquared)) {
        m_flags |= 1;
    }
}

void EnergyCoreTask::Create(int type, Fn8040Vector3 *position, Fn8040Vector3 *rotation,
                            float range) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new EnergyCoreTask(type, parent, position, rotation, range);
}

} // namespace EnergyCore

extern "C" void fn_801011AC(void) {}

extern "C" void fn_801011A8(void) {}

void EnergyCore::SetEnergyCoreTask::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new SetEnergyCoreTask(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "type", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80525BBC = {
    "ENERGY_CORE",
    fn_801011AC,
    fn_801011A8,
    EnergyCore::SetEnergyCoreTask::Create,
    0,
    0,
    0x33,
    1,
    0,
    10,
    s_paramInfo,
};
