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
    /* Stores owner in m_owner and registers the task with it (fn_800C9CD0) only when owner is not
     * null; when it is null, the constructor does not write m_owner. */
    BossRootTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner, bool flag, BossVarParam *varParam);
    /* When m_owner is not null, releases the task from it (fn_800C9BE8) and clears m_owner. */
    virtual ~BossRootTask();
    /* Until m_initialized is set, it waits for fn_80012408 to give 3 for resource key 0x16 (the key
     * under which the recovered boss files register their .one file) and, tested only then, for
     * the word at 0x8 of fn_80074470's manager to be 3; it then calls Init and goes on in the same
     * call. With an owner, it requests mode 2 when the current mode is 1 (field_0x14 and
     * field_0xC of the variables, see BossVarParam.h) and sets Task flag 0x01 when fn_800C9A64 is
     * true. Its branch for fn_800C9D0C being true never runs, because fn_800C9D0C returns false
     * unconditionally in this build (see Gadget.h), so Update never calls ReadOwner. */
    virtual void Update(float dt);
    /* Sets up the boss once its resources are loaded (name inferred). */
    virtual void Init() = 0;

    /* Copies the owner's position and rotation into the character info of the variables
     * (fn_800C9F14, fn_800C9ED4) and passes each of the three angles through fn_8040E99C (name
     * inferred). It does not test m_owner. Update's call to it is in the branch that never runs;
     * the Init of each recovered boss file calls it without a test. */
    void ReadOwner();

    /* The class name the constructor stores in m_name (name inferred). */
    static const char *s_name;

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
