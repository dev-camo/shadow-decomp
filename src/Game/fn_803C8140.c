#include "CRI/lsc.h"

/* lsc.c error literals (its .rodata is not sourced yet). */
extern Char8 lbl_80516D18[];
extern Char8 lbl_80516D44[];

/* Refill threshold for the stream controller. */
void LSC_SetFlowLimit(LSC lsc, Sint32 min_val) {
    if (lsc == 0) {
        LSC_CallErrFunc(lbl_80516D18);
    } else if (min_val < 0 || min_val > lsc->bufsize) {
        LSC_CallErrFunc(lbl_80516D44, min_val);
    } else {
        lsc->min_val = min_val;
    }
}
