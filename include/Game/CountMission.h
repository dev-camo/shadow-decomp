#ifndef GAME_COUNTMISSION_H
#define GAME_COUNTMISSION_H

#include "Game/Mission.h"
#include <dolphin/types.h>
#include "layout_check.h"

namespace Mission {

/* Mission::CountMission (class name from RTTI, with Mission::Mission as its virtual base): 0x9C
 * bytes with its Mission part (0x58 before it; the mission factory allocates 0x9C bytes for one).
 * It keeps two counters (0x18 and 0x1C). The overrides are described at their Mission
 * declarations; the signatures are inferred from the implementations noted. */
class CountMission : public virtual Mission {
  public:
    /* Constructs the Mission part with id and index unless a derived class does, stores the other
     * four arguments at 0x8, 0xC, 0x10 and 0x14, constructs the message manager at 0x28 and then
     * makes the virtual call fn_8016AF48 (fn_8016AD9C; the parameter types are inferred). */
    CountMission(int id, int index, int arg2, int arg3, int arg4, int arg5);
    virtual ~CountMission();
    virtual int fn_8016AF38();
    virtual int fn_8016B240(int which);
    virtual int fn_8016B238();
    virtual int fn_8016B230();
    virtual void fn_8016B224(int value, int which);
    virtual void fn_8016B220(int value, int which);
    virtual void fn_8016B21C(int value, int which);
    virtual void fn_8016AF40();
    virtual void fn_8016AF44();
    virtual void fn_8016AF48();

    /* 0x08 */ int field_0x8;
    /* 0x0C */ int field_0xC;
    /* 0x10 */ int field_0x10;
    /* 0x14 */ int field_0x14;
    /* 0x18 */ int field_0x18;
    /* 0x1C */ int field_0x1C;
    /* 0x20 */ int field_0x20;
    /* 0x24 */ int field_0x24;
    /* A Mission::CountMessageManage (constructed by fn_802ED710, destroyed by fn_802ED644); not
     * mapped here. */
    /* 0x28 */ u8 field_0x28[0x58 - 0x28];
};
CHECK_SIZEOF(CountMission, 0x9C);

} // namespace Mission

#endif
