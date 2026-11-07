/* PowerDeviceNaked (class name from RTTI): the gadget of a power device (model file
 * POWERDEVICENAKED.BIN) built from two models, a base and a core. The core takes damage from
 * attacks, shows its damage level in its colours and breaks when its health runs out; the health
 * and the broken flag are kept in a PowerDeviceNakedSetState (class name from RTTI, derived from
 * SetAdapterState) that the owner remembers (behaviour inferred). File name inferred from the
 * class. Method and member names marked "(name inferred)" are not original. Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/CharInfo.h"
#include "Game/CharaColliAttack.h"
#include "Game/DebugDisp.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SetAdapterState.h"
#include "Game/Target.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_80009548.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkModelAtomicState.h"
#include "Game/UnkObject_802B4254.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The gadget parameters in the order of the descriptor's table ("MODEL 0:Out 1:In"). */
struct PowerDeviceNakedParams {
    /* 0x0 */ u32 model; /* 0 Out or 1 In; passed to the base model's fn_80162A68 */
};
CHECK_SIZEOF(PowerDeviceNakedParams, 0x4);

class PowerDeviceNaked : public Gadget {
  public:
    PowerDeviceNaked(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~PowerDeviceNaked();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, the rotation, the parameters and the tag from the owner (name
     * inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When both models exist, places them at the character info, m_object above it and, while
     * there is one, m_object2 above it (name inferred). */
    void UpdateModelMatrix();
    /* Gives each core part that fn_80162298 finds the colour of the damage level and passes the
     * level's id to m_target when there is one (name inferred). */
    void ApplyDamageLevel();
    /* Enters state 0 (the core is intact and can be attacked) or state 1 (the core is broken)
     * (name inferred). */
    void EnterState(int state);
    /* The core attack object's callback (behaviour inferred): an accepted attack takes the
     * attack's value at 0x50 from the core's health. */
    void OnAttack(AttackCallbackParam &param);

    static const char *s_name;

    /* The names of the members below are inferred. */
    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<Target> m_object;
    /* Present while the core is intact. */
    /* 0x40 */ boost::shared_ptr<UnkObject_802B4254> m_object2;
    /* 0x48 */ boost::shared_ptr<GadgetModelInfo> m_model; /* the base */
    /* 0x50 */ boost::shared_ptr<GadgetModelInfo> m_coreModel;
    /* 0x58 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x60 */ float m_flashTimer; /* seconds of blinking left after a hit */
    /* 0x64 */ s32 m_state;        /* 0 intact, 1 broken */
    /* Bit 0: the core has just been broken; bit 1: the core blinks after a hit; bit 2: the damage
     * level has changed. */
    /* 0x68 */ UnkBitFlags_80014E04 m_flags;
    /* 0x6C */ PowerDeviceNakedParams m_params;
    /* 0x70 */ u8 m_tag; /* copied from the owner; passed to fn_8016907C */
    /* 0x74 */ UnkSoundHandle_801D3DC8 m_sound;
};
CHECK_SIZEOF(PowerDeviceNaked, 0x90);

/* The ids that ApplyDamageLevel passes to m_target for each damage level (name inferred). */
static const s32 s_levelTargetIds[4] = {0, 1, 2, 3};

/* The state that the owner remembers between the creations of the gadget (class name from RTTI,
 * derived from SetAdapterState; member names inferred): the core's health, 12 when full, and
 * whether the core has been broken. */
class PowerDeviceNakedSetState : public SetAdapterState {
  public:
    PowerDeviceNakedSetState() : m_health(12.0f), m_broken(false) {}

    virtual ~PowerDeviceNakedSetState() {}

    /* 0x4 */ float m_health;
    /* 0x8 */ bool m_broken;
};
CHECK_SIZEOF(PowerDeviceNakedSetState, 0xC);

/* Values that the original builds at startup (names inferred from their uses): the height of
 * m_object above the character info, the colours of the two core parts for each damage level,
 * the height of m_object2 above the character info and the value passed to its fn_802B4498. */
static UnkStartupFloat_800E2570 s_objectHeight(35.0f);
static UnkDebugColor_802E0E84 s_part0Colors[4] = {
    UnkDebugColor_802E0E84(0xFF, 0x13, 0xFF, 0x68),
    UnkDebugColor_802E0E84(0xFF, 0xFF, 0xFC, 0x00),
    UnkDebugColor_802E0E84(0xFF, 0xFF, 0x71, 0x00),
    UnkDebugColor_802E0E84(0xFF, 0xFF, 0x00, 0x00),
};
static UnkDebugColor_802E0E84 s_part1Colors[4] = {
    UnkDebugColor_802E0E84(0xFF, 0x08, 0x79, 0x32),
    UnkDebugColor_802E0E84(0xFF, 0x7E, 0x76, 0x05),
    UnkDebugColor_802E0E84(0xFF, 0x7E, 0x3B, 0x00),
    UnkDebugColor_802E0E84(0xFF, 0x7E, 0x03, 0x00),
};
static UnkStartupFloat_800E2570 s_object2Height(54.0f);
static UnkStartupFloat_800E2570 s_object2Value(200.0f);

/* The damage level for the core's health (name inferred): 0 from three quarters of the full
 * health of 12 up, 3 below one quarter. */
static inline u32 GetDamageLevel(float health) {
    float ratio = health / 12.0f;

    if (ratio < 0.25f) {
        return 3;
    }
    if (ratio < 0.5f) {
        return 2;
    }
    if (ratio < 0.75f) {
        return 1;
    }
    return 0;
}

const char *PowerDeviceNaked::s_name = "PowerDeviceNaked";

extern "C" void fn_800D49B8(void) {
    fn_8006B474("POWERDEVICENAKED.BIN", 1);
}

extern "C" void fn_800D49B4(void) {}

void PowerDeviceNaked::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new PowerDeviceNaked(fn_801EB304(fn_8001C4F8(), 11), owner);
}

PowerDeviceNaked::PowerDeviceNaked(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_flashTimer(0.0f), m_state(0), m_flags(0) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x32, 4);
    m_charInfo->field_0x30 = 1;

