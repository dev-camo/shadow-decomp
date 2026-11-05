/* The FIRE_VINE gadget: Vine::Vine and the fire balls it shoots, Vine::FireBall and
 * Vine::FireBallBlast (class names from RTTI; behaviour in the class comments). File name inferred
 * from the classes. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first).
 *
 * The vine model's attack callback, the fire ball's collision callback and the blast's attack
 * callback are boost::function objects built with boost::bind(&Class::Method, this, _1); their
 * managers, invokers and copy functions come from the boost headers. */
/* The unit's .rodata starts with the type names that TNumEdit.h and CharaColliBaseCharInfo.h
 * produce, in that order (see those headers): the TNumEdit names at USA 0x804C1588 and
 * CharaColliBaseCharInfo's at 0x804C15FC.
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit those names;
 * revisit when the original inline functions or their real callers are recovered. They also emit
 * four RTTI base lists at the start of this object's .data (+0x00, +0x0C, +0x18, +0x24) that the
 * original's data for this file does not have. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/Vine.h"
#include "Game/AngleConversion.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/EffectElement.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/MotionSequenceModule.h"
#include "Game/UnkFrameSearch_80169E8C.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_8012773C.h"
#include "Game/fn_804157E8_motion_control.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "rw/rpworld.h"
#include "rw/rwcore.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;
using SonicteamUSA::System::CharaColli::CharaColliBase;

static const char *s_modelFile = "VINE.BIN";
/* The names of the two models' entries, motions and attack nodes. */
static const char *s_vine = "vine";
static const char *s_seed = "seed";
/* The event of the vine's attack motion at which it shoots a fire ball (name inferred). */
static const unsigned short s_fireBallEvent = 0x5000;

/* The motion sequence tables of the vine and the seed: one single-entry group per motion
 * (see MotionSequenceModule.h). */
static const UnkEnemyParaEntry_800C9178 s_vineEntry0[1] = {
    {5, -1, -1, 30, 1},
};
static const UnkEnemyParaEntry_800C9178 s_vineEntry1[1] = {
    {4, -1, -1, 30, 0},
};
static const UnkEnemyParaEntry_800C9178 s_vineEntry2[1] = {
    {3, -1, -1, 30, 0},
};
static const UnkEnemyParaEntry_800C9178 s_vineEntry3[1] = {
    {1, -1, -1, 30, 0},
};
static const UnkEnemyParaEntry_800C9178 s_vineEntry4[1] = {
    {2, -1, -1, 30, 0},
};
static const UnkEnemyParaGroup_800C961C s_vineGroups[5] = {
    {s_vineEntry0, 1, -1}, {s_vineEntry1, 1, -1}, {s_vineEntry2, 1, -1},
    {s_vineEntry3, 1, -1}, {s_vineEntry4, 1, -1},
};
static const UnkEnemyParaData_8017B8E4 s_vineSequences = {s_vineGroups, 5};

static const UnkEnemyParaEntry_800C9178 s_seedEntry0[1] = {
    {1, -1, -1, 30, 0},
};
static const UnkEnemyParaEntry_800C9178 s_seedEntry1[1] = {
    {2, -1, -1, 30, 0},
};
static const UnkEnemyParaGroup_800C961C s_seedGroups[2] = {
    {s_seedEntry0, 1, -1},
    {s_seedEntry1, 1, -1},
};
static const UnkEnemyParaData_8017B8E4 s_seedSequences = {s_seedGroups, 2};

namespace Vine {

const char *Vine::s_name = "Vine";

Vine::Vine(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x29, 4, "Vine");
    ReadOwner(owner);

    m_models[0] = fn_8015F800(1, s_modelFile);
    m_models[0]->fn_801629D8(s_vine, 0);
    m_models[1] = fn_8015F800(1, s_modelFile);
    m_models[1]->fn_801629D8(s_seed, 0);
    m_models[1]->fn_80162790();
    m_models[0]->fn_80160BA0(s_vine);
    m_models[0]->fn_8015FF60();
    m_models[1]->fn_80160BA0(s_seed);
    m_models[1]->fn_8015FF60();
    SetupSequences();

