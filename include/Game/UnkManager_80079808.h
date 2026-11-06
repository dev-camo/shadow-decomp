#ifndef GAME_UNKMANAGER_80079808_H
#define GAME_UNKMANAGER_80079808_H

#include <dolphin/types.h>
#include "layout_check.h"

struct Fn8040Vector3;
struct UnkRotation3;

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

/* An entry of the table of 0x4C-byte entries at 0x728 (placeholder name). Only two members are
 * mapped: the word at 0x28 that fn_8016F280 returns (the SAVEPOINT gadget reads it for entry 0)
 * and the counters at 0x2C that the attack callback of FallBuildingHolder uses. */
class UnkEntry_80178B5C {
  public:
    /* 0x00 */ u8 field_0x0[0x28];
    /* 0x28 */ s32 field_0x28; /* fn_8016F280 returns it */
    /* 0x2C */ UnkCounters_8016F5B8 field_0x2C;
    /* 0x44 */ u8 field_0x44[0x4C - 0x44];
};
CHECK_SIZEOF(UnkEntry_80178B5C, 0x4C);

/* The object that fn_80079808 returns: a static object (at 0x80575FBC in the USA release) that the
 * function constructs and caches on first use (placeholder name; the original class is unknown).
 * Only the members that FallBuildingHolder and the SECRETKEY gadget's file use are mapped; its size
 * is not known. */
class UnkManager_80079808 {
  public:
    /* 0x000 */ u8 field_0x0[0x70C];
    /* 0x70C */ UnkState_8016F0CC field_0x70C;
    /* The SECRETKEY gadget's file passes its address to fn_803357F4 when it adds a key. It follows
     * the two mapped entries of field_0x70C; whether it belongs to that record is unknown. */
    /* 0x724 */ u32 field_0x724;
};
CHECK_OFFSETOF(UnkManager_80079808, field_0x70C, 0x70C);
CHECK_OFFSETOF(UnkManager_80079808, field_0x724, 0x724);

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
/* Stores the word at 0x90 of the global at 0x8057D6A0 in *value (behaviour inferred from its
 * body). */
void fn_803357F4(u32 *value);
/* Returns the word that value points to (behaviour from its body). The SAVEPOINT gadget passes
 * the word at 0x28 of entry 0 of the table at 0x728. */
s32 fn_8016F280(const s32 *value);
/* Returns the vector of entry index of row row (behaviour from its body). The rows start at 0xC
 * of the manager, 0x380 bytes apart, and hold 32 entries of 0x1C bytes; an entry holds a vector at
 * 0x0, three rotation floats at 0xC and a flag byte at 0x18. */
Fn8040Vector3 *fn_8016F344(UnkManager_80079808 *manager, int index, int row);
/* Returns how many of the 32 entries of the first row have their flag byte set (behaviour from its
 * body). */
int fn_8016F3A0(UnkManager_80079808 *manager);
/* Returns 0 when index is 32 or more. Otherwise stores the word at 0x90 and the float at 0x94 of
 * lbl_8057D6A0 at 0x4 and 0x8, stores the vector, the three floats of rotation and a set flag
 * byte in entry index of both rows, raises the word at 0x0 to index when it is lower, and returns 1
 * (behaviour inferred from its body; the second row's copy also changes the vector's x by an
 * amount computed from the row number). */
int fn_8016F3D4(UnkManager_80079808 *manager, const Fn8040Vector3 *position,
                const UnkRotation3 *rotation, int index);
}

#endif
