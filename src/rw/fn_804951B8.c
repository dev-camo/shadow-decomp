/* RenderWare GameCube rasters: the raster plugin data, mip level sizes and strides, conversion
 * between linear pixels and GX texture tiles, locking and unlocking rasters and their palettes,
 * choosing a raster's GX texture format, creating and destroying rasters, setting a texture's
 * raster, sub rasters, and copying the frame buffer into a camera texture. The original file name
 * is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>
#include <dolphin/os.h>
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

/* Error codes (inferred names): a debug message, an unsupported raster depth, format or type, a
 * raster that cannot be locked, and one that cannot be unlocked */
#define ERRORDEBUGMESSAGE 2
#define ERRORINVALIDRASTERDEPTH 0x8000000C
#define ERRORINVALIDRASTERFORMAT 0x8000000D
#define ERRORCANNOTLOCK 0x8000000E
#define ERRORCANNOTUNLOCK 0x80000011

/* The memory hint of raster memory (inferred) */
#define rwMEMHINT_RASTER 0x30411

/* raster types */
#define rwRASTERTYPENORMAL 0x0
#define rwRASTERTYPEZBUFFER 0x1
#define rwRASTERTYPECAMERA 0x2
#define rwRASTERTYPECAMERATEXTURE 0x5
#define rwRASTERTYPEMASK 0x7

/* A raster type RenderWare does not define; the driver's switches reject it explicitly */
#define RASTERTYPEUNDEFINED 0x3
#define rwRASTERFLAGSMASK 0xF8

/* raster formats */
#define rwRASTERFORMAT16 0x0700
#define rwRASTERFORMAT24 0x0800
#define rwRASTERFORMAT32 0x0900

/* raster private flags */
#define rwRASTERPIXELLOCKEDREAD 0x02
#define rwRASTERPIXELLOCKEDWRITE 0x04
#define rwRASTERPALETTELOCKEDREAD 0x08
#define rwRASTERPALETTELOCKEDWRITE 0x10
#define rwRASTERPIXELLOCKEDRAW 0x20
#define rwRASTERPIXELLOCKED (rwRASTERPIXELLOCKEDREAD | rwRASTERPIXELLOCKEDWRITE)

/* raster lock modes */
#define rwRASTERLOCKWRITE 0x1
#define rwRASTERLOCKREAD 0x2
#define rwRASTERLOCKRAW 0x8

/* The next draw sync token (declared volatile in rpworld.h) */
volatile RwUInt16 lbl_805EDF40 = 1;

/* The raster plugin's offset */
RwInt32 lbl_805F1658;

/* The raster plugin data of a raster's parent */
#define RASTEREXT(raster) RWPLUGINOFFSET(RwGameCubeRasterExt, (raster)->parent, lbl_805F1658)

/* Copies the frame buffer behind a sub raster of a camera texture into the texture, halved with a
 * box filter when halve is set */
void fn_804951B8(RwRaster *raster, RwBool halve) {
    RwRaster *parent = raster->parent;
    RwGameCubeRasterExt *rasterExt = RWPLUGINOFFSET(RwGameCubeRasterExt, parent, lbl_805F1658);
    RwInt32 offset;

    GXSetCopyFilter(GX_FALSE, NULL, GX_FALSE, NULL);
    if (halve) {
        GXSetTexCopySrc(raster->nOffsetX * 2, raster->nOffsetY * 2, raster->width * 2,
                        raster->height * 2);
    } else {
        GXSetTexCopySrc(raster->nOffsetX, raster->nOffsetY, raster->width, raster->height);
    }
    GXSetTexCopyDst(parent->width, parent->height, rasterExt->texFormat, halve);

    /* The offset of the sub raster's first tile */
    switch (parent->depth) {
    case 4:
        offset = ((raster->nOffsetX << 3) + ((parent->width + 7) & ~7) * raster->nOffsetY) >> 1;
        break;
    case 8:
        offset = (raster->nOffsetX << 2) + ((parent->width + 7) & ~7) * raster->nOffsetY;
        break;
    case 16:
        offset = ((raster->nOffsetX << 2) + ((parent->width + 3) & ~3) * raster->nOffsetY) << 1;
        break;
    case 32:
        offset = ((raster->nOffsetX << 2) + ((parent->width + 3) & ~3) * raster->nOffsetY) << 2;
        break;
    case 24:
    default:
        RWERROR((ERRORINVALIDRASTERDEPTH));
        return;
    }

    GXCopyTex(rasterExt->texels + offset, lbl_805F15E0);
    GXPixModeSync();
    GXSetCopyFilter(lbl_805F164C->aa, lbl_805F164C->sample_pattern, GX_TRUE, lbl_805F164C->vfilter);
    if (rasterExt->texRegion != NULL) {
        GXInvalidateTexRegion(rasterExt->texRegion);
    } else {
        GXInvalidateTexAll();
    }
}

/* Waits for the queued frames to be shown, then gives the shown external frame buffer and the
 * one the next frame is copied to */
