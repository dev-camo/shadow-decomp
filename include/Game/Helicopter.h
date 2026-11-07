#ifndef GAME_HELICOPTER_H
#define GAME_HELICOPTER_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class CharCommand;

/* The gadget parameters in the order of the descriptor's table (names from the table's strings).
 * The label callback names the three move parameters after "MoveType": the first label for
 * "Straight" (0), the second for "Circle" (1). */
struct HelicopterParams {
    /* 0x00 */ u32 moveType;     /* "MoveType" */
    /* 0x04 */ float moveParam0; /* "MoveLengthX" or "CircleRadius" */
    /* 0x08 */ float moveParam1; /* "MoveLengthY" or "MoveSpd (point/sec)" */
    /* 0x0C */ float moveParam2; /* "MoveLengthZ" or "InitPos (deg)" */
    /* 0x10 */ float moveSec;    /* "MoveSec"; labelled "Not Used" for "Circle" */
    /* 0x14 */ float pauseSec;   /* "PauseSec"; labelled "Not Used" for "Circle" */
};
CHECK_SIZEOF(HelicopterParams, 0x18);

/* Helicopter (class name from RTTI; the gadget's descriptor has the same name): the model
 * HELICOPTER.BIN, moved by its parameters, with two bars below it that the character of
 * lbl_805EE588 can hang on (the bars and the hanging are inferred from the probes and the
 * Player::TouchHangCommand it sends). Behaviour inferred from the code:
 * - "Straight" moves it from the owner's position along the "MoveLength" vector on a half cosine
 *   over "MoveSec" seconds. When the timer reaches "MoveSec" (or at once when fn_8000A950 reports
 *   "MoveSec" as 0) it starts the pause where the last sampled position left it, without placing it
 *   at the end of the vector; once more than "PauseSec" seconds have passed it moves back the same
 *   way, and so on (MoveStraight). "Circle" turns it about the owner's position by "MoveSpd"
 *   divided by "CircleRadius" radians a second at the distance "CircleRadius" for a positive
 *   radius, unless the offset from the owner is shorter than 0.000001 (fn_8040CE74 then normalises
 *   it to the zero vector and the gadget stays at the owner's position); a negative radius puts it
 *   on the opposite side of the owner's position each update, and a zero radius is used as a
 *   divisor without a test. ReadOwner places it "CircleRadius" from the owner's position at the
 *   angle "InitPos" (MoveCircle, ReadOwner).
 * - Each update turns its yaw towards its horizontal movement and leans it forward with the speed
 *   (UpdateTilt).
 * - When a hit of one of the two bar probes is the character of lbl_805EE588, it sends that
 *   character a TouchHangCommand with the nearest point of the bar (OnCollision). A RideCommand
 *   received after that makes the command's own weak rider the rider, without checking that it is
 *   the character the bar hit; while STATE_RIDDEN is set, every update sends the rider a
 *   TouchHangCommand again. Command 12, or a rider that no longer exists, ends the ride
 *   (ReleaseRider). ReadOwner clears STATE_RIDDEN without calling ReleaseRider, which stops these
 *   sends but keeps the rider and the handle targets' flag bits.
 * - Attacks lower its health from 10 (OnAttack); once destroyed, the next update shows the break
 *   effect, ends the ride and finishes the gadget.
 * Method, member and constant names are inferred, and so are the parameters' pointer, reference
 * and const spellings unless a comment gives the evidence. */
class Helicopter : public Gadget {
  public:
    /* The values of HelicopterParams::moveType (names from the label strings). */
    enum MoveType { MOVE_STRAIGHT = 0, MOVE_CIRCLE = 1 };

    /* The bits of m_states (names inferred from where they are set and cleared). ReadOwner clears
     * all of them. */
    enum State {
        /* Set by OnAttack when the health runs out; nothing else clears it. */
        STATE_DESTROYED = 0,
        /* Set by OnCollision after it sends a TouchHangCommand; cleared by a RideCommand and by
         * ReleaseRider. */
        STATE_TOUCHED = 1,
        /* Set by a RideCommand received while STATE_TOUCHED is set; cleared by ReleaseRider. */
        STATE_RIDDEN = 2
    };

