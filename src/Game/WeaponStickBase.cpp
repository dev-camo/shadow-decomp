/* Weapon::WeaponStickBase and Weapon::WeaponStickBaseImpl (class names from RTTI): the base of the
 * melee weapons that strike along a line. The stick keeps the line between two updates and, while
 * it attacks, tests an attack object along segments interpolated from the previous line to the
 * current one; hits are counted through the callbacks the attack object calls, and the model
 * stretches while the attack lasts. File name inferred from the class. Method and member names
 * marked inferred are not original. Definitions are in reverse address order (deferred inlining
 * emits functions last-defined first).
 *
 * The callbacks are boost::function objects built with boost::bind(&WeaponStickBaseImpl::method,
 * this, _1); their manager, invoker and copy functions come from the boost headers. */
#include "Game/CharInfo.h"
#include "Game/UnkAttack_8005A840.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkProbeTarget_800587A0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "Game/WeaponStickBase.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;

namespace Weapon {

/* Weapon::WeaponStickBaseImpl (class name from RTTI): the state of a weapon stick. */
class WeaponStickBaseImpl {
  public:
    WeaponStickBaseImpl(const boost::weak_ptr<CharInfo> &owner, float arg1);
    ~WeaponStickBaseImpl() {
        ReleaseAttack();
    }

    /* The stick's methods of the same names forward to these four (names inferred). */
    void Update(float dt);
    void StartAttack();
    void EndAttack();
    void SetLine(const Fn8040Vector3 &start, const Fn8040Vector3 &end);
    /* Tests the attack along segments interpolated from the previous line to the current one, a
     * sweep step apart (name inferred). */
    void SweepAttack();
    /* Creates the attack object, binds the callbacks and marks the stick attacking (name
     * inferred). */
    void CreateAttack();
    /* Releases the attack object and clears the mark (name inferred). */
    void ReleaseAttack();

    /* The two callbacks the attack object calls. The first writes the position at 0x18 of the
     * parameters relative to the owner's character info; the second counts a hit and passes the
     * attack at 0x8 of the parameters on to the attack object (behaviour inferred). */
    void fn_80067B7C(AttackCallbackParam &param);
    void fn_80067B0C(AttackCallbackParam &param);

    /* Records the stick that holds this implementation (name inferred). */
    void SetStick(WeaponStickBase *stick) {
        m_stick = stick;
    }

    /* The number of hits counted since the last update (name inferred). */
    int GetHitCount() const {
        return m_hitCount;
    }

    /* Whether the attack object exists (name inferred). */
    bool IsAttacking() const {
        return m_flags.Test(0);
    }

