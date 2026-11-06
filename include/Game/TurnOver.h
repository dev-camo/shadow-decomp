#ifndef GAME_TURNOVER_H
#define GAME_TURNOVER_H

#include "Game/CharCommand.h"
#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/LineTarget.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkTarget_80166AF4.h"
#include "rw/rwcore.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace TurnOver {

/* The gadget parameters in the order of the descriptors' table (names from the table's strings).
 * The constructor clears them before reading them from the owner. */
struct TurnOverParams {
    TurnOverParams() : model(0), blast(0) {}

    /* 0x0 */ s32 model; /* the model entry to show */
    /* 0x4 */ s32 blast; /* no code here reads it */
};
CHECK_SIZEOF(TurnOverParams, 0x8);

/* TurnOver::TurnOverTask (class name from RTTI): the gadget of the "TANK" and "TURNOVER"
 * descriptors. Their Create functions differ only in the model file (TANK.BIN or TURNOVER.BIN) and
 * in the first argument that the constructor passes with it to fn_80012700 and fn_8015F800 (0 or
 * 1). The gadget shows the model entry that the parameter "model" selects and keeps a LineTarget
 * on the segment between the model's frames 1000 and 1001. In STATE_IDLE, command 14 requests
 * motion 1 of the entry (fn_80160A84) and enters STATE_LIFTING. In STATE_LIFTING, a UseCommand
 * (id 13) records its user, requests motion 2 and enters STATE_TURNING, and command 15 clears the
 * user, requests motion 3 and enters STATE_CANCELLING, which returns to STATE_IDLE. After
 * STATE_TURNING the gadget waits in STATE_WAITING and STATE_FINISHING and then finishes (see
 * UpdateBody). ScoreCommands go on to the recorded user (see OnCommand). Behaviour inferred;
 * method, member and state names are not original, and the parameters' pointer, reference and
 * const spellings are inferred. */