    if (owner->m_adapterState == 0) {
        fn_800C9930(m_owner, new PowerDeviceNakedSetState);
    }

    ReadOwner(owner);
    m_model = fn_8015F800(1, "POWERDEVICENAKED.BIN");
    m_coreModel = fn_8015F800(1, "POWERDEVICENAKED.BIN");
    m_model->fn_80162A68(m_params.model, 0);
    m_coreModel->fn_801629D8("Core", 0);
    m_model->fn_80161F08("BaseColli", boost::weak_ptr<CharInfo>(m_charInfo));
    m_object = fn_801E72DC(0x2800);

    /* The original reads the flag here without a dynamic_cast (there is no __dynamic_cast call). */
    if (static_cast<PowerDeviceNakedSetState *>(owner->m_adapterState)->m_broken) {
        m_state = 1;
    } else {
        m_state = 0;
    }

    EnterState(m_state);
}

PowerDeviceNaked::~PowerDeviceNaked() {
    m_sound.fn_801D455C();
}

void PowerDeviceNaked::UpdateBody(float dt) {
    switch (m_state) {
    case 0:
        if (m_flags.Test(0)) {
            fn_801D54F4(fn_80053164(), 0x733B, &m_charInfo->m_position, 0, 1);

            UnkEffectParams_8015F0B8 effect(m_charInfo, 0);

            effect.field_0x44 =
                static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, "CN0402POWERDIVICEBRK.DFF"));
            effect.field_0x40 = m_charInfo->m_position.y;
            effect.field_0x34 = 1.5f;
            effect.field_0x4A = 0;
            effect.field_0x54 = 1;
            fn_8015D93C(&effect);

            fn_8000D664(fn_80009548(), 0, 4.0f, 1.0f, 20.0f);
            m_state = 1;
            EnterState(m_state);

            PowerDeviceNakedSetState *state =
                dynamic_cast<PowerDeviceNakedSetState *>(m_owner->m_adapterState);

            state->m_broken = true;
            fn_800C985C(m_owner);
            fn_8016907C(fn_800CB69C(), m_tag, 1);

            {
                boost::shared_ptr<Mission::Mission> mission = fn_800740E4(fn_8006D6A8(), 2).lock();

                if (mission.get() != 0) {
                    mission->fn_8016B224(1, 0);
                }
            }

            m_flags.Reset(0);
        }

        if (m_flags.Test(1)) {
            m_flashTimer -= dt;

            if (m_flashTimer < 0.0f) {
                m_coreModel->fn_8016271C();
                m_flags.Reset(1);
            } else if (lbl_8057D6A0.field_0x90 & 2) {
                m_coreModel->fn_80162790();
            } else {
                m_coreModel->fn_8016271C();
            }
        }

        if (m_flags.Test(2)) {
            ApplyDamageLevel();
            m_flags.Reset(2);
        }
        break;

    /* The original's compare tree also tests state 1, which has no work of its own here. */
    case 1:
        break;
    }

    UpdateModelMatrix();
    m_model->fn_80162FB0(dt);
    m_coreModel->fn_80162FB0(dt);

    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }
}

void PowerDeviceNaked::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    PowerDeviceNakedParams previous = m_params;

    ReadOwner(owner);

    if (m_params.model != previous.model) {
        m_model->fn_80162804();
        m_model->fn_80162A68(m_params.model, 0);
    }

    UpdateModelMatrix();
    m_model->fn_8016108C();
    m_coreModel->fn_8016108C();
    m_model->fn_80162EF4();
    m_coreModel->fn_80162EF4();
}

