#include "CRI/lsc.h"

/* lsc.c error literal (its .rodata is not sourced yet). */
extern Char8 lbl_80516CC0[];

/* Sets the scheduler's loop flag (LSC/GC Ver.2.22 addition; original name unknown). */
void fn_803C80C4(LSC lsc, Sint32 flag) {
    if (lsc == 0) {
        LSC_CallErrFunc(lbl_80516CC0);
    } else {
        lsc->loop_flag = flag;
    }
}
