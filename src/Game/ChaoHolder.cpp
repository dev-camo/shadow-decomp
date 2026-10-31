/* ChaoHolder (class name from RTTI): the gadget of a Chao or Cheese (model files CHAO.BIN and
 * CHEEZ.BIN, selected by the parameter "0:CHAO 1:CHEEZ") that moves around a point (behaviour
 * inferred). It holds a Chao object (named after the boost::checked_deleter<Chao> RTTI string) that
 * shows the model, owns a probe and an attack object with a bound callback, and keeps its state in
 * a ChaoData that the owner remembers. File name inferred from the class. Method names marked
 * "(name inferred)" are not original. Definitions are in reverse address order (deferred inlining
 * emits functions last-defined first).
 *
 * NonMatching: checked_delete<Chao> is a local symbol where the original's is weak, and the data
 * has records that the original unit does not have (listed in the note in configure.py). */
#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SetAdapterState.h"
#include "Game/SharedConstants.h"
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
#include "Game/fn_8040_random.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class Chao;
class ChaoHolder;

/* Five floats that the original builds at startup (names inferred from their uses): the height
 * of the character info (probe centre) above the Chao's position, the radius of the probe around
 * the character info, the height of the second object (m_object2) above the character info, the
 * value passed to that object's fn_802B4498, and the movement speed that corresponds to the
 * walking motion's normal frame speed. */
static UnkStartupFloat_800E2570 s_charInfoHeight(2.0f);
static UnkStartupFloat_800E2570 s_probeRadius(2.0f);
static UnkStartupFloat_800E2570 s_object2Height(8.0f);
static UnkStartupFloat_800E2570 s_object2Value(80.0f);
static UnkStartupFloat_800E2570 s_animationSpeedBase(7.0f);

/* The gadget parameters in the order of the descriptor's table below. */
struct ChaoParams {
    /* 0x0 */ u32 type;         /* 0 CHAO or 1 CHEEZ */
    /* 0x4 */ float moveRadius; /* MOVE_RADIUS (m); the code scales it by 10 */
    /* 0x8 */ float moveSpeed;  /* MOVE_SPPED (m/sec); the code scales it by 10 */
};
CHECK_SIZEOF(ChaoParams, 0xC);

/* A motion of the model: both words are passed to fn_80160A84, the first one also to the model's
 * other motion functions (names inferred). */
struct ChaoMotion {
    /* 0x0 */ s32 id;
    /* 0x4 */ s32 arg;
};
CHECK_SIZEOF(ChaoMotion, 0x8);

/* A model file and its four motions, indexed by the Chao's state (name inferred). */
struct ChaoModelEntry {
    /* 0x00 */ const char *file;
    /* 0x04 */ ChaoMotion motions[4];
};
CHECK_SIZEOF(ChaoModelEntry, 0x24);

static const ChaoModelEntry s_modelEntries[2] = {
    {"CHAO.BIN", {{0, 2}, {0, 1}, {1, 2}, {1, 1}}},
    {"CHEEZ.BIN", {{0, 2}, {0, 1}, {1, 2}, {1, 1}}},
};

/* The state that the owner remembers between the creations of a Chao (class name from RTTI,
 * derived from SetAdapterState): bit 0 of the flags is set when it has been touched, bit 1 when it
 * has been attacked (behaviour inferred), and the position is where the Chao stands. */
class ChaoData : public SetAdapterState {
  public:
    ChaoData() : m_flags(0) {}
    virtual ~ChaoData() {}

    /* 0x04 */ UnkBitFlags_80014E04 m_flags;
    /* 0x08 */ Fn8040Vector3 m_position;
};
CHECK_SIZEOF(ChaoData, 0x14);

class Chao {
  public:
    Chao(ChaoHolder *holder, UnkGadgetOwner_800C9CD0 *owner);
    ~Chao();

