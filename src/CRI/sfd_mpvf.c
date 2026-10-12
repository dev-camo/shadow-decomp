/* Sofdec video driver: decoded frame pool management (standby / reference / free frames). */
#include "CRI/sfd.h"

#define SFMPVF_MPV(sfd) ((SFMPV_WORK *)(sfd)->tr[2].hn)

/* frame `a` is displayed before frame `b` (a 0 `b` always loses) */
static Bool sfmpvf_IsPrior(SFMPV_FRM *a, SFMPV_FRM *b) {
    if (b == 0) {
        return TRUE;
    }
    if (a->gopno < b->gopno) {
        return TRUE;
    }
    if (a->gopno > b->gopno) {
        return FALSE;
    }
    if (a->x8c < b->x8c) {
        return TRUE;
    }
    if (a->x8c > b->x8c) {
        return FALSE;
    }
    if (a->x88 < b->x88) {
        return TRUE;
    }
    if (a->x88 > b->x88) {
        return FALSE;
    }
    if (a->xc4 < b->xc4) {
        return TRUE;
    }
    return FALSE;
}

/* the two earliest standby/reference frames */
static void sfmpvf_SearchStbyFrm(SFD sfd, SFMPV_FRM **frm1, SFMPV_FRM **frm2) {
    SFMPV_WORK *mpv;
    SFMPV_FRM *frm;
    Sint32 nfrm;
    Sint32 i;
    Sint32 n;

    mpv = SFMPVF_MPV(sfd);
    nfrm = mpv->nfrm;
    frm = mpv->frm;
    *frm1 = 0;
    *frm2 = 0;
    n = 0;
    for (i = 0; i < nfrm; i++, frm++) {
        if ((frm->stat == SFMPV_FRM_STBY || frm->stat == SFMPV_FRM_REF) && frm->x58 == -1) {
            n++;
            if (sfmpvf_IsPrior(frm, *frm1)) {
                *frm2 = *frm1;
                *frm1 = frm;
            } else if (sfmpvf_IsPrior(frm, *frm2)) {
                *frm2 = frm;
            }
        }
    }
    if (mpv->termflg == 0) {
        n--;
    }
    if (n <= 0) {
        *frm1 = 0;
        *frm2 = 0;
        return;
    }
    if (n == 1) {
        *frm2 = 0;
    }
}

/* While playing: the earliest standby frame after the one being shown, only if its display time has
 * come (unless no clock is used); 0 otherwise. */
SFMPV_FRM *sfmpvf_ReferNextFrmReady(SFD sfd) {
    SFMPV_FRM *frm1;
    SFMPV_FRM *frm;
    Sint32 cs;

    SFLIB_LockCs(&cs);
    if (sfd->stat != SFD_STAT_PLAYING) {
        frm = 0;
    } else {
        sfmpvf_SearchStbyFrm(sfd, &frm1, &frm);
        if (frm != 0) {
            if (SFSET_GetCond(sfd, 15) != 0) {
                if (SFTIM_IsGetFrmTimeTunit(sfd, frm->ftime, frm->tunit) == 0) {
                    frm = 0;
                }
            }
        }
    }
    SFLIB_UnlockCs(&cs);
    return frm;
}

/* the original did not inline sfmpvf_ReferNextFrmReady here */
#pragma dont_inline on
/* Whether another decoded frame is already due (mwPlyGetCurFrm uses it to skip late frames). */
Bool SFD_IsNextFrmReady(SFD sfd) {
    if (SFLIB_CheckHn(sfd) != 0) {
        SFLIB_SetErr(0, 0xFF000183);
        return FALSE;
    }
    return sfmpvf_ReferNextFrmReady(sfd) != 0;
}
#pragma dont_inline off

/* the earliest standby frame; *lastflg is set when it is the last one of a terminated stream */
SFMPV_FRM *SFMPVF_HoldFrm(SFD sfd) {
    Sint32 nfrm;
    Sint32 n;
    SFMPV_FRM *frm;
    SFMPV_WORK *mpv;
    SFMPV_FRM *hold;
    Sint32 i;
    Sint32 cs;

    SFLIB_LockCs(&cs);
    mpv = SFMPVF_MPV(sfd);
    hold = 0;
    n = 0;
    nfrm = mpv->nfrm;
    frm = mpv->frm;
    for (i = 0; i < nfrm; i++, frm++) {
        if ((frm->stat == SFMPV_FRM_STBY || frm->stat == SFMPV_FRM_REF) && frm->x58 == -1) {
            n++;
            if (sfmpvf_IsPrior(frm, hold)) {
                hold = frm;
            }
        }
    }
    if (n == 1 && mpv->termflg == 0 && mpv->gopstat == 0) {
        hold = 0;
    }
    SFLIB_UnlockCs(&cs);
    return hold;
}

/* A frame stops being a reference: REF -> STBY (still to display), anything else -> FREE. */
void SFMPVF_EndRefFrm(SFMPV_FRM *frm) {
    if (frm == 0) {
        return;
    }
    if (frm->stat == SFMPV_FRM_REF) {
        frm->stat = SFMPV_FRM_STBY;
    } else {
        frm->stat = SFMPV_FRM_FREE;
    }
}

