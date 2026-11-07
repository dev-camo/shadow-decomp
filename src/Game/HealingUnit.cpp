/* The HEALING_UNIT gadget (classes described in Game/HealingUnit.h). File name inferred from the
 * namespace. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first).
 *
 * The unit's two character info handlers and the blast's collision callback are boost::function
 * objects built with boost::bind(&Class::method, this, _1); their manager, invoker and copy
 * functions come from the boost headers. */
#include "Game/HealingUnit.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/Target.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_801072C4.h"
#include "Game/fn_801BFC28.h"
#include "Game/fn_8033F0F8.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::CharaColliBase;

namespace HealingUnit {

const char *HealingBlast::s_name = "HealingBlast";

/* What HealingBlast::OnCollision passes to fn_8015B384 for the hit callback (placeholder name): the
 * blast's owner and amount, for the RecoverCommands that the hit callback sends. The original has
 * weak out-of-line copies of its constructor (0x801075DC, which also sets the amount to 0) and
 * destructor (0x80107590), hence the user-declared constructor. */
struct UnkHitContext_8010750C {
    UnkHitContext_8010750C() : m_amount(0.0f) {}

    /* 0x0 */ boost::weak_ptr<CharInfo> m_owner;
    /* 0x8 */ float m_amount;
};
CHECK_SIZEOF(UnkHitContext_8010750C, 0xC);

HealingUnit::HealingUnit() : PickupBase(0x4E) {
    fn_801DC984("HealingUnit");
}

HealingUnit::HealingUnit(const boost::weak_ptr<CharInfo> &owner)
    : PickupBase(0x4E), m_owner(owner) {
    fn_801DC984("HealingUnit");
}

HealingUnit::~HealingUnit() {
    if (!m_owner.expired()) {
        boost::shared_ptr<CharInfo> owner(m_owner.lock());

        if (owner.get() != 0) {
            owner->fn_801DC980(new UseCommand(boost::weak_ptr<CharInfo>(m_charInfo)));
        }
    }
}

void HealingUnit::Initialize(UnkGadgetOwner_800C9CD0 *owner) {
    m_flags.Clear();
    m_sweepFlags.Set(1);

    m_charInfo = boost::shared_ptr<PickupCharInfo>(new PickupCharInfo(0x4E));
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&HealingUnit::OnCommand, this, _1));
    m_charInfo->fn_801E4160(boost::weak_ptr<CharInfo>(m_charInfo), 0x80, 30.0f);
    m_charInfo->fn_801DC984("HealingUnit");
    m_charInfo->m_grabHandler = PickupCharInfo::SetGrabPositionHandler(
        boost::bind(&HealingUnit::OnSetGrabPosition, this, _1));

    m_model = fn_8015F800(0, "HEALINGUNIT.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo));
}

void HealingUnit::Update(float dt) {
    if (m_flags.Test(FLAG_MOVING)) {
        UpdateMotion(dt);
    }
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case STATE_START:
        m_state = STATE_WAITING;
        break;

    case STATE_WAITING:
        break;

    case STATE_PICKED: {
        /* The state is stored before the locked pointer is released (USA (GUPE8P) 0x80108654),
         * so the pointer lives until the end of the case. */
        boost::shared_ptr<CharInfo> picker(m_picker.lock());

        if (picker.get() != 0) {
            picker->fn_801DC980(new ReadyCommand(boost::weak_ptr<CharInfo>(m_charInfo)));
        }
        m_state = STATE_HELD;
        break;
    }

    case STATE_HELD:
        UpdateHeldPosition();
        break;

    case STATE_FLYING:
        SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);
        break;

    case STATE_BURST:
        fn_80107060(m_picker, &m_charInfo->m_position, 0);
        if (boost::shared_ptr<CharInfo> picker = m_picker.lock()) {
            fn_8020642C(picker, 0x20);
        }
        m_flags.Set(FLAG_DONE);
        break;

    case STATE_VACUUMED:
        m_flags.Set(FLAG_DONE);
        break;
    }

    Fn8040Vector3 targetPosition = m_charInfo->m_position;
    targetPosition.y += 8.0f;
    m_charInfo->fn_801E413C(&targetPosition);
}

bool HealingUnit::TestFlags(const UnkBitFlags_80014E04 &flags) {
    if (flags.Test(0) && m_flags.Test(FLAG_PICKED)) {
        return true;
    }
    if (flags.Test(1) && m_flags.Test(FLAG_DONE)) {
        return true;
    }
    return false;
}

void HealingUnit::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    if (m_charInfo.get() != 0) {
        fn_800C9F14(owner, &m_charInfo->m_position);
        fn_800C9ED4(owner, &m_charInfo->m_rotation);
    }
    m_state = STATE_START;
    SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);
}

void HealingUnit::ResetTransform(const Fn8040Vector3 *position, const UnkRotation3 *rotation) {
    if (m_charInfo.get() != 0) {
        m_charInfo->m_position = *position;
        m_charInfo->m_rotation = *rotation;
    }
    m_state = STATE_START;
    SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);
}

