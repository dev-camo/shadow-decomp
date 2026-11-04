#ifndef GAME_BOSSCTRL_H
#define GAME_BOSSCTRL_H

#include "Game/Task.h"
#include "layout_check.h"

class BossVarParam;
class UnkBossCtrlImpl_8017A108;

/* BossCtrl (class name from RTTI, with Task as its only base): a child task that a boss creates for
 * its variables. The constructor (0x80179B3C) allocates the object at 0x28 and passes it the parent,
 * itself and the variables. Update runs the control units through that object. */
class BossCtrl : public Task {
  public:
    BossCtrl(Task *parent, BossVarParam *varParam);
    virtual ~BossCtrl();
    virtual void Update(float dt);

    /* 0x28 */ UnkBossCtrlImpl_8017A108 *field_0x28;
};
CHECK_SIZEOF(BossCtrl, 0x30);

#endif
