/* CRI Sofdec MPV frame entry points (mpv_frm.c): MPV_DecodeFrmSj decodes one picture from a stream
 * joint into the frame buffers described by MPV_FRM, MPV_SkipFrmSj skips to the next picture, GOP,
 * sequence header or sequence end code. The graphics quantisation registers are saved and set for
 * the paired-single motion compensation and IDCT kernels around the decode.
 * Bodies from the RE4 GameCube decompilation of the same file.
 */
#include "CRI/dct.h"
#include "CRI/mpv.h"
#include "CRI/uty.h"

/* Matching workaround: MPV_SkipFrmSj and MPV_DecodeFrmSj (like MPV_DecodePicAtrSj in mpv_hdec.c)
 * take the handle as void * and convert it to MPV_OBJ in a local. With an MPV_OBJ * parameter they
 * differ in 9 and 50 words (MPV_DecodePicAtrSj in 98): the handle and the stream joint are
 * allocated to other registers. Revisit when the original handle type is known. */

/* Advances the stream joint past the current picture: consumes start codes until one of the
 * MPV_DLM_PIC_BOUND classes is reached. Error 0xFF030305 if the data ends first.
 */
Sint32 MPV_SkipFrmSj(void *hn, SJ sj) {
    MPV_OBJ *mpv = hn;
    Sint32 code;
    Sint32 delim;

    if (MPVLIB_CheckHn(mpv) != 0) {
        return MPVERR_SetCode(0, 0xFF03020A);
    }
    code = 0xFF030305;
    for (;;) {
        delim = MPV_GoNextDelimSj(sj);
        if (delim == 0) {
            break;
        }
        if (delim & MPV_DLM_PIC_BOUND) {
            code = 0;
            break;
        }
        if (MPV_MoveChunk(sj, 1, 4) != 4) {
            break;
        }
    }
    return MPVERR_SetCode(mpv, code);
}

/* Decodes the picture at the front of sj into frm's buffers: saves and sets the GQRs, initialises
 * the output reference planes and the motion compensation block tables, runs the picture header
 * and slice decoder, then returns the picture attributes and the picture counts of the call in
 * *frm.
 * MPEG-2 streams go to the M2V decoder, which this library does not contain.
 */
Sint32 MPV_DecodeFrmSj(void *hn, SJ sj, MPV_FRM *frm) {
    MPV_OBJ *mpv = hn;
    UTY_GQR gqr;
    Sint32 n0c;
    Sint32 n10;
    Sint32 ret;

    if (MPVLIB_CheckHn(mpv) != 0) {
        return MPVERR_SetCode(0, 0xFF030209);
    }
    UTY_PushGqr(&gqr);
    MPVUMC_SetGqr();
    DCT_FsriSetGqr();
    if (mpv->m2v_mode == 2) {
        return MPVM2V_DecodeFrm(mpv, sj, frm);
    }
    n0c = mpv->errinf.x0c;
    n10 = mpv->errinf.x10;
    mpv->frm = *frm;
    MPVUMC_InitOutRfb(mpv);
    MPVCMC_InitMcOiRt(mpv);
    MPVCMC_SetCcnt(mpv);
    MPVCDEC_InitFrm(mpv);
    ret = MPVHDEC_DecPicture(mpv, sj);
    MPVUMC_EndOfFrame(mpv);
    *frm->picatr = mpv->picatr;
    frm->x28 = mpv->errinf.x0c - n0c;
    frm->x2c = mpv->errinf.x10 - n10;
    UTY_PopGqr(&gqr);
    return ret;
}

/* Nothing to initialise. */
void MPVFRM_Init(void) {}
