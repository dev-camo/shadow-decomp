/* RenderWare GameCube raster and image conversion: packing colors into the GameCube texel formats
 * and back, copying rasters into images and images into rasters (palettizing images with the
 * palette quantizer when a raster needs fewer bits), and finding the raster format and size for
 * an image. The original file name is unknown. */
#include "rw/rpworld.h"

/* The largest raster side made from an image */
#define MAXRASTERSIZE 1024

/* Packs a color into a texel, and unpacks a texel into a color */
typedef RwUInt32 (*PixelPackFunc)(RwRGBA *color);
typedef void (*PixelUnpackFunc)(RwRGBA *color, RwUInt32 pixel);

/* The power of two at or below a value (as its exponent) */
RwInt32 fn_80491AB4(RwInt32 value) {
    RwInt32 exponent = -1;

    while (value != 0) {
        value >>= 1;
        exponent++;
    }
    return exponent;
}

/* Packs a color into an opaque RGB5A3 texel (1555 with the alpha bit set) */
RwUInt32 fn_80491AD4(RwRGBA *color) {
    return 0x8000 | ((color->red << 7) & 0x7C00) | ((color->green << 2) & 0x03E0) |
           ((color->blue >> 3) & 0x001F);
}

/* Packs a color into an RGB565 texel */
RwUInt32 fn_80491AF4(RwRGBA *color) {
    return ((color->red << 8) & 0xF800) | ((color->green << 3) & 0x07E0) |
           ((color->blue >> 3) & 0x001F);
}

/* Packs a color into an RGB5A3 texel: opaque colors as 1555, the rest as 3444 */
RwUInt32 fn_80491B14(RwRGBA *color) {
    if (color->alpha != 0xFF) {
        return ((color->alpha << 7) & 0x7000) | ((color->red << 4) & 0x0F00) |
               (color->green & 0x00F0) | ((color->blue >> 4) & 0x000F);
    }
    return fn_80491AD4(color);
}

/* Packs a color into an opaque RGBA8 texel */
RwUInt32 fn_80491B64(RwRGBA *color) {
    return 0xFF000000 | (color->red << 16) | (color->green << 8) | color->blue;
}

/* Packs a color into an RGBA8 texel */
RwUInt32 fn_80491B88(RwRGBA *color) {
    return (color->alpha << 24) | (color->red << 16) | (color->green << 8) | color->blue;
}

/* Packs a color into a pixel of a raster format: out is the pixel, inOut the color and numIn the
 * format */
RwBool fn_80491BAC(void *out, void *inOut, RwInt32 numIn) {
    RwRGBA *color = (RwRGBA *)inOut;
    RwUInt32 pixel;

    switch (numIn & rwRASTERFORMATPIXELFORMATMASK) {
    case rwRASTERFORMATDEFAULT:
        /* The default format is the 565 one */
        if (!fn_80491BAC(&pixel, inOut, rwRASTERFORMAT565)) {
            return FALSE;
        }
        break;
    case rwRASTERFORMATLUM8:
        RWERRORCORE((E_RW_DEBUGMESSAGE, "rwRASTERFORMATLUM8 not yet supported"));
        break;
    case rwRASTERFORMAT1555:
    case rwRASTERFORMAT4444:
        pixel = fn_80491B14(color);
        break;
    case rwRASTERFORMAT555:
        pixel = fn_80491AD4(color);
        break;
    case rwRASTERFORMAT565:
        pixel = fn_80491AF4(color);
        break;
    case rwRASTERFORMAT8888:
        pixel = fn_80491B88(color);
        break;
    case rwRASTERFORMAT888:
        pixel = fn_80491B64(color);
        break;
    default:
        RWERRORCORE((E_RW_INVRASTERFORMAT));
        break;
    }
    *(RwUInt32 *)out = pixel;
    return TRUE;
}

/* Unpacks an RGB5A3 texel stored with the alpha bit set (555) */
void fn_80491D98(RwRGBA *color, RwUInt32 pixel) {
    color->red = (pixel >> 7) & 0xF8;
    color->green = (pixel >> 2) & 0xF8;
    color->blue = (pixel << 3) & 0xF8;
    color->alpha = 0xFF;
}

