#ifndef GAME_TENEMYSETTASK_H
#define GAME_TENEMYSETTASK_H

#include "Game/Task.h"
#include "Game/EnemyReferer.h"
#include "layout_check.h"

class UnkGadgetOwner_800C9CD0;

/* Original RTTI name. The task that runs a placed enemy; it replaces both Task slots. */
class TEnemySetTask : public virtual EnemyReferer, public Task {
  public:
    TEnemySetTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~TEnemySetTask();
    virtual void Update(float dt);

    /* 0x2C */ int field_0x2C;
};
CHECK_SIZEOF(TEnemySetTask, 0x38);

#endif
