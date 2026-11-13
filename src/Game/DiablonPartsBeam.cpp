/* File name inferred from the RTTI name Boss::Diablon::Parts::Beam. */
#include "Game/DiablonPartsBeam.h"
#include "Game/AngleConversion.h"
#include "Game/DiablonEffect.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWorld_80021B08.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include <cmath>

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;
using SonicteamUSA::System::Collision::CollisionResultList;

/* Matching workaround: the shared startup-float stand-in preserves the two stores in
 * the original .ctors function (USA (GUPE8P) 0x8031DB20). Plain floats would be initialised data.
 * Revisit when the original initialisation form is recovered. Names follow the uses below. */
static UnkStartupFloat_800E2570 s_baseLength(100.0f);
static UnkStartupFloat_800E2570 s_maxLength(3000.0f);

namespace Boss {
namespace Diablon {
namespace Parts {

Beam::Beam(PartsBase *parent, const boost::shared_ptr<Diablon> &diablon, Fn8040Matrix34 *matrix)
    : PartsBase(parent, "Beam", diablon), m_sourceMatrix(matrix) {
    m_targetPosition = fn_8040FB08(m_sourceMatrix);
    m_length = 0.0f;
    m_elapsed = 0.0f;
    m_aimDelay = m_diablon->m_params->field_0x4C;
    m_state = STATE_START;
}

Beam::~Beam() {
    if (!m_beamEffect.expired()) {
        m_beamEffect.lock()->ClearFlags(::Effect::System::ELEMENT_ACTIVE);
    }
    if (!m_chargeEffect.expired()) {
        m_chargeEffect.lock()->ClearFlags(::Effect::System::ELEMENT_ACTIVE);
    }
    if (!m_hitEffect.expired()) {
        m_hitEffect.lock()->ClearFlags(::Effect::System::ELEMENT_ACTIVE);
    }
}

void Beam::Update(float dt) {
    m_position = fn_8040FB08(m_sourceMatrix);
    if (m_aimDelay > 0.0f) {
        m_aimDelay -= dt;
        if (m_aimDelay <= 0.0f) {
            UpdateTargetPosition();
        }
    }

    switch (m_state) {
    case STATE_START:
        StartChargeEffect();
        m_state++;
        break;
    case STATE_WAITING:
        break;
    case STATE_AIM:
        if (m_aimDelay > 0.0f) {
            UpdateTargetPosition();
            m_aimDelay = 0.0f;
        }
        Aim();
        m_state++;
        break;
    case STATE_FIRE:
        StartBeam();
        m_state++;
        /* Fall through: the first sweep happens in the firing update. */
    case STATE_SWEEP: {
        m_length += s_maxLength.value * dt / 0.35f;
        m_length = fn_800704AC(m_length, s_maxLength.value);

        const UnkDiablonParams_80558710 *params = m_diablon->m_params;
        float angle = DegreesToRadians(dt * params->field_0x54 / params->field_0x58);
        Fn8040Matrix34 rotation = fn_8040FD8C(&m_axis, angle);
        m_direction = rotation * m_direction;

        Fn8040Vector3 position;
        Fn8040Vector3 hitVector;
        if (Probe(position, hitVector)) {
            UpdateHitEffect(position, hitVector);
        } else {
            StopHitEffect();
        }

        m_elapsed += dt;
        if (m_elapsed >= m_diablon->m_params->field_0x58) {
            m_state = STATE_STOP;
        }
        break;
    }
    case STATE_STOP:
        StopChargeEffect();
        StopHitEffect();
        if (!m_beamEffect.expired()) {
            m_beamEffect.lock()->RequestStop();
            m_beamEffect.reset();
        }
        m_state++;
        break;
    case STATE_FINISHED:
        field_0x20 = 1;
        return;
    }

    UpdateMatrix();
    ApplyAttack();
}

void Beam::OnAction(const Action &action) {
    switch (action.m_type) {
    case 6:
        if (m_state < STATE_AIM) {
            m_state = STATE_AIM;
        }
        break;
    case 7:
        if (m_state < STATE_STOP) {
            m_state = STATE_STOP;
        }
        break;
    }
}

void Beam::StartBeam() {
    m_beamEffect = m_diablon->m_effect->fn_802DC998(&m_matrix, 0x14);
    CreateAttack();
    fn_801D54F4(fn_80053164(), 0x7E31, &m_diablon->m_position, 0, 1);
}

void Beam::UpdateMatrix() {
    m_matrix = fn_8040B034(m_position);

    UnkRotation3 angles = fn_8040F7F8(&m_direction);
    fn_8040B848(&m_matrix, angles.y);
    fn_8040B884(&m_matrix, angles.x);
    fn_8040B720(&m_matrix, 1.0f, 1.0f, m_length / s_baseLength.value);
}

void Beam::UpdateTargetPosition() {
    boost::shared_ptr<CharInfo> target(m_diablon->m_target.lock());
    if (target.get() != 0) {
        m_targetPosition = target->m_position;
    } else {
        m_targetPosition = *m_sourceMatrix * Fn8040Vector3(0.0f, 0.0f, 1.0f);
    }
}

void Beam::Aim() {
    Fn8040Vector3 offset(m_targetPosition - m_position);
    m_direction.x = 0.0f;
    m_direction.y = offset.y;
    offset.y = 0.0f;
    m_direction.z = -fn_8040CF50(offset);
    fn_8040CE74(&m_direction);

    float limit = DegreesToRadians(m_diablon->m_params->field_0x50);
    float cosine = std::cos(limit);
    if (-m_direction.z < cosine) {
        m_direction.z = -cosine;
        float sign = m_direction.y > 0.0f ? 1.0f : -1.0f;
        m_direction.y = sign * std::sin(limit);
        fn_8040CE74(&m_direction);
    }

    m_direction = fn_8040D0AC(m_direction, m_diablon->m_rotation.y);
    Fn8040Vector3 side(fn_8040CFB8(m_direction, Fn8040Vector3(0.0f, 1.0f, 0.0f)));
    m_axis = fn_8040CFB8(side, m_direction);
    fn_8040CE74(&m_axis);

    Fn8040Matrix34 rotation =
        fn_8040FD8C(&m_axis, fn_8000AB90(-m_diablon->m_params->field_0x54 / 2.0f));
    m_direction = rotation * m_direction;
}

bool Beam::Probe(Fn8040Vector3 &position, Fn8040Vector3 &hitVector) {
    UnkProbeHit_8001E9BC *hit;
    UnkSegment_80020A20 segment(m_position, m_position + m_direction * m_length);
    CollisionResultList *hits = fn_80021FE4(fn_80020D40(), &segment, 1, 6, 0, 0);
    if (hits != 0) {
        hit = hits->fn_8002735C()->hit;
        position = *fn_8001E9BC(hit);
        hitVector = *fn_800266D4(hit);
        float distance = fn_8040CF50(position - m_position);
        if (m_length > distance) {
            m_length = distance;
        }
        delete hits;
        return true;
    }
    return false;
}

void Beam::CreateAttack() {
    m_attack = boost::shared_ptr<CharaColliAttack>(new CharaColliAttack);
    if (m_attack.get() != 0) {
        m_attack->fn_80054DD0(4, 0x100, AttackCallback(boost::bind(&Beam::fn_8031C83C, this, _1)),
                              AttackCallback(boost::bind(&Beam::fn_8031C838, this, _1)), 1.0f);
        m_attack->m_owner = m_diablon;
    }
}

void Beam::ApplyAttack() {
    if (m_attack.get() != 0) {
        UnkSegment_80020A20 segment(m_position, m_position + m_direction * m_length);
        UnkWorld_80020D40 *probeWorld = fn_80020D40();
        UnkWorld_80021B08 *world = fn_80021B08();
        CollisionResultList *hits = fn_80021FE4(probeWorld, &segment, 3, 6, 0, 0);
        if (hits != 0) {
            UnkProbeIterator_80027374 iterator(hits->fn_8001EE04());
            while (iterator.node != hits->fn_8001EDEC()) {
                UnkProbeNode_80021FE4 *node = iterator.node;
                fn_800C40C4(world, m_attack, node, 0);
                fn_80027374(&iterator, 0);
            }
            delete hits;
        }
    }
}

void Beam::fn_8031C83C(AttackCallbackParam &param) {
    param.field_0x34 = m_direction;
    param.field_0x4C = 1.0f;
    param.field_0x50 = 1.0f;
}

void Beam::fn_8031C838(AttackCallbackParam &param) {}

void Beam::StartChargeEffect() {
    m_chargeEffect = m_diablon->m_effect->fn_802DC998(m_sourceMatrix, 2);
    fn_801D54F4(fn_80053164(), 0x7E30, &m_diablon->m_position, 0, 1);
}

void Beam::StopChargeEffect() {
    if (!m_chargeEffect.expired()) {
        m_chargeEffect.lock()->RequestStop();
        m_chargeEffect.reset();
    }
}

void Beam::UpdateHitEffect(const Fn8040Vector3 &position, const Fn8040Vector3 &hitVector) {
    if (m_hitEffect.expired()) {
        m_hitEffect = m_diablon->m_effect->fn_802DC95C(0x17);
    }
    if (!m_hitEffect.expired()) {
        boost::shared_ptr< ::Effect::System::EffectElement> effect(m_hitEffect.lock());
        fn_80425E4C(effect.get(), &position);
    }
}

void Beam::StopHitEffect() {
    if (!m_hitEffect.expired()) {
        m_hitEffect.lock()->RequestStop();
        m_hitEffect.reset();
    }
}

} // namespace Parts
} // namespace Diablon
} // namespace Boss

extern "C" Boss::Diablon::Parts::PartsBase *
fn_8031C5C4(Boss::Diablon::Parts::PartsBase *parent,
            const boost::shared_ptr<Boss::Diablon::Diablon> &diablon, Fn8040Matrix34 *matrix) {
    return new Boss::Diablon::Parts::Beam(parent, diablon, matrix);
}