void fn_804953C0(void **shownFrameBuffer, void **nextFrameBuffer) {
    while (lbl_805F161C == 1 || lbl_805F1610 != lbl_805F1614) {
    }
    *shownFrameBuffer = lbl_805F1604;
    *nextFrameBuffer = lbl_805F1600;
}

/* Registers the raster plugin data */
RwInt32 fn_804953F0(void) {
    return lbl_805F1658 =
               fn_80489C54(sizeof(RwGameCubeRasterExt), rwID_DEVICEMODULE, NULL, NULL, NULL);
}

/* The size of a raster's mip level in GX tiles, 32-byte aligned; the whole raster's size while it
 * is locked */
RwInt32 fn_80495428(RwRaster *raster, RwUInt8 level) {
    RwInt32 width;
    RwUInt32 height;
    RwUInt32 size;
    RwRaster *parent = raster->parent;

    if (raster->privateFlags & rwRASTERPIXELLOCKED) {
        width = parent->width;
        height = parent->height;
    } else {
        RwInt32 levelWidth = parent->width >> level;
        RwUInt32 levelHeight = parent->height >> level;

        width = 1;
        if (levelWidth != 0) {
            width = levelWidth;
        }
        height = 1;
        if (levelHeight != 0) {
            height = levelHeight;
        }
    }

    switch (raster->depth) {
    case 4:
        width = (width + 7) & ~7;
        height = (height + 7) & ~7;
        size = (width * height) >> 1;
        break;
    case 8:
        width = (width + 7) & ~7;
        height = (height + 3) & ~3;
        size = width * height;
        break;
    case 16:
        width = (width + 3) & ~3;
        height = (height + 3) & ~3;
        size = (width * height) << 1;
        break;
    case 32:
        width = (width + 3) & ~3;
        height = (height + 3) & ~3;
        size = (width * height) << 2;
        break;
    case 24:
    default:
        RWERROR((ERRORINVALIDRASTERDEPTH));
        return 0;
    }
    return (size + 31) & ~31;
}

/* The size of all of a raster's mip levels */
RwInt32 fn_80495558(RwRaster *raster) {
    RwInt32 size = 0;
    RwInt32 numLevels;

    fn_80495688(&numLevels, raster, 0);
    while (numLevels--) {
        size += fn_80495428(raster, numLevels);
    }
    return size;
}

/* The stride of a raster's mip level in GX tiles */
RwInt32 fn_804955CC(RwRaster *raster, RwUInt8 level) {
    RwInt32 levelWidth = raster->parent->width >> level;
    RwInt32 width = (levelWidth != 0) ? levelWidth : 1;

    switch (raster->depth) {
    case 4:
        return ((width + 7) & ~7) >> 1;
    case 8:
        return (width + 7) & ~7;
    case 16:
        return ((width + 3) & ~3) << 1;
    case 32:
        return ((width + 3) & ~3) << 2;
    case 24:
    default:
        RWERROR((ERRORINVALIDRASTERDEPTH));
        return 0;
    }
}

/* Gives the number of a raster's mip levels: out is the number and inOut the raster */
RwBool fn_80495688(void *out, void *inOut, RwInt32 numIn) {
    RwInt32 *numLevels = (RwInt32 *)out;
    RwRaster *raster = (RwRaster *)inOut;
    RwUInt8 maxLevel = RASTEREXT(raster)->maxLevel;

    if (maxLevel != 0xFF) {
        *numLevels = maxLevel + 1;
    } else {
        RwUInt8 count;

        if (RwRasterGetFormat(raster) & rwRASTERFORMATMIPMAP) {
            if (raster->width > raster->height) {
                count = (RwUInt8)fn_80491AB4(raster->width) + 1;
            } else {
                count = (RwUInt8)fn_80491AB4(raster->height) + 1;
            }
        } else {
            count = 1;
        }
        *numLevels = count;
    }
    return TRUE;
}

/* Copies linear pixels into GX texture tiles: 8x8 texel tiles of 4 bits, 8x4 of 8 bits, and 4x4 of
 * 16 bits or 32 bits (the alpha and red, then green and blue halves) */
