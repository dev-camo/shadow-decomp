/* The CONTAINER gadget (classes described in Game/Container.h). File name inferred from the
 * namespace. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first).
 *
 * The container's two character info handlers and its attack callback are boost::function objects
 * built with boost::bind(&ContainerNormal::method, this, _1); their manager, invoker and copy
 * functions come from the boost headers. */
#include <cstring>
#include "Game/Container.h"
#include "Game/AngleConversion.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/ItemBox.h"
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

/* 45 degrees in radians, set by the file's static initialiser; nothing in this file reads it. */
static float s_angle = DegreesToRadians(45.0f);

namespace Container {

ContainerNormal::ContainerNormal() : PickupBase(4) {
    fn_801DC984("ContainerNormal");
}

ContainerNormal::~ContainerNormal() {}

void ContainerNormal::Initialize(UnkGadgetOwner_800C9CD0 *owner) {
    m_flags.Clear();
    m_sweepFlags.Set(1);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    m_charInfo = boost::shared_ptr<PickupCharInfo>(new PickupCharInfo(4));
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&ContainerNormal::OnCommand, this, _1));
    m_charInfo->fn_801E4160(boost::weak_ptr<CharInfo>(m_charInfo), 0x80, 30.0f);
    m_charInfo->fn_801DC984("ContainerNormal");
    m_charInfo->SetField0x5C(4);
    m_charInfo->m_grabHandler = PickupCharInfo::SetGrabPositionHandler(
        boost::bind(&ContainerNormal::OnSetGrabPosition, this, _1));

    m_modelIndex = m_params.model;
    LoadModel(m_modelIndex);
    m_state = STATE_START;
    SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);
}

void ContainerNormal::Update(float dt) {
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
        /* The state is stored before the locked pointer is released (USA (GUPE8P) 0x800F7AB0),
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
        SpawnDebris();
        SpawnContents();
        if (boost::shared_ptr<CharInfo> breaker = m_breaker.lock()) {
            fn_8020642C(breaker, GetEventId());
        }
        m_flags.Set(FLAG_DONE);
        break;

    case STATE_VACUUMED:
        m_flags.Set(FLAG_DONE);
        break;
    }

    m_charInfo->fn_801E413C(&m_charInfo->m_position);
}

bool ContainerNormal::TestFlags(const UnkBitFlags_80014E04 &flags) {
    if (flags.Test(0) && m_flags.Test(FLAG_PICKED)) {
        return true;
    }
    if (flags.Test(1) && m_flags.Test(FLAG_DONE)) {
        return true;
    }
    return false;
}

void ContainerNormal::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    s32 previousModel = m_params.model;

    if (m_charInfo.get() != 0) {
        fn_800C9F14(owner, &m_charInfo->m_position);
        fn_800C9ED4(owner, &m_charInfo->m_rotation);
        m_charInfo->m_position.y += 10.0f;
    }
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    if (previousModel != m_params.model) {
        ReleaseModel();
        m_modelIndex = m_params.model;
        LoadModel(m_modelIndex);
    }

    SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);
    m_state = STATE_START;
}

void ContainerNormal::SetTransform(const Fn8040Vector3 *position, const UnkRotation3 *rotation) {
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
    if (m_behavior.get() != 0) {
        m_behavior->SetTransform(position, rotation);
    }
}

void ContainerNormal::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x11: /* PickupCommand */
        if (m_state == STATE_WAITING && !m_flags.Test(FLAG_PICKED)) {
            PickupCommand &pickup = static_cast<PickupCommand &>(command);

            field_0x10C = 0.0f;
            field_0xAC = pickup.field_0x2C;
            field_0xA8 = -320.0f;
            field_0x110 = pickup.field_0x20;
            field_0xC4 = m_charInfo->m_position;
            field_0xD0 = pickup.field_0x14 + pickup.field_0x2C * 10.0f;
            m_picker = pickup.m_picker;
            m_charInfo->m_target->ClearFlags(1);
            m_model->fn_80161230(0, -1);
            m_behavior.reset();
            m_state = STATE_PICKED;
            m_flags.Set(FLAG_PICKED);
            m_flags.Set(FLAG_MOVING);
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
            m_model->fn_80161880();

            UnkRotation3 angles(0.0f, 0.0f, 0.0f);
            fn_800494F8(&angles, &m_grabMatrix);

            Fn8040Vector3 acceleration = field_0xAC * field_0xA8;
            fn_8027B338(thrown.field_0x14, m_model, m_params.model, m_params.select, m_params.value,
                        &m_charInfo->m_position, &angles, &m_charInfo->field_0x18, acceleration,
                        GetEventId(), 0.0f, 1.0f, 600.0f);
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
            fn_80254384(static_cast<VacuumCommand *>(&command), m_model, m_params.model,
                        m_params.select, m_params.value, &m_position, &m_rotation, GetEventId());
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

void ContainerNormal::OnSetGrabPosition(PickupCharInfo::SetGrabPositionArgs &args) {
    fn_8040C148(&m_grabMatrix, args.m_matrix);

    if (m_state == STATE_HELD) {
        UpdateHeldPosition();
    }
}

void ContainerNormal::UpdateHeldPosition() {
    m_charInfo->m_position = fn_800496F8(&m_grabMatrix);
    PickupBase::SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);

    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &m_grabMatrix);
    fn_8040B52C(&matrix, 0.0f, 10.0f, 0.0f);
    m_model->fn_801633DC(&matrix);
}

