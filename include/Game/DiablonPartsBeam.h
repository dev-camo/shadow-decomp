#ifndef GAME_DIABLONPARTSBEAM_H
#define GAME_DIABLONPARTSBEAM_H

#include "Game/CharaColliAttack.h"
#include "Game/Diablon.h"
#include "Game/DiablonPartsBase.h"
#include "Game/EffectElement.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace Boss {
namespace Diablon {
namespace Parts {

/* Class name and base from RTTI. Method, member and state names are inferred, as are
 * pointer/reference and const spellings. The factory allocates 0xB8 bytes. */
class Beam : public PartsBase {
  public:
    enum State {
        STATE_START = 0,
        STATE_WAITING = 1,
        STATE_AIM = 100,
        STATE_FIRE = 101,
        STATE_SWEEP = 102,
        STATE_STOP = 200,
        STATE_FINISHED = 201
    };

    Beam(PartsBase *parent, const boost::shared_ptr<Diablon> &diablon, Fn8040Matrix34 *matrix);
    virtual ~Beam();
    virtual void Update(float dt);
    virtual void OnAction(const Action &action);

    void StartBeam();
    void UpdateMatrix();
    void UpdateTargetPosition();
    void Aim();
    bool Probe(Fn8040Vector3 &position, Fn8040Vector3 &hitVector);
    void CreateAttack();
    void ApplyAttack();
    void fn_8031C83C(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    void fn_8031C838(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    void StartChargeEffect();
    void StopChargeEffect();
    /* The second vector is supplied by Probe but is unused by this function. */
    void UpdateHitEffect(const Fn8040Vector3 &position, const Fn8040Vector3 &hitVector);
    void StopHitEffect();

    /* 0x24 */ float m_length;
    /* 0x28 */ Fn8040Vector3 m_direction;
    /* 0x34 */ Fn8040Vector3 m_position;
    /* 0x40 */ Fn8040Vector3 m_axis;
    /* 0x4C */ Fn8040Vector3 m_targetPosition;
    /* 0x58 */ boost::weak_ptr< ::Effect::System::EffectElement> m_beamEffect;
    /* 0x60 */ boost::weak_ptr< ::Effect::System::EffectElement> m_chargeEffect;
    /* 0x68 */ boost::weak_ptr< ::Effect::System::EffectElement> m_hitEffect;
    /* 0x70 */ boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliAttack> m_attack;
    /* 0x78 */ Fn8040Matrix34 *m_sourceMatrix;
    /* 0x7C */ Fn8040Matrix34 m_matrix;
    /* 0xAC */ float m_elapsed;
    /* 0xB0 */ float m_aimDelay;
    /* 0xB4 */ s32 m_state;
};
CHECK_SIZEOF(Boss::Diablon::Parts::Beam, 0xB8);

} // namespace Parts
} // namespace Diablon
} // namespace Boss

extern "C" {
/* Creates the Beam part beneath parent; fn_802D7224 passes its matrix for part 5.
 * The return type is inferred. Allocation failure returns null. */
Boss::Diablon::Parts::PartsBase *
fn_8031C5C4(Boss::Diablon::Parts::PartsBase *parent,
            const boost::shared_ptr<Boss::Diablon::Diablon> &diablon, Fn8040Matrix34 *matrix);
}

#endif
