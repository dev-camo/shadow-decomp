#include "CRI/adx_stm.h"

void fn_804034B0(ADXSTM stm, Sint32 min_nsct, Sint32 max_nsct) {
    if (stm != 0) {
        ADXSTM_SetBufSize(stm, min_nsct, max_nsct);
    }
}