void fn_8049572C(RwUInt8 *tiles, RwUInt8 *pixels, RwInt32 width, RwInt32 height, RwInt32 depth,
                 RwInt32 stride) {
    switch (depth) {
    case 4: {
        RwInt32 tilesWide = ((width + 7) & ~7) >> 3;
        RwInt32 y;
        RwInt32 rowOffset = 0;

        for (y = 0; y < height; y++) {
            RwInt32 tileRow = tilesWide * (y >> 3);
            RwUInt8 *row = pixels + rowOffset;
            RwInt32 lineOffset = (y & 7) << 3;
            RwInt32 source = 0;
            RwInt32 tile = 0;
            RwInt32 x;

            for (x = 0; x < width; x += 8) {
                memcpy(tiles + ((RwUInt32)(lineOffset + ((tileRow + tile) << 6)) >> 1),
                       row + source, 4);
                source += 4;
                tile++;
            }
            rowOffset += stride;
        }
        break;
    }
    case 8: {
        RwInt32 tilesWide = ((width + 7) & ~7) >> 3;
        RwInt32 y;
        RwInt32 rowOffset = 0;

        for (y = 0; y < height; y++) {
            RwUInt8 *row = pixels + rowOffset;
            RwInt32 tileRow = tilesWide * (y >> 2);
            RwInt32 lineOffset = (y & 3) << 3;
            RwInt32 x;
            RwInt32 tile = 0;

            for (x = 0; x < width; x += 8) {
                memcpy(lineOffset + (((tileRow + tile) << 5) + tiles), row + x, 8);
                tile++;
            }
            rowOffset += stride;
        }
        break;
    }
    case 16: {
        RwInt32 tilesWide = ((width + 3) & ~3) >> 2;
        RwInt32 y;
        RwInt32 rowOffset = 0;

        for (y = 0; y < height; y++) {
            RwInt32 tileRow = tilesWide * (y >> 2);
            RwInt32 lineOffset = (y & 3) << 2;
            RwUInt8 *source = pixels + rowOffset;
            RwInt32 x;
            RwInt32 tile = 0;

            for (x = 0; x < width; x += 4) {
                memcpy(tiles + ((lineOffset + ((tileRow + tile) << 4)) << 1), source, 8);
                source += 8;
                tile++;
            }
            rowOffset += stride;
        }
        break;
    }
    case 32: {
        RwInt32 tilesWide = ((width + 3) & ~3) >> 2;
        RwInt32 y;
        RwInt32 rowOffset = 0;

        for (y = 0; y < height; y++) {
            RwInt32 tileRow = tilesWide * (y >> 2);
            RwUInt8 *row = pixels + rowOffset;
            RwInt32 lineOffset = (y & 3) << 2;
            RwInt32 source = 0;
            RwInt32 tileOffset = 0;
            RwInt32 x;

            for (x = 0; x < width; x++) {
                RwUInt16 *texel;

                if ((x & 3) == 0) {
                    tileOffset = (tileRow + (x >> 2)) << 4;
                }
                texel = (RwUInt16 *)(tiles + (tileOffset << 2) + ((lineOffset + (x & 3)) << 1));
                texel[0] = *(RwUInt16 *)(row + source);
                texel[16] = *(RwUInt16 *)(row + source + 2);
                source += 4;
            }
            rowOffset += stride;
        }
        break;
    }
    case 24:
    default:
        RWERROR((ERRORINVALIDRASTERDEPTH));
        break;
    }
}

/* Copies GX texture tiles into linear pixels: the inverse of fn_8049572C */
void fn_804959C0(RwUInt8 *pixels, RwUInt8 *tiles, RwInt32 width, RwInt32 height, RwInt32 depth,
                 RwInt32 stride) {
    switch (depth) {
    case 4: {
        RwInt32 tilesWide = ((width + 7) & ~7) >> 3;
        RwInt32 y;
        RwInt32 rowOffset = 0;

        for (y = 0; y < height; y++) {
            RwInt32 tileRow = tilesWide * (y >> 3);
            RwUInt8 *row = pixels + rowOffset;
            RwInt32 lineOffset = (y & 7) << 3;
            RwInt32 source = 0;
            RwInt32 tile = 0;
            RwInt32 x;

            for (x = 0; x < width; x += 8) {
                memcpy(row + source,
                       tiles + ((RwUInt32)(lineOffset + ((tileRow + tile) << 6)) >> 1), 4);
                source += 4;
                tile++;
            }
            rowOffset += stride;
        }
        break;
    }
    case 8: {
        RwInt32 tilesWide = ((width + 7) & ~7) >> 3;
        RwInt32 y;
        RwInt32 rowOffset = 0;

        for (y = 0; y < height; y++) {
            RwUInt8 *row = pixels + rowOffset;
            RwInt32 tileRow = tilesWide * (y >> 2);
            RwInt32 lineOffset = (y & 3) << 3;
            RwInt32 x;
            RwInt32 tile = 0;

            for (x = 0; x < width; x += 8) {
                memcpy(row + x, lineOffset + (((tileRow + tile) << 5) + tiles), 8);
                tile++;
            }
            rowOffset += stride;
        }
        break;
    }
    case 16: {
        RwInt32 tilesWide = ((width + 3) & ~3) >> 2;
        RwInt32 y;
        RwInt32 rowOffset = 0;

        for (y = 0; y < height; y++) {
            RwInt32 tileRow = tilesWide * (y >> 2);
            RwInt32 lineOffset = (y & 3) << 2;
            RwUInt8 *source = pixels + rowOffset;
            RwInt32 x;
            RwInt32 tile = 0;

            for (x = 0; x < width; x += 4) {
                memcpy(source, tiles + ((lineOffset + ((tileRow + tile) << 4)) << 1), 8);
                source += 8;
                tile++;
            }
            rowOffset += stride;
        }
        break;
    }
    case 32: {
        RwInt32 tilesWide = ((width + 3) & ~3) >> 2;
        RwInt32 y;
        RwInt32 rowOffset = 0;

        for (y = 0; y < height; y++) {
            RwInt32 tileRow = tilesWide * (y >> 2);
            RwUInt8 *row = pixels + rowOffset;
            RwInt32 lineOffset = (y & 3) << 2;
            RwInt32 source = 0;
            RwInt32 tileOffset = 0;
            RwInt32 x;

            for (x = 0; x < width; x++) {
                RwUInt16 *texel;

                if ((x & 3) == 0) {
                    tileOffset = (tileRow + (x >> 2)) << 4;
                }
                texel = (RwUInt16 *)(tiles + (tileOffset << 2) + ((lineOffset + (x & 3)) << 1));
                *(RwUInt16 *)(row + source) = texel[0];
                *(RwUInt16 *)(row + source + 2) = texel[16];
                source += 4;
            }
            rowOffset += stride;
        }
        break;
    }
    case 24:
    default:
        RWERROR((ERRORINVALIDRASTERDEPTH));
        break;
    }
}

