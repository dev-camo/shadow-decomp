/* CRI stream-joint critical section (sj_crs.c): a nesting interrupt lock and the SJ library
 * init count. Names follow the Xenoblade CRI sj_crs.c, whose bodies match this file.
 */
#include <dolphin/os.h>

#include "CRI/sj.h"

/* The nesting level and the init count are volatile: in the original each update is followed by
 * a reload, which MWCC emits only for a volatile object. Declared plain (measured per function
 * against the original): sjcrs_lvl makes SJCRS_Unlock 17 words instead of 19 (10 aligned words
 * differ); sj_init_count makes SJCRS_Init 11 words instead of 12 (7 differ) and SJCRS_Finish 10
 * instead of 12 (6 differ). */
volatile Sint32 sjcrs_lvl;
Sint32 sjcrs_msk;
static volatile Sint32 sj_init_count;

void SJCRS_Unlock(void) {
    sjcrs_lvl--;
    if (sjcrs_lvl == 0) {
        OSRestoreInterrupts(sjcrs_msk);
    }
}

void SJCRS_Lock(void) {
    if (sjcrs_lvl == 0) {
        sjcrs_msk = OSDisableInterrupts();
    }
    sjcrs_lvl++;
}

/* Releases one SJ library user; the last one resets the lock nesting. */
void SJCRS_Finish(void) {
    sj_init_count--;
    if (sj_init_count == 0) {
        sjcrs_lvl = 0;
    }
}

/* Registers one SJ library user; the first one resets the lock nesting. */
void SJCRS_Init(void) {
    sj_init_count++;
    if (sj_init_count == 1) {
        sjcrs_lvl = 0;
    }
}
