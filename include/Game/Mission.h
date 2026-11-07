#ifndef GAME_MISSION_H
#define GAME_MISSION_H

#include <dolphin/types.h>
#include "layout_check.h"

namespace Mission {

/* Mission::Mission (class name from RTTI): the base of the mission classes, 0x40 bytes (the
 * mission factory at 0x8016B384 allocates 0x40 bytes for a plain Mission). Count and timer
 * missions derive from it virtually; as a virtual base it is followed by a 4-byte word at 0x40
 * that the derived constructors and destructors set and the this-adjusting thunks read. The
 * mission manager keeps each mission in a shared pointer to this class (fn_8016B844 converts a
 * TimerCountMission pointer to its Mission part for one), and gadgets look missions up through
 * fn_800740E4.
 *
 * Its virtual functions are named after their implementations here (vtable 0x8052B7C8, 16 slots);
 * their purpose is not established. The signatures are inferred from the implementations and
 * overrides noted. */
class Mission {
  public:
    /* Stores id at 0x4 and index at 0x8 and constructs the message manager at 0x18 (fn_8016B328;
     * the parameter names and types are inferred: the factory passes both through, and the
     * mission compares the word at 0x8 with 0 and 2). */
    Mission(int id, int index);
    virtual ~Mission();
    /* Returns 0 here, 2 for a CountMission, 4 for a TimerMission and 6 for a TimerCountMission. */
    virtual int fn_8016AF38();
    /* Returns 0 here and 1 for a TimerMission. */
    virtual int fn_8016AA90();
    /* Returns a float constant here and a TimerMission's float at 0x8. */
    virtual float fn_8016B228();
    /* Here, with which 0, whether the state at 0xC is 1, otherwise whether it is 2; a CountMission
     * returns its counter selected by which (0x18 for 0, otherwise 0x1C). */
    virtual int fn_8016B240(int which);
    /* Return 1 here; a CountMission returns its words at 0x8 and 0xC. */
    virtual int fn_8016B238();
    virtual int fn_8016B230();
    /* Empty here. A CountMission updates the counter that which selects, adding value
     * (fn_8016B224) or storing it (fn_8016B220):
     * - which 0: for a negative value the counter at 0x18 becomes 0 (storing) or the sum clamped
     *   to 0 (adding), and nothing else happens; otherwise the counter is updated and passed with
     *   this + 0x28 to fn_802ED4E0, and when it is at least the word at 0x8 the word at 0x0 goes
     *   to fn_8016B2A4;
     * - any other which: nothing happens while the word at 0xC is 0; otherwise the counter at 0x1C
     *   is updated, and when it is at least the word at 0xC the word at 0x0 goes to fn_8016B270.
     * fn_8016B21C sets the word at 0x8 or 0xC. */
    virtual void fn_8016B224(int value, int which);
    virtual void fn_8016B220(int value, int which);
    virtual void fn_8016B21C(int value, int which);
    /* Returns 0 here. */
    virtual int fn_8016B214();
    /* Empty here. */
    virtual void fn_8016B210();
    /* Empty here. A CountMission copies its counters to 0x20/0x24 (fn_8016AF40), copies them
     * back (fn_8016AF44) or sets both the counters and the copies from 0x10/0x14 (fn_8016AF48),
     * then makes the matching virtual call on its message manager, which is all a TimerMission
     * does. */
    virtual void fn_8016AF40();
    virtual void fn_8016AF44();
    virtual void fn_8016AF48();
    /* Empty here; a TimerMission passes the float at 0xA0 of lbl_8057D6A0 to its message manager
     * (fn_802EF224) and may then call fn_8016B270. */
    virtual void fn_8016AA98();

    /* Matching workaround: calling these inline accessors through the smart pointer (Field0xCEquals
     * as an if condition, possibly negated) leaves the smart pointer's operator-> out of line, as
     * in the original; a plain field access expands it in place. The original accessors are
     * unknown. Revisit when they are identified.
     *
     * Whether the word at 0xC equals value (name inferred; the gadgets test it against 1 and 2). */
    bool Field0xCEquals(int value) {
        return field_0xC == value;
    }

    /* Stores value in the byte at 0x14 (name inferred; its meaning is unknown). */
    void SetField0x14(u8 value) {
        field_0x14 = value;
    }

    /* 0x04 */ int field_0x4;
    /* 0x08 */ int field_0x8;
    /* 0x0C: 0 at construction; fn_8016B2A4 sets 1, fn_8016B270 sets 2 when it is not 1. */
    /* 0x0C */ int field_0xC;
    /* 0x10 */ int field_0x10;
    /* 0x14 */ u8 field_0x14; /* a gadget stores 1 here */
    /* 0x15 */ u8 field_0x15;
    /* A Mission::BasicMessageManage (constructed by fn_802EBFE0, destroyed by fn_802EBF14); not
     * mapped here. */
    /* 0x18 */ u8 field_0x18[0x40 - 0x18];
};
CHECK_SIZEOF(Mission, 0x40);

} // namespace Mission

#endif
