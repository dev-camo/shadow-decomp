/* CRI Sofdec server time statistics (sfd_tmr.c): sum / min / max / count of 64-bit timer deltas,
 * kept per handle for the server steps (SFD_OBJ.tsum[]).
 * Bodies from the RE4 GameCube decompilation of the same file.
 */
#include "CRI/cri_xpt.h"

typedef struct {
    Sint64 sum; /* 0x00 */
    Sint64 min; /* 0x08 */
    Sint64 max; /* 0x10 */
    Sint32 cnt; /* 0x18 */
} SFTMR_TSUM;

#define SFTMR_MAX(a, b) ((a) > (b) ? (a) : (b))

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

/* Empty statistics (min = INT64_MAX). */
void SFTMR_InitTsum(SFTMR_TSUM *ts) {
    ts->sum = 0;
    ts->min = 0x7FFFFFFFFFFFFFFF;
    ts->max = 0;
    ts->cnt = 0;
}
