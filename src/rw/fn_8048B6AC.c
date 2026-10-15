/* RenderWare textures and texture dictionaries: the texture module, reading textures and their
 * mip levels from image files, generating mip levels, palettizing images for rasters, creating,
 * naming and destroying textures, and the dictionaries that hold them. The original file name is
 * unknown. */
#include "rw/rwcore.h"
#include <stddef.h>
#include <string.h>

#define rwMEMHINTDUR_EVENT 0x30000
#define rwMEMHINTDUR_GLOBAL 0x40000

/* chunk and module IDs */
#define rwID_TEXTURE 0x6
#define rwID_TEXDICTIONARY 0x16
#define rwID_TEXTUREMODULE 0x408

/* object type */
#define rwTEXDICTIONARY 6

/* raster types and formats */
#define rwRASTERTYPETEXTURE 0x4
#define rwRASTERFORMATAUTOMIPMAP 0x1000
#define rwRASTERFORMATPAL8 0x2000
#define rwRASTERFORMATPAL4 0x4000
#define rwRASTERFORMATMIPMAP 0x8000
#define rwRASTERFORMATMASK 0xFF00

/* A raster's format bits */
#define RwRasterGetFormat(raster) (((raster)->cFormat & (rwRASTERFORMATMASK >> 8)) << 8)

/* raster lock modes */
#define rwRASTERLOCKWRITE 0x1
#define rwRASTERLOCKREAD 0x2
#define rwRASTERLOCKNOFETCH 0x4

/* errors (inferred) */
#define E_RW_INVIMAGEFORMAT 0x80000009  /* the image cannot be put in a raster */
#define E_RW_STRINGTRUNCATED 0x8000001E /* reported with the string, its limit and the cut */
#define E_RW_READTEXMASK 0x16           /* a texture and mask could not be read */

/* The core's error plugin ID (inferred) */
#define rwCOREERRORPLUGIN 1

/* Reports an error of the core; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwCOREERRORPLUGIN;                                                       \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

/* The size of the buffers image file names are built in */
#define rwTEXTUREFILENAMELENGTH 256

/* The most mip levels of a raster */
#define rwMAXTEXTUREMIPLEVELS 16

/* Copies a string into a buffer of size bytes; a longer string is reported and cut */
#define TEXTURECOPYSTRING(buffer, string, size)                                                    \
    do {                                                                                           \
        rwstrncpy((buffer), (string), (size));                                                     \
        if (rwstrlen(string) >= (size)) {                                                          \
            RWERROR((E_RW_STRINGTRUNCATED, (string), (size), (size) - 1, (string)[(size) - 1]));   \
            (buffer)[(size) - 1] = '\0';                                                           \
        }                                                                                          \
    } while (0)

/* Copies an optional mask name into a buffer; no mask leaves it empty */
#define TEXTURECOPYMASKNAME(buffer, maskName)                                                      \
    do {                                                                                           \
        (buffer)[0] = '\0';                                                                        \
        if ((maskName) != NULL && (maskName)[0] != '\0') {                                         \
            TEXTURECOPYSTRING((buffer), (maskName), rwTEXTUREFILENAMELENGTH);                      \
        }                                                                                          \
    } while (0)

/* The texture whose dictionary link this is, and the dictionary whose list link this is */
#define TEXTUREFROMDICTLINK(link)                                                                  \
    ((RwTexture *)((RwUInt8 *)(link) - offsetof(RwTexture, lInDictionary)))
#define DICTFROMLINK(link)                                                                         \
    ((RwTexDictionary *)((RwUInt8 *)(link) - offsetof(RwTexDictionary, lInInstance)))

typedef RwTexture *(*RwTextureCallBackRead)(const RwChar *name, const RwChar *maskName);
typedef RwTexture *(*RwTextureCallBackFind)(const RwChar *name);
typedef RwTexture *(*RwTextureCallBack)(RwTexture *texture, void *data);
typedef RwRaster *(*RwTextureCallBackMipmapGeneration)(RwRaster *raster, RwImage *image);
typedef RwBool (*RwTextureCallBackMipmapName)(RwChar *name, RwChar *maskName, RwUInt8 mipLevel,
                                              RwInt32 format);

/* The engine globals of the texture module (0x34 bytes) */
typedef struct rwTextureGlobals {
    /* 0x00 */ RwLinkList texDictList;
    /* 0x08 */ RwFreeList *textureFreeList;
    /* 0x0C */ RwFreeList *texDictFreeList;
    /* 0x10 */ RwTexDictionary *currentTexDict;
    /* 0x14 */ RwTextureCallBackRead textureReadCallBack;
    /* 0x18 */ RwTextureCallBackFind textureFindCallBack;
    /* 0x1C */ RwBool haveTexMipmaps;     /* new textures are mipmapped */
    /* 0x20 */ RwBool generateTexMipmaps; /* and their mip levels are generated */
    /* 0x24 */ void *field_0x24;          /* freed with the memory functions on close */
    /* 0x28 */ RwUInt16 field_0x28;
    /* 0x2C */ RwTextureCallBackMipmapGeneration mipmapGenerationCallBack;
    /* 0x30 */ RwTextureCallBackMipmapName mipmapNameCallBack;
} rwTextureGlobals;

