/* The Stick gadget (class described in Game/Stick.h). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The attack object's callback is a boost::function object built with
 * boost::bind(&Stick::OnAttack, this, _1); its manager, invoker and copy functions come from the
 * boost headers. */
#include <cstring>
#include "Game/Stick.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SetWeapon.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/WeaponItem.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "Game/fn_804157E8_motion_control.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The height added to the character info's position for fn_8011E938's input position (name
 * inferred).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original type of this variable.
 * The original sets the value with a start-up function in .ctors (a constant float initialiser
 * would be emitted as initialised .sdata data instead), and the shared stand-in reproduces that
 * function. Revisit when the original type is found.
 * Defined at the top of the file: a definition just before UpdateBody, its only user, places
 * the variable in .sbss after the three one-byte objects of the boost::function templates, where
 * the original has it before them (USA (GUPE8P) 0x805EED74). */
static UnkStartupFloat_800E2570 s_weaponHeight(20.0f);

namespace Stick {

const char *Stick::s_name = "Stick";

Stick::Stick(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x45, 4, "Stick");
    ReadOwner(m_owner);

    m_loadedModel = m_params.model;
    m_model = fn_8015F800(1, "STICK.BIN");
    m_resource = static_cast<UnkResource_80012700 *>(fn_80012700(fn_800122D4(), 1, "STICK.BIN"));
    if (!GadgetBuilder::fn_80208BB8(m_resource, m_params.model)) {
        fn_8015A794(this);
        m_flags |= 1;
        return;
    }

    m_crashInfo = GadgetBuilder::fn_80208978(m_resource, m_params.model);
    m_model->fn_801629D8(m_crashInfo->fn_802087D8(), 0);

    const char *entryName = m_crashInfo->fn_802087D8();
    const char *nodeName = m_crashInfo->fn_802087BC();
    bool attached = false;

    if (m_model->fn_80160BA0(entryName)) {
        m_model->fn_8015FF60();
        m_model->fn_80160960(entryName, 1);
        m_motionFlags.Set(0);
        if (m_model->fn_80161AB8(nodeName, boost::weak_ptr<CharInfo>(m_charInfo), entryName)) {
            attached = true;
        }
    } else {
        m_motionFlags.Reset(0);
        if (m_model->fn_80161F08(nodeName, boost::weak_ptr<CharInfo>(m_charInfo))) {
            attached = true;
        }
    }

    if (attached) {
        boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161334(nodeName);

        if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
            attack->field_0x54 = 0;
            attack->field_0x55 = 1;
            attack->field_0x1C.field_0x0 = AttackCallback(boost::bind(&Stick::OnAttack, this, _1));
        }
    }

    UpdateModelMatrix();
}

Stick::~Stick() {
    m_sound.fn_801D455C();
}

