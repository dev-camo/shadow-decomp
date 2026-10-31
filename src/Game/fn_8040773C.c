#include "CRI/mwsfd.h"
#include "CRI/mwstm.h"

/* Parameter names from MWSTM_SetFlowLimit, which receives them unchanged. */
void fn_8040773C(MWPLY mwply, int min_nsct, int max_nsct) {
    MWSTM_SetFlowLimit(mwply->stm, min_nsct, max_nsct);
    MWSFLSC_SetFlowLimit(mwply, min_nsct);
}
