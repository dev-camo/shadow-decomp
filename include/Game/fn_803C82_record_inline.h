#ifndef GAME_FN_803C82_RECORD_INLINE_H
#define GAME_FN_803C82_RECORD_INLINE_H

#include "CRI/lsc.h"

/* lsc.c error literals (its .rodata is not sourced yet). */
extern Char8 lbl_80516F50[];
extern Char8 lbl_80516F7C[];

#ifdef __cplusplus
extern "C" {
#endif

/* Stops the stream controller (adx_stmc.c). */
void ADXSTM_Stop(ADXSTM stm);
void lsc_ClearEntry(LSC lsc);

#ifdef __cplusplus
}
#endif

/* Body of lsc_ClearEntry: empties the queue of a stopped scheduler. LSC_Stop inlines it. */
static inline void lsc_clear_entry_body(LSC lsc) {
    if (lsc == 0) {
        LSC_CallErrFunc(lbl_80516F7C);
    } else if (lsc->stat == LSC_STAT_STOP) {
        lsc->wr_idx = 0;
        lsc->rd_idx = 0;
        lsc->num_stm = 0;
    }
}

/* Body of LSC_Stop: stops the stream controller this scheduler started and clears the queue.
 * LSC_Destroy inlines it.
 */
static inline void lsc_stop_body(LSC lsc) {
    if (lsc == 0) {
        LSC_CallErrFunc(lbl_80516F50);
    } else if (lsc->stat != LSC_STAT_STOP) {
        lsc->stat = LSC_STAT_STOP;
        if (lsc->stm != 0 && lsc->stm_start == 1) {
            ADXSTM_Stop(lsc->stm);
            lsc->stm_start = 0;
        }
        lsc->cur_nsct = 0;
        lsc_clear_entry_body(lsc);
        lsc->pad34 = 0;
    }
}

#endif
