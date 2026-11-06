#ifndef GAME_CONTAINER_H
#define GAME_CONTAINER_H

#include "Game/CharCommand.h"
#include "Game/CharaColliAttack.h"
#include "Game/ContainerBehavior.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetPickup.h"
#include "Game/PickupCharInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class UnkGadgetOwner_800C9CD0;

namespace Container {

/* The CONTAINER gadget's parameters, in the order of its parameter table ("model", "select",
 * "value"; member names from the table). */
struct ContainerParams {
    ContainerParams() : model(0), select(0), value(0) {}

    /* The model's index; the table's limit is 2. */
    /* 0x0 */ s32 model;
    /* What the container holds; the labels are "None", "Item", "Weapon", "Ring", "HealUnit" and
     * "EnergyCore". */
    /* 0x4 */ s32 select;
    /* 0x8 */ s32 value; /* labelled by the item or weapon type, "Ring Num", "Hero" or "Dark" */
};
CHECK_SIZEOF(ContainerParams, 0xC);

/* Container::ContainerNormal (class name from RTTI, with GadgetPickup::PickupBase, GadgetCharInfo
 * and CharInfo as its bases): the CONTAINER gadget's container, an object that the player picks
 * up, throws or casts, or breaks with an attack. It keeps a PickupCharInfo of its own (m_charInfo,
 * also named "ContainerNormal") with a target, the model of the resource 0xCA and a
 * ContainerBehavior. Behaviour inferred from the code:
 * - The update after Initialize or ResetFromOwner (STATE_START) creates m_behavior, or resets it
 *   when it exists, and enters STATE_WAITING, where each update passes the position, lowered by
 *   10, through m_behavior's SetTransform, Update and GetPosition when m_behavior is not null.
 * - In STATE_WAITING with FLAG_PICKED clear a PickupCommand (id 0x11) makes it picked up (see
 *   OnCommand); the next update sends the picker a ReadyCommand only when the picker's weak pointer
 *   locks, and enters STATE_HELD either way. In STATE_HELD each update and each call of the grab
 *   position handler place it at the grab matrix (UpdateHeldPosition).
 * - In STATE_HELD a ThrowCommand (0x13) passes the model and the parameters to fn_8027B338,
 *   releases the model and enters STATE_THROWN; a CastCommand (0x12) starts STATE_FLYING, where
 *   UpdateMotion moves it and its sweep test, when fn_801BFC28 reports true, enters
 *   STATE_BROKEN.
 * - In STATE_WAITING an attack whose attack object still locks and has bit 1 of its flags at 0x40
 *   clear enters STATE_BROKEN (see OnAttack); the update in STATE_BROKEN calls SpawnDebris
 *   (fn_8033E644, which uses the model's entries only when the model is not null) and
 *   SpawnContents (fn_8033E828, which creates nothing for select 0 and lowers the position by 10
 *   for select 4), then sets FLAG_DONE (see Update).
 * - In STATE_WAITING a VacuumCommand (0x14) hands the model to fn_80254384 and enters
 *   STATE_VACUUMED.
 * GadgetPickup::PickupBaseTask's update calls Update and then TestFlags with bit 1, which reports
 * FLAG_DONE, and finishes the task when it is true. Method, member and constant names are inferred,
 * and so are the parameters' pointer, reference and const spellings. */
class ContainerNormal : public GadgetPickup::PickupBase {
  public:
    /* The values of m_state (names inferred from what each state does). */
    enum State {
        /* Entered by Initialize and ResetFromOwner. */
        STATE_START = 0,
        STATE_WAITING = 1,
        STATE_PICKED = 2,
        STATE_HELD = 3,
        /* Entered by a ThrowCommand; each update sets FLAG_DONE. */
        STATE_THROWN = 4,
        /* Entered by a CastCommand. */
        STATE_FLYING = 5,
        /* Entered by a VacuumCommand; each update sets FLAG_DONE. */
        STATE_VACUUMED = 6,
        /* Entered by OnAttack and by UpdateMotion's sweep test. */
        STATE_BROKEN = 7
    };