/* The application released a displayed frame: REF -> DRAWN (still a reference), else FREE. */
void SFMPVF_EndDrawFrm(SFMPV_FRM *frm) {
    if (frm == 0) {
        return;
    }
    if (frm->stat == SFMPV_FRM_REF) {
        frm->stat = SFMPV_FRM_DRAWN;
    } else {
        frm->stat = SFMPV_FRM_FREE;
    }
    frm->x58 = -1;
}

/* Marks a decoded I/P frame as reference + waiting for display (REF). */
void SFMPVF_RefStbyFrm(SFMPV_FRM *frm) {
    if (frm == 0) {
        return;
    }
    frm->stat = SFMPV_FRM_REF;
}

/* Marks a decoded frame as waiting for display (STBY). */
void SFMPVF_StbyFrm(SFMPV_FRM *frm) {
    if (frm == 0) {
        return;
    }
    frm->stat = SFMPV_FRM_STBY;
}

/* Returns a frame buffer to the pool. */
void SFMPVF_FreeFrm(SFMPV_FRM *frm) {
    if (frm == 0) {
        return;
    }
    frm->stat = SFMPV_FRM_FREE;
}

/* Takes the first FREE, unlocked frame of the pool (state ALLOC) for the picture about to be decoded;
 * 0 when the pool (nfrm_pool + 2 frames) is exhausted. */
SFMPV_FRM *SFMPVF_AllocFrm(SFD sfd) {
    SFMPV_FRM *frm;
    Sint32 i;
    SFMPV_WORK *mpv;
    Sint32 nfrm;
    Sint32 cs;

    SFLIB_LockCs(&cs);
    mpv = SFMPVF_MPV(sfd);
    i = 0;
    nfrm = mpv->nfrm;
    frm = mpv->frm;
    for (; i < nfrm; i++, frm++) {
        if (frm->stat == SFMPV_FRM_FREE && frm->lock == 0) {
            frm->stat = SFMPV_FRM_ALLOC;
            break;
        }
    }
    if (i == nfrm) {
        frm = 0;
    }
    SFLIB_UnlockCs(&cs);
    return frm;
}

/* Decoded frames waiting for display (STBY or REF); -1 when the decoder terminated and none are left. */
Sint32 SFMPVF_GetNumFrm(SFD sfd) {
    SFMPV_WORK *mpv;
    SFMPV_FRM *frm;
    Sint32 i;
    Sint32 n;
    Sint32 cs;

    SFLIB_LockCs(&cs);
    mpv = SFMPVF_MPV(sfd);
    n = 0;
    frm = mpv->frm;
    for (i = 0; i < mpv->nfrm; i++, frm++) {
        if ((frm->stat == SFMPV_FRM_STBY || frm->stat == SFMPV_FRM_REF) && frm->x58 == -1) {
            n++;
        }
    }
    if (mpv->termflg == 1 && n == 0) {
        n = -1;
    }
    SFLIB_UnlockCs(&cs);
    return n;
}

/* GOP state flag (0 after a decode; nonzero while a GOP boundary holds back the last frame). */
void SFMPVF_SetGopStat(SFD sfd, Sint32 stat) {
    SFMPVF_MPV(sfd)->gopstat = stat;
}

/* The video decoder reached the end of the stream. */
Sint32 SFMPVF_IsTermDec(SFD sfd) {
    return SFMPVF_MPV(sfd)->termflg;
}

/* Marks the decoder terminated. */
void SFMPVF_TermDec(SFD sfd) {
    SFMPVF_MPV(sfd)->termflg = 1;
}

/* The SFD_VFRM slot paired with frame object `frm` (same index). */
SFD_VFRM *SFMPVF_SearchVfrmData(SFD sfd, SFMPV_FRM *frm) {
    Sint32 i;
    SFMPV_WORK *mpv;
    SFMPV_FRM *p;

    mpv = SFMPVF_MPV(sfd);
    p = mpv->frm;
    for (i = 0; i < mpv->nfrm; i++, p++) {
        if (p == frm) {
            return &sfd->vfrm[i];
        }
    }
    return 0;
}

/* The frame object paired with the user-visible information block `inf`. */
/* This game only: returns the frame whose x58 equals key (inferred). */
SFMPV_FRM *fn_803F8FC0(SFD sfd, Sint32 key) {
    Sint32 i;
    SFMPV_FRM *frm;

    frm = SFMPVF_MPV(sfd)->frm;
    for (i = 0; i < SFD_VFRM_NUM; i++, frm++) {
        if (frm->x58 == key) {
            return frm;
        }
    }
    return 0;
}

SFMPV_FRM *SFMPVF_SearchFrmObj(SFD sfd, SFD_VFRM_INF *inf) {
    Sint32 i;
    SFMPV_FRM *frm;
    SFD_VFRM *vf;

    frm = SFMPVF_MPV(sfd)->frm;
    vf = sfd->vfrm;
    for (i = 0; i < SFD_VFRM_NUM; i++, vf++) {
        if (&vf->inf == inf) {
            return &frm[i];
        }
    }
    return 0;
}
