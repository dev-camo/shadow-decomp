#ifndef GAME_UNKSWITCHCOUNTER_801660A0_H
#define GAME_UNKSWITCHCOUNTER_801660A0_H

#include <dolphin/types.h>
#include "layout_check.h"

/* The switch counter (placeholder name; its constructor is at 0x801660A0 and its destructor at
 * 0x80165F7C). It keeps, per com id, a record of three words: the count (0x0), the number to reach
 * (0x4) and a third word that starts equal to the count (behaviour inferred from fn_80165A08).
 * The methods are named after the functions that implement them. */
class UnkSwitchCounter_801660A0 {
  public:
    UnkSwitchCounter_801660A0();
    ~UnkSwitchCounter_801660A0();

    /* True when the id has a record (behaviour inferred). */
    bool fn_80165944(u8 id);
    /* Creates the record for id with the number to reach and the starting count (behaviour
     * inferred). */
    void fn_80165A08(u8 id, int target, int count);
    /* Adds delta to the count of id and, once it reaches the target, sets state 1 of the id in the
     * UnkManager_800CB69C (behaviour inferred). */
    void fn_801658AC(u8 id, int delta);
    /* Calls fn_80166424 and clears the byte at 0x10, when it is set (behaviour inferred). */
    void fn_80165EFC();
    /* Calls fn_80166424 and sets the byte at 0x10, when it is clear (behaviour inferred). */
    void fn_80165F3C();

    /* The counter, constructed on first use: a function-local static whose address is cached in
     * s_instance (name inferred). CandleStick.cpp emits its code; other units call it too, and
     * where it is defined in the original source is not known. */
    static UnkSwitchCounter_801660A0 *GetInstance();

    static UnkSwitchCounter_801660A0 *s_instance;

    /* 0x00 */ u8 field_0x0[0x10];
    /* 0x10 */ u8 field_0x10;
    /* 0x11 */ u8 field_0x11[0x14 - 0x11];
};
CHECK_SIZEOF(UnkSwitchCounter_801660A0, 0x14);

#endif
