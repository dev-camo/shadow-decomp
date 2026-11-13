/* ThrowObject (class name from RTTI) and ThrowObjectImpl (class name from the RTTI name of its
 * bound callbacks): a task that moves a position under a constant acceleration and sweeps a
 * sphere along each step (described in Game/ThrowObject.h). File name inferred from the class.
 * Method and member names are inferred. Definitions are in reverse address order (deferred
 * inlining emits functions last-defined first).
 *
 * The attack object's two callbacks are boost::function objects built with
 * boost::bind(&ThrowObjectImpl::OnAttackCallback0, this, _1) and the same for OnAttackCallback1;
 * their manager, invoker and copy functions come from the boost headers. */
#include "Game/ThrowObject.h"
#include "Game/CharInfo.h"
#include "Game/CharaColliAttack.h"
#include "Game/EffectElement.h"
#include "Game/PlayerBase.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

using Effect::System::EffectElement;
using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;
using SonicteamUSA::System::CharaColli::CharaColliBase;
using SonicteamUSA::System::Collision::CollisionResultList;

/* A value that nothing in this file reads (name inferred). Its bits are 0x3CA3D70B, the float just
 * above 0.02f (0x3CA3D70A), which suggests that the original initialiser was an expression:
 * 1.2f / 60.0f and 20.0f * 0.001f both give it, and neither is established. 0.020000001f is the
 * shortest literal with that value.
 * Matching workaround: the original sets this float with a start-up function in .ctors (a
 * constant initialiser would be emitted as initialised .sdata data instead).
 * UnkStartupFloat_800E2570 stands in for the original initialiser expression and gives that
 * start-up function. Revisit when the original initialiser is found. */
static const float s_unusedValue = UnkStartupFloat_800E2570(0.020000001f).value;

/* What SweepSphere passes to its predicate as the context (name inferred). */
struct SweepFilter {
    /* 0x0 */ u32 m_flags;
    /* 0x4 */ boost::shared_ptr<CharaColliBase> m_ignored;
};
CHECK_SIZEOF(SweepFilter, 0xC);

/* The probe predicate of SweepSphere (name inferred). Skips a record whose collision object is
 * the filter's m_ignored, when neither is null. Of the other records, one with zero flags is
 * tested, and one with flags only when they share a bit with the filter's. */
static bool IsSweepTarget(const UnkProbeFilterRecord *record) {
    const SweepFilter *filter = static_cast<const SweepFilter *>(record->context);
    /* Read before the tests, as in the original (USA (GUPE8P) 0x8027DCD0). */
    u32 flags = filter->m_flags;

    if (filter->m_ignored.get() != 0 && record->target != 0 &&
        filter->m_ignored.get() == record->target) {
        return false;
    }
    if (record->flags != 0) {
        return (record->flags & flags) != 0;
    }
    return true;
}

/* Sweeps a sphere of the given radius from position along a copy of move (fn_80021934 with the
 * flags 3, arg3 and IsSweepTarget, which gets flags and ignored as its filter). Afterwards it
 * adds the copy to the sphere's centre and stores the centre in position. When the sweep returns
 * a hit list, it passes attack and the list to fn_800C4190 and deletes the list. Returns whether
 * there was a list (name inferred). */
static bool SweepSphere(Fn8040Vector3 *position, const Fn8040Vector3 &move, u32 flags, int arg3,
                        const boost::shared_ptr<CharaColliBase> &ignored,
                        const boost::shared_ptr<CharaColliAttack> &attack, float radius) {
    UnkWorld_80020D40 *world = fn_80020D40();
    UnkSphere_80021C04 sphere(position, radius);
    Fn8040Vector3 motion = move;
    SweepFilter filter;

    filter.m_flags = flags;
    filter.m_ignored = ignored;

    /* hits is declared before hit; declared after it, the two exchange their registers here. The
     * original declaration order is not known. */
    CollisionResultList *hits;
    bool hit = false;

    hits = fn_80021934(world, &sphere, &motion, 3, arg3, 0, IsSweepTarget, &filter, false);

    fn_8040CFF8(&sphere.m_center, &motion);

    if (hits != 0) {
        hit = true;

        UnkWorld_80021B08 *colliWorld = fn_80021B08();

        fn_800C4190(colliWorld, attack, hits, 0);
        delete hits;
    }

    *position = sphere.m_center;
    return hit;
}

