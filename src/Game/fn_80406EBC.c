#include "CRI/mwsfd.h"
#include "CRI/mwstm.h"

/* Ends the SFD supply once the file stream controller has read its whole range. */
void mwPlyChkSupply(MWPLY mwply) {
    ADXSTM stm = mwply->stm;
    SFD sfd = mwply->sfd;

    if (stm != 0 && MWSTM_GetStat(stm) == ADXSTM_STAT_END) {
        SFD_TermSupply(sfd);
    }
}