/* Waits until the GPU has used a raster: sets a draw sync token when the raster's is the next one,
 * then waits for the raster's token (inferred helper) */
static inline void RasterWaitForGPU(RwGameCubeRasterExt *rasterExt) {
    if (rasterExt->syncToken == lbl_805EDF40) {
        GXSetDrawSync(lbl_805EDF40);
        lbl_805EDF40 = (lbl_805EDF40 + 1) % rwGAMECUBEMAXSYNCTOKEN;
    }
    while (!fn_8049A78C(rasterExt->syncToken)) {
    }
}

/* Locks a mip level of a raster: out is the pixels, inOut the raster and numIn the lock mode with
 * the level above it. Without rwRASTERLOCKRAW the level is converted through a linear buffer */
RwBool fn_80495C54(void *out, void *inOut, RwInt32 numIn) {
    RwInt32 stride;
    RwInt32 lockMode;
    RwInt32 raw;
    RwUInt32 offset;
    RwRaster *raster;
    RwRaster *parent;
    RwGameCubeRasterExt *rasterExt;
    RwUInt8 i;
    RwUInt8 *texels;
    RwUInt8 level;

    raster = (RwRaster *)inOut;
    lockMode = numIn;
    level = (numIn >> 8) & 0xFF;
    parent = raster->parent;
    rasterExt = RWPLUGINOFFSET(RwGameCubeRasterExt, parent, lbl_805F1658);

    switch (raster->cType & rwRASTERTYPEMASK) {
    case rwRASTERTYPENORMAL:
    case rwRASTERTYPETEXTURE:
    case rwRASTERTYPECAMERATEXTURE: {
        /* The level's texels follow the larger levels' */
        i = level;
        offset = 0;
        while (i--) {
            offset += fn_80495428(raster, i);
        }
        texels = rasterExt->texels + offset;
        stride = fn_804955CC(raster, level);

        raw = lockMode & rwRASTERLOCKRAW;
        if (!raw) {
            RwInt32 levelHeight = parent->height >> level;
            RwInt32 height = 1;

            if (levelHeight != 0) {
                height = levelHeight;
            }
            rasterExt->lockBuffer = RwMalloc(stride * height, rwMEMHINT_RASTER);
            if (rasterExt->lockBuffer == NULL) {
                return FALSE;
            }
        }

        if (parent == raster) {
            RwInt32 levelWidth = raster->width >> level;
            RwInt32 levelHeight = raster->height >> level;
            RwInt32 width;
            RwInt32 height;

            width = 1;
            if (levelWidth != 0) {
                width = levelWidth;
            }
            height = 1;
            if (levelHeight != 0) {
                height = levelHeight;
            }
            raster->originalWidth = raster->width;
            raster->originalHeight = raster->height;
            raster->width = width;
            raster->height = height;
            if (!raw) {
                raster->cpPixels = rasterExt->lockBuffer;
            } else {
                raster->cpPixels = texels;
            }
        } else if (!raw) {
            switch (raster->depth) {
            case 4:
                raster->cpPixels =
                    rasterExt->lockBuffer + stride * raster->nOffsetY + (raster->nOffsetX >> 1);
                break;
            case 8:
                raster->cpPixels =
                    rasterExt->lockBuffer + stride * raster->nOffsetY + raster->nOffsetX;
                break;
            case 16:
                raster->cpPixels =
                    rasterExt->lockBuffer + stride * raster->nOffsetY + (raster->nOffsetX << 1);
                break;
            case 32:
                raster->cpPixels =
                    rasterExt->lockBuffer + stride * raster->nOffsetY + (raster->nOffsetX << 2);
                break;
            case 24:
            default:
                RWERROR((ERRORINVALIDRASTERDEPTH));
                return FALSE;
            }
        } else {
            RWERROR((ERRORCANNOTLOCK));
            return FALSE;
        }

        rasterExt->lockedTexels = texels;
        raster->stride = stride;
        rasterExt->lockedLevel = level;

        if (lockMode & rwRASTERLOCKREAD) {
            if ((raster->cType & rwRASTERTYPEMASK) == rwRASTERTYPECAMERATEXTURE) {
                DCFlushRange(rasterExt->lockedTexels, fn_80495428(raster, level));
            }
            raster->privateFlags |= rwRASTERPIXELLOCKEDREAD;
            parent->privateFlags |= rwRASTERPIXELLOCKEDREAD;
            if (!raw) {
                fn_804959C0(rasterExt->lockBuffer, rasterExt->lockedTexels, parent->width,
                            parent->height, raster->depth, raster->stride);
            }
        }
        if (lockMode & rwRASTERLOCKWRITE) {
            raster->privateFlags |= rwRASTERPIXELLOCKEDWRITE;
            parent->privateFlags |= rwRASTERPIXELLOCKEDWRITE;
            RasterWaitForGPU(rasterExt);
        }
        if (raw) {
            raster->privateFlags |= rwRASTERPIXELLOCKEDRAW;
            parent->privateFlags |= rwRASTERPIXELLOCKEDRAW;
        }
        break;
    }
    case RASTERTYPEUNDEFINED:
    default:
        RWERROR((ERRORCANNOTLOCK));
        return FALSE;
    }
    *(RwUInt8 **)out = raster->cpPixels;
    return TRUE;
}
/* Unlocks a raster: converts written pixels back into tiles, and regenerates the mip levels of an
 * automatically mipmapped raster written at its first level */