#define RWTEXTUREGLOBAL(var)                                                                       \
    (RWPLUGINOFFSET(rwTextureGlobals, lbl_805F15B4, lbl_805F15D4.globalsOffset)->var)

/* The texture and texture dictionary plugin registries */
RwPluginRegistry lbl_8056E030 = {sizeof(RwTexture), sizeof(RwTexture), 0, 0, NULL, NULL};
RwPluginRegistry lbl_8056E048 = {
    sizeof(RwTexDictionary), sizeof(RwTexDictionary), 0, 0, NULL, NULL};

RwFreeList lbl_805E2D10; /* the texture free list */
RwFreeList lbl_805E2D34; /* the texture dictionary free list */

RwModuleInfo lbl_805F15D4;     /* the texture module */
RwTexDictionary *lbl_805F15D0; /* the dictionary created when the module opens */

/* The hex digits mip level names end in */
static const RwChar lbl_8051C2D0[] = "0123456789abcdef";

/* The default mip level names: the image file names with "m" and the level in hex added */
RwBool fn_8048B6AC(RwChar *name, RwChar *maskName, RwUInt8 mipLevel, RwInt32 format) {
    RwChar suffix[3];
    RwBool hasDigit = FALSE;

    suffix[0] = 'm';
    if (mipLevel != 0 && mipLevel < 16) {
        hasDigit = TRUE;
    }
    suffix[1] = hasDigit ? lbl_8051C2D0[mipLevel] : '\0';
    suffix[2] = '\0';
    if (suffix[1] != '\0') {
        rwstrcat(name, suffix);
        if (maskName != NULL && maskName[0] != '\0') {
            rwstrcat(maskName, suffix);
        }
    }
    return TRUE;
}

/* Replaces an image with one of depth 4 or 8 bits, palettized from its colors */
RwBool fn_8048B774(RwImage **image, RwInt32 depth) {
    RwRGBA palette[256];
    RwPalQuant palQuant;
    RwInt32 numColors;
    RwImage *palettized;

    if (!fn_8048FED8(&palQuant)) {
        return FALSE;
    }
    fn_8048DEB0(&palQuant, *image, 1.0f);
    numColors = 1 << depth;
    fn_8048F5D0(palette, numColors, &palQuant);

    palettized = fn_80487D08((*image)->width, (*image)->height, depth);
    if (palettized != NULL) {
        fn_80487E18(palettized);
        fn_8048FAB0(palettized->cpPixels, palettized->stride, palettized->depth, FALSE, &palQuant,
                    *image);
        memcpy(palettized->palette, palette, numColors * sizeof(RwRGBA));
        fn_80487DB0(*image);
        *image = palettized;
    } else {
        return FALSE;
    }
    fn_804903F8(&palQuant);
    return TRUE;
}

/* Gives a raster's mip level images one palette: the first image's when every level already has
 * the same one, else one quantized from all their colors, into which they are palettized. The
 * images replaced are destroyed, except keepImage. */
RwBool fn_8048B860(RwRGBA *palette, RwImage *keepImage, RwImage **images, RwInt32 numImages,
                   RwInt32 depth) {
    RwPalQuant palQuant;
    RwInt32 numColors;
    RwInt32 i;

    if (images[0]->palette != NULL) {
        numColors = 1 << depth;

        for (i = 1; i < numImages; i++) {
            const RwUInt32 *colors = (const RwUInt32 *)images[0]->palette;
            const RwUInt32 *otherColors = (const RwUInt32 *)images[i]->palette;
            RwInt32 j;

            /* (inferred) the original marks a difference with an index past any image count */
            if (colors == NULL || otherColors == NULL) {
                i = 64;
                break;
            }
            for (j = 0; j < numColors; j++) {
                if (colors[j] != otherColors[j]) {
                    i = 64;
                    break;
                }
            }
        }
        if (i == numImages) {
            memcpy(palette, images[0]->palette, (1 << images[0]->depth) * sizeof(RwRGBA));
            return TRUE;
        }
    }

    if (!fn_8048FED8(&palQuant)) {
        return FALSE;
    }
    for (i = 0; i < numImages; i++) {
        fn_8048DEB0(&palQuant, images[i], 1.0f);
    }
    fn_8048F5D0(palette, 1 << depth, &palQuant);
    for (i = 0; i < numImages; i++) {
        RwImage *image = images[i];
        RwImage *palettized = fn_80487D08(image->width, image->height, depth);

        if (palettized != NULL) {
            fn_80487E18(palettized);
            fn_8048FAB0(palettized->cpPixels, palettized->stride, palettized->depth, FALSE,
                        &palQuant, image);
            palettized->palette = palette;
            images[i] = palettized;
            if (image != keepImage) {
                fn_80487DB0(image);
            }
        } else {
            return FALSE;
        }
    }
    fn_804903F8(&palQuant);
    return TRUE;
}

