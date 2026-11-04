#ifndef GAME_ENEMYTEMPLATE_H
#define GAME_ENEMYTEMPLATE_H

#include "Game/TEnemySetTask.h"

class UnkGadgetOwner_800C9CD0;

/* Original RTTI name (e.g. EnemyTemplate<BkWingSmallBase, BkWingSmallAI>): joins an enemy body,
 * its AI and the task that runs it. Every instantiation's constructor points the shared
 * EnemyReferer at the enemy and makes the AI its control component. */
template <class Base, class AI> class EnemyTemplate : public Base, public AI, public TEnemySetTask {
  public:
    EnemyTemplate(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
        : Base(), AI(), TEnemySetTask(parent, owner) {
        m_enemy = this;
        m_control = this;
    }
    virtual ~EnemyTemplate() {}
    virtual int fn_8017D788() {
        return TEnemySetTask::field_0x2C;
    }
};

#endif