/* Unpacks an RGB565 texel */
void fn_80491DBC(RwRGBA *color, RwUInt32 pixel) {
    color->red = (pixel >> 8) & 0xF8;
    color->green = (pixel >> 3) & 0xFC;
    color->blue = (pixel << 3) & 0xF8;
    color->alpha = 0xFF;
}

/* Unpacks an RGB5A3 texel of a 1555 raster: a translucent texel's alpha is dropped */
void fn_80491DE0(RwRGBA *color, RwUInt32 pixel) {
    if (pixel & 0x8000) {
        color->red = (pixel >> 7) & 0xF8;
        color->green = (pixel >> 2) & 0xF8;
        color->blue = (pixel << 3) & 0xF8;
        color->alpha = 0xFF;
    } else {
        color->red = (pixel >> 4) & 0xF0;
        color->green = pixel & 0xF0;
        color->blue = (pixel << 4) & 0xF0;
        color->alpha = 0;
    }
}

/* Unpacks an RGB5A3 texel of a 4444 raster */
void fn_80491E30(RwRGBA *color, RwUInt32 pixel) {
    if (pixel & 0x8000) {
        color->red = (pixel >> 7) & 0xF8;
        color->green = (pixel >> 2) & 0xF8;
        color->blue = (pixel << 3) & 0xF8;
        color->alpha = 0xFF;
    } else {
        color->red = (pixel >> 4) & 0xF0;
        color->green = pixel & 0xF0;
        color->blue = (pixel << 4) & 0xF0;
        color->alpha = (pixel >> 7) & 0xE0;
    }
}

/* Unpacks an opaque RGBA8 texel */
void fn_80491E80(RwRGBA *color, RwUInt32 pixel) {
    color->alpha = 0xFF;
    color->red = (pixel >> 16) & 0xFF;
    color->green = (pixel >> 8) & 0xFF;
    color->blue = pixel & 0xFF;
}

/* Unpacks an RGBA8 texel */
void fn_80491EA0(RwRGBA *color, RwUInt32 pixel) {
    color->alpha = pixel >> 24;
    color->red = (pixel >> 16) & 0xFF;
    color->green = (pixel >> 8) & 0xFF;
    color->blue = pixel & 0xFF;
}

/* Unpacks a pixel of a raster format into a color: out is the color, inOut the pixel and numIn the
 * format */
RwBool fn_80491EC0(void *out, void *inOut, RwInt32 numIn) {
    RwRGBA *color = (RwRGBA *)out;
    RwUInt32 pixel = *(RwUInt32 *)inOut;

    switch (numIn & rwRASTERFORMATPIXELFORMATMASK) {
    case rwRASTERFORMATDEFAULT:
        /* The default format is the 565 one */
        if (!fn_80491EC0(out, inOut, rwRASTERFORMAT565)) {
            return FALSE;
        }
        break;
    case rwRASTERFORMATLUM8:
        RWERRORCORE((E_RW_DEBUGMESSAGE, "rwRASTERFORMATLUM8 not yet supported"));
        break;
    case rwRASTERFORMAT555:
        fn_80491D98(color, pixel);
        break;
    case rwRASTERFORMAT1555:
        fn_80491DE0(color, pixel);
        break;
    case rwRASTERFORMAT565:
        fn_80491DBC(color, pixel);
        break;
    case rwRASTERFORMAT4444:
        fn_80491E30(color, pixel);
        break;
    case rwRASTERFORMAT888:
        fn_80491E80(color, pixel);
        break;
    case rwRASTERFORMAT8888:
        fn_80491EA0(color, pixel);
        break;
    default:
        RWERRORCORE((E_RW_INVRASTERFORMAT));
        break;
    }
    return TRUE;
}

