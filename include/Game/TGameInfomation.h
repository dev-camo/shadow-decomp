#ifndef GAME_TGAMEINFOMATION_H
#define GAME_TGAMEINFOMATION_H

/* Functions of TGameInfomation's file (class name from RTTI; 0x800C02CC-0x800C06A0 in the USA
 * release, no source yet) that other files call. The class is not declared yet. */

extern "C" {
/* Formats name, the language suffix ("_JP", "_EN", ...) that the word at 0x58 of lbl_8057D6A0
 * selects and ".txd" into a static buffer, replaces the buffer's contents with the copy that
 * fn_80051810 makes of them and returns the buffer (behaviour inferred from its body; the parameter
 * name and the types are inferred). */
const char *fn_800C02CC(const char *name);
}

#endif
