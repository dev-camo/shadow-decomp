#ifndef GAME_UNKGRAVITY_80293928_H
#define GAME_UNKGRAVITY_80293928_H

#include "Game/fn_8040_vector_types.h"
#include <dolphin/types.h>

/* The stage's gravity direction (placeholder names; the original class and functions are unknown,
 * the behaviour is inferred from the GravityChangeCollision gadget). A four-byte state object at
 * lbl_805F0340 holds the index of the current direction in a table of vectors at lbl_8058436C. */
extern "C" {
/* Creates the state object with index 0 unless it exists (GravityChangeCollision's gadget type
 * calls it from its hookA). */
void fn_80293B1C(void);
/* Deletes the state object (the same gadget type calls it from its hookB). */
void fn_80293AE4(void);
/* The current index, or 0 when the state object does not exist. */
int fn_80293AC8(void);
/* Stores index, then sends a CharCommand carrying the new direction vector to the character info
 * that fn_800A8B74 returns for lbl_805EE588 and to those fn_8007488C returns for 1 to 13 (purpose
 * not understood; does nothing when the state object does not exist). */
void fn_80293928(int index);
/* The vector at index in the table of directions. */
const Fn8040Vector3 *fn_80293B60(int index);
}

#endif
