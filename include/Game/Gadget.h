#ifndef GAME_GADGET_H
#define GAME_GADGET_H

#include "Game/Task.h"
#include "Game/fn_8040_vector_types.h"
#include "layout_check.h"

class SetAdapterState;
class UnkGadgetOwner_800C9CD0;

/* Copy the owner's position, its rotation converted from degrees to radians, and its byte at
 * +0x22 (behaviour inferred). */
extern "C" {
void fn_800C9F14(UnkGadgetOwner_800C9CD0 *owner, Fn8040Vector3 *position);
void fn_800C9ED4(UnkGadgetOwner_800C9CD0 *owner, Fn8040Vector3 *rotation);
/* CandleStick's parameter SAME_COMID_SWITCH_NUM suggests that the byte at +0x22 is the com id
 * (CandleStick.cpp stores it as m_comId). */
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
/* The writing counterpart of fn_800C9DBC: copies size bytes of params into the owner's parameter
 * block when its size equals size, and otherwise flags the owner (behaviour inferred). */
void fn_800C9D68(UnkGadgetOwner_800C9CD0 *owner, const void *params, int size);
/* Clears bit 0 of the word at 0x18 of the record that owner points to, then calls fn_800C9BE8,
 * fn_800C9B84 and fn_800CB534 (purpose unknown). ThreatObjHolder calls it when its ThreatObj is
 * finished. */
void fn_800C9B34(UnkGadgetOwner_800C9CD0 *owner);
/* Sets bit 3 of the word at 0x18 of the record that owner points to (behaviour inferred from its
 * body). */
void fn_800C985C(UnkGadgetOwner_800C9CD0 *owner);
/* Stores state in the owner's word at 0x14 (behaviour inferred from its body). */
void fn_800C9930(UnkGadgetOwner_800C9CD0 *owner, SetAdapterState *state);
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

/* Passes the gadget's owner (m_owner) to fn_800C9B34 when there is one (behaviour inferred). */
extern "C" void fn_8015A794(Gadget *gadget);

#endif
