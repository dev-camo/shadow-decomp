/* The SECRET CONTAINER gadget (class described in Game/ContainerSecret.h). File name inferred from
 * the class name. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first).
 *
 * The container's two character info handlers and its attack callback are boost::function objects
 * built with boost::bind(&ContainerSecret::method, this, _1); their manager, invoker and copy
 * functions come from the boost headers. */
/* Matching workaround: the unit's .data starts with the base lists of the type records of
 * TNumEdit's and CharaColliBase's derived classes, which TNumEdit.h and CharaColliBaseCharInfo.h
 * emit through their stand-in inline functions (see TNumEdit.h). The two headers come first, in
 * the order of the two names they leave at the start of the unit's .rodata: TNumEdit, then
 * CharaColliBase. Revisit when the original inline functions or their real callers are
 * recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include <cstring>
#include "Game/ContainerSecret.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/Target.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/VacuumContainer.h"
#include "Game/WeaponItem.h"
#include "Game/fn_801BFC28.h"
#include "Game/fn_8027B338.h"
#include "Game/fn_8033E644.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The model file that Initialize loads. It is a pointer variable because the original loads the
 * name from a pointer in .sdata there (USA (GUPE8P) 0x80279A4C); the descriptor's first hook uses
 * the string's address directly. */
static const char *s_modelFile = "CONTAINERSECRET.BIN";

namespace Container {

ContainerSecret::ContainerSecret() : PickupBase(4), m_weapon(0) {
    fn_801DC984("ContainerSecret");
}

ContainerSecret::~ContainerSecret() {}

void ContainerSecret::Initialize(UnkGadgetOwner_800C9CD0 *owner) {
    m_flags.Clear();
    m_sweepFlags.Set(1);

    m_charInfo = boost::shared_ptr<PickupCharInfo>(new PickupCharInfo(4));
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&ContainerSecret::OnCommand, this, _1));
    m_charInfo->fn_801E4160(boost::weak_ptr<CharInfo>(m_charInfo), 0x80, 30.0f);
    m_charInfo->fn_801DC984("ContainerSecret");
    m_charInfo->SetField0x5C(4);
    m_charInfo->m_grabHandler = PickupCharInfo::SetGrabPositionHandler(
        boost::bind(&ContainerSecret::OnSetGrabPosition, this, _1));

    m_model = fn_8015F800(0, s_modelFile);
    m_model->fn_80162A68(MODEL_INDEX, 1);
    m_model->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo));

    boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161428(0);
    if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 =
            AttackCallback(boost::bind(&ContainerSecret::OnAttack, this, _1));
    }

    m_state = STATE_START;
    SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);
}