void HealingUnit::SetTransform(const Fn8040Vector3 *position, const UnkRotation3 *rotation) {
    PickupBase::SetTransform(position, rotation);

    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040B6F8(&matrix);
        fn_8040B59C(&matrix, *position);
        fn_8040B848(&matrix, rotation->y);
        m_model->fn_801633DC(&matrix);
    }
}

void HealingUnit::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x11: /* PickupCommand */
        if (m_state == STATE_WAITING && !m_flags.Test(FLAG_PICKED)) {
            m_picker = static_cast<PickupCommand &>(command).m_picker;
            m_charInfo->m_target->ClearFlags(1);
            m_model->fn_80161230(0, -1);
            m_state = STATE_PICKED;
            m_flags.Set(FLAG_PICKED);
            m_flags.Set(FLAG_MOVING);
        }
        break;

    case 0x13: /* ThrowCommand */
        if (m_state == STATE_HELD) {
            field_0x90 = static_cast<ThrowCommand &>(command).field_0x1C;
            field_0x8C = -320.0f;
            fn_8040CE60(&m_charInfo->field_0x24);
            fn_800494F8(&m_charInfo->m_rotation, &m_grabMatrix);

            m_charInfo->field_0x18 = fn_8040B618(m_grabMatrix, Fn8040Vector3(0.0f, 100.0f, -80.0f));
            m_state = STATE_FLYING;
            m_flags.Set(FLAG_ACCELERATE);
            m_flags.Set(FLAG_MOVING);
            m_flags.Set(FLAG_SWEEP);
        }
        break;

    case 0x12: /* CastCommand */
        if (m_state == STATE_HELD) {
            field_0x90 = static_cast<CastCommand &>(command).field_0x1C;
            field_0x8C = -320.0f;
            fn_8040CE60(&m_charInfo->field_0x24);

            m_charInfo->field_0x18 = fn_8040B618(m_grabMatrix, Fn8040Vector3(0.0f, 100.0f, 0.0f));
            m_state = STATE_FLYING;
            m_flags.Set(FLAG_ACCELERATE);
            m_flags.Set(FLAG_MOVING);
            m_flags.Set(FLAG_SWEEP);
        }
        break;

    case 0x14: /* VacuumCommand */
        if (m_state == STATE_WAITING) {
            m_model->fn_80161880();
            fn_8033F0F8(&command, m_model, 1, &m_position, &m_rotation);
            m_model.reset();
            m_state = STATE_VACUUMED;
        }
        break;
    }

    command.m_flags |= 2;
}

void HealingUnit::OnSetGrabPosition(PickupCharInfo::SetGrabPositionArgs &args) {
    fn_8040C148(&m_grabMatrix, args.m_matrix);

    if (m_state == STATE_HELD) {
        UpdateHeldPosition();
    }
}

void HealingUnit::UpdateHeldPosition() {
    m_charInfo->m_position = fn_800496F8(&m_grabMatrix);
    m_model->fn_801633DC(&m_grabMatrix);
    PickupBase::SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);
}

void HealingUnit::UpdateMotion(float dt) {
    if (m_flags.Test(FLAG_ACCELERATE)) {
        AddVector(&m_charInfo->field_0x24, field_0x90 * field_0x8C);
    }
    fn_804103E8(&m_charInfo->m_position, &m_charInfo->field_0x18, &m_charInfo->field_0x24, 1.0f,
                dt);

    if (m_flags.Test(FLAG_SWEEP)) {
        Fn8040Vector3 movement = m_charInfo->m_position - m_previousPosition;
        Fn8040Vector3 position = m_previousPosition;

        /* Matching workaround: the world getter is called on its own and its result is unused.
         * The original calls it at this point and discards r3 (USA (GUPE8P) 0x80107FBC, before
         * 0 is loaded as fn_801BFC28's first argument), as all seven callers of fn_801BFC28 that
         * pass 0 there do (see fn_801BFC28.h); the source form behind the call, probably a shared
         * inline wrapper, is unknown. Revisit when one of those callers or the wrapper is
         * recovered. */
        fn_80021B08();
        if (fn_801BFC28(0, &position, &movement, &m_sweepFlags, 0, 10.0f)) {
            m_charInfo->m_position = position;
            m_charInfo->field_0x18 = movement * (1.0f / dt);
            m_state = STATE_BURST;
        }
    }

    m_previousPosition = m_charInfo->m_position;
    fn_8040CE60(&m_charInfo->field_0x24);
}

/* The hit callback of HealingBlast::OnCollision (placeholder name): when the context is not null,
 * sends the character info a new RecoverCommand with the context's owner and amount (null when
 * its allocation fails; sent either way). */
static void fn_80107EAC(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_8010750C *context = static_cast<UnkHitContext_8010750C *>(arg);

    if (context != 0) {
        hit->fn_801DC980(new RecoverCommand(context->m_owner, context->m_amount));
    }
}

