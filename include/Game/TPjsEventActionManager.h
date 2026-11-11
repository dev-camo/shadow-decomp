#ifndef GAME_TPJSEVENTACTIONMANAGER_H
#define GAME_TPJSEVENTACTIONMANAGER_H

#include "Game/Task.h"
#include "layout_check.h"

/* TPjsEventActionData (class name from RTTI): a base class of TPjsEventActionManager. The manager's
 * RTTI base list gives it offset 0, the same as Task, so it has no data members and no virtual
 * functions; its member functions are unknown. TPjsEventActionScene's RTTI base list (USA
 * 0x8052BA10) also names it at offset 0, so that class, which has no source yet, derives from
 * this one too. */
class TPjsEventActionData {};
/* An empty class has size 1 on its own; as a base of the manager it takes no room. */
CHECK_SIZEOF(TPjsEventActionData, 0x1);

/* TPjsEventActionManager (class name from RTTI): the 0x90-byte task that fn_8017430C creates
 * (USA 0x80174330-0x80174350). Its RTTI base list names Task and then TPjsEventActionData, both at
 * offset 0, which is what the compiler writes when the empty class is the first base and Task the
 * second. Its vtable (USA 0x8052BA7C) has Task's two slots. Its constructor (USA 0x80173B10) and
 * its own members are not mapped. */
class TPjsEventActionManager : public TPjsEventActionData, public Task {
  public:
    virtual ~TPjsEventActionManager();
    virtual void Update(float dt);

    /* 0x28 */ u8 field_0x28[0x90 - 0x28];
};
CHECK_SIZEOF(TPjsEventActionManager, 0x90);

/* The object that fn_801723E0 returns (placeholder name; USA 0x80580208 in .bss). Only its first
 * word is mapped; the size is unknown. */
class UnkObject_801723E0 {
  public:
    /* fn_8017430C stores the manager it creates here. */
    /* 0x00 */ TPjsEventActionManager *field_0x0;
};
/* Checks only the mapped prefix, not the object's size. */
CHECK_SIZEOF(UnkObject_801723E0, 0x4);

extern "C" {
/* Returns the address of the object at USA 0x80580208, which it stores in a static pointer on the
 * first call (behaviour inferred from its body). */
UnkObject_801723E0 *fn_801723E0(void);
/* Calls fn_800447B8 on fn_80044254's manager with value and arg2. The float and the integer
 * arrive in different registers, so the code does not show which parameter comes first. */
void fn_80173834(float value, int arg2);
/* Calls fn_800446D0 on fn_80044254's manager with value, arg2 and arg3. As for fn_80173834, the
 * code does not show where the float stands among the parameters. */
void fn_80173878(float value, int arg2, int arg3);
}

#endif
