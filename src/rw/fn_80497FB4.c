/* RenderWare GameCube raster rendering and clearing: drawing a raster into the raster context (the
 * camera's frame buffer) as a textured quad, and clearing a rectangle of a raster, either by drawing
 * a quad into the frame buffer and Z buffer or by writing the pixels of a locked raster. The
 * original file name is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>
#include <dolphin/vi.h>
#include <string.h>

/* Reports an error of the core; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwID_COREPLUGIN;                                                         \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

/* Error codes (inferred names, as in the raster unit): a debug message, an unsupported raster depth
 * and an unsupported raster format or type */
#define ERRORDEBUGMESSAGE 2
#define ERRORINVALIDRASTERDEPTH 0x8000000C
#define ERRORINVALIDRASTERFORMAT 0x8000000D

/* raster types */
#define rwRASTERTYPENORMAL 0x0
#define rwRASTERTYPEZBUFFER 0x1
#define rwRASTERTYPECAMERA 0x2
#define rwRASTERTYPECAMERATEXTURE 0x5

/* A raster type RenderWare does not define; the driver's switches name it explicitly */
#define RASTERTYPEUNDEFINED 0x3

/* raster lock modes and the private flag of a raster locked for writing */
#define rwRASTERLOCKWRITE 0x1
#define rwRASTERLOCKREAD 0x2
#define rwRASTERLOCKRAW 0x8
#define rwRASTERPIXELLOCKEDWRITE 0x04

/* camera clear modes */
#define rwCAMERACLEARIMAGE 0x1
#define rwCAMERACLEARZ 0x2

/* The raster rendering and clearing draw into */
RwRaster *lbl_805F1668;

/* Sets up GX to draw a raster into the raster context through rect: position and texture
 * coordinate formats, texturing, the frame buffer viewport and a scissor of rect within the
 * context, and an orthographic projection of the frame buffer */
