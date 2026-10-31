#ifndef GAME_TASK_H
#define GAME_TASK_H

#include <dolphin/types.h>
#include "layout_check.h"

/* Task (class name from RTTI): a node of the game's task tree. Each task links to its parent, its
 * neighbouring siblings and its first child; every constructor stores its class name in m_name. */
class Task {
  public:
    /* 0x00 */ const char *m_name;
    /* 0x04 */ u16 m_flags; /* 0x20 while being destroyed; 0x01 marks a finished task (inferred) */
    /* 0x08 */ Task *m_prev;
    /* 0x0C */ Task *m_next;
    /* 0x10 */ Task *m_parent;
    /* 0x14 */ Task *m_child;

    Task(Task *parent);
    virtual ~Task();
    /* Per-frame entry point (name inferred). */
    virtual void Update(float dt);

    /* 0x1C */ u32 field_0x1C;
    /* 0x20 */ u32 field_0x20;
    /* 0x24 */ u32 field_0x24;
};
CHECK_SIZEOF(Task, 0x28);

/* fn_8001C4F8 returns the task manager (placeholder class name); fn_801EB304 returns its parent
 * task for new tasks of the given layer (behaviour inferred). fn_8001C4F8 constructs a
 * function-local static manager on the first call, registers its destructor and caches its address
 * in a static pointer. The E3 2005 prototype (G9SJ8P) expands this sequence in its callers (213
 * copies), which suggests a header inline accessor. */
class UnkTaskManager_8001C4F8;

extern "C" {
UnkTaskManager_8001C4F8 *fn_8001C4F8(void);
Task *fn_801EB304(UnkTaskManager_8001C4F8 *manager, int index);
}

#endif
