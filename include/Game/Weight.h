#ifndef GAME_WEIGHT_H
#define GAME_WEIGHT_H

#include "Game/CharaColliAttack.h"
#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace Weight {

/* The gadget parameters in the order of the WEIGHT descriptor's table, whose entry names give the
 * member names. The constructor stores the table's default values. */
struct WeightParams {
    WeightParams()
        : type(0), height(0.0f), intervalUpper(0.0f), intervalLower(0.0f), intervalDamage(0.0f),
          sw(0), scaleX(1.0f), scaleY(1.0f), scaleZ(1.0f) {}

    /* 0x00 */ s32 type;             /* 0 "type : Piston" or 1 "type : Fall" (the value labels) */
    /* 0x04 */ float height;         /* how far above the owner's position the weight rests */
    /* 0x08 */ float intervalUpper;  /* "intv upper(sec)": the wait at the top */
    /* 0x0C */ float intervalLower;  /* "intv lower(sec)": the wait at the bottom */
    /* 0x10 */ float intervalDamage; /* "intv damage(sec)": the pause after the health runs out */
    /* 0x14 */ s32 sw;               /* 0 "switch : disable" or 1 "switch : enable" */
    /* 0x18 */ float scaleX;
    /* 0x1C */ float scaleY;
    /* 0x20 */ float scaleZ;
};
CHECK_SIZEOF(WeightParams, 0x24);

/* Weight::Weight (class name from RTTI; the gadget's descriptor is "WEIGHT"): a WEIGHT.BIN model on
 * a character info that rests "height" above the owner's position and drops to it (behaviour
 * inferred). It falls 2.5 units and rises 1 unit per update, until its height is <= the bottom or
 * >= the top. Its timed waits subtract dt from their timer each update and end only when the
 * result is <= 0 (a NaN timer never ends). At the bottom it tries to play sound 0x1019, waits
 * "intv lower(sec)" and then rises. Back at the top it loads "intv upper(sec)" into the timer: a
 * "Piston" weight (type 0) counts that wait down and drops when it ends, and a "Fall" weight
 * (type 1), whose wait at the top is not counted down, drops when its trigger probe's collision
 * callback runs while it waits there and fn_8015AC84 finds a hit belonging to a character info
 * whose fn_80078B4C returns true. ReadOwner sets the timer to 0 and the state to STATE_START, or
 * to STATE_INACTIVE (an empty case of the update) when "height" <= 0. From STATE_START with
 * "sw" 0, the first update starts the wait at the top with that timer of 0 (when no pause has
 * started before it), so a Piston weight switches to falling at the second update when that
 * update's dt is >= 0 (a negative or NaN dt keeps it waiting) and no pause starts in between. From
 * STATE_START with "sw" not 0, the first update puts the weight at the bottom, where it waits
 * until fn_8016904C reports 1 for its com id; its bottom wait then starts with a timer of 0 and
 * ends under the same conditions. With "sw" 1, a weight back at the top for which fn_8016904C does
 * not report 1 waits for that instead, and its wait at the top then starts with a timer of 0.
 * While it falls, the press probe's collision callback has fn_8015AA84 pass the character infos
 * it takes from the hits to a callback. While the weight's own character info exists, that
 * callback sends each character commands that place it at its own x and z one unit above the
 * owner's height, a DamageCommand of 1.0, a command that starts external control and one that
 * clears its velocity. A failed allocation passes a null command. Only the last character caught
 * is kept. When the weight is back at the top with the caught flag set, it tries to send that
 * character a Player::EndExternalControlCommand if the character still exists (a failed
 * allocation passes a null command), and clears the caught flag either way.
 * Outside the pause, an attack whose attack object exists and has bit 1 of its word at 0x40 set
 * gets 5 in the parameter's word at 0x54 and leaves the health unchanged. Any other attack gets 1
 * there and subtracts its value from the health, which ReadOwner sets to 10. When the health is
 * then <= 0 (a NaN is not), the callback saves the current state, switches to STATE_PAUSED with
 * the timer at "intv damage(sec)" and refills the health. The following updates count the pause
 * down. When its timer runs out the saved state comes back, with its wait restarted only if it
 * is the top or bottom wait. During the pause every attack gets 5. Method and member names are
 * inferred from the code. */
