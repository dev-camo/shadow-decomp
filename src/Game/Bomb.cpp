/* The BOMB gadget (classes described in Game/Bomb.h). File name inferred from the namespace.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The bomb's two character info handlers and the attack callbacks of the bomb and the blast are
 * boost::function objects built with boost::bind(&Class::method, this, _1); their managers,
 * invokers and copy functions come from the boost headers. */
/* Matching workaround: the unit's .data starts with the base lists of the type records of
 * TNumEdit's and CharaColliBase's derived classes, which TNumEdit.h and CharaColliBaseCharInfo.h
 * emit through their stand-in inline functions (see TNumEdit.h). The two headers come first, in
 * the order of what they put in the unit's .rodata: the TNumEdit names, then the CharaColliBase
 * names. Revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/Bomb.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/EffectElement.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/Target.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_800EDA3C.h"
#include "Game/fn_801BFC28.h"
#include "Game/fn_8033F0F8.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;
using SonicteamUSA::System::CharaColli::CharaColliBase;

namespace Bomb {

const char *BombBlast::s_name = "BombBlast";

Bomb::Bomb() : PickupBase(0xE) {
    fn_801DC984("Bomb");
}

Bomb::Bomb(const boost::weak_ptr<CharInfo> &owner) : PickupBase(0xE), m_owner(owner) {
    fn_801DC984("Bomb");
}

Bomb::~Bomb() {
    if (!m_owner.expired()) {
        boost::shared_ptr<CharInfo> owner(m_owner.lock());

        if (owner.get() != 0) {
            owner->fn_801DC980(new UseCommand(boost::weak_ptr<CharInfo>(m_charInfo)));
        }
    }
}

void Bomb::Initialize(UnkGadgetOwner_800C9CD0 *owner) {
    m_flags.Clear();
    m_sweepFlags.Set(1);

    m_charInfo = boost::shared_ptr<PickupCharInfo>(new PickupCharInfo(0xE));
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&Bomb::OnCommand, this, _1));
    m_charInfo->fn_801E4160(boost::weak_ptr<CharInfo>(m_charInfo), 0x880, 30.0f);
    m_charInfo->fn_801DC984("Bomb");
    m_charInfo->SetField0x5C(4);
    m_charInfo->m_grabHandler =
        PickupCharInfo::SetGrabPositionHandler(boost::bind(&Bomb::OnSetGrabPosition, this, _1));

    m_model = fn_8015F800(0, "BOMB.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo));

    boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161428(0);
    if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 = AttackCallback(boost::bind(&Bomb::OnAttack, this, _1));
    }

    m_state = STATE_START;
    m_comId = 0;
    SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);
}

void Bomb::Update(float dt) {
    if (m_flags.Test(FLAG_MOVING)) {
        UpdateMotion(dt);
    }
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case STATE_START:
        if (m_behavior.get() == 0) {
            m_behavior = boost::shared_ptr<ContainerBehavior>(
                new ContainerBehavior(&m_charInfo->m_position, m_model));
        } else {
            m_behavior->Reset(&m_charInfo->m_position, &m_charInfo->m_rotation);
        }
        m_state = STATE_WAITING;
        break;

    case STATE_WAITING:
        if (m_behavior.get() != 0) {
            m_charInfo->m_position.y -= 10.0f;
            m_behavior->SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);
            m_behavior->Update();
            m_behavior->GetPosition(&m_charInfo->m_position);
            m_charInfo->m_position.y += 10.0f;
        }
        if (m_comId > 0 && fn_8016904C(fn_800CB69C(), m_comId) == 1) {
            m_state = STATE_BURST;
        }
        SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);
        break;

    case STATE_PICKED: {
        /* The state is stored before the locked pointer is released (USA (GUPE8P) 0x800EF2E0),
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
        fn_800ED9E8(&m_charInfo->m_position);
        if (boost::shared_ptr<CharInfo> breaker = m_breaker.lock()) {
            fn_8020642C(breaker, 0x11);
        }
        m_flags.Set(FLAG_DONE);
        break;

    case STATE_VACUUMED:
        if (boost::shared_ptr<CharInfo> breaker = m_breaker.lock()) {
            fn_8020642C(breaker, 0x11);
        }
        m_flags.Set(FLAG_DONE);
        break;
    }

    m_charInfo->fn_801E413C(&m_charInfo->m_position);
}

bool Bomb::TestFlags(const UnkBitFlags_80014E04 &flags) {
    if (flags.Test(0) && m_flags.Test(FLAG_PICKED)) {
        return true;
    }
    if (flags.Test(1) && m_flags.Test(FLAG_DONE)) {
        return true;
    }
    return false;
}

void Bomb::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    if (m_charInfo.get() != 0) {
        fn_800C9F14(owner, &m_charInfo->m_position);
        fn_800C9ED4(owner, &m_charInfo->m_rotation);
        fn_800C9E50(owner, &m_comId);
        m_charInfo->m_position.y += 10.0f;
    }
    m_state = STATE_START;
    SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);
}

void Bomb::ResetTransform(const Fn8040Vector3 *position, const UnkRotation3 *rotation) {
    if (m_charInfo.get() != 0) {
        m_charInfo->m_position = *position;
        m_charInfo->m_rotation = *rotation;
        m_charInfo->m_position.y += 10.0f;
    }
    m_state = STATE_START;
    m_comId = 0;
    SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);
}

void Bomb::SetTransform(const Fn8040Vector3 *position, const UnkRotation3 *rotation) {
    PickupBase::SetTransform(position, rotation);

    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, position);
        fn_8040B848(&matrix, rotation->y);
        fn_8040B884(&matrix, rotation->x);
        fn_8040B80C(&matrix, rotation->z);
        m_model->fn_801633DC(&matrix);
    }
}

void Bomb::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x11: /* PickupCommand */
        if (m_state == STATE_WAITING && !m_flags.Test(FLAG_PICKED)) {
            m_picker = static_cast<PickupCommand &>(command).m_picker;
            m_charInfo->m_target->ClearFlags(1);
            m_model->fn_80161230(0, -1);
            m_behavior.reset();
            m_state = STATE_PICKED;
            m_flags.Set(FLAG_MOVING);
            m_flags.Set(FLAG_PICKED);
        }
        break;

    case 0x13: /* ThrowCommand */
        if (m_state == STATE_HELD) {
            ThrowCommand &thrown = static_cast<ThrowCommand &>(command);

            m_breaker = thrown.field_0x14;
            field_0xA4 = thrown.field_0x1C;
            field_0xA0 = -320.0f;
            fn_8040CE60(&m_charInfo->field_0x24);
            m_charInfo->m_position.y += 10.0f;
            fn_800494F8(&m_charInfo->m_rotation, &m_grabMatrix);

            m_charInfo->field_0x18 =
                fn_8040B618(m_grabMatrix, Fn8040Vector3(0.0f, 100.0f, -150.0f));
            m_state = STATE_FLYING;
            m_flags.Set(FLAG_ACCELERATE);
            m_flags.Set(FLAG_MOVING);
            m_flags.Set(FLAG_SWEEP);
        }
        break;

    case 0x12: /* CastCommand */
        if (m_state == STATE_HELD) {
            field_0xA4 = static_cast<CastCommand &>(command).field_0x1C;
            field_0xA0 = -320.0f;
            fn_8040CE60(&m_charInfo->field_0x24);
            m_charInfo->m_position.y += 10.0f;

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
            fn_8033F0F8(&command, m_model, 0, &m_position, &m_rotation);
            m_model.reset();
            m_state = STATE_VACUUMED;
        }
        break;
    }

    command.m_flags |= 2;
}

