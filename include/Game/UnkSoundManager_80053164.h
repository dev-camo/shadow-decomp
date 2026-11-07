#ifndef GAME_UNKSOUNDMANAGER_80053164_H
#define GAME_UNKSOUNDMANAGER_80053164_H

#include "Game/fn_8040_vector_types.h"
#include <dolphin/types.h>

/* The manager returned by fn_80053164 (placeholder names: the original class and function names
 * are unknown). fn_80053164 constructs it on first use, registers its destructor and caches its
 * address in a static pointer (behaviour inferred). */
class UnkSoundManager_80053164;

extern "C" {
UnkSoundManager_80053164 *fn_80053164(void);
/* Plays sound id at position; returns false for id 0xFFFF (behaviour inferred; arg3 and arg4 are
 * passed on and not understood). The weapons pass ids from 0x2000 upwards. */
bool fn_801D54F4(UnkSoundManager_80053164 *manager, u16 id, const Fn8040Vector3 *position, int arg3,
                 u16 arg4);
/* Another way to play sound id: gets an object from fn_801D5B74 (with 0 for its last two
 * arguments), passes it to fn_801D9780 and stores arg2 as its byte at 0x6 and arg3 as its word at
 * 0x18; returns false for id 0xFFFF or when there is no object (behaviour inferred; arg2 and arg3
 * are not understood). */
bool fn_801D560C(UnkSoundManager_80053164 *manager, u16 id, int arg2, int arg3);
/* Plays sound id at position (behaviour inferred); returns false for id 0xFFFF or when fn_801D5B74
 * gives no object. owner is a key identifying the caller, not a typed object: it is only compared
 * with a sound object's word at 0x18 and stored there, and callers pass themselves, other values
 * or 0. When owner is not null and fn_801D5A64 finds a sound for id and owner, it moves that sound
 * to position (fn_801D956C), stores arg5 as its byte at 0x6 and, unless fn_801D9608 returns 1,
 * passes arg4 and arg6 to fn_801D95A4; either way it returns true without starting another sound.
 * Otherwise it starts a new sound with fn_801D5B74 (bit 1 of arg4 selects that call's flag),
 * passes it to fn_801D9780, passes arg4 with the position to fn_801D9688 and arg4 with arg6 to
 * fn_801D9714, and stores arg5 as its byte at 0x6 and owner as its word at 0x18. arg4 to arg6 are
 * not understood. arg5 is wider than a byte: ElecFan converts a computed value to a signed byte
 * and sign-extends it when it passes it (the width is otherwise unknown). */
bool fn_801D53C4(UnkSoundManager_80053164 *manager, u16 id, const Fn8040Vector3 *position,
                 const void *owner, u16 arg4, int arg5, s16 arg6);
/* Plays sound 0xE00B through fn_801D5B74 and returns whether it started (behaviour inferred; the
 * RING gadget calls it when a ring is collected). It returns false without playing while the
 * float at 0x18 of manager is greater than the float at 0x805F5DD8, and when fn_801D5B74 returns
 * null; otherwise it sets that float to the float at 0x805F5DDC and gives the sound a byte from a
 * two-entry table at 0x8053CE14, taking the entries in turn. */
bool fn_801D5300(UnkSoundManager_80053164 *manager);
/* Passes id, arg2 and arg3 to fn_801D7A50 and ignores manager (behaviour from its body; purpose
 * unknown, the return type is inferred). Weapon::ChaosSpear::ChaosSpear passes 0xF and 0x10 with -1
 * and -1. */
void fn_801D51A0(UnkSoundManager_80053164 *manager, int id, s8 arg2, int arg3);
}

#endif
