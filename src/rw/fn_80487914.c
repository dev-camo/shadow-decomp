/* RenderWare images: the image module, creating and destroying images and their pixels, masks,
 * copying and converting pixels, registering image file formats, reading images along the image
 * path, gamma correction and the image and raster standard functions. Original file name unknown. */
#include "rw/rwcore.h"
#include <math.h>
#include <string.h>
#include "layout_check.h"

/* raster private flags */
#define rwRASTERGAMMACORRECTED 0x1

/* errors (inferred) */
#define E_RW_INVIMAGEDEPTH 0x80000008  /* the image depth is not supported */
#define E_RW_INVIMAGEFORMAT 0x80000009 /* the depths cannot be converted */
#define E_RW_INVIMAGESIZE 0x8000000A   /* the image and mask sizes differ */

/* Reports an error of the core; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwID_COREPLUGIN;                                                         \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

/* The longest image file extension, with its terminator */
#define rwIMAGEEXTENSIONLENGTH 20

/* An image file format: its extensions and how its files are read and written (0x34 bytes) */
typedef struct rwImageFormat {
    /* 0x00 */ RwChar lowerCaseExtension[rwIMAGEEXTENSIONLENGTH];
    /* 0x14 */ RwChar upperCaseExtension[rwIMAGEEXTENSIONLENGTH];
    /* 0x28 */ RwImage *(*imageRead)(const RwChar *imageName);
    /* 0x2C */ RwImage *(*imageWrite)(RwImage *image, const RwChar *imageName); /* (inferred) */
    /* 0x30 */ struct rwImageFormat *nextFormat;
} rwImageFormat;
CHECK_SIZEOF(rwImageFormat, 0x34);

/* The engine globals of the image module (0x220 bytes) */
typedef struct rwImageGlobals {
    /* 0x000 */ RwFreeList *imageFreeList;
    /* 0x004 */ RwChar *imagePath; /* the directories images are looked for in, ';' separated */
    /* 0x008 */ RwInt32 imagePathSize;
    /* 0x00C */ RwUInt8 gammaTable[256];
    /* 0x10C */ RwUInt8 invGammaTable[256];
    /* 0x20C */ RwReal gammaVal;
    /* 0x210 */ RwChar *scratchMem; /* where file paths are built */
    /* 0x214 */ RwInt32 scratchMemSize;
    /* 0x218 */ RwFreeList *imageFormatFreeList;
    /* 0x21C */ rwImageFormat *imageFormats;
} rwImageGlobals;
CHECK_SIZEOF(rwImageGlobals, 0x220);

#define RWIMAGEGLOBAL(var)                                                                         \
    (RWPLUGINOFFSET(rwImageGlobals, lbl_805F15B4, lbl_805F15C0.globalsOffset)->var)

/* A function called with each file path tried for a file name; it returns NULL to stop */
typedef RwChar *(*ImagePathCallBack)(RwChar *path, void *data);

/* What reading an image along the image path uses and finds */
typedef struct ImageReadData {
    RwImage *(*imageRead)(const RwChar *imageName);
    RwImage *image;
} ImageReadData;
CHECK_SIZEOF(ImageReadData, 0x8);

/* The image plugin registry */
RwPluginRegistry lbl_8056E000 = {sizeof(RwImage), sizeof(RwImage), 0, 0, NULL, NULL};

RwFreeList lbl_805E2CA0; /* the image free list */
RwFreeList lbl_805E2CC4; /* the image format free list */

RwInt32 lbl_805EDEE8 = 128; /* images, and image formats, per free list block */
RwInt32 lbl_805EDEEC = 1;   /* image free list blocks to preallocate */
RwInt32 lbl_805EDEF0 = 1;   /* image format free list blocks to preallocate */

RwModuleInfo lbl_805F15C0; /* the image module */

/* Opens the image module: its free lists, the empty image path, a gamma of 1 and the scratch
 * memory paths are built in */
