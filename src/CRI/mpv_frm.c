/* CRI Sofdec MPV frame entry points (mpv_frm.c): MPV_DecodeFrmSj decodes one picture from a stream
 * joint into the frame buffers described by MPV_FRM, MPV_SkipFrmSj skips to the next picture, GOP
 * or sequence start code. The graphics quantisation registers are saved and set for the
 * paired-single motion compensation and IDCT kernels around the decode.
 * Bodies from the RE4 GameCube decompilation of the same file.
 */
#include "CRI/dct.h"
#include "CRI/mpv.h"
#include "CRI/uty.h"

/* Start code classes that end a skip: picture 0x04, GOP 0x08, sequence header 0x40, sequence end
 * 0x80.
 */
#define MPVFRM_SKIP_DELIM 0xCC

/* These two entry points take the handle untyped and convert it to MPV_OBJ: the original
 * allocates registers for a separate handle parameter, which an MPV_OBJ * parameter does not give.
 */

/* Advances the stream joint past the current picture: consumes start codes until one of the
 * MPVFRM_SKIP_DELIM classes is reached. Error 0xFF030305 if the data ends first.
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
        if (delim & MPVFRM_SKIP_DELIM) {
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
    Sint32 nfrm;
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
    nfrm = mpv->errinf.nfrm_dec;
    n10 = mpv->errinf.x10;
    mpv->frm = *frm;
    MPVUMC_InitOutRfb(mpv);
    MPVCMC_InitMcOiRt(mpv);
    MPVCMC_SetCcnt(mpv);
    MPVCDEC_InitFrm(mpv);
    ret = MPVHDEC_DecPicture(mpv, sj);
    MPVUMC_EndOfFrame(mpv);
    *frm->picatr = mpv->picatr;
    frm->nfrm = mpv->errinf.nfrm_dec - nfrm;
    frm->x2c = mpv->errinf.x10 - n10;
    UTY_PopGqr(&gqr);
    return ret;
}

/* Nothing to initialise. */
void MPVFRM_Init(void) {}