HealingBlast::HealingBlast(Task *parent, const boost::weak_ptr<CharInfo> &owner,
                           const Fn8040Vector3 *position, int type, float amount)
    : Task(parent) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x4E, 4, "HealingBlast");
    m_charInfo->m_position = *position;

    float radius = 0.0f;
    int effectId = 0;
    switch (type) {
    case 0:
        radius = 20.0f;
        effectId = 0xC5;
        break;
    case 1:
        radius = 40.0f;
        effectId = 0x2DA;
        break;
    }

    boost::shared_ptr<CharaColliBase> probe =
        fn_800C7EA8(fn_80021B08(), &m_charInfo->m_position, 0x40000, 3, 0,
                    boost::weak_ptr<CharInfo>(lbl_805EE698), &lbl_805E07A0, radius);

    m_handle = fn_8015B8C4(probe);
    m_handle->field_0x0.fn_800F5980(
        GadgetColliDetectCallback(boost::bind(&HealingBlast::OnCollision, this, _1)));

    Fn8040Vector3 up(0.0f, 1.0f, 0.0f);
    m_effect = fn_8015D7DC(&m_charInfo->m_position, &up, effectId);

    m_amount = amount;
    m_state = STATE_START;
    m_timer = 1.0f;
    m_owner = owner;
}

HealingBlast::~HealingBlast() {
    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

        effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
    }
}

void HealingBlast::Update(float dt) {
    switch (m_state) {
    case STATE_START:
        m_state = STATE_RUNNING;
        break;
    case STATE_RUNNING:
        m_timer -= dt;
        if (m_timer <= 0.0f) {
            m_state = STATE_FINISHED;
        }
        break;
    case STATE_FINISHED:
        m_flags |= 1;
        break;
    }

    if (m_handle.get() != 0) {
        fn_8015BE9C(m_handle.get(), dt);
    }
}

void HealingBlast::OnCollision(ColliDetectArgs &args) {
    UnkHitContext_8010750C context;

    context.m_owner = m_owner;
    context.m_amount = m_amount;
    fn_8015B384(&args, fn_80107EAC, &context);
    m_handle->SetFlags(2);
}

} // namespace HealingUnit

/* The descriptor's first hook: adds HEALINGUNIT.BIN to the resource list (fn_8006B474 with 1). */
extern "C" void fn_801074E0(void) {
    fn_8006B474("HEALINGUNIT.BIN", 1);
}

/* The descriptor's second hook: does nothing. */
extern "C" void fn_801074DC(void) {}

void HealingUnit::HealingUnit::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);
    boost::shared_ptr<GadgetPickup::PickupBase> unit =
        boost::shared_ptr<GadgetPickup::PickupBase>(new HealingUnit());

    unit->Initialize(owner);
    unit->ResetFromOwner(owner);
    new GadgetPickup::SetPickupBaseTask(parent, owner, unit);
}

/* Initializes the unit without an owner, moves it to position and rotation, and creates the task
 * that updates it under task layer 11 (only when the allocation succeeds), calling its SetRadius
 * with radius without testing the allocation (placeholder name).
 *
 * Matching workaround: unit is a non-const reference, a spelling inferred from the bytes: with a
 * const reference the object-pointer load for the first virtual call moves from function offset
 * 0x28 (USA (GUPE8P) 0x80107384) to 0x1C, ahead of the two argument register copies and the
 * argument 0, so the four words at 0x1C-0x28 differ. Revisit when a caller of another shape or
 * the original helper is recovered. */
static void fn_8010735C(boost::shared_ptr<GadgetPickup::PickupBase> &unit,
                        const Fn8040Vector3 *position, const UnkRotation3 *rotation, float radius) {
    unit->Initialize(0);
    unit->ResetTransform(position, rotation);

    Task *parent = fn_801EB304(fn_8001C4F8(), 11);
    GadgetPickup::PickupBaseTask *task = new GadgetPickup::PickupBaseTask(parent, unit);
    task->SetRadius(radius);
}

extern "C" void fn_801072C4(const Fn8040Vector3 *position, const UnkRotation3 *rotation,
                            const boost::weak_ptr<CharInfo> &owner, float radius) {
    boost::shared_ptr<GadgetPickup::PickupBase> unit =
        boost::shared_ptr<GadgetPickup::PickupBase>(new HealingUnit::HealingUnit(owner));

    fn_8010735C(unit, position, rotation, radius);
}

extern "C" void fn_801070CC(const Fn8040Vector3 *position, const UnkRotation3 *rotation,
                            float radius) {
    boost::shared_ptr<GadgetPickup::PickupBase> unit =
        boost::shared_ptr<GadgetPickup::PickupBase>(new HealingUnit::HealingUnit());

    fn_8010735C(unit, position, rotation, radius);
}

extern "C" void fn_80107060(const boost::weak_ptr<CharInfo> &owner, const Fn8040Vector3 *position,
                            int type) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new HealingUnit::HealingBlast(parent, owner, position, type, 1.0f);
}

GadgetTypeInfo lbl_80526234 = {
    "HEALING_UNIT",
    fn_801074E0,
    fn_801074DC,
    HealingUnit::HealingUnit::Create,
    0,
    0,
    0x38,
    1,
    0,
    10,
    0,
};
