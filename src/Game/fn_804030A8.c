#include "Game/fn_803C81_query_types.h"
#include "Game/fn_803CB_state_types.h"

/* Chosen pointer/predicate projection; original full types/arity unknown. */
int fn_804030A8(const Fn803C81QueryView *object) {
    return LSC_GetStat(object) == 3;
}
/* Chosen side-effect projection; original full arity/types/return unknown. */
void fn_804030D4(void) {
    SVM_GotoSvrBorder(6);
}