void ContainerNormal::SpawnDebris() {
    fn_8033E644(m_model, m_params.model, &m_charInfo->m_position, &m_charInfo->m_rotation);
}

void ContainerNormal::OnAttack(AttackCallbackParam &param) {
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

void ContainerNormal::UpdateMotion(float dt) {
    if (m_flags.Test(FLAG_ACCELERATE)) {
        AddVector(&m_charInfo->field_0x24, field_0xAC * field_0xA8);
    }
    fn_804103E8(&m_charInfo->m_position, &m_charInfo->field_0x18, &m_charInfo->field_0x24, 1.0f,
                dt);

    if (m_flags.Test(FLAG_SWEEP)) {
        Fn8040Vector3 movement = m_charInfo->m_position - m_previousPosition;
        Fn8040Vector3 position = m_previousPosition;

        /* Matching workaround: the world getter is called on its own and its result is unused.
         * The original calls it at this point and discards r3 (USA (GUPE8P) 0x800F700C, before
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

void ContainerNormal::SpawnContents() {
    fn_8033E828(m_params.select, m_params.value, &m_charInfo->m_position, &m_charInfo->m_rotation);
}

void ContainerNormal::LoadModel(int index) {
    m_model = fn_8015F9D8(0, 0xCA);
    m_model->fn_80162A68(index, 1);
    m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo));

    boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161428(0);
    if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 =
            AttackCallback(boost::bind(&ContainerNormal::OnAttack, this, _1));
    }
}

void ContainerNormal::ReleaseModel() {
    m_behavior.reset();

    if (m_model.get() != 0) {
        m_model->fn_80161880();
        m_model->fn_80162804();
    }
}

} // namespace Container

/* The descriptor's first hook: adds the resource 0xCA to the resource list (fn_8006B50C with
 * 1). */
extern "C" void fn_800F688C(void) {
    fn_8006B50C(0xCA, 1);
}

/* The descriptor's second hook: does nothing. */
extern "C" void fn_800F6888(void) {}

void Container::ContainerNormal::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);
    boost::shared_ptr<GadgetPickup::PickupBase> unit =
        boost::shared_ptr<GadgetPickup::PickupBase>(new ContainerNormal());

    unit->Initialize(owner);
    unit->ResetFromOwner(owner);
    new GadgetPickup::SetPickupBaseTask(parent, owner, unit);
}

static const char *s_selectLabels[6] = {"None", "Item", "Weapon", "Ring", "HealUnit", "EnergyCore"};
static const char *s_energyCoreLabels[3] = {"Hero", "Dark", "N/A"};

/* The label callback of the "select" and "value" parameters (behaviour inferred). For "select" it
 * stores the value and returns its label. For "value" it reads the "select" value stored last:
 * for 1 (Item) and 2 (Weapon, only when fn_8005BF14 accepts the type) it returns the type's name
 * when that is not null or empty; for 3 (Ring) it returns "Ring Num"; for 5 (EnergyCore) and a
 * value of 0 or 1 it returns "Hero" or "Dark". Every other case returns "N/A". */
static const char *fn_800F64D0(UnkParamCursor_800CBC20 *cursor) {
    static const char *s_ringNum = "Ring Num";
    static const char *s_notAvailable = "N/A";
    static s32 s_select = 0;

    if (cursor->m_index == 1) {
        s_select = cursor->m_value;
        return s_selectLabels[s_select];
    } else if (cursor->m_index == 2) {
        /* Matching workaround: the empty branches for 0 (None) and 4 (HealUnit), which have no
         * value label, reproduce the original's tests of those values (USA (GUPE8P) 0x800F656C
         * and 0x800F65F8, each branching to the "N/A" return); a switch or explicit returns
         * compile differently. Revisit if another gadget's label callback shows the source
         * form. */
        if (s_select == 0) {
        } else if (s_select == 1) {
            const char *name = fn_8010B7F8(cursor->m_value);

            if (name != 0 && strcmp(name, "") != 0) {
                return name;
            }
        } else if (s_select == 2) {
            int type = cursor->m_value;

            if (fn_8005BF14(type)) {
                const char *name = fn_8005BEEC(type);

                if (name != 0 && strcmp(name, "") != 0) {
                    return name;
                }
            }
        } else if (s_select == 3) {
            return s_ringNum;
        } else if (s_select == 4) {
        } else if (s_select == 5) {
            if (cursor->m_value >= 0 && cursor->m_value <= 1) {
                return s_energyCoreLabels[cursor->m_value];
            }
        }
        return s_notAvailable;
    }

    return s_notAvailable;
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "model", 0, 0, 2, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {1, "select", 0, 0, 6, 0.0f, 0.0f, 0.0f, fn_800F64D0, 0},
    {1, "value", 0, 0, 0xE10, 0.0f, 0.0f, 0.0f, fn_800F64D0, 0},
    {0},
};

GadgetTypeInfo lbl_80525470 = {
    "CONTAINER", fn_800F688C, fn_800F6888, Container::ContainerNormal::Create, 0, 0, 9, 1,
    0,           10,          s_paramInfo,
};
