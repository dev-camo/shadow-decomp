/* CRI Sofdec manual video output driver (sfd_vom.c, SFD_tr_vo_manu, driver slot 6): the
 * application pulls frames itself (SFD_GetFrm / SFD_RelFrm). GetRead forwards to the frame table
 * buffer for the next decoded frame (in STBY/PLAYING only; no time check here), AddRead returns it
 * to the video decoder's pool. */
#include "CRI/sfd.h"

/* No seek (0). */
Sint32 SFVOM_Seek(void) {
    return 0;
}

/* SFD_RelFrm: gives the frame back to the video decoder through the frame table buffer. */
Sint32 SFVOM_AddRead(SFD sfd, void *frm, Sint32 arg) {
    return SFBUF_VfrmAddRead(sfd, sfd->tr[6].bufin, frm, arg);
}

/* SFD_GetFrm: in STBY/PLAYING asks the frame table for the oldest decoded frame. */
Sint32 SFVOM_GetRead(SFD sfd, void **frm, Sint32 arg) {
    Sint32 ret;
    Sint32 err;

    if ((Uint32)(sfd->stat - 3) > 1) {
        *frm = 0;
        return 0;
    }
    ret = SFBUF_VfrmGetRead(sfd, sfd->tr[6].bufin, frm, arg);
    err = 0;
    if (ret != 0) {
        err = ret;
    }
    return err;
}

/* Not supported: error 0xFF000701. */
Sint32 SFVOM_AddWrite(SFD sfd) {
    return SFLIB_SetErr(sfd, 0xFF000701);
}

/* Not supported: error 0xFF000701. */
Sint32 SFVOM_GetWrite(SFD sfd) {
    return SFLIB_SetErr(sfd, 0xFF000701);
}

Sint32 SFVOM_Pause(void) {
    return 0;
}

Sint32 SFVOM_Stop(void) {
    return 0;
}

Sint32 SFVOM_Start(void) {
    return 0;
}

Sint32 SFVOM_Standby(void) {
    return 0;
}

Sint32 SFVOM_Destroy(void) {
    return 0;
}

Sint32 SFVOM_Create(void) {
    return 0;
}

/* With video enabled: terminates the driver when the frame buffer terminated and the clock has
 * passed the video end (or no clock is used), and mirrors the prepared flag. */
Sint32 SFVOM_ExecServer(SFD sfd) {
    Sint32 term;

    if (SFSET_GetCond(sfd, SFD_COND_VIDEO_ON) == 0) {
        return 0;
    }
    if (SFTRN_GetTermFlg(sfd, 6) != 1) {
        if (SFBUF_GetTermFlg(sfd, sfd->tr[6].bufin) == 1) {
            if (SFSET_GetCond(sfd, 15) == 0) {
                term = 1;
            } else if (SFTIM_IsVideoTerm(sfd) == 0) {
                term = 0;
            } else {
                term = 1;
            }
            if (term != 0) {
                SFTRN_SetTermFlg(sfd, 6, 1);
            }
        }
    }
    if (SFTRN_GetPrepFlg(sfd, 6) != 1) {
        if (SFBUF_GetPrepFlg(sfd, sfd->tr[6].bufin) == 1) {
            SFTRN_SetPrepFlg(sfd, 6, 1);
        }
    }
    return 0;
}

Sint32 SFVOM_Finish(void) {
    return 0;
}

Sint32 SFVOM_Init(void) {
    return 0;
}

const SFD_TR_IF SFD_tr_vo_manu = {
    SFVOM_Init,     SFVOM_Finish,  SFVOM_ExecServer, SFVOM_Create, SFVOM_Destroy,
    SFVOM_Standby,  SFVOM_Start,   SFVOM_Stop,       SFVOM_Pause,  SFVOM_GetWrite,
    SFVOM_AddWrite, SFVOM_GetRead, SFVOM_AddRead,    SFVOM_Seek,
};
