#include "CRI/mwsfd.h"

/* Sets the library work's "decode server running" flag. */
void MWSFSVR_SetMwsfdSvrFlg(Sint32 flg) {
    MWSFLIB_GetLibWorkPtr()->svr_flg = flg;
}
