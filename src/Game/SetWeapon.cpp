/* SetWeapon (class names from RTTI; the gadget's descriptor is "SET_WEAPON"): weapons lying in the
 * stage. SetWeaponTask places a weapon of the type its "weapon" parameter selects; fn_8011E630 and
 * fn_8011E938 put weapons down for other objects, each kept by a WeaponTask for 30 seconds. A
 * weapon is an Arms: the weapon item, the model 0xDF and a character info that hands the weapon
 * to the character that uses it (behaviour inferred). File name inferred from the namespace.
 * Method names marked "(name inferred)" in the header are not original. Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first).
 *
 * The character info's command handler is a boost::function object built with
 * boost::bind(&Arms::OnCommand, this, _1); its manager, invoker and copy functions come from the
 * boost headers. */
#include <cstring>
#include "Game/SetWeapon.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharaColliBase.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "Game/fn_8003B8F4.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_80169BE4.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::CharaColliBase;

/* The number of the weapon's model: Arms's constructor loads it and the descriptor's load hook
 * (fn_8011EC80) adds it to the resource list (name inferred). */
static const int kArmsModelId = 0xDF;

namespace SetWeapon {

Arms::Arms(const boost::shared_ptr<Weapon::Item> &item, u32 bit) {
    m_charInfo = fn_8015CD90(0x34, 4);
    m_charInfo->m_commandUnit = boost::shared_ptr<CharCommandUnit>(new CharCommandUnit());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&Arms::OnCommand, this, _1));

    m_model = fn_8015F9D8(0, kArmsModelId);
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo));

    m_flags.Clear();
    m_item = item;
    if (bit != 0) {
        m_flags.Set(bit);
    }

    if (m_item.get() != 0) {
        Fn8040Vector2 size = fn_8005B898(m_item->GetType());
        Fn8040Vector3 offset(0.0f, 5.0f, 0.0f);
        boost::weak_ptr<CharaColliBase> probe = m_model->fn_80161614(0);

        if (boost::shared_ptr<CharaColliBase> collision = probe.lock()) {
            collision->SetSize(size.x, -2, 0.0f, size.y);
            collision->UnkSlot17(&offset, -2);
        }

        m_charInfo->field_0x34 = m_item->m_type;
    }
}

/* Matching workaround: auto_inline around this destructor. The original keeps checked_delete<T>
 * out of line and emits this destructor at its definition; with the stock GC/2.7 compiler and
 * -inline noauto that needs auto_inline here. A tooling constraint, not a claim about the original
 * source; revisit if another compiler build reproduces this without the pragma, or if an original
 * checked_delete keeps out of line a destructor whose body calls only non-inline functions (the
 * pragma would inline that destructor into it). A destructor found inlined into checked_delete in
 * the original would instead mean that the original used auto_inline here. */
#pragma auto_inline on
Arms::~Arms() {}
#pragma auto_inline reset

void Arms::Update(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_item.get() != 0) {
        m_item->Update(dt);
    }
}

bool Arms::IsFarFromPlayers(float squaredDistance) {
    return fn_80169BE4(&m_charInfo->m_position, squaredDistance);
}

void Arms::SetTransform(const Fn8040Vector3 *position, UnkRotation3 *rotation) {
    m_charInfo->m_position = *position;
    m_charInfo->m_rotation = *rotation;
    UpdateModels();
}

void Arms::UpdateModels() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        m_model->fn_801633DC(&matrix);
    }

    if (m_item.get() != 0 && !m_flags.Test(1)) {
        Fn8040Vector3 position = m_charInfo->m_position;
        position.y += 5.0f;
        m_item->fn_8005F274(&position, &m_charInfo->m_rotation);
    }
}

void Arms::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0xD:
        if (!IsTaken() && m_item.get() != 0) {
            if (boost::shared_ptr<CharInfo> user =
                    static_cast<UseCommand &>(command).m_user.lock()) {
                user->fn_801DC980(
                    new Player::AddWeaponCommand(m_item->m_type, m_item->field_0xC, 4));
                if (m_model.get() != 0) {
                    m_model->fn_80161A24(0);
                }
                m_flags.Set(0);
            }
        }
        break;
    }
}

void Arms::fn_8011F350() {
    if (m_item.get() != 0) {
        m_item->fn_8005F09C();
    }
}

const char *WeaponTask::s_name = "WeaponTask";

const char *SetWeaponTask::s_name = "SetWeaponTask";

WeaponTask::WeaponTask(Task *parent, boost::shared_ptr<Arms> arms) : Task(parent) {
    m_name = s_name;
    m_arms = arms;
    m_timer = 30.0f;
    m_state.Clear();
}

WeaponTask::~WeaponTask() {}

void WeaponTask::Update(float dt) {
    m_timer -= dt;
    if (m_timer <= 0.0f) {
        m_state.Set(2);
    } else if (m_timer <= 3.0f) {
        m_state.Set(3);
    }

    if (!m_state.Test(4) && m_state.Test(3)) {
        m_arms->fn_8011F350();
        m_state.Set(4);
    }

    if (m_arms.get() != 0) {
        if (m_arms->IsFarFromPlayers(1000000.0f)) {
            m_state.Set(2);
        } else if (m_arms->IsTaken()) {
            m_state.Set(2);
        }
    }

    if (m_state.Test(2)) {
        m_flags |= 1;
    } else {
        m_arms->Update(dt);
    }
}

