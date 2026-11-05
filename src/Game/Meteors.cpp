/* MeteorsHolder and MeteorsTask (class names from RTTI; the gadget's descriptor is "METEOR_S"):
 * the placed meteor and the fragments that Meteor breaks into. Both own a Meteors (class name from
 * the bound callbacks' RTTI strings), which waits until a character comes within its range, then
 * flies along its direction while turning, shifts its model colour in the last second of its
 * lifetime and breaks up with an effect, a CommonBomb and a sound when the lifetime runs out, when
 * attacks have used up its health or when its swept sphere hits an object (behaviour inferred).
 * File name inferred from the class. Method names marked "(name inferred)" are not original.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
/* Matching workaround: TNumEdit.h and CharaColliBaseCharInfo.h are included for the RTTI data that
 * their never-called stand-in functions emit (see TNumEdit.h); this file uses neither class.
 * TNumEdit.h comes first, in the order their names appear in the unit's .rodata (TNumEdit at
 * 0x804F0768 in USA, then CharaColliBase); alphabetical order would reverse them. Revisit when the
 * original inline functions behind that data are found. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/AngleConversion.h"
#include "Game/CharInfo.h"
#include "Game/CommonBomb.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/Meteors.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkModelAtomicState.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <cstring>

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;
using SonicteamUSA::System::CharaColli::CharaColliBase;
using SonicteamUSA::System::Collision::CollisionResultList;

/* The radius of the sphere that a moving meteor sweeps along its velocity (name inferred).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original type of this variable.
 * The original's start-up function in .ctors (0x80220124) loads the value and stores it to this
 * .sbss word (0x805EFD74); a constant float initialiser would be emitted as initialised .sdata data
 * instead, and the shared stand-in reproduces that store. The original type is unknown. Revisit
 * when the original wrapper type is found. */
static UnkStartupFloat_800E2570 s_sweepRadius(25.0f);

/* The model colour at the start and at the end of the last second of the lifetime (names
 * inferred), the same values as Meteor's. */
static UnkDebugColor_802E0E84 s_startColor(0xFF, 0x4B, 0x1B, 0x1B);
static UnkDebugColor_802E0E84 s_endColor(0xFF, 0xFF, 0x73, 0x00);

/* The flag bits of the objects that do not break the meteor up: the sweep's predicate skips the
 * records whose flags have any of them and the collision callback skips the objects whose flags
 * word at 0x1C has any of them (name inferred). */
static const u32 s_ignoredFlags = 0x40018;

/* The probe predicate of the sweep in Meteors::Update (placeholder name; behaviour inferred):
 * skips the meteor's own collision object, which the sweep passes as the context, and the records
 * with any of s_ignoredFlags. */
static bool fn_8021FE5C(const UnkProbeFilterRecord *record) {
    if (record->target != 0 && record->target == record->context) {
        return false;
    }
    if (record->flags & s_ignoredFlags) {
        return false;
    }
    return true;
}

/* The original stores zero to the three rotation speeds before it stores the converted values. */
UnkMeteorsParams_8021FCE4::UnkMeteorsParams_8021FCE4(const MeteorParams &params)
    : m_rotateSpeed(0.0f, 0.0f, 0.0f) {
    m_range = 10.0f * params.range;
    m_moveSpeed = 10.0f * params.moveSpeed;
    m_rotateSpeed.x = DegreesToRadians(params.rotateSpeedX);
    m_rotateSpeed.z = DegreesToRadians(params.rotateSpeedZ);
    m_rotateSpeed.y = DegreesToRadians(params.rotateSpeedY);

    Fn8040Vector3 forward(0.0f, 0.0f, 1.0f);
    Fn8040Vector3 turnedX;
    Fn8040Vector3 turnedY;

    fn_800091D0(&m_direction, &forward);
    fn_8040D118(&turnedX, &m_direction, DegreesToRadians(params.directionX));
    fn_800091D0(&m_direction, &turnedX);
    fn_8040D0AC(&turnedY, &m_direction, DegreesToRadians(params.directionY));
    fn_800091D0(&m_direction, &turnedY);

    m_lifetime = params.lifetime;
}

UnkMeteorsParams_8021FCE4::UnkMeteorsParams_8021FCE4(float range, float moveSpeed,
                                                     const Fn8040Vector3 &direction,
                                                     UnkRotation3 rotateSpeed, float lifetime)
    : m_range(range), m_moveSpeed(moveSpeed) {
    fn_800091EC(&m_direction, &direction);
    m_rotateSpeed = rotateSpeed;
    m_lifetime = lifetime;
}