void *fn_80487914(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F15C0.globalsOffset = offset;
    RWIMAGEGLOBAL(imageFreeList) =
        fn_8047FC04(lbl_8056E000.sizeOfStruct, lbl_805EDEE8, 4, lbl_805EDEEC, &lbl_805E2CA0,
                    rwMEMHINTDUR_GLOBAL | rwID_IMAGE);
    if (RWIMAGEGLOBAL(imageFreeList) == NULL) {
        return NULL;
    }

    RWIMAGEGLOBAL(imageFormatFreeList) =
        fn_8047FC04(sizeof(rwImageFormat), lbl_805EDEE8, 4, lbl_805EDEF0, &lbl_805E2CC4,
                    rwMEMHINTDUR_GLOBAL | rwID_IMAGEMODULE);
    if (RWIMAGEGLOBAL(imageFormatFreeList) == NULL) {
        fn_8047FCDC(RWIMAGEGLOBAL(imageFreeList));
        RWIMAGEGLOBAL(imageFreeList) = NULL;
        return NULL;
    }

    RWIMAGEGLOBAL(imagePathSize) = 256;
    RWIMAGEGLOBAL(imagePath) =
        (RwChar *)RwMalloc(RWIMAGEGLOBAL(imagePathSize),
                           rwMEMHINTFLAG_RESIZABLE | rwMEMHINTDUR_GLOBAL | rwID_IMAGEMODULE);
    if (RWIMAGEGLOBAL(imagePath) == NULL) {
        fn_8047FCDC(RWIMAGEGLOBAL(imageFormatFreeList));
        RWIMAGEGLOBAL(imageFormatFreeList) = NULL;
        fn_8047FCDC(RWIMAGEGLOBAL(imageFreeList));
        RWIMAGEGLOBAL(imageFreeList) = NULL;
        return NULL;
    }
    RWIMAGEGLOBAL(imagePath)[0] = '\0';

    lbl_805F15C0.numInstances++;
    fn_80489830(1.0f);
    RWIMAGEGLOBAL(imageFormats) = NULL;

    RWIMAGEGLOBAL(scratchMemSize) = 256;
    RWIMAGEGLOBAL(scratchMem) = (RwChar *)RwMalloc(
        RWIMAGEGLOBAL(scratchMemSize), rwMEMHINTFLAG_RESIZABLE | rwMEMHINTDUR_GLOBAL | rwID_IMAGE);
    if (RWIMAGEGLOBAL(scratchMem) == NULL) {
        RwFree(RWIMAGEGLOBAL(imagePath));
        RWIMAGEGLOBAL(imagePath) = NULL;
        RWIMAGEGLOBAL(imagePathSize) = 0;
        fn_8047FCDC(RWIMAGEGLOBAL(imageFormatFreeList));
        RWIMAGEGLOBAL(imageFormatFreeList) = NULL;
        fn_8047FCDC(RWIMAGEGLOBAL(imageFreeList));
        RWIMAGEGLOBAL(imageFreeList) = NULL;
        return NULL;
    }
    return instance;
}

/* Closes the image module: the scratch memory, the image path, the registered formats and the
 * free lists */
void *fn_80487BB0(void *instance, RwInt32 offset, RwInt32 size) {
    if (RWIMAGEGLOBAL(scratchMem) != NULL) {
        RwFree(RWIMAGEGLOBAL(scratchMem));
        RWIMAGEGLOBAL(scratchMem) = NULL;
        RWIMAGEGLOBAL(scratchMemSize) = 0;
    }
    if (RWIMAGEGLOBAL(imagePath) != NULL) {
        RwFree(RWIMAGEGLOBAL(imagePath));
        RWIMAGEGLOBAL(imagePath) = NULL;
        RWIMAGEGLOBAL(imagePathSize) = 0;
    }

    while (RWIMAGEGLOBAL(imageFormats) != NULL) {
        rwImageFormat *format = RWIMAGEGLOBAL(imageFormats);

        RWIMAGEGLOBAL(imageFormats) = format->nextFormat;
        RwFreeListFree(RWIMAGEGLOBAL(imageFormatFreeList), format);
    }

    if (RWIMAGEGLOBAL(imageFormatFreeList) != NULL) {
        fn_8047FCDC(RWIMAGEGLOBAL(imageFormatFreeList));
        RWIMAGEGLOBAL(imageFormatFreeList) = NULL;
    }
    if (RWIMAGEGLOBAL(imageFreeList) != NULL) {
        fn_8047FCDC(RWIMAGEGLOBAL(imageFreeList));
        RWIMAGEGLOBAL(imageFreeList) = NULL;
    }
    lbl_805F15C0.numInstances--;
    return instance;
}