SetWeaponTask::SetWeaponTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_weaponParam(1) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x34, 4);
    ReadOwner(owner);

    boost::shared_ptr<Weapon::Item> item = fn_8005B9D4(m_weaponType, -1);

    if (item.get() != 0) {
        m_arms = boost::shared_ptr<Arms>(new Arms(item, 0));
        UpdateArms();
    } else {
        fn_8015A794(this);
        m_flags |= 1;
    }
}

SetWeaponTask::~SetWeaponTask() {}

void SetWeaponTask::UpdateBody(float dt) {
    if (m_arms.get() != 0) {
        if (m_arms->IsTaken()) {
            if (m_owner != 0) {
                fn_800C9B34(m_owner);
            }
            m_flags |= 1;
        } else {
            m_arms->Update(dt);
        }
    }
}

void SetWeaponTask::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    s32 previousType = m_weaponType;

    ReadOwner(owner);
    UpdateArms();

    if (previousType != m_weaponType && fn_8005BF14(m_weaponType)) {
        const char *name = fn_8005BEEC(m_weaponType);

        if (name != 0 && strcmp(name, "") != 0) {
            m_arms.reset();
            m_arms = boost::shared_ptr<Arms>(new Arms(fn_8005B9D4(m_weaponType, -1), 0));
        }
    }
}

void SetWeaponTask::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_weaponParam, sizeof(m_weaponParam));
    m_weaponType = m_weaponParam;
    m_charInfo->field_0x34 = m_weaponParam;
}

void SetWeaponTask::UpdateArms() {
    if (m_arms.get() != 0) {
        m_arms->SetTransform(&m_charInfo->m_position, &m_charInfo->m_rotation);
    }
}

} // namespace SetWeapon

extern "C" void fn_8011EC80(void) {
    fn_8006B50C(kArmsModelId, 1);
}

extern "C" void fn_8011EC7C(void) {}

void SetWeapon::SetWeaponTask::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new SetWeaponTask(fn_801EB304(fn_8001C4F8(), 11), owner);
}

/* The label callback of the "weapon" parameter: the name of the weapon type the value selects, or
 * "Not Used" when the type is not available or its name is empty (behaviour inferred). */
static const char *fn_8011EB88(UnkParamCursor_800CBC20 *cursor) {
    static const char *s_notUsed = "Not Used";

    if (cursor->m_index == 0) {
        int type = cursor->m_value;

        if (fn_8005BF14(type)) {
            const char *name = fn_8005BEEC(type);

            if (name != 0 && strcmp(name, "") != 0) {
                return name;
            }
            return s_notUsed;
        }
        return s_notUsed;
    }

    return 0;
}

/* The probe filter of fn_8011E938: accepts the records with bit 0 of the flags set (behaviour
 * inferred). */
static bool fn_8011EB7C(const UnkProbeFilterRecord *record) {
    return record->flags & 1;
}

extern "C" void fn_8011E938(int type, const Fn8040Vector3 *position, UnkRotation3 *rotation) {
    Fn8040Vector3 ground = *position;
    Fn8040Vector3 start;
    Fn8040Vector3 end;

    start = end = *position;
    end.y -= 1000.0f;

    UnkSegment_80020A20 segment(start, end);
    UnkProbeNode_80021FE4 *hit = fn_80021E24(fn_80020D40(), &segment, 3, 8, fn_8011EB7C, 0);

    if (hit != 0) {
        if (hit->hit != 0) {
            ground = *fn_8001E9BC(hit->hit);
        }
        delete hit;
    }

    boost::shared_ptr<Weapon::Item> item = fn_8005B9D4(type, -1);

    if (item.get() != 0) {
        boost::shared_ptr<SetWeapon::Arms> arms =
            boost::shared_ptr<SetWeapon::Arms>(new SetWeapon::Arms(item, 0));

        arms->SetTransform(&ground, rotation);

        /* The parent is looked up before the allocation here (unlike in Create). */
        Task *parent = fn_801EB304(fn_8001C4F8(), 11);

        new SetWeapon::WeaponTask(parent, arms);
    }
}

extern "C" void fn_8011E630(const boost::shared_ptr<Weapon::Item> &item,
                            const Fn8040Vector3 *position, const Fn8040Vector3 *direction) {
    UnkRotation3 rotation(0.0f, 0.0f, 0.0f);

    fn_8003B8F4(direction, &rotation.x, &rotation.z);

    boost::shared_ptr<SetWeapon::Arms> arms =
        boost::shared_ptr<SetWeapon::Arms>(new SetWeapon::Arms(item, 1));

    arms->SetTransform(position, &rotation);

    /* The parent is looked up before the allocation here (unlike in Create). */
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new SetWeapon::WeaponTask(parent, arms);
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "weapon", 1, 1, 71, 0.0f, 0.0f, 0.0f, fn_8011EB88, 0},
    {0},
};

GadgetTypeInfo lbl_80527734 = {
    "SET_WEAPON", fn_8011EC80, fn_8011EC7C, SetWeapon::SetWeaponTask::Create, 0, 0, 0x20, 1, 0,
    10,           s_paramInfo,
};
