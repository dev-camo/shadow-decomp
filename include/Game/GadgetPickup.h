#ifndef GAME_GADGETPICKUP_H
#define GAME_GADGETPICKUP_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/Task.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class UnkGadgetOwner_800C9CD0;

namespace GadgetPickup {

/* GadgetPickup::PickupBase (class name from RTTI, with GadgetCharInfo and CharInfo as its bases):
 * the base of the objects that the player can pick up, such as the HEALING_UNIT gadget's unit. Its
 * vtable is at 0x8052B62C; the constructor and destructor are inline in the code of the derived
 * classes, and the destructor's out-of-line copy is at 0x800F0C7C. The class is 0x70 bytes, 4 more
 * than GadgetCharInfo: the HEALING_UNIT unit's members start at 0x70, and so do those of every
 * other constructor that stores this vtable (0x800F0BD4, 0x800F0CD4, 0x800F8910, 0x800FB98C,
 * 0x800FEBEC and 0x8027A1CC each build a shared_ptr<PickupCharInfo> at 0x70). The inline
 * constructor does not initialise those 4 bytes and what uses them is unknown. The virtual
 * functions after CharInfo's are named after what the HEALING_UNIT unit's overrides do and where
 * GadgetPickup's tasks call them (names inferred); the parameter types and their pointer, reference
 * and const spellings are inferred too. */
class PickupBase : public GadgetCharInfo {
  public:
    PickupBase(int arg0) : GadgetCharInfo(arg0) {}
    virtual ~PickupBase() {}

    /* Copies position to the character info's position and rotation to its rotation (0x801689FC,
     * behaviour from its body). */
    virtual void SetTransform(const Fn8040Vector3 *position, const UnkRotation3 *rotation);
    /* The HEALING_UNIT gadget's Create passes its owner and the unit factories pass null. This
     * implementation (0x800EDB38) does nothing. */
    virtual void Initialize(UnkGadgetOwner_800C9CD0 *owner) {}
    /* Called by PickupBaseTask's update with the update's dt. */
    virtual void Update(float dt) = 0;
    /* The HEALING_UNIT gadget's Create calls it with its owner after Initialize. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) = 0;
    /* The unit factories call it with a position and a rotation after Initialize. This
     * implementation (0x800EDB34) does nothing. */
    virtual void ResetTransform(const Fn8040Vector3 *position, const UnkRotation3 *rotation) {}
    /* PickupBaseTask's update passes flags with only bit 1 set and finishes the task when the
     * result is true. */
    virtual bool TestFlags(const UnkBitFlags_80014E04 &flags) = 0;

    /* 0x6C */ u8 field_0x6C[0x70 - 0x6C];
};
CHECK_SIZEOF(PickupBase, 0x70);

/* GadgetPickup::PickupBaseTask (class name from RTTI, with Task as its only base): the task that
 * updates a pickup object. Its constructor (0x80168944) builds the Task under parent, copies unit,
 * zeroes m_radiusSq and points the task's name at the characters at 0x3C of the object. Its update
 * (0x80168834) calls the object's Update with dt, then finishes the task when the object's
 * TestFlags, given flags with only bit 1 set, reports true, and also, only when m_radiusSq > 0
 * (0, a negative value and a NaN skip it), when fn_80169BE4 reports true for the object
 * (behaviour inferred). */
class PickupBaseTask : public Task {
  public:
    PickupBaseTask(Task *parent, const boost::shared_ptr<PickupBase> &unit);
    /* Defined elsewhere (0x801688DC). */
    virtual ~PickupBaseTask();
    /* Defined elsewhere (0x80168834). */
    virtual void Update(float dt);

    /* Stores radius * radius in m_radiusSq (0x80168798, behaviour from its body; name inferred). */
    void SetRadius(float radius);

    /* 0x28 */ boost::shared_ptr<PickupBase> m_unit;
    /* 0x30 */ float m_radiusSq;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(PickupBaseTask, 0x38);

/* GadgetPickup::SetPickupBaseTask (class name from RTTI, with Gadget and Task as its bases): the
 * gadget that a pickup object's descriptor creates. Its constructor (0x80168710) builds the Gadget
 * with parent and owner, creates a PickupBaseTask with no parent for unit (none when the allocation
 * fails) and stores it in m_task, then calls ResetFromOwner with owner (behaviour inferred). */
class SetPickupBaseTask : public Gadget {
  public:
    SetPickupBaseTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner,
                      const boost::shared_ptr<PickupBase> &unit);
    /* Defined elsewhere (0x8016867C). */
    virtual ~SetPickupBaseTask();
    /* Defined elsewhere (0x801685F0). */
    virtual void UpdateBody(float dt);
    /* Defined elsewhere (0x80168584). */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* 0x30 */ PickupBaseTask *m_task;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(SetPickupBaseTask, 0x38);

} // namespace GadgetPickup

#endif