UnkMeteorsParams_8021FCE4::UnkMeteorsParams_8021FCE4(const UnkMeteorsParams_8021FCE4 &other)
    : m_range(other.m_range), m_moveSpeed(other.m_moveSpeed) {
    fn_800091EC(&m_direction, &other.m_direction);
    m_rotateSpeed = other.m_rotateSpeed;
    m_lifetime = other.m_lifetime;
}

Meteors::Meteors(Task *task, const Fn8040Vector3 &position, const UnkRotation3 &rotation,
                 const UnkMeteorsParams_8021FCE4 &params, int state)
    : m_task(task), m_state(state), m_params(params), m_flags(0), m_timer(params.m_lifetime),
      m_health(1.0f) {
    m_flags.Set(0);

    m_charInfo = fn_8015CD90(0x62, 4);
    fn_800091D0(&m_charInfo->m_position, &position);
    m_charInfo->m_rotation = rotation;
    m_object = fn_801E72DC(0xA800);
    m_model = fn_8015F800(1, "METEORS.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80162424(0);
    UnkModelAtomicState *atomicState = m_model->fn_80162298(0);
    atomicState->color = s_startColor;
    UpdateModelMatrix();

    m_target = fn_80166AF4();
    {
        UnkWeakHolder_800CC6C8 weakTarget;
        weakTarget.ptr = m_target;
        m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        m_target->fn_80167010(-1);
    }

    m_model->fn_80161FC8(0, m_charInfo);
    m_model->m_collisionCallback =
        ColliDetectCallback(boost::bind(&Meteors::OnCollision, this, _1));

    {
        boost::shared_ptr<CharaColliBase> probe(m_model->fn_80161614(0).lock());
        if (probe.get() != 0) {
            probe->UnkSlot20(0x100100, -1);
        }
    }
    m_model->fn_80161230(0, -1);

    boost::shared_ptr<CharaColliAttack> attack(m_model->fn_80161428(0).lock());
    if (attack.get() != 0) {
        attack->fn_80096B64(4, AttackCallback(boost::bind(&Meteors::OnAttack, this, _1)));
        attack->fn_80054DD0(1, 1, AttackCallback(boost::bind(&Meteors::fn_8021E4D8, this, _1)),
                            AttackCallback(boost::bind(&Meteors::fn_8021E4A4, this, _1)), 3.0f);
    }
}

/* Matching workaround: auto_inline around this destructor. The original keeps checked_delete<T>
 * out of line and emits this destructor at its definition; with the stock GC/2.7 compiler and
 * -inline noauto that needs auto_inline here. A tooling constraint, not a claim about the original
 * source; revisit if a destructor called from checked_delete turns out to be inlined in the
 * original. */
#pragma auto_inline on
Meteors::~Meteors() {}
#pragma auto_inline reset

void Meteors::Update(float dt) {
    switch (m_state) {
    case 0: {
        boost::shared_ptr<CharInfo> nearest(
            fn_800A8A7C(fn_80074194(), &m_charInfo->m_position).lock());
        if (nearest.get() != 0) {
            /* The original computes the difference into a compiler temporary and then copies it;
             * the named local stands in for that temporary (the stack-offset residual noted in
             * configure.py). */
            Fn8040Vector3 difference;
            Fn8040Vector3 offset;

            fn_8040CD74(&difference, &nearest->m_position, &m_charInfo->m_position);
            fn_800091EC(&offset, &difference);
            if (fn_8040CF50(&offset) < m_params.m_range) {
                m_state = 1;
            }
        }
        break;
    }
    case 1: {
        /* A named local for the original's compiler temporary, as for the difference above. */
        Fn8040Vector3 velocity;

        fn_8040CDA8(&velocity, &m_params.m_direction, m_params.m_moveSpeed * dt);
        fn_800091D0(&m_charInfo->field_0x18, &velocity);
        AddVector(&m_charInfo->m_position, m_charInfo->field_0x18);

        m_timer -= dt;
        if (m_timer < 0.0f) {
            m_flags.Set(1);
        } else if (m_timer < 1.0f) {
            float t = 1.0f - m_timer;
            float red;
            float green;
            float blue;

            red = t * (s_endColor.field_0x1 - s_startColor.field_0x1);
            green = t * (s_endColor.field_0x2 - s_startColor.field_0x2);
            blue = t * (s_endColor.field_0x3 - s_startColor.field_0x3);
            red += s_startColor.field_0x1;
            red = fn_80070460(red, 0.0f, 255.0f);
            green += s_startColor.field_0x2;
            green = fn_80070460(green, 0.0f, 255.0f);
            blue += s_startColor.field_0x3;
            blue = fn_80070460(blue, 0.0f, 255.0f);

            UnkModelAtomicState *atomicState = m_model->fn_80162298(0);
            u8 blueByte = blue;
            u8 greenByte = green;
            u8 redByte = red;

            atomicState->color.field_0x0 = 0xFF;
            atomicState->color.field_0x1 = redByte;
            atomicState->color.field_0x2 = greenByte;
            atomicState->color.field_0x3 = blueByte;
        }
        break;
    }
    }

    if (m_state != 2) {
        if (!m_flags.Test(0)) {
            boost::shared_ptr<CharaColliBase> probe(m_model->fn_80161614(0).lock());
            if (probe.get() != 0) {
                UnkSphere_80021C04 sphere(&m_charInfo->m_position, s_sweepRadius.value);
                Fn8040Vector3 motion;

                fn_800091EC(&motion, &m_charInfo->field_0x18);
                boost::shared_ptr<CollisionResultList> hits(fn_80021934(
                    fn_80020D40(), &sphere, &motion, 3, 0xF, 0, fn_8021FE5C, probe.get(), false));
                if (hits.get() != 0) {
                    m_flags.Set(1);
                }
            }
        }

        m_charInfo->m_rotation.x += m_params.m_rotateSpeed.x * dt;
        m_charInfo->m_rotation.y += m_params.m_rotateSpeed.y * dt;
        m_charInfo->m_rotation.z += m_params.m_rotateSpeed.z * dt;
        UpdateModelMatrix();
        m_model->fn_80162FB0(dt);
        if (m_target.get() != 0) {
            m_target->fn_8016761C(dt);
        }
    }

    if (m_flags.Test(0)) {
        m_model->fn_8016111C(0, -1);
        m_flags.Reset(0);
    }

    if (m_flags.Test(1)) {
        m_model->fn_80161A24(0);
        if (m_target.get() != 0) {
            m_target->fn_80166F88();
            m_target.reset();
        }
        m_model->fn_80162790();

        UnkEffectParams_8015F0B8 parameters(m_charInfo, 0);
        UnkModelEntry_8015F7BC *entry = m_model->fn_8015F7BC(1);
        parameters.field_0x44 = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, entry->name));
        parameters.field_0x40 = m_charInfo->m_position.y;
        parameters.field_0x34 = 1.0f;
        parameters.field_0x4A = 0;
        parameters.field_0x54 = 1;
        fn_8015D93C(&parameters);

        fn_801DD0BC(&m_charInfo->m_position, 1, 7, 3.0f);
        fn_801D54F4(fn_80053164(), 0x772F, &m_charInfo->m_position, 0, 1);

        m_task->m_flags |= 1;
        m_state = 2;
        m_flags.Reset(1);
    }
}

