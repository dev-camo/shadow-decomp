/* CRI Sofdec MPV state accessors (mpv_get.c): GOP link flags, VBV buffer size/delay, bit rate and a
 * copy of the current picture attributes (MPV_PICATR) for the SFD video driver.
 * Bodies from the RE4 GameCube decompilation of the same file.
 */
#include "CRI/mpv.h"

/* closed_gop and broken_link of the last GOP header. */
Sint32 MPV_GetLinkFlg(MPV_OBJ *mpv, Sint32 *flg1, Sint32 *flg2) {
    if (MPVLIB_CheckHn(mpv) != 0) {
        return MPVERR_SetCode(0, 0xFF03020E);
    }
    *flg1 = mpv->linkflg1;
    *flg2 = mpv->linkflg2;
    return 0;
}

/* vbv_buffer_size in bytes (<< 11), vbv_delay, and the delay in bytes (delay * bitrate / 1800; -1
 * for variable bit rate 0x3FFFF). */
Sint32 MPV_GetVbvBufSiz(MPV_OBJ *mpv, Sint32 *bufsiz, Sint32 *delay, Sint32 *delay_byte) {
    if (MPVLIB_CheckHn(mpv) != 0) {
        return MPVERR_SetCode(0, 0xFF03020F);
    }
    *bufsiz = mpv->vbv_size << 11;
    *delay = mpv->vbv_delay;
    if (mpv->bitrate == 0x3FFFF) {
        *delay_byte = -1;
    } else {
        *delay_byte = (mpv->vbv_delay * mpv->bitrate) / 1800;
    }
    return 0;
}

/* bit_rate of the sequence header (400 bit/s units). */
Sint32 MPV_GetBitRate(MPV_OBJ *mpv, Sint32 *bitrate) {
    if (MPVLIB_CheckHn(mpv) != 0) {
        return MPVERR_SetCode(0, 0xFF03020D);
    }
    *bitrate = mpv->bitrate;
    return 0;
}

/* Copy of the picture attributes of the last decoded headers (size, rate, temporal reference, type,
 * GOP timecode, header counts). */
Sint32 MPV_GetPicAtr(MPV_OBJ *mpv, MPV_PICATR *picatr) {
    if (MPVLIB_CheckHn(mpv) != 0) {
        return MPVERR_SetCode(0, 0xFF03020C);
    }
    *picatr = mpv->picatr;
    return 0;
}
