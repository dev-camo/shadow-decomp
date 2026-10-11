#include "Game/fn_803C82_record_inline.h"

extern void *memset(void *destination, int value, unsigned int size);

/* Stops and clears the scheduler. */
void LSC_Destroy(LSC lsc) {
    if (lsc != 0) {
        lsc_stop_body(lsc);
        lsc->used = 0;
        memset(lsc, 0, sizeof(LSC_OBJ));
    }
}
