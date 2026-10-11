#include "CRI/lsc.h"
#include "CRI/svm.h"

/* Chosen predicate projection; original name and return type unknown. */
int fn_804030A8(LSC lsc) {
    return LSC_GetStat(lsc) == LSC_STAT_ERROR;
}

/* Chosen side-effect projection; original name unknown. Server type 6 is the MW idle server. */
void fn_804030D4(void) {
    SVM_GotoSvrBorder(6);
}
