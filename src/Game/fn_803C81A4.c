#include "CRI/lsc.h"

/* lsc.c error literals (its .rodata is not sourced yet). */
extern Char8 lbl_80516ECC[];
extern Char8 lbl_80516EF8[];

/* Entries still queued (including the one loading). */
Sint32 LSC_GetNumStm(LSC lsc) {
    if (lsc == 0) {
        LSC_CallErrFunc(lbl_80516ECC);
        return -1;
    }
    return lsc->num_stm;
}

/* Scheduler state (LSC_STAT_*). */
Sint32 LSC_GetStat(LSC lsc) {
    if (lsc == 0) {
        LSC_CallErrFunc(lbl_80516EF8);
        return -1;
    }
    return lsc->stat;
}
