#ifndef GAME_BOMB_H
#define GAME_BOMB_H

#include "Game/CharCommand.h"
#include "Game/CharaColliAttack.h"
#include "Game/ContainerBehavior.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetPickup.h"
#include "Game/PickupCharInfo.h"
#include "Game/Task.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class UnkGadgetOwner_800C9CD0;

namespace Bomb {

/* Bomb::BombBlast (class name from RTTI, with Task as its only base): the blast of a bomb, a probe
 * sphere at a fixed position. Its first update requests sound 0x1008 (fn_801D54F4 plays nothing
 * when its sound setup returns null); after 0.13 seconds, when it has a probe handle, it asks for
 * an attack object for the probe (fn_800C4374 gives one only for a probe that passes its dynamic
 * cast) and grows the probe's radius from 5 to 50 over the next 0.87 seconds, then finishes.
 * Created by fn_800ED9E8 when its allocation succeeds. Method and member names are inferred, and
 * so are the parameters' pointer and const spellings. */
class BombBlast : public Task {
  public:
    /* The values of m_state (names inferred). */
    enum State { STATE_START = 0, STATE_WAITING = 1, STATE_GROWING = 2, STATE_FINISHED = 3 };

    /* Builds the Task under parent, sets the task name to s_name and copies *position to
     * m_position. Creates the sphere probe there (fn_800C7EA8 with 0x40000, 0xF, 0, an empty owner,
     * lbl_805E07A0 and 1) and wraps it in the handle target, starts effect 0x19 at the position
     * with the direction (0, 1, 0) without keeping it, and enters STATE_START with the timer and
     * the radius at 0. */
    BombBlast(Task *parent, const Fn8040Vector3 *position);
    virtual ~BombBlast();
    /* First the state:
     * - STATE_START: sets the timer to 0 and the radius to 5, requests sound 0x1008 at m_position
     *   from fn_801D54F4 (which plays nothing when its sound setup returns null) and enters
     *   STATE_WAITING.
     * - STATE_WAITING: adds dt to the timer; once it is >= 0.13 (a NaN timer never is), enters
     *   STATE_GROWING with the timer at 0 and, when the handle pointer is not null, asks
     *   fn_800C4374 for the probe's attack object (empty unless the probe passes its dynamic
     *   cast) and, when that locks, stores 1 in its byte at 0x54, 3 in its word at 0x38, 4 in its
     *   float at 0x34 and 0x200 in its word at 0x40 and installs OnAttack as its first callback;
     *   then, still only with a handle, passes the radius to the probe's SetSize.
     * - STATE_GROWING: adds dt to the timer and, once it is >= 0.87, sets it to 0.87 and enters
     *   STATE_FINISHED; sets the radius to (50 - 5) / 0.87 * timer + 5, limited to 50 once it is
     *   >= 50, and passes it to the probe's SetSize when the handle pointer is not null.
     * - STATE_FINISHED: sets bit 0 of the task flags (finished).
     * Then, in every state, runs the handle target (fn_8015BE9C) when the handle pointer is not
     * null. */
    virtual void Update(float dt);

