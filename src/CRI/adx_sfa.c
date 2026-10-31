/* CRI SFA (streamed file archive) module stub (adx_sfa.c): only the reference counter survives.
 *
 * Source from the RE4 GameCube decompilation. The rest of the 8-byte .bss slot is padding.
 */
#include "CRI/cri_xpt.h"

static Sint32 sfa_init_cnt;

/* Decrements the SFA user count. */
void SFA_Finish(void) {
    sfa_init_cnt--;
}

/* Increments the SFA user count. */
void SFA_Init(void) {
    sfa_init_cnt++;
}
