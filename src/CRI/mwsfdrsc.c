/* CRI Sofdec MW player resource save/restore hooks (mwsfdrsc.c): the player calls mwPlySaveRsc
 * before it waits for the idle server border and mwPlyRestoreRsc after it. Both are empty on the
 * GameCube.
 *
 * Names follow the RE4 GameCube decompilation of the same file, whose two empty functions come in
 * this order. */
#include "CRI/mwsfdrsc.h"

/* No-op on GameCube. */
void mwPlyRestoreRsc(void) {}

/* No-op on GameCube. */
void mwPlySaveRsc(void) {}
