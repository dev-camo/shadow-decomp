#ifndef GAME_CONTAINERSECRET_H
#define GAME_CONTAINERSECRET_H

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

class Target;
class UnkGadgetOwner_800C9CD0;

namespace Container {

/* Container::ContainerSecret (class name from RTTI, with GadgetPickup::PickupBase, GadgetCharInfo
 * and CharInfo as its bases): the SECRET CONTAINER gadget's container. It works like the CONTAINER
 * gadget's Container::ContainerNormal (see Game/Container.h), whose states, flags and commands it
 * shares, with these differences (behaviour inferred from the code):
 * - Its model comes from the file CONTAINERSECRET.BIN and Initialize loads it once; it has no
 *   model parameter, so ResetFromOwner never reloads the model.
 * - Its one parameter is "weapon" (m_weapon). It passes MODEL_INDEX where ContainerNormal passes
 *   its "model" parameter (fn_8027B338, fn_80254384 and fn_8033E644), SELECT_SECRET and m_weapon
 *   where ContainerNormal passes its "select" and "value" parameters (fn_8027B338, fn_80254384
 *   and fn_8033E828), and EVENT_ID where ContainerNormal passes an id that depends on its model
 *   (fn_8027B338, fn_80254384 and fn_8020642C).
 * - A PickupCommand only keeps the picker; it does not record the command's vectors and float.
 * - A ThrowCommand does not call the model's fn_80161880 before it hands the model to
 *   fn_8027B338; the update in STATE_BROKEN calls it before SpawnDebris.
 * The methods that Container.h describes for ContainerNormal do the same here unless their
 * comments below say otherwise. Method, member and constant names are inferred, and so are the
 * parameters' pointer, reference and const spellings. */
class ContainerSecret : public GadgetPickup::PickupBase {
  public:
    /* The values of m_state (names inferred from what each state does; the same values as
     * ContainerNormal's). */
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

    /* The bits of m_flags (names inferred; the same bits as ContainerNormal's). Initialize clears
     * them all; nothing else clears one. */
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

    /* What the container passes where ContainerNormal passes its "model" and "select" parameters
     * and its event id (names inferred from those parameters and from this class's use; none is
     * a recovered label). */
    enum {
        MODEL_INDEX = 0,
        /* The value after ContainerNormal's six "select" labels (0 to 5); for it fn_8033E828
         * passes the position, the rotation, the vector (0, 1, 0) and the value to fn_8033E9A0
         * (see Game/fn_8033E644.h). */
        SELECT_SECRET = 6,
        EVENT_ID = 0x10
    };

    /* Builds the PickupBase with 4, zeroes m_weapon and calls fn_801DC984 with
     * "ContainerSecret". */
    ContainerSecret();
    virtual ~ContainerSecret();

    /* As ContainerNormal::SetTransform. */
    virtual void SetTransform(const Fn8040Vector3 *position, const UnkRotation3 *rotation);
    /* Clears m_flags and sets bit 1 of m_sweepFlags; builds m_charInfo (PickupCharInfo(4)),
     * installs OnCommand as its command unit's handler, creates its target (fn_801E4160 with 0x80
     * and 30), calls its fn_801DC984 with "ContainerSecret", stores 4 in its word at 0x5C and
     * installs OnSetGrabPosition as its grab position handler. Then it loads the model from
     * CONTAINERSECRET.BIN (fn_8015F800 with 0), calls its fn_80162A68 with MODEL_INDEX and 1 and
     * its fn_80161FC8 with 0 and m_charInfo and, when the attack object of the model's part 0
     * locks, clears its byte at 0x54, sets its byte at 0x55 and installs OnAttack as its second
     * callback. Last it enters STATE_START and calls SetTransform with m_charInfo's position and
     * rotation. It does not use owner, and it does not test the model or m_charInfo for null. */
    virtual void Initialize(UnkGadgetOwner_800C9CD0 *owner);
    /* As ContainerNormal::Update, except STATE_BROKEN: it calls the model's fn_80161880 (without
     * testing the model pointer) and SpawnDebris, passes SELECT_SECRET, m_weapon and m_charInfo's
     * position and rotation to fn_8033E828 and the breaker, when its weak pointer locks, to
     * fn_8020642C with EVENT_ID, then sets FLAG_DONE. It stays in this state. */
    virtual void Update(float dt);
    /* When m_charInfo is not null, copies the owner's position (fn_800C9F14) and its rotation
     * converted to radians (fn_800C9ED4) to it, copies the owner's parameter into m_weapon
     * (fn_800C9DBC: only when the owner's parameter block is 4 bytes) and raises the position by
     * 10. Then calls SetTransform with m_charInfo's position and rotation, without testing
     * m_charInfo, and enters STATE_START. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* True when bit 0 of flags and FLAG_PICKED are set, or bit 1 of flags and FLAG_DONE. */
    virtual bool TestFlags(const UnkBitFlags_80014E04 &flags);

