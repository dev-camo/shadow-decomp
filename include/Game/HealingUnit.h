#ifndef GAME_HEALINGUNIT_H
#define GAME_HEALINGUNIT_H

#include "Game/CharCommand.h"
#include "Game/EffectElement.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetPickup.h"
#include "Game/PickupCharInfo.h"
#include "Game/Task.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class UnkGadgetOwner_800C9CD0;

namespace HealingUnit {

/* HealingUnit::HealingBlast (class name from RTTI, with Task as its only base): a task with a
 * character info ("HealingBlast") at a position, a sphere probe there with a handle target, and an
 * effect. The first time the handle target passes the probe's hits on, OnCollision sends the
 * character infos in them RecoverCommands (see OnCollision). The 1-second timer counts down from
 * the second update on; the update that finds it <= 0 enters STATE_FINISHED and the next one marks
 * the task finished. Created by fn_80107060. Method and member names are inferred, and so are the
 * parameters' pointer, reference and const spellings. */
class HealingBlast : public Task {
  public:
    /* The values of m_state (names inferred). Nothing stores 1. */
    enum State { STATE_START = 0, STATE_RUNNING = 2, STATE_FINISHED = 3 };

    /* Builds the Task under parent, sets the task name to s_name, builds the character info
     * (fn_8015CB88(0x4E, 4, "HealingBlast")) and copies *position to its position. type 0 gives a
     * probe radius of 20 and effect 0xC5, type 1 a radius of 40 and effect 0x2DA, and any other
     * value a radius of 0 and effect 0. Creates the sphere probe at the character info's position
     * (fn_800C7EA8 with 0x40000, 3, 0, an empty owner and lbl_805E07A0), wraps it in the handle
     * target with OnCollision as its callback, starts the effect at the position with the direction
     * (0, 1, 0), stores amount, enters STATE_START with the timer at 1 and copies owner. */
    HealingBlast(Task *parent, const boost::weak_ptr<CharInfo> &owner,
                 const Fn8040Vector3 *position, int type, float amount);
    /* Clears the effect's active flag while the effect exists, before the members are
     * destroyed. */
    virtual ~HealingBlast();
    /* First the state: STATE_START enters STATE_RUNNING; STATE_RUNNING counts the timer down by dt
     * and enters STATE_FINISHED once it is <= 0 (a NaN timer never is); STATE_FINISHED sets bit 0
     * of the task flags (finished). Then, in every state, runs the handle target (fn_8015BE9C)
     * when the handle pointer is not null; it can call OnCollision in the same update. */
    virtual void Update(float dt);

