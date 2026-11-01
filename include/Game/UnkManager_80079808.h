#ifndef GAME_UNKMANAGER_80079808_H
#define GAME_UNKMANAGER_80079808_H

#include <dolphin/types.h>
#include "layout_check.h"

/* The state at 0x70C of the object that fn_80079808 returns (placeholder name; the original class
 * is unknown). fn_8016F0CC reads it as 0xC-byte entries of three words; only the two entries that
 * its guard loop walks are mapped and the size is not known. */
class UnkState_8016F0CC {
  public:
    /* 0x00 */ s32 field_0x0[2][3];
};

/* The state at 0x2C of an entry of the table at 0x728 of the same object (placeholder name; the
 * original class is unknown). Only the six words that fn_8016F5B8 writes are mapped. */
class UnkCounters_8016F5B8 {
  public:
    /* 0x00 */ s32 field_0x0[3];
    /* 0x0C */ s32 field_0xC[3];
};
CHECK_SIZEOF(UnkCounters_8016F5B8, 0x18);

/* An entry of the table of 0x4C-byte entries at 0x728 (placeholder name; only the member the
 * attack callback of FallBuildingHolder uses is mapped). */
class UnkEntry_80178B5C {
  public:
    /* 0x00 */ u8 field_0x0[0x2C];
    /* 0x2C */ UnkCounters_8016F5B8 field_0x2C;
    /* 0x44 */ u8 field_0x44[0x4C - 0x44];
};
CHECK_SIZEOF(UnkEntry_80178B5C, 0x4C);

/* The object that fn_80079808 returns: a static object (at 0x80575FBC in the USA release) that the
 * function constructs and caches on first use (placeholder name; the original class is unknown).
 * Only the members that FallBuildingHolder uses are mapped; its size is not known. */
class UnkManager_80079808 {
  public:
    /* 0x000 */ u8 field_0x0[0x70C];
    /* 0x70C */ UnkState_8016F0CC field_0x70C;
};
CHECK_OFFSETOF(UnkManager_80079808, field_0x70C, 0x70C);

extern "C" {
UnkManager_80079808 *fn_80079808(void);
/* For a positive amount, returns at once if the first word of either entry has reached its second
 * word. It also returns if the third word of entry index is positive, or if the amount is not
 * positive and the byte at 0xC8 of the global at 0x8057D6A0 is set. Otherwise it adds amount to
 * the first word of entry index and stores back the word that fn_8016F21C returns a pointer to
 * (called with pointers to that word, the word at 0x805E6808 and the entry's second word)
 * (behaviour inferred from its body). The attack callback of FallBuildingHolder passes 1 and 900. */
void fn_8016F0CC(UnkState_8016F0CC *state, int index, int amount);
/* Returns the entry of the table at 0x728 with the given index, or null for a negative index
 * (behaviour inferred). */
UnkEntry_80178B5C *fn_80178B5C(UnkManager_80079808 *manager, int index);
/* Returns at once when amount is 0. Otherwise adds amount to field_0x0[index] when index is 0 to
 * 2, then, for any index, copies the word at 0x90 of the global at 0x8057D6A0 into
 * field_0xC[index] (behaviour inferred from its body). The attack callback of FallBuildingHolder
 * passes 2 and 200. */
void fn_8016F5B8(UnkCounters_8016F5B8 *counters, int index, int amount);
}

#endif
