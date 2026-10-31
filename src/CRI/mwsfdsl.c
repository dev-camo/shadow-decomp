/* CRI Sofdec MW player load scheduler accessors (mwsfdsl.c): the flow limit and the error state of
 * a handle's load scheduler.
 *
 * Names follow the RE4 GameCube decompilation of the same file.
 */
#include "CRI/mwsfd.h"

/* Sets the refill threshold of the handle's load scheduler, if it has one (LSC_SetFlowLimit checks
 * it against the scheduler buffer size, so it is a byte count despite the parameter name). */
void MWSFLSC_SetFlowLimit(MWPLY mwply, Sint32 nsct) {
    if (mwply->lsc != 0) {
        LSC_SetFlowLimit(mwply->lsc, nsct);
    }
}

/* Whether the load scheduler is in its error state. */
Bool MWSFLSC_IsFsStatErr(LSC lsc) {
    return LSC_GetStat(lsc) == LSC_STAT_ERROR;
}
