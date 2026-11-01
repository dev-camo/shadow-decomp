/* CRI Sofdec MW player creation and destruction (mwsfdcre.c): the creation of the SFD handle, the
 * converter and the other parts of a player, the work they are carved from, and their destruction.
 *
 * Names follow the RE4 GameCube decompilation of the same file. Only mwSfdDestroy,
 * MWSFCRE_SetSupplySj and MWSFCRE_ResetSfdHn are written so far; the rest of the original code,
 * data and literals of the file is still taken from the original object.
 */
#include <string.h>

#include "CRI/mwsfd.h"
#include "CRI/mwsfdsvm.h"
#include "CRI/mwstm.h"

/* Gives back every block of component work the player holds. The blocks are carved from the user
 * work (nothing to free) or taken through the library's allocator callbacks. Helper and name
 * inferred, after the same helper in the RE4 decompilation. */
static void mwsfcre_FreeAll(MWPLY mwply) {
    Sint32 i;

    for (i = 0; i < MWSFD_CWK_NUM; i++) {
        void *blk = mwply->cwk_tbl[MWSFD_CWK_NUM - 1 - i];

        if (blk != NULL) {
            if (mwply->cwk_buf == NULL) {
                MWSFD_LIBWORK *lw = MWSFLIB_GetLibWorkPtr();

                lw->free_fn(lw->mem_obj, blk);
            }
            mwply->cwk_cnt--;
            mwply->cwk_tbl[MWSFD_CWK_NUM - 1 - i] = NULL;
        }
    }
}

/* Destroys the SFD handle. */
static inline void MWSFCRE_DestroySfd(SFD sfd) {
    if (SFD_Destroy(sfd) != 0) {
        MWSFLIB_SetErrCode(-0x132);
        MWSFSVM_Error("E20010703E MWSFCRE_DestroySfd: ");
    }
}

/* Destroys the player: the decoder, the converter, the load scheduler, the stream controller and
 * the stream joints, then returns the component work and clears the handle. */
void mwSfdDestroy(MWPLY mwply) {
    if (mwply == NULL) {
        return;
    }
    mwSfdStopDec(mwply);
    mwply->used = 0;
    MWSFTAG_DestroyAinfSj(mwply);
    if (mwply->sfx != NULL) {
        MWSFSFX_Destroy(mwply->sfx);
    }
    if (mwply->lsc != NULL) {
        LSC_Destroy(mwply->lsc);
    }
    if (mwply->stm != NULL) {
        MWSTM_Destroy(mwply->stm);
    }
    if (mwply->file_sj != NULL) {
        SJ_Destroy(mwply->file_sj);
    }
    if (mwply->mem_sj != NULL) {
        SJ_Destroy(mwply->mem_sj);
    }
    if (mwply->sfd != NULL) {
        MWSFCRE_DestroySfd(mwply->sfd);
    }
    MWSST_Destroy(&mwply->sst);
    mwsfcre_FreeAll(mwply);
    if (mwply->cwk_cnt != 0) {
        MWSFSVM_Error("E2053005: forgot free.");
    }
    memset(mwply, 0, sizeof(MWPLY_OBJ));
    mwply->used = 0;
}

/* Gives the SFD handle the player's picture user data buffer (at creation, and again in
 * MWSFCRE_ResetSfdHn). */
static inline void mwsfcre_AttachPicUsrBuf(MWPLY mwply) {
    MWSFD_PICUSR *picusr;
    Uint8 *buf;
    Sint32 usize;
    Sint32 nskip;

    picusr = mwply->picusr_ptr;
    if (picusr == NULL) {
        MWSFSVM_Error("E02120501: Internal Error: mwsfcre_AttachPicUsrBuf().");
        return;
    }
    nskip = mwply->prm.max_skip;
    usize = picusr->usize;
    buf = picusr->buf;
    if (picusr->bsize < (nskip + 3) * usize) {
        MWSFSVM_Error("E02120502: mwsfcre_AttachPicUsrBuf(): usrdatbuf is short.");
        return;
    }
    if (MWSFD_GetUsePicUsr() == 1) {
        SFD_SetPicUsrBuf(mwply->sfd, buf, nskip + 3, usize);
    }
}

/* Stops the SFD handle, installs the player's error function in it again and gives it the
 * picture user data buffer again, for a new play; 0, or -1 when that fails. */
Sint32 MWSFCRE_ResetSfdHn(MWPLY mwply) {
    SFD sfd;

    sfd = mwply->sfd;
    mwply->sfd = NULL;
    if (SFD_Stop(sfd) != 0) {
        MWSFSVM_Error("E0203261: MWSFCRE_ResetSfdHn: SFD_Stop() failed.");
        return -1;
    }
    mwply->sfd = sfd;
    if (SFD_SetErrFn(sfd, MWSFLIB_SfdErrFunc, mwply) != 0) {
        MWSFLIB_SetErrCode(-0x12F);
        MWSFSVM_Error("E0203262: MWSFCRE_ResetSfdHn: SFD_SetErrFn() failed.");
        return -1;
    }
    mwsfcre_AttachPicUsrBuf(mwply);
    return 0;
}

/* Tells the SFD handle where the stream joint of the play gets its data: the memory buffer of a
 * play from memory (kind 1), the ring buffer of the file stream joint (kind 0), or the supply
 * given with the joint. */
void MWSFCRE_SetSupplySj(MWPLY mwply) {
    SFBUF_SUP sup;
    SJ sj = mwply->sji;
    SFD sfd = mwply->sfd;

    if (sj == NULL) {
        return;
    }
    if (sj == mwply->mem_sj) {
        sup.kind = 1;
        sup.sj = sj;
        sup.ofst = (Uint32)mwply->mem_buf;
        sup.size = mwply->mem_size;
        sup.xsize = 0;
        sup.x14 = 0;
    } else if (sj == mwply->file_sj) {
        sup.kind = 0;
        sup.sj = sj;
        sup.ofst = mwply->x1c4;
        sup.size = mwply->flow_nsct;
        sup.xsize = mwply->x1cc;
        sup.x14 = 0;
    } else {
        sup.kind = mwply->sup_kind;
        sup.sj = sj;
        sup.ofst = mwply->sup_ofst;
        sup.size = mwply->sup_size;
        sup.xsize = mwply->sup_xsize;
        sup.x14 = 0;
    }
    if (SFD_SetSupplySj(sfd, &sup) != 0) {
        MWSFLIB_SetErrCode(-0x138);
        MWSFSVM_Error("E20010703B MWSFCRE_SetSupplySj: ");
    }
}
