#ifndef GAME_PARACHUTE_H
#define GAME_PARACHUTE_H

#include "Game/CharInfo.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Task.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharCommand;

namespace Parachute {

/* Parachute::Parachute (class name from RTTI, with Task as its only base): the task that carries
 * the rider of the ROCKET gadget's rocket down after the flight. It has a character info of its
 * own ("Parachute") and uses the entries with the id 2 of ROCKET.BIN. Behaviour inferred from
 * the code:
 * - Every update in which FLAG_RELEASED is clear sends the rider, when it still exists, a
 *   Player::TouchHangCommand with the character info's position and rotation.
 * - STATE_OPENING waits for the model's fn_80160040(2) (the end of a motion, see
 *   GadgetModelInfo.h) and then sets FLAG_MOVING, which the constructor has already set.
 * - While FLAG_MOVING is set, every update moves the character info (see UpdateMotion). A hit
 *   within s_groundDistance below it, or the command 12, sets FLAG_RELEASED and starts
 *   STATE_RELEASE, which sends the rider a Player::CancelBehaviorCommand and is followed by
 *   STATE_FINISHED, where the task finishes (Task flag 0x01).
 * - Every command it sends is allocated with CharCommand's operator new and passed to
 *   fn_801DC980 even when the allocation returns null (USA (GUPE8P) 0x80292EE0, 0x80292FC0,
 *   0x802935F8).
 * The constructor does not store a task name. Method, member and constant names are inferred, and
 * so are the parameters' pointer, reference and const spellings. */
class Parachute : public Task {
  public:
    /* The values of m_state (names inferred from what each state does). */
    enum State { STATE_OPENING = 0, STATE_DESCENDING = 1, STATE_RELEASE = 2, STATE_FINISHED = 3 };

    /* The bits of m_stateFlags (names inferred from where they are set and tested). */
    enum Flag {
        /* Set by the command 12 and by a hit below the parachute; while it is clear, every update
         * sends the rider a TouchHangCommand. Nothing clears it. */
        FLAG_RELEASED = 0,
        /* Set by the constructor and at the end of STATE_OPENING, cleared by a hit below the
         * parachute; while it is set, every update calls UpdateMotion. */
        FLAG_MOVING = 1
    };

    /* angles is not const: see the note at the constructor's definition. Stores the rider and
     * STATE_OPENING; builds the character info with its command unit and handler (OnCommand) at
     * position with the rotation angles; makes the position the anchor and the previous position;
     * loads the model and starts its entries with the id 2; sends the rider, when it still
     * exists, the first TouchHangCommand; places the model; sets FLAG_MOVING and bit 1 of
     * m_sweepFlags and plays sound 0x100C. */
    Parachute(Task *parent, const boost::weak_ptr<CharInfo> &rider, const Fn8040Vector3 *position,
              UnkRotation3 *angles);
    virtual ~Parachute();
    /* Moves the parachute while FLAG_MOVING is set, places and advances the model, sends the
     * rider the TouchHangCommand while FLAG_RELEASED is clear and runs the state (see the
     * class). STATE_OPENING calls the model without testing its pointer. */
    virtual void Update(float dt);

    /* When the model pointer is non-null, gives the model the character info's matrix
     * (fn_8015CEA0). */
    void UpdateModelMatrix();
    /* Advances the character info's position and velocity (field_0x18) under its acceleration
     * (field_0x24) with fn_804103E8 and lowers the position by 66 * dt. Sweeps the movement since
     * the previous position (fn_801BFC28, radius 10): on a hit, takes the position that the call
     * leaves and the movement that it leaves, divided by dt, as the velocity, with its y zeroed.
     * Stores the previous position and zeroes the acceleration. Probes the segment from the
     * position to s_groundDistance below it, ignoring what fn_802937E4 rejects: a hit with a
     * non-null hit record sets FLAG_RELEASED, clears FLAG_MOVING and starts STATE_RELEASE. Then
     * keeps the position within s_driftRadius of the anchor in the x-z plane and plays sound
     * 0x100D at no position with this task as the owner key. */
    void UpdateMotion(float dt);
    /* The character info's command handler. Command 12 sets FLAG_RELEASED and starts
     * STATE_RELEASE. Command 1 (an AccelerateInputCommand) makes its vector times 80, with y
     * zeroed, the character info's acceleration. Other commands are ignored. */
    void OnCommand(CharCommand &command);

    /* How far from the anchor UpdateMotion lets the parachute drift in the x-z plane (200). The
     * ROCKET gadget reads it as the radius of the debug cylinder it draws at the end of the
     * flight. The class scope is inferred: the constant is defined in this file, ahead of its
     * other constants in .sdata2, and read from another file. */
    static const float s_driftRadius;

    /* 0x28 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x30 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x38 */ boost::weak_ptr<CharInfo> m_rider;
    /* 0x40 */ s32 m_state;            /* a State */
    /* 0x44 */ Fn8040Vector3 m_anchor; /* the position at construction */
    /* The position after UpdateMotion's sweep, before its drift clamp (copied at USA (GUPE8P)
     * 0x80292C34; the clamp stores follow at 0x80292D54 and 0x80292D68). */
    /* 0x50 */ Fn8040Vector3 m_previousPosition;
    /* 0x5C */ UnkBitFlags_80014E04 m_stateFlags; /* Flag bits */
    /* 0x60 */ UnkBitFlags_80014E04 m_sweepFlags; /* passed to fn_801BFC28 */
    /* The creator allocates 0x68 bytes (USA (GUPE8P) 0x80292A90). In this source view the four
     * bytes after m_sweepFlags are alignment padding (the class is 8-byte aligned); whether the
     * original declares anything there is not known. */
};
CHECK_SIZEOF(Parachute, 0x68);

} // namespace Parachute

/* Allocates a Parachute::Parachute task under task layer 11 and, when the allocation succeeds,
 * passes the three arguments to its constructor. A failed allocation skips the constructor (USA
 * (GUPE8P) 0x80292A98-0x80292AA0). The ROCKET gadget's rocket passes its rider and its hang
 * position and angles when its flight ends. The parameter spellings are inferred (angles is not
 * const because the constructor's is not), and so is the void return type: the function returns
 * with the allocation or constructor result in r3, and its only caller (0x801161BC) does not use
 * r3 after the call. */
extern "C" void fn_80292A64(const boost::weak_ptr<CharInfo> &rider, const Fn8040Vector3 *position,
                            UnkRotation3 *angles);

#endif
