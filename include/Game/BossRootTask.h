#ifndef GAME_BOSSROOTTASK_H
#define GAME_BOSSROOTTASK_H

#include "Game/EffectLaunchInfoUSA.h"
#include "Game/Task.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class BossVarParam;
class UnkGadgetOwner_800C9CD0;

/* BossRootTask (class name from RTTI, with Task as its only base): the task of a boss. Like a
 * Gadget it registers with an owner object; it also keeps a pointer to the boss's variables, which
 * the derived class owns. The layout comes from its constructor (0x8027E028). */
class BossRootTask : public Task {
  public:
    BossRootTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner, bool flag, BossVarParam *varParam);
    virtual ~BossRootTask();
    /* Calls Init and sets m_initialized once the resources of key 0x16 are loaded and a second
     * manager reports state 3; from then on, starting in the same frame, it updates the variables
     * from the owner (behaviour inferred). */
    virtual void Update(float dt);
    /* Sets up the boss once its resources are loaded (name inferred). */
    virtual void Init() = 0;

    /* Copies the owner's position and rotation into the character info of the variables and wraps
     * the three angles (name inferred). */
    void ReadOwner();

    /* 0x28 */ UnkGadgetOwner_800C9CD0 *m_owner;
    /* 0x2C */ BossVarParam *m_param;
    /* 0x30 */ bool m_initialized; /* name inferred */
    /* 0x31 */ bool field_0x31;    /* the constructor's flag */
};
CHECK_SIZEOF(BossRootTask, 0x38);

/* The effect launcher shared by the boss tasks (behaviour inferred): the BossRootTask file
 * constructs it at startup, every boss's load hook sets it from fn_801D2CA4(3), and its Init copies
 * it into BossVarParam::field_0x6C. */
extern boost::shared_ptr<SonicteamUSA::System::Effect::EffectLaunchInfoUSA> lbl_805F0240;

#endif