    /* The bits of m_flags (names inferred). Initialize clears them all; nothing else clears
     * one. */
    enum Flag {
        /* Set by a PickupCommand and a CastCommand; while it is set each update starts with
         * UpdateMotion. */
        FLAG_MOVING = 0,
        /* Set by a PickupCommand; while it is set a PickupCommand is ignored. */
        FLAG_PICKED = 1,
        /* Set by every update in STATE_THROWN, STATE_VACUUMED and STATE_BROKEN. */
        FLAG_DONE = 2,
        /* Set by a CastCommand: UpdateMotion adds the acceleration. */
        FLAG_ACCELERATE = 3,
        /* Set by a CastCommand: UpdateMotion tests the movement. */
        FLAG_SWEEP = 4
    };

    /* Builds the PickupBase with 4 and calls fn_801DC984 with "ContainerNormal". */
    ContainerNormal();
    virtual ~ContainerNormal();

    /* Copies the position and rotation to the container's own character info (PickupBase); when
     * the model pointer is not null, places the model at position, turned about the y, x and z axes
     * by the rotation; then, when m_behavior is not null, passes position and rotation to its
     * SetTransform. */
    virtual void SetTransform(const Fn8040Vector3 *position, const UnkRotation3 *rotation);
    /* Clears m_flags, sets bit 1 of m_sweepFlags and copies the owner's parameters into m_params
     * (fn_800C9DBC: only when the owner's parameter block is 0xC bytes); builds m_charInfo
     * (PickupCharInfo(4)), installs OnCommand as its command unit's handler, creates its target
     * (fn_801E4160 with 0x80 and 30), calls its fn_801DC984 with "ContainerNormal", stores 4 in
     * its word at 0x5C and installs OnSetGrabPosition as its grab position handler; then loads the
     * model of m_params.model (LoadModel), enters STATE_START and calls SetTransform with
     * m_charInfo's position and rotation. None of the pointers is tested for null. */
    virtual void Initialize(UnkGadgetOwner_800C9CD0 *owner);
    /* Runs UpdateMotion first while FLAG_MOVING is set (its sweep test can enter STATE_BROKEN,
     * which the state switch below then handles in the same update), then advances the model when
     * the model pointer is not null, then the state:
     * - STATE_START: when m_behavior is null, creates a ContainerBehavior at m_charInfo's position
     *   with the model (none when the allocation fails) and keeps it in m_behavior; otherwise calls
     *   m_behavior's Reset with m_charInfo's position and rotation. Then enters STATE_WAITING.
     * - STATE_WAITING: when m_behavior is not null, lowers m_charInfo's position by 10, passes it
     *   and the rotation to m_behavior's SetTransform, calls its Update, copies its position back
     *   with GetPosition and raises the position by 10; then, whether m_behavior is null or not,
     *   SetTransform with m_charInfo's position and rotation.
     * - STATE_PICKED: sends the picker, when its weak pointer locks, a new ReadyCommand naming
     *   m_charInfo (a request; null when its allocation fails, sent either way), then enters
     *   STATE_HELD.
     * - STATE_HELD: UpdateHeldPosition.
     * - STATE_THROWN and STATE_VACUUMED: set FLAG_DONE.
     * - STATE_FLYING: SetTransform with m_charInfo's position and rotation.
     * - STATE_BROKEN: SpawnDebris and SpawnContents; passes the breaker, when its weak pointer
     *   locks, to fn_8020642C with GetEventId(); sets FLAG_DONE. It stays in this state.
     * Last, in every state, passes m_charInfo's position to its fn_801E413C. */
    virtual void Update(float dt);
    /* When m_charInfo is not null, copies the owner's position (fn_800C9F14) and its rotation
     * converted to radians (fn_800C9ED4) to it and raises the position by 10. Copies the owner's
     * parameters into m_params (fn_800C9DBC: only when the owner's parameter block is 0xC bytes);
     * when m_params.model then differs from its value before the copy, calls ReleaseModel and
     * loads the model of the new m_params.model. Then calls SetTransform with m_charInfo's position
     * and rotation and enters STATE_START. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* True when bit 0 of flags and FLAG_PICKED are set, or bit 1 of flags and FLAG_DONE. */
    virtual bool TestFlags(const UnkBitFlags_80014E04 &flags);