/* Reads a texture's image file: the name and mask name, with the extension of the image file
 * found added. A size of 0 takes the size, depth and format of the raster that suits the image;
 * an image of another size is resampled, and palettized again if it was. */
RwImage *fn_8048BA30(const RwChar *name, const RwChar *maskName, RwInt32 rasterType, RwInt32 *width,
                     RwInt32 *height, RwInt32 *depth, RwInt32 *format) {
    RwChar nameBuffer[rwTEXTUREFILENAMELENGTH];
    RwChar maskBuffer[rwTEXTUREFILENAMELENGTH];
    const RwChar *extension;
    RwImage *image;
    RwImage *other; /* the image being replaced, then its replacement */

    TEXTURECOPYSTRING(nameBuffer, name, rwTEXTUREFILENAMELENGTH);
    extension = fn_80488B58(name);
    if (extension != NULL) {
        rwstrcat(nameBuffer, extension);
    }
    maskBuffer[0] = '\0';
    if (maskName != NULL && maskName[0] != '\0') {
        TEXTURECOPYSTRING(maskBuffer, maskName, rwTEXTUREFILENAMELENGTH);
        extension = fn_80488B58(maskName);
        if (extension != NULL) {
            rwstrcat(maskBuffer, extension);
        }
    }

    image = fn_80488B94(nameBuffer, maskBuffer);
    if (image == NULL) {
        return NULL;
    }
    if (*width == 0 || *height == 0) {
        if (fn_80489A90(image, rasterType, width, height, depth, format) == NULL) {
            fn_80487DB0(image);
            RWERROR((E_RW_INVIMAGEFORMAT));
            return NULL;
        }
    }

    if (image->width != *width || image->height != *height) {
        RwInt32 imageDepth;

        other = image;
        imageDepth = other->depth;

        if (imageDepth != 32) {
            /* A 32-bit copy is resampled */
            image = fn_80487D08(image->width, image->height, 32);
            if (image == NULL) {
                fn_80487DB0(other);
                return NULL;
            }
            if (fn_80487E18(image) == NULL) {
                fn_80487DB0(image);
                fn_80487DB0(other);
                return NULL;
            }
            fn_804893B8(image, other);
            fn_80487DB0(other);
        }

        other = fn_80487D08(*width, *height, 32);
        if (other == NULL) {
            fn_80487DB0(image);
            return NULL;
        }
        if (fn_80487E18(other) == NULL) {
            fn_80487DB0(other);
            fn_80487DB0(image);
            return NULL;
        }
        fn_8048A790(other, image);
        fn_80487DB0(image);
        image = other;

        if (imageDepth == 4) {
            fn_8048B774(&image, imageDepth);
        } else if (imageDepth == 8) {
            fn_8048B774(&image, imageDepth);
        }
    }
    return image;
}

/* Reads a texture without mip levels */
RwTexture *fn_8048BD84(const RwChar *name, const RwChar *maskName) {
    RwChar nameBuffer[rwTEXTUREFILENAMELENGTH];
    RwChar maskBuffer[rwTEXTUREFILENAMELENGTH];
    RwRGBA palette[256];
    RwImage *image;
    RwInt32 width;
    RwInt32 height;
    RwInt32 depth;
    RwInt32 format;
    RwTexture *texture;
    RwRaster *raster;

    TEXTURECOPYSTRING(nameBuffer, name, rwTEXTUREFILENAMELENGTH);
    TEXTURECOPYMASKNAME(maskBuffer, maskName);
    fn_8048D0EC(nameBuffer, maskBuffer, 0, rwRASTERTYPETEXTURE);

    width = 0;
    height = 0;
    image =
        fn_8048BA30(nameBuffer, maskBuffer, rwRASTERTYPETEXTURE, &width, &height, &depth, &format);
    if (image == NULL) {
        return NULL;
    }
    raster = fn_80489E74(width, height, depth, format);
    if (raster == NULL) {
        fn_80487DB0(image);
        return NULL;
    }

    if (RwRasterGetFormat(raster) & (rwRASTERFORMATPAL4 | rwRASTERFORMATPAL8)) {
        if (RwRasterGetFormat(raster) & rwRASTERFORMATPAL4) {
            fn_8048B860(palette, NULL, &image, 1, 4);
        } else {
            fn_8048B860(palette, NULL, &image, 1, 8);
        }
        image->palette = palette;
    }
    fn_804894B8(image);
    if (fn_80489A1C(raster, image) == NULL) {
        fn_80489BDC(raster);
        fn_80487DB0(image);
        return NULL;
    }
    fn_80487DB0(image);

    texture = fn_8048CCDC(raster);
    if (texture == NULL) {
        fn_80489BDC(raster);
        return NULL;
    }
    fn_8048CE54(texture, name);
    if (maskName != NULL) {
        fn_8048CF00(texture, maskName);
    } else {
        fn_8048CF00(texture, "");
    }
    return texture;
}