    m_models[0]->fn_80161AB8(s_vine, boost::weak_ptr<CharInfo>(m_charInfo), s_vine);
    {
        /* The original destroys weakAttack after the seed's two calls below and before the frame
         * search: they live in this block. */
        boost::weak_ptr<CharaColliAttack> weakAttack = m_models[0]->fn_80161334(s_vine);
        if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
            attack->field_0x54 = 0;
            attack->field_0x55 = 1;
            attack->field_0x1C.field_0x0 = AttackCallback(boost::bind(&Vine::OnAttack, this, _1));
        }
        m_models[1]->fn_80161F08(s_seed, boost::weak_ptr<CharInfo>(m_charInfo));
        m_models[1]->fn_801611A4(s_seed, -1);
    }

    UnkFrameSearch_80169E8C search;
    search.id = 1;
    m_models[0]->fn_8015FA78(fn_80169E8C, &search);
    m_launchFrame = search.frame;

    m_target = fn_801E72DC(0x800);
    UpdateModelMatrix();
}

Vine::~Vine() {
    fn_801E7108(m_target);
    for (int i = 0; i < 2; i++) {
        if (m_sequences[i] != 0) {
            delete m_sequences[i];
            m_sequences[i] = 0;
        }
    }
}

void Vine::UpdateBody(float dt) {
    for (int i = 0; i < 2; i++) {
        if (m_models[i].get() != 0) {
            m_models[i]->fn_80162FB0(dt);
        }
    }
    UpdateSequences(dt);

    switch (m_state) {
    case STATE_START:
        UpdateStart(dt);
        break;
    case STATE_WAIT:
        UpdateWait(dt);
        break;
    case STATE_ATTACK:
        UpdateAttack(dt);
        break;
    case STATE_DIE:
        UpdateDie(dt);
        break;
    case STATE_DEAD:
        UpdateDead(dt);
        break;
    case STATE_REVIVE:
        UpdateRevive(dt);
        break;
    case STATE_DAMAGE:
        UpdateDamage(dt);
        break;
    case STATE_8:
        /* Matching workaround: an empty arm for state 8, whose body is unknown (nothing in this file
         * enters state 8). The original tests the state against 8 and its jump table has an entry
         * for 8; without this arm the bound becomes 7, one word of UpdateBody differs and the table
         * loses that entry. Revisit when what state 8 does is found. */
        break;
    }

    UpdateModelMatrix();
}

void Vine::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();
    if (m_models[0].get() != 0) {
        m_models[0]->fn_80162EF4();
        m_models[0]->fn_8016108C();

        Fn8040Vector3 rotation(0.0f, 0.0f, 0.0f);

        rotation.x = DegreesToRadians(90.0f);
        fn_802E0C60(&m_charInfo->m_position, &rotation, 0, 5.0f, m_params.radius);
    }
}

void Vine::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_health = 3.0f;
    m_timer = 0.0f;
    SetState(STATE_WAIT);
}

void Vine::UpdateModelMatrix() {
    static const float s_turnStep = DegreesToRadians(2.0f);
    static const float s_halfTurn = DegreesToRadians(180.0f);

    if (m_state == STATE_WAIT || m_state == STATE_ATTACK) {
        Fn8040Matrix34 rotation;

        fn_8040C148(&rotation, &lbl_805E0720);
        fn_8040B848(&rotation, m_charInfo->m_rotation.y);
        fn_8040B884(&rotation, m_charInfo->m_rotation.x);
        fn_8040B80C(&rotation, m_charInfo->m_rotation.z);

        /* The vine's up direction, then the direction to the target. */
        Fn8040Vector3 direction(0.0f, 1.0f, 0.0f);
        Fn8040Vector3 up;

        fn_8040B67C(&up, &rotation, &direction);
        fn_800091D0(&direction, &up);
        if (direction.y < 0.0f) {
            Fn8040Vector3 offset;
            float yaw;
            float pitch;

            fn_8040CD74(&offset, &m_targetPosition, &m_charInfo->m_position);
            fn_800091D0(&direction, &offset);
            fn_8003B9E8(&direction, &pitch, &yaw);
            yaw += s_halfTurn;
            m_charInfo->m_rotation.y = fn_8012773C(yaw, m_charInfo->m_rotation.y, s_turnStep);
        } else {
            Fn8040Vector3 offset;
            float yaw;
            float pitch;

            fn_8040CD74(&offset, &m_targetPosition, &m_charInfo->m_position);
            fn_800091D0(&direction, &offset);
            fn_8003B9E8(&direction, &pitch, &yaw);
            m_charInfo->m_rotation.y = fn_8012773C(yaw, m_charInfo->m_rotation.y, s_turnStep);
        }
    }

    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
    for (int i = 0; i < 2; i++) {
        if (m_models[i].get() != 0) {
            m_models[i]->fn_801633DC(&matrix);
        }
    }

    if (m_launchFrame != 0 && m_target.get() != 0) {
        RwMatrix *ltm = RwFrameGetLTM(m_launchFrame);
        Fn8040Vector3 position(ltm->pos.x, ltm->pos.y, ltm->pos.z);

        fn_800CCE38(m_target.get(), &position);
    }
}

