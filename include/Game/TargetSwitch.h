#ifndef GAME_TARGETSWITCH_H
#define GAME_TARGETSWITCH_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The gadget's parameters, in the order of the descriptor's table ("X", "Y", "Z", "sec", "pause",
 * "hitpoint"). The constructor stores zero in the floats and 2 in hitpoint; the table gives sec and
 * pause 1 as their first float (their default, inferred). */
struct TargetSwitchParams {
    TargetSwitchParams() : x(0.0f), y(0.0f), z(0.0f), sec(0.0f), pause(0.0f), hitpoint(2) {}

    /* x, y, z: the offset from the owner's position to the far end of the movement (inferred). */
    /* 0x00 */ float x;
    /* 0x04 */ float y;
    /* 0x08 */ float z;
    /* 0x0C */ float sec;   /* the nominal time of one movement there and back (inferred) */
    /* 0x10 */ float pause; /* the wait at each end of the movement (inferred) */
    /* 0x14 */ s32 hitpoint;
};
CHECK_SIZEOF(TargetSwitchParams, 0x18);

/* TargetSwitch (class name from RTTI; the gadget's descriptor is "TARGETSWITCH"): the
 * TARGETSWITCH.BIN model, which moves from the owner's position towards that position plus the
 * offset (X, Y, Z) along a cosine curve and back, a nominal "sec" seconds per movement, waiting
 * "pause" seconds after each leg's threshold is crossed, at the position that update computed,
 * which is not clamped to the endpoint (see UpdateBody);
 * unless "sec" > 0 it stays at the owner's position. Each attack it accepts (see OnAttack) takes
 * the attack's float at 0x50 (the damage, inferred) from the hit points and recolours the model;
 * once they are <= 0, the next update sets the state of the owner's com id to 1 in the manager of
 * fn_800CB69C when m_owner is set and finishes the gadget (see UpdateBody; behaviour inferred).
 * Method, member and constant names and the parameters' pointer and reference spellings are
 * inferred. */
class TargetSwitch : public Gadget {
  public:
    /* m_state values (names inferred from what each state does). */
    enum State { STATE_START, STATE_GO, STATE_RETURN, STATE_WAIT, STATE_DESTROYED };

    TargetSwitch(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~TargetSwitch();
    /* Advances the model when the model pointer is non-null, runs the state and places the model
     * (UpdateModelMatrix). 0 enters 1. 1 advances the timer and enters 3 with the wait timer set
     * to "pause" when the angle (m_angleSpeed times the timer, in degrees) is >= 180. 2 advances
     * the timer and, when it is > "sec", restarts it at 0 and enters 3 the same way. In both, the
     * angle computed before that test places the gadget in the same update (UpdatePosition). 3
     * counts the wait timer down and, when it is <= 0, enters 2 if the timer is > 0 and 1
     * otherwise (a NaN timer gives 1). 4, only when m_owner is set, sets the state of the owner's
     * com id (fn_800C9E60) to 1 in the manager of fn_800CB69C (fn_801690D0) and passes the owner
     * to fn_800C9B34; then, in any case, it passes the character info's position and 0xA9 to
     * fn_8015D794 (an effect, inferred), discards the returned weak pointer and sets Task flag
     * 0x01. Unless "sec" > 0 the angle speed is 0, so state 1 lasts until an attack sets 4. */
    virtual void UpdateBody(float dt);
    /* Rereads the owner (CopyFromOwner) and places the model (UpdateModelMatrix). When the model
     * pointer is non-null it also calls the model's fn_80162EF4 and fn_8016108C and queues a debug
     * line from the character info's position to that position plus the offset (fn_802E0B58). */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and its rotation (converted to radians). Copies the owner's
     * parameters only when the owner's parameter size equals sizeof(TargetSwitchParams), 0x18
     * (fn_800C9DBC); otherwise m_params keeps its previous values. Then restarts the timer, sets
     * the angle speed to 360 / "sec" when "sec" > 0 and to 0 otherwise (a NaN included), and sets
     * the hit points to "hitpoint". It does not change the state. */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When the model pointer is non-null, places the model at the character info's position,
     * turned about the y, x and z axes by the character info's rotation. When the target pointer
     * is non-null, copies that position to the target object. */
    void UpdateModelMatrix();
    /* Places the character info at the owner's current position plus the offset's direction
     * scaled by half its length times 1 - cos(angle), the angle in degrees. An offset shorter
     * than 1e-6 normalises to the zero vector (fn_8040CEE8), which leaves it at the owner's
     * position. It also adds the owner's position to a copy of the offset, a result that it does
     * not use. */
    void UpdatePosition(float angle);
    /* The attack object's callback at 0x1C. In state 4, or when the attack object of the attack
     * still exists (its weak pointer locks) and has bit 0x2 set in its word at 0x40, it reports 5
     * and does nothing else. Otherwise it takes the attack's float at 0x50 from the hit points.
     * When they are then <= 0, it reports 3, enters state 4 (which the next update handles) and
     * tries to play sound 0x100F; otherwise (a NaN included) it reports 1 and tries to play sound
     * 0x100E. Then it calls the model's fn_80160B18(0) when the model pointer is non-null and
     * recolours the model (SetColor). */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* When the model pointer is non-null, calls the model's fn_8016232C and passes it a name for
     * the hit points to fn_8016239C (a colour, inferred from the words): "purple" from 8, "blue"
     * from 6, "green" from 4, "yellow" from 2 and "red" below 2 (each threshold inclusive, tested
     * from the highest; a NaN gives "red"). */
    void SetColor(float hitpoints);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ TargetSwitchParams m_params;
    /* 0x58 */ float m_timer;      /* seconds into the current movement there and back */
    /* 0x5C */ float m_angleSpeed; /* degrees per second */
    /* 0x60 */ float m_waitTimer;  /* seconds left to wait at an end */
    /* 0x64 */ float m_hitpoints;
    /* 0x68 */ s32 m_state; /* a State; 0 after construction */
    /* 0x6C */ boost::shared_ptr<Target> m_target;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(TargetSwitch, 0x78);

#endif