    /* Builds the character info (id 0x53) with its command handler (OnCommand), reads the owner
     * (ReadOwner), creates three Targets for the character info (fn_801E7128 with 0xA800, 0x8400
     * and 0x8400; UpdateModelMatrix places them), loads the model and calls its fn_80162A68,
     * fn_80160C28, fn_8016082C, fn_8015FF60, fn_80160B18 and fn_80161BD8, and, when the attack
     * object that the model's fn_80161428(0) returns still exists, installs OnAttack as that
     * object's callback (fn_80096B64 with 1). Then it creates the two bar probes, each with a
     * handle target whose callback is OnCollisionBarA or OnCollisionBarB, places everything,
     * hands an UnkTarget_80166AF4 to the model's parts and starts sound 0x713B at the position. */
    Helicopter(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    /* Stops the sound before the members are destroyed. */
    virtual ~Helicopter();
    /* When STATE_DESTROYED is set: releases the UnkTarget_80166AF4 (when there is one), calls the
     * model's fn_80162790 and fn_80161880, passes an effect parameter block for the character
     * info (with the resource that fn_80012700 looks up for key 1 under the name of the model's
     * entry 1000, which fn_8015F7BC finds and the code reads without a test; the lookup result is
     * null when the resource is absent and is not tested; and the y coordinate of the position)
     * to fn_8015D93C, calls fn_8000D664 with
     * fn_80009548's manager and (0, 0.7, 1.6, 8), ends the ride (ReleaseRider), stops
     * the sound, plays sound 0x713C at the position, calls fn_800C9AD4 with the owner and sets
     * Task flag 0x01. Otherwise moves (MoveStraight or MoveCircle by the move type; other values
     * do not move), stores the movement divided by dt in the
     * character info's field_0x18 (the zero vector when fn_8000A950 reports dt as 0) and tilts
     * (UpdateTilt). Then, with STATE_RIDDEN set, sends the rider a TouchHangCommand while it
     * still exists and ends the ride otherwise; places everything and updates the model, both
     * handle targets and the UnkTarget_80166AF4 (when there is one). */
    virtual void UpdateBody(float dt);
    /* Rereads the owner and places everything, calls the model's fn_80162EF4 and fn_8016108C and
     * queues a debug shape: for "Straight" a line from the position along the "MoveLength"
     * vector, for "Circle" a cylinder at the owner's position, turned by 90 degrees about x, with
     * half length 2.5 and radius "CircleRadius". */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and its rotation (converted to radians) into the character
     * info, with the x and z angles set to 0, resets field_0x18 (fn_8040CE60) and copies the
     * owner's parameters when the owner's parameter size equals sizeof(HelicopterParams), 0x18
     * (fn_800C9DBC); otherwise m_params keeps its previous values. For "Circle" it then moves the
     * position by (0, 0, -"CircleRadius") turned about y by "InitPos" degrees and, unless
     * fn_8000A950 reports the radius or the speed as 0, turns the yaw along the circle (the other
     * way for a negative speed). Last it clears the states, starts moving forward from the
     * beginning, and sets the health to 10 and the turn speed to 0. */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the position, turned about y and x by the rotation, the first Target at
     * the position and the bar Targets at the bar centres. With STATE_RIDDEN set it also moves
     * the hang position with the model (m_hangOffset) and points the hang angles from the
     * position towards it in the horizontal plane. */
    void UpdateModelMatrix();
    /* "Straight": adds dt to the timer. Moving (m_moveState 0): while the timer is below
     * "MoveSec" and fn_8000A950 does not report "MoveSec" as 0, places the gadget at the owner's
     * position plus the "MoveLength" vector times 0.5 * (1 - cos(pi * timer / "MoveSec")), or
     * times 1 minus that while m_moveBack is set; otherwise zeroes the timer and starts waiting
     * (m_moveState 1) without moving the gadget, so it waits at the last sampled position, not
     * at the end of the vector. Waiting: once the timer is above "PauseSec", zeroes it, flips
     * m_moveBack and moves again. */
    void MoveStraight(float dt);
    /* "Circle": unless fn_8000A950 reports "MoveSpd" as 0, normalises the offset from the owner's
     * position, turns it about y by "MoveSpd" * dt / "CircleRadius" radians and multiplies it by
     * "CircleRadius". For a positive radius that keeps the gadget "CircleRadius" from the owner's
     * position, except when the offset is shorter than 0.000001: fn_8040CE74 then makes it the
     * zero vector (USA (GUPE8P) 0x8040CE90) and the gadget stays at the owner's position. A
     * negative radius puts it on the opposite side, and a zero radius is used as the
     * divisor without a test. */
    void MoveCircle(float dt);
    /* Unless the squared length of field_0x18 is below 0.0001, takes its part across the y axis.
     * When that part is that short too, sets the x angle to 0. Otherwise, unless fn_8000A950
     * reports the angle between the facing and that part as 0, raises the turn speed by 10
     * degrees a second each second up to 15 degrees a second and turns the yaw towards the part
     * by at most the turn speed times dt; when it reports 0, sets the turn speed to 0. Then sets
     * the x angle to 16 degrees times the part's length divided by 60, clamped to 0 to 1. */
    void UpdateTilt(float dt);
    /* Sends rider a Player::TouchHangCommand with this gadget's character info, the hang position
     * and the hang angles (and 3 and 4). The command is null when its allocation fails; the
     * send happens either way. rider is passed by value: the callers copy it into a temporary. */
    void SendTouchHang(boost::shared_ptr<CharInfo> rider);
    /* The handle targets' callbacks: OnCollision for bar A or bar B. */
    void OnCollisionBarA(ColliDetectArgs &args);
    void OnCollisionBarB(ColliDetectArgs &args);
    /* Does nothing without a hit list. Otherwise takes the character info that fn_800C4568 finds
     * for each hit; at the first one that still exists and is the character info that
     * fn_800A8B74 returns for lbl_805EE588 (only when that one still exists), sets the hang
     * position to the point of the bar (in the gadget's current placement) nearest the gadget's
     * position, keeps it in the model's space (m_hangOffset), sends that character a
     * TouchHangCommand, sets STATE_TOUCHED and stops. */
    void OnCollision(ColliDetectArgs &args, bool barA);
    /* The character info's command handler. Command 11 (a RideCommand), only while
     * STATE_TOUCHED is set: clears it, makes the command's rider the rider, sets bit 1 of both
     * handle targets' flags and sets STATE_RIDDEN. Command 12: ReleaseRider. Sets bit 1 of the
     * command's flags in every case. */
    void OnCommand(CharCommand &command);
    /* Clears STATE_RIDDEN and STATE_TOUCHED, clears bit 1 of both handle targets' flags,
     * suspends both probes for 1 second and resets the rider. */
    void ReleaseRider();
    /* The attack callback. Reports 5 unless the attack object still exists with bit 1 (0x2) of its
     * word at 0x40 clear: then it reports 1 and, unless STATE_DESTROYED is already set, subtracts
     * the callback parameter's field_0x50 from the health; when the health is then below 0 or
     * fn_8000A950 reports it as 0, it reports 3, passes the attacker (fn_8015D570), when there is
     * one, to fn_8020642C with 0x39 and sets STATE_DESTROYED. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x30 */ HelicopterParams m_params;
    /* 0x48 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x50 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x58 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_barHandleA;
    /* 0x60 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_barHandleB;
    /* 0x68 */ boost::shared_ptr<UnkTarget_80166AF4> m_effectTarget;
    /* 0x70 */ UnkSoundHandle_801D3DC8 m_sound;        /* sound 0x713B */
    /* 0x8C */ boost::shared_ptr<Target> m_target;     /* at the position */
    /* 0x94 */ boost::shared_ptr<Target> m_barTargetA; /* at the centre of bar A */
    /* 0x9C */ boost::shared_ptr<Target> m_barTargetB; /* at the centre of bar B */
    /* 0xA4 */ UnkBitFlags_80014E04 m_states;          /* State bits */
    /* 0xA8 */ s32 m_moveState;                        /* "Straight": 0 moving, 1 waiting */
    /* 0xAC */ s32 m_moveBack;                         /* "Straight": nonzero while moving back */
    /* 0xB0 */ float m_moveTimer; /* "Straight": seconds in the current move or wait */
    /* 0xB4 */ float m_health;
    /* 0xB8 */ float m_turnSpeed; /* radians a second */
    /* 0xBC */ boost::weak_ptr<CharInfo> m_rider;
    /* 0xC4 */ Fn8040Vector3 m_hangPosition;
    /* 0xD0 */ UnkRotation3 m_hangAngles;
    /* 0xDC */ Fn8040Vector3 m_hangOffset; /* the hang position in the model's space */
};
CHECK_SIZEOF(Helicopter, 0xE8);

#endif