/* Reads a texture with its mip levels: each level from its own image file, unless the raster
 * generates them */
RwTexture *fn_8048C038(const RwChar *name, const RwChar *maskName) {
    RwChar nameBuffer[rwTEXTUREFILENAMELENGTH];
    RwChar maskBuffer[rwTEXTUREFILENAMELENGTH];
    RwImage *images[rwMAXTEXTUREMIPLEVELS];
    RwRGBA palette[256];
    RwInt32 width;
    RwInt32 height;
    RwInt32 depth;
    RwInt32 format;
    RwRaster *raster;
    RwInt32 rasterFormat;
    RwTexture *texture;

    TEXTURECOPYSTRING(nameBuffer, name, rwTEXTUREFILENAMELENGTH);
    TEXTURECOPYMASKNAME(maskBuffer, maskName);
    rasterFormat = rwRASTERTYPETEXTURE;
    if (RWTEXTUREGLOBAL(haveTexMipmaps)) {
        rasterFormat |= rwRASTERFORMATMIPMAP;
        if (RWTEXTUREGLOBAL(generateTexMipmaps)) {
            rasterFormat |= rwRASTERFORMATAUTOMIPMAP;
        }
    }
    fn_8048D0EC(nameBuffer, maskBuffer, 0, rasterFormat);

    width = 0;
    height = 0;
    images[0] = fn_8048BA30(nameBuffer, maskBuffer, rasterFormat, &width, &height, &depth, &format);
    if (images[0] == NULL) {
        return NULL;
    }
    raster = fn_80489E74(width, height, depth, format);
    if (raster == NULL) {
        fn_80487DB0(images[0]);
        return NULL;
    }

    if (format & rwRASTERFORMATMIPMAP) {
        if (format & rwRASTERFORMATAUTOMIPMAP) {
            /* The raster generates its mip levels from the first */
            if (fn_80489A1C(raster, images[0]) == NULL) {
                fn_80489BDC(raster);
                fn_80487DB0(images[0]);
                return NULL;
            }
            fn_80487DB0(images[0]);
        } else {
            RwInt32 level;
            RwInt32 numLevels = fn_80489CE4(raster);

            for (level = 1; level < numLevels; level++) {
                TEXTURECOPYSTRING(nameBuffer, name, rwTEXTUREFILENAMELENGTH);
                TEXTURECOPYMASKNAME(maskBuffer, maskName);
                fn_8048D0EC(nameBuffer, maskBuffer, level, rasterFormat);

                /* Each level's image is read at the level's size */
                fn_80489F70(raster, level, rwRASTERLOCKWRITE | rwRASTERLOCKNOFETCH);
                width = raster->width;
                height = raster->height;
                depth = raster->depth;
                format = (raster->cFormat << 8) | raster->cType;
                fn_80489B40(raster);

                images[level] = fn_8048BA30(nameBuffer, maskBuffer, rasterFormat, &width, &height,
                                            &depth, &format);
                if (images[level] == NULL) {
                    while (--level >= 0) {
                        fn_80487DB0(images[level]);
                    }
                    fn_80489BDC(raster);
                    return NULL;
                }
            }

            if (RwRasterGetFormat(raster) & (rwRASTERFORMATPAL4 | rwRASTERFORMATPAL8)) {
                if (RwRasterGetFormat(raster) & rwRASTERFORMATPAL4) {
                    fn_8048B860(palette, NULL, images, numLevels, 4);
                } else {
                    fn_8048B860(palette, NULL, images, numLevels, 8);
                }
                /* The levels share the first image's palette */
                fn_804894B8(images[0]);
            } else {
                for (level = 0; level < numLevels; level++) {
                    fn_804894B8(images[level]);
                }
            }

            for (level = 0; level < numLevels; level++) {
                if (fn_80489F70(raster, level, rwRASTERLOCKWRITE | rwRASTERLOCKNOFETCH) != NULL) {
                    if (fn_80489A1C(raster, images[level]) == NULL) {
                        for (; level < numLevels; level++) {
                            fn_80487DB0(images[level]);
                        }
                        fn_80489BDC(raster);
                        return NULL;
                    }
                    fn_80489B40(raster);
                }
                fn_80487DB0(images[level]);
            }
        }
    } else {
        fn_804894B8(images[0]);
        if (fn_80489A1C(raster, images[0]) == NULL) {
            fn_80489BDC(raster);
            fn_80487DB0(images[0]);
            return NULL;
        }
        fn_80487DB0(images[0]);
    }

    texture = fn_8048CCDC(raster);
    if (texture == NULL) {
        fn_80489BDC(raster);
        return NULL;
    }
    fn_8048CE54(texture, name);
    if (maskName != NULL) {
        fn_8048CF00(texture, maskName);
    } else {
        fn_8048CF00(texture, "");
    }
    return texture;
}