/* ThrowObjectImpl (class name from the RTTI name of its bound callbacks): the state of a
 * ThrowObject. */
class ThrowObjectImpl {
  public:
    ThrowObjectImpl();
    ~ThrowObjectImpl();

    /* Advances m_position and m_velocity under m_acceleration by dt (fn_804103E8 with a drag of
     * 0), passes the velocity to SetDirection and adds the distance from m_prevPosition to
     * m_position to m_distance. */
    void Advance(float dt);
    /* Stores the pitch and the yaw of direction (fn_8040F760) in m_rotation's x and y, unless
     * direction's squared length is below 0.0001. */
    void SetDirection(const Fn8040Vector3 &direction);
    /* Calls SweepSphere from m_prevPosition along the step to m_position, with every flag bit
     * except 0xC0018, the lock of field_0x0, m_attack and m_radius. Only when it returns true,
     * stores the position it gives in m_position and the step divided by dt in m_velocity, and
     * returns true. */
    bool Sweep(float dt);
    /* Stores in m_attack the attack object that fn_8005A300 creates for owner with the two
     * callbacks below and 6. */
    void CreateAttack(const boost::weak_ptr<CharInfo> &owner);

    /* The two callbacks the attack object is created with, in the order of fn_8005A300's
     * callback0 and callback1. The first stores the normalised velocity in the parameters'
     * field_0x34 and the velocity's length in field_0x4C. The second, when the attack object
     * exists, passes a copy of the parameters' field_0x8 to its fn_800532F8. */
    void OnAttackCallback0(AttackCallbackParam &param);
    void OnAttackCallback1(AttackCallbackParam &param);

    /* Sweep passes its lock to SweepSphere as the collision object to skip. Nothing assigns it
     * (the class is used in this file only), so that lock is always empty. */
    /* 0x00 */ boost::weak_ptr<CharaColliBase> field_0x0;
    /* 0x08 */ boost::shared_ptr<CharaColliAttack> m_attack;
    /* 0x10 */ Fn8040Vector3 m_position;
    /* 0x1C */ Fn8040Vector3 m_prevPosition; /* m_position at the end of the previous update */
    /* 0x28 */ Fn8040Vector3 m_velocity;
    /* 0x34 */ Fn8040Vector3 m_acceleration;
    /* The effect's rotation argument. SetDirection stores x and y; nothing stores z, and the
     * constructor does not initialise it. */
    /* 0x40 */ Fn8040Vector3 m_rotation;
    /* 0x4C */ float m_radius;
    /* 0x50 */ float m_range;
    /* 0x54 */ float m_distance; /* the summed step lengths */
    /* 0x58 */ boost::weak_ptr<EffectElement> m_effect;
    /* 0x60 */ boost::weak_ptr<CharInfo> m_owner;
};
CHECK_SIZEOF(ThrowObjectImpl, 0x68);

ThrowObjectImpl::ThrowObjectImpl() {
    m_distance = 0.0f;
}

/* Matching workaround: auto_inline around this destructor. The original keeps checked_delete<T>
 * out of line (USA (GUPE8P) 0x8027D040, called by scoped_ptr's destructor at 0x8027CFF4) and
 * emits this destructor at its definition; with the stock GC/2.7 compiler and -inline noauto
 * that needs auto_inline here: without it checked_delete is expanded in scoped_ptr's destructor
 * and has no copy of its own (measured). A tooling constraint, not a claim about the original
 * source; revisit if another compiler build reproduces this without the pragma. */
#pragma auto_inline on
ThrowObjectImpl::~ThrowObjectImpl() {
    if (m_attack.get() != 0) {
        m_attack.reset();
    }
}
#pragma auto_inline reset