void Meteors::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();

    /* Draws the range around the meteor and a line along its path, at least 50 long. */
    fn_802E0E84(&m_charInfo->m_position, lbl_805F06D8, m_params.m_range);

    float length = m_params.m_lifetime * m_params.m_moveSpeed;
    if (length < 50.0f) {
        length = 50.0f;
    }

    Fn8040Vector3 path;
    Fn8040Vector3 scaled;
    Fn8040Vector3 end;

    fn_8040CDA8(&scaled, &m_params.m_direction, length);
    fn_800091EC(&path, &scaled);
    fn_8040CDD0(&end, &m_charInfo->m_position, &path);

    UnkSegment_80020A20 segment(m_charInfo->m_position, end);

    fn_802E0AA0(segment, UnkDebugColor_802E0E84(0xFF, 0xFF, 0xFF, 0), 10.0f);
}

void Meteors::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
    m_model->fn_801633DC(&matrix);
    fn_800CCE38(m_object.get(), &m_charInfo->m_position);
}

void Meteors::OnCollision(ColliDetectArgs &args) {
    if (m_state != 2 && args.m_hits != 0) {
        for (UnkProbeIterator_80027374 iterator(args.m_hits->fn_8001EE04());
             iterator.node != args.m_hits->fn_8001EDEC(); fn_80027374(&iterator, 0)) {
            if ((iterator.node->field_0xC->flags & s_ignoredFlags) == 0) {
                m_flags.Set(1);
                return;
            }
        }
    }
}