/* The default texture read: with mip levels when new textures are mipmapped */
RwTexture *fn_8048C5F4(const RwChar *name, const RwChar *maskName) {
    if (RWTEXTUREGLOBAL(haveTexMipmaps)) {
        return fn_8048C038(name, maskName);
    }
    return fn_8048BD84(name, maskName);
}

/* The default mip level generation: each level is resampled from the one before, starting from
 * the image (or the raster's first level), then palettized and gamma corrected as the raster
 * needs */
RwRaster *fn_8048C634(RwRaster *raster, RwImage *image) {
    RwImage *images[rwMAXTEXTUREMIPLEVELS];
    RwRGBA palette[256];
    RwInt32 autoMipmap;
    RwInt32 level;
    RwInt32 numLevels;

    RwInt32 width = raster->width;
    RwInt32 height = raster->height;

    if (image == NULL) {
        images[0] = fn_80487D08(width, height, 32);
        if (images[0] != NULL) {
            if (fn_80487E18(images[0]) == NULL) {
                return NULL;
            }
            fn_804899A8(images[0], raster);
        }
    } else if (image->depth != 32) {
        images[0] = fn_80487D08(width, height, 32);
        if (images[0] != NULL) {
            if (fn_80487E18(images[0]) == NULL) {
                return NULL;
            }
            fn_804893B8(images[0], image);
        }
    } else {
        images[0] = image;
    }
    if (images[0] == NULL) {
        return NULL;
    }

    /* The raster is written level by level, so it must not generate them meanwhile */
    autoMipmap = raster->cFormat & (rwRASTERFORMATAUTOMIPMAP >> 8);
    raster->cFormat &= ~autoMipmap;

    numLevels = fn_80489CE4(raster);
    for (level = 1; level < numLevels; level++) {
        images[level] = NULL;
        if (fn_80489F70(raster, level, rwRASTERLOCKREAD) != NULL) {
            images[level] = fn_8048A96C(images[level - 1], raster->width, raster->height);
            fn_80489B40(raster);
        }
        if (images[level] == NULL) {
            while (--level >= 0) {
                if (images[level] != image) {
                    fn_80487DB0(images[level]);
                }
            }
            raster->cFormat |= autoMipmap;
            return NULL;
        }
    }

    if (RwRasterGetFormat(raster) & (rwRASTERFORMATPAL4 | rwRASTERFORMATPAL8)) {
        if (RwRasterGetFormat(raster) & rwRASTERFORMATPAL4) {
            if (!fn_8048B860(palette, image, images, numLevels, 4)) {
                for (level = 0; level < numLevels; level++) {
                    if (images[0] != image) {
                        fn_80487DB0(images[0]);
                    }
                    raster->cFormat |= autoMipmap;
                    return NULL;
                }
            }
        } else {
            if (!fn_8048B860(palette, image, images, numLevels, 8)) {
                for (level = 0; level < numLevels; level++) {
                    if (images[0] != image) {
                        fn_80487DB0(images[0]);
                    }
                    raster->cFormat |= autoMipmap;
                    return NULL;
                }
            }
        }
        fn_804894B8(images[0]);
    } else {
        for (level = 0; level < numLevels; level++) {
            fn_804894B8(images[level]);
        }
    }

    for (level = 0; level < numLevels; level++) {
        if (fn_80489F70(raster, level, rwRASTERLOCKWRITE | rwRASTERLOCKNOFETCH) != NULL) {
            fn_80489A1C(raster, images[level]);
            fn_80489B40(raster);
        }
        if (images[level] != image) {
            fn_80487DB0(images[level]);
        }
    }
    raster->cFormat |= autoMipmap;
    return raster;
}