void Bomb::OnSetGrabPosition(PickupCharInfo::SetGrabPositionArgs &args) {
    fn_8040C148(&m_grabMatrix, args.m_matrix);

    if (m_state == STATE_HELD) {
        UpdateHeldPosition();
    }
}

void Bomb::UpdateHeldPosition() {
    m_charInfo->m_position = fn_800496F8(&m_grabMatrix);
    PickupBase::SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);

    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &m_grabMatrix);
    fn_8040B52C(&matrix, 0.0f, 10.0f, 0.0f);
    m_model->fn_801633DC(&matrix);
}

void Bomb::OnAttack(AttackCallbackParam &param) {
    if (m_state == STATE_WAITING) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if (attack->field_0x40 & 2) {
                param.field_0x54 = 5;
                return;
            }
            m_breaker = attack->m_owner;
            m_state = STATE_BURST;
            param.field_0x54 = 3;
            return;
        }
    }

    param.field_0x54 = 5;
}

void Bomb::UpdateMotion(float dt) {
    if (m_flags.Test(FLAG_ACCELERATE)) {
        AddVector(&m_charInfo->field_0x24, field_0xA4 * field_0xA0);
    }
    fn_804103E8(&m_charInfo->m_position, &m_charInfo->field_0x18, &m_charInfo->field_0x24, 1.0f,
                dt);

    if (m_flags.Test(FLAG_SWEEP)) {
        Fn8040Vector3 movement = m_charInfo->m_position - m_previousPosition;
        Fn8040Vector3 position = m_previousPosition;

        /* Matching workaround: the world getter is called on its own and its result is unused.
         * The original calls it at this point and discards r3 (USA (GUPE8P) 0x800EE8C0, before
         * 0 is loaded as fn_801BFC28's first argument), as the other callers of fn_801BFC28 that
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

BombBlast::BombBlast(Task *parent, const Fn8040Vector3 *position) : Task(parent) {
    m_name = s_name;
    m_position = *position;

    boost::shared_ptr<CharaColliBase> probe =
        fn_800C7EA8(fn_80021B08(), &m_position, 0x40000, 0xF, 0,
                    boost::weak_ptr<CharInfo>(lbl_805EE698), &lbl_805E07A0, 1.0f);

    m_handle = fn_8015B8C4(probe);

    Fn8040Vector3 direction(0.0f, 1.0f, 0.0f);

    fn_8015D7DC(&m_position, &direction, 0x19);
    m_state = STATE_START;
    m_timer = 0.0f;
    m_radius = 0.0f;
}

BombBlast::~BombBlast() {}

void BombBlast::Update(float dt) {
    switch (m_state) {
    case STATE_START:
        m_timer = 0.0f;
        m_state = STATE_WAITING;
        m_radius = 5.0f;
        fn_801D54F4(fn_80053164(), 0x1008, &m_position, 0, 1);
        break;
    case STATE_WAITING:
        m_timer += dt;
        if (m_timer >= 0.13f) {
            m_state = STATE_GROWING;
            m_timer = 0.0f;
            if (m_handle.get() != 0) {
                boost::weak_ptr<CharaColliAttack> weakAttack =
                    fn_800C4374(fn_80021B08(), m_handle->m_probe);

                if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
                    attack->field_0x54 = 1;
                    attack->field_0x38 = 3;
                    attack->field_0x34 = 4.0f;
                    attack->field_0x40 = 0x200;
                    attack->field_0x10.field_0x0 =
                        AttackCallback(boost::bind(&BombBlast::OnAttack, this, _1));
                }
                m_handle->m_probe->SetSize(m_radius, -2, 0.0f, 0.0f);
            }
        }
        break;
    case STATE_GROWING:
        m_timer += dt;
        if (m_timer >= 0.87f) {
            m_timer = 0.87f;
            m_state = STATE_FINISHED;
        }
        /* The radius grows from 5 to 50 while the timer runs to 0.87. */
        m_radius = (50.0f - 5.0f) / 0.87f * m_timer + 5.0f;
        if (m_radius >= 50.0f) {
            m_radius = 50.0f;
        }
        if (m_handle.get() != 0) {
            m_handle->m_probe->SetSize(m_radius, -2, 0.0f, 0.0f);
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

void BombBlast::OnAttack(AttackCallbackParam &param) {}

} // namespace Bomb

/* The descriptor's first hook: adds BOMB.BIN to the resource list (fn_8006B474 with 1). */
extern "C" void fn_800EDE08(void) {
    fn_8006B474("BOMB.BIN", 1);
}

/* The descriptor's second hook: does nothing. */
extern "C" void fn_800EDE04(void) {}

void Bomb::Bomb::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);
    boost::shared_ptr<GadgetPickup::PickupBase> unit =
        boost::shared_ptr<GadgetPickup::PickupBase>(new Bomb());

    unit->Initialize(owner);
    unit->ResetFromOwner(owner);
    new GadgetPickup::SetPickupBaseTask(parent, owner, unit);
}

extern "C" void fn_800EDA3C(const Fn8040Vector3 *position, const UnkRotation3 *rotation,
                            const boost::weak_ptr<CharInfo> &owner, float radius) {
    boost::shared_ptr<GadgetPickup::PickupBase> unit =
        boost::shared_ptr<GadgetPickup::PickupBase>(new Bomb::Bomb(owner));

    unit->Initialize(0);
    unit->ResetTransform(position, rotation);

    Task *parent = fn_801EB304(fn_8001C4F8(), 11);
    GadgetPickup::PickupBaseTask *task = new GadgetPickup::PickupBaseTask(parent, unit);
    task->SetRadius(radius);
}

extern "C" void fn_800ED9E8(const Fn8040Vector3 *position) {
    new Bomb::BombBlast(fn_801EB304(fn_8001C4F8(), 11), position);
}

GadgetTypeInfo lbl_80524C78 = {
    "BOMB", fn_800EDE08, fn_800EDE04, Bomb::Bomb::Create, 0, 0, 0xD, 1, 0, 10, 0,
};
