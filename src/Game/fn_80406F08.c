/* MWSFPLY_SetFlowLimit: passes MWSFD_SetFlowLimit 80% of the player's flow sector count and the
 * count itself. */
#include "CRI/mwsfd.h"

void MWSFPLY_SetFlowLimit(MWPLY mwply) {
    Sint32 max_nsct = mwply->flow_nsct;

    MWSFD_SetFlowLimit(mwply, (Sint32)(max_nsct * 0.8), max_nsct);
}