    /* The descriptor's factory: builds a ContainerNormal in a shared pointer, calls its Initialize
     * and ResetFromOwner with owner and creates a GadgetPickup::SetPickupBaseTask for it under
     * task layer 11 (only when the allocation succeeds; the result is not kept). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* The id that the container passes to fn_8020642C when it breaks and to fn_8027B338 and
     * fn_80254384: 9, 0xB or 0xD for the models 0, 1 and 2, and 0x27 for any other model (for
     * which fn_8020642C returns 0). */
    int GetEventId() const {
        switch (m_params.model) {
        case 0:
            return 9;
        case 1:
            return 0xB;
        case 2:
            return 0xD;
        default:
            return 0x27;
        }
    }

    /* Releases m_behavior and, when the model pointer is not null, calls the model's fn_80161880
     * and fn_80162804. */
    void ReleaseModel();
    /* Loads the model from the resource 0xCA (fn_8015F9D8) and calls its fn_80162A68 with index
     * and 1 and its fn_80161F08 with "col" and m_charInfo; when the attack object of the model's
     * entry 0 locks, clears its byte at 0x54, sets its byte at 0x55 and installs OnAttack as its
     * second callback. */
    void LoadModel(int index);
    /* Passes m_params.select, m_params.value and m_charInfo's position and rotation to
     * fn_8033E828. */
    void SpawnContents();
    /* While FLAG_ACCELERATE is set, adds field_0xAC * field_0xA8 to m_charInfo's acceleration
     * (field_0x24). Advances m_charInfo's position and velocity (field_0x18) by fn_804103E8 with
     * that acceleration, a drag of 1 and dt. While FLAG_SWEEP is set, passes the address of a copy
     * of m_previousPosition, the movement since it, m_sweepFlags and 10 to fn_801BFC28; when the
     * result is true, moves m_charInfo to that copy, sets its velocity to the movement divided by
     * dt and enters STATE_BROKEN. Then stores m_charInfo's position in m_previousPosition and
     * resets its acceleration with fn_8040CE60. */
    void UpdateMotion(float dt);
    /* The model's attack callback. In STATE_WAITING, when the attack object (param.field_0x0)
     * locks: if bit 1 of its flags at 0x40 is set, stores 5 in param.field_0x54; otherwise keeps
     * its owner as the breaker, enters STATE_BROKEN and stores 3 in param.field_0x54. In any other
     * state, or when the attack object does not lock, stores 5 in param.field_0x54. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* Passes a copy of the model pointer, m_params.model and m_charInfo's position and rotation to
     * fn_8033E644. */
    void SpawnDebris();
    /* Moves m_charInfo to the grab matrix's translation, copies its position and rotation to the
     * container's own character info (PickupBase::SetTransform) and places the model (without
     * testing the model pointer) with a copy of the grab matrix moved by (0, 10, 0) along its own
     * axes (fn_8040B52C). */
    void UpdateHeldPosition();
    /* m_charInfo's grab position handler: copies *args.m_matrix to m_grabMatrix and, in STATE_HELD,
     * calls UpdateHeldPosition. */
    void OnSetGrabPosition(PickupCharInfo::SetGrabPositionArgs &args);
    /* m_charInfo's command handler; sets bit 1 of every command's flags byte after handling it.
     * - PickupCommand (0x11), in STATE_WAITING with FLAG_PICKED clear: clears field_0x10C, copies
     *   the command's field_0x2C to field_0xAC, sets field_0xA8 to -320, copies the command's
     *   float to field_0x110 and m_charInfo's position to field_0xC4, stores the command's
     *   field_0x14 + field_0x2C * 10 in field_0xD0, keeps the command's picker, clears bit 0 of
     *   m_charInfo's target flags, calls the model's fn_80161230(0, -1) and releases m_behavior;
     *   enters STATE_PICKED and sets FLAG_PICKED and FLAG_MOVING.
     * - ThrowCommand (0x13), in STATE_HELD: keeps the command's character info as the breaker and
     *   copies its vector to field_0xAC, sets field_0xA8 to -320, resets m_charInfo's acceleration
     *   with fn_8040CE60, raises its position by 10 and sets its velocity to the grab matrix's
     *   rotation applied to (0, 100, -150); calls the model's fn_80161880, takes the grab matrix's
     *   angles (fn_800494F8) and passes the thrower, the model, the parameters, m_charInfo's
     *   position, the angles, m_charInfo's velocity, field_0xAC * field_0xA8 and GetEventId() to
     *   fn_8027B338; then releases the model and enters STATE_THROWN.
     * - CastCommand (0x12), in STATE_HELD: copies the command's vector to field_0xAC, sets
     *   field_0xA8 to -320, resets m_charInfo's acceleration, raises its position by 10 and sets
     *   its velocity to the grab matrix's rotation applied to (0, 100, 0); enters STATE_FLYING and
     *   sets FLAG_ACCELERATE, FLAG_MOVING and FLAG_SWEEP.
     * - VacuumCommand (0x14), in STATE_WAITING: calls the model's fn_80161880, passes the command,
     *   the model, the parameters, the container's own position and rotation and GetEventId() to
     *   fn_80254384, releases the model and enters STATE_VACUUMED.
     * - SetTransparentCommand (0x17), in any state: when its alpha is < 1, passes it to
     *   fn_8015FDD8, otherwise (also for a NaN) calls fn_8015FD10; either only when the model
     *   pointer is not null.
     * Other commands, and these ones in other states, only get the flag bit. */
    void OnCommand(CharCommand &command);

