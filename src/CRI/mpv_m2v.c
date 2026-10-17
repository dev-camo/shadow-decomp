/* CRI Sofdec MPV MPEG-2 hooks (mpv_m2v.c): the MPEG-2 video decoder is not built into this library;
 * every MPVM2V_* entry is a stub that fails (-1) or does nothing, so an MPEG-2 stream cannot be
 * decoded (m2v_mode 2 paths in mpv_frm / mpv_hdec).
 *
 * Source from the RE4 GameCube decompilation; the entries are declared with the arguments their
 * callers pass. */
#include "CRI/mpv.h"

/* No MPEG-2 decoder: -1. */
Sint32 MPVM2V_DecodeFrm(MPV_OBJ *mpv, SJ sj, MPV_FRM *frm) {
    return -1;
}

/* No MPEG-2 decoder: -1. */
Sint32 MPVM2V_DecodePicAtr(MPV_OBJ *mpv, SJ sj) {
    return -1;
}

/* No-op. */
void MPVM2V_SetCond(MPV_OBJ *mpv, Sint32 id, Sint32 val) {}

/* No-op. */
void MPVM2V_Destroy(MPV_OBJ *mpv) {}

/* NULL: no MPEG-2 decoder object. */
void *MPVM2V_Create(MPV_OBJ *mpv) {
    return 0;
}

/* No-op. */
void MPVM2V_Finish(void) {}

/* No-op. */
void MPVM2V_Init(void) {}