    /* Advances the Chao by dt (name inferred). */
    void Update(float dt);
    /* Moves the Chao around the point and turns it towards the character info that fn_800A8B74
     * returns for lbl_805EE588 (name inferred). */
    void UpdateMovement(float dt);
    /* Puts the Chao on the circle of the holder's radius around the point (name inferred). */
    void ResetPosition();
    /* Re-reads the owner's position, rotation and byte and places the model (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the Chao's position, the character info s_charInfoHeight above it, the
     * probe and the object at 0x2C at the character info, and the second object s_object2Height
     * above the character info (name inferred). */
    void UpdateModelMatrix();
    /* Shows a motion of the model, blending from the one shown before when blend is set (name
     * inferred). */
    void SetMotion(const ChaoMotion *motion, bool blend);
    /* The attack object's callback (behaviour inferred). When fn_8015D4E4 accepts the attacker and
     * the Chao is not in state 2 or 3, it stores 1 in the parameter's word at 0x54 and switches to
     * state 2 and its motion; the first such attack also reports the attacker to fn_8020642C and
     * plays a sound. Otherwise it stores 8 in that word. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    /* 0x00 */ ChaoHolder *m_holder;
    /* 0x04 */ const ChaoModelEntry *m_entry;
    /* 0x08 */ ChaoData *m_data;
    /* 0x0C */ Fn8040Vector3 m_center;   /* the owner's position */
    /* 0x18 */ Fn8040Vector3 m_rotation; /* the owner's rotation */
    /* 0x24 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x2C */ boost::shared_ptr<UnkObject_801E72DC> m_object;
    /* 0x34 */ boost::shared_ptr<UnkObject_802B4254> m_object2;
    /* 0x3C */ boost::shared_ptr<UnkProbeTarget_800587A0> m_probe;
    /* 0x44 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* The state (names inferred from the code that sets them): 0 walking around, 1 touched, 2
     * attacked, 3 after the attack motion. */
    /* 0x4C */ s32 m_state;
    /* 0x50 */ float m_timer;  /* seconds until the next idle sound while walking */
    /* 0x54 */ bool m_touched; /* the probe reported a hit in the last update */
    /* 0x55 */ u8 m_tag;       /* copied from the owner; passed to fn_8016907C (name inferred) */
    /* 0x58 */ const ChaoMotion *m_motion; /* the motion being shown */
};
CHECK_SIZEOF(Chao, 0x5C);