/* The default texture find: in the current dictionary, else in every dictionary */
RwTexture *fn_8048C91C(const RwChar *name) {
    rwTextureGlobals *globals =
        RWPLUGINOFFSET(rwTextureGlobals, lbl_805F15B4, lbl_805F15D4.globalsOffset);
    RwLLLink *link;

    if (globals->currentTexDict != NULL) {
        return fn_8048CFFC(globals->currentTexDict, name);
    }
    for (link = globals->texDictList.link.next; link != &globals->texDictList.link;
         link = link->next) {
        RwTexture *texture = fn_8048CFFC(DICTFROMLINK(link), name);

        if (texture != NULL) {
            return texture;
        }
    }
    return NULL;
}

RwBool fn_8048C9A8(RwBool enable) {
    RWTEXTUREGLOBAL(haveTexMipmaps) = enable;
    return TRUE;
}

RwBool fn_8048C9C0(void) {
    return RWTEXTUREGLOBAL(haveTexMipmaps);
}

RwBool fn_8048C9D4(RwBool enable) {
    RWTEXTUREGLOBAL(generateTexMipmaps) = enable;
    return TRUE;
}

RwBool fn_8048C9EC(void) {
    return RWTEXTUREGLOBAL(generateTexMipmaps);
}

RwTexture *fn_8048CA00(RwTexture *texture, RwRaster *raster) {
    if (raster != NULL) {
        if (lbl_805F15B4->stdFunc[8](texture, raster, 0)) {
            return texture;
        }
        return NULL;
    }
    texture->raster = NULL;
    return texture;
}

RwTexDictionary *fn_8048CA64(void) {
    RwTexDictionary *dict;

    dict = (RwTexDictionary *)RwFreeListAlloc(RWTEXTUREGLOBAL(texDictFreeList),
                                              rwMEMHINTDUR_EVENT | rwID_TEXDICTIONARY);
    if (dict == NULL) {
        return NULL;
    }
    dict->object.type = rwTEXDICTIONARY;
    dict->object.subType = 0;
    dict->object.flags = 0;
    dict->object.privateFlags = 0;
    dict->object.parent = NULL;
    rwLinkListAddLLLink(&RWTEXTUREGLOBAL(texDictList), &dict->lInInstance);
    rwLinkListInitialize(&dict->texturesInDict);
    fn_8048209C(&lbl_8056E048, dict);
    return dict;
}

RwBool fn_8048CB38(RwTexDictionary *dict) {
    if (RWTEXTUREGLOBAL(currentTexDict) == dict) {
        RWTEXTUREGLOBAL(currentTexDict) = NULL;
    }
    fn_8048CBE0(dict, (RwTextureCallBack)fn_8048CD8C, NULL);
    fn_80482144(&lbl_8056E048, dict);
    rwLinkListRemoveLLLink(&dict->lInInstance);
    RwFreeListFree(RWTEXTUREGLOBAL(texDictFreeList), dict);
    return TRUE;
}

RwTexDictionary *fn_8048CBE0(RwTexDictionary *dict, RwTextureCallBack callBack, void *data) {
    RwLLLink *next;
    RwLLLink *link;
    RwLLLink *end = &dict->texturesInDict.link;

    for (link = dict->texturesInDict.link.next; link != end; link = next) {
        next = link->next;
        if (callBack(TEXTUREFROMDICTLINK(link), data) == NULL) {
            break;
        }
    }
    return dict;
}

RwBool fn_8048CC54(RwTexDictionary *(*callBack)(RwTexDictionary *dict, void *data), void *data) {
    RwLinkList *list = &RWTEXTUREGLOBAL(texDictList);
    RwLLLink *link = list->link.next;

    while (link != &list->link) {
        RwLLLink *next = link->next;

        if (callBack(DICTFROMLINK(link), data) == NULL) {
            break;
        }
        link = next;
    }
    return TRUE;
}

/* Creates a texture of a raster: unnamed, in no dictionary, wrapping and filtered to the nearest
 * texel */
RwTexture *fn_8048CCDC(RwRaster *raster) {
    RwTexture *texture;

    texture = (RwTexture *)RwFreeListAlloc(RWTEXTUREGLOBAL(textureFreeList),
                                           rwMEMHINTDUR_EVENT | rwID_TEXTURE);
    if (texture != NULL) {
        texture->dict = NULL;
        texture->name[0] = '\0';
        texture->mask[0] = '\0';
        texture->raster = raster;
        texture->refCount = 1;
        texture->filterAddressing = 0;
        texture->filterAddressing = (texture->filterAddressing & ~rwTEXTUREADDRESSINGMASK) |
                                    (rwTEXTUREADDRESSWRAP << 8) | (rwTEXTUREADDRESSWRAP << 12);
        texture->filterAddressing =
            (texture->filterAddressing & ~rwTEXTUREFILTERMODEMASK) | rwFILTERNEAREST;
        fn_8048209C(&lbl_8056E030, texture);
    }
    return texture;
}

