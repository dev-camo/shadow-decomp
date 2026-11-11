#ifndef GAME_UNKRAILROUTE_8054B8E0_H
#define GAME_UNKRAILROUTE_8054B8E0_H

#include <dolphin/types.h>
#include "layout_check.h"

/* A record of the table of eleven at 0x8054B8E0 (placeholder names; the table is defined in a unit
 * that is not recovered). At construction, the RAIL SWITCH gadget indexes the table with its
 * "route" parameter and shows itself switched on when the record's word at 0x8 is 1. The command
 * handler fn_8025E9C0, on a SwitchCommand, indexes the table with the command's word at 0x14 when
 * it is from 0 to 10 and the handler object's word at 0x38 is not 0, and sets the record's word
 * at 0x8 to 1 when it was not 1, and to 0 when it was. fn_8025FD2C, for the record that its
 * object's word at 0x58 selects, passes the word at 0x4 to fn_8001C7B4 when the word at 0x8 is 1
 * and the word at 0x0 when it is not, and fn_8025FE00 clears the word at 0x8 of all eleven records
 * (behaviour inferred).
 *
 * BKTankController.cpp has its own table of four records of this shape (0x8055C870): its command
 * handler toggles the word at 0x8 in the same way for a SwitchCommand whose word at 0x14 is from
 * 0 to 3 while the controller's pointer at 0x38 is not null, and its path control selects the
 * path number by that word in the same way. */
struct UnkRailRoute_8054B8E0 {
    /* 0x0 */ s32 field_0x0;
    /* 0x4 */ s32 field_0x4;
    /* 0x8 */ s32 field_0x8;
    /* Bytes 0xC and 0xD hold 0 or 1 in the table; fn_8025F8C4 tests the byte at 0xD. */
    /* 0xC */ u8 field_0xC[4];
};
CHECK_SIZEOF(UnkRailRoute_8054B8E0, 0x10);

extern UnkRailRoute_8054B8E0 lbl_8054B8E0[11];

#endif