class ChaoHolder : public Gadget {
  public:
    ChaoHolder(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ChaoHolder();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 and its Chao (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the parameters from the owner (name inferred). */
    void ReadParams(UnkGadgetOwner_800C9CD0 *owner);
    /* Replaces the Chao with a new one (name inferred). */
    void CreateChao(UnkGadgetOwner_800C9CD0 *owner);
    /* The parameters; the Chao calls it out of line (name inferred). */
    const ChaoParams *GetParams();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<Chao> m_chao;
    /* 0x38 */ ChaoParams m_params;
    /* 0x44 */ u32 field_0x44; /* not accessed here; the object is 0x48 bytes */
};
CHECK_SIZEOF(ChaoHolder, 0x48);

/* The two sounds that a walking Chao plays at random intervals (ids as passed to
 * fn_801D54F4). */
static const u16 s_idleSounds[2] = {0x7447, 0x745D};

Chao::Chao(ChaoHolder *holder, UnkGadgetOwner_800C9CD0 *owner)
    : m_holder(holder), m_entry(&s_modelEntries[m_holder->m_params.type]),
      m_rotation(0.0f, 0.0f, 0.0f), m_touched(false), m_motion(0) {
    m_charInfo = fn_8015CD90(0x3C, 4);
    fn_800C9F14(owner, &m_center);
    fn_800C9ED4(owner, &m_rotation);
    fn_800C9E60(owner, &m_tag);

    if (owner->m_adapterState == 0) {
        m_data = new ChaoData;
        m_data->m_flags.Clear();
        ResetPosition();
        fn_800C9930(owner, m_data);
    } else {
        m_data = dynamic_cast<ChaoData *>(owner->m_adapterState);
    }

    if (m_holder->m_params.type == 1) {
        if (m_data->m_flags.Test(0)) {
            m_state = 1;
        } else {
            if (m_data->m_flags.Test(1)) {
                m_state = 3;
            } else {
                m_state = 0;
            }

            m_object2 = fn_802B4254(2, 0);
            fn_802B4498(m_object2.get(), s_object2Value.value);
            fn_802B4510(m_object2.get(), 1);
        }
    } else {
        if (m_data->m_flags.Test(1)) {
            m_state = 3;
        } else if (m_data->m_flags.Test(0)) {
            m_state = 1;
        } else {
            m_state = 0;
        }
    }

    m_model = fn_8015F800(1, m_entry->file);
    SetMotion(&m_entry->motions[m_state], false);

    m_probe = fn_800C7DEC(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 2, 7, 0,
                          &lbl_805E07A0, s_probeRadius.value);

    boost::weak_ptr<UnkAttack_8005A840> attackWeak = fn_800C4374(fn_80021B08(), m_probe);
    boost::shared_ptr<UnkAttack_8005A840> attack = attackWeak.lock();

    attack->fn_80096B64(1, AttackCallback(boost::bind(&Chao::OnAttack, this, _1)));

    m_object = fn_801E72DC(0x2000);
    UpdateModelMatrix();
    m_timer = fn_8040EC80(0.5f, 6.0f);
}

/* Matching workaround: auto_inline around this destructor. The original keeps checked_delete<T>
 * out of line and emits this destructor at its definition; with the stock GC/2.7 compiler and
 * -inline noauto that needs auto_inline here. A tooling constraint, not a claim about the original
 * source; revisit if another compiler build reproduces this without the pragma, or if an original
 * checked_delete keeps out of line a destructor whose body calls only non-inline functions (the
 * pragma would inline that destructor into it). A destructor found inlined into checked_delete in
 * the original would instead mean that the original used auto_inline here. */
#pragma auto_inline on
Chao::~Chao() {}
#pragma auto_inline reset

void Chao::Update(float dt) {
    if (fn_8015D2EC(m_probe->GetHits())) {
        if (!m_touched) {
            if (m_holder->m_params.type == 1) {
                if (m_state != 1 && m_state != 2) {
                    SetMotion(&m_entry->motions[1], false);
                    m_state = 1;
                    fn_801D54F4(fn_80053164(), 0x745E, &m_charInfo->m_position, 0, 1);

                    if (!m_data->m_flags.Test(0)) {
                        m_data->m_flags.Set(0);

                        {
                            boost::shared_ptr<UnkActor_800740E4> actor =
                                fn_800740E4(fn_8006D6A8(), 2).lock();

                            if (actor.get() != 0) {
                                actor->UnkSlot7(1, 0);

                                if (!actor->Field0xCEquals(1)) {
                                    fn_801FAF00(fn_80109D64(), 0x3D2, fn_801FAAC4(2), 0, 0);
                                }
                            }
                        }

                        m_object2.reset();
                        fn_8016907C(fn_800CB69C(), m_tag, 1);

                        if (m_holder->m_owner != 0) {
                            fn_800C985C(m_holder->m_owner);
                        }
                    }
                }
            } else if (m_state == 0) {
                SetMotion(&m_entry->motions[1], true);
                m_state = 1;
                fn_801D54F4(fn_80053164(), 0x745E, &m_charInfo->m_position, 0, 1);

                if (!m_data->m_flags.Test(0)) {
                    m_data->m_flags.Set(0);
                }
            }

            m_touched = true;
        }
    } else {
        m_touched = false;
    }

    switch (m_state) {
    case 0:
        m_timer -= dt;

        if (m_timer < 0.0f) {
            u16 sound = s_idleSounds[fn_8040EA90(2)];

            fn_801D54F4(fn_80053164(), sound, &m_charInfo->m_position, 0, 1);
            m_timer = fn_8040EC80(6.0f, 12.0f);
        }
        break;

    case 2:
        if (stMtnControlWorkFlag(m_model->fn_801602B4(m_entry->motions[2].id).lock().get(), 0, 3,
                                 1)) {
            m_state = 3;
            SetMotion(&m_entry->motions[3], true);
        }
        break;
    }

    UpdateMovement(dt);
    UpdateModelMatrix();
    m_model->fn_80162FB0(dt);
}

void Chao::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_center);
    fn_800C9ED4(owner, &m_rotation);
    fn_800C9E60(owner, &m_tag);
    ResetPosition();
    UpdateModelMatrix();
    m_model->fn_80162EF4();
}