void PowerDeviceNaked::EnterState(int state) {
    switch (state) {
    case 0: {
        m_coreModel->fn_80162424(0);
        m_coreModel->fn_80162424(1);
        m_coreModel->fn_80160C28(0);
        m_coreModel->fn_8016082C(0);
        m_coreModel->fn_8015FF60();
        m_coreModel->fn_80161F08("CoreColli", boost::weak_ptr<CharInfo>(m_charInfo));

        {
            boost::shared_ptr<CharaColliAttack> attack =
                m_coreModel->fn_80161334("CoreColli").lock();

            if (attack.get() != 0) {
                attack->fn_80096B64(
                    1, AttackCallback(boost::bind(&PowerDeviceNaked::OnAttack, this, _1)));
            }
        }

        m_coreModel->fn_80160B18(0);
        m_object->SetFlags(1);
        m_object2 = fn_802B4254(2, 0);
        fn_802B4498(m_object2.get(), s_object2Value.value);
        fn_802B4510(m_object2.get(), 1);
        UpdateModelMatrix();
        m_sound = fn_801D3E58(0x7339, &m_charInfo->m_position, this);
        m_target = fn_80166AF4();

        {
            UnkWeakHolder_800CC6C8 weakTarget;

            weakTarget.ptr = m_target;
            m_coreModel->fn_8015FA78(fn_80169D00, &weakTarget);
        }

        ApplyDamageLevel();
        break;
    }

    case 1:
        m_coreModel->fn_80162790();

        if (m_target.get() != 0) {
            m_target->fn_80166F88();
        }

        m_coreModel->fn_80161880();
        m_coreModel->fn_801604FC();
        m_sound.fn_801D455C();
        m_object->ClearFlags(1);
        m_object2.reset();
        UpdateModelMatrix();
        break;
    }
}

void PowerDeviceNaked::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E60(owner, &m_tag);
}

void PowerDeviceNaked::UpdateModelMatrix() {
    if (m_model.get() != 0 && m_coreModel.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
        m_coreModel->fn_801633DC(&matrix);

        Fn8040Vector3 objectPosition = m_charInfo->m_position;
        objectPosition.y += s_objectHeight.value;
        fn_800CCE38(m_object.get(), &objectPosition);

        if (m_object2.get() != 0) {
            Fn8040Vector3 above(0.0f, s_object2Height.value, 0.0f);

            const Fn8040Vector3 &object2Position = above + m_charInfo->m_position;
            fn_802B44C8(m_object2.get(), &object2Position);
        }
    }
}

void PowerDeviceNaked::ApplyDamageLevel() {
    PowerDeviceNakedSetState *state =
        dynamic_cast<PowerDeviceNakedSetState *>(m_owner->m_adapterState);
    u32 level = GetDamageLevel(state->m_health);
    UnkModelAtomicState *part = m_coreModel->fn_80162298(0);

    if (part != 0) {
        part->color = s_part0Colors[level];
    }

    part = m_coreModel->fn_80162298(1);

    if (part != 0) {
        part->color = s_part1Colors[level];
    }

    if (m_target.get() != 0) {
        m_target->fn_80166F88();
        m_target->fn_80167010(s_levelTargetIds[level]);
    }
}

void PowerDeviceNaked::OnAttack(AttackCallbackParam &param) {
    if (m_state == 0 && !m_flags.Test(0) && fn_8015D4E4(&param)) {
        boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock();

        if (attack.get() != 0 && !(attack->field_0x40 & 2)) {
            PowerDeviceNakedSetState *state =
                dynamic_cast<PowerDeviceNakedSetState *>(m_owner->m_adapterState);

            state->m_health -= param.field_0x50;
            param.field_0x54 = 1;

            if (state->m_health < 0.0001f) {
                param.field_0x54 = 3;
                m_flags.Set(0);
                m_flags.Reset(1);

                boost::shared_ptr<CharInfo> attacker = fn_8015D570(&param);

                if (attacker.get() != 0) {
                    fn_8020642C(attacker, 0x42);
                }
            } else {
                if (GetDamageLevel(state->m_health + param.field_0x50) !=
                    GetDamageLevel(state->m_health)) {
                    m_flags.Set(2);
                }

                m_flags.Set(1);
                m_flashTimer = 0.5f;
            }

            return;
        }
    }

    param.field_0x54 = 5;
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "MODEL 0:Out 1:In", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_805231C8 = {
    "POWER_DEVICE_NAKED",
    fn_800D49B8,
    fn_800D49B4,
    PowerDeviceNaked::Create,
    0,
    0,
    0x106C,
    1,
    0,
    10,
    s_paramInfo,
};
