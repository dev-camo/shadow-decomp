#ifndef GAME_EGGLASTMECHTASK_H
#define GAME_EGGLASTMECHTASK_H

#include "Game/EggLastMech.h"
#include "Game/Task.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class UnkGadgetOwner_800C9CD0;

namespace EggLastMech {

/* EggLastMech::EggLastMechTask (class name from RTTI): the task of the EGG_LASTMECH gadget, which
 * owns the boss. */
class EggLastMechTask : public Task {
  public:
    EggLastMechTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~EggLastMechTask();
    virtual void Update(float dt);

    /* Ends the boss and marks the task finished (name inferred). */
    void Finish();

    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    static const char *s_name;

    /* 0x28 */ boost::shared_ptr<EggLastMech> m_mech;
    /* 0x30 */ UnkGadgetOwner_800C9CD0 *m_owner;
};
CHECK_SIZEOF(EggLastMechTask, 0x38);

} // namespace EggLastMech

extern "C" {
/* The gadget's load and unload hooks (EggLastMechTask.cpp), named in its descriptor. */
void fn_802FDF30(void);
void fn_802FDF04(void);

/* fn_8032D9B8 is called by the gadget's load hook and fn_8032D970 by its unload hook; both are
 * defined in a file that has no header yet (placeholder names, types inferred). */
void fn_8032D9B8(void);
void fn_8032D970(void);
}

#endif