    /* The attack object's callback; it does nothing. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x28 */ Fn8040Vector3 m_position;
    /* 0x34 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle; /* of the sphere probe */
    /* 0x3C */ float m_radius;
    /* 0x40 */ float m_timer;
    /* 0x44 */ s32 m_state; /* a State */
};
CHECK_SIZEOF(BombBlast, 0x48);

/* Bomb::Bomb (class name from RTTI, with GadgetPickup::PickupBase, GadgetCharInfo and CharInfo as
 * its bases): the BOMB gadget's bomb, an object that the player picks up, throws or casts, and
 * that bursts by requesting a BombBlast (built only when its allocation succeeds). It keeps a
 * PickupCharInfo of its own (m_charInfo, also named "Bomb") with a target, the model of BOMB.BIN
 * and a ContainerBehavior. Behaviour inferred from the code:
 * - The update after Initialize, ResetFromOwner or ResetTransform (STATE_START) creates
 *   m_behavior, or resets it when it exists, and enters STATE_WAITING, where each update passes
 *   the position, lowered by 10, through m_behavior's SetTransform, Update and GetPosition when
 *   m_behavior is not null, and enters STATE_BURST when m_comId is > 0 and fn_8016904C reports 1
 *   for it.
 * - In STATE_WAITING with FLAG_PICKED clear a PickupCommand (id 0x11) makes it picked up (see
 *   OnCommand); the next update sends the picker a ReadyCommand only when the picker's weak pointer
 *   locks, and enters STATE_HELD either way. In STATE_HELD each update and each call of the grab
 *   position handler place it at the grab matrix (UpdateHeldPosition).
 * - In STATE_HELD a ThrowCommand (0x13) or CastCommand (0x12) starts STATE_FLYING: UpdateMotion
 *   moves it under an acceleration and passes its movement to fn_801BFC28, whose true result
 *   enters STATE_BURST.
 * - In STATE_WAITING an attack whose attack object still locks and has bit 1 of its flags at 0x40
 *   clear enters STATE_BURST (see OnAttack).
 * - Each update in STATE_BURST asks fn_800ED9E8 for a BombBlast at its position (constructed only
 *   when its allocation succeeds), passes the breaker to fn_8020642C with 0x11 when the breaker's
 *   weak pointer locks, and sets FLAG_DONE.
 * - In STATE_WAITING a VacuumCommand (0x14) hands the model to fn_8033F0F8 and enters
 *   STATE_VACUUMED, where each update does the same as STATE_BURST without the BombBlast.
 * GadgetPickup::PickupBaseTask's update calls Update and then TestFlags with bit 1, which reports
 * FLAG_DONE, and finishes the task when it is true. Method, member and constant names are inferred,
 * and so are the parameters' pointer, reference and const spellings. */
class Bomb : public GadgetPickup::PickupBase {
  public:
    /* The values of m_state (names inferred from what each state does). */
    enum State {
        /* Entered by Initialize, ResetFromOwner and ResetTransform. */
        STATE_START = 0,
        STATE_WAITING = 1,
        STATE_PICKED = 2,
        STATE_HELD = 3,
        /* Entered by a ThrowCommand or a CastCommand. */
        STATE_FLYING = 4,
        /* Entered by OnAttack, by UpdateMotion's sweep test and by Update in STATE_WAITING. */
        STATE_BURST = 5,
        /* Entered by a VacuumCommand. */
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

    /* Both constructors build the PickupBase with 0xE and call fn_801DC984 with "Bomb"; the second
     * also copies owner. */
    Bomb();
    Bomb(const boost::weak_ptr<CharInfo> &owner);
    /* When the owner still exists (its weak pointer locks), sends it a new UseCommand naming
     * m_charInfo (a request; the command is null when its allocation fails, and it is sent either
     * way), before the members are destroyed. */
    virtual ~Bomb();

