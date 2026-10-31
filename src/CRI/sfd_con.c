/* CRI Sofdec concatenated playback (sfd_con.c, SFCON): when several files are streamed back to back
 * (cond 0x31, SFD_SetConcatPlay) the end codes are skipped and the clock is offset by the
 * accumulated duration of the finished files; a 32-entry queue passes each file's total sample
 * count from the demuxer to the audio driver. Unused by this game (single-file movies).
 *
 * Source from the RE4 GameCube decompilation; this game's SFD_OBJ holds the work at 0xD30. */
#include "CRI/sfd.h"

/* Pops the next total-sample entry of the concatenation queue (the audio driver reads the sample
 * count of each concatenated file); *val = -1 and 0 when empty. */
Sint32 SFCON_ReadTotSmplQue(SFD sfd, Sint32 *val, Sint32 *last) {
    SFTIM tim = SFD_TIM(sfd);
    Sint32 cs;
    Sint32 ret;

    SFLIB_LockCs(&cs);
    if (tim->que_wr - tim->que_rd <= 0) {
        *val = -1;
        ret = 0;
    } else {
        *last = tim->tot_last;
        ret = 1;
        *val = tim->tot_que[tim->que_rd % SFCON_QUE_NUM];
        tim->que_rd++;
    }
    SFLIB_UnlockCs(&cs);
    return ret;
}

/* Pushes a file's total sample count onto the 32-entry queue (from the demuxer at each concatenated
 * file); 0 when full. */
Sint32 SFCON_WriteTotSmplQue(SFD sfd, Sint32 val, Sint32 last) {
    SFTIM tim = SFD_TIM(sfd);
    Sint32 cs;
    Sint32 ret;

    SFLIB_LockCs(&cs);
    if (tim->que_wr - tim->que_rd >= SFCON_QUE_NUM) {
        ret = 0;
    } else {
        tim->tot_last = last;
        ret = 1;
        tim->tot_que[tim->que_wr % SFCON_QUE_NUM] = val;
        tim->que_wr++;
    }
    SFLIB_UnlockCs(&cs);
    return ret;
}

/* Adds a finished file's duration to the accumulated concatenation time and records it in the
 * per-file ring (the clock adds it to the next file's timestamps). */
void SFCON_UpdateConcatTime(SFD sfd, Sint32 t) {
    SFTIM tim = SFD_TIM(sfd);
    Sint32 cs;
    Sint32 idx;

    SFLIB_LockCs(&cs);
    tim->ctime += t;
    idx = tim->ctime_idx;
    idx++;
    tim->ctime_que[idx % SFCON_QUE_NUM] = tim->ctime;
    tim->ctime_idx = idx;
    SFLIB_UnlockCs(&cs);
}

/* Concatenated play (cond 0x31) or video end-code skip (0x39): a sequence end code does not end
 * the video. */
Sint32 SFCON_IsVideoEndcodeSkip(SFD sfd) {
    if (SFSET_GetCond(sfd, 0x31) != 0 || SFSET_GetCond(sfd, 0x39) != 0) {
        return 1;
    }
    return 0;
}

/* Concatenated play or system end-code skip (0x38): a program end code does not end the demux. */
Sint32 SFCON_IsSystemEndcodeSkip(SFD sfd) {
    if (SFSET_GetCond(sfd, 0x31) != 0 || SFSET_GetCond(sfd, 0x38) != 0) {
        return 1;
    }
    return 0;
}

/* Concatenated play is on (cond 0x31). */
Sint32 SFCON_IsEndcodeSkip(SFD sfd) {
    return SFSET_GetCond(sfd, 0x31) != 0;
}

/* Enables concatenated play: the streams continue across end codes (linked file playback,
 * mwPlyLinkStm). */
Sint32 SFD_SetConcatPlay(SFD sfd) {
    if (SFLIB_CheckHn(sfd) != 0) {
        return SFLIB_SetErr(0, 0xFF000161);
    }
    SFSET_SetCond(sfd, 0x31, 1);
    return 0;
}
