#ifndef GAME_DIABLONPARTSBOMB_H
#define GAME_DIABLONPARTSBOMB_H

#include "Game/CharaColliAttack.h"
#include "Game/CharaColliBase.h"
#include "Game/Diablon.h"
#include "Game/DiablonPartsBase.h"
#include "Game/EffectElement.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

namespace Boss {
namespace Diablon {
namespace Parts {

/* Boss::Diablon::Parts::Bomb (class name from RTTI, with PartsBase as its only base): a part that
 * fn_8031DB34 creates at the translation of the matrix it is given. When its first update finds it
 * still in STATE_START (the constructor's state), it starts an effect with m_matrix as its matrix
 * source (kept in m_effect), a sound that follows the diablon's position, and a second effect that
 * it does not keep; an action of kind 9 or 10 that arrives before that update changes the state
 * (other kinds are ignored), and the update then takes the explode or stop case instead. An action
 * of kind 9 received before STATE_EXPLODE makes the next update explode the bomb: it creates a
 * probe sphere with an attack object, and the following updates grow the radius towards the
 * diablon's parameter. An action of kind 10 received before STATE_STOP makes the next update stop
 * the effect and the sound, and the update after that stores 1 in field_0x20 (whether or not the
 * bomb exploded). Behaviour inferred; method and member names are inferred unless they are
 * placeholders, and so are the parameters' pointer, reference and const spellings. */
class Bomb : public PartsBase {
  public:
    /* The values of m_state (names inferred); no other value is stored. */
    enum State {
        STATE_START = 0,
        STATE_WAITING = 1,
        STATE_EXPLODE = 100,
        STATE_GROWING = 101,
        STATE_STOP = 200,
        STATE_FINISHED = 201
    };

    /* Builds the PartsBase under parent with the name "Bomb", sets m_position to the translation
     * of *matrix (fn_8040FB08), m_radius to s_startRadius and enters STATE_START. */
    Bomb(PartsBase *parent, const boost::shared_ptr<Diablon> &diablon, Fn8040Matrix34 *matrix);
    /* Calls StopEffect before the members are destroyed. */
    virtual ~Bomb();
    /* First the state:
     * - STATE_START: StartEffect, then fn_8031E3B0, then enters STATE_WAITING.
     * - STATE_EXPLODE: Explode, then enters STATE_GROWING.
     * - STATE_GROWING: with the diablon's m_params, adds dt * (field_0x70 - s_startRadius) /
     *   field_0x74 to m_radius, then replaces m_radius by field_0x70 unless m_radius is less
     *   (fn_800704AC; a NaN radius is replaced too). The state does not change.
     * - STATE_STOP: StopEffect, then enters STATE_FINISHED.
     * - STATE_FINISHED: stores 1 in field_0x20 and returns, in every update in this state.
     * Then, in every state but STATE_FINISHED, UpdateMatrix and UpdateProbe. */
    virtual void Update(float dt);
    /* An action of kind 9 enters STATE_EXPLODE when m_state is less than STATE_EXPLODE; one of
     * kind 10 enters STATE_STOP when m_state is less than STATE_STOP. Other kinds are ignored. The
     * next Update acts on the new state. */
    virtual void OnAction(const Action &action);

    /* The attack object's second callback (see Explode); it does nothing. */
    void fn_8031DB8C(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The attack object's first callback (see Explode): stores the normalised direction from the
     * diablon's position to param's field_0x18 in field_0x34 (the zero vector when that
     * difference is too short to normalise), and 1 in field_0x4C and field_0x50. */
    void fn_8031DB90(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* When the probe pointer is not null, moves the probe to m_position and passes m_radius to its
     * SetSize (with -2, 0 and 0). */
    void UpdateProbe();
    /* Creates a probe sphere at the zero vector with a radius of 0 for the diablon (fn_800C7EA8
     * with 0x40000, 6 and 0) and stores it in m_probe; UpdateProbe moves and sizes it later. When
     * the probe pointer is not null, sets the probe's byte at 0x2C, asks fn_800C4374 for its attack
     * object and keeps it in m_attack; when that locks, passes 4, 0x100, the callbacks fn_8031DB90
     * and fn_8031DB8C and 1 to its fn_80054DD0 and stores the diablon in its m_owner. Then, in
     * either case, stops m_sound, requests sound 0x7E34 at the diablon's position (fn_801D54F4
     * plays nothing when its sound setup returns null) and asks fn_8034DE08 for an object of kind
     * 7 there with -2, discarding the result. */
    void Explode();
    /* Asks the diablon's Effect for effect 0x32 (fn_802DC95C) and, when it locks, passes a matrix
     * with the identity rotation and the translation (m_position.x, 0, m_position.z) to
     * fn_80425E68; the effect is not kept. */
    void fn_8031E3B0();
    /* Sets m_matrix to the identity rotation with the translation m_position, then scales its
     * rotation columns by m_radius / s_baseRadius. */
    void UpdateMatrix();
    /* When m_effect has not expired, requests the effect to stop through the locked pointer (which
     * is not tested for null) and resets m_effect; then, in either case, stops m_sound. */
    void StopEffect();
    /* Asks the diablon's Effect for effect 0 (fn_802DC998, which keeps m_matrix as the effect's
     * matrix source when the effect locks) and keeps it in m_effect, then starts sound 0x7E33 that
     * follows the diablon's position, with this part as its owner, and keeps its handle in
     * m_sound. */
    void StartEffect();

    /* 0x24 */ boost::weak_ptr< ::Effect::System::EffectElement> m_effect;
    /* 0x2C */ boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> m_probe;
    /* 0x34 */ boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliAttack> m_attack;
    /* 0x3C */ UnkSoundHandle_801D3DC8 m_sound;
    /* 0x58 */ Fn8040Matrix34 m_matrix;
    /* 0x88 */ Fn8040Vector3 m_position;
    /* 0x94 */ float m_radius;
    /* 0x98 */ s32 m_state; /* a State */
};
CHECK_SIZEOF(Boss::Diablon::Parts::Bomb, 0x9C);

} // namespace Parts
} // namespace Diablon
} // namespace Boss

extern "C" {
/* Creates a Boss::Diablon::Parts::Bomb under parent at the translation of *matrix and returns it
 * (null when the allocation fails). The body part's OnAction (0x802D7224) calls it for one action
 * kind, passing itself, its m_diablon and the matrix that its fn_802DA7B8 returns for 8, and
 * discards the result. The return type and the pointer spellings are inferred. */
Boss::Diablon::Parts::PartsBase *
fn_8031DB34(Boss::Diablon::Parts::PartsBase *parent,
            const boost::shared_ptr<Boss::Diablon::Diablon> &diablon, Fn8040Matrix34 *matrix);
}

#endif