    /* Copies the position and rotation to the bomb's own character info (PickupBase); then, when
     * the model pointer is not null, places the model at position, turned about the y, x and z
     * axes by the rotation. */
    virtual void SetTransform(const Fn8040Vector3 *position, const UnkRotation3 *rotation);
    /* owner is not used. Clears m_flags and sets bit 1 of m_sweepFlags; builds m_charInfo
     * (PickupCharInfo(0xE)), installs OnCommand as its command unit's handler, creates its target
     * (fn_801E4160 with 0x880 and 30), calls its fn_801DC984 with "Bomb", stores 4 in its word at
     * 0x5C and installs OnSetGrabPosition as its grab position handler; loads the model from
     * BOMB.BIN and calls its fn_80162A68(0, 0) and its fn_80161F08 with "col" and m_charInfo; when
     * the attack object of the model's entry 0 locks, clears its byte at 0x54, sets its byte at
     * 0x55 and installs OnAttack as its second callback. Then enters STATE_START, clears m_comId
     * and calls SetTransform with m_charInfo's position and rotation. The attack object is used
     * only when its weak pointer locks; none of the other pointers is tested for null. */
    virtual void Initialize(UnkGadgetOwner_800C9CD0 *owner);
    /* Runs UpdateMotion first while FLAG_MOVING is set (its sweep test can enter STATE_BURST,
     * which the state switch below then handles in the same update), then advances the model when
     * the model pointer is not null, then the state:
     * - STATE_START: when m_behavior is null, creates a ContainerBehavior at m_charInfo's position
     *   with the model (none when the allocation fails) and keeps it in m_behavior; otherwise calls
     *   m_behavior's Reset with m_charInfo's position and rotation. Then enters STATE_WAITING.
     * - STATE_WAITING: when m_behavior is not null, lowers m_charInfo's position by 10, passes it
     *   and the rotation to m_behavior's SetTransform, calls its Update, copies its position back
     *   with GetPosition and raises the position by 10. Then, when m_comId is > 0 and fn_8016904C
     *   reports 1 for it, enters STATE_BURST (handled from the next update on). Last, in either
     *   case, SetTransform with m_charInfo's position and rotation.
     * - STATE_PICKED: sends the picker, when its weak pointer locks, a new ReadyCommand naming
     *   m_charInfo (a request; null when its allocation fails, sent either way), then enters
     *   STATE_HELD.
     * - STATE_HELD: UpdateHeldPosition.
     * - STATE_FLYING: SetTransform with m_charInfo's position and rotation.
     * - STATE_BURST: creates a BombBlast at m_charInfo's position with fn_800ED9E8 (constructed
     *   only when its allocation succeeds), passes the breaker, when its weak pointer locks, to
     *   fn_8020642C with 0x11 and sets FLAG_DONE. It keeps this state, but FLAG_DONE finishes the
     *   task after this update.
     * - STATE_VACUUMED: the same without the BombBlast.
     * Last, in every state, passes m_charInfo's position to its fn_801E413C. */
    virtual void Update(float dt);
    /* When m_charInfo is not null, copies the owner's position (fn_800C9F14) and its rotation
     * converted to radians (fn_800C9ED4) to it, the owner's com id to m_comId (fn_800C9E50) and
     * raises m_charInfo's position by 10. Then enters STATE_START and calls SetTransform with
     * m_charInfo's position and rotation. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When m_charInfo is not null, copies *position and *rotation to it and raises its position by
     * 10. Then enters STATE_START, clears m_comId and calls SetTransform with m_charInfo's position
     * and rotation. */
    virtual void ResetTransform(const Fn8040Vector3 *position, const UnkRotation3 *rotation);
    /* True when bit 0 of flags and FLAG_PICKED are set, or bit 1 of flags and FLAG_DONE. */
    virtual bool TestFlags(const UnkBitFlags_80014E04 &flags);