    /* The descriptor's factory: builds a ContainerSecret in a shared pointer, calls its Initialize
     * and ResetFromOwner with owner and creates a GadgetPickup::SetPickupBaseTask for it under
     * task layer 11 (only when the allocation succeeds; the result is not kept). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* As ContainerNormal::UpdateMotion. */
    void UpdateMotion(float dt);
    /* The model's attack callback; as ContainerNormal::OnAttack. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* Passes a copy of the model pointer, MODEL_INDEX and m_charInfo's position and rotation to
     * fn_8033E644. */
    void SpawnDebris();
    /* As ContainerNormal::UpdateHeldPosition. */
    void UpdateHeldPosition();
    /* m_charInfo's grab position handler; as ContainerNormal::OnSetGrabPosition. */
    void OnSetGrabPosition(PickupCharInfo::SetGrabPositionArgs &args);
    /* m_charInfo's command handler; sets bit 1 of every command's flags byte after handling it.
     * The CastCommand (0x12) and SetTransparentCommand (0x17) cases are ContainerNormal's. The
     * others:
     * - PickupCommand (0x11), in STATE_WAITING with FLAG_PICKED clear: keeps the command's picker,
     *   clears bit 0 of m_charInfo's target flags, calls the model's fn_80161230(0, -1) and
     *   releases m_behavior; enters STATE_PICKED and sets FLAG_MOVING and FLAG_PICKED.
     * - ThrowCommand (0x13), in STATE_HELD: keeps the command's character info as the breaker and
     *   copies its vector to field_0xAC, sets field_0xA8 to -320, resets m_charInfo's acceleration
     *   with fn_8040CE60, raises its position by 10 and sets its velocity to the grab matrix's
     *   rotation applied to (0, 100, -150); takes the grab matrix's angles (fn_800494F8) and passes
     *   the thrower, the model, MODEL_INDEX, SELECT_SECRET, m_weapon, m_charInfo's position, the
     *   angles, m_charInfo's velocity, field_0xAC * field_0xA8, EVENT_ID and 0, 1 and 600 to
     *   fn_8027B338; then releases the model and enters STATE_THROWN.
     * - VacuumCommand (0x14), in STATE_WAITING: calls the model's fn_80161880, passes the command,
     *   the model, MODEL_INDEX, SELECT_SECRET, m_weapon, the container's own position and rotation
     *   and EVENT_ID to fn_80254384, releases the model and enters STATE_VACUUMED.
     * Other commands, and these ones in other states, only get the flag bit. */
    void OnCommand(CharCommand &command);

    /* 0x70 */ boost::shared_ptr<PickupCharInfo> m_charInfo;
    /* 0x78 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x80 */ boost::shared_ptr<ContainerBehavior> m_behavior;
    /* Only the constructor and the destructor touch it; nothing in this file assigns it. */
    /* 0x88 */ boost::shared_ptr<Target> field_0x88;
    /* 0x90 */ s32 m_state; /* a State */
    /* The gadget's one parameter, "weapon" (name from its parameter table): a weapon type, which
     * the table's label callback names with fn_8005BEEC when fn_8005BF14 accepts it. The table
     * entry's field_0x10, an integer limit by inference (see Game/GadgetTypeInfo.h), is 0x47. */
    /* 0x94 */ s32 m_weapon;
    /* 0x98 */ boost::weak_ptr<CharInfo> m_picker;  /* from the PickupCommand */
    /* 0xA0 */ boost::weak_ptr<CharInfo> m_breaker; /* from OnAttack or a ThrowCommand */
    /* 0xA8 */ float field_0xA8;         /* -320 after a throw or cast; scales field_0xAC */
    /* 0xAC */ Fn8040Vector3 field_0xAC; /* the vector of the last of those commands */
    /* 0xB8 */ Fn8040Vector3 m_previousPosition;
    /* 0xC4 */ Fn8040Matrix34 m_grabMatrix;       /* the last matrix of the grab position handler */
    /* 0xF4 */ UnkBitFlags_80014E04 m_flags;      /* Flag bits */
    /* 0xF8 */ UnkBitFlags_80014E04 m_sweepFlags; /* passed to fn_801BFC28 */
};
CHECK_SIZEOF(ContainerSecret, 0xFC);

} // namespace Container

#endif