void Vine::OnAttack(AttackCallbackParam &param) {
    if (m_state == STATE_WAIT || m_state == STATE_ATTACK) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if ((attack->field_0x40 & 0x3D1) == 0) {
                param.field_0x54 = 5;
                return;
            }
        }

        m_health -= param.field_0x50;
        if (m_health <= 0.0f) {
            SetState(STATE_DIE);
        } else {
            SetState(STATE_DAMAGE);
        }
        param.field_0x54 = 1;
    } else {
        param.field_0x54 = 5;
    }
}

void Vine::ShootFireBall() {
    if (m_models[0].get() != 0 && m_launchFrame != 0) {
        FireBallParam_8014EDBC param;
        RwMatrix *ltm = RwFrameGetLTM(m_launchFrame);

        param.m_position.x = ltm->pos.x;
        param.m_position.y = ltm->pos.y;
        param.m_position.z = ltm->pos.z;

        Fn8040Vector3 direction;
        Fn8040Vector3 offset;

        fn_8040CD74(&offset, &m_targetPosition, &param.m_position);
        fn_8040CEE8(&direction, &offset);
        fn_800091D0(&param.m_velocity, &direction);
        fn_8040CE38(&param.m_velocity, 3.0f);
        FireBall::Create(param);
        fn_801D54F4(fn_80053164(), 0x7543, &param.m_position, 0, 1);
    }
}

bool Vine::UpdateTarget() {
    if (m_targetInfo.expired()) {
        boost::weak_ptr<CharInfo> nearest;

        nearest = fn_800A8A7C(fn_80074194(), &m_charInfo->m_position);
        if (!nearest.expired()) {
            boost::shared_ptr<CharInfo> info = nearest.lock();
            Fn8040Vector3 infoPosition;
            Fn8040Vector3 position;
            Fn8040Vector3 offset;

            fn_800091D0(&infoPosition, &info->m_position);
            infoPosition.y = 0.0f;
            fn_800091D0(&position, &m_charInfo->m_position);
            position.y = 0.0f;
            fn_8040CD74(&offset, &infoPosition, &position);
            float radiusSq = m_params.radius * m_params.radius;
            if (fn_8040CF2C(&offset) < radiusSq) {
                m_targetInfo = info;
                fn_800091D0(&m_targetPosition, &info->m_position);
                return true;
            }
        }
    } else if (boost::shared_ptr<CharInfo> info = m_targetInfo.lock()) {
        Fn8040Vector3 infoPosition;
        Fn8040Vector3 position;
        Fn8040Vector3 offset;

        fn_800091D0(&infoPosition, &info->m_position);
        infoPosition.y = 0.0f;
        fn_800091D0(&position, &m_charInfo->m_position);
        position.y = 0.0f;
        fn_8040CD74(&offset, &infoPosition, &position);
        if (fn_8040CF2C(&offset) > GetLoseDistanceSq()) {
            m_targetInfo.reset();
            return false;
        }
        fn_800091D0(&m_targetPosition, &info->m_position);
        return true;
    }
    return false;
}

/* STATE_START: enters STATE_WAIT. */
void Vine::UpdateStart(float dt) {
    switch (m_step) {
    case 0:
        SetState(STATE_WAIT);
        break;
    /* Matching workaround: empty arms for steps 1 and 2, whose bodies are unknown (nothing in this
     * file sets step 2). With both arms the function is 0x28 bytes, as in the original; without
     * case 2 it is 0x2C bytes, without case 1 0x34 and without both 0x20. Each of the other state
     * functions likewise has an empty case 2: without it the original's extra branch after the
     * compares is lost and the function is one word shorter. Revisit when the body of step 2 is
     * found. */
    case 1:
        break;
    case 2:
        break;
    }
}

/* STATE_WAIT: plays the vine's group 0 and attacks once there has been a target for "attack sec". */
void Vine::UpdateWait(float dt) {
    switch (m_step) {
    case 0:
        SetSequenceGroup(0, 0);
        m_step = 1;
        m_timer = m_params.attackSec;
        break;
    case 1:
        if (UpdateTarget()) {
            m_timer -= dt;
            if (m_timer < 0.0f) {
                SetState(STATE_ATTACK);
            }
        }
        break;
    case 2: /* Matching workaround: as above (the original's branch for step 2). */
        break;
    }
}