class Weight : public Gadget {
  public:
    /* m_state values (names inferred from what each state does). STATE_TOP_WAIT_SWITCH and
     * STATE_BOTTOM_WAIT_SWITCH wait for the switch; STATE_PAUSED is the pause after the health ran
     * out. */
    enum State {
        STATE_START,
        STATE_TOP_WAIT_SWITCH,
        STATE_BOTTOM_WAIT_SWITCH,
        STATE_TOP_WAIT,
        STATE_BOTTOM_WAIT,
        STATE_RISE,
        STATE_FALL,
        STATE_PAUSED,
        STATE_INACTIVE
    };

    Weight(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Weight();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, rotation and com id from the owner, and the parameters when the owner's
     * block is 0x24 bytes (fn_800C9DBC); raises the character info by "height" above the position,
     * which it keeps as m_baseY; sets the timer to 0, the health to 10 and clears the flags; then
     * sets m_state and m_resumeState to STATE_INACTIVE when "height" <= 0, and to STATE_START
     * otherwise (also for a NaN). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model, when there is one, at the character info, turned about y by its rotation
     * and scaled by the scale parameters. When there is a trigger probe, moves it to the owner's
     * position and rotation; without an owner it passes a zero rotation and a position left
     * uninitialised (original behaviour). */
    void UpdateModelMatrix();
    /* Scales the unscaled sizes by the scale parameters and applies them to whichever of the model
     * and the two probes exist: all three axes for the model's part 0 (fn_801617D0, then
     * fn_80161740 with (0, the scaled height, 0)); x and z for the probes' SetSize, the press probe
     * keeping its unscaled height and the trigger probe taking half of "height", which it also
     * passes to UnkSlot17 as (0, that height, 0). */
    void ApplyScale();
    /* The model attack object's callback (behaviour in the class comment). */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The press probe's collision callback: while the weight falls (STATE_FALL), passes the hits to
     * the catching callback with the owner's height; when that callback caught a character, sets
     * bit 0 of m_holdFlags and keeps the last character caught in m_target. */
    void OnPressCollision(ColliDetectArgs &args);
    /* The trigger probe's collision callback: a "Fall" weight (type 1) waiting at the top
     * (STATE_TOP_WAIT) starts to fall when fn_8015AC84 finds a hit belonging to a character info
     * whose fn_80078B4C returns true. */
    void OnTriggerCollision(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x38 */ WeightParams m_params;
    /* 0x5C */ s32 m_state; /* a State */
    /* The state that STATE_PAUSED returns to: OnAttack saves m_state here when the pause starts,
     * and ReadOwner sets it to the starting state. */
    /* 0x60 */ s32 m_resumeState;
    /* 0x64 */ float m_timer;
    /* 0x68 */ float m_baseY; /* the owner's height: the bottom of the drop */
    /* 0x6C */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x74 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_pressHandle;
    /* 0x7C */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_triggerHandle;
    /* 0x84 */ boost::weak_ptr<CharInfo> m_target; /* the last character caught */
    /* 0x8C */ float m_health;
    /* 0x90 */ u8 m_comId; /* the owner's com id (fn_800C9E60) */
    /* Bit 0 is set by OnPressCollision when it catches a character and cleared when the weight is
     * back at the top (after the EndExternalControlCommand attempt); the constructor and ReadOwner
     * clear all bits. m_target itself is not cleared. */
    /* 0x94 */ UnkBitFlags_80014E04 m_holdFlags;
};
CHECK_SIZEOF(Weight, 0x98);

} // namespace Weight

#endif
