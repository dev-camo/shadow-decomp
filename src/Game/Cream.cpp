/* Cream (class name from RTTI): the gadget of a Cream (model file CREAM.BIN) that stands at a point
 * and reacts to being touched and to attacks (behaviour inferred). Like the Chao gadget it shows a
 * model, owns a probe and an attack object with a bound callback, and keeps its state in a
 * CreamSetState (class name from RTTI, derived from SetAdapterState) that the owner remembers.
 * File name inferred from the class. Method names marked "(name inferred)" are not original.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SetAdapterState.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkAttack_8005A840.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkObject_801E72DC.h"
#include "Game/UnkObject_802B4254.h"
#include "Game/UnkProbeTarget_800587A0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class Cream;

/* Four floats that the original builds at startup (names inferred from their uses): the height of
 * the character info (probe centre) above the Cream's position, the radius of the probe around the
 * character info, the height of the second object (m_object2) above the character info and the
 * value passed to that object's fn_802B4498. */
static UnkStartupFloat_800E2570 s_charInfoHeight(4.0f);
static UnkStartupFloat_800E2570 s_probeRadius(4.0f);
static UnkStartupFloat_800E2570 s_object2Height(8.0f);
static UnkStartupFloat_800E2570 s_object2Value(80.0f);

/* The state that the owner remembers between the creations of a Cream (class name from RTTI,
 * derived from SetAdapterState): bit 0 of the flags is set once the Cream has been touched, bit 1
 * once it has been attacked (behaviour inferred). */
class CreamSetState : public SetAdapterState {
  public:
    CreamSetState() {
        m_flags.Clear();
    }

    virtual ~CreamSetState() {}

    /* 0x04 */ UnkBitFlags_80014E04 m_flags;
};
CHECK_SIZEOF(CreamSetState, 0x8);