RwBool fn_804960B4(void *out, void *inOut, RwInt32 numIn) {
    RwRaster *raster = (RwRaster *)inOut;
    RwRaster *parent = raster->parent;
    RwGameCubeRasterExt *rasterExt = RWPLUGINOFFSET(RwGameCubeRasterExt, parent, lbl_805F1658);

    switch (raster->cType & rwRASTERTYPEMASK) {
    case rwRASTERTYPENORMAL:
    case rwRASTERTYPETEXTURE:
    case rwRASTERTYPECAMERATEXTURE:
        if (raster->privateFlags & rwRASTERPIXELLOCKEDWRITE) {
            if (!(raster->privateFlags & rwRASTERPIXELLOCKEDRAW)) {
                fn_8049572C(rasterExt->lockedTexels, rasterExt->lockBuffer, parent->width,
                            parent->height, raster->depth, raster->stride);
            }
            DCFlushRange(rasterExt->lockedTexels, fn_80495428(raster, rasterExt->lockedLevel));
            GXInvalidateTexAll();
        }
        if (parent == raster) {
            raster->width = raster->originalWidth;
            raster->height = raster->originalHeight;
        }
        if (!(raster->privateFlags & rwRASTERPIXELLOCKEDRAW)) {
            RwFree(rasterExt->lockBuffer);
            rasterExt->lockBuffer = NULL;
        }
        raster->stride = 0;
        raster->cpPixels = NULL;
        if ((raster->privateFlags & rwRASTERPIXELLOCKEDWRITE) &&
            (raster->cFormat & (rwRASTERFORMATAUTOMIPMAP >> 8)) && rasterExt->lockedLevel == 0) {
            rasterExt->lockedLevel = 0xFF;
            raster->privateFlags &= ~(rwRASTERPIXELLOCKED | rwRASTERPIXELLOCKEDRAW);
            parent->privateFlags &= ~(rwRASTERPIXELLOCKED | rwRASTERPIXELLOCKEDRAW);
            fn_8048D2C0(raster, NULL);
        } else {
            rasterExt->lockedLevel = 0xFF;
            raster->privateFlags &= ~(rwRASTERPIXELLOCKED | rwRASTERPIXELLOCKEDRAW);
            parent->privateFlags &= ~(rwRASTERPIXELLOCKED | rwRASTERPIXELLOCKEDRAW);
        }
        break;
    case RASTERTYPEUNDEFINED:
    default:
        RWERROR((ERRORCANNOTUNLOCK));
        return FALSE;
    }
    return TRUE;
}

/* Locks a raster's palette: out is the palette, inOut the raster and numIn the lock mode */
RwBool fn_80496268(void *out, void *inOut, RwInt32 numIn) {
    RwRaster *raster = (RwRaster *)inOut;

    switch (raster->cType & rwRASTERTYPEMASK) {
    case rwRASTERTYPENORMAL:
    case rwRASTERTYPETEXTURE:
        if (raster == raster->parent && raster->palette == NULL) {
            RwGameCubeRasterExt *rasterExt =
                RWPLUGINOFFSET(RwGameCubeRasterExt, raster, lbl_805F1658);

            if (numIn & rwRASTERLOCKREAD) {
                raster->privateFlags |= rwRASTERPALETTELOCKEDREAD;
            }
            if (numIn & rwRASTERLOCKWRITE) {
                raster->privateFlags |= rwRASTERPALETTELOCKEDWRITE;
            }
            raster->palette = (RwUInt8 *)rasterExt->palette;
            *(RwUInt8 **)out = raster->palette;
        }
        return TRUE;
    case RASTERTYPEUNDEFINED:
    case rwRASTERTYPECAMERATEXTURE:
    default:
        break;
    }
    RWERROR((ERRORCANNOTLOCK));
    return FALSE;
}