    /* The descriptor's factory: builds a Bomb in a shared pointer, calls its Initialize and
     * ResetFromOwner with owner and creates a GadgetPickup::SetPickupBaseTask for it under task
     * layer 11 (only when the allocation succeeds; the result is not kept). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* While FLAG_ACCELERATE is set, adds field_0xA4 * field_0xA0 to m_charInfo's acceleration
     * (field_0x24). Advances m_charInfo's position and velocity (field_0x18) by fn_804103E8 with
     * that acceleration, a drag of 1 and dt. While FLAG_SWEEP is set, passes the address of a copy
     * of m_previousPosition, the movement since it, m_sweepFlags and 10 to fn_801BFC28; when the
     * result is true, moves m_charInfo to that copy, sets its velocity to the movement divided by
     * dt and enters STATE_BURST. Then stores m_charInfo's position in m_previousPosition and resets
     * its acceleration with fn_8040CE60. */
    void UpdateMotion(float dt);
    /* The model's attack callback. In STATE_WAITING, when the attack object (param.field_0x0)
     * locks: if bit 1 of its flags at 0x40 is set, stores 5 in param.field_0x54; otherwise keeps
     * its owner as the breaker, enters STATE_BURST and stores 3 in param.field_0x54. In any other
     * state, or when the attack object does not lock, stores 5 in param.field_0x54. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* Moves m_charInfo to the grab matrix's translation, copies its position and rotation to the
     * bomb's own character info (PickupBase::SetTransform) and places the model (without testing
     * the model pointer) with a copy of the grab matrix moved by (0, 10, 0) along its own axes
     * (fn_8040B52C). */
    void UpdateHeldPosition();
    /* m_charInfo's grab position handler: copies *args.m_matrix to m_grabMatrix and, in STATE_HELD,
     * calls UpdateHeldPosition. */
    void OnSetGrabPosition(PickupCharInfo::SetGrabPositionArgs &args);
    /* m_charInfo's command handler; sets bit 1 of every command's flags byte after handling it.
     * - PickupCommand (0x11), in STATE_WAITING with FLAG_PICKED clear: keeps the command's picker,
     *   clears bit 0 of m_charInfo's target flags, calls the model's fn_80161230(0, -1) and
     *   releases m_behavior; enters STATE_PICKED and sets FLAG_PICKED and FLAG_MOVING.
     * - ThrowCommand (0x13) and CastCommand (0x12), in STATE_HELD: copy the command's vector to
     *   field_0xA4, set field_0xA0 to -320, reset m_charInfo's acceleration with fn_8040CE60, raise
     *   its position by 10 and set its velocity to the grab matrix's rotation applied to
     *   (0, 100, -150) for a throw or (0, 100, 0) for a cast; a throw first keeps the command's
     *   character info as the breaker, and sets m_charInfo's rotation from the grab matrix
     *   (fn_800494F8) before the velocity. Both enter STATE_FLYING and set FLAG_ACCELERATE,
     *   FLAG_MOVING and FLAG_SWEEP.
     * - VacuumCommand (0x14), in STATE_WAITING: calls the model's fn_80161880, passes the command,
     *   the model, 0 and the bomb's own position and rotation to fn_8033F0F8, releases the model
     *   and enters STATE_VACUUMED.
     * Other commands, and these ones in other states, only get the flag bit. */
    void OnCommand(CharCommand &command);

    /* 0x70 */ boost::shared_ptr<PickupCharInfo> m_charInfo;
    /* 0x78 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x80 */ boost::shared_ptr<ContainerBehavior> m_behavior;
    /* 0x88 */ s32 m_state;                         /* a State */
    /* 0x8C */ s32 m_comId;                         /* the owner's com id (fn_800C9E50) */
    /* 0x90 */ boost::weak_ptr<CharInfo> m_picker;  /* from the PickupCommand */
    /* 0x98 */ boost::weak_ptr<CharInfo> m_breaker; /* from OnAttack or a ThrowCommand */
    /* 0xA0 */ float field_0xA0;         /* -320 after a throw or cast; scales field_0xA4 */
    /* 0xA4 */ Fn8040Vector3 field_0xA4; /* the vector of the ThrowCommand or CastCommand */
    /* 0xB0 */ Fn8040Vector3 m_previousPosition;
    /* 0xBC */ Fn8040Matrix34 m_grabMatrix;       /* the last matrix of the grab position handler */
    /* 0xEC */ boost::weak_ptr<CharInfo> m_owner; /* from the second constructor */
    /* 0xF4 */ UnkBitFlags_80014E04 m_flags;      /* Flag bits */
    /* 0xF8 */ UnkBitFlags_80014E04 m_sweepFlags; /* passed to fn_801BFC28 */
};
CHECK_SIZEOF(Bomb, 0xFC);

} // namespace Bomb

extern "C" {
/* Creates a Bomb::BombBlast at position under task layer 11 (only when the allocation succeeds).
 * The bomb and fn_8033EE64 are the callers (the parameter's pointer and const spellings are
 * inferred). */
void fn_800ED9E8(const Fn8040Vector3 *position);
}

#endif