/* STATE_ATTACK: plays the vine's group 1, shoots a fire ball at the motion's event 0x5000 and
 * returns to STATE_WAIT when the group ends. */
void Vine::UpdateAttack(float dt) {
    switch (m_step) {
    case 0:
        SetSequenceGroup(0, 1);
        m_step = 1;
        break;
    case 1:
        if (boost::shared_ptr<_stMtnPlayWork> motion = m_motions[0].lock()) {
            /* fn_80415A18 copies the event's data here; it is not used. */
            u8 payload[8];

            if (fn_80415A18(motion.get(), 0, s_fireBallEvent, payload, 0) != 0) {
                ShootFireBall();
            }
        }
        if (IsSequenceFinished(0)) {
            SetState(STATE_WAIT);
        }
        break;
    case 2: /* Matching workaround: as above (the original's branch for step 2). */
        break;
    }
}

/* STATE_DIE: plays the vine's group 3 and the seed's group 0, then enters STATE_DEAD. */
void Vine::UpdateDie(float dt) {
    switch (m_step) {
    case 0:
        m_models[1]->fn_8016271C();
        m_step = 1;
        SetSequenceGroup(0, 3);
        SetSequenceGroup(1, 0);
        break;
    case 1:
        if (IsSequenceFinished(0)) {
            SetState(STATE_DEAD);
        }
        break;
    case 2: /* Matching workaround: as above (the original's branch for step 2). */
        break;
    }
}

/* STATE_DEAD: once the seed's group has ended, waits "revive sec", then enters STATE_REVIVE with
 * the vine's group 4 and the seed's group 1. */
void Vine::UpdateDead(float dt) {
    switch (m_step) {
    case 0:
        m_step = 1;
        m_timer = m_params.reviveSec;
        m_models[0]->fn_801611A4(s_vine, -1);
        m_models[1]->fn_80161090(s_seed, -1);
        break;
    case 1:
        if (IsSequenceFinished(1)) {
            m_timer -= dt;
            if (m_timer <= 0.0f) {
                SetState(STATE_REVIVE);
                SetSequenceGroup(0, 4);
                SetSequenceGroup(1, 1);
            }
        }
        break;
    case 2: /* Matching workaround: as above (the original's branch for step 2). */
        break;
    }
}

/* STATE_REVIVE: restores the health, plays sound 0x7530 and enters STATE_WAIT when the vine's
 * group ends. */
void Vine::UpdateRevive(float dt) {
    switch (m_step) {
    case 0:
        m_step = 1;
        m_health = 3.0f;
        fn_801D54F4(fn_80053164(), 0x7530, &m_charInfo->m_position, 0, 1);
        break;
    case 1:
        if (IsSequenceFinished(0)) {
            SetState(STATE_WAIT);
            m_models[0]->fn_80161090(s_vine, -1);
            m_models[1]->fn_80162790();
            m_models[1]->fn_801611A4(s_seed, -1);
        }
        break;
    case 2: /* Matching workaround: as above (the original's branch for step 2). */
        break;
    }
}

/* STATE_DAMAGE: plays the vine's group 2, then returns to STATE_WAIT. */
void Vine::UpdateDamage(float dt) {
    switch (m_step) {
    case 0:
        SetSequenceGroup(0, 2);
        m_step = 1;
        break;
    case 1:
        if (IsSequenceFinished(0)) {
            SetState(STATE_WAIT);
        }
        break;
    case 2: /* Matching workaround: as above (the original's branch for step 2). */
        break;
    }
}

/* The constructor of MotionSequenceModule is defined here, among the Vine methods: the executable's
 * only copy of it is at 0x8014F144, in this file's code, and the EnemyMotionSequenceModule
 * constructor at 0x8018F3AC calls that copy. */
MotionSequenceModule::MotionSequenceModule()
    : m_data(0), m_group(0), m_previousGroup(0), field_0x10(-3), m_motion(-1), field_0x18(0),
      m_loop(0), m_finished(false), field_0x1B(0), field_0x1C(0), field_0x1E(0), m_groupIndex(0),
      field_0x24(0), m_entryCount(0), m_entryIndex(0), field_0x30(0), field_0x34(0.0f),
      field_0x38(0.0f) {
    field_0x1D = 1;
}

