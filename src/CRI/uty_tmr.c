/* CRI UTY timer (uty_tmr.c): the PowerPC time base as the Sofdec profiling clock, with its unit
 * (ticks per second) taken from the bus clock. Channel -1 gives a void timer (unit 1; unit 0
 * before initialisation).
 *
 * Structure and the UTY_* names follow the RE4 GameCube decompilation of the same file. This
 * version looks the channel up in the configuration table first (key "TMR_CH"), and its
 * UTY_IsTmrVoid also treats unit 0 as void. */
#include <dolphin/os.h>

#include "CRI/uty.h"

/* Time base register numbers (SPRs) read by UTY_GetTmr. */
#define UTYTMR_TBU 269
#define UTYTMR_TBL 268

static Sint32 utytmr_ch = 0;
/* volatile: UTY_FinishTmr and UTY_InitTmr reload the counter, as the original does */
static volatile Sint32 utytmr_init_cnt = 0;
static Sint64 utytmr_unit;

/* Time base ticks per second (bus clock / 4), or 1 for a void timer. */
Sint64 UTY_GetTmrUnit(void) {
    return utytmr_unit;
}

/* Whether the timer is unusable (unit 1, or 0 before initialisation). */
Bool UTY_IsTmrVoid(void) {
    if (utytmr_unit == 1 || utytmr_unit == 0) {
        return TRUE;
    }
    return FALSE;
}

/* Current time base value (0 when not initialised or void). The upper word is read twice and the
 * read repeated until both reads agree, so a carry from the lower word cannot give a torn value. */
Sint64 UTY_GetTmr(void) {
    register Uint32 hi;
    register Uint32 lo;
    register Uint32 hi2;

    if (utytmr_init_cnt <= 0 || utytmr_ch == -1) {
        return 0;
    }
    // clang-format off
    asm {
    loop:
        mftb hi, UTYTMR_TBU
        mftb lo, UTYTMR_TBL
        mftb hi2, UTYTMR_TBU
        cmpw hi2, hi
        bne loop
    }
    // clang-format on
    return ((Sint64)hi << 32) | lo;
}

/* Drops a reference. */
void UTY_FinishTmr(void) {
    utytmr_init_cnt--;
    /* unused read: the original loads the counter a second time here */
    utytmr_init_cnt;
    if (utytmr_init_cnt < 0) {
        utytmr_init_cnt = 0;
    }
}

/* Reference-counted init for channel `ch` (overridden by the "TMR_CH" configuration entry):
 * unit from the bus clock, or void for ch -1. */
void UTY_InitTmr(Sint32 ch) {
    unsigned int cfg_ch;

    if (fn_803CC9D0("TMR_CH", &cfg_ch) == 0) {
        ch = cfg_ch;
    }
    utytmr_init_cnt++;
    if (utytmr_init_cnt > 1) {
        if (utytmr_ch == ch) {
            return;
        }
    }
    utytmr_ch = ch;
    if (ch == -1) {
        utytmr_unit = 1;
        return;
    }
    utytmr_unit = OS_TIMER_CLOCK;
}