void Stick::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case STATE_RESET: {
        m_state = STATE_INTACT;

        if (m_model.get() != 0) {
            m_target = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;
            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        }

        if (m_target.get() != 0) {
            m_target->fn_80167180(-1);
        }

        UnkResourceRecord_80208848 *record = fn_80208888(m_resource, m_crashInfo->fn_802087D8());
        if (record != 0) {
            Fn8040Vector3 position(record->field_0x10, record->field_0x14, record->field_0x18);

            AddVector(&position, m_charInfo->m_position);
            m_sound = fn_801D3E58(static_cast<u16>(record->field_0x8), &position, this);
        }
        break;
    }
    case STATE_INTACT:
        if (m_motionFlags.Test(0)) {
            boost::shared_ptr<_stMtnPlayWork> motion =
                m_model->fn_801601A0(m_crashInfo->fn_802087D8()).lock();

            if (motion.get() != 0) {
                _stMtnPlayWork *control = motion.get();
                unsigned short eventId;
                void *payload;

                while (fn_80415854(control, 0, &eventId, &payload, 0)) {
                    if (eventId == 0x5001) {
                        u32 *sound = static_cast<u32 *>(payload);

                        fn_801D54F4(fn_80053164(), static_cast<u16>(*sound),
                                    &m_charInfo->m_position, 0, 1);
                    }
                    control = 0;
                }
            }
        }
        break;
    case STATE_HIT:
        StartCrashEffect();

        if (m_target.get() != 0) {
            m_target.reset();
        }

        m_sound.fn_801D455C();

        /* Matching workaround: m_timer is set in both branches because the original has a
         * separate lfs/stfs pair in each (USA (GUPE8P) 0x801227C4 and 0x801227E0, the second
         * after the fn_8015A794 call); a single store after the if/else makes UpdateBody 256
         * words instead of 259. Revisit if the original form of this case is found. */
        if (ShowBrokenModel()) {
            m_timer = 0.5f;
        } else {
            m_model.reset();
            fn_8015A794(this);
            m_timer = 0.5f;
            m_state = STATE_NO_MODEL;
        }
        break;
    case STATE_BREAK_MOTION: {
        boost::shared_ptr<_stMtnPlayWork> motion =
            m_model->fn_801601A0(m_crashInfo->fn_802087A0()).lock();

        if (motion.get() != 0 && stMtnControlWorkFlag(motion.get(), 0, 1, 1) == 0 &&
            stMtnControlWorkFlag(motion.get(), 0, 3, 1) != 0) {
            m_model->fn_80160400(m_crashInfo->fn_802087A0());
            m_state = STATE_DROP_WAIT;
        }
        break;
    }
    case STATE_DROP_WAIT:
        m_timer -= dt;
        if (m_timer <= 0.0f) {
            Fn8040Vector3 position = m_charInfo->m_position;
            position.y += s_weaponHeight.value;
            fn_8011E938(m_params.weapon, &position, &m_charInfo->m_rotation);
            m_state = STATE_DROPPED;
        }
        break;
    case STATE_NO_MODEL:
        m_timer -= dt;
        if (m_timer <= 0.0f) {
            m_flags |= 1;

            Fn8040Vector3 position = m_charInfo->m_position;
            position.y += s_weaponHeight.value;
            fn_8011E938(m_params.weapon, &position, &m_charInfo->m_rotation);
        }
        break;
    }

    UpdateModelMatrix();
}

void Stick::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    s32 previousModel = m_params.model;

    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() == 0) {
        return;
    }

    if (m_params.model != previousModel) {
        m_model->fn_80161880();
        m_model->fn_80162804();
        m_crashInfo.reset();
        m_sound.fn_801D455C();
        if (!GadgetBuilder::fn_80208BB8(m_resource, m_params.model)) {
            return;
        }

        m_crashInfo = GadgetBuilder::fn_80208978(m_resource, m_params.model);
        m_loadedModel = m_params.model;
        m_model->fn_801629D8(m_crashInfo->fn_802087D8(), 0);

        if (m_model->fn_80161F08(m_crashInfo->fn_802087BC(),
                                 boost::weak_ptr<CharInfo>(m_charInfo))) {
            boost::weak_ptr<CharaColliAttack> weakAttack = m_model->fn_80161428(m_loadedModel);

            if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
                attack->field_0x54 = 0;
                attack->field_0x55 = 1;
                attack->field_0x1C.field_0x0 =
                    AttackCallback(boost::bind(&Stick::OnAttack, this, _1));
            }
        }
    }

    m_model->fn_8016108C();
}

void Stick::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_state = STATE_RESET;
    m_timer = 0.0f;
}

void Stick::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        m_model->fn_801633DC(&matrix);
    }
}

void Stick::OnAttack(AttackCallbackParam &param) {
    if (m_state == STATE_INTACT) {
        boost::weak_ptr<CharInfo> attacker;

        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if ((attack->field_0x40 & 2) != 0) {
                param.field_0x54 = 5;
                return;
            }
            attacker = attack->m_owner;
        }

        if (boost::shared_ptr<CharInfo> breaker = attacker.lock()) {
            fn_8020642C(breaker, 0x23);
        }

        param.field_0x54 = 3;
        m_state = STATE_HIT;
    } else {
        param.field_0x54 = 5;
    }
}

