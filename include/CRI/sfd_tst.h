#ifndef CRI_SFD_TST_H
#define CRI_SFD_TST_H

/* CRI Sofdec time stabiliser (sfd_tst.c). */
#include "CRI/cri_xpt.h"
#include "layout_check.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 64-bit time: count / unit */
typedef struct {
    Sint64 cnt;  /* 0x00 */
    Sint64 unit; /* 0x08 */
} SFTST_TIME;
CHECK_SIZEOF(SFTST_TIME, 0x10);

#define SFTST_HIST_NUM 60
#define SFTST_DEBOUT_MARGIN 0x400

typedef struct {
    Sint32 tstflg;               /* 0x000 */
    Sint32 pastat;               /* 0x004 pause */
    Sint32 adjmode;              /* 0x008 */
    Sint32 adjflg;               /* 0x00C */
    Sint32 movave_range;         /* 0x010 */
    Sint32 hist_idx;             /* 0x014 */
    Sint32 hist[SFTST_HIST_NUM]; /* 0x018 */
    SFTST_TIME mt;               /* 0x108 last master time */
    SFTST_TIME hlp;              /* 0x118 last helper time */
    SFTST_TIME out;              /* 0x128 last output time */
    SFTST_TIME tolerance;        /* 0x138 */
    SFTST_TIME excesserr;        /* 0x148 */
    SFTST_TIME adjstart;         /* 0x158 */
    SFTST_TIME adjpoff;          /* 0x168 */
    Sint64 base_hlp;             /* 0x178 helper time at the last base (-1: none) */
    Sint64 base_out;             /* 0x180 output time at the last base */
    Sint64 mt_max;               /* 0x188 */
    Sint32 adj_limit;            /* 0x190 */
    Sint32 adj_front;            /* 0x194 */
    Sint32 adj_rear;             /* 0x198 */
    Sint32 resethist;            /* 0x19C */
    Sint32 excesserr_cnt;        /* 0x1A0 */
    Sint32 movave_1st;           /* 0x1A4 */
    Sint32 movave_2nd;           /* 0x1A8 */
    Sint32 diff_l_max;           /* 0x1AC */
    Sint32 diff_l_min;           /* 0x1B0 */
    Sint32 diff_a_max;           /* 0x1B4 */
    Sint32 diff_a_min;           /* 0x1B8 */
    Sint32 rsv;                  /* 0x1BC */
} SFTST_WORK;
CHECK_SIZEOF(SFTST_WORK, 0x1C0);

typedef SFTST_WORK *SFTST;

void SFTST_Create(SFTST tst);
void SFTST_Calc(SFTST tst, SFTST_TIME *mt, SFTST_TIME *hlp, SFTST_TIME *out);
void SFTST_GoNextFrame(SFTST tst, SFTST_TIME *frm);
void SFTST_SetAdjFlg(SFTST tst, Sint32 flg);
void SFTST_Pause(SFTST tst, Sint32 sw);
void SFTST_SetMovaveRange(SFTST tst, Sint32 range);
void SFTST_SetAdjPoff(SFTST tst, SFTST_TIME *t);
void SFTST_SetAdjStart(SFTST tst, SFTST_TIME *t);
void SFTST_SetExcessErr(SFTST tst, SFTST_TIME *t);
void SFTST_SetTolerance(SFTST tst, SFTST_TIME *t);
void SFTST_SetTstFlg(SFTST tst, Sint32 flg);

#ifdef __cplusplus
}
#endif

#endif