/* The unpacking function of a raster's pixel format (inferred helper) */
static inline PixelUnpackFunc RasterUnpackFunc(RwRaster *raster) {
    PixelUnpackFunc unpack = NULL;

    switch (RwRasterGetFormat(raster) & rwRASTERFORMATPIXELFORMATMASK) {
    case rwRASTERFORMATLUM8:
        RWERRORCORE((E_RW_DEBUGMESSAGE, "rwRASTERFORMATLUM8 not yet supported"));
        break;
    case rwRASTERFORMAT555:
        unpack = fn_80491D98;
        break;
    case rwRASTERFORMAT565:
        unpack = fn_80491DBC;
        break;
    case rwRASTERFORMAT1555:
        unpack = fn_80491DE0;
        break;
    case rwRASTERFORMAT4444:
        unpack = fn_80491E30;
        break;
    case rwRASTERFORMAT8888:
        unpack = fn_80491EA0;
        break;
    case rwRASTERFORMAT888:
        unpack = fn_80491E80;
        break;
    default:
        RWERRORCORE((E_RW_INVRASTERFORMAT));
        break;
    }
    return unpack;
}

/* The packing function of a raster's pixel format (inferred helper) */
static inline PixelPackFunc RasterPackFunc(RwRaster *raster) {
    PixelPackFunc pack = NULL;

    switch (RwRasterGetFormat(raster) & rwRASTERFORMATPIXELFORMATMASK) {
    case rwRASTERFORMATLUM8:
        RWERRORCORE((E_RW_DEBUGMESSAGE, "rwRASTERFORMATLUM8 not yet supported"));
        break;
    case rwRASTERFORMAT555:
        pack = fn_80491AD4;
        break;
    case rwRASTERFORMAT565:
        pack = fn_80491AF4;
        break;
    case rwRASTERFORMAT1555:
    case rwRASTERFORMAT4444:
        pack = fn_80491B14;
        break;
    case rwRASTERFORMAT888:
        pack = fn_80491B64;
        break;
    case rwRASTERFORMAT8888:
        pack = fn_80491B88;
        break;
    default:
        RWERRORCORE((E_RW_INVRASTERFORMAT));
        break;
    }
    return pack;
}

/* Copies a locked raster into a 4-bit image: only 4-bit rasters can be */
void fn_804920C8(RwImage *image, RwRaster *raster) {
    PixelUnpackFunc unpack = RasterUnpackFunc(raster);

    switch (raster->depth) {
    case 4: {
        RwInt32 i;
        RwInt32 y;

        for (i = 0; i < 16; i++) {
            unpack(&image->palette[i], ((RwUInt16 *)raster->palette)[i]);
        }
        for (y = 0; y < raster->height; y++) {
            RwUInt8 *destination = image->cpPixels + image->stride * y;
            RwUInt8 *source = raster->cpPixels + raster->stride * y;
            RwInt32 x;

            for (x = 0; x < raster->width; x += 2) {
                destination[0] = (*source >> 4) & 0xF;
                destination[1] = *source & 0xF;
                source++;
                destination += 2;
            }
        }
        break;
    }
    case 8:
    case 16:
    case 32:
        RWERRORCORE((E_RW_DEBUGMESSAGE,
                     "Conversion from 8/16/32bit rasters to 4bit images is not supported"));
        break;
    case 24:
    default:
        RWERRORCORE((E_RW_INVRASTERDEPTH));
        break;
    }
}

/* Copies a locked raster into an 8-bit image: only 4- and 8-bit rasters can be */
void fn_8049231C(RwImage *image, RwRaster *raster) {
    PixelUnpackFunc unpack = RasterUnpackFunc(raster);

    switch (raster->depth) {
    case 4: {
        RwInt32 i;
        RwInt32 y;

        for (i = 0; i < 16; i++) {
            unpack(&image->palette[i], ((RwUInt16 *)raster->palette)[i]);
        }
        for (y = 0; y < raster->height; y++) {
            RwUInt8 *destination = image->cpPixels + image->stride * y;
            RwUInt8 *source = raster->cpPixels + raster->stride * y;
            RwInt32 x;

            for (x = 0; x < raster->width; x += 2) {
                destination[0] = (*source >> 4) & 0xF;
                destination[1] = *source & 0xF;
                source++;
                destination += 2;
            }
        }
        break;
    }
    case 8: {
        RwInt32 i;
        RwInt32 y;

        for (i = 0; i < 256; i++) {
            unpack(&image->palette[i], ((RwUInt16 *)raster->palette)[i]);
        }
        for (y = 0; y < raster->height; y++) {
            RwUInt8 *source = raster->cpPixels + raster->stride * y;
            RwUInt8 *destination = image->cpPixels + image->stride * y;
            RwInt32 x;

            for (x = 0; x < raster->width; x++) {
                *destination = *source;
                destination++;
                source++;
            }
        }
        break;
    }
    case 16:
    case 32:
        RWERRORCORE((E_RW_DEBUGMESSAGE,
                     "Conversion from 16/32bit rasters to 8bit images is not supported"));
        break;
    case 24:
    default:
        RWERRORCORE((E_RW_INVRASTERDEPTH));
        break;
    }
}