const ChaoParams *ChaoHolder::GetParams() {
    return &m_params;
}

void Chao::ResetPosition() {
    Fn8040Vector3 offset(0.0f, 0.0f, -m_holder->GetParams()->moveRadius);
    Fn8040Vector3 rotated;
    Fn8040Vector3 position;

    fn_8040D0AC(&rotated, &offset, m_rotation.y);
    fn_800091D0(&offset, &rotated);
    fn_8040CDD0(&position, &m_center, &offset);
    fn_800091D0(&m_data->m_position, &position);
    m_charInfo->m_rotation.y = m_rotation.y;
}

void Chao::UpdateMovement(float dt) {
    Fn8040Vector3 previous;
    Fn8040Vector3 direction;
    Fn8040Vector3 heading;
    Fn8040Vector3 chaoPosition;
    Fn8040Vector3 characterPosition;
    Fn8040Vector3 facing;
    Fn8040Vector3 forward;
    Fn8040Vector3 axis;
    Fn8040Vector3 directionTemporary;
    Fn8040Vector3 rotatedTemporary;
    Fn8040Vector3 headingTemporary;
    Fn8040Vector3 moved;
    Fn8040Vector3 facingTemporary;
    Fn8040Vector3 turnedTemporary;
    Fn8040Vector3 axisTemporary;

    switch (m_state) {
    case 0:
        if (!fn_8000B044(m_holder->m_params.moveRadius) &&
            !fn_8000B044(m_holder->m_params.moveSpeed)) {
            float angleStep = dt * m_holder->m_params.moveSpeed / m_holder->m_params.moveRadius;

            fn_800091EC(&previous, &m_data->m_position);
            fn_8040CD74(&directionTemporary, &m_data->m_position, &m_center);
            fn_800091EC(&direction, &directionTemporary);
            fn_8040CE74(&direction);
            fn_8040D0AC(&rotatedTemporary, &direction, angleStep);
            fn_800091D0(&direction, &rotatedTemporary);
            fn_8040CE38(&direction, m_holder->m_params.moveRadius);
            fn_8040CFF8(&direction, &m_center);
            fn_8040CD74(&headingTemporary, &direction, &m_data->m_position);
            fn_800091EC(&heading, &headingTemporary);
            heading.y = 0.0f;
            fn_8040CE74(&heading);
            fn_8003B9E8(&heading, &m_charInfo->m_rotation.x, &m_charInfo->m_rotation.y);
            fn_800091D0(&m_data->m_position, &direction);

            boost::shared_ptr<UnkMotionControl_801602B4> control =
                m_model->fn_801602B4(m_entry->motions[m_state].id).lock();

            if (control.get() != 0) {
                float speed;

                if (fn_8000A950(dt)) {
                    speed = 0.0f;
                } else {
                    fn_8040CD74(&moved, &previous, &m_data->m_position);
                    speed = fn_8040CF50(&moved) / dt;
                }

                stMtnRequestFrameSpeed(control.get(), 0,
                                       speed / (60.0f * (s_animationSpeedBase.value / 60.0f)));
            }
        }
        break;

    case 1: {
        boost::shared_ptr<CharInfo> character = fn_800A8B74(fn_80074194(), lbl_805EE588).lock();

        if (character.get() != 0) {
            fn_800091EC(&chaoPosition, &m_data->m_position);
            fn_800091EC(&characterPosition, &character->m_position);
            chaoPosition.y = 0.0f;
            characterPosition.y = 0.0f;
            fn_8040CD74(&facingTemporary, &characterPosition, &chaoPosition);
            fn_800091EC(&facing, &facingTemporary);
            forward.Set(0.0f, 0.0f, 1.0f);
            fn_8040D0AC(&turnedTemporary, &forward, m_charInfo->m_rotation.y);
            fn_800091D0(&forward, &turnedTemporary);
            fn_8040CE74(&facing);
            fn_8040CFB8(&axisTemporary, &facing, &forward);
            fn_800091EC(&axis, &axisTemporary);

            float angle = fn_8040F8FC(&facing, &forward);
            float maxTurn = dt * (270.0f * (lbl_805FAD38 / 180.0f));

            if (fn_8000A980(angle) > maxTurn) {
                if (axis.y > 0.0f) {
                    m_charInfo->m_rotation.y -= maxTurn;
                } else {
                    m_charInfo->m_rotation.y += maxTurn;
                }
            }
        }
        break;
    }
    }
}

