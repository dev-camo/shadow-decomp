/* CRI Sofdec timer code (sfd_tmr.c): the profiling clock read by the player, and the sum / min /
 * max / count statistics of 64-bit timer deltas kept per handle for the server steps
 * (SFD_OBJ.tsum[]).
 *
 * The statistics bodies are from the RE4 GameCube decompilation of the same file. The unit is
 * built with deferred inlining, which emits the functions in reverse source order: that is why
 * fn_803FEA5C is defined before fn_803FE96C, which expands it (its result is unused).
 */
#include "CRI/sfd.h"
#include "CRI/uty.h"

#define SFTMR_MAX(a, b) ((a) > (b) ? (a) : (b))

/* Clock units per second of the last clock read (name inferred); 0 until the first read. */
static Sint64 sftmr_unit;

/* Reads the clock and records its units per second. Without a usable UTY timer the clock is the
 * external clock source of the last handle, or else the vsync count x 1000 in units of the refresh
 * rate x 1000 per second. */
Sint64 fn_803FEA5C(void) {
    SFD hn;
    Sint32 count;
    Sint32 unit;

    if (!UTY_IsTmrVoid()) {
        sftmr_unit = UTY_GetTmrUnit();
        return UTY_GetTmr();
    }
    hn = sfd_hn_last;
    if (hn != 0 && hn->stat != SFD_STAT_NONE && SFD_TIM(hn)->extfn != 0) {
        SFD_TIM(hn)->extfn(SFD_TIM(hn)->ext_obj, &count, &unit);
        sftmr_unit = unit;
        return count;
    }
    sftmr_unit = SFTIM_LIBWK->vrate;
    return SFTIM_LIBWK->vcnt * 1000;
}

/* Clock units per second; reads the clock once to find them. */
Sint64 fn_803FE96C(void) {
    if (sftmr_unit == 0) {
        fn_803FEA5C();
    }
    return sftmr_unit;
}

/* Empty statistics (min = INT64_MAX). */
void SFTMR_InitTsum(SFTMR_TSUM *ts) {
    ts->sum = 0;
    ts->min = 0x7FFFFFFFFFFFFFFF;
    ts->max = 0;
    ts->cnt = 0;
}

/* Adds one measured duration to the statistics. */
void SFTMR_AddTsum(SFTMR_TSUM *ts, Sint64 t) {
    Sint64 min;

    ts->sum += t;
    min = ts->min;
    if (t < min) {
        min = t;
    }
    ts->min = min;
    ts->max = SFTMR_MAX(t, ts->max);
    ts->cnt++;
}