/* Releases a reference to a texture; the last one destroys it and its raster */
RwBool fn_8048CD8C(RwTexture *texture) {
    texture->refCount--;
    if (texture->refCount <= 0) {
        /* The plugins see the texture referenced while they are deinitialized */
        texture->refCount++;
        fn_80482144(&lbl_8056E030, texture);
        if (texture->dict != NULL) {
            rwLinkListRemoveLLLink(&texture->lInDictionary);
        }
        if (texture->raster != NULL) {
            fn_80489BDC(texture->raster);
            texture->raster = NULL;
        }
        texture->refCount--;
        RwFreeListFree(RWTEXTUREGLOBAL(textureFreeList), texture);
    }
    return TRUE;
}

RwTexture *fn_8048CE54(RwTexture *texture, const RwChar *name) {
    TEXTURECOPYSTRING(texture->name, name, rwTEXTUREBASENAMELENGTH);
    return texture;
}

RwTexture *fn_8048CF00(RwTexture *texture, const RwChar *maskName) {
    TEXTURECOPYSTRING(texture->mask, maskName, rwTEXTUREBASENAMELENGTH);
    return texture;
}

/* Moves a texture into a dictionary */
RwTexture *fn_8048CFAC(RwTexDictionary *dict, RwTexture *texture) {
    if (texture->dict != NULL) {
        rwLinkListRemoveLLLink(&texture->lInDictionary);
    }
    texture->dict = dict;
    rwLinkListAddLLLink(&dict->texturesInDict, &texture->lInDictionary);
    return texture;
}

/* Whether two texture names are the same, ignoring case */
static inline RwBool TextureNamesMatch(const RwChar *name1, const RwChar *name2) {
    while (*name1 != '\0' && *name2 != '\0') {
        RwChar char1 = *name1;
        RwChar char2 = *name2;

        if (char1 >= 'a' && char1 <= 'z') {
            char1 -= 'a' - 'A';
        }
        if (char2 >= 'a' && char2 <= 'z') {
            char2 -= 'a' - 'A';
        }
        if (char1 != char2) {
            return FALSE;
        }
        name1++;
        name2++;
    }
    if (*name1 == *name2) {
        return TRUE;
    }
    return FALSE;
}

RwTexture *fn_8048CFFC(RwTexDictionary *dict, const RwChar *name) {
    RwLLLink *link = dict->texturesInDict.link.next;
    RwLLLink *end = &dict->texturesInDict.link;

    for (; link != end; link = link->next) {
        RwTexture *texture = TEXTUREFROMDICTLINK(link);

        if (texture->name != NULL && TextureNamesMatch(texture->name, name)) {
            return texture;
        }
    }
    return NULL;
}

RwTexDictionary *fn_8048D0C4(RwTexDictionary *dict) {
    RWTEXTUREGLOBAL(currentTexDict) = dict;
    return dict;
}

RwTexDictionary *fn_8048D0D8(void) {
    return RWTEXTUREGLOBAL(currentTexDict);
}

RwBool fn_8048D0EC(RwChar *name, RwChar *maskName, RwUInt8 mipLevel, RwInt32 format) {
    RwTextureCallBackMipmapName callBack = RWTEXTUREGLOBAL(mipmapNameCallBack);

    if (callBack != NULL) {
        return callBack(name, maskName, mipLevel, format);
    }
    return FALSE;
}

/* Reads a texture: one already found gains a reference, else it is read and joins the current
 * dictionary */
RwTexture *fn_8048D130(const RwChar *name, const RwChar *maskName) {
    RwTexture *texture = RWTEXTUREGLOBAL(textureFindCallBack)(name);

    if (texture != NULL) {
        texture->refCount++;
        return texture;
    }
    texture = RWTEXTUREGLOBAL(textureReadCallBack)(name, maskName);
    if (texture == NULL) {
        if (maskName != NULL) {
            RWERROR((E_RW_READTEXMASK, name, maskName));
        } else {
            RWERROR((E_RW_READTEXMASK, name, "(null)"));
        }
        return NULL;
    }
    if (RWTEXTUREGLOBAL(currentTexDict) != NULL) {
        fn_8048CFAC(RWTEXTUREGLOBAL(currentTexDict), texture);
    }
    return texture;
}

RwInt32 fn_8048D280(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB,
                    RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB) {
    return fn_80481DC8(&lbl_8056E030, size, pluginID, constructCB, destructCB, copyCB);
}

RwBool fn_8048D2C0(RwRaster *raster, RwImage *image) {
    return RWTEXTUREGLOBAL(mipmapGenerationCallBack)(raster, image) != NULL;
}

