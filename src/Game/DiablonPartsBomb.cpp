/* The Boss::Diablon::Parts::Bomb part (behaviour in the class comment). File name inferred from the
 * class. Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first).
 *
 * The two callbacks of the attack object are boost::function objects built with
 * boost::bind(&Bomb::..., this, _1); their manager, invoker and copy functions come from the boost
 * headers. */
#include "Game/DiablonPartsBomb.h"
#include "Game/CharInfo.h"
#include "Game/DiablonEffect.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The radius at which UpdateMatrix leaves the matrix unscaled, and the radius that the bomb starts
 * with and grows from (names inferred from their uses; both are 60).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original type of these
 * variables. The original sets them with a start-up function in .ctors (0x8031E950), and the
 * shared stand-in reproduces that function; plain float initialisers would place the values in
 * initialised data instead. Revisit when the original wrapper type is found. */
static UnkStartupFloat_800E2570 s_baseRadius(60.0f);
static UnkStartupFloat_800E2570 s_startRadius(60.0f);

namespace Boss {
namespace Diablon {
namespace Parts {

Bomb::Bomb(PartsBase *parent, const boost::shared_ptr<Diablon> &diablon, Fn8040Matrix34 *matrix)
    : PartsBase(parent, "Bomb", diablon) {
    m_position = fn_8040FB08(matrix);
    m_radius = s_startRadius.value;
    m_state = STATE_START;
}

Bomb::~Bomb() {
    StopEffect();
}

void Bomb::Update(float dt) {
    switch (m_state) {
    case STATE_START:
        StartEffect();
        fn_8031E3B0();
        m_state++;
        break;
    case STATE_EXPLODE:
        Explode();
        m_state++;
        break;
    case STATE_GROWING: {
        const UnkDiablonParams_80558710 *params = m_diablon->m_params;

        m_radius += dt * (params->field_0x70 - s_startRadius.value) / params->field_0x74;
        m_radius = fn_800704AC(m_radius, params->field_0x70);
        break;
    }
    case STATE_STOP:
        StopEffect();
        m_state++;
        break;
    case STATE_FINISHED:
        field_0x20 = 1;
        return;
    }

    UpdateMatrix();
    UpdateProbe();
}

void Bomb::OnAction(const Action &action) {
    switch (action.m_type) {
    case 9:
        if (m_state < STATE_EXPLODE) {
            m_state = STATE_EXPLODE;
        }
        break;
    case 10:
        if (m_state < STATE_STOP) {
            m_state = STATE_STOP;
        }
        break;
    }
}

void Bomb::StartEffect() {
    m_effect = m_diablon->m_effect->fn_802DC998(&m_matrix, 0);
    m_sound = fn_801D3DC8(0x7E33, &m_diablon->m_position, this);
}

void Bomb::StopEffect() {
    if (!m_effect.expired()) {
        m_effect.lock()->RequestStop();
        m_effect.reset();
    }

    m_sound.fn_801D455C();
}

void Bomb::UpdateMatrix() {
    float scale = m_radius / s_baseRadius.value;
    m_matrix = fn_8040B034(m_position);
    fn_8040B720(&m_matrix, scale, scale, scale);
}

void Bomb::fn_8031E3B0() {
    boost::weak_ptr< ::Effect::System::EffectElement> weakEffect;
    boost::shared_ptr< ::Effect::System::EffectElement> effect;

    weakEffect = m_diablon->m_effect->fn_802DC95C(0x32);
    effect = weakEffect.lock();

    if (effect.get() != 0) {
        Fn8040Matrix34 matrix(fn_8040B034(Fn8040Vector3(m_position.x, 0.0f, m_position.z)));
        fn_80425E68(effect.get(), matrix);
    }
}

void Bomb::Explode() {
    UnkWorld_80021B08 *world = fn_80021B08();

    m_probe = fn_800C7EA8(world, &lbl_805E07A0, 0x40000, 6, 0, m_diablon, &lbl_805E07A0, 0.0f);

    if (m_probe.get() != 0) {
        m_probe->SetField0x2C(1);
        m_attack = fn_800C4374(world, m_probe);

        boost::shared_ptr<CharaColliAttack> attack(m_attack.lock());

        if (attack.get() != 0) {
            attack->fn_80054DD0(4, 0x100, AttackCallback(boost::bind(&Bomb::fn_8031DB90, this, _1)),
                                AttackCallback(boost::bind(&Bomb::fn_8031DB8C, this, _1)), 1.0f);
            attack->m_owner = m_diablon;
        }
    }

    m_sound.fn_801D455C();
    fn_801D54F4(fn_80053164(), 0x7E34, &m_diablon->m_position, 0, 1);
    fn_8034DE08(7, &m_diablon->m_position, -2);
}

void Bomb::UpdateProbe() {
    if (m_probe.get() != 0) {
        m_probe->SetPosition(&m_position);
        m_probe->SetSize(m_radius, -2, 0.0f, 0.0f);
    }
}

void Bomb::fn_8031DB90(AttackCallbackParam &param) {
    param.field_0x34 = fn_8040CEE8(param.field_0x18 - m_diablon->m_position);
    param.field_0x4C = 1.0f;
    param.field_0x50 = 1.0f;
}

void Bomb::fn_8031DB8C(AttackCallbackParam &param) {}

} // namespace Parts
} // namespace Diablon
} // namespace Boss

extern "C" Boss::Diablon::Parts::PartsBase *
fn_8031DB34(Boss::Diablon::Parts::PartsBase *parent,
            const boost::shared_ptr<Boss::Diablon::Diablon> &diablon, Fn8040Matrix34 *matrix) {
    return new Boss::Diablon::Parts::Bomb(parent, diablon, matrix);
}
