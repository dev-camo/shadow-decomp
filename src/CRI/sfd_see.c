/* Sofdec seek support: total time / byte rate estimation from the stream headers and the values
 * the user supplies (SFD_SetFileSize / SFD_SetTotTime / SFD_SetByteRate).
 *
 * Source from the RE4 GameCube decompilation with this game's SFD_OBJ layout.
 */
#include "CRI/sfd.h"

/* refresh the byte-rate estimate from whatever is known */
static void sfsee_CalcByteRate(SFD sfd) {
    SFSEE_WORK *wk;
    Sint32 fsize;
    Sint32 tottime;
    Sint32 tunit;

    wk = sfd->see.wk;
    if (wk->byterate > 0) {
        wk->rate = wk->byterate;
        return;
    }
    fsize = wk->fsize;
    tottime = wk->tottime;
    tunit = wk->tunit;
    if (fsize > 0 && tottime > 0) {
        wk->rate = UTY_MulDiv(fsize, tunit, tottime);
        return;
    }
    if (wk->ncount > 0) {
        wk->rate = wk->ncount;
        return;
    }
    if (fsize <= 0) {
        fsize = wk->fsize_est;
    }
    if (tottime <= 0) {
        tottime = wk->tot_est;
        tunit = wk->tunit_est;
    }
    if (fsize > 0 && tottime > 0) {
        wk->rate = UTY_MulDiv(fsize, tunit, tottime);
        return;
    }
    wk->rate = wk->ncount;
}

/* Requested seek position (sectors) into the seek work; ignored without a seek work. */
Sint32 SFD_SetSeekPos(SFD sfd, Sint32 pos) {
    SFSEE_WORK *wk;

    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF00015C);
    }
    wk = sfd->see.wk;
    if (wk == 0) {
        return 0;
    }
    wk->seekpos = pos;
    return 0;
}

/* User-supplied byte rate for seeking; refreshes the rate estimate. */
Sint32 SFD_SetByteRate(SFD sfd, Sint32 rate) {
    SFSEE_WORK *wk;

    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF00015B);
    }
    wk = sfd->see.wk;
    if (wk == 0) {
        return 0;
    }
    wk->byterate = rate;
    sfsee_CalcByteRate(sfd);
    return 0;
}

/* User-supplied total time (count/unit); refreshes the rate estimate. */
Sint32 SFD_SetTotTime(SFD sfd, Sint32 tottime, Sint32 tunit) {
    SFSEE_WORK *wk;

    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF00015A);
    }
    wk = sfd->see.wk;
    if (wk == 0) {
        return 0;
    }
    wk->tottime = tottime;
    wk->tunit = tunit;
    sfsee_CalcByteRate(sfd);
    return 0;
}

/* User-supplied file size in bytes; refreshes the rate estimate. */
Sint32 SFD_SetFileSize(SFD sfd, Sint32 fsize) {
    SFSEE_WORK *wk;

    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF000159);
    }
    wk = sfd->see.wk;
    if (wk == 0) {
        return 0;
    }
    wk->fsize = fsize;
    sfsee_CalcByteRate(sfd);
    return 0;
}