    /* 0x70 */ boost::shared_ptr<PickupCharInfo> m_charInfo;
    /* 0x78 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x80 */ boost::shared_ptr<ContainerBehavior> m_behavior;
    /* 0x88 */ s32 m_modelIndex; /* the m_params.model last passed to LoadModel */
    /* 0x8C */ ContainerParams m_params;
    /* 0x98 */ boost::weak_ptr<CharInfo> m_breaker; /* from OnAttack or a ThrowCommand */
    /* 0xA0 */ boost::weak_ptr<CharInfo> m_picker;  /* from the PickupCommand */
    /* 0xA8 */ float field_0xA8;         /* -320 after a pickup, throw or cast; scales field_0xAC */
    /* 0xAC */ Fn8040Vector3 field_0xAC; /* the vector of the last of those commands */
    /* 0xB8 */ Fn8040Vector3 m_previousPosition;
    /* 0xC4 */ Fn8040Vector3 field_0xC4;      /* set by a PickupCommand; nothing here reads it */
    /* 0xD0 */ Fn8040Vector3 field_0xD0;      /* set by a PickupCommand; nothing here reads it */
    /* 0xDC */ Fn8040Matrix34 m_grabMatrix;   /* the last matrix of the grab position handler */
    /* 0x10C */ float field_0x10C;            /* set by a PickupCommand; nothing here reads it */
    /* 0x110 */ float field_0x110;            /* set by a PickupCommand; nothing here reads it */
    /* 0x114 */ UnkBitFlags_80014E04 m_flags; /* Flag bits */
    /* 0x118 */ UnkBitFlags_80014E04 m_sweepFlags; /* passed to fn_801BFC28 */
    /* 0x11C */ s32 m_state;                       /* a State */
};
CHECK_SIZEOF(ContainerNormal, 0x120);

} // namespace Container

#endif