/* Creates an image without pixels */
RwImage *fn_80487D08(RwInt32 width, RwInt32 height, RwInt32 depth) {
    RwImage *image;

    image =
        (RwImage *)RwFreeListAlloc(RWIMAGEGLOBAL(imageFreeList), rwMEMHINTDUR_EVENT | rwID_IMAGE);
    if (image == NULL) {
        return NULL;
    }
    image->width = width;
    image->height = height;
    image->depth = depth;
    image->cpPixels = NULL;
    image->palette = NULL;
    image->flags = 0;
    fn_8048209C(&lbl_8056E000, image);
    return image;
}

RwBool fn_80487DB0(RwImage *image) {
    if (image->flags & rwIMAGEALLOCATED) {
        fn_80487F50(image);
    }
    fn_80482144(&lbl_8056E000, image);
    RwFreeListFree(RWIMAGEGLOBAL(imageFreeList), image);
    return TRUE;
}

/* Allocates an image's pixels, with rows padded to 4 bytes, and a palette after them for a 4 or
 * 8-bit image */
RwImage *fn_80487E18(RwImage *image) {
    /* The original compares the depth unsigned */
    RwUInt32 depth = image->depth;
    RwBool palettized = FALSE;
    RwInt32 paletteSize;
    RwInt32 pixelSize;
    RwInt32 size;

    if (depth == 4 || depth == 8) {
        palettized = TRUE;
    }
    paletteSize = palettized ? (1 << image->depth) * (RwInt32)sizeof(RwRGBA) : 0;

    image->stride = (image->depth + 7) >> 3;
    image->stride *= image->width;
    image->stride = (image->stride + 3) & ~3;
    pixelSize = image->stride * image->height;
    size = pixelSize + paletteSize;

    image->cpPixels = (RwUInt8 *)RwMalloc(size, rwMEMHINTDUR_EVENT | rwID_IMAGE);
    if (image->cpPixels == NULL) {
        RWERROR((E_RW_NOMEM, size));
        return NULL;
    }
    image->palette = palettized ? (RwRGBA *)(image->cpPixels + pixelSize) : NULL;
    image->flags |= rwIMAGEALLOCATED;
    return image;
}

RwImage *fn_80487F50(RwImage *image) {
    RwFree(image->cpPixels);
    image->cpPixels = NULL;
    image->palette = NULL;
    image->flags &= ~rwIMAGEALLOCATED;
    return image;
}

RwImage *fn_80487FA8(RwImage *image) {
    switch (image->depth) {
    case 4:
    case 8: {
        RwInt32 numColors = 1 << image->depth;
        RwRGBA *palette = image->palette;
        RwInt32 i;

        for (i = 0; i < numColors; i++) {
            RwRGBA *color = &palette[i];
            RwInt32 brightest = color->red;

            if (color->green > brightest) {
                brightest = color->green;
            }
            if (color->blue > brightest) {
                brightest = color->blue;
            }
            color->alpha = (RwUInt8)brightest;
        }
        break;
    }
    case 32: {
        RwInt32 y;
        RwUInt8 *row = image->cpPixels;

        for (y = 0; y < image->height; y++) {
            RwRGBA *pixels = (RwRGBA *)row;
            RwInt32 x;

            for (x = 0; x < image->width; x++) {
                RwRGBA *pixel = &pixels[x];
                RwInt32 brightest = pixel->red;

                if (pixel->green > brightest) {
                    brightest = pixel->green;
                }
                if (pixel->blue > brightest) {
                    brightest = pixel->blue;
                }
                pixel->alpha = (RwUInt8)brightest;
            }
            row += image->stride;
        }
        break;
    }
    }
    return image;
}