void Meteors::fn_8021E4D8(AttackCallbackParam &param) {
    fn_800091D0(&param.field_0x34, &m_params.m_direction);
    param.field_0x4C = m_params.m_moveSpeed;
    param.field_0x50 = 3.0f;
}

void Meteors::fn_8021E4A4(AttackCallbackParam &param) {
    if (param.field_0x54 == 1) {
        m_flags.Set(1);
    }
}

void Meteors::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 1;
    if (m_state != 2) {
        m_health -= param.field_0x50;
        if (m_health < 0.0f || fn_8000A950(m_health)) {
            param.field_0x54 = 3;

            boost::shared_ptr<CharInfo> attacker = fn_8015D570(&param);
            if (attacker.get() != 0) {
                fn_8020642C(attacker, 0x49);
            }
            m_flags.Set(1);
        }
    }
}

const char *MeteorsTask::s_name = "MeteorsTask";

MeteorsTask::MeteorsTask(Task *parent, const Fn8040Vector3 &position, const UnkRotation3 &rotation,
                         const UnkMeteorsParams_8021FCE4 &params, int state)
    : Task(parent) {
    m_name = s_name;
    m_meteors = boost::shared_ptr<Meteors>(new Meteors(this, position, rotation, params, state));
}

MeteorsTask::~MeteorsTask() {}

void MeteorsTask::Update(float dt) {
    if (m_meteors.get() != 0) {
        m_meteors->Update(dt);
    }
}

const char *MeteorsHolder::s_name = "MeteorsHolder";

MeteorsHolder::MeteorsHolder(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
}

MeteorsHolder::~MeteorsHolder() {}

extern "C" void fn_8021E19C(void) {
    fn_8006B474("METEORS.BIN", 1);
}

extern "C" void fn_8021E198(void) {}

void MeteorsHolder::Create(UnkGadgetOwner_800C9CD0 *owner) {
    MeteorsHolder *holder = new MeteorsHolder(fn_801EB304(fn_8001C4F8(), 11), owner);

    holder->CreateMeteors(owner);
    fn_800C9AD4(owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "RANGE m", 0, 0, 0, 50.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "MOVE_SPD m/s", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "DIRECTION_X deg", 0, 0, 0, 0.0f, -180.0f, 180.0f, fn_800CBC20, 0},
    {4, "DIRECTION_Y deg", 0, 0, 0, 0.0f, -180.0f, 180.0f, fn_800CBC20, 0},
    {4, "ROT_SPD_X deg/s", 0, 0, 0, 0.0f, -1080.0f, 1080.0f, fn_800CBC20, 0},
    {4, "ROT_SPD_Z deg/s", 0, 0, 0, 0.0f, -1080.0f, 1080.0f, fn_800CBC20, 0},
    {4, "ROT_SPD_Y deg/s", 0, 0, 0, 0.0f, -1080.0f, 1080.0f, fn_800CBC20, 0},
    {4, "LIFETIME s", 0, 0, 0, 5.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80545E40 = {
    "METEOR_S", fn_8021E19C, fn_8021E198, MeteorsHolder::Create, 0, 0, 0x138B,
    1,          0,           10,          s_paramInfo,
};

void MeteorsHolder::UpdateBody(float dt) {
    if (m_meteors.get() != 0) {
        m_meteors->Update(dt);
    }
}

void MeteorsHolder::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    MeteorParams previous = m_params;

    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    if (memcmp(&previous, &m_params, sizeof(m_params)) != 0) {
        CreateMeteors(owner);
    }
    if (m_meteors.get() != 0) {
        m_meteors->ReadOwner(owner);
    }
}

void MeteorsHolder::CreateMeteors(UnkGadgetOwner_800C9CD0 *owner) {
    Fn8040Vector3 position;
    UnkRotation3 rotation(0.0f, 0.0f, 0.0f);

    fn_800C9F14(owner, &position);
    fn_800C9ED4(owner, &rotation);
    UnkMeteorsParams_8021FCE4 params(m_params);
    m_meteors = boost::shared_ptr<Meteors>(new Meteors(this, position, rotation, params, 0));
}