/* total time from the first analysed stream header (video, else audio 1, else audio 2) */
static void sfsee_ExecHeadAnaly(SFD sfd) {
    SFSEE_WORK *wk;
    Sint32 a2;
    Sint32 a1;
    Sint32 wait;
    Sint32 v;
    Sint32 fsize;
    Sint32 rate;
    Sint32 ncount;
    Sint32 tscale;

    wk = sfd->see.wk;
    if (wk->analyzed != 0) {
        return;
    }
    if (SFTRN_IsSetup(sfd, 3) != 0 && SFSET_GetCond(sfd, SFD_COND_AUDIO_ON) == 1) {
        a2 = 1;
        if (wk->a2hdr == 0) {
            wait = 1;
        } else {
            wait = 0;
        }
    } else {
        a2 = 0;
        wait = 0;
    }
    if (wait != 0) {
        return;
    }
    if (SFTRN_IsSetup(sfd, 2) != 0 && SFSET_GetCond(sfd, SFD_COND_VIDEO_ON) == 1) {
        a1 = 1;
        if (wk->a1hdr == 0) {
            wait = 1;
        } else {
            wait = 0;
        }
    } else {
        a1 = 0;
        wait = 0;
    }
    if (wait != 0) {
        return;
    }
    v = (SFTRN_IsSetup(sfd, 1) != 0);
    if (v) {
        wk->shdr.analyzed = 1;
        if (wk->fhd.valid != 0 && wk->fhd.byterate > 0) {
            fsize = wk->fsize;
            rate = wk->fhd.maxplylen_vid;
            if (fsize > 0 && rate > 0) {
                ncount = UTY_MulDiv(fsize, 1000, rate);
            } else {
                ncount = wk->fhd.byterate;
            }
        } else if (wk->fhd.valid != 0 && SFHDS_GetMuxVerNum(sfd) < 108) {
            ncount = (wk->shdr.ncount * 2048) / 2018;
        } else {
            ncount = wk->shdr.ncount;
        }
        tscale = wk->shdr.tscale;
    } else if (a1 != 0) {
        ncount = wk->a1ncount;
        tscale = wk->a1tscale;
    } else if (a2 != 0) {
        ncount = wk->a2ncount;
        tscale = wk->a2tscale;
    } else {
        return;
    }
    wk->ncount = ncount;
    wk->tscale = tscale;
    wk->analyzed = 1;
    sfsee_CalcByteRate(sfd);
}

/* end position of the input stream (-1 while unknown) */
static Sint32 sfsee_GetInputEndPos(SFD sfd) {
    SFD_TR *tr;
    SFD_TR *out;
    Sint32 v;

    tr = sfd->tr;
    out = &tr[sfd->buf[tr[0].bufout].out_tr];
    v = out->x20;
    if (v >= 0) {
        return v;
    }
    return -1;
}

/* estimate the file size and the total time once the input driver knows them */
static void sfsee_ExecEstimate(SFD sfd, SFSEE_HN *see) {
    Sint32 upd;
    Sint32 pos;
    Sint32 endpos;
    SFSEE_WORK *wk;
    SFSEE_REQ *req;

    wk = see->wk;
    req = &see->req;
    if (SFCON_IsEndcodeSkip(sfd) != 0) {
        return;
    }
    upd = 0;
    if (wk->fsize_est <= 0) {
        if (req->pos == -3) {
            pos = 0;
        } else {
            pos = wk->seekpos;
        }
        if (pos >= 0) {
            endpos = sfsee_GetInputEndPos(sfd);
            if (endpos != -1) {
                wk->fsize_est = pos + endpos;
                upd = 1;
            }
        }
    }
    if (wk->tot_est <= 0) {
        if (sfd->con.tottime > 0) {
            wk->tot_est = sfd->con.tottime;
            upd = 1;
            wk->tunit_est = sfd->con.tunit;
        }
    }
    if (upd != 0) {
        sfsee_CalcByteRate(sfd);
    }
}

/* Matching constraint: wk and req are locals of the inlined sfsee_ExecEstimate (req declared after
 * wk), and `see` is a local with three uses; together they give the original register order and
 * the two-step address of req. */
void SFSEE_ExecServer(SFD sfd) {
    SFSEE_HN *see;

    see = &sfd->see;
    if (see->wk == 0) {
        return;
    }
    sfsee_ExecHeadAnaly(sfd);
    sfsee_ExecEstimate(sfd, see);
}

/* Records which of video/audio actually play (set once from sfply_StatPrep) for the seek
 * estimate. */
void SFSEE_FixAvPlay(SFD sfd, Sint32 a, Sint32 b) {
    SFSEE_WORK *wk;

    wk = sfd->see.wk;
    if (wk == 0) {
        return;
    }
    if (wk->av_a < 0) {
        wk->av_a = a;
    }
    if (wk->av_b < 0) {
        wk->av_b = b;
    }
}

/* Attaches the user's seek work (SFSEE_WORK) to the handle; no seek work exists in this game. */
Sint32 SFD_EntrySeek(SFD sfd, SFSEE_WORK *wk) {
    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF000151);
    }
    sfd->see.wk = wk;
    return 0;
}

/* No seek work, no request (pos -3). */
void SFSEE_InitHn(SFSEE_HN *see) {
    see->wk = 0;
    see->req.x00 = 0;
    see->req.pos = -3;
    see->req.x08 = 1;
}
