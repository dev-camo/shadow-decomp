#ifndef GAME_TIMERCOUNTMISSION_H
#define GAME_TIMERCOUNTMISSION_H

#include "Game/CountMission.h"
#include "Game/TimerMission.h"
#include "layout_check.h"

namespace Mission {

/* Mission::TimerCountMission (class name from RTTI, with Mission::CountMission at 0x0,
 * Mission::TimerMission at 0x58 and their virtual base Mission::Mission at 0x94): 0xD8 bytes (the
 * mission factory allocates 0xD8 bytes for one). */
class TimerCountMission : public CountMission, public TimerMission {
  public:
    /* The parameter types of id and index are inferred, as for Mission. */
    TimerCountMission(int id, int index, int count, float time);
    virtual ~TimerCountMission();
    virtual int fn_8016AF38();
    virtual int fn_8016B240(int which);
    virtual int fn_8016B238();
    virtual int fn_8016B230();
    virtual void fn_8016B224(int value, int which);
    virtual void fn_8016B220(int value, int which);
    virtual void fn_8016AF40();
    virtual void fn_8016AF44();
    virtual void fn_8016AF48();
    virtual void fn_8016AA98();
};
CHECK_SIZEOF(TimerCountMission, 0xD8);

} // namespace Mission

#endif
