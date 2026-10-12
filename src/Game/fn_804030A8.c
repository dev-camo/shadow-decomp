#include "CRI/lsc.h"
#include "CRI/svm.h"

/* Returns whether the scheduler is in the error state. Name and return type unknown. */
int fn_804030A8(LSC lsc) {
    return LSC_GetStat(lsc) == LSC_STAT_ERROR;
}

/* Moves the server manager to server type 6 (the MW idle server). Name unknown. */
void fn_804030D4(void) {
    SVM_GotoSvrBorder(6);
}
