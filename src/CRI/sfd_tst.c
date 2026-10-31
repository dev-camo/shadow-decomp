/* CRI Sofdec: time stabiliser (sfd_tst.c). Smooths the master clock against a helper clock: the
 * difference history is averaged and the output time base is shifted in tolerance steps. */
#include <string.h>
#include <stdio.h>

#include "CRI/cri_xpt.h"
#include "CRI/sfd.h"
#include "CRI/sfd_tst.h"
#include "CRI/uty.h"
#include "layout_check.h"

Sint32 sftst_debout_siz;
Char8 *sftst_debout_buf;
Char8 *sftst_debout_round;
Char8 *sftst_debout_write;
SFTST sftst_last;

/* Matching workaround: order-only stand-in for the stripped SFTST_SetDebugOut, whose body is
 * unknown. MWCC places .bss objects in order of first reference, so this never-called function
 * refers to the debug log variables in storage order before any other code does. Storing the
 * addresses keeps the references. Revisit when SFTST_SetDebugOut's real body is found. */
static void sftst_BssOrder(void **out) {
    out[0] = &sftst_debout_siz;
    out[1] = &sftst_debout_buf;
    out[2] = &sftst_debout_round;
    out[3] = &sftst_debout_write;
}

/* Clears the difference history and its index. */
static void sftst_ResetHist(SFTST tst) {
    memset(tst->hist, 0, sizeof(tst->hist));
    tst->hist_idx = 0;
    tst->resethist++;
}

/* `t` converted to the given unit: unit * t->cnt / t->unit. */
static Sint64 sftst_Conv(SFTST_TIME *t, Sint64 unit) {
    return unit * t->cnt / t->unit;
}

/* Sum of the history window. */
static Sint32 sftst_SumHist(SFTST tst) {
    Sint32 i;
    Sint32 sum;

    sum = 0;
    for (i = 0; i < tst->movave_range; i++) {
        sum += tst->hist[i];
    }
    return sum;
}

/* Subtracts the correction step from the history window. */
static void sftst_SubHist(SFTST tst, Sint32 sub) {
    Sint32 i;

    for (i = 0; i < tst->movave_range; i++) {
        tst->hist[i] -= sub;
    }
}

/* Matching workaround: the debug log header and the row format are named string objects (names
 * invented), and SFTST_Create copies the header from a struct that wraps the string. The original
 * emitted SFTST_Create's header string in .rodata before SFTST_Calc's format, although SFTST_Calc
 * comes first in .text; a named string is emitted at its declaration, which fixes that order.
 * Counterfactuals (measured against the original object): a local `Char8 hdr[] = "..."` with the
 * format written in the sprintf call puts the format first in .rodata (the .rodata bytes and the
 * targets of the four relocations to the two strings differ; no instruction word differs); a
 * memcpy into a `Char8` array makes SFTST_Create 108 words instead of 116 (83 aligned words
 * differ); naming only the header leaves the format anonymous, so SFTST_Calc's two relocations
 * to it no longer name sftst_msg_fmt (same .rodata bytes). The struct wrapper makes the copy the
 * same inline block copy as the array initialiser. Revisit when the original declarations are
 * found. */
typedef struct {
    Char8 s[0x112]; /* 0x00 the header string's 0x111 characters and its NUL */
} SFTST_HDRSTR;
CHECK_SIZEOF(SFTST_HDRSTR, 0x112);
static const SFTST_HDRSTR sftst_msg_hdr = {
    "tst, help_time_sec, help_time_msec, help_time_64, help_time, mt_max, master_time, out_time,  "
    "mt_ot, mtmax_ot,  diff_l_max, diff_l_min, diff_a_max, tst->diff_a_min, pastat, adjmode, "
    "resethist, excesserr, adj_limit, adj_front, adj_rear,  movave_1st, movave_2nd,  "
    "adxt_stat \n\n"};
static const Char8 sftst_msg_fmt[] =
    "%p, %ld, %ld, %08lX%08lX, %ld, %ld, %ld, %ld,   %ld, %ld,   %ld, %ld, %ld, %ld,   %ld, %ld,   "
    "%ld, %ld, %ld, %ld, %ld,   %ld, %ld,   %ld \n";

