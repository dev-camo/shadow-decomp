#ifndef GAME_UNKGRAVITY_80293928_H
#define GAME_UNKGRAVITY_80293928_H

#include "Game/fn_8040_vector_types.h"
#include <dolphin/types.h>

/* The stage's gravity direction (placeholder names; the original class and functions are unknown,
 * the behaviour is inferred from the GravityChangeCollision gadget). The pointer at
 * lbl_UnkGravityState_805F0340 refers to a separately allocated four-byte state holding the current
 * direction index in the file's table of six directions (0x8058436C in USA (GUPE8P)): +y, -y, +x,
 * -x, +z and -z. */
class UnkGravityState_805F0340;

extern "C" {
extern UnkGravityState_805F0340 *lbl_UnkGravityState_805F0340;
/* Creates the state object with index 0 unless it exists (the gravity gadgets' hookA calls it:
 * GravityChangeCollision, GravitySignBoard). */
void fn_80293B1C(void);
/* Deletes the state object (called from the same gadgets' hookB). */
void fn_80293AE4(void);
/* The current index, or 0 when the state object does not exist. */
int fn_80293AC8(void);
/* Stores index, then sends a Player::SetGravityCommand carrying the new direction vector to the
 * character info that fn_800A8B74 returns for lbl_805EE588, when it still exists, and to those
 * fn_8007488C returns for 1 to 13 that have not expired (which characters these are is not
 * understood). A command whose allocation fails is passed on as null. Does nothing when the state
 * object does not exist. */
void fn_80293928(int index);
/* The vector at index in the table of directions. */
const Fn8040Vector3 *fn_80293B60(int index);
/* The vector (0, 1, 0), built by the same start-up code (at 0x80293B74) that fills the table of
 * directions, whose first entry is a copy of it; GravitySignBoard takes it as the direction its
 * model faces when it is not turned (purpose inferred). */
extern Fn8040Vector3 lbl_80584360;
}

#endif
