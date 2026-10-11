#include "CRI/adx_stm.h"

/* Leading fields of the ADX stream controller object used here (layout as in the RE4
 * adx_stmc.c reconstruction; later fields are not established for this build).
 */
struct ADXSTM_OBJ {
    /* 0x00 */ Uint8 unknown_0[0x18];
    /* 0x18 */ Sint32 rd_max; /* chunk request size */
    /* 0x1C */ Sint32 rd_min; /* buffer level below which reads are issued */
};

/* Sets the read policy: read when fewer than min_nsct bytes are buffered, at most max_nsct
 * per request.
 */
Sint32 ADXSTM_SetBufSize(ADXSTM stm, Sint32 min_nsct, Sint32 max_nsct) {
    stm->rd_min = min_nsct;
    stm->rd_max = max_nsct;
    return 1;
}