/* Sets an image's alpha from a mask of the same size. A 4 or 8-bit image becomes 32-bit first. */
RwImage *fn_80488088(RwImage *image, const RwImage *mask) {
    if (image->width != mask->width || image->height != mask->height) {
        RWERROR((E_RW_INVIMAGESIZE));
        return NULL;
    }

    switch (image->depth) {
    case 4:
    case 8: {
        RwImage *copy = fn_80487D08(image->width, image->height, image->depth);

        if (copy == NULL) {
            return NULL;
        }
        if (fn_80487E18(copy) == NULL) {
            fn_80487DB0(copy);
            return NULL;
        }
        fn_804893B8(copy, image);
        if (image->flags & rwIMAGEALLOCATED) {
            fn_80487F50(image);
        }
        image->depth = 32;
        fn_80487E18(image);
        fn_804893B8(image, copy);
        fn_80487F50(copy);
        fn_80487DB0(copy);
    }
        /* fall through: the image is now 32-bit */
    case 32: {
        RwInt32 y;
        RwInt32 x;
        const RwUInt8 *maskRow = mask->cpPixels;
        const RwRGBA *maskPalette = mask->palette;
        RwUInt8 *row = image->cpPixels;

        for (y = 0; y < image->height; y++) {
            RwRGBA *pixel = (RwRGBA *)row;

            switch (mask->depth) {
            case 4:
            case 8: {
                const RwUInt8 *maskIndex = maskRow;

                for (x = 0; x < image->width; x++) {
                    pixel->alpha = maskPalette[*maskIndex].alpha;
                    maskIndex++;
                    pixel++;
                }
                break;
            }
            case 32: {
                const RwRGBA *maskPixel = (const RwRGBA *)maskRow;

                for (x = 0; x < image->width; x++) {
                    pixel->alpha = maskPixel->alpha;
                    maskPixel++;
                    pixel++;
                }
                break;
            }
            }
            maskRow += mask->stride;
            row += image->stride;
        }
        break;
    }
    default:
        RWERROR((E_RW_INVIMAGEFORMAT));
        return NULL;
    }
    return image;
}

/* Sets the image path: directories separated by ';' */
const RwChar *fn_804885DC(const RwChar *path) {
    RwInt32 size = rwstrlen(path) + 1;

    if (size > RWIMAGEGLOBAL(imagePathSize)) {
        RwChar *newPath =
            (RwChar *)RwRealloc(RWIMAGEGLOBAL(imagePath), size,
                                rwMEMHINTFLAG_RESIZABLE | rwMEMHINTDUR_GLOBAL | rwID_IMAGEMODULE);

        if (newPath == NULL) {
            RWERROR((E_RW_NOMEM, size));
            return NULL;
        }
        RWIMAGEGLOBAL(imagePath) = newPath;
        RWIMAGEGLOBAL(imagePathSize) = size;
    }
    memcpy(RWIMAGEGLOBAL(imagePath), path, size);
    return path;
}

/* The scratch memory, grown to at least size bytes (inferred helper) */
/* Sets result to the scratch memory, grown to at least size bytes, or NULL when it cannot grow.
 * (inferred: written as a macro, since the original's path function is not inlined into its
 * callers as a call to a helper would let it be) */
