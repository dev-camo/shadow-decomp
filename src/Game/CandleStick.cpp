/* CandleStick (class name from RTTI): a gadget of a candle stick (model file CANDLESTICK.BIN, with
 * the breaking model PR0300CANDLESTICK_BRK.DFF) that is lit by an attack and breaks when its
 * health is used up (behaviour inferred). Candle sticks that share a com id are counted by a
 * switch counter, which reports the id to the manager of UnkManager_800CB69C when the number given
 * by the parameter SAME_COMID_SWITCH_NUM has been lit. CandleStickSetState (class name from RTTI,
 * derived from SetAdapterState) is the state that the owner remembers between creations. File name
 * inferred from the class. Method names marked "(name inferred)" are not original. Definitions
 * are in reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SetAdapterState.h"
#include "Game/UnkAttack_8005A840.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/EffectElement.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkSwitchCounter_801660A0.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class CandleStick;

/* The two startup vectors: the offset of the effect above the candle stick, and the second
 * argument of fn_80168C48 (names inferred from their uses). The original's startup function builds
 * each one in a temporary and copies it with the out-of-line copy at 0x800091EC (25 instead of 12
 * instructions); here the constructor stores the components in place. */
static Fn8040Vector3 s_effectOffset(0.0f, 15.0f, 0.0f);
static Fn8040Vector3 s_effectArgument(0.0f, 1.0f, 0.0f);

/* The model file that the broken candle stick shows its break effect from. */
static const char *s_breakFile = "PR0300CANDLESTICK_BRK.DFF";

/* The state that the owner remembers between the creations of a candle stick (class name from
 * RTTI, derived from SetAdapterState): the health left, bit 0 of the flags is set once it has
 * broken and bit 1 once it was lit (behaviour inferred). */
class CandleStickSetState : public SetAdapterState {
  public:
    CandleStickSetState() {
        Reset();
    }

    virtual ~CandleStickSetState() {}

    /* Restores the full health and clears the flags (name inferred). */
    void Reset() {
        m_health = 2.0f;
        m_flags.Clear();
    }

    /* 0x04 */ float m_health;
    /* 0x08 */ UnkBitFlags_80014E04 m_flags;
};
CHECK_SIZEOF(CandleStickSetState, 0xC);

/* The gadget parameters in the order of the descriptor's table below. */
struct CandleStickParams {
    /* 0x0 */ s32 switchNum; /* SAME_COMID_SWITCH_NUM */
};
CHECK_SIZEOF(CandleStickParams, 0x4);

class CandleStick : public Gadget {
  public:
    CandleStick(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~CandleStick();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the parameter, the com id, the position and the rotation from the owner (name
     * inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Puts the model and the effect where the character info is (name inferred). */
    void UpdateModelMatrix();
    /* Shows the model and the effect that the remembered state calls for, hooks the character
     * info and the attack callback to the model and registers the com id with the switch counter
     * (name inferred). */
    void ApplyState();
    /* The attack object's callback (behaviour inferred). A hit removes the attack's strength from
     * the health; when it is used up the candle stick breaks. An attack whose flag word has bit
     * 0x400 lights it. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::weak_ptr<Effect::System::EffectElement> m_effect;
    /* 0x48 */ u8 m_comId; /* copied from the owner (name inferred) */
    /* Set by OnAttack and consumed by UpdateBody: bit 0 the candle stick has broken, bit 1 it was
     * lit (behaviour inferred). */
    /* 0x4C */ UnkBitFlags_80014E04 m_flags;
    /* 0x50 */ CandleStickSetState *m_data;
    /* 0x54 */ CandleStickParams m_params;
    /* 0x58 */ UnkSoundHandle_801D3DC8 m_sound;
    /* 0x74 */ u32 field_0x74; /* not accessed here; the object is 0x78 bytes */
};
CHECK_SIZEOF(CandleStick, 0x78);

const char *CandleStick::s_name = "CandleStick";

CandleStick::CandleStick(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_flags.Clear();

    if (owner->m_adapterState == 0) {
        fn_800C9930(owner, new CandleStickSetState);
    }

    m_data = dynamic_cast<CandleStickSetState *>(owner->m_adapterState);
    m_charInfo = fn_8015CD90(0x4C, 4);
    ReadOwner(owner);
    m_model = fn_8015F800(1, "CANDLESTICK.BIN");
    ApplyState();
    UpdateModelMatrix();
}

/* Matching workaround: the block that stops the effect is written out both in this destructor and
 * in ResetFromOwner. As an inline member the compiler leaves it a call, and the original has no
 * such function. Revisit if the original's shared inline form is found. */
CandleStick::~CandleStick() {
    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

        if (effect.get() != 0) {
            effect->ClearFlags(1);
        }

        m_effect.reset();
    }

    m_sound.fn_801D455C();
}

extern "C" void fn_8012D678(void) {
    fn_8006B474("CANDLESTICK.BIN", 1);
    UnkSwitchCounter_801660A0::GetInstance()->fn_80165F3C();
}

extern "C" void fn_8012D654(void) {
    UnkSwitchCounter_801660A0::GetInstance()->fn_80165EFC();
}

void CandleStick::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new CandleStick(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void CandleStick::UpdateBody(float dt) {
    if (m_flags.Test(0)) {
        m_model->fn_80162804();
        m_model->fn_80162A68(1, 0);
        UpdateModelMatrix();

        {
            UnkEffectParams_8015F0B8 breakEffect(m_charInfo, 0);

            breakEffect.field_0x44 =
                static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, s_breakFile));
            breakEffect.field_0x40 = m_charInfo->m_position.y;
            breakEffect.field_0x34 = 1.0f;
            breakEffect.field_0x4A = 0;
            breakEffect.field_0x54 = 1;
            fn_8015D93C(&breakEffect);
        }

        m_data->m_flags.Set(0);
        m_flags.Reset(0);
    }

