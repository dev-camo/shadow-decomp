/* CRI Sofdec SFX settings (sfx_set.c): crop of a frame and the settings of a converter handle
 * (division mode, tag information, colour adjustment, output buffer size, component layout).
 *
 * The file name is the one that holds the handle settings in the RE4 GameCube decompilation.
 */
#include <stddef.h>

#include "CRI/sfx.h"
#include "CRI/sj.h"

/* Moves the frame's top left corner to (x, y), rounded toward zero to an even position so that
 * the half-size chroma planes stay aligned, and shortens the planes by the rows cut off. */
void sfxcnv_CropFrm(SFX_SRCFRM *frm, Sint32 x, Sint32 y) {
    y = y / 2 * 2;
    x = x / 2 * 2;
    frm->pln[0].buf += y * frm->pln[0].pitch + x;
    frm->pln[0].height -= y;
    x /= 2;
    y /= 2;
    frm->pln[1].buf += y * frm->pln[1].pitch + x;
    frm->pln[1].height -= y;
    frm->pln[2].buf += y * frm->pln[2].pitch + x;
    frm->pln[2].height -= y;
}

Sint32 SFX_GetBottomUp(SFX_OBJ *sfx) {
    return sfx->bottom_up;
}

Sint32 SFX_GetField0x5C(SFX_OBJ *sfx) {
    return sfx->field_0x5C;
}

Sint32 SFX_GetDivMode(SFX_OBJ *sfx) {
    return sfx->div_mode;
}

/* The tag information given to SFX_SetTagInf, or NULL and 0 when there is none. */
void SFX_GetTagInf(SFX_OBJ *sfx, Uint8 **tag_ptr, Sint32 *tag_len) {
    if (sfx->taginf_flg != 1) {
        *tag_ptr = NULL;
        *tag_len = 0;
    } else {
        *tag_ptr = sfx->tag_ptr;
        *tag_len = sfx->tag_len;
    }
}

/* Stores the tag information and passes its "SFXZ" part to the Z object. */
void SFX_SetTagInf(SFX_OBJ *sfx, Uint8 *tag_ptr, Sint32 tag_len) {
    SJCK inf;
    SJCK out;
    SFXZ_OBJ *sfxz;

    sfxz = sfx->sfxz;
    sfx->tag_ptr = tag_ptr;
    sfx->tag_len = tag_len;
    inf.data = tag_ptr;
    inf.len = tag_len;
    if (SJ_SearchTag(&inf, "SFXZ", "SFXINFE", &out) == NULL) {
        SFXZ_SetTagInf(sfxz, NULL, 0);
    } else {
        SFXZ_SetTagInf(sfxz, out.data, out.len);
    }
    sfx->taginf_flg = 1;
}

void SFX_SetColAdj(SFX_OBJ *sfx, Sint32 coladj) {
    sfx->coladj = coladj;
}

/* The size of the output buffer; 0 stands for the size of the frame. */
void SFX_SetOutBufSize(SFX_OBJ *sfx, Sint32 width, Sint32 height) {
    sfx->outbuf_width = width;
    sfx->outbuf_height = height;
}

void SFX_SetCompoMode(SFX_OBJ *sfx, Sint32 compo) {
    sfx->compo = compo;
}