#define IMAGESCRATCH(result, size)                                                                 \
    do {                                                                                           \
        RwInt32 size_ = (size);                                                                    \
                                                                                                   \
        if (size_ > RWIMAGEGLOBAL(scratchMemSize)) {                                               \
            RwChar *mem_;                                                                          \
                                                                                                   \
            if (RWIMAGEGLOBAL(scratchMem) != NULL) {                                               \
                mem_ = (RwChar *)RwRealloc(RWIMAGEGLOBAL(scratchMem), size_,                       \
                                           rwMEMHINTFLAG_RESIZABLE | rwMEMHINTDUR_GLOBAL |         \
                                               rwID_IMAGE);                                        \
            } else {                                                                               \
                mem_ = (RwChar *)RwMalloc(size_, rwMEMHINTFLAG_RESIZABLE | rwMEMHINTDUR_GLOBAL |   \
                                                     rwID_IMAGE);                                  \
            }                                                                                      \
            if (mem_ == NULL) {                                                                    \
                RWERROR((E_RW_NOMEM, size_));                                                      \
                (result) = NULL;                                                                   \
                break;                                                                             \
            }                                                                                      \
            RWIMAGEGLOBAL(scratchMem) = mem_;                                                      \
            RWIMAGEGLOBAL(scratchMemSize) = size_;                                                 \
        }                                                                                          \
        (result) = RWIMAGEGLOBAL(scratchMem);                                                      \
    } while (0)

/* Calls a function with the paths of a file name: the name itself when it is absolute or there is
 * no image path, else the name in each directory of the image path until the function returns
 * NULL. extraLength is room left after the name. Returns NULL when out of memory, else the name. */
const RwChar *fn_804886CC(const RwChar *name, RwInt32 extraLength, ImagePathCallBack callBack,
                          void *data) {
    const RwChar *directory = RWIMAGEGLOBAL(imagePath);

    if (fn_80483444(name) || directory == NULL || directory[0] == '\0') {
        RwChar *path;

        IMAGESCRATCH(path, rwstrlen(name) + extraLength);

        if (path == NULL) {
            return NULL;
        }
        rwstrcpy(path, name);
        callBack(path, data);
    } else {
        while (directory != NULL && directory[0] != '\0') {
            const RwChar *next;
            RwInt32 directoryLength;
            RwChar *path;

            /* The original looks for the separator twice */
            rwstrchr(directory, ';');
            next = rwstrchr(directory, ';');
            if (next != NULL) {
                directoryLength = next - directory;
                next++;
            } else {
                directoryLength = rwstrlen(directory);
            }

            IMAGESCRATCH(path, directoryLength + (rwstrlen(name) + extraLength));
            if (path == NULL) {
                return NULL;
            }
            memcpy(path, directory, directoryLength);
            rwstrcpy(path + directoryLength, name);
            if (callBack(path, data) == NULL) {
                return name;
            }
            directory = next;
        }
    }
    return name;
}

/* Reads the image at a path that exists; stops when it is read */
RwChar *fn_80488A04(RwChar *path, void *data) {
    ImageReadData *readData = (ImageReadData *)data;

    if (fn_8047A67C(path)) {
        readData->image = readData->imageRead(path);
        if (readData->image != NULL) {
            return NULL;
        }
    }
    return path;
}

/* Looks for a file at a path with each registered extension; stops at the first that exists and
 * returns that extension through data */
RwChar *fn_80488A70(RwChar *path, void *data) {
    rwImageFormat *format;
    const RwChar **extension = (const RwChar **)data;
    RwChar *pathEnd = path + rwstrlen(path);

    for (format = RWIMAGEGLOBAL(imageFormats); format != NULL; format = format->nextFormat) {
        rwstrcpy(pathEnd, format->lowerCaseExtension);
        if (fn_8047A67C(path)) {
            *extension = format->lowerCaseExtension;
            return NULL;
        }
        rwstrcpy(pathEnd, format->upperCaseExtension);
        if (fn_8047A67C(path)) {
            *extension = format->upperCaseExtension;
            return NULL;
        }
    }
    return path;
}

const RwChar *fn_80488B58(const RwChar *name) {
    const RwChar *extension = NULL;

    fn_804886CC(name, rwIMAGEEXTENSIONLENGTH, fn_80488A70, &extension);
    return extension;
}

/* Registers the functions that read and write image files with an extension, adding the format
 * if the extension is new; a NULL function keeps the one already registered (name and callback
 * types inferred). Retail strips it, and nothing else adds to imageFormats, so retail registers
 * no format and reading an image by extension finds none. The E3 2005 prototype (G9SJ8P) has it
 * at 0x803EC7E4, where its initialisation code registers readers for "bmp" and "png". */