void Stick::StartCrashEffect() {
    if (m_crashInfo.get() != 0) {
        const UnkCrashEntry_802086C0 *entry = m_crashInfo->fn_802086C0();

        if (entry != 0) {
            RpClump *clump = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, entry->fileName));

            if (clump != 0) {
                UnkEffectParams_8015F0B8 parameters(m_charInfo, 0);

                parameters.field_0x4.x += entry->field_0x18.x;
                parameters.field_0x4.y += entry->field_0x18.y;
                parameters.field_0x4.z += entry->field_0x18.z;
                parameters.field_0x34 = 1.3f;
                parameters.field_0x44 = clump;
                parameters.field_0x54 = 1;
                parameters.field_0x4C = 150;
                fn_8015D93C(&parameters);
            }
        }

        const UnkResourceRecord_80208848 *record = m_crashInfo->fn_80208618();

        if (record != 0) {
            Fn8040Vector3 position = m_charInfo->m_position;
            position.x += record->field_0x10;
            position.y += record->field_0x14;
            position.z += record->field_0x18;
            fn_801D54F4(fn_80053164(), record->field_0x8, &position, 0, 1);
        }
    }
}

bool Stick::ShowBrokenModel() {
    if (m_crashInfo.get() != 0 && m_crashInfo->fn_802087A0() != 0) {
        m_model->fn_80161880();
        m_model->fn_80162804();
        m_model->fn_801629D8(m_crashInfo->fn_802087A0(), 0);
        UpdateModelMatrix();

        if (m_model->fn_80160BA0(m_crashInfo->fn_802087A0())) {
            m_model->fn_8015FF60();
            m_model->fn_80161AB8(m_crashInfo->fn_80208784(), boost::weak_ptr<CharInfo>(m_charInfo),
                                 m_crashInfo->fn_802087A0());
            m_model->fn_801609F8(m_crashInfo->fn_802087A0());
            UpdateModelMatrix();
            m_state = STATE_BREAK_MOTION;
        } else if (m_crashInfo->fn_80208784() != 0) {
            m_model->fn_80161F08(m_crashInfo->fn_80208784(), boost::weak_ptr<CharInfo>(m_charInfo));
            m_state = STATE_DROP_WAIT;
        } else {
            /* Matching workaround: the store is written in both branches because the original
             * has a separate li/stw pair in each (USA (GUPE8P) 0x80121C70 and 0x80121C7C, the
             * first after the destructor of the attach call's weak_ptr temporary); a single store
             * after the attach makes the function 117 words instead of 120. Revisit if the
             * original form of this branch is found. */
            m_state = STATE_DROP_WAIT;
        }

        UnkResourceRecord_80208848 *record = fn_80208888(m_resource, m_crashInfo->fn_802087A0());
        if (record != 0) {
            Fn8040Vector3 position(record->field_0x10, record->field_0x14, record->field_0x18);

            AddVector(&position, m_charInfo->m_position);
            m_sound = fn_801D3E58(static_cast<u16>(record->field_0x8), &position, this);
        }
        return true;
    }

    return false;
}

} // namespace Stick

extern "C" void fn_80121B04(void) {
    fn_8006B474("STICK.BIN", 1);
}

extern "C" void fn_80121B00(void) {}

void Stick::Stick::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new Stick(fn_801EB304(fn_8001C4F8(), 11), owner);
}

/* The label callback of the "weapon" parameter: for the cursor's entry 1, the name that
 * fn_8005BEEC gives for the value, or "Not Used" when that name is empty (the name is not checked
 * for null); null for any other entry (behaviour inferred). */
static const char *fn_80121A2C(UnkParamCursor_800CBC20 *cursor) {
    static const char *s_notUsed = "Not Used";

    if (cursor->m_index == 1) {
        const char *name = fn_8005BEEC(cursor->m_value);

        if (strcmp(name, "") == 0) {
            return s_notUsed;
        }
        return name;
    }

    return 0;
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "model", 0, 0, 0x10000, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {1, "weapon", 1, 1, 71, 0.0f, 0.0f, 0.0f, fn_80121A2C, 0},
    {0},
};

GadgetTypeInfo lbl_80527A10 = {
    "STICK", fn_80121B04, fn_80121B00, Stick::Stick::Create, 0, 0, 0x12C, 1, 0, 10, s_paramInfo,
};
