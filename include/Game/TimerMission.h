#ifndef GAME_TIMERMISSION_H
#define GAME_TIMERMISSION_H

#include "Game/Mission.h"
#include <dolphin/types.h>
#include "layout_check.h"

namespace Mission {

/* Mission::TimerMission (class name from RTTI, with Mission::Mission as its virtual base): 0x80
 * bytes with its Mission part (0x3C before it; the mission factory allocates 0x80 bytes for one).
 * The overrides are described at their Mission declarations; the signatures are inferred from the
 * implementations noted. */
class TimerMission : public virtual Mission {
  public:
    /* Constructs the Mission part with id and index unless a derived class does, stores time at
     * 0x8 and constructs the message manager at 0xC (fn_802C8E50; the parameter types of id and
     * index are inferred). */
    TimerMission(int id, int index, float time);
    virtual ~TimerMission();
    virtual int fn_8016AF38();
    virtual int fn_8016AA90();
    virtual float fn_8016B228();
    virtual void fn_8016AF40();
    virtual void fn_8016AF44();
    virtual void fn_8016AF48();
    virtual void fn_8016AA98();

    /* 0x08 */ float field_0x8;
    /* A Mission::TimerMessageManage (constructed by fn_802EF49C, destroyed by fn_802EF3D0); not
     * mapped here. */
    /* 0x0C */ u8 field_0xC[0x3C - 0xC];
};
CHECK_SIZEOF(TimerMission, 0x80);

} // namespace Mission

#endif