RwBool ImageRegisterFormat(const RwChar *extension, RwImage *(*imageRead)(const RwChar *imageName),
                           RwImage *(*imageWrite)(RwImage *image, const RwChar *imageName)) {
    rwImageFormat *format;
    rwImageFormat *current;
    RwChar lowerCaseExtension[rwIMAGEEXTENSIONLENGTH];

    lowerCaseExtension[0] = '.';
    rwstrncpy(&lowerCaseExtension[1], extension, rwIMAGEEXTENSIONLENGTH - 2);
    lowerCaseExtension[rwIMAGEEXTENSIONLENGTH - 1] = '\0';
    rwstrlwr(lowerCaseExtension);

    format = NULL;
    current = RWIMAGEGLOBAL(imageFormats);
    while (current != NULL && format == NULL) {
        if (rwstrcmp(current->lowerCaseExtension, lowerCaseExtension) == 0) {
            format = current;
        } else {
            current = current->nextFormat;
        }
    }

    if (format == NULL) {
        format = (rwImageFormat *)RwFreeListAlloc(RWIMAGEGLOBAL(imageFormatFreeList),
                                                  rwMEMHINTDUR_EVENT | rwID_IMAGEMODULE);
        if (format == NULL) {
            return FALSE;
        }
        memcpy(format->lowerCaseExtension, lowerCaseExtension, rwIMAGEEXTENSIONLENGTH);
        format->upperCaseExtension[0] = '.';
        rwstrncpy(&format->upperCaseExtension[1], extension, rwIMAGEEXTENSIONLENGTH - 2);
        format->upperCaseExtension[rwIMAGEEXTENSIONLENGTH - 1] = '\0';
        rwstrupr(format->upperCaseExtension);
        format->imageRead = NULL;
        format->imageWrite = NULL;
        format->nextFormat = RWIMAGEGLOBAL(imageFormats);
        RWIMAGEGLOBAL(imageFormats) = format;
    }

    if (imageRead != NULL) {
        format->imageRead = imageRead;
    }
    if (imageWrite != NULL) {
        format->imageWrite = imageWrite;
    }
    return TRUE;
}

/* Reads an image with the format registered for its extension, along the image path (inferred
 * helper; the original's copy is not in the game, nor in the E3 prototype) */
static RwImage *ImageRead(const RwChar *imageName) {
    const RwChar *searchStart;
    const RwChar *separator;
    const RwChar *extension;

    /* The extension is looked for after the last drive or directory separator */
    searchStart = imageName;

    separator = rwstrrchr(searchStart, ':');
    searchStart = (separator != NULL) ? separator : searchStart;

    separator = rwstrrchr(searchStart, '/');
    searchStart = (separator != NULL) ? separator : searchStart;

    separator = rwstrrchr(searchStart, '\\');
    searchStart = (separator != NULL) ? separator : searchStart;

    extension = rwstrrchr(searchStart, '.');
    if (extension != NULL) {
        rwImageFormat *format = RWIMAGEGLOBAL(imageFormats);

        while (format != NULL) {
            if (rwstrcmp(format->lowerCaseExtension, extension) == 0 ||
                rwstrcmp(format->upperCaseExtension, extension) == 0) {
                if (format->imageRead != NULL) {
                    ImageReadData readData;

                    readData.imageRead = format->imageRead;
                    readData.image = NULL;
                    fn_804886CC(imageName, 5, fn_80488A04, &readData);
                    return readData.image;
                }
                return NULL;
            }
            format = format->nextFormat;
        }
        /* No format is registered for the extension */
        return NULL;
    }
    return NULL;
}