void ContainerSecret::Update(float dt) {
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
        SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);
        break;

    case STATE_PICKED: {
        /* The state is stored before the locked pointer is released (USA (GUPE8P) 0x80278DB8),
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

    case STATE_THROWN:
        m_flags.Set(FLAG_DONE);
        break;

    case STATE_FLYING:
        SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);
        break;

    case STATE_BROKEN:
        m_model->fn_80161880();
        SpawnDebris();
        fn_8033E828(SELECT_SECRET, m_weapon, &m_charInfo->m_position, &m_charInfo->m_rotation);
        if (boost::shared_ptr<CharInfo> breaker = m_breaker.lock()) {
            fn_8020642C(breaker, EVENT_ID);
        }
        m_flags.Set(FLAG_DONE);
        break;

    case STATE_VACUUMED:
        m_flags.Set(FLAG_DONE);
        break;
    }

    m_charInfo->fn_801E413C(&m_charInfo->m_position);
}

bool ContainerSecret::TestFlags(const UnkBitFlags_80014E04 &flags) {
    if (flags.Test(0) && m_flags.Test(FLAG_PICKED)) {
        return true;
    }
    if (flags.Test(1) && m_flags.Test(FLAG_DONE)) {
        return true;
    }
    return false;
}

void ContainerSecret::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    if (m_charInfo.get() != 0) {
        fn_800C9F14(owner, &m_charInfo->m_position);
        fn_800C9ED4(owner, &m_charInfo->m_rotation);
        fn_800C9DBC(owner, &m_weapon, sizeof(m_weapon));
        m_charInfo->m_position.y += 10.0f;
    }

    SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);
    m_state = STATE_START;
}

void ContainerSecret::SetTransform(const Fn8040Vector3 *position, const UnkRotation3 *rotation) {
    PickupBase::SetTransform(position, rotation);

    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, *position);
        fn_8040B848(&matrix, rotation->y);
        fn_8040B884(&matrix, rotation->x);
        fn_8040B80C(&matrix, rotation->z);
        m_model->fn_801633DC(&matrix);
    }
    if (m_behavior.get() != 0) {
        m_behavior->SetTransform(position, rotation);
    }
}

void ContainerSecret::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x11: /* PickupCommand */
        if (m_state == STATE_WAITING && !m_flags.Test(FLAG_PICKED)) {
            PickupCommand &pickup = static_cast<PickupCommand &>(command);

            m_picker = pickup.m_picker;
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
            field_0xAC = thrown.field_0x1C;
            field_0xA8 = -320.0f;
            fn_8040CE60(&m_charInfo->field_0x24);
            m_charInfo->m_position.y += 10.0f;
            m_charInfo->field_0x18 =
                fn_8040B618(m_grabMatrix, Fn8040Vector3(0.0f, 100.0f, -150.0f));

            UnkRotation3 angles(0.0f, 0.0f, 0.0f);
            fn_800494F8(&angles, &m_grabMatrix);

            Fn8040Vector3 acceleration = field_0xAC * field_0xA8;
            fn_8027B338(thrown.field_0x14, m_model, MODEL_INDEX, SELECT_SECRET, m_weapon,
                        &m_charInfo->m_position, &angles, &m_charInfo->field_0x18, acceleration,
                        EVENT_ID, 0.0f, 1.0f, 600.0f);
            m_model.reset();
            m_state = STATE_THROWN;
        }
        break;

    case 0x12: /* CastCommand */
        if (m_state == STATE_HELD) {
            CastCommand &cast = static_cast<CastCommand &>(command);

            field_0xAC = cast.field_0x1C;
            field_0xA8 = -320.0f;
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
            fn_80254384(static_cast<VacuumCommand *>(&command), m_model, MODEL_INDEX, SELECT_SECRET,
                        m_weapon, &m_position, &m_rotation, EVENT_ID);
            m_model.reset();
            m_state = STATE_VACUUMED;
        }
        break;

    case 0x17: { /* SetTransparentCommand */
        SetTransparentCommand &transparent = static_cast<SetTransparentCommand &>(command);

        if (transparent.m_alpha < 1.0f) {
            if (m_model.get() != 0) {
                fn_8015FDD8(m_model.get(), transparent.m_alpha);
            }
        } else {
            if (m_model.get() != 0) {
                fn_8015FD10(m_model.get());
            }
        }
        break;
    }
    }

    command.m_flags |= 2;
}

void ContainerSecret::OnSetGrabPosition(PickupCharInfo::SetGrabPositionArgs &args) {
    fn_8040C148(&m_grabMatrix, args.m_matrix);

    if (m_state == STATE_HELD) {
        UpdateHeldPosition();
    }
}

void ContainerSecret::UpdateHeldPosition() {
    m_charInfo->m_position = fn_800496F8(&m_grabMatrix);
    PickupBase::SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);

    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &m_grabMatrix);
    fn_8040B52C(&matrix, 0.0f, 10.0f, 0.0f);
    m_model->fn_801633DC(&matrix);
}

void ContainerSecret::SpawnDebris() {
    fn_8033E644(m_model, MODEL_INDEX, &m_charInfo->m_position, &m_charInfo->m_rotation);
}

