/* CRI Sofdec MPV macroblock coefficient decode (mpv_cdec.c): runs the intra or non-intra block
 * decoder over the six 8x8 blocks of a macroblock, then the IDCT installed in the handle.
 * Bodies from the RE4 GameCube decompilation of the same file; this version calls the IDCT through
 * the handle (x2e4, x2e8) instead of directly.
 */
#include "CRI/mpv.h"

#define MPVCDEC_NBLK 6

/* Clears one 8x8 block (0x100 bytes) with 32 8-byte stores through a stepping cursor. */
static inline void mpvcdec_ClearBlk(Float64 **cur) {
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
    *(*cur)++ = 0.0;
}

/* Non-intra macroblock: for each of the 6 blocks flagged in the coded block pattern runs the
 * non-intra coefficient decoder (nintra_func) into blk[i] with the non-intra matrix and the
 * current qscale; then the IDCT of the coded blocks (x2e8). */
Sint32 MPVCDEC_NintraBlocks(MPV mpv) {
    Sint32 msk;
    Sint32 i;
    MPV_BLKPRM *prm;
    DCT_PA *pa;

    pa = &mpv->dctpa;
    prm = &mpv->blkprm;
    prm->qscale = mpv->qscale;
    prm->iqm = mpv->nintra_iqm;
    prm->nintra = 1;
    msk = mpv->cbp_code << 2;
    pa->cbp_msk = msk;
    for (i = 0; i < MPVCDEC_NBLK; i++) {
        if (msk < 0) {
            prm->dst = mpv->blk[i];
            pa->cbp[i] = mpv->nintra_func(mpv, prm);
        }
        msk <<= 1;
    }
    mpv->x2e8(pa);
    return 0;
}

/* Intra macroblock: clears the 6 coefficient blocks, decodes all six with the intra decoder
 * (intra_func, DC prediction per component, intra matrix) and runs the IDCT on all six (x2e4). */
Sint32 MPVCDEC_IntraBlocks(MPV mpv) {
    MPV_BLKPRM *prm;
    DCT_PA *pa;
    Float64 *cur;

    cur = (Float64 *)mpv->blk[0];
    mpvcdec_ClearBlk(&cur);
    mpvcdec_ClearBlk(&cur);
    mpvcdec_ClearBlk(&cur);
    mpvcdec_ClearBlk(&cur);
    mpvcdec_ClearBlk(&cur);
    mpvcdec_ClearBlk(&cur);
    prm = &mpv->blkprm;
    pa = &mpv->dctpa;
    prm->qscale = mpv->qscale;
    prm->iqm = mpv->intra_iqm;
    prm->nintra = 0;
    prm->dctbl = mpv->dctbl_y;
    prm->dcpred = &mpv->dcpred[0];
    prm->dst = mpv->blk[0];
    pa->cbp[0] = mpv->intra_func(mpv, prm);
    prm->dst = mpv->blk[1];
    pa->cbp[1] = mpv->intra_func(mpv, prm);
    prm->dst = mpv->blk[2];
    pa->cbp[2] = mpv->intra_func(mpv, prm);
    prm->dst = mpv->blk[3];
    pa->cbp[3] = mpv->intra_func(mpv, prm);
    prm->dctbl = mpv->dctbl_c;
    prm->dcpred = &mpv->dcpred[1];
    prm->dst = mpv->blk[4];
    pa->cbp[4] = mpv->intra_func(mpv, prm);
    prm->dcpred = &mpv->dcpred[2];
    prm->dst = mpv->blk[5];
    pa->cbp[5] = mpv->intra_func(mpv, prm);
    mpv->x2e4(pa);
    return 0;
}

/* Nothing per frame. */
void MPVCDEC_InitFrm(MPV mpv) {}
