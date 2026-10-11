/* CRI GameCube renderer critical section (rna_crs.c): AXRNA's lock around the AX/MIX voice calls,
 * mapped to the stream-joint lock (SJCRS).
 *
 * Source from the RE4 GameCube decompilation. RE4's two unused statics have no .bss slot in this
 * game, so they are left out.
 */
#include "CRI/sj.h"

/* Leaves the renderer critical section. */
void GCRNA_UnlockCs(void) {
    SJCRS_Unlock();
}

/* Enters the renderer critical section. */
void GCRNA_LockCs(void) {
    SJCRS_Lock();
}