void Vine::SetupSequences() {
    m_motions[0] = m_models[0]->fn_801601A0(s_vine);
    m_motionBases[0] = m_models[0]->fn_80160124(s_vine);
    m_sequences[0] = new MotionSequenceModule;
    m_sequences[0]->m_data = &s_vineSequences;
    m_sequences[0]->SelectGroup(0);

    m_motions[1] = m_models[1]->fn_801601A0(s_seed);
    m_motionBases[1] = m_models[1]->fn_80160124(s_seed);
    m_sequences[1] = new MotionSequenceModule;
    m_sequences[1]->m_data = &s_seedSequences;
    m_sequences[1]->SelectGroup(0);
}

void Vine::UpdateSequences(float dt) {
    for (int i = 0; i < 2; i++) {
        if (m_sequences[i] != 0) {
            if (boost::shared_ptr<_stMtnPlayWork> motion = m_motions[i].lock()) {
                m_sequences[i]->fn_800C9178(motion.get(), m_motionBases[i], dt);
                m_sequences[i]->fn_800C9554(motion.get());
            }
        }
    }
}

void Vine::SetSequenceGroup(int model, int group) {
    if (m_sequences[model] != 0) {
        m_sequences[model]->SelectGroup(group);
    }
}

bool Vine::IsSequenceFinished(int model) {
    if (m_sequences[model] != 0) {
        return m_sequences[model]->m_finished;
    }
    return false;
}

float Vine::GetLoseDistanceSq() {
    float distance = m_params.radius + 10.0f;

    return distance * distance;
}

} // namespace Vine

/* Uses the literal: the original takes the string's address here instead of loading s_modelFile. */
extern "C" void fn_8014EE20(void) {
    fn_8006B474("VINE.BIN", 1);
}

extern "C" void fn_8014EE1C(void) {}

void Vine::Vine::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new Vine(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "radius", 0, 0, 0, 100.0f, 1.0f, 10000.0f, fn_800CBC20, 0},
    {4, "attack sec", 0, 0, 0, 1.0f, 0.0f, 10000.0f, fn_800CBC20, 0},
    {4, "revive sec", 0, 0, 0, 1.0f, 0.0f, 10000.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8052A494 = {
    "FIRE_VINE", fn_8014EE20, fn_8014EE1C, Vine::Vine::Create, 0, 0, 0x1131, 1, 0, 10, s_paramInfo,
};

namespace Vine {

const char *FireBall::s_name = "FireBall";

FireBallParam_8014EDBC::FireBallParam_8014EDBC() : m_lifetime(5.0f) {}

FireBallParam_8014EDBC &FireBallParam_8014EDBC::operator=(const FireBallParam_8014EDBC &other) {
    fn_800091D0(&m_position, &other.m_position);
    fn_800091D0(&m_velocity, &other.m_velocity);
    m_lifetime = other.m_lifetime;
    return *this;
}

FireBall::FireBall(Task *parent, const FireBallParam_8014EDBC &param) : Task(parent) {
    m_name = s_name;
    m_param = param;
    m_clump =
        fn_80457090(static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, "JG0000FVINEBALL.DFF")));
    m_charInfo = fn_8015CB88(0x46, 4, "FireBall");
    fn_800091D0(&m_charInfo->m_position, &m_param.m_position);
    m_target = fn_8016690C(m_clump);
    m_target->fn_80167010(-1);

    void *collision = fn_80012700(fn_800122D4(), 1, "JG0000FVINEBALL.CCL");
    {
        /* The original destroys probe before it clears m_state: it lives in this block. */
        boost::shared_ptr<CharaColliBase> probe =
            fn_800C81F4(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), collision, 6, 0);

        m_handle = fn_8015B8C4(probe);
        m_handle->field_0x0.fn_800F5980(
            GadgetColliDetectCallback(boost::bind(&FireBall::OnCollision, this, _1)));
    }
    m_state = 0;
}

FireBall::~FireBall() {
    m_target.reset();
    if (m_clump != 0) {
        if (RpWorld *world = fn_8046256C(m_clump)) {
            fn_804623EC(world, m_clump);
        }
        fn_80457468(m_clump);
        m_clump = 0;
    }
}