/* Unlocks a raster's palette, flushing it when it was written */
RwBool fn_80496338(void *out, void *inOut, RwInt32 numIn) {
    RwRaster *raster = (RwRaster *)inOut;

    switch (raster->cType & rwRASTERTYPEMASK) {
    case rwRASTERTYPENORMAL:
    case rwRASTERTYPETEXTURE:
        if (raster == raster->parent) {
            RwGameCubeRasterExt *rasterExt =
                RWPLUGINOFFSET(RwGameCubeRasterExt, raster, lbl_805F1658);

            if (raster->privateFlags & rwRASTERPALETTELOCKEDWRITE) {
                DCFlushRange(rasterExt->palette, (1 << raster->depth) * 2);
            }
            raster->privateFlags &= ~(rwRASTERPALETTELOCKEDREAD | rwRASTERPALETTELOCKEDWRITE);
            raster->palette = NULL;
        }
        return TRUE;
    case RASTERTYPEUNDEFINED:
    case rwRASTERTYPECAMERATEXTURE:
    default:
        break;
    }
    RWERROR((ERRORCANNOTUNLOCK));
    return FALSE;
}

/* Sets up a raster's type, flags, depth and GX formats from the flags it is created with: the
 * raster format's pixel format chooses the GX texture format, or the depth does when it has
 * none */
RwBool fn_80496408(RwRaster *raster, RwInt32 flags) {
    RwGameCubeRasterExt *rasterExt = RASTEREXT(raster);
    RwUInt32 format = flags & rwRASTERFORMATMASK;

    raster->cType = flags & rwRASTERTYPEMASK;
    raster->cFlags = flags & rwRASTERFLAGSMASK;
    switch (raster->cType) {
    case rwRASTERTYPECAMERATEXTURE:
        if (format & rwRASTERFORMATPALMASK) {
            RWERROR((ERRORDEBUGMESSAGE, "rwRASTERTYPECAMERATEXTURE can not be palletized"));
            return FALSE;
        }
    case rwRASTERTYPENORMAL:
    case rwRASTERTYPETEXTURE:
        if (!(format & rwRASTERFORMATPIXELFORMATMASK)) {
            /* The depth chooses the format */
            switch (raster->depth) {
            case 0:
                if (raster->cType == rwRASTERTYPECAMERATEXTURE) {
                    rasterExt->texFormat = GX_TF_RGB565;
                    format |= rwRASTERFORMAT565;
                    rasterExt->flags = 0;
                    raster->depth = 16;
                } else {
                    rasterExt->texFormat = GX_TF_RGB5A3;
                    format |= rwRASTERFORMAT1555;
                    rasterExt->flags = rwGAMECUBERASTERHASALPHA;
                    if (format & rwRASTERFORMATPAL4) {
                        raster->depth = 4;
                    } else if (format & rwRASTERFORMATPAL8) {
                        raster->depth = 8;
                    } else {
                        raster->depth = 16;
                    }
                }
                break;
            case 4:
                if (format & rwRASTERFORMATPAL4) {
                    rasterExt->texFormat = GX_TF_C4;
                    rasterExt->tlutFormat = GX_TL_RGB5A3;
                    format |= rwRASTERFORMAT1555;
                    rasterExt->flags = rwGAMECUBERASTERHASALPHA;
                } else {
                    rasterExt->texFormat = GX_TF_I4;
                    rasterExt->flags = 0;
                }
                break;
            case 8:
                if (format & rwRASTERFORMATPAL8) {
                    rasterExt->texFormat = GX_TF_C8;
                    rasterExt->tlutFormat = GX_TL_RGB5A3;
                    format |= rwRASTERFORMAT1555;
                    rasterExt->flags = rwGAMECUBERASTERHASALPHA;
                } else {
                    rasterExt->texFormat = GX_TF_I8;
                    rasterExt->flags = 0;
                }
                break;
            case 16:
                rasterExt->texFormat = GX_TF_RGB5A3;
                format |= rwRASTERFORMAT1555;
                rasterExt->flags = rwGAMECUBERASTERHASALPHA;
                break;
            case 32:
                rasterExt->texFormat = GX_TF_RGBA8;
                format |= rwRASTERFORMAT8888;
                rasterExt->flags = rwGAMECUBERASTERHASALPHA;
                break;
            default:
                RWERROR((ERRORINVALIDRASTERDEPTH));
                return FALSE;
            }
        } else {
            switch (format & rwRASTERFORMATPIXELFORMATMASK) {
            case rwRASTERFORMAT555:
                if (format & rwRASTERFORMATPAL4) {
                    rasterExt->texFormat = GX_TF_C4;
                    rasterExt->tlutFormat = GX_TL_RGB5A3;
                    raster->depth = 4;
                } else if (format & rwRASTERFORMATPAL8) {
                    rasterExt->texFormat = GX_TF_C8;
                    rasterExt->tlutFormat = GX_TL_RGB5A3;
                    raster->depth = 8;
                } else {
                    rasterExt->texFormat = GX_TF_RGB5A3;
                    raster->depth = 16;
                }
                rasterExt->flags = 0;
                break;
            case rwRASTERFORMAT565:
                if (format & rwRASTERFORMATPAL4) {
                    rasterExt->texFormat = GX_TF_C4;
                    rasterExt->tlutFormat = GX_TL_RGB565;
                    raster->depth = 4;
                } else if (format & rwRASTERFORMATPAL8) {
                    rasterExt->texFormat = GX_TF_C8;
                    rasterExt->tlutFormat = GX_TL_RGB565;
                    raster->depth = 8;
                } else {
                    rasterExt->texFormat = GX_TF_RGB565;
                    raster->depth = 16;
                }
                rasterExt->flags = 0;
                break;
            case rwRASTERFORMAT1555:
            case rwRASTERFORMAT4444:
                if (format & rwRASTERFORMATPAL4) {
                    rasterExt->texFormat = GX_TF_C4;
                    rasterExt->tlutFormat = GX_TL_RGB5A3;
                    raster->depth = 4;
                } else if (format & rwRASTERFORMATPAL8) {
                    rasterExt->texFormat = GX_TF_C8;
                    rasterExt->tlutFormat = GX_TL_RGB5A3;
                    raster->depth = 8;
                } else {
                    rasterExt->texFormat = GX_TF_RGB5A3;
                    raster->depth = 16;
                }
                rasterExt->flags = rwGAMECUBERASTERHASALPHA;
                break;
            case rwRASTERFORMAT888:
                if (format & rwRASTERFORMATPALMASK) {
                    RWERROR((ERRORDEBUGMESSAGE, "rwRASTERFORMAT888 invalid format"));
                    return FALSE;
                }
                rasterExt->texFormat = GX_TF_RGBA8;
                rasterExt->flags = 0;
                raster->depth = 32;
                break;
            case rwRASTERFORMAT8888:
                if (format & rwRASTERFORMATPALMASK) {
                    RWERROR((ERRORDEBUGMESSAGE, "rwRASTERFORMAT8888 invalid format"));
                    return FALSE;
                }
                rasterExt->texFormat = GX_TF_RGBA8;
                rasterExt->flags = rwGAMECUBERASTERHASALPHA;
                raster->depth = 32;
                break;
            case rwRASTERFORMATLUM8:
                RWERROR((ERRORDEBUGMESSAGE, "rwRASTERFORMATLUM8 invalid format"));
                return FALSE;
            case rwRASTERFORMAT16:
                RWERROR((ERRORDEBUGMESSAGE, "rwRASTERFORMAT16 invalid format"));
                return FALSE;
            case rwRASTERFORMAT24:
                RWERROR((ERRORDEBUGMESSAGE, "rwRASTERFORMAT24 invalid format"));
                return FALSE;
            case rwRASTERFORMAT32:
                RWERROR((ERRORDEBUGMESSAGE, "rwRASTERFORMAT32 invalid format"));
                return FALSE;
            default:
                RWERROR((ERRORINVALIDRASTERFORMAT));
                return FALSE;
            }
        }
        break;
    case rwRASTERTYPEZBUFFER:
    case rwRASTERTYPECAMERA:
        if (!(format & rwRASTERFORMATPIXELFORMATMASK)) {
            /* The frame buffer's format: 16-bit when antialiased */
            if (lbl_805F164C->aa) {
                rasterExt->texFormat = GX_TF_RGB565;
                format |= rwRASTERFORMAT565;
                rasterExt->flags = 0;
                raster->depth = 16;
            } else {
                rasterExt->texFormat = GX_TF_RGBA8;
                format |= rwRASTERFORMAT888;
                rasterExt->flags = 0;
                raster->depth = 32;
            }
        } else if (format != rwRASTERFORMAT16 || !lbl_805F164C->aa ||
                   (format == rwRASTERFORMAT24 && !lbl_805F164C->aa)) {
            /* Only a 16-bit raster of an antialiased frame buffer is accepted; the original also
             * tests a 24-bit raster without antialiasing, which cannot be reached */
            RWERROR((ERRORINVALIDRASTERFORMAT));
            return FALSE;
        }
        break;
    default:
        RWERROR((ERRORINVALIDRASTERFORMAT));
        return FALSE;
    }
    raster->cFormat = format >> 8;
    return TRUE;
}

