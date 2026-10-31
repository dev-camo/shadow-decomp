/* CRI Sofdec SFX library (sfx_lib.c, "CRI SFX/GC Ver.2.29"): the frame conversion layer between
 * the decoded YCC 4:2:0 frames and the application's textures. The library work holds a table of
 * converter handles; this file creates and destroys them, keeps the error callback and starts
 * and stops the library.
 *
 * Bodies adapted from the RE4 GameCube decompilation of the same file.
 */
#include <string.h>

#include "CRI/cft.h"
#include "CRI/sfx.h"

const Char8 sfx_ver_str[] = "\nCRI SFX/GC Ver.2.29 Build:Feb 28 2005 19:36:34\n";

Sint32 sfx_init_cnt = 0;
Sint32 sfxcnv_forcesplit = 0;
SFX_LIBWORK sfx_libwork;
const Char8 *sfx_dummy;

/* The converter option kept in sfxcnv_forcesplit. The name is inferred from the variable. */
Sint32 SFX_GetForceSplit(void) {
    return sfxcnv_forcesplit;
}

/* CCIR 601 range conversion switch. */
Sint32 SFX_GetCcirFx(void) {
    return sfx_libwork.ccir_fx;
}

/* Counts the error and passes the message to the registered callback. */
void SFXLIB_Error(SFX_OBJ *sfx, SFX_SRCFRM *frm, const Char8 *msg) {
    SFX_ERRFN fn;
    void *obj;

    fn = sfx_libwork.errfn;
    obj = sfx_libwork.errobj;
    sfx_libwork.err_cnt++;
    if (fn != NULL) {
        fn(obj, msg);
    }
}

/* Releases the handle and its Z and alpha objects. SFX_Create also calls it when it fails to
 * create an object, so hn_cnt is then decremented although it was never incremented for that
 * handle (original behaviour). */
void SFX_Destroy(SFX_OBJ *sfx) {
    SFXZ_OBJ *sfxz;
    SFXA_OBJ *sfxa;

    if (sfx == NULL) {
        return;
    }
    sfxz = sfx->sfxz;
    sfxa = sfx->sfxa;
    sfx->used = 0;
    SFXZ_Destroy(sfxz);
    SFXA_Destroy(sfxa);
    sfx_libwork.hn_cnt--;
}

/* The first handle that is not in use, or NULL when all are taken. */
static SFX_OBJ *sfx_GetFreeHn(void) {
    SFX_OBJ *sfx;
    Sint32 i;

    sfx = sfx_libwork.hn;
    for (i = 0; i < sfx_libwork.max_hn; i++) {
        if (sfx->used == 0) {
            return sfx;
        }
        sfx++;
    }
    return NULL;
}

static Bool sfx_IsEnoughWork(Sint32 wsize) {
    return wsize >= SFX_MIN_WORK_SIZE;
}

/* Takes a free handle and sets it up over the caller's work: four 0x400-byte buffers from the
 * 32-byte aligned start of the work, plus a Z object and an alpha object. Returns NULL (and
 * reports an error, except when no handle is free) when the work is too small or an object
 * cannot be created. */
SFX_OBJ *SFX_Create(void *work, Sint32 wsize) {
    SFX_OBJ *sfx;
    SFXZ_OBJ *sfxz;
    SFXA_OBJ *sfxa;

    sfx = sfx_GetFreeHn();
    if (sfx == NULL) {
        return sfx;
    }
    if (sfx_IsEnoughWork(wsize) != TRUE) {
        SFXLIB_Error(NULL, NULL, "E201194: sfx_InitHn: work size is short.");
        return NULL;
    }
    memset(sfx, 0, sizeof(SFX_OBJ));
    sfx->compo = 0;
    sfx->outbuf_width = 0;
    sfx->outbuf_height = 0;
    sfx->field_0x28 = 1;
    sfx->field_0x2C = 0;
    sfx->tbl = 0;
    sfx->buf[0] = (Uint8 *)(((Uint32)work + 0x1F) & ~0x1F);
    sfx->buf[1] = sfx->buf[0] + SFX_BUF_SIZE;
    sfx->buf[2] = sfx->buf[1] + SFX_BUF_SIZE;
    sfx->buf[3] = sfx->buf[2] + SFX_BUF_SIZE;
    sfx->work = work;
    sfx->wsize = wsize;
    sfx->div_mode = -1;
    sfx->bottom_up = 0;
    sfx->used = 1;
    sfxz = SFXZ_Create();
    if (sfxz == NULL) {
        SFXLIB_Error(NULL, NULL, "E201281: SfxZHn: can't create.");
        SFX_Destroy(sfx);
        return NULL;
    }
    sfx->sfxz = sfxz;
    sfxa = SFXA_Create();
    if (sfxa == NULL) {
        SFXLIB_Error(NULL, NULL, "E202011: SfxAHn: can't create.");
        SFX_Destroy(sfx);
        return NULL;
    }
    sfx->sfxa = sfxa;
    sfx_libwork.hn_cnt++;
    return sfx;
}

/* Installs the error callback. */
void SFX_SetErrFn(SFX_ERRFN fn, void *obj) {
    sfx_libwork.errfn = fn;
    sfx_libwork.errobj = obj;
}

/* Stops the modules started by SFX_Init. The name is inferred: the counterpart of SFX_Init. */
void SFX_Finish(void) {
    if (sfx_init_cnt > 0) {
        SFXZ_Finish();
        SFXA_Finish();
        SFXSUD_Finish();
        CFT_Finish();
        sfx_init_cnt--;
    }
}

/* One-time library start: clears the work (eight handles, CCIR range on) and starts the colour
 * format, user data, Z and alpha modules. */
void SFX_Init(void) {
    if (sfx_init_cnt < 1) {
        sfx_dummy = sfx_ver_str;
        memset(&sfx_libwork, 0, sizeof(sfx_libwork));
        sfx_libwork.max_hn = SFX_MAX_HN;
        sfx_libwork.ccir_fx = 1;
        CFT_Init();
        SFXSUD_Init();
        SFXZ_Init();
        SFXA_Init();
        sfxcnv_forcesplit = 0;
        sfx_init_cnt++;
    }
}