/* Reads an image and, when a mask is named, sets its alpha from the mask's brightest components */
RwImage *fn_80488B94(const RwChar *imageName, const RwChar *maskName) {
    RwImage *image = ImageRead(imageName);

    if (image != NULL) {
        if (maskName != NULL && maskName[0] != '\0') {
            RwImage *mask = ImageRead(maskName);

            if (mask == NULL) {
                fn_80487DB0(image);
                return NULL;
            }
            if (fn_80487FA8(mask) == NULL) {
                fn_80487DB0(image);
                fn_80487DB0(mask);
                return NULL;
            }
            if (fn_80488088(image, mask) == NULL) {
                fn_80487DB0(image);
                fn_80487DB0(mask);
                return NULL;
            }
            fn_80487DB0(mask);
        }
        return image;
    }
    return NULL;
}

RwRGBA *fn_80489118(RwRGBA *rgbOut, RwUInt32 pixelValue, RwInt32 rasterFormat) {
    lbl_805F15B4->stdFunc[rwSTANDARDPIXELTORGB](rgbOut, &pixelValue, rasterFormat);
    return rgbOut;
}

/* Copies an image's pixels to an image of another depth: 4-bit indices to 8-bit, or a 4 or 8-bit
 * image's colors to 32-bit */
RwBool fn_8048915C(RwImage *destImage, const RwImage *sourceImage) {
    RwBool result;
    RwInt32 y;
    RwInt32 x;
    RwInt32 width;
    RwInt32 height;
    const RwRGBA *palette;
    RwUInt8 *destRow;
    const RwUInt8 *sourceRow;

    result = FALSE;
    width = destImage->width;
    height = destImage->height;
    palette = sourceImage->palette;
    sourceRow = sourceImage->cpPixels;
    destRow = destImage->cpPixels;

    switch ((sourceImage->depth << 8) | destImage->depth) {
    case (4 << 8) | 4:
    case (8 << 8) | 8:
    case (32 << 8) | 32:
        result = TRUE;
        break;
    case (4 << 8) | 8: {
        for (y = 0; y < height; y++) {
            memcpy(destRow, sourceRow, width);
            sourceRow += sourceImage->stride;
            destRow += destImage->stride;
        }
        result = TRUE;
        break;
    }
    case (4 << 8) | 32:
    case (8 << 8) | 32: {
        for (y = 0; y < height; y++) {
            RwRGBA *destPixels = (RwRGBA *)destRow;

            for (x = 0; x < width; x++) {
                destPixels[x] = palette[sourceRow[x]];
            }
            sourceRow += sourceImage->stride;
            destRow += destImage->stride;
        }
        result = TRUE;
        break;
    }
    case (8 << 8) | 4:
    case (32 << 8) | 4:
    case (32 << 8) | 8:
    default:
        RWERROR((E_RW_INVIMAGEFORMAT));
        break;
    }
    return result;
}

/* Copies the palette and pixels between images of the same depth; it cannot fail
 * (inferred helper) */
static inline RwBool ImageCopySameDepth(RwImage *destImage, const RwImage *sourceImage) {
    RwInt32 y;
    RwInt32 rowSize;
    const RwUInt8 *sourceRow;
    RwUInt8 *destRow;

    if (destImage->palette != NULL && sourceImage->palette != NULL && sourceImage->depth <= 8) {
        memcpy(destImage->palette, sourceImage->palette,
               (1 << sourceImage->depth) * sizeof(RwRGBA));
    }

    rowSize = ((destImage->depth + 7) >> 3) * destImage->width;
    sourceRow = sourceImage->cpPixels;
    destRow = destImage->cpPixels;
    for (y = 0; y < destImage->height; y++) {
        memcpy(destRow, sourceRow, rowSize);
        sourceRow += sourceImage->stride;
        destRow += destImage->stride;
    }
    return TRUE;
}

/* Copies an image's pixels, palette and gamma correction to an image of the same size */
RwImage *fn_804893B8(RwImage *destImage, const RwImage *sourceImage) {
    if (destImage->depth == sourceImage->depth) {
        if (!ImageCopySameDepth(destImage, sourceImage)) {
            destImage = NULL;
        }
    } else if (!fn_8048915C(destImage, sourceImage)) {
        destImage = NULL;
    }

    destImage->flags &= ~rwIMAGEGAMMACORRECTED;
    destImage->flags |= sourceImage->flags & rwIMAGEGAMMACORRECTED;
    return destImage;
}

