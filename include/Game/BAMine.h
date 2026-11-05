#ifndef GAME_BAMINE_H
#define GAME_BAMINE_H

#include "Game/CharCommand.h"
#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace BAMine {

/* The gadget's parameters, in the order of the descriptor's table ("X", "Y", "Z", "second"). */
struct BAMineParams {
    BAMineParams() : x(0.0f), y(0.0f), z(0.0f), second(0.0f) {}

    /* x, y, z: the offset from the owner's position to the far end of the movement (inferred). */
    /* 0x0 */ float x;
    /* 0x4 */ float y;
    /* 0x8 */ float z;
    /* 0xC */ float second; /* the nominal time of one movement there and back (inferred) */
};
CHECK_SIZEOF(BAMineParams, 0x10);

/* BAMine::BAMine (class name from RTTI; the gadget's descriptor is "BA Mine"): the BAMINE.BIN
 * model, moving from the owner's position towards that position plus the offset (X, Y, Z) and back
 * along a cosine curve, one movement in a nominal "second" seconds: the update whose timer passes
 * "second" restarts it at 0 and discards the excess, so large time steps skip parts of the
 * movement (with "second" 1 and steps of 2 it stays at the start). With "second" not positive it
 * does not move and stays at the owner's position. Sound 0x7933 can play at two points of a
 * movement, but either can be skipped (see UpdateMove). The callbacks OnHit and OnAttack, which
 * the constructor installs when the model has an attack object, can make it explode at the next
 * update once it is running (state 1): it creates a CommonBomb at its position, plays sound 0x7934
 * and finishes. Command 0x14 in state 1 hands its model to fn_8033F0F8 and finishes it without an
 * explosion; in any other state the command only gets marked (see OnCommand; behaviour inferred).
 * Method, member and constant names are inferred. */
class BAMine : public Gadget {
  public:
    /* m_state values (names inferred from what each state does). */
    enum State { STATE_START, STATE_RUN, STATE_FINISH, STATE_EXPLODE };

    /* m_moveFlags bits (names inferred). */
    enum MoveFlag {
        MOVE_STILL,       /* the gadget does not move ("second" not positive) */
        MOVE_PLAY_SOUND,  /* play sound 0x7933 at this update */
        MOVE_SOUND_START, /* the sound was played when the timer passed 0.5 seconds */
        MOVE_SOUND_HALF   /* the sound was played when the timer passed half of "second" plus 0.5 */
    };

    BAMine(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~BAMine();
    /* Advances the model when the model pointer is non-null, then runs the state: 0 enters 1; 1
     * moves the gadget and places the model (UpdateMove, UpdateModelMatrix); 2 finishes the
     * gadget; 3 explodes and finishes it. */
    virtual void UpdateBody(float dt);
    /* Rereads the owner (ReadOwner), places the model (UpdateModelMatrix) and draws a debug line
     * from the character info's position to that position plus the offset. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and its rotation (converted to radians). Copies the owner's
     * parameters only when the owner's parameter size equals sizeof(BAMineParams), 0x10
     * (fn_800C9DBC); otherwise m_params keeps its previous values. Then enters state 0, keeps the
     * position as the start of the movement, sets the angle speed to 360 / "second" (0 when
     * "second" is 0), restarts the timer, and sets MOVE_STILL in m_moveFlags when "second" is not
     * positive and clears it otherwise. */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When the model pointer is non-null (OnCommand releases the model), places the model at the
     * character info's position, turned about the y axis by the character info's rotation. When
     * the target pointer is non-null, copies that position to the target object. */
    void UpdateModelMatrix();
    /* Unless MOVE_STILL is set ("second" not positive), advances the timer by dt and tests it in
     * this order, each test strictly greater than: above "second", it restarts the timer at 0,
     * discarding the excess, and clears the flags; otherwise, above half of "second" plus 0.5, it
     * plays sound 0x7933 unless MOVE_SOUND_HALF is set; otherwise, above 0.5, it plays the sound
     * unless MOVE_SOUND_START is set. Each sound plays at most once per movement and can be
     * skipped: because the restart test comes first, neither sound test can trigger when
     * 0 < "second" <= 0.5 and the half-way one cannot when 0 < "second" <= 1; a time step from at
     * most 0.5 to above half of "second" plus 0.5 skips the 0.5-second sound, and a step past
     * "second" skips any sound not yet played. Then places the character info at the start
     * position plus the offset's direction scaled by half its length times
     * 1 - cos(m_angleSpeed * m_timer), the angle in degrees; with MOVE_STILL set the timer stays
     * 0, so this is the start position. */
    void UpdateMove(float dt);
    /* The attack object's callback at 0x1C: reports 5, or, in state 1 when the attack object of
     * the attack still exists (its weak pointer locks) and has one of the bits 0x3D1 set in its
     * word at 0x40, reports 3 and enters state 3. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The attack object's callback at 0x10: enters state 3 from state 1. */
    void OnHit(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The character info's command handler. Command 0x14, in state 1, calls the model's
     * fn_80161880, passes the command, the model, the character info's position and its rotation
     * to fn_8033F0F8, releases the model and enters state 2. Every command gets bit 1 of its byte
     * at 0x7 set. */
    void OnCommand(CharCommand &command);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<Target> m_target;
    /* 0x48 */ s32 m_state; /* a State; 0 until the first update after construction or a reset */
    /* 0x4C */ BAMineParams m_params;
    /* 0x5C */ Fn8040Vector3 m_startPosition;    /* the owner's position */
    /* 0x68 */ float m_angleSpeed;               /* degrees per second */
    /* 0x6C */ float m_timer;                    /* seconds into the current movement */
    /* 0x70 */ UnkBitFlags_80014E04 m_moveFlags; /* MoveFlag bits */
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(BAMine, 0x78);

} // namespace BAMine

#endif