void ContainerSecret::OnAttack(AttackCallbackParam &param) {
    if (m_state == STATE_WAITING) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if (attack->field_0x40 & 2) {
                param.field_0x54 = 5;
                return;
            }
            m_breaker = attack->m_owner;
            m_state = STATE_BROKEN;
            param.field_0x54 = 3;
            return;
        }
    }

    param.field_0x54 = 5;
}

void ContainerSecret::UpdateMotion(float dt) {
    if (m_flags.Test(FLAG_ACCELERATE)) {
        AddVector(&m_charInfo->field_0x24, field_0xAC * field_0xA8);
    }
    fn_804103E8(&m_charInfo->m_position, &m_charInfo->field_0x18, &m_charInfo->field_0x24, 1.0f,
                dt);

    if (m_flags.Test(FLAG_SWEEP)) {
        Fn8040Vector3 movement = m_charInfo->m_position - m_previousPosition;
        Fn8040Vector3 position = m_previousPosition;

        /* Matching workaround: the world getter is called on its own and its result is unused.
         * The original calls it at this point and discards r3 (USA (GUPE8P) 0x8027842C, before
         * 0 is loaded as fn_801BFC28's first argument), as the other callers of fn_801BFC28 that
         * pass 0 there do (see fn_801BFC28.h); the source form behind the call, probably a shared
         * inline wrapper, is unknown. Revisit when one of those callers or the wrapper is
         * recovered. */
        fn_80021B08();
        if (fn_801BFC28(0, &position, &movement, &m_sweepFlags, 0, 10.0f)) {
            m_charInfo->m_position = position;
            m_charInfo->field_0x18 = movement * (1.0f / dt);
            m_state = STATE_BROKEN;
        }
    }

    m_previousPosition = m_charInfo->m_position;
    fn_8040CE60(&m_charInfo->field_0x24);
}

} // namespace Container

/* The descriptor's first hook: adds CONTAINERSECRET.BIN to the resource list (fn_8006B474 with
 * 1). */
extern "C" void fn_80278354(void) {
    fn_8006B474("CONTAINERSECRET.BIN", 1);
}

/* The descriptor's second hook: does nothing. */
extern "C" void fn_80278350(void) {}

void Container::ContainerSecret::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);
    boost::shared_ptr<GadgetPickup::PickupBase> unit =
        boost::shared_ptr<GadgetPickup::PickupBase>(new ContainerSecret());

    unit->Initialize(owner);
    unit->ResetFromOwner(owner);
    new GadgetPickup::SetPickupBaseTask(parent, owner, unit);
}

/* The label callback of the "weapon" parameter, the table's entry 0 (behaviour inferred): the
 * weapon type's name when fn_8005BF14 accepts the type and the name is neither null nor empty,
 * otherwise "N/A". Returns null for any other entry. The original returns "N/A" in two places,
 * one for a type without a name (USA (GUPE8P) 0x802780F0) and one for a rejected type
 * (0x802780F8), so each case has its own return. */
static const char *fn_8027807C(UnkParamCursor_800CBC20 *cursor) {
    static const char *s_notAvailable = "N/A";

    if (cursor->m_index == 0) {
        int type = cursor->m_value;

        if (fn_8005BF14(type)) {
            const char *name = fn_8005BEEC(type);

            if (name != 0 && strcmp(name, "") != 0) {
                return name;
            }
            return s_notAvailable;
        }
        return s_notAvailable;
    }

    return 0;
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "weapon", 1, 1, 0x47, 0.0f, 0.0f, 0.0f, fn_8027807C, 0},
    {0},
};

GadgetTypeInfo lbl_8054D744 = {
    "SECRET CONTAINER",
    fn_80278354,
    fn_80278350,
    Container::ContainerSecret::Create,
    0,
    0,
    0x3A,
    1,
    0,
    10,
    s_paramInfo,
};
