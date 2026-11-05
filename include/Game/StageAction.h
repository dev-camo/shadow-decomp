#ifndef GAME_STAGEACTION_H
#define GAME_STAGEACTION_H

#include "layout_check.h"

namespace StageAction {

/* StageAction::IAction (class name from RTTI): the only base of the StageAction classes. Its own
 * vtable and those of DeadAction, EndAction, EventAction, FlashbackAction, GoalAction, InitAction,
 * PauseAction, PlayAction, Result2PAction, SaveAction, ShowAction, TimeupAction and TryAgainAction
 * share its second virtual function (0x8013D37C), which returns false; ContinueAction and
 * RestartAction override it with a function that returns whether their word at 0x4 is 2. Its
 * meaning is unknown. Only the destructor and this function are declared. */
class IAction {
  public:
    virtual ~IAction();
    virtual bool fn_8013D37C() {
        return false;
    }
};

} // namespace StageAction

/* The declared view only (a vtable pointer); the original base may hold more. */
CHECK_SIZEOF(StageAction::IAction, 0x4);

#endif