void FireBall::Update(float dt) {
    switch (m_state) {
    case 0:
        m_state = 1;
        break;
    case 1: {
        Fn8040Vector3 hit;
        Fn8040Vector3 end;
        Fn8040Vector3 next;

        fn_8040CDD0(&next, &m_charInfo->m_position, &m_param.m_velocity);
        fn_800091EC(&end, &next);
        if (fn_8015D61C(&hit, m_charInfo->m_position, end)) {
            fn_800091D0(&m_charInfo->m_position, &hit);
            m_state = 2;
        } else {
            AddVector(&m_charInfo->m_position, m_param.m_velocity);
        }
        m_param.m_lifetime -= dt;
        if (m_param.m_lifetime < 0.0f) {
            m_state = 3;
        }
        break;
    }
    case 2:
        FireBallBlast::Create(&m_charInfo->m_position);
        m_state = 3;
        break;
    case 3:
        m_flags |= 1;
        break;
    }

    UpdateClumpPosition();
    if (m_handle.get() != 0) {
        fn_8015BE9C(m_handle.get(), dt);
    }
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }
}

void FireBall::UpdateClumpPosition() {
    RwV3d position;

    position.x = m_charInfo->m_position.x;
    position.y = m_charInfo->m_position.y;
    position.z = m_charInfo->m_position.z;

    fn_804875B0(static_cast<RwFrame *>(m_clump->object.parent), &position, rwCOMBINEREPLACE);
}

void FireBall::OnCollision(ColliDetectArgs &args) {
    if (m_state == 1) {
        m_state = 2;
    }
}

void FireBall::Create(const FireBallParam_8014EDBC &param) {
    new FireBall(fn_801EB304(fn_8001C4F8(), 11), param);
}

const char *FireBallBlast::s_name = "FireBallBlast";

FireBallBlast::FireBallBlast(Task *parent, const Fn8040Vector3 *position) : Task(parent) {
    m_name = s_name;
    fn_800091D0(&m_position, position);

    boost::shared_ptr<CharaColliBase> probe =
        fn_800C7EA8(fn_80021B08(), &m_position, 0x40000, 6, 0,
                    boost::weak_ptr<CharInfo>(lbl_805EE698), &lbl_805E07A0, 1.0f);

    m_handle = fn_8015B8C4(probe);

    Fn8040Vector3 direction(0.0f, 1.0f, 0.0f);

    fn_80168C48(&m_position, &direction, 0x1A);
    m_state = 0;
    m_timer = 0.0f;
    m_radius = 0.0f;
}

FireBallBlast::~FireBallBlast() {}

void FireBallBlast::Update(float dt) {
    switch (m_state) {
    case 0:
        m_timer = 0.0f;
        m_state = 1;
        m_radius = 5.0f;
        fn_801D54F4(fn_80053164(), 0x7544, &m_position, 0, 1);
        break;
    case 1:
        m_timer += dt;
        if (m_timer >= 0.13f) {
            m_state = 2;
            m_timer = 0.0f;
            if (m_handle.get() != 0) {
                boost::weak_ptr<CharaColliAttack> weakAttack =
                    fn_800C4374(fn_80021B08(), m_handle->m_probe);

                if (boost::shared_ptr<CharaColliAttack> attack = weakAttack.lock()) {
                    attack->field_0x54 = 1;
                    attack->field_0x38 = 3;
                    attack->field_0x34 = 2.0f;
                    attack->field_0x40 = 1;
                    attack->field_0x10.field_0x0 =
                        AttackCallback(boost::bind(&FireBallBlast::OnAttack, this, _1));
                }
                m_handle->m_probe->SetSize(m_radius, -2, 0.0f, 0.0f);
            }
        }
        break;
    case 2:
        m_timer += dt;
        if (m_timer >= 0.87f) {
            m_timer = 0.87f;
            m_state = 3;
        }
        /* The radius grows from 5 to 15 while the timer runs to 0.87. */
        m_radius = (15.0f - 5.0f) / 0.87f * m_timer + 5.0f;
        if (m_radius >= 15.0f) {
            m_radius = 15.0f;
        }
        if (m_handle.get() != 0) {
            m_handle->m_probe->SetSize(m_radius, -2, 0.0f, 0.0f);
        }
        break;
    case 3:
        m_flags |= 1;
        break;
    }

    if (m_handle.get() != 0) {
        fn_8015BE9C(m_handle.get(), dt);
    }
}

void FireBallBlast::OnAttack(AttackCallbackParam &param) {}

void FireBallBlast::Create(const Fn8040Vector3 *position) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new FireBallBlast(parent, position);
}

} // namespace Vine