void SFTST_Calc(SFTST tst, SFTST_TIME *mt, SFTST_TIME *hlp, SFTST_TIME *out) {
    Sint64 est;
    Sint64 q;
    Sint64 adj;
    Sint64 diff;
    Sint64 adiff;
    Sint64 ave;
    Sint64 aave;
    Sint64 tol;
    Sint64 step;
    Sint32 idx;
    Sint32 d;
    Sint64 t;
    Sint64 excess;
    Char8 buf[0x100];
    Sint32 n;

    if (hlp->unit == 1 || tst->tstflg == 0) {
        *out = *mt;
        return;
    }
    mt->cnt = (tst->mt_max > mt->cnt) ? tst->mt_max : mt->cnt;
    if (tst->pastat == 1) {
        tst->adjmode = 0;
    } else if (tst->adjmode == 0) {
        if (mt->cnt > tst->mt_max) {
            tst->adjmode = 1;
            if (tst->base_hlp == -1) {
                adj = sftst_Conv(&tst->adjstart, mt->unit);
            } else {
                adj = sftst_Conv(&tst->adjpoff, mt->unit);
            }
            tst->base_hlp = hlp->cnt;
            tst->base_out = mt->cnt + adj;
            sftst_ResetHist(tst);
        } else if (tst->adjflg == 0) {
            tst->adjmode = 1;
        }
    }
    tst->mt_max = (tst->mt_max > mt->cnt) ? tst->mt_max : mt->cnt;
    if (tst->base_hlp == -1) {
        est = 0;
    } else {
        est = tst->base_out + mt->unit * (hlp->cnt - tst->base_hlp) / hlp->unit;
    }
    if (tst->adjmode == 0) {
        if (tst->mt_max < est) {
            if (tst->adjflg != 0) {
                tst->base_hlp = hlp->cnt;
                tst->base_out = tst->mt_max;
            } else {
                tst->base_hlp = hlp->cnt;
                tst->base_out = tst->out.cnt;
            }
            tst->adj_limit++;
        }
    } else if (tst->adjflg == 1) {
        /* Matching workaround: the nested assignment keeps the subtraction ahead of the
         * sftst_Conv call; splitting it into two statements changes 49 of SFTST_Calc's 660
         * words. Revisit when the original expression is found. */
        adiff = ((diff = mt->cnt - est) < 0) ? -diff : diff;
        excess = sftst_Conv(&tst->excesserr, mt->unit);
        if (excess < adiff) {
            tst->base_hlp = hlp->cnt;
            tst->base_out = mt->cnt;
            sftst_ResetHist(tst);
            tst->excesserr_cnt++;
        } else {
            idx = tst->hist_idx;
            tst->hist_idx = idx + 1;
            tst->hist[idx % tst->movave_range] = (Sint32)diff;
            ave = sftst_SumHist(tst) / tst->movave_range;
            tst->movave_1st = (Sint32)ave;
            tst->movave_2nd = (Sint32)ave;
            /* Matching workaround: sftst_Conv written out. The call
             * `tol = sftst_Conv(&tst->tolerance, mt->unit)` changes 41 of SFTST_Calc's 660
             * words. Revisit when the original inline setting or source is found. */
            tol = mt->unit * tst->tolerance.cnt / tst->tolerance.unit;
            aave = (ave < 0) ? -ave : ave;
            if (tol < aave) {
                if (tol < ave) {
                    q = ave * 2 / tol - 1;
                    tst->adj_front += (Sint32)q;
                } else {
                    q = ave * 2 / tol + 1;
                    tst->adj_rear += (Sint32)-q;
                }
                step = q * tol / 2;
                tst->base_hlp = hlp->cnt;
                tst->base_out = est + step;
                sftst_SubHist(tst, (Sint32)step);
                tst->movave_2nd = sftst_SumHist(tst) / tst->movave_range;
            }
        }
    }
    if (tst->base_hlp == -1) {
        t = 0;
    } else {
        t = tst->base_out + mt->unit * (hlp->cnt - tst->base_hlp) / hlp->unit;
    }
    out->cnt = t;
    out->unit = mt->unit;
    if (out->cnt < tst->out.cnt) {
        *out = tst->out;
    }
    tst->mt = *mt;
    tst->hlp = *hlp;
    tst->out = *out;
    d = (Sint32)(mt->cnt - out->cnt);
    if (tst->adjmode == 0) {
        tst->diff_l_max = (tst->diff_l_max > d) ? tst->diff_l_max : d;
        tst->diff_l_min = (tst->diff_l_min < d) ? tst->diff_l_min : d;
    } else {
        tst->diff_a_max = (tst->diff_a_max > d) ? tst->diff_a_max : d;
        tst->diff_a_min = (tst->diff_a_min < d) ? tst->diff_a_min : d;
    }
    sftst_last = tst;
    if (sftst_debout_buf != 0) {
        n = sprintf(buf, sftst_msg_fmt, tst, (Sint32)(tst->hlp.cnt / tst->hlp.unit),
                    UTY_MulDiv(1000, (Sint32)tst->hlp.cnt, (Sint32)tst->hlp.unit),
                    (Sint32)(tst->hlp.cnt >> 32), (Sint32)tst->hlp.cnt,
                    (Sint32)(tst->hlp.cnt & 0x7FFFFFFF), (Sint32)tst->mt_max, (Sint32)tst->mt.cnt,
                    (Sint32)tst->out.cnt, (Sint32)(tst->mt.cnt - tst->out.cnt),
                    (Sint32)(tst->mt_max - tst->out.cnt), tst->diff_l_max, tst->diff_l_min,
                    tst->diff_a_max, tst->diff_a_min, tst->pastat, tst->adjmode, tst->resethist,
                    tst->excesserr_cnt, tst->adj_limit, tst->adj_front, tst->adj_rear,
                    tst->movave_1st, tst->movave_2nd, sfadxt_stat);
        strcpy(sftst_debout_write, buf);
        sftst_debout_write += n;
        if (sftst_debout_write >= sftst_debout_buf + sftst_debout_siz - SFTST_DEBOUT_MARGIN) {
            sftst_debout_write = sftst_debout_round;
        }
    }
}