/* Allocates the texels (and the palette) of a raster with a number of mip levels */
RwBool fn_80496A20(RwRaster *raster, RwUInt8 numLevels) {
    RwInt32 size = 0;
    RwGameCubeRasterExt *rasterExt = RWPLUGINOFFSET(RwGameCubeRasterExt, raster, lbl_805F1658);
    RwInt32 levels;

    rasterExt->maxLevel = numLevels - 1;
    fn_80495688(&levels, raster, 0);
    while (levels--) {
        size += fn_80495428(raster, levels);
    }

    if (RwRasterGetFormat(raster) & rwRASTERFORMATPALMASK) {
        RwInt32 allocSize = size + (1 << raster->depth) * 2 + 31;

        rasterExt->memory = RwMalloc(allocSize, rwMEMHINT_RASTER);
        if (rasterExt->memory == NULL) {
            RWERROR((E_RW_NOMEM, allocSize));
            return FALSE;
        }
        rasterExt->texels = (RwUInt8 *)(((RwUInt32)rasterExt->memory + 31) & ~31);
        rasterExt->palette = (RwUInt16 *)(rasterExt->texels + size);
        GXInitTlutObj(&rasterExt->tlutObj, rasterExt->palette, rasterExt->tlutFormat,
                      1 << raster->depth);
    } else {
        rasterExt->memory = RwMalloc(size + 31, rwMEMHINT_RASTER);
        if (rasterExt->memory == NULL) {
            RWERROR((E_RW_NOMEM, size + 31));
            return FALSE;
        }
        rasterExt->texels = (RwUInt8 *)(((RwUInt32)rasterExt->memory + 31) & ~31);
        if (raster->cType == rwRASTERTYPECAMERATEXTURE) {
            DCInvalidateRange(rasterExt->texels, size);
        }
    }
    return TRUE;
}

