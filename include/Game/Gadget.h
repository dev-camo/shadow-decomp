#ifndef GAME_GADGET_H
#define GAME_GADGET_H

#include "Game/Task.h"
#include "Game/fn_8040_vector_types.h"
#include "layout_check.h"

class UnkGadgetOwner_800C9CD0;

/* Copy the owner's position, its rotation converted from degrees to radians, and its byte at
 * +0x22 (behaviour inferred). */
extern "C" {
void fn_800C9F14(UnkGadgetOwner_800C9CD0 *owner, Fn8040Vector3 *position);
void fn_800C9ED4(UnkGadgetOwner_800C9CD0 *owner, Fn8040Vector3 *rotation);
void fn_800C9E60(UnkGadgetOwner_800C9CD0 *owner, u8 *value);
/* Copies the owner's parameter block into params when its size equals size, and otherwise flags
 * the owner (behaviour inferred). */
void fn_800C9DBC(UnkGadgetOwner_800C9CD0 *owner, void *params, int size);
/* Registers task with the owner when the task is constructed, and releases it when the task is
 * destroyed (behaviour inferred). */
void fn_800C9CD0(UnkGadgetOwner_800C9CD0 *owner, Task *task);
void fn_800C9BE8(UnkGadgetOwner_800C9CD0 *owner);
/* Owner state queries and the call that follows two of them (behaviour inferred; the original
 * names are unknown). Gadget::IsFinished marks the task finished when fn_800C9A64 or fn_800C996C
 * is true. fn_800C9D0C returns false unconditionally in this build. */
bool fn_800C9D0C(UnkGadgetOwner_800C9CD0 *owner);
bool fn_800C9D14(UnkGadgetOwner_800C9CD0 *owner);
void fn_800C9CF8(UnkGadgetOwner_800C9CD0 *owner);
bool fn_800C9A64(UnkGadgetOwner_800C9CD0 *owner);
bool fn_800C996C(UnkGadgetOwner_800C9CD0 *owner);
}

/* Gadget (class name from RTTI): a stage object task registered with an owner object. */
class Gadget : public Task {
  public:
    Gadget(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Gadget();
    /* Runs UpdateBody unless IsFinished reports the gadget done (name inferred). */
    virtual void Update(float dt);
    /* Per-frame work of the concrete gadget (name inferred). */
    virtual void UpdateBody(float dt) = 0;
    virtual void fn_800CBC1C();
    /* Re-reads the owner's settings (name inferred). */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) = 0;
    /* True once the owner reports the gadget finished; also sets Task flag 0x01 (name inferred). */
    virtual bool IsFinished();

    /* 0x28 */ UnkGadgetOwner_800C9CD0 *m_owner;
    /* 0x2C */ u32 field_0x2C;
};
CHECK_SIZEOF(Gadget, 0x30);

#endif