    /* The member names from m_attack to m_scale are inferred. */
    /* 0x00 */ boost::shared_ptr<UnkAttack_8005A840> m_attack;
    /* 0x08 */ WeaponStickBase *m_stick;       /* the stick that holds this implementation */
    /* 0x0C */ UnkSegment_80020A20 m_lines[2]; /* the current line, then the previous one */
    /* 0x3C */ UnkBitFlags_80014E04 m_flags;   /* bit 0 while the attack object exists */
    /* 0x40 */ int m_hitCount;
    /* 0x44 */ float m_scale;    /* the model stretch: 1 at rest, 1.5 while attacking */
    /* 0x48 */ float field_0x48; /* the constructor's arg1, passed on to the attack object */
    /* 0x4C */ int field_0x4C;   /* passed to fn_800C5D74: 3 or 6 by the owner's kind */
    /* 0x50 */ int field_0x50;   /* passed to the attack object: 3 or 4 by the owner's kind */
};
CHECK_SIZEOF(Weapon::WeaponStickBaseImpl, 0x54);

/* The distance between two tested segments of a sweep (name inferred). The original initialises
 * the word at start-up with a 12-byte function in .ctors that stores the pooled literal; a float
 * initialised with a plain literal is a static initialiser instead (measured). The initialiser
 * is written as an inline call, one form that produces that start-up store; what the original
 * wrote there is not known. */
static inline float DefaultSweepStep() {
    return 5.0f;
}

static float s_sweepStep = DefaultSweepStep();

WeaponStickBaseImpl::WeaponStickBaseImpl(const boost::weak_ptr<CharInfo> &owner, float arg1) {
    field_0x48 = arg1;

    boost::shared_ptr<CharInfo> charInfo(owner.lock());

    if (charInfo.get() != 0 && charInfo->field_0x5C == 1) {
        field_0x4C = 3;
        field_0x50 = 3;
    } else {
        field_0x4C = 6;
        field_0x50 = 4;
    }

    fn_8040CE60(&m_lines[0].start);
    fn_8040CE60(&m_lines[0].end);
    fn_8040CE60(&m_lines[1].start);
    fn_8040CE60(&m_lines[1].end);
    m_scale = 1.0f;
    m_flags.Clear();
}

void WeaponStickBaseImpl::Update(float dt) {
    bool attacked = false;

    m_hitCount = 0;

    if (IsAttacking() && m_attack.get() != 0) {
        SweepAttack();
        attacked = true;
    }

    boost::shared_ptr<CharInfo> charInfo(m_stick->GetOwner().lock());

    if (charInfo.get() != 0 && charInfo->field_0x5C == 1) {
        if (attacked) {
            if (m_scale < 1.5f) {
                m_scale += 0.5f * dt / 0.1f;
                if (m_scale > 1.5f) {
                    m_scale = 1.5f;
                }
            }
        } else {
            if (m_scale > 1.0f) {
                m_scale += -0.5f * dt / 0.1f;
                if (m_scale < 1.0f) {
                    m_scale = 1.0f;
                }
            }
        }

        if (m_stick->field_0x1C != 0) {
            Fn8040Vector3 scale(1.0f, m_scale, 1.0f);

            fn_800631FC(m_stick->field_0x1C, 0, &scale);
        }
    }
}

void WeaponStickBaseImpl::StartAttack() {
    if (!IsAttacking()) {
        CreateAttack();
    }
}

void WeaponStickBaseImpl::EndAttack() {
    if (IsAttacking()) {
        ReleaseAttack();
    }
}

void WeaponStickBaseImpl::SetLine(const Fn8040Vector3 &start, const Fn8040Vector3 &end) {
    m_lines[1] = m_lines[0];
    fn_800091D0(&m_lines[0].start, &start);
    fn_800091D0(&m_lines[0].end, &end);
}

void WeaponStickBaseImpl::SweepAttack() {
    Fn8040Vector3 startMove;
    Fn8040Vector3 endMove;
    Fn8040Vector3 start;
    Fn8040Vector3 end;
    /* The differences are built in temporaries and copied, as the original does; it places them
     * after the named locals, so they are declared after them here. */
    Fn8040Vector3 startDelta;
    Fn8040Vector3 endDelta;

    fn_8040CD74(&startDelta, &m_lines[0].start, &m_lines[1].start);
    fn_800091EC(&startMove, &startDelta);
    fn_8040CD74(&endDelta, &m_lines[0].end, &m_lines[1].end);
    fn_800091EC(&endMove, &endDelta);

    /* The whole movements are scaled in place to one step's movement once the count is known. */
    int count = (int)(fn_8040CF50(&endMove) / s_sweepStep) + 1;
    float fraction = 1.0f / (float)count;

    fn_8040CE38(&startMove, fraction);
    fn_8040CE38(&endMove, fraction);

    float length = fn_8040D1B4(&m_lines[0].end, &m_lines[0].start);
    /* Each segment's length, from the previous line's length towards the current one's. */
    float segmentLength = fn_8040D1B4(&m_lines[1].end, &m_lines[1].start);
    float lengthStep = fraction * (length - segmentLength);

    fn_800091EC(&start, &m_lines[1].start);
    fn_800091EC(&end, &m_lines[1].end);

    for (int i = 0; i < count; i++) {
        UnkSegment_80020A20 segment;
        /* The end point's temporaries, in the order the original's frame holds them (it built the
         * point from nested vector operations). */
        Fn8040Vector3 segmentEnd;
        Fn8040Vector3 offset;
        Fn8040Vector3 unit;
        Fn8040Vector3 direction;

        fn_800091D0(&segment.start, &start);
        fn_8040CD74(&direction, &end, &start);
        fn_8040CEE8(&unit, &direction);
        fn_8040CDA8(&offset, &unit, segmentLength);
        fn_8040CDD0(&segmentEnd, &offset, &start);
        fn_800091D0(&segment.end, &segmentEnd);

        UnkProbeList_80021FE4 *hits =
            fn_800C5D74(fn_80021B08(), &segment, field_0x4C, 1, 0, 0, 0, m_attack);

        /* The list is deleted without being read. */
        if (hits != 0) {
            delete hits;
        }

        fn_8040CFF8(&start, &startMove);
        fn_8040CFF8(&end, &endMove);
        segmentLength += lengthStep;
    }
}

void WeaponStickBaseImpl::CreateAttack() {
    m_attack = boost::shared_ptr<UnkAttack_8005A840>(new UnkAttack_8005A840());

    if (m_attack.get() != 0) {
        m_attack->fn_80054DD0(
            field_0x50, 0x80,
            AttackCallback(boost::bind(&WeaponStickBaseImpl::fn_80067B7C, this, _1)),
            AttackCallback(boost::bind(&WeaponStickBaseImpl::fn_80067B0C, this, _1)), field_0x48);
        m_attack->m_owner = m_stick->GetOwner();
        m_flags.Set(0);
    }
}

void WeaponStickBaseImpl::ReleaseAttack() {
    if (m_attack.get() != 0) {
        m_attack.reset();
        m_flags.Reset(0);
    }
}

void WeaponStickBaseImpl::fn_80067B7C(AttackCallbackParam &param) {
    boost::shared_ptr<CharInfo> charInfo(param.field_0x0.lock()->m_owner.lock());

    if (charInfo.get() != 0) {
        Fn8040Vector3 offset;

        fn_8040CD74(&offset, &param.field_0x18, &charInfo->m_position);
        fn_800091D0(&param.field_0x34, &offset);
    }
}

void WeaponStickBaseImpl::fn_80067B0C(AttackCallbackParam &param) {
    if (param.field_0x54 & 1) {
        if (m_attack.get() != 0) {
            m_attack->fn_800532F8(param.field_0x8);
        }
        m_hitCount++;
    }
}

WeaponStickBase::WeaponStickBase(const boost::weak_ptr<CharInfo> &owner, int arg1, float arg2)
    : WeaponBase(owner, arg1, 5), m_impl(new WeaponStickBaseImpl(owner, arg2)) {
    m_impl->SetStick(this);
}

WeaponStickBase::~WeaponStickBase() {}

void WeaponStickBase::Update(float dt) {
    m_impl->Update(dt);
}

void WeaponStickBase::StartAttack() {
    m_impl->StartAttack();
}

void WeaponStickBase::EndAttack() {
    m_impl->EndAttack();
}

bool WeaponStickBase::IsAttacking() const {
    return m_impl->IsAttacking();
}

float WeaponStickBase::GetScale() const {
    return m_impl->m_scale;
}

void WeaponStickBase::PlaySound(int index) {
    fn_801D54F4(fn_80053164(), index + 0x2005, &m_position, 0, 1);
}

int WeaponStickBase::GetHitCount() const {
    return m_impl->GetHitCount();
}

void WeaponStickBase::SetLine(const Fn8040Vector3 &start, const Fn8040Vector3 &end) {
    m_impl->SetLine(start, end);
}

} // namespace Weapon
