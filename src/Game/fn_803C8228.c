#include "CRI/lsc.h"

/* Scheduler server pass: lsc_ExecHndl on every live scheduler, under the lock. */
void LSC_ExecServer(void) {
    Sint32 msk;
    LSC_OBJ *lsc;
    Sint32 i;

    LSC_LockCrs(&msk);
    for (i = 0; i < LSC_MAX_OBJ; i++) {
        lsc = &lsc_obj[i];
        if (lsc->used == 1) {
            lsc_ExecHndl(lsc);
        }
    }
    LSC_UnlockCrs(&msk);
}
