#ifndef GAME_SAVEPOINTSELECT_H
#define GAME_SAVEPOINTSELECT_H

#include "Game/Task.h"
#include "boost/function.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

/* The 0x84-byte object that the SavePointSelectTask constructor creates (placeholder name after
 * its constructor; nothing about it is mapped). */
class UnkObject_80331E94;

namespace SavePointSelect {

/* SavePointSelect::Args (name from the RTTI name of the SAVEPOINT gadget's callback binder): the
 * argument of the task's two callbacks. Only the word at 0x0 is mapped: the SAVEPOINT gadget's
 * selection callback does nothing with it when it is -1 and otherwise passes it to fn_8016F344 as
 * an index (meaning inferred). The complete extent is unknown; the check covers the declared view
 * only. */
struct Args {
    /* 0x0 */ s32 field_0x0;
};
CHECK_SIZEOF(Args, 0x4);

/* The type of the task's callbacks. */
typedef boost::function1<void, Args &> SelectCallback;

/* SavePointSelect::SavePointSelectTask (class name from RTTI): a 0x48-byte task. Its constructor
 * (0x80331114) runs Task's, constructs the two callbacks, creates the object at 0x40 and stores the
 * name "SavePointSelectTask". The SAVEPOINT gadget binds its selection callback to field_0x28 and
 * its cancel callback to field_0x34; when the task calls them is not established here. */
class SavePointSelectTask : public Task {
  public:
    /* arg1 is passed on to the constructor of the object at 0x40 (the SAVEPOINT gadget passes its
     * "priority" parameter; meaning inferred). */
    SavePointSelectTask(Task *parent, int arg1);
    /* 0x80330FD8 */
    virtual ~SavePointSelectTask();
    /* 0x80330EAC */
    virtual void Update(float dt);

    /* 0x28 */ SelectCallback field_0x28;
    /* 0x34 */ SelectCallback field_0x34;
    /* 0x40 */ UnkObject_80331E94 *field_0x40;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(SavePointSelectTask, 0x48);

} // namespace SavePointSelect

extern "C" {
/* Returns the flag byte at 0x805F0C39 in USA (GUPE8P), which fn_80330CC4 sets and fn_80330CB8
 * clears (behaviour from their bodies; the result type is inferred). */
bool fn_80330CB0(void);
/* Clears the flag byte that fn_80330CB0 returns (the SAVEPOINT descriptor's second hook). */
void fn_80330CB8(void);
/* The work of the SAVEPOINT descriptor's first hook after its resource request: sets the flag byte
 * that fn_80330CB0 returns and clears it again unless the value it reads is 5 to 8 (behaviour
 * inferred from its body). */
void fn_80330CC4(void);
}

#endif
