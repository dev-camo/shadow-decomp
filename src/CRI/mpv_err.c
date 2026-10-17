/* CRI Sofdec MPV error reporting (mpv_err.c): the per-handle / library error record (callback,
 * object, last code) and MPVERR_SetCode, the return path of every MPV_* API (0xFF03xxxx codes).
 * The SFD video driver installs sfmpv_ErrFn.
 * Bodies from the RE4 GameCube decompilation of the same file.
 */
#include "CRI/mpv.h"

MPVERR_INF mpverrinf;

/* Records `code` on the handle (or the library when mpv is 0) and calls the error callback for a
 * nonzero code; returns the code. */
Sint32 MPVERR_SetCode(MPV_OBJ *mpv, Sint32 code) {
    if (mpv == 0) {
        mpverrinf.code = code;
        if (code != 0) {
            if (mpverrinf.func != 0) {
                mpverrinf.func(mpverrinf.obj, code);
            }
        }
    } else {
        mpv->errinf.code = code;
        if (code != 0) {
            if (mpv->errinf.func != 0) {
                mpv->errinf.func(mpv->errinf.obj, code);
            }
        }
    }
    return code;
}

/* Installs the handle's error callback (func(obj, code)). */
Sint32 MPV_SetErrFunc(MPV_OBJ *mpv, void (*func)(void *obj, Sint32 code), void *obj) {
    if (MPVLIB_CheckHn(mpv) != 0) {
        return MPVERR_SetCode(0, 0xFF030203);
    }
    mpv->errinf.func = func;
    mpv->errinf.obj = obj;
    return 0;
}

/* Clears an error record. */
void MPVERR_InitErrInf(MPVERR_INF *inf) {
    inf->func = 0;
    inf->obj = 0;
    inf->code = 0;
    inf->x0c = 0;
    inf->x10 = 0;
}

/* Clears the library-wide error record. */
void MPVERR_Init(void) {
    MPVERR_InitErrInf(&mpverrinf);
}
