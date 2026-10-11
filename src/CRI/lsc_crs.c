/* CRI LSC critical section (lsc_crs.c): the scheduler lock maps onto the stream-joint lock. */

#include "CRI/lsc.h"

/* The mask argument is unused: SJCRS keeps the saved interrupt state itself. */
void LSC_UnlockCrs(Sint32 *msk) {
    SJCRS_Unlock();
}

void LSC_LockCrs(Sint32 *msk) {
    SJCRS_Lock();
}