void ThrowObjectImpl::Advance(float dt) {
    fn_804103E8(&m_position, &m_velocity, &m_acceleration, 0.0f, dt);
    SetDirection(m_velocity);
    m_distance += fn_8040D1B4(&m_position, &m_prevPosition);
}

void ThrowObjectImpl::SetDirection(const Fn8040Vector3 &direction) {
    if (direction.x * direction.x + direction.y * direction.y + direction.z * direction.z <
        0.0001f) {
        return;
    }

    Fn8040Vector2 angles = fn_8040F760(&direction);

    m_rotation.x = angles.x;
    m_rotation.y = angles.y;
}

bool ThrowObjectImpl::Sweep(float dt) {
    Fn8040Vector3 move = m_position - m_prevPosition;
    Fn8040Vector3 position = m_prevPosition;

    if (SweepSphere(&position, move, ~0xC0018u, 3, field_0x0.lock(), m_attack, m_radius)) {
        m_position = position;
        m_velocity = move * (1.0f / dt);
        return true;
    }
    return false;
}

void ThrowObjectImpl::CreateAttack(const boost::weak_ptr<CharInfo> &owner) {
    m_attack = fn_8005A300(
        0, owner, AttackCallback(boost::bind(&ThrowObjectImpl::OnAttackCallback0, this, _1)),
        AttackCallback(boost::bind(&ThrowObjectImpl::OnAttackCallback1, this, _1)), 6.0f);
}

void ThrowObjectImpl::OnAttackCallback0(AttackCallbackParam &param) {
    param.field_0x34 = fn_8040CEE8(m_velocity);
    param.field_0x4C = fn_8040CF50(m_velocity);
}

void ThrowObjectImpl::OnAttackCallback1(AttackCallbackParam &param) {
    if (m_attack.get() != 0) {
        m_attack->fn_800532F8(param.field_0x8);
    }
}

ThrowObject::ThrowObject(const boost::weak_ptr<CharInfo> &owner, const Fn8040Vector3 &position,
                         const Fn8040Vector3 &velocity, const Fn8040Vector3 &acceleration,
                         float radius, float range)
    : Task(fn_801EB304(fn_8001C4F8(), 11)), m_impl(new ThrowObjectImpl()) {
    m_name = "ThrowObject";

    m_impl->m_position = m_impl->m_prevPosition = position;
    m_impl->m_velocity = velocity;
    m_impl->m_acceleration = acceleration;
    m_impl->m_radius = radius;
    m_impl->m_range = range;
    m_impl->SetDirection(velocity);

    if (!owner.expired() && owner.lock()->fn_80078B44()) {
        m_impl->CreateAttack(fn_800B75DC(owner));
    } else {
        m_impl->CreateAttack(owner);
    }

    m_impl->m_owner = owner;
}

ThrowObject::~ThrowObject() {
    if (!m_impl->m_effect.expired()) {
        m_impl->m_effect.lock()->RequestStop();
        m_impl->m_effect.reset();
    }
}

void ThrowObject::Update(float dt) {
    m_impl->Advance(dt);

    if (m_impl->Sweep(dt)) {
        OnHit();
        m_flags |= 1;
    }

    if (m_impl->m_distance > m_impl->m_range) {
        OnRangeOver();
        m_flags |= 1;
    }

    m_impl->m_prevPosition = m_impl->m_position;
}

Fn8040Vector3 ThrowObject::GetPosition() {
    return m_impl->m_position;
}

void ThrowObject::StartEffect(int id) {
    if (!m_impl->m_effect.expired()) {
        m_impl->m_effect.lock()->RequestStop();
        m_impl->m_effect.reset();
    }

    m_impl->m_effect = fn_801D2BEC(0, &m_impl->m_position, &m_impl->m_rotation, id, 0);
}

void ThrowObject::OnRangeOver() {}

void ThrowObject::OnHit() {}

boost::weak_ptr<CharInfo> ThrowObject::GetOwner() {
    return m_impl->m_owner;
}