/* Copies a locked raster into a 32-bit image */
void fn_80492610(RwImage *image, RwRaster *raster) {
    PixelUnpackFunc unpack = RasterUnpackFunc(raster);
    RwRGBA palette256[256];
    RwRGBA palette16[16];

    switch (raster->depth) {
    case 4: {
        RwInt32 i;
        RwInt32 y;

        for (i = 0; i < 16; i++) {
            unpack(&palette16[i], ((RwUInt16 *)raster->palette)[i]);
        }
        for (y = 0; y < raster->height; y++) {
            RwRGBA *destination = (RwRGBA *)(image->cpPixels + image->stride * y);
            RwUInt8 *source = raster->cpPixels + raster->stride * y;
            RwInt32 x;

            for (x = 0; x < raster->width; x += 2) {
                destination[0] = palette16[(*source >> 4) & 0xF];
                destination[1] = palette16[*source & 0xF];
                source++;
                destination += 2;
            }
        }
        break;
    }
    case 8: {
        RwInt32 i;
        RwInt32 y;

        for (i = 0; i < 256; i++) {
            unpack(&palette256[i], ((RwUInt16 *)raster->palette)[i]);
        }
        for (y = 0; y < raster->height; y++) {
            RwUInt8 *source = raster->cpPixels + raster->stride * y;
            RwRGBA *destination = (RwRGBA *)(image->cpPixels + image->stride * y);
            RwInt32 x;

            for (x = 0; x < raster->width; x++) {
                *destination = palette256[*source];
                destination++;
                source++;
            }
        }
        break;
    }
    case 16: {
        RwInt32 y;

        for (y = 0; y < raster->height; y++) {
            RwInt32 x;
            RwUInt16 *source = (RwUInt16 *)(raster->cpPixels + raster->stride * y);
            RwRGBA *destination = (RwRGBA *)(image->cpPixels + image->stride * y);

            for (x = 0; x < raster->width; x++) {
                unpack(destination, *source);
                destination++;
                source++;
            }
        }
        break;
    }
    case 24:
        RWERRORCORE((E_RW_INVRASTERDEPTH));
        break;
    case 32: {
        RwInt32 y;

        for (y = 0; y < raster->height; y++) {
            RwInt32 x;
            RwUInt32 *source = (RwUInt32 *)(raster->cpPixels + raster->stride * y);
            RwRGBA *destination = (RwRGBA *)(image->cpPixels + image->stride * y);

            for (x = 0; x < raster->width; x++) {
                unpack(destination, *source);
                destination++;
                source++;
            }
        }
        break;
    }
    default:
        RWERRORCORE((E_RW_INVRASTERDEPTH));
        break;
    }
}

/* Copies a raster into an image, locking the raster and its palette for reading when they are not
 * locked: out is the image and inOut the raster */
RwBool fn_804929F0(void *out, void *inOut, RwInt32 numIn) {
    RwImage *image = (RwImage *)out;
    RwRaster *raster = (RwRaster *)inOut;
    RwBool unlockPixels = FALSE;
    RwBool unlockPalette = FALSE;

    if (!(raster->privateFlags & rwRASTERPIXELLOCKEDREAD)) {
        fn_80489F70(raster, 0, rwRASTERLOCKREAD);
        unlockPixels = TRUE;
    }
    if ((RwRasterGetFormat(raster) & rwRASTERFORMATPALMASK) &&
        !(raster->privateFlags & rwRASTERPALETTELOCKEDREAD)) {
        fn_80489C94(raster, rwRASTERLOCKREAD);
        unlockPalette = TRUE;
    }

    switch (image->depth) {
    case 4:
        fn_804920C8(image, raster);
        break;
    case 8:
        fn_8049231C(image, raster);
        break;
    case 32:
        fn_80492610(image, raster);
        break;
    default:
        RWERRORCORE((E_RW_INVIMAGEDEPTH));
        break;
    }

    if (unlockPalette == TRUE) {
        fn_80489B88(raster);
    }
    if (unlockPixels == TRUE) {
        fn_80489B40(raster);
    }
    return TRUE;
}

