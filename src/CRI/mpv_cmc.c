/* CRI Sofdec MPEG video motion-compensation setup (mpv_cmc.c): the output block tables (chroma
 * count and the six block descriptors) and the library table of the 8x8 one-reference kernels.
 *
 * Source from the RE4 GameCube decompilation, unchanged apart from using the shared MPV_OBJ
 * layout. */
#include "CRI/mpv.h"

void (*mpvcmc_oneref[8])(MPVMC *mc);

/* Chroma block count for the output block tables: 4 when motion compensation writes chroma
 * (cond[5]), -1 otherwise.
 */
void MPVCMC_SetCcnt(MPV_OBJ *mpv) {
    Sint32 ccnt;

    if (mpv->cond[5] == 0) {
        ccnt = -1;
    } else {
        ccnt = 4;
    }
    mpv->ob.ccnt = ccnt;
    mpv->ob_rt.ccnt = ccnt;
}

/* The real-time output table: chroma count and the frame width/height as the block strides. */
void MPVCMC_InitMcOiRt(MPV_OBJ *mpv) {
    MPVCMC_OUTBLK *ob = &mpv->ob_rt;
    MPVCMC_REF *oi = ob->oi;
    Sint32 ccnt;
    Sint32 i;
    Sint32 w;
    Sint32 h;

    if (mpv->cond[5] == 0) {
        ccnt = -1;
    } else {
        ccnt = 4;
    }
    ob->ccnt = ccnt;
    w = mpv->frm.width;
    for (i = 0; i < 2; i++) {
        oi[i].n = w;
    }
    h = mpv->frm.height;
    for (i = 2; i < 6; i++) {
        oi[i].n = h;
    }
}

/* Per-handle motion-compensation setup: installs the 8x8 and 16x16 kernel tables, points the six
 * output block descriptors at the work buffer with 8-pixel rows.
 */
void MPVCMC_InitObj(MPV_OBJ *mpv) {
    Uint8 *work;
    MPVCMC_OUTBLK *ob;
    MPVCMC_REF *oi;
    Sint32 ccnt;
    Sint32 i;

    MPVMC08_Init(&mpv->mc);
    MPVMC16_Init(&mpv->mc);
    ob = &mpv->ob;
    oi = ob->oi;
    work = mpv->work;
    if (mpv->cond[5] == 0) {
        ccnt = -1;
    } else {
        ccnt = 4;
    }
    ob->ccnt = ccnt;
    for (i = 0; i < 6; i++) {
        oi[i].p = work;
    }
    for (i = 0; i < 6; i++) {
        oi[i].n = 8;
    }
}

/* Library table of the 8x8 one-reference kernels by half-pel case: 1p, H2, V2, 4p, then 1p, H2,
 * V2 and V2 again (entry 7 repeats V2 rather than 4p).
 */
void MPVCMC_Init(void) {
    mpvcmc_oneref[0] = MPVMC08_OneRef1p_TuneC;
    mpvcmc_oneref[1] = MPVMC08_OneRefH2_TuneC;
    mpvcmc_oneref[2] = MPVMC08_OneRefV2_TuneC;
    mpvcmc_oneref[3] = MPVMC08_OneRef4p_TuneC;
    mpvcmc_oneref[4] = MPVMC08_OneRef1p_TuneC;
    mpvcmc_oneref[5] = MPVMC08_OneRefH2_TuneC;
    mpvcmc_oneref[6] = MPVMC08_OneRefV2_TuneC;
    mpvcmc_oneref[7] = MPVMC08_OneRefV2_TuneC;
}
