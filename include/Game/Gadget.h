#ifndef GAME_GADGET_H
#define GAME_GADGET_H

#include "Game/Task.h"

class UnkGadgetOwner_800C9CD0;

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

#endif
