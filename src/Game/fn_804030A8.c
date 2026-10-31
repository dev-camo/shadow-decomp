#include "CRI/lsc.h"

/* Returns whether the scheduler is in the error state. Name and return type unknown. */
int fn_804030A8(LSC lsc) {
    return LSC_GetStat(lsc) == LSC_STAT_ERROR;
}