/* Advances the output clock by one frame duration when the stabiliser is not adjusting. */
void SFTST_GoNextFrame(SFTST tst, SFTST_TIME *frm) {
    if (tst->adjflg == 0) {
        tst->out.cnt += tst->out.unit * frm->cnt / frm->unit;
    }
}

/* Adjustment on/off. */
void SFTST_SetAdjFlg(SFTST tst, Sint32 flg) {
    tst->adjflg = flg;
}

/* Pause state of the stabiliser (differences are not accumulated while paused). */
void SFTST_Pause(SFTST tst, Sint32 sw) {
    tst->pastat = sw;
}

/* Moving-average window (frames) for the difference history. */
void SFTST_SetMovaveRange(SFTST tst, Sint32 range) {
    if (range > 0) {
        tst->movave_range = range;
    }
}

/* Offset added to the master time to form the new base when a later adjustment starts. */
void SFTST_SetAdjPoff(SFTST tst, SFTST_TIME *t) {
    tst->adjpoff = *t;
}

/* Offset added to the master time to form the base when the first adjustment starts. */
void SFTST_SetAdjStart(SFTST tst, SFTST_TIME *t) {
    tst->adjstart = *t;
}

/* Difference treated as a gross error (history reset). */
void SFTST_SetExcessErr(SFTST tst, SFTST_TIME *t) {
    tst->excesserr = *t;
}

/* Deviation threshold for a correction; a correction shifts the base by a multiple of half of it. */
void SFTST_SetTolerance(SFTST tst, SFTST_TIME *t) {
    tst->tolerance = *t;
}

/* Stabiliser enable flag. */
void SFTST_SetTstFlg(SFTST tst, Sint32 flg) {
    tst->tstflg = flg;
}

void SFTST_Create(SFTST tst) {
    memset(tst, 0, sizeof(SFTST_WORK));
    tst->tstflg = 1;
    tst->pastat = 0;
    tst->adjmode = 0;
    tst->adjflg = 1;
    tst->movave_range = 10;
    sftst_ResetHist(tst);
    tst->mt.cnt = 0;
    tst->mt.unit = 1;
    tst->hlp.cnt = 0;
    tst->hlp.unit = 1;
    tst->out.cnt = 0;
    tst->out.unit = 1;
    tst->tolerance.cnt = 16683;
    tst->tolerance.unit = 1000000;
    tst->excesserr.cnt = 200000;
    tst->excesserr.unit = 1000000;
    tst->adjstart.cnt = -16683;
    tst->adjstart.unit = 1000000;
    tst->adjpoff.cnt = -16683;
    tst->adjpoff.unit = 1000000;
    tst->base_hlp = -1;
    tst->base_out = 0;
    tst->mt_max = 0;
    tst->adj_limit = 0;
    tst->adj_front = 0;
    tst->adj_rear = 0;
    tst->resethist = 0;
    tst->excesserr_cnt = 0;
    tst->movave_1st = 0;
    tst->movave_2nd = 0;
    tst->diff_l_max = 0;
    tst->diff_l_min = 0;
    tst->diff_a_max = 0;
    tst->diff_a_min = 0;
    {
        /* local copy of the header string (see sftst_msg_hdr) */
        SFTST_HDRSTR hdr = sftst_msg_hdr;

        if (sftst_debout_buf != 0) {
            memset(sftst_debout_buf, 0, sftst_debout_siz);
            sftst_debout_write = sftst_debout_buf;
            strcpy(sftst_debout_write, hdr.s);
            sftst_debout_write += strlen(hdr.s);
            sftst_debout_round = sftst_debout_write;
        }
    }
}