void fn_80497FB4(RwRect *rect) {
    /* The orthographic projection of the frame buffer; the x and y scales are set for its size */
    static f32 projection[7] = {GX_ORTHOGRAPHIC, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f};
    /* The position matrix: pixel centers, toward the camera */
    static f32 matrix[3][4] = {
        {1.0f, 0.0f, 0.0f, 0.5f},
        {0.0f, 1.0f, 0.0f, 0.5f},
        {0.0f, 0.0f, -1.0f, 0.0f},
    };
    GXRenderModeObj *renderMode;

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_S16, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetNumChans(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);

    renderMode = lbl_805F164C;
    if (renderMode->field_rendering) {
        GXSetViewportJitter(0.0f, 0.0f, renderMode->fbWidth, renderMode->xfbHeight, 0.0f, 1.0f,
                            VIGetNextField() ^ 1);
    } else {
        GXSetViewport(0.0f, 0.0f, renderMode->fbWidth, renderMode->xfbHeight, 0.0f, 1.0f);
    }

    if (lbl_805F15F0 == 0) {
        GXSetScissor(lbl_805F1668->nOffsetX + rect->x, lbl_805F1668->nOffsetY + rect->y, rect->w,
                     rect->h);
    } else if (lbl_805EDF28 != 0) {
        /* Viewports are at double height: scissor the part of rect above the split line */
        if ((lbl_805F1668->nOffsetY << 1) + rect->y + rect->h <= lbl_805F1648 + 2) {
            GXSetScissor(lbl_805F1668->nOffsetX + rect->x, (lbl_805F1668->nOffsetY << 1) + rect->y,
                         rect->w, rect->h);
        } else if ((lbl_805F1668->nOffsetY << 1) + rect->y > lbl_805F1648 + 2) {
            GXSetScissor(0, 0, lbl_805F164C->fbWidth, lbl_805F1648 + 2);
        } else {
            GXSetScissor(lbl_805F1668->nOffsetX + rect->x, (lbl_805F1668->nOffsetY << 1) + rect->y,
                         rect->w, lbl_805F1648 + 2 - ((lbl_805F1668->nOffsetY << 1) + rect->y));
        }
        GXSetScissorBoxOffset(0, 0);
    } else {
        /* ... or the part below it */
        if ((lbl_805F1668->nOffsetY << 1) + rect->y >= lbl_805F1648 - 2) {
            GXSetScissor(lbl_805F1668->nOffsetX + rect->x, (lbl_805F1668->nOffsetY << 1) + rect->y,
                         rect->w, rect->h);
        } else if ((lbl_805F1668->nOffsetY << 1) + rect->y + rect->h < lbl_805F1648 - 2) {
            GXSetScissor(0, lbl_805F1648 - 2, lbl_805F164C->fbWidth, lbl_805F1648 + 2);
        } else {
            GXSetScissor(lbl_805F1668->nOffsetX + rect->x, lbl_805F1648 - 2, rect->w,
                         (lbl_805F1668->nOffsetY << 1) + rect->y + rect->h - (lbl_805F1648 - 2));
        }
        GXSetScissorBoxOffset(0, lbl_805F1648 - 2);
    }

    projection[1] = 2.0f / lbl_805F164C->fbWidth;
    projection[3] = -2.0f / lbl_805F164C->xfbHeight;
    GXSetProjectionv(projection);
    GXLoadPosMtxImm(matrix, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
}

/* Draws raster into the raster context at rect, through the raster's parent as a texture; scaled
 * filters linearly (rect has the drawn size), and blend blends with the raster's alpha. It saves
 * and restores the fog, texture filter, U/V addressing and texture raster render states, and
 * restores the cached cull and blend modes. Setting up the texture also selects the alpha test for
 * the parent's format; that is not restored, as restoring the texture raster only sets the raster
 * back. */
void fn_80498344(RwRaster *raster, RwRect *rect, RwBool scaled, RwBool blend) {
    RwRaster *parent = raster->parent;
    RwInt32 fogEnable;
    RwInt32 filter;
    RwInt32 addressU;
    RwInt32 addressV;
    RwRaster *textureRaster;
    RwReal invWidth;
    RwReal invHeight;

    fn_80497474(rwRENDERSTATEFOGENABLE, &fogEnable);
    fn_80497474(rwRENDERSTATETEXTUREFILTER, &filter);
    fn_80497474(rwRENDERSTATETEXTUREADDRESSU, &addressU);
    fn_80497474(rwRENDERSTATETEXTUREADDRESSV, &addressV);
    fn_80497474(rwRENDERSTATETEXTURERASTER, &textureRaster);

    GXSetCullMode(GX_CULL_NONE);
    if (blend) {
        fn_804978C0(rwBLENDSRCALPHA, rwBLENDINVSRCALPHA);
    } else {
        fn_804978C0(rwBLENDONE, rwBLENDZERO);
    }
    fn_80497904(rwRENDERSTATEFOGENABLE, (void *)FALSE);
    fn_80497904(rwRENDERSTATETEXTUREFILTER,
                scaled ? (void *)rwFILTERLINEAR : (void *)rwFILTERNEAREST);
    fn_80497904(rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSCLAMP);
    fn_80497904(rwRENDERSTATETEXTURERASTER, parent);
    fn_80497804();
    fn_80497FB4(rect);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    if (scaled) {
        invWidth = 1.0f / parent->width;
        invHeight = 1.0f / parent->height;
        GXPosition2s16(lbl_805F1668->nOffsetX + rect->x, lbl_805F1668->nOffsetY + rect->y);
        GXTexCoord2f32(raster->nOffsetX * invWidth, raster->nOffsetY * invHeight);
        GXPosition2s16(lbl_805F1668->nOffsetX + rect->x,
                       lbl_805F1668->nOffsetY + rect->y + rect->h);
        GXTexCoord2f32(raster->nOffsetX * invWidth,
                       invHeight * (raster->nOffsetY + raster->height));
        GXPosition2s16(lbl_805F1668->nOffsetX + rect->x + rect->w,
                       lbl_805F1668->nOffsetY + rect->y + rect->h);
        GXTexCoord2f32(invWidth * (raster->nOffsetX + raster->width),
                       invHeight * (raster->nOffsetY + raster->height));
        GXPosition2s16(lbl_805F1668->nOffsetX + rect->x + rect->w,
                       lbl_805F1668->nOffsetY + rect->y);
        GXTexCoord2f32(invWidth * (raster->nOffsetX + raster->width), raster->nOffsetY * invHeight);
    } else {
        invWidth = 1.0f / parent->width;
        invHeight = 1.0f / parent->height;
        GXPosition2s16(lbl_805F1668->nOffsetX + rect->x, lbl_805F1668->nOffsetY + rect->y);
        GXTexCoord2f32(raster->nOffsetX * invWidth, raster->nOffsetY * invHeight);
        GXPosition2s16(lbl_805F1668->nOffsetX + rect->x,
                       lbl_805F1668->nOffsetY + rect->y + raster->height);
        GXTexCoord2f32(raster->nOffsetX * invWidth,
                       invHeight * (raster->nOffsetY + raster->height));
        GXPosition2s16(lbl_805F1668->nOffsetX + rect->x + raster->width,
                       lbl_805F1668->nOffsetY + rect->y + raster->height);
        GXTexCoord2f32(invWidth * (raster->nOffsetX + raster->width),
                       invHeight * (raster->nOffsetY + raster->height));
        GXPosition2s16(lbl_805F1668->nOffsetX + rect->x + raster->width,
                       lbl_805F1668->nOffsetY + rect->y);
        GXTexCoord2f32(invWidth * (raster->nOffsetX + raster->width), raster->nOffsetY * invHeight);
    }

    GXSetCullMode(lbl_805E3210.cullMode - 1);
    fn_804978C0(lbl_805E3210.srcBlend, lbl_805E3210.destBlend);
    fn_80497904(rwRENDERSTATEFOGENABLE, (void *)fogEnable);
    fn_80497904(rwRENDERSTATETEXTUREFILTER, (void *)filter);
    fn_80497904(rwRENDERSTATETEXTUREADDRESSU, (void *)addressU);
    fn_80497904(rwRENDERSTATETEXTUREADDRESSV, (void *)addressV);
    fn_80497904(rwRENDERSTATETEXTURERASTER, textureRaster);
}

/* Renders raster into the raster context at the position in rect (and its size when scaled); only
 * normal, texture and camera texture rasters can be drawn, and only into a camera raster */
RwBool fn_804988D4(RwRaster *raster, RwRect *rect, RwBool scaled, RwBool blend) {
    RwRect drawRect;

    drawRect.x = rect->x;
    drawRect.y = rect->y;
    if (scaled) {
        drawRect.w = rect->w;
        drawRect.h = rect->h;
    } else {
        drawRect.w = raster->width;
        drawRect.h = raster->height;
    }

    switch (lbl_805F1668->cType) {
    case rwRASTERTYPENORMAL:
    case rwRASTERTYPETEXTURE:
    case rwRASTERTYPECAMERATEXTURE:
        switch (raster->cType) {
        case rwRASTERTYPENORMAL:
        case rwRASTERTYPEZBUFFER:
        case rwRASTERTYPECAMERA:
        case rwRASTERTYPETEXTURE:
        case rwRASTERTYPECAMERATEXTURE:
            RWERROR((ERRORDEBUGMESSAGE, "SRC, DST Raster render combination not supported"));
            return FALSE;
        case RASTERTYPEUNDEFINED:
            break;
        default:
            break;
        }
        break;
    case rwRASTERTYPECAMERA:
        switch (raster->cType) {
        case rwRASTERTYPENORMAL:
        case rwRASTERTYPETEXTURE:
        case rwRASTERTYPECAMERATEXTURE:
            fn_80498344(raster, &drawRect, scaled, blend);
            break;
        case rwRASTERTYPEZBUFFER:
        case rwRASTERTYPECAMERA:
            RWERROR((ERRORDEBUGMESSAGE, "SRC, DST Raster render combination not supported"));
            return FALSE;
        case RASTERTYPEUNDEFINED:
            break;
        }
        break;
    default:
        RWERROR((ERRORDEBUGMESSAGE, "SRC, DST Raster render combination not supported"));
        return FALSE;
    }
    return TRUE;
}

/* Renders a raster (inferred: the standard raster render function) */
RwBool fn_80498A4C(void *out, void *inOut, RwInt32 numIn) {
    return fn_804988D4(out, inOut, FALSE, TRUE);
}

/* Renders a raster without blending (inferred: the standard fast raster render function) */
RwBool fn_80498A74(void *out, void *inOut, RwInt32 numIn) {
    return fn_804988D4(out, inOut, FALSE, FALSE);
}

/* Renders a raster scaled to a rectangle (inferred: the standard scaled raster render function) */
RwBool fn_80498A9C(void *out, void *inOut, RwInt32 numIn) {
    return fn_804988D4(out, inOut, TRUE, TRUE);
}

/* Sets up GX to clear raster: untextured positions colored by the material color, and the
 * viewport, scissor and projection of the frame buffer; a camera texture is drawn into a fixed
 * 640x528 frame buffer with a pixel format that has alpha when the raster has */
void fn_80498AC4(RwRaster *raster) {
    /* The orthographic projection; the x and y scales are set for the frame buffer's size */
    static f32 projection[7] = {GX_ORTHOGRAPHIC, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f};
    /* The position matrix: pixel centers, toward the camera */
    static f32 matrix[3][4] = {
        {1.0f, 0.0f, 0.0f, 0.5f},
        {0.0f, 1.0f, 0.0f, 0.5f},
        {0.0f, 0.0f, -1.0f, 0.0f},
    };
    GXRenderModeObj *renderMode;

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);

    if (raster->cType == rwRASTERTYPECAMERATEXTURE) {
        /* bit 0 of the flags: the raster's format has alpha */
        if (RWPLUGINOFFSET(RwGameCubeRasterExt, raster->parent, lbl_805F1658)->flags & 1) {
            if (lbl_805F15E4 != GX_PF_RGBA6_Z24) {
                GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
                lbl_805F15E8 = GX_PF_RGBA6_Z24;
            }
        } else if (lbl_805F15E4 != GX_PF_RGB8_Z24) {
            GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
            lbl_805F15E8 = GX_PF_RGB8_Z24;
        }
        GXSetViewport(0.0f, 0.0f, 640.0f, 528.0f, 0.0f, 1.0f);
        GXSetScissor(0, 0, 640, 528);
        projection[1] = 2.0f / 639.0f;
        projection[3] = -2.0f / 527.0f;
    } else {
        if (lbl_805F15E8 != lbl_805F15E4) {
            GXSetPixelFmt(lbl_805F15E4, GX_ZC_LINEAR);
            lbl_805F15E8 = lbl_805F15E4;
        }

        renderMode = lbl_805F164C;
        if (renderMode->field_rendering) {
            GXSetViewportJitter(0.0f, 0.0f, renderMode->fbWidth, renderMode->xfbHeight, 0.0f, 1.0f,
                                VIGetNextField() ^ 1);
        } else {
            GXSetViewport(0.0f, 0.0f, renderMode->fbWidth, renderMode->xfbHeight, 0.0f, 1.0f);
        }

        if (lbl_805F15F0 == 0) {
            GXSetScissor(0, 0, lbl_805F164C->fbWidth, lbl_805F164C->xfbHeight);
        } else if (lbl_805EDF28 != 0) {
            GXSetScissor(0, 0, lbl_805F164C->fbWidth, lbl_805F1648 + 2);
            GXSetScissorBoxOffset(0, 0);
        } else {
            GXSetScissor(0, lbl_805F1648 - 2, lbl_805F164C->fbWidth, lbl_805F1648 + 2);
            GXSetScissorBoxOffset(0, lbl_805F1648 - 2);
        }

        projection[1] = 2.0f / lbl_805F164C->fbWidth;
        projection[3] = -2.0f / lbl_805F164C->xfbHeight;
    }
    GXSetProjectionv(projection);
    GXLoadPosMtxImm(matrix, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
}

/* Clears rect of raster (in frame buffer coordinates) to color and/or the far Z by drawing a quad,
 * as clearMode selects. Afterwards it turns fog back on if it was on, sets the cached blend mode,
 * Z compare and Z write (with the Z test on) and cull mode, and turns color updates on; alpha
 * testing stays off for opaque drawing, and the GX setup of the clear is left as is. */
void fn_80498E08(RwRaster *raster, RwRect *rect, RwRGBA *color, RwInt32 clearMode) {
    RwBool fogEnabled = FALSE;
    GXColor clearColor;

    if (clearMode & rwCAMERACLEARIMAGE) {
        clearColor.r = color->red;
        clearColor.g = color->green;
        clearColor.b = color->blue;
        clearColor.a = color->alpha;
        if (clearMode & rwCAMERACLEARZ) {
            GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
        } else {
            GXSetZMode(GX_TRUE, GX_ALWAYS, GX_FALSE);
        }
    } else if (clearMode & rwCAMERACLEARZ) {
        /* Only the Z buffer: leave the colors */
        clearColor.r = 0;
        clearColor.g = 0;
        clearColor.b = 0;
        clearColor.a = 255;
        GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
        GXSetColorUpdate(GX_FALSE);
    } else {
        return;
    }

    GXSetChanMatColor(GX_COLOR0A0, clearColor);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    GXSetCullMode(GX_CULL_NONE);
    fn_80497778(TRUE);
    if (lbl_805E3210.fogEnable) {
        fn_80497668(FALSE);
        fogEnabled = TRUE;
    }
    fn_80498AC4(raster);

    /* The far plane, just in front of it */
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(rect->x, rect->y, 0.99999994f);
    GXPosition3f32(rect->x, rect->y + rect->h, 0.99999994f);
    GXPosition3f32(rect->x + rect->w, rect->y + rect->h, 0.99999994f);
    GXPosition3f32(rect->x + rect->w, rect->y, 0.99999994f);

    if (fogEnabled) {
        fn_80497668(TRUE);
    }
    fn_804978C0(lbl_805E3210.srcBlend, lbl_805E3210.destBlend);
    GXSetZMode(GX_TRUE, lbl_805E3210.zCompare, lbl_805E3210.zWriteEnable);
    GXSetCullMode(lbl_805E3210.cullMode - 1);
    GXSetColorUpdate(GX_TRUE);
}

/* Clears rect of the raster context to color, a pixel value of its format: a camera raster through
 * a drawn quad, a Z raster to the far Z, and other rasters by writing their locked pixels */
RwBool fn_804990D4(RwRaster *raster, RwRect *rect, RwInt32 color) {
    RwBool locked;
    RwInt32 y;

    switch (lbl_805F1668->cType) {
    case rwRASTERTYPECAMERA: {
        RwRGBA rgba;

        fn_80489118(&rgba, color, RwRasterGetFormat(lbl_805F1668));
        rect->x += lbl_805F1668->nOffsetX;
        rect->y += lbl_805F1668->nOffsetY;
        fn_80498E08(raster, rect, &rgba, rwCAMERACLEARIMAGE);
        break;
    }
    case rwRASTERTYPEZBUFFER:
        rect->x += lbl_805F1668->nOffsetX;
        rect->y += lbl_805F1668->nOffsetY;
        fn_80498E08(raster, rect, NULL, rwCAMERACLEARZ);
        break;
    case rwRASTERTYPENORMAL:
    case rwRASTERTYPETEXTURE:
    case rwRASTERTYPECAMERATEXTURE:
        locked = FALSE;
        if (lbl_805F1668->parent == lbl_805F1668 && lbl_805F1668->nOffsetX == 0 &&
            lbl_805F1668->nOffsetY == 0 && lbl_805F1668->width == rect->w &&
            lbl_805F1668->height == rect->h) {
            /* The whole raster */
            if (!(lbl_805F1668->privateFlags & rwRASTERPIXELLOCKEDWRITE)) {
                locked = TRUE;
                fn_80489F70(lbl_805F1668, 0, rwRASTERLOCKWRITE | rwRASTERLOCKRAW);
            }
            if (color == 0 || color == -1) {
                /* Every byte of the pixels is the same */
                memset(lbl_805F1668->cpPixels, color, lbl_805F1668->stride * lbl_805F1668->height);
                if (locked) {
                    fn_80489B40(lbl_805F1668);
                }
                return TRUE;
            }
        } else if (!(lbl_805F1668->privateFlags & rwRASTERPIXELLOCKEDWRITE)) {
            locked = TRUE;
            fn_80489F70(lbl_805F1668, 0, rwRASTERLOCKWRITE | rwRASTERLOCKREAD);
        }

        switch (lbl_805F1668->depth) {
        case 4:
            /* Both pixels of each byte */
            color = (((color << 4) & 0xF0) | (color & 0xF)) & 0xFF;
            for (y = 0; y < rect->h; y++) {
                memset(lbl_805F1668->cpPixels + lbl_805F1668->stride * (rect->y + y) +
                           (rect->x >> 1),
                       color, rect->w >> 1);
            }
            break;
        case 8:
            color &= 0xFF;
            for (y = 0; y < rect->h; y++) {
                memset(lbl_805F1668->cpPixels + lbl_805F1668->stride * (rect->y + y) + rect->x,
                       color, rect->w);
            }
            break;
        case 16: {
            RwUInt16 color16 = color;
            RwInt32 x;

            for (y = 0; y < rect->h; y++) {
                RwUInt16 *pixel =
                    (RwUInt16 *)(lbl_805F1668->cpPixels + lbl_805F1668->stride * (rect->y + y) +
                                 (rect->x << 1));

                for (x = 0; x < rect->w; x++) {
                    *pixel++ = color16;
                }
            }
            break;
        }
        case 32: {
            RwInt32 x;

            for (y = 0; y < rect->h; y++) {
                RwUInt32 *pixel =
                    (RwUInt32 *)(lbl_805F1668->cpPixels + lbl_805F1668->stride * (rect->y + y) +
                                 (rect->x << 2));

                for (x = 0; x < rect->w; x++) {
                    *pixel++ = color;
                }
            }
            break;
        }
        case 24:
        default:
            RWERROR((ERRORINVALIDRASTERDEPTH));
            break;
        }

        if (locked) {
            fn_80489B40(lbl_805F1668);
        }
        break;
    default:
        RWERROR((ERRORINVALIDRASTERFORMAT));
        return FALSE;
    }
    return TRUE;
}

/* Clears a rectangle of the raster context (inferred: the standard raster clear rectangle
 * function) */
RwBool fn_804994B8(void *out, void *inOut, RwInt32 numIn) {
    return fn_804990D4(lbl_805F1668, inOut, numIn);
}

/* Clears the raster context (inferred: the standard raster clear function) */
RwBool fn_804994DC(void *out, void *inOut, RwInt32 numIn) {
    RwRect rect;

    rect.x = 0;
    rect.y = 0;
    rect.w = lbl_805F1668->width;
    rect.h = lbl_805F1668->height;
    return fn_804990D4(lbl_805F1668, &rect, numIn);
}

/* Sets the raster context (inferred: the standard set raster context function) */
RwBool fn_80499520(void *out, void *inOut, RwInt32 numIn) {
    lbl_805F1668 = inOut;
    return TRUE;
}