void Chao::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_800091D0(&m_charInfo->m_position, &m_data->m_position);
    m_charInfo->m_position.y += s_charInfoHeight.value;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_data->m_position);
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

void Chao::SetMotion(const ChaoMotion *motion, bool blend) {
    if (m_motion == 0) {
        m_model->fn_80162A68(motion->id, 0);
        m_model->fn_80160C28(motion->id);
        m_model->fn_8015FF60();
    } else if (m_motion->id != motion->id) {
        m_model->fn_80160670(m_motion->id);
        m_model->fn_80162804();
        m_model->fn_80162A68(motion->id, 0);
        m_model->fn_80160C28(motion->id);
        m_model->fn_8015FF60();
    }

    boost::shared_ptr<UnkMotionControl_801602B4> control = m_model->fn_801602B4(motion->id).lock();

    if (blend && control.get() != 0) {
        stMtnPrepareBlendWithTarget(control.get(), 0, -1, -1);
        stMtnRequestBlendFrame(control.get(), 0, 15.0f);
    }

    m_model->fn_80160A84(motion->id, motion->arg);

    if (control.get() != 0) {
        stMtnRequestFrameSpeed(control.get(), 0, 1.0f);
    }

    m_motion = motion;
}

void Chao::OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param) {
    if (fn_8015D4E4(&param) && m_state != 3 && m_state != 2) {
        param.field_0x54 = 1;
        m_state = 2;
        SetMotion(&m_entry->motions[2], false);

        if (!m_data->m_flags.Test(1)) {
            {
                boost::shared_ptr<CharInfo> attacker = fn_8015D570(&param);

                if (attacker.get() != 0) {
                    fn_8020642C(attacker, 0x2F);
                }
            }

            fn_801D54F4(fn_80053164(), 0x7448, &m_charInfo->m_position, 0, 1);
            m_data->m_flags.Set(1);
        }
    } else {
        param.field_0x54 = 8;
    }
}

ChaoHolder::ChaoHolder(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    ReadParams(owner);
}

ChaoHolder::~ChaoHolder() {}

extern "C" void fn_8012DE94(void) {
    fn_8006B474("CHAO.BIN", 1);
    fn_8006B474("CHEEZ.BIN", 1);
}

extern "C" void fn_8012DE90(void) {}

void ChaoHolder::Create(UnkGadgetOwner_800C9CD0 *owner) {
    ChaoHolder *holder = new ChaoHolder(fn_801EB304(fn_8001C4F8(), 11), owner);

    holder->CreateChao(owner);
}

void ChaoHolder::UpdateBody(float dt) {
    if (m_chao.get() != 0) {
        m_chao->Update(dt);
    }
}

void ChaoHolder::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ChaoParams previous = m_params;

    ReadParams(owner);

    if (previous.type != m_params.type) {
        CreateChao(owner);
    }

    m_chao->ReadOwner(owner);
}

void ChaoHolder::ReadParams(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_params.moveRadius = 10.0f * m_params.moveRadius;
    m_params.moveSpeed = 10.0f * m_params.moveSpeed;
}

void ChaoHolder::CreateChao(UnkGadgetOwner_800C9CD0 *owner) {
    m_chao = boost::shared_ptr<Chao>(new Chao(this, owner));
}

const char *ChaoHolder::s_name = "ChaoHolder";

static GadgetParamInfo s_paramInfo[] = {
    {2, "0:CHAO 1:CHEEZ", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "MOVE_RADIUS m", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "MOVE_SPPED m/sec", 0, 0, 0, 0.5f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_805285D8 = {
    "Chao", fn_8012DE94, fn_8012DE90, ChaoHolder::Create, 0, 0, 0xBBE, 1, 0, 10, s_paramInfo,
};