/* A palettized copy of an image, with pixels of the given depth (inferred) */
RwImage *fn_80492B24(RwImage *image, RwInt32 depth) {
    RwPalQuant palQuant;
    RwImage *palettized = fn_80487D08(image->width, image->height, depth);

    if (palettized == NULL) {
        return NULL;
    }
    fn_80487E18(palettized);
    if (!fn_8048FED8(&palQuant)) {
        return NULL;
    }
    fn_8048DEB0(&palQuant, image, 1.0f);
    fn_8048F5D0(palettized->palette, 1 << depth, &palQuant);
    fn_8048FAB0(palettized->cpPixels, palettized->stride, palettized->depth, FALSE, &palQuant,
                image);
    fn_804903F8(&palQuant);
    return palettized;
}

/* Copies an image into a locked 4-bit raster, palettizing 8- and 32-bit images; the palette is
 * written when the locked mip level is the first */
void fn_80492BE8(RwRaster *raster, RwImage *image) {
    RwGameCubeRasterExt *rasterExt =
        RWPLUGINOFFSET(RwGameCubeRasterExt, raster->parent, lbl_805F1658);
    PixelPackFunc pack = RasterPackFunc(raster);

    switch (image->depth) {
    case 4: {
        RwInt32 y;

        for (y = 0; y < raster->height; y++) {
            RwUInt8 *source = image->cpPixels + image->stride * y;
            RwUInt8 *destination = raster->cpPixels + raster->stride * y;
            RwInt32 x;

            for (x = 0; x < raster->width; x += 2) {
                *destination = ((source[0] & 0xF) << 4) | (source[1] & 0xF);
                source += 2;
                destination++;
            }
        }
        if (rasterExt->lockedLevel == 0) {
            RwUInt16 *palette = (RwUInt16 *)raster->palette;
            RwInt32 i;

            for (i = 0; i < 16; i++) {
                palette[i] = pack(&image->palette[i]);
            }
        }
        break;
    }
    case 8:
    case 32: {
        RwImage *palettized = fn_80492B24(image, raster->depth);

        if (palettized != NULL) {
            fn_80492BE8(raster, palettized);
            fn_80487DB0(palettized);
        }
        break;
    }
    default:
        RWERRORCORE((E_RW_INVIMAGEDEPTH));
        break;
    }
}

/* Copies an image into a locked 8-bit raster, palettizing 32-bit images */
void fn_80492E40(RwRaster *raster, RwImage *image) {
    PixelPackFunc pack = RasterPackFunc(raster);

    switch (image->depth) {
    case 4:
    case 8: {
        RwInt32 y;
        RwGameCubeRasterExt *rasterExt =
            RWPLUGINOFFSET(RwGameCubeRasterExt, raster->parent, lbl_805F1658);

        for (y = 0; y < raster->height; y++) {
            RwUInt8 *source = image->cpPixels + image->stride * y;
            RwUInt8 *destination = raster->cpPixels + raster->stride * y;
            RwInt32 x;

            for (x = 0; x < raster->width; x++) {
                *destination = *source;
                destination++;
                source++;
            }
        }
        if (rasterExt->lockedLevel == 0) {
            RwUInt16 *palette = (RwUInt16 *)raster->palette;
            RwInt32 i;

            for (i = 0; i < (1 << image->depth); i++) {
                palette[i] = pack(&image->palette[i]);
            }
        }
        break;
    }
    case 32: {
        RwImage *palettized = fn_80492B24(image, raster->depth);

        if (palettized != NULL) {
            fn_80492E40(raster, palettized);
            fn_80487DB0(palettized);
        }
        break;
    }
    default:
        RWERRORCORE((E_RW_INVIMAGEDEPTH));
        break;
    }
}

