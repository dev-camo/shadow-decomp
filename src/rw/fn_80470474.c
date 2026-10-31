/* RenderWare 2D toolkit brushes: opening and closing the brush free list, creating and destroying
 * brushes, and setting a brush's colours, texture coordinates and texture. The original file name
 * is unknown. */
#include "rw/rt2d.h"
#include <string.h>

RwFreeList lbl_805E18E0; /* the brush free list */

RwInt32 lbl_805EDDF0 = 8; /* brush free list entries per block */
RwInt32 lbl_805EDDF4 = 1; /* brush free list blocks to preallocate */

void fn_80470474(void) {
    fn_8047FCDC(lbl_805E1950.brushFreeList);
    lbl_805E1950.brushFreeList = NULL;
    RwFree(lbl_805E1950.vertexBuffer);
    lbl_805E1950.vertexBuffer = NULL;
}

RwBool fn_804704D4(void) {
    lbl_805E1950.vertexBuffer = RwMalloc(0x900, rwMEMHINTDUR_GLOBAL | rwID_2DPLUGIN);
    lbl_805E1950.brushFreeList = fn_8047FC04(sizeof(Rt2dBrush), lbl_805EDDF0, 16, lbl_805EDDF4,
                                             &lbl_805E18E0, rwMEMHINTDUR_GLOBAL | rwID_2DBRUSH);
    return TRUE;
}

/* Creates a white, untextured brush of half width 0.5 */
Rt2dBrush *fn_8047054C(void) {
    Rt2dBrush *brush;

    brush = RwFreeListAlloc(lbl_805E1950.brushFreeList, rwMEMHINTDUR_EVENT | rwID_2DBRUSH);
    if (brush != NULL) {
        RwRGBA white;
        RwV2d zero;

        memset(brush, 0, sizeof(Rt2dBrush));

        white.red = 255;
        white.green = 255;
        white.blue = 255;
        white.alpha = 255;
        fn_80470694(brush, &white, &white, &white, &white);

        zero.x = 0.0f;
        zero.y = 0.0f;
        fn_80470998(brush, &zero, &zero, &zero, &zero);

        fn_80470A4C(brush, NULL);
        brush->halfwidth = 0.5f;
        brush->refCount = 1;
        return brush;
    }
    return NULL;
}

/* Releases a reference to a brush and frees it with its texture reference after the last */
RwBool fn_80470620(Rt2dBrush *brush) {
    if (--brush->refCount <= 0) {
        if (brush->texture != NULL) {
            fn_8048CD8C(brush->texture);
        }
        RwFreeListFree(lbl_805E1950.brushFreeList, brush);
        return TRUE;
    }
    return FALSE;
}

/* Converts a colour's components to reals, unscaled */
#define RGBAREALFROMRGBA(o, i)                                                                     \
    do {                                                                                           \
        (o)->red = (i)->red;                                                                       \
        (o)->green = (i)->green;                                                                   \
        (o)->blue = (i)->blue;                                                                     \
        (o)->alpha = (i)->alpha;                                                                   \
    } while (0)

#define RGBAREALSUB(o, a, b)                                                                       \
    do {                                                                                           \
        (o)->red = (a)->red - (b)->red;                                                            \
        (o)->green = (a)->green - (b)->green;                                                      \
        (o)->blue = (a)->blue - (b)->blue;                                                         \
        (o)->alpha = (a)->alpha - (b)->alpha;                                                      \
    } while (0)

/* Sets the colours at the brush's corners: col3 and col2 at the top edge and col0 and col1 at the
 * bottom edge */
Rt2dBrush *fn_80470694(Rt2dBrush *brush, RwRGBA *col0, RwRGBA *col1, RwRGBA *col2, RwRGBA *col3) {
    RGBAREALFROMRGBA(&brush->top.col, col3);
    RGBAREALFROMRGBA(&brush->dtop.col, col2);
    RGBAREALSUB(&brush->dtop.col, &brush->dtop.col, &brush->top.col);

    RGBAREALFROMRGBA(&brush->bottom.col, col0);
    RGBAREALFROMRGBA(&brush->dbottom.col, col1);
    RGBAREALSUB(&brush->dbottom.col, &brush->dbottom.col, &brush->bottom.col);

    memcpy(&brush->colorCache, col3, sizeof(RwRGBA));

    if (0.0f == brush->dtop.col.red && 0.0f == brush->dbottom.col.red &&
        brush->top.col.red == brush->bottom.col.red && 0.0f == brush->dtop.col.green &&
        0.0f == brush->dbottom.col.green && brush->top.col.green == brush->bottom.col.green &&
        0.0f == brush->dtop.col.blue && 0.0f == brush->dbottom.col.blue &&
        brush->top.col.blue == brush->bottom.col.blue && 0.0f == brush->dtop.col.alpha &&
        0.0f == brush->dbottom.col.alpha && brush->top.col.alpha == brush->bottom.col.alpha) {
        brush->flag &= ~FIELDRGBA;
    } else {
        brush->flag |= FIELDRGBA;
    }
    return brush;
}

/* Sets the texture coordinates at the brush's corners, in the same order as the colours */
Rt2dBrush *fn_80470998(Rt2dBrush *brush, RwV2d *uv0, RwV2d *uv1, RwV2d *uv2, RwV2d *uv3) {
    brush->top.uv = *uv3;
    brush->dtop.uv.x = uv2->x - uv3->x;
    brush->dtop.uv.y = uv2->y - uv3->y;

    brush->bottom.uv = *uv0;
    brush->dbottom.uv.x = uv1->x - uv0->x;
    brush->dbottom.uv.y = uv1->y - uv0->y;

    if (0.0f == brush->dtop.uv.x && 0.0f == brush->dtop.uv.y && 0.0f == brush->dbottom.uv.x &&
        0.0f == brush->dbottom.uv.y) {
        brush->flag &= ~FIELDUV;
    } else {
        brush->flag |= FIELDUV;
    }
    return brush;
}

/* Sets the brush's texture, taking a reference to it */
Rt2dBrush *fn_80470A4C(Rt2dBrush *brush, RwTexture *texture) {
    if (texture != NULL) {
        brush->flag |= FIELDUV;
        texture->refCount++;
    } else {
        brush->flag &= ~FIELDUV;
    }
    if (brush->texture != NULL) {
        fn_8048CD8C(brush->texture);
    }
    brush->texture = texture;
    return brush;
}