    if (m_flags.Test(1)) {
        m_model->fn_80161FC8(1, m_charInfo);

        Fn8040Vector3 effectPosition;

        fn_8040CDD0(&effectPosition, &m_charInfo->m_position, &s_effectOffset);
        m_effect = fn_80168C48(&effectPosition, &s_effectArgument, 0x19);
        fn_801D54F4(fn_80053164(), 0x7431, &m_charInfo->m_position, 0, 1);
        m_sound = fn_801D3E58(0x7430, &m_charInfo->m_position, this);
        UnkSwitchCounter_801660A0::GetInstance()->fn_801658AC(m_comId, 1);
        m_data->m_flags.Set(1);
        m_flags.Reset(1);
    }

    m_model->fn_80162FB0(dt);
}

void CandleStick::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CandleStickParams previous = m_params;
    u8 previousComId = m_comId;

    ReadOwner(owner);

    if (m_params.switchNum != previous.switchNum || m_comId != previousComId) {
        m_data->Reset();
        m_model->fn_80162804();
        m_model->fn_80161880();
        m_sound.fn_801D455C();

        if (!m_effect.expired()) {
            boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

            if (effect.get() != 0) {
                effect->ClearFlags(1);
            }

            m_effect.reset();
        }

        ApplyState();
    }

    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();
}

void CandleStick::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E60(owner, &m_comId);
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
}

/* Emitted after the template instances that follow ApplyState; the original emits it directly
 * after ApplyState. */
UnkSwitchCounter_801660A0 *UnkSwitchCounter_801660A0::GetInstance() {
    if (s_instance == 0) {
        static UnkSwitchCounter_801660A0 counter;

        s_instance = &counter;
    }

    return s_instance;
}

void CandleStick::ApplyState() {
    if (m_data->m_flags.Test(0)) {
        m_model->fn_80162A68(1, 0);
    } else {
        m_model->fn_80162A68(0, 0);
    }

    if (m_data->m_flags.Test(1)) {
        m_model->fn_80161FC8(1, m_charInfo);

        Fn8040Vector3 effectPosition;

        fn_8040CDD0(&effectPosition, &m_charInfo->m_position, &s_effectOffset);
        m_effect = fn_80168C48(&effectPosition, &s_effectArgument, 0x19);
        m_sound = fn_801D3DC8(0x7430, &m_charInfo->m_position, this);
    }

    m_model->fn_80161FC8(0, m_charInfo);

    {
        boost::shared_ptr<UnkAttack_8005A840> attack(m_model->fn_80161428(0).lock());

        if (attack.get() != 0) {
            attack->fn_80096B64(1, AttackCallback(boost::bind(&CandleStick::OnAttack, this, _1)));
        }
    }

    if (!UnkSwitchCounter_801660A0::GetInstance()->fn_80165944(m_comId)) {
        UnkSwitchCounter_801660A0::GetInstance()->fn_80165A08(m_comId, m_params.switchNum, 0);
    }
}

void CandleStick::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    m_model->fn_801633DC(&matrix);

    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

        if (effect.get() != 0) {
            Fn8040Vector3 effectPosition;

            fn_8040CDD0(&effectPosition, &m_charInfo->m_position, &s_effectOffset);
            fn_80425E4C(effect.get(), &effectPosition);
        }
    }
}

void CandleStick::OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param) {
    param.field_0x54 = 1;

    if (!m_data->m_flags.Test(1) && fn_8015D4E4(&param)) {
        boost::shared_ptr<UnkAttack_8005A840> attack(param.field_0x0.lock());

        if (attack.get() != 0 && (attack->field_0x40 & 0x400) != 0) {
            m_flags.Set(1);
        }
    }

    if (!m_data->m_flags.Test(0)) {
        m_data->m_health -= param.field_0x50;

        if (fn_8000B044(m_data->m_health) || m_data->m_health < 0.0f) {
            param.field_0x54 = 3;

            if (!m_flags.Test(0)) {
                m_flags.Set(0);

                boost::shared_ptr<CharInfo> attacker(fn_8015D570(&param));

                if (attacker.get() != 0) {
                    fn_8020642C(attacker, 0x23);
                }
            }
        }
    }
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "SAME_COMID_SWITCH_NUM", 1, 1, 0x100, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_805284B0 = {
    "CandleStick", fn_8012D678, fn_8012D654, CandleStick::Create, 0, 0, 0xBBB, 1, 0, 10,
    s_paramInfo,
};

/* Defined last: this keeps it after the unit's other .sbss objects, as in the original. */
UnkSwitchCounter_801660A0 *UnkSwitchCounter_801660A0::s_instance;