class Cream : public Gadget {
  public:
    Cream(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Cream();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, the rotation and the tag from the owner and places the character info
     * the height s_charInfoHeight above the position (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the Cream's position, the probe and the object at 0x44 at the character
     * info, and the second object s_object2Height above the character info (name inferred). */
    void UpdateModelMatrix();
    /* Blends the model's motion control to the motion with the given index (name inferred). */
    void SetMotion(int motion);
    /* The attack object's callback (behaviour inferred): an accepted attack plays the attack
     * motion and is reported once. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x30 */ Fn8040Vector3 m_position; /* the owner's position */
    /* 0x3C */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x44 */ boost::shared_ptr<UnkObject_801E72DC> m_object;
    /* 0x4C */ boost::shared_ptr<UnkObject_802B4254> m_object2;
    /* 0x54 */ boost::shared_ptr<UnkProbeTarget_800587A0> m_probe;
    /* 0x5C */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x64 */ CreamSetState *m_data;
    /* The state (names inferred from the code that sets them): 0 waiting, 1 touched for the first
     * time, 2 touched before, 3 attacked, 4 after the attack motion. */
    /* 0x68 */ s32 m_state;
    /* 0x6C */ bool m_touched; /* the probe reported a hit in the last update */
    /* 0x6D */ u8 m_tag;       /* copied from the owner; passed to fn_8016907C (name inferred) */
};
CHECK_SIZEOF(Cream, 0x70);

const char *Cream::s_name = "Cream";

Cream::Cream(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_touched(false) {
    m_name = s_name;

    if (owner->m_adapterState == 0) {
        fn_800C9930(owner, new CreamSetState);
    }

    m_data = dynamic_cast<CreamSetState *>(owner->m_adapterState);
    m_charInfo = fn_8015CD90(0x3B, 4);
    m_model = fn_8015F800(1, "CREAM.BIN");
    m_model->fn_80162A68(0, 0);

    m_probe = fn_800C7DEC(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 2, 7, 0,
                          &lbl_805E07A0, s_probeRadius.value);

    boost::weak_ptr<UnkAttack_8005A840> attackWeak = fn_800C4374(fn_80021B08(), m_probe);
    boost::shared_ptr<UnkAttack_8005A840> attack = attackWeak.lock();

    attack->fn_80096B64(1, AttackCallback(boost::bind(&Cream::OnAttack, this, _1)));

    m_object = fn_801E72DC(0x2000);
    m_model->fn_80160C28(0);
    m_model->fn_8015FF60();

    if (!m_data->m_flags.Test(0)) {
        m_state = 0;
        m_model->fn_80160A84(0, 2);
        m_object2 = fn_802B4254(2, 0);
        fn_802B4498(m_object2.get(), s_object2Value.value);
        fn_802B4510(m_object2.get(), 1);
    } else {
        m_state = 2;
        m_model->fn_80160A84(0, 1);
    }

    ReadOwner(owner);
    UpdateModelMatrix();
}

Cream::~Cream() {}

extern "C" void fn_80134840(void) {
    fn_8006B474("CREAM.BIN", 1);
}

extern "C" void fn_8013483C(void) {}

void Cream::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new Cream(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void Cream::UpdateBody(float dt) {
    if (fn_8015D2EC(m_probe->GetHits())) {
        if (!m_touched) {
            if (m_state == 0 || m_state == 4) {
                if (m_data->m_flags.Test(0)) {
                    SetMotion(1);
                    m_state = 2;
                } else {
                    SetMotion(5);
                    m_state = 1;
                    m_data->m_flags.Set(0);

                    {
                        boost::shared_ptr<UnkActor_800740E4> actor =
                            fn_800740E4(fn_8006D6A8(), 2).lock();

                        if (actor.get() != 0) {
                            actor->UnkSlot7(1, 0);

                            if (actor->Field0xCEquals(1)) {
                                fn_801FAF00(fn_80109D64(), 0x3D1, 0, 0, 0);
                            } else {
                                fn_801FAF00(fn_80109D64(), 0x3D0, 0, 0, 0);
                            }
                        }
                    }

                    m_object2.reset();
                    fn_800C985C(m_owner);
                    fn_8016907C(fn_800CB69C(), m_tag, 1);
                }
            }

            m_touched = true;
        }
    } else {
        m_touched = false;
    }

    switch (m_state) {
    case 3:
        if (stMtnControlWorkFlag(m_model->fn_801602B4(0).lock().get(), 0, 3, 1)) {
            SetMotion(3);
            m_state = 4;
        }
        break;

    case 1:
        if (stMtnControlWorkFlag(m_model->fn_801602B4(0).lock().get(), 0, 3, 1)) {
            SetMotion(1);
            m_state = 2;
        }
        break;
    }

    UpdateModelMatrix();
    m_model->fn_80162FB0(dt);
}

void Cream::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();
    m_model->fn_80162EF4();
}

void Cream::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E60(owner, &m_tag);
    fn_800091D0(&m_charInfo->m_position, &m_position);
    m_charInfo->m_position.y += s_charInfoHeight.value;
}

void Cream::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    m_model->fn_801633DC(&matrix);
    m_probe->SetPosition(&m_charInfo->m_position);
    fn_800CCE38(m_object.get(), &m_charInfo->m_position);

    if (m_object2.get() != 0) {
        Fn8040Vector3 above(0.0f, s_object2Height.value, 0.0f);
        Fn8040Vector3 object2Position;

        fn_8040CDD0(&object2Position, &above, &m_charInfo->m_position);
        fn_802B44C8(m_object2.get(), &object2Position);
    }
}

void Cream::SetMotion(int motion) {
    boost::shared_ptr<UnkMotionControl_801602B4> control = m_model->fn_801602B4(0).lock();

    if (control.get() != 0) {
        stMtnPrepareBlendWithTarget(control.get(), 0, -1, -1);
        stMtnRequestBlendFrame(control.get(), 0, 15.0f);
    }

    m_model->fn_80160A84(0, motion);
}

void Cream::OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param) {
    if (fn_8015D4E4(&param) && m_state != 4 && m_state != 3) {
        param.field_0x54 = 1;
        m_state = 3;
        m_model->fn_80160A84(0, 4);
        fn_801D54F4(fn_80053164(), 0x7446, &m_charInfo->m_position, 0, 1);

        if (!m_data->m_flags.Test(1)) {
            {
                boost::shared_ptr<CharInfo> attacker = fn_8015D570(&param);

                if (attacker.get() != 0) {
                    fn_8020642C(attacker, 0x2E);
                }
            }

            m_data->m_flags.Set(1);
        }
    } else {
        param.field_0x54 = 8;
    }
}

GadgetTypeInfo lbl_80528AB0 = {
    "Cream", fn_80134840, fn_8013483C, Cream::Create, 0, 0, 0xBBD, 1, 0, 10, 0,
};