/* Gamma corrects colors through the gamma table; alpha is kept (inferred helper) */
static inline void ImageGammaCorrectColors(RwRGBA *dest, const RwRGBA *source, RwInt32 numColors) {
    RwUInt8 *gammaTable = RWIMAGEGLOBAL(gammaTable);

    while (numColors--) {
        dest->red = gammaTable[source->red];
        dest->green = gammaTable[source->green];
        dest->blue = gammaTable[source->blue];
        dest->alpha = source->alpha;
        dest++;
        source++;
    }
}

RwImage *fn_804894B8(RwImage *image) {
    switch (image->depth) {
    case 4:
    case 8: {
        RwInt32 numColors = 1 << image->depth;
        RwRGBA *palette = image->palette;

        if (palette == NULL) {
            RWERROR((E_RW_NULLP));
            return NULL;
        }
        ImageGammaCorrectColors(palette, palette, numColors);
        break;
    }
    case 32: {
        RwUInt8 *row = image->cpPixels;
        RwInt32 width = image->width;
        RwInt32 height = image->height;
        RwInt32 y;

        if (row == NULL) {
            RWERROR((E_RW_NULLP));
            return NULL;
        }
        for (y = 0; y < height; y++) {
            ImageGammaCorrectColors((RwRGBA *)row, (RwRGBA *)row, width);
            row += image->stride;
        }
        break;
    }
    default:
        RWERROR((E_RW_INVIMAGEDEPTH));
        return NULL;
    }
    image->flags |= rwIMAGEGAMMACORRECTED;
    return image;
}

/* Sets the gamma, and the tables that apply it and its inverse to 8-bit components */
RwBool fn_80489830(RwReal gamma) {
    RwReal invGamma = 1.0f / gamma;
    RwInt32 i;

    RWIMAGEGLOBAL(gammaVal) = gamma;
    RWIMAGEGLOBAL(gammaTable)[0] = 0;
    RWIMAGEGLOBAL(invGammaTable)[0] = 0;
    for (i = 1; i < 256; i++) {
        RwReal value = (RwReal)i / 255.0f;
        RwReal corrected;
        RwReal inverse;

        /* Each component is scaled back to 0..255 and rounded */
        corrected = 255.0f * (RwReal)pow(value, invGamma);
        RWIMAGEGLOBAL(gammaTable)[i] = (RwUInt8)(RwInt32)(0.5f + corrected);
        inverse = 255.0f * (RwReal)pow(value, gamma);
        RWIMAGEGLOBAL(invGammaTable)[i] = (RwUInt8)(RwInt32)(0.5f + inverse);
    }
    return TRUE;
}

RwImage *fn_804899A8(RwImage *image, RwRaster *raster) {
    if (lbl_805F15B4->stdFunc[rwSTANDARDIMAGEGETRASTER](image, raster, 0)) {
        if (raster->privateFlags & rwRASTERGAMMACORRECTED) {
            image->flags |= rwIMAGEGAMMACORRECTED;
        }
        return image;
    }
    return NULL;
}

RwRaster *fn_80489A1C(RwRaster *raster, RwImage *image) {
    if (lbl_805F15B4->stdFunc[rwSTANDARDRASTERSETIMAGE](raster, image, 0)) {
        if (image->flags & rwIMAGEGAMMACORRECTED) {
            raster->privateFlags |= rwRASTERGAMMACORRECTED;
        }
        return raster;
    }
    return NULL;
}

RwImage *fn_80489A90(RwImage *image, RwInt32 rasterType, RwInt32 *width, RwInt32 *height,
                     RwInt32 *depth, RwInt32 *format) {
    RwRaster raster;

    if (!lbl_805F15B4->stdFunc[rwSTANDARDIMAGEFINDRASTERFORMAT](&raster, image, rasterType)) {
        return NULL;
    }
    *format = (raster.cFormat << 8) | raster.cType;
    *width = raster.width;
    *height = raster.height;
    *depth = raster.depth;
    return image;
}