/* Copies an image into a locked 16-bit raster */
void fn_8049309C(RwRaster *raster, RwImage *image) {
    PixelPackFunc pack = RasterPackFunc(raster);
    RwUInt16 palette[256];

    switch (image->depth) {
    case 4:
    case 8: {
        RwInt32 i;
        RwInt32 y;

        for (i = 0; i < (1 << image->depth); i++) {
            palette[i] = pack(&image->palette[i]);
        }
        for (y = 0; y < raster->height; y++) {
            RwUInt8 *source = image->cpPixels + image->stride * y;
            RwUInt16 *destination = (RwUInt16 *)(raster->cpPixels + raster->stride * y);
            RwInt32 x;

            for (x = 0; x < raster->width; x++) {
                *destination = palette[*source];
                destination++;
                source++;
            }
        }
        break;
    }
    case 32: {
        RwInt32 y;

        for (y = 0; y < raster->height; y++) {
            RwInt32 x;
            RwRGBA *source = (RwRGBA *)(image->cpPixels + image->stride * y);
            RwUInt16 *destination = (RwUInt16 *)(raster->cpPixels + raster->stride * y);

            for (x = 0; x < raster->width; x++) {
                *destination = pack(source);
                destination++;
                source++;
            }
        }
        break;
    }
    default:
        RWERRORCORE((E_RW_INVIMAGEDEPTH));
        break;
    }
}

/* Copies an image into a locked 32-bit raster */
void fn_80493330(RwRaster *raster, RwImage *image) {
    PixelPackFunc pack = RasterPackFunc(raster);
    RwUInt32 palette[256];

    switch (image->depth) {
    case 4:
    case 8: {
        RwInt32 i;
        RwInt32 y;

        for (i = 0; i < (1 << image->depth); i++) {
            palette[i] = pack(&image->palette[i]);
        }
        for (y = 0; y < raster->height; y++) {
            RwUInt8 *source = image->cpPixels + image->stride * y;
            RwUInt32 *destination = (RwUInt32 *)(raster->cpPixels + raster->stride * y);
            RwInt32 x;

            for (x = 0; x < raster->width; x++) {
                *destination = palette[*source];
                destination++;
                source++;
            }
        }
        break;
    }
    case 32: {
        RwInt32 y;

        for (y = 0; y < raster->height; y++) {
            RwInt32 x;
            RwRGBA *source = (RwRGBA *)(image->cpPixels + image->stride * y);
            RwUInt32 *destination = (RwUInt32 *)(raster->cpPixels + raster->stride * y);

            for (x = 0; x < raster->width; x++) {
                *destination = pack(source);
                destination++;
                source++;
            }
        }
        break;
    }
    default:
        RWERRORCORE((E_RW_INVIMAGEDEPTH));
        break;
    }
}

/* Copies an image into a raster, locking the raster and its palette for writing when they are not
 * locked: inOut is the image; returns FALSE when a lock fails */
RwBool fn_804935C4(void *out, void *inOut, RwInt32 numIn) {
    RwRaster *raster = (RwRaster *)out;
    RwInt32 format = RwRasterGetFormat(raster);
    RwImage *image = (RwImage *)inOut;
    RwBool pixelsLocked = FALSE;
    RwBool paletteLocked = FALSE;

    if (raster->privateFlags & rwRASTERPIXELLOCKEDWRITE) {
        pixelsLocked = TRUE;
    }
    if (!pixelsLocked && fn_80489F70(raster, 0, rwRASTERLOCKWRITE | rwRASTERLOCKNOFETCH) == NULL) {
        return FALSE;
    }

    if (format &= rwRASTERFORMATPALMASK) {
        if (raster->privateFlags & rwRASTERPALETTELOCKEDWRITE) {
            paletteLocked = TRUE;
        }
        if (!paletteLocked &&
            fn_80489C94(raster, rwRASTERLOCKWRITE | rwRASTERLOCKNOFETCH) == NULL) {
            return FALSE;
        }
    }

    switch (raster->depth) {
    case 4:
        fn_80492BE8(raster, image);
        break;
    case 8:
        fn_80492E40(raster, image);
        break;
    case 16:
        fn_8049309C(raster, image);
        break;
    case 24:
        RWERRORCORE((E_RW_INVRASTERDEPTH));
        break;
    case 32:
        fn_80493330(raster, image);
        break;
    default:
        RWERRORCORE((E_RW_INVRASTERDEPTH));
        break;
    }

    if (format != 0 && !paletteLocked) {
        fn_80489B88(raster);
    }
    if (!pixelsLocked) {
        fn_80489B40(raster);
    }
    return TRUE;
}

