#ifndef GAME_TASK_H
#define GAME_TASK_H

#include <dolphin/types.h>

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

/* fn_8001C4F8 returns the task manager; fn_801EB304 returns its parent task for new tasks of the
 * given layer (behaviour inferred). */
extern "C" {
void *fn_8001C4F8(void);
Task *fn_801EB304(void *manager, int index);
}

#endif