/* (The free list block counts follow the unit's string literals in .sdata) */
RwInt32 lbl_805EDF0C = 128; /* textures per free list block */
RwInt32 lbl_805EDF10 = 1;   /* texture free list blocks to preallocate */
RwInt32 lbl_805EDF14 = 5;   /* texture dictionaries per free list block */
RwInt32 lbl_805EDF18 = 1;   /* texture dictionary free list blocks to preallocate */

/* Closes the texture module: the buffer at 0x24, the dictionary created on opening, and the free
 * lists */
void *fn_8048D300(void *instance, RwInt32 offset, RwInt32 size) {
    if (RWTEXTUREGLOBAL(field_0x24) != NULL) {
        RwFree(RWTEXTUREGLOBAL(field_0x24));
        RWTEXTUREGLOBAL(field_0x24) = NULL;
        RWTEXTUREGLOBAL(field_0x28) = 0;
    }
    if (RWTEXTUREGLOBAL(textureFreeList) != NULL && RWTEXTUREGLOBAL(texDictFreeList) != NULL) {
        RwLinkList *list = &RWTEXTUREGLOBAL(texDictList);
        RwLLLink *link = list->link.next;

        while (link != &list->link) {
            RwTexDictionary *dict = DICTFROMLINK(link);

            link = link->next;
            if (dict == lbl_805F15D0) {
                fn_8048CB38(lbl_805F15D0);
                lbl_805F15D0 = NULL;
                break;
            }
        }
    }
    if (RWTEXTUREGLOBAL(textureFreeList) != NULL) {
        fn_8047FCDC(RWTEXTUREGLOBAL(textureFreeList));
        RWTEXTUREGLOBAL(textureFreeList) = NULL;
    }
    if (RWTEXTUREGLOBAL(texDictFreeList) != NULL) {
        fn_8047FCDC(RWTEXTUREGLOBAL(texDictFreeList));
        RWTEXTUREGLOBAL(texDictFreeList) = NULL;
    }
    lbl_805F15D4.numInstances--;
    return instance;
}

/* Opens the texture module: its free lists, a dictionary that becomes the current one, and the
 * default read, find, mip level generation and mip level name callbacks */
void *fn_8048D4DC(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F15D4.globalsOffset = offset;
    RWTEXTUREGLOBAL(textureFreeList) =
        fn_8047FC04(lbl_8056E030.sizeOfStruct, lbl_805EDF0C, 4, lbl_805EDF10, &lbl_805E2D10,
                    rwMEMHINTDUR_GLOBAL | rwID_TEXTURE);
    if (RWTEXTUREGLOBAL(textureFreeList) == NULL) {
        return NULL;
    }
    RWTEXTUREGLOBAL(texDictFreeList) =
        fn_8047FC04(lbl_8056E048.sizeOfStruct, lbl_805EDF14, 4, lbl_805EDF18, &lbl_805E2D34,
                    rwMEMHINTDUR_GLOBAL | rwID_TEXTUREMODULE);
    if (RWTEXTUREGLOBAL(texDictFreeList) == NULL) {
        fn_8047FCDC(RWTEXTUREGLOBAL(textureFreeList));
        RWTEXTUREGLOBAL(textureFreeList) = NULL;
        return NULL;
    }

    rwLinkListInitialize(&RWTEXTUREGLOBAL(texDictList));
    lbl_805F15D4.numInstances++;

    lbl_805F15D0 = fn_8048CA64();
    RWTEXTUREGLOBAL(currentTexDict) = lbl_805F15D0;
    if (RWTEXTUREGLOBAL(currentTexDict) == NULL) {
        fn_8047FCDC(RWTEXTUREGLOBAL(texDictFreeList));
        RWTEXTUREGLOBAL(texDictFreeList) = NULL;
        fn_8047FCDC(RWTEXTUREGLOBAL(textureFreeList));
        RWTEXTUREGLOBAL(textureFreeList) = NULL;
        return NULL;
    }

    RWTEXTUREGLOBAL(haveTexMipmaps) = FALSE;
    RWTEXTUREGLOBAL(generateTexMipmaps) = FALSE;
    RWTEXTUREGLOBAL(textureFindCallBack) = fn_8048C91C;
    RWTEXTUREGLOBAL(textureReadCallBack) = fn_8048C5F4;
    RWTEXTUREGLOBAL(mipmapGenerationCallBack) = fn_8048C634;
    RWTEXTUREGLOBAL(mipmapNameCallBack) = fn_8048B6AC;
    RWTEXTUREGLOBAL(field_0x24) = NULL;
    RWTEXTUREGLOBAL(field_0x28) = 0;
    return instance;
}