/* The raster format that suits an image's colors: 4444 when any alpha is between 0x10 and 0xFE,
 * otherwise 1555 when any color is translucent and 565 when all are opaque, palettized for 4- and
 * 8-bit images */
RwInt32 fn_8049375C(RwImage *image) {
    RwInt32 depth = image->depth;
    RwBool translucent = FALSE;
    RwInt32 format;

    if (depth == 4 || depth == 8) {
        RwInt32 width = image->width;
        RwInt32 height = image->height;
        RwInt32 y;
        RwUInt8 *row = image->cpPixels;
        RwRGBA *palette = image->palette;

        for (y = 0; y < height; y++) {
            RwUInt8 *pixel = row;
            RwInt32 x;

            for (x = 0; x < width; x++) {
                RwUInt8 alpha = palette[*pixel].alpha;

                if (alpha != 0xFF) {
                    translucent = TRUE;
                    if (alpha > 0xF) {
                        format = rwRASTERFORMAT4444;
                        if (depth == 4) {
                            format |= rwRASTERFORMATPAL4;
                        } else {
                            format |= rwRASTERFORMATPAL8;
                        }
                        return format;
                    }
                }
                pixel++;
            }
            row += image->stride;
        }
    } else {
        RwInt32 width = image->width;
        RwInt32 height = image->height;
        RwInt32 y;
        RwUInt8 *row = image->cpPixels;

        for (y = 0; y < height; y++) {
            RwRGBA *pixel = (RwRGBA *)row;
            RwInt32 x;

            for (x = 0; x < width; x++) {
                RwUInt8 alpha = pixel->alpha;

                if (alpha != 0xFF) {
                    translucent = TRUE;
                    if (alpha > 0xF) {
                        return rwRASTERFORMAT4444;
                    }
                }
                pixel++;
            }
            row += image->stride;
        }
    }

    format = translucent ? rwRASTERFORMAT1555 : rwRASTERFORMAT565;
    if (depth == 4) {
        format |= rwRASTERFORMATPAL4;
    } else if (depth == 8) {
        format |= rwRASTERFORMATPAL8;
    }
    return format;
}

/* Sets a raster's type, size and format for an image: out is the raster, inOut the image and numIn
 * the raster type and format flags; returns FALSE for an unsupported type */
RwBool fn_80493888(void *out, void *inOut, RwInt32 numIn) {
    RwRaster *raster = (RwRaster *)out;
    RwImage *image = (RwImage *)inOut;
    RwInt32 type = numIn & rwRASTERTYPEMASK;

    raster->cType = type;
    raster->depth = 0;
    switch (type) {
    case rwRASTERTYPENORMAL:
    case rwRASTERTYPETEXTURE:
    case rwRASTERTYPECAMERATEXTURE: {
        RwInt32 format;

        raster->width = (image->width > MAXRASTERSIZE) ? MAXRASTERSIZE : image->width;
        raster->height = (image->height > MAXRASTERSIZE) ? MAXRASTERSIZE : image->height;
        if (numIn & rwRASTERFORMATMIPMAP) {
            raster->width = 1 << fn_80491AB4(raster->width);
            raster->height = 1 << fn_80491AB4(raster->height);
        }
        format = fn_8049375C(image);
        raster->cFormat =
            (format | (numIn & (rwRASTERFORMATMIPMAP | rwRASTERFORMATAUTOMIPMAP))) >> 8;
        return TRUE;
    }
    case rwRASTERTYPEZBUFFER:
    case rwRASTERTYPECAMERA:
        raster->cFormat = 0;
        raster->width = image->width;
        raster->height = image->height;
        return TRUE;
    default:
        RWERRORCORE((E_RW_INVRASTERFORMAT));
        return FALSE;
    }
}