/* Creates a raster: out is unused, inOut the raster and numIn its flags */
RwBool fn_80496C58(void *out, void *inOut, RwInt32 numIn) {
    RwRaster *raster = (RwRaster *)inOut;
    RwGameCubeRasterExt *rasterExt = RWPLUGINOFFSET(RwGameCubeRasterExt, raster, lbl_805F1658);

    raster->stride = 0;
    rasterExt->texFormat = 0xFF;
    rasterExt->tlutFormat = 0xFF;
    rasterExt->flags = 0;
    rasterExt->memory = NULL;
    rasterExt->texels = NULL;
    rasterExt->palette = NULL;
    rasterExt->lockedTexels = NULL;
    rasterExt->lockBuffer = NULL;
    rasterExt->texRegion = NULL;
    rasterExt->syncToken = lbl_805F1678;
    rasterExt->maxLevel = 0xFF;
    rasterExt->lockedLevel = 0xFF;

    if (!fn_80496408(raster, numIn)) {
        return FALSE;
    }
    if (raster->width != 0 && raster->height != 0) {
        switch (raster->cType) {
        case rwRASTERTYPENORMAL:
        case rwRASTERTYPETEXTURE:
        case rwRASTERTYPECAMERATEXTURE:
            if (!(raster->cFlags & rwRASTERDONTALLOCATE)) {
                RwUInt8 numLevels;

                if (RwRasterGetFormat(raster) & rwRASTERFORMATMIPMAP) {
                    if (raster->width > raster->height) {
                        numLevels = (RwUInt8)fn_80491AB4(raster->width) + 1;
                    } else {
                        numLevels = (RwUInt8)fn_80491AB4(raster->height) + 1;
                    }
                } else {
                    numLevels = 1;
                }
                if (!fn_80496A20(raster, numLevels)) {
                    RWERROR((ERRORDEBUGMESSAGE, "Failed to create surface for texture"));
                    return FALSE;
                }
            }
            break;
        case rwRASTERTYPEZBUFFER:
        case rwRASTERTYPECAMERA:
            /* The frame buffer is used directly */
            raster->cFlags = rwRASTERDONTALLOCATE;
            break;
        case RASTERTYPEUNDEFINED:
        default:
            RWERROR((ERRORINVALIDRASTERFORMAT));
            return FALSE;
        }
    } else {
        raster->cFlags = rwRASTERDONTALLOCATE;
        return TRUE;
    }
    return TRUE;
}

/* Destroys a raster: waits for the GPU to finish with it, takes it off the 2D texture and frees
 * its memory */
RwBool fn_80497008(void *out, void *inOut, RwInt32 numIn) {
    RwRaster *raster = (RwRaster *)inOut;

    if (raster->parent == raster && !(raster->cFlags & rwRASTERDONTALLOCATE)) {
        switch (raster->cType) {
        case rwRASTERTYPENORMAL:
        case rwRASTERTYPETEXTURE:
        case rwRASTERTYPECAMERATEXTURE: {
            RwGameCubeRasterExt *rasterExt =
                RWPLUGINOFFSET(RwGameCubeRasterExt, raster, lbl_805F1658);

            RasterWaitForGPU(rasterExt);
            if (lbl_805F1664 != NULL && raster == lbl_805F1664->raster) {
                fn_8049714C(lbl_805F1664, NULL, 0);
            }
            RwFree(rasterExt->memory);
            break;
        }
        case rwRASTERTYPEZBUFFER:
        case rwRASTERTYPECAMERA:
            break;
        case RASTERTYPEUNDEFINED:
        default:
            RWERROR((ERRORINVALIDRASTERFORMAT));
            return FALSE;
        }
    }
    return TRUE;
}

/* Sets a texture's raster: out is the texture and inOut the raster */
RwBool fn_8049714C(void *out, void *inOut, RwInt32 numIn) {
    RwTexture *texture = (RwTexture *)out;

    texture->raster = (RwRaster *)inOut;
    RWPLUGINOFFSET(RwGameCubeTextureExt, texture, lbl_805F1670)->flags = rwGAMECUBETEXTURENOTBUILT;
    return TRUE;
}

/* Makes a sub raster of a raster: out is the sub raster and inOut the raster */
RwBool fn_80497168(void *out, void *inOut, RwInt32 numIn) {
    RwRaster *subRaster = (RwRaster *)out;
    RwRaster *raster = (RwRaster *)inOut;

    subRaster->stride = raster->stride;
    subRaster->depth = raster->depth;
    subRaster->cType = raster->cType;
    subRaster->cFormat = raster->cFormat;
    subRaster->cpPixels = NULL;
    return TRUE;
}