class TurnOverTask : public Gadget {
  public:
    /* modelArg goes to fn_80012700 and fn_8015F800 together with fileName: 0 for "TANK", 1 for
     * "TURNOVER". After building the probe (CreateProbe), the constructor passes -1 to the probe's
     * UnkSlot2 when there is a handle target; the UseCommand later passes -1 to its UnkSlot1. */
    TurnOverTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner, int modelArg, const char *fileName);
    virtual ~TurnOverTask();
    /* Advances the model (unchecked), and the target and the handle target when they exist, by
     * dt, runs the state, then calls UpdatePositions. The timer tests are m_timer <= 0.0f (false
     * for NaN).
     * - STATE_RESET: when there is a model, creates the target (fn_80166AF4) and hands it to the
     *   model's parts (fn_80169D00); passes 4 to the target's fn_80167010 when there is a target;
     *   enters STATE_IDLE.
     * - STATE_IDLE: nothing (it waits for command 14).
     * - STATE_LIFTING: while bit 1 of m_soundFlags is clear, counts m_timer down by dt and, once
     *   it is <= 0, requests the lift-up sound (PlaySound(0)) and sets the bit, also when that
     *   request finds no sound record and makes no sound call.
     * - STATE_CANCELLING: once the model's fn_80160040 reports true for the entry, enters
     *   STATE_IDLE, passes 1 to the target's fn_80167448 when there is a target, passes 5 and the
     *   character info's position to fn_80169B80 and requests the cancel sound (PlaySound(1),
     *   whose lookup can fail as well).
     * - STATE_TURNING: when m_motion has expired, enters STATE_WAITING, passes -1 to the probe's
     *   UnkSlot2 when there is a handle target and passes 8, 2.0 and the character info's
     *   position to fn_80169AEC. Otherwise, when stMtnControlWorkFlag(motion, 0, 3, 1) is
     *   nonzero, it enters STATE_WAITING and calls UnkSlot2(-1) the same way (without the
     *   fn_80169AEC call); in either case of that test it then passes the sound id of the
     *   motion's event 0x5001 and the character info's position to fn_801D54F4 (which returns
     *   without playing for the id 0xFFFF) and passes the value of event 0x5000 to the
     *   target's fn_80167448 (when there is a target), for each event that fn_80415A18 reports.
     * - STATE_WAITING: counts m_timer down (from the 2 seconds that the UseCommand set); once it
     *   is <= 0, sets it to 1.5, passes -1.0 to the model's fn_8016268C and enters
     *   STATE_FINISHING.
     * - STATE_FINISHING: counts m_timer down; once it is <= 0, calls fn_8015A794 and sets Task
     *   flag 0x01, at every such update. */
    virtual void UpdateBody(float dt);
    /* Rereads the owner (ReadOwner). When the parameter "model" has changed, releases the target
     * and the handle target and calls the model's fn_80161880, fn_801604FC and fn_80162804; it
     * then returns at once unless the model has an entry with the new value (fn_80162EA8), and
     * otherwise shows that entry and finds frames 1000 and 1001 again. The probe is not rebuilt.
     * Then (also when the parameter is unchanged) it queues a debug cross at each end of the line
     * target's segment and a debug line between them, calls UpdatePositions and
     * UpdateLineTarget, the model's fn_80162EF4 and fn_8016108C, and fn_8015BE64 when there is a
     * handle target. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Create the gadget under task layer 11 for the "TANK" and "TURNOVER" descriptors (names
     * inferred). */
    static void CreateTank(UnkGadgetOwner_800C9CD0 *owner);
    static void CreateTurnOver(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position and rotation from the owner, copies the owner's parameter block into
     * m_params only when its size is 8 (fn_800C9DBC flags the owner otherwise) and returns
     * to STATE_RESET (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info, turned about the y axis by the character info's
     * rotation, then gives the line target the segment from frame 1000 to frame 1001 of the
     * model, both raised by 1 (name inferred). Neither the model nor the line target is checked. */
    void UpdatePositions();
    /* When there is a line target, stores s_lineTargetField20 (20.0) in its float at 0x20 and
     * passes the z axis, turned about the y axis by the character info's rotation, to fn_801E73B0
     * (name inferred). */
    void UpdateLineTarget();
    /* The character info's command handler (name inferred; see the class comment). Each accepted
     * command 14, 13 or 15 calls UpdatePositions after it requests the motion.
     * - Command 14, only in STATE_IDLE: sets m_timer to 0.2, clears bit 1 of m_soundFlags and
     *   passes 1 to the target's fn_80167448 when there is a target.
     * - UseCommand (id 13), only in STATE_LIFTING: sets m_timer to 2, keeps the motion control of
     *   the entry (fn_801602B4) in m_motion, passes 2 to the target's fn_80167448 when there is a
     *   target and -1 to the probe's UnkSlot1 when there is a handle target.
     * - Command 15, only in STATE_LIFTING: clears m_user.
     * - ScoreCommand (id 28), in any state: while m_user still locks, passes it a new
     *   ScoreCommand with the received one's five words (a null pointer when the allocation
     *   fails). */
    void OnCommand(CharCommand &command);
    /* The first callback passed to fn_80054DD0: stores 1 in param's field_0x30 when the attack
     * object of param's field_0x8 still exists and its owner still exists and has the value 1 in
     * its word at 0x5C. */
    void fn_80125F1C(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The second callback passed to fn_80054DD0; it does nothing. */
    void fn_80125F18(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* Builds the probe of the model entry from its collision file (fn_801616CC) and its motion
     * control (fn_801602B4) with fn_800C7F7C, only when the file is not null and the motion
     * control still locks; when the probe's attack object exists, gives it the two callbacks above
     * with 3, 0xBC0 and 999.0 (fn_80054DD0); then wraps the probe in the handle target (name
     * inferred). */
    void CreateProbe();
    /* Releases the handle target (name inferred). */
    void ReleaseProbe();
    /* Requests a sound: looks up the record of m_resource named after the model (fn_801626D8)
     * with "_liftup" (sound 0) or "_cancel" (sound 1) and, only when the record exists, passes the
     * low 16 bits of its word at 0x8 to fn_801D560C (which returns without playing for the id
     * 0xFFFF or when it gets no sound object); other values do nothing (name inferred). */
    void PlaySound(int sound);

    /* The values of m_state (names inferred from the commands and motions that lead to them). */
    enum State {
        STATE_RESET = 0,
        STATE_IDLE = 1,
        STATE_LIFTING = 2,
        STATE_CANCELLING = 3,
        STATE_TURNING = 4,
        STATE_WAITING = 5,
        STATE_FINISHING = 6
    };

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<LineTarget> m_lineTarget;
    /* 0x48 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x50 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
    /* The motion control of the model entry, kept by the UseCommand. */
    /* 0x58 */ boost::weak_ptr<_stMtnPlayWork> m_motion;
    /* The user of the last accepted UseCommand; an accepted command 15 clears it; resets do not
     * touch it. */
    /* 0x60 */ boost::weak_ptr<CharInfo> m_user;
    /* 0x68 */ s32 m_state;
    /* The model's frames with the ids 1000 and 1001 (null when not found), searched by the
     * constructor and by a reset that changes the model entry; nothing checks them for null. */
    /* 0x6C */ RwFrame *m_frame1000;
    /* 0x70 */ RwFrame *m_frame1001;
    /* Seconds left in STATE_LIFTING (before the lift-up sound), STATE_WAITING and
     * STATE_FINISHING. A UseCommand or command 15 that arrives before STATE_LIFTING's timer is
     * <= 0 leaves that state without the lift-up sound (the UseCommand sets the timer to 2, which
     * STATE_WAITING later counts down). */
    /* 0x74 */ float m_timer;
    /* 0x78 */ s32 m_modelIndex; /* the entry identifier passed to the model lookups */
    /* 0x7C */ TurnOverParams m_params;
    /* 0x84 */ UnkResource_80012700 *m_resource; /* the model file's resource */
    /* Bit 1: set once STATE_LIFTING has made its lift-up sound request (whether or not the lookup
     * found a sound); an accepted command 14 clears it. The constructor starts it clear; ReadOwner
     * and ResetFromOwner do not touch it. */
    /* 0x88 */ UnkBitFlags_80014E04 m_soundFlags;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(TurnOverTask, 0x90);

} // namespace TurnOver

#endif
