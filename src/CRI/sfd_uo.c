/* CRI Sofdec user output driver (sfd_uo.c, SFD_tr_uo, driver slot 8): hands private-stream packets
 * (the Sofdec additional-info / tag data) to stream joints the application registers with
 * SFD_SetUsrSj; the MW player registers its additional-info ring as channel 2. */
#include "CRI/sfd.h"

/* No seek (0). */
Sint32 SFUO_Seek(void) {
    return 0;
}

/* Not supported: error 0xFF000601. */
Sint32 SFUO_AddRead(SFD sfd) {
    return SFLIB_SetErr(sfd, 0xFF000601);
}

/* Not supported: error 0xFF000601. */
Sint32 SFUO_GetRead(SFD sfd) {
    return SFLIB_SetErr(sfd, 0xFF000601);
}

/* Not supported: error 0xFF000601. */
Sint32 SFUO_AddWrite(SFD sfd) {
    return SFLIB_SetErr(sfd, 0xFF000601);
}

/* Not supported: error 0xFF000601. */
Sint32 SFUO_GetWrite(SFD sfd) {
    return SFLIB_SetErr(sfd, 0xFF000601);
}

/* Nothing to do. */
Sint32 SFUO_Pause(void) {
    return 0;
}

/* Nothing to do. */
Sint32 SFUO_Stop(void) {
    return 0;
}

/* Nothing to do. */
Sint32 SFUO_Start(void) {
    return 0;
}

/* Nothing to do. */
Sint32 SFUO_Standby(void) {
    return 0;
}

/* Nothing to do. */
Sint32 SFUO_Destroy(void) {
    return 0;
}

/* Clears the three user output channels and hands them to the buffer. Kept as an inlined helper
 * taking `&sfd->uo_tbl` directly: that gives the original's loop registers. */
static void sfuo_InitCh(SFD sfd, SFUO *uo, Sint32 uobuf) {
    Sint32 i;

    for (i = 0; i < 3; i++) {
        SFUO_CH *ch = &uo->ch[i];

        ch->sj = 0;
        ch->prm = 0;
        ch->rsv1 = 0;
        ch->rsv2 = 0;
        SFBUF_SetUoch(sfd, uobuf, i, ch);
    }
}

/* Attaches the handle's SFUO channel table to slot 8 and clears its 3 channels into buffer 7. */
Sint32 SFUO_Create(SFD sfd) {
    Sint32 uobuf;

    sfd->tr[8].hn = &sfd->uo_tbl;
    uobuf = sfd->tr[8].bufin;
    sfd->uo_tbl.nch = 0;
    sfuo_InitCh(sfd, &sfd->uo_tbl, uobuf);
    return 0;
}

/* Mirrors buffer 7's prepared/terminated flags onto the driver. */
Sint32 SFUO_ExecServer(SFD sfd) {
    if (SFTRN_GetTermFlg(sfd, 8) != 1) {
        if (SFBUF_GetTermFlg(sfd, sfd->tr[8].bufin) == 1) {
            SFTRN_SetTermFlg(sfd, 8, 1);
        }
    }
    if (SFTRN_GetPrepFlg(sfd, 8) != 1) {
        if (SFBUF_GetPrepFlg(sfd, sfd->tr[8].bufin) == 1) {
            SFTRN_SetPrepFlg(sfd, 8, 1);
        }
    }
    return 0;
}

/* Nothing to do. */
Sint32 SFUO_Finish(void) {
    return 0;
}

/* Nothing to do. */
Sint32 SFUO_Init(void) {
    return 0;
}

/* Registers stream joint `sj` (+ parameter) as user-output channel `chno`; the demuxer then copies
 * the matching private stream into it. Error 0xFF000602 when the handle has no user output buffer. */
Sint32 SFD_SetUsrSj(SFD sfd, Sint32 chno, void *sj, void *prm) {
    SFUO *uo;
    Sint32 uobuf;

    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF000191);
    }
    uobuf = sfd->tr[8].bufin;
    uo = sfd->tr[8].hn;
    if (uobuf == 8) {
        return SFLIB_SetErr(sfd, 0xFF000602);
    }
    uo->ch[chno].sj = sj;
    uo->ch[chno].prm = prm;
    uo->ch[chno].rsv1 = 0;
    uo->ch[chno].rsv2 = 0;
    SFBUF_SetUoch(sfd, uobuf, chno, &uo->ch[chno]);
    return 0;
}

const SFD_TR_IF SFD_tr_uo = {
    SFUO_Init, SFUO_Finish, SFUO_ExecServer, SFUO_Create,   SFUO_Destroy, SFUO_Standby, SFUO_Start,
    SFUO_Stop, SFUO_Pause,  SFUO_GetWrite,   SFUO_AddWrite, SFUO_GetRead, SFUO_AddRead, SFUO_Seek,
};