    /* The handle target's callback for the probe's hits. Passes them to fn_8015B384 with the hit
     * callback (fn_80107EAC) and a context holding m_owner and m_amount: fn_8015B384 calls the hit
     * callback only for the hits whose collision object and character info still lock and whose
     * character info's fn_80073634 returns true, and the hit callback, which tests only that the
     * context is not null, sends each of them a new RecoverCommand with the owner and the amount (a
     * request; the command is null when its allocation fails, and it is sent either way). Then sets
     * bit 1 of the handle target's flags, after which fn_8015BE9C no longer passes the probe's hits
     * on, so this runs once. */
    void OnCollision(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x28 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x30 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle; /* of the sphere probe */
    /* 0x38 */ boost::weak_ptr<CharInfo> m_owner; /* the sender of the RecoverCommands */
    /* 0x40 */ boost::weak_ptr<Effect::System::EffectElement> m_effect;
    /* 0x48 */ s32 m_state;    /* a State */
    /* 0x4C */ float m_amount; /* passed to each RecoverCommand; fn_80107060 passes 1 */
    /* 0x50 */ float m_timer;  /* seconds left in STATE_RUNNING */
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(HealingBlast, 0x58);

/* HealingUnit::HealingUnit (class name from RTTI, with GadgetPickup::PickupBase, GadgetCharInfo
 * and CharInfo as its bases): the object that the player picks up, throws or casts. It keeps a
 * PickupCharInfo of its own (m_charInfo, also named "HealingUnit") with a target, and the model of
 * HEALINGUNIT.BIN. Behaviour inferred from the code:
 * - In STATE_WAITING with FLAG_PICKED clear a PickupCommand (id 0x11) makes it picked up (see
 *   OnCommand); the next update sends the picker a ReadyCommand only when the picker's weak pointer
 *   locks, and enters STATE_HELD either way. In STATE_HELD each update and each call of the grab
 *   position handler place it at the grab matrix (UpdateHeldPosition).
 * - In STATE_HELD a ThrowCommand (0x13) or CastCommand (0x12) starts STATE_FLYING: UpdateMotion
 *   moves it under an acceleration and passes its movement to fn_801BFC28, whose true result
 *   enters STATE_BURST. Each update in STATE_BURST creates a HealingBlast (type 0) at its position
 *   with the picker as owner and sets FLAG_DONE.
 * - In STATE_WAITING a VacuumCommand (0x14) hands the model to fn_8033F0F8 and enters
 *   STATE_VACUUMED, where each update sets FLAG_DONE.
 * GadgetPickup::PickupBaseTask's update calls Update and then TestFlags with bit 1, which reports
 * FLAG_DONE, and finishes the task when it is true. Method, member and
 * constant names are inferred, and so are the parameters' pointer, reference and const spellings
 * unless a comment gives the evidence. */
class HealingUnit : public GadgetPickup::PickupBase {
  public:
    /* The values of m_state (names inferred from what each state does). */
    enum State {
        /* Entered by ResetFromOwner and ResetTransform; the next update enters STATE_WAITING. */
        STATE_START = 0,
        STATE_WAITING = 1,
        STATE_PICKED = 2,
        STATE_HELD = 3,
        STATE_FLYING = 4,
        STATE_BURST = 5,
        STATE_VACUUMED = 6
    };

    /* The bits of m_flags (names inferred). Initialize clears them all; nothing else clears
     * one. */
    enum Flag {
        /* Set by a PickupCommand, a ThrowCommand and a CastCommand; while it is set each update
         * starts with UpdateMotion. */
        FLAG_MOVING = 0,
        /* Set by a PickupCommand; while it is set a PickupCommand is ignored. */
        FLAG_PICKED = 1,
        /* Set by every update in STATE_BURST and STATE_VACUUMED. */
        FLAG_DONE = 2,
        /* Set by a ThrowCommand and a CastCommand: UpdateMotion adds the acceleration. */
        FLAG_ACCELERATE = 3,
        /* Set by a ThrowCommand and a CastCommand: UpdateMotion tests the movement. */
        FLAG_SWEEP = 4
    };

    /* Both constructors build the PickupBase with 0x4E and call fn_801DC984 with "HealingUnit";
     * the second also copies owner. */
    HealingUnit();
    HealingUnit(const boost::weak_ptr<CharInfo> &owner);
    /* When the owner still exists (its weak pointer locks), sends it a new UseCommand naming
     * m_charInfo (a request; the command is null when its allocation fails, and it is sent either
     * way), before the members are destroyed. */
    virtual ~HealingUnit();

    /* Copies the position and rotation to the unit's own character info (PickupBase), and when the
     * model pointer is not null places the model at position, turned about the y axis by the
     * rotation's y. */
    virtual void SetTransform(const Fn8040Vector3 *position, const UnkRotation3 *rotation);
    /* owner is not used. Clears m_flags and sets bit 1 of m_sweepFlags; builds m_charInfo
     * (PickupCharInfo(0x4E)), installs OnCommand as its command unit's handler, creates its target
     * (fn_801E4160 with 0x80 and 30), calls its fn_801DC984 with "HealingUnit" and installs
     * OnSetGrabPosition as its grab position handler; loads the model from HEALINGUNIT.BIN and
     * calls its fn_80162A68(0, 0) and fn_80161FC8(0, m_charInfo). None of the pointers is tested
     * for null. */
    virtual void Initialize(UnkGadgetOwner_800C9CD0 *owner);
    /* Runs UpdateMotion first while FLAG_MOVING is set, then advances the model when the model
     * pointer is not null, then the state:
     * - STATE_START: enters STATE_WAITING.
     * - STATE_PICKED: sends the picker, when it still exists, a new ReadyCommand naming m_charInfo
     *   (a request; null when its allocation fails, sent either way), then enters STATE_HELD.
     * - STATE_HELD: UpdateHeldPosition.
     * - STATE_FLYING: SetTransform with m_charInfo's position and rotation.
     * - STATE_BURST: creates a HealingBlast with fn_80107060 (the picker as owner, m_charInfo's
     *   position, type 0), passes the picker to fn_8020642C with 0x20 when it still exists, and
     *   sets FLAG_DONE; it stays in this state.
     * - STATE_VACUUMED: sets FLAG_DONE.
     * Last, in every state, passes m_charInfo's position raised by 8 to its fn_801E413C. */
    virtual void Update(float dt);
    /* When m_charInfo is not null, copies the owner's position and rotation to it; then enters
     * STATE_START and calls SetTransform with m_charInfo's position and rotation. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When m_charInfo is not null, copies *position and *rotation to it; then enters STATE_START
     * and calls SetTransform with m_charInfo's position and rotation. */
    virtual void ResetTransform(const Fn8040Vector3 *position, const UnkRotation3 *rotation);
    /* True when bit 0 of flags and FLAG_PICKED are set, or bit 1 of flags and FLAG_DONE. */
    virtual bool TestFlags(const UnkBitFlags_80014E04 &flags);

    /* The descriptor's factory: builds a HealingUnit in a shared pointer, calls its Initialize and
     * ResetFromOwner with owner and creates a GadgetPickup::SetPickupBaseTask for it under task
     * layer 11 (only when the allocation succeeds; the result is not kept). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* While FLAG_ACCELERATE is set, adds field_0x90 * field_0x8C to m_charInfo's acceleration
     * (field_0x24). Advances m_charInfo's position and velocity (field_0x18) by fn_804103E8 with
     * that acceleration, a drag of 1 and dt. While FLAG_SWEEP is set, passes the address of a copy
     * of m_previousPosition, the movement since it, m_sweepFlags and 10 to fn_801BFC28; when the
     * result is true, moves m_charInfo to that copy, sets its velocity to the movement divided by
     * dt and enters STATE_BURST. Then stores m_charInfo's position in m_previousPosition and resets
     * its acceleration with fn_8040CE60. */
    void UpdateMotion(float dt);
    /* Moves m_charInfo to the grab matrix's translation, places the model with the grab matrix
     * (without testing the model pointer) and copies m_charInfo's position and rotation to the
     * unit's own character info (PickupBase::SetTransform). */
    void UpdateHeldPosition();
    /* m_charInfo's grab position handler: copies *args.m_matrix to m_grabMatrix and, in STATE_HELD,
     * calls UpdateHeldPosition. */
    void OnSetGrabPosition(PickupCharInfo::SetGrabPositionArgs &args);
    /* m_charInfo's command handler; sets bit 1 of every command's flags byte after handling it.
     * - PickupCommand (0x11), in STATE_WAITING with FLAG_PICKED clear: keeps the command's picker,
     *   clears bit 0 of m_charInfo's target flags, calls the model's fn_80161230(0, -1), enters
     *   STATE_PICKED and sets FLAG_PICKED and FLAG_MOVING.
     * - ThrowCommand (0x13) and CastCommand (0x12), in STATE_HELD: copy the command's vector to
     *   field_0x90, set field_0x8C to -320, reset m_charInfo's acceleration with fn_8040CE60 and
     *   set its velocity to the grab matrix's rotation applied to (0, 100, -80) for a throw or
     *   (0, 100, 0) for a cast; a throw also sets m_charInfo's rotation from the grab matrix
     *   (fn_800494F8) before the velocity. Both enter STATE_FLYING and set FLAG_ACCELERATE,
     *   FLAG_MOVING and FLAG_SWEEP.
     * - VacuumCommand (0x14), in STATE_WAITING: calls the model's fn_80161880, passes the
     *   command, the model, 1 and the unit's own position and rotation to fn_8033F0F8, releases
     *   the model and enters STATE_VACUUMED.
     * Other commands, and these ones in other states, only get the flag bit. */
    void OnCommand(CharCommand &command);

    /* 0x70 */ s32 m_state; /* a State */
    /* 0x74 */ boost::shared_ptr<PickupCharInfo> m_charInfo;
    /* 0x7C */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x84 */ boost::weak_ptr<CharInfo> m_picker; /* from the PickupCommand */
    /* 0x8C */ float field_0x8C;         /* -320 after a throw or cast; scales field_0x90 */
    /* 0x90 */ Fn8040Vector3 field_0x90; /* the vector of the ThrowCommand or CastCommand */
    /* 0x9C */ Fn8040Vector3 m_previousPosition;
    /* 0xA8 */ Fn8040Matrix34 m_grabMatrix;       /* the last matrix of the grab position handler */
    /* 0xD8 */ boost::weak_ptr<CharInfo> m_owner; /* from the second constructor */
    /* 0xE0 */ UnkBitFlags_80014E04 m_flags;      /* Flag bits */
    /* 0xE4 */ UnkBitFlags_80014E04 m_sweepFlags; /* passed to fn_801BFC28 */
};
CHECK_SIZEOF(HealingUnit, 0xE8);

} // namespace HealingUnit

extern "C" {
/* Creates a HealingUnit::HealingBlast under task layer 11 (only when the allocation succeeds) with
 * owner, position, type and the amount 1. The unit and fn_8033EE64 pass type 0, fn_80053010 passes
 * 0 and 1. */
void fn_80107060(const boost::weak_ptr<CharInfo> &owner, const Fn8040Vector3 *position, int type);
/* As fn_801072C4 (Game/fn_801072C4.h) for a unit without an owner: builds a HealingUnit in a shared
 * pointer, calls its Initialize with null and its ResetTransform with position and rotation,
 * creates a GadgetPickup::PickupBaseTask for it under task layer 11 (only when the allocation
 * succeeds) and calls SetRadius(radius) on the result without testing it for null. fn_8033E828 is
 * the caller. */
void fn_801070CC(const Fn8040Vector3 *position, const UnkRotation3 *rotation, float radius);
}

#endif
