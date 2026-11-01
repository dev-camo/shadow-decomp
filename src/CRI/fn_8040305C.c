/* CRI Sofdec MW player: the vertical sync wait the game calls between the resource hooks
 * (mwsfdrsc.c) and the load scheduler accessors (mwsfdsl.c).
 *
 * The original file name and the function's name are unknown (it has no counterpart in the RE4
 * GameCube decompilation), so the unit keeps a placeholder name.
 */
#include "CRI/adx.h"

/* Waits for the next vertical sync (ADXM_WaitVsync). */
void fn_8040305C(void) {
    ADXM_WaitVsync();
}
