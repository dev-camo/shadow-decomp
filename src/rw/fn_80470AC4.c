/* RenderWare 2D toolkit fonts: showing strings with outline or texture characters, reading fonts
 * from metrics files or streams, and opening and closing the fonts. The original file name is
 * unknown. */
#include "rw/rt2d.h"
#include <string.h>

#define rwMEMHINTDUR_FUNCTION 0x10000
#define rwMEMHINTDUR_EVENT 0x30000
#define rwMEMHINTDUR_GLOBAL 0x40000
#define rwMEMHINTFLAG_RESIZABLE 0x1000000

/* The object IDs of the memory hints (inferred) */
#define RT2DMEMID 0x190
#define RT2DFONTMEMID 0x199

/* The chunk ID of a path */
#define rwID_2DPATH 0x1A1

RwFreeList lbl_805E1908; /* the font free list */
RwFreeList lbl_805E192C; /* the font dictionary node free list */

RwInt32 lbl_805F1538;

/* Reads the character code at a string position: two bytes for fonts of wide characters */
#define FONTCHARCODE(p) ((p)[0] << highShift | ((p)[lowIndex] & lowMask) << lowShift)

/* Looks up the character of a code, or -1 */
#define FONTCHARINDEX(index, font, code)                                                           \
    do {                                                                                           \
        if ((code) >= rt2dFONTMAPSIZE) {                                                           \
            RwInt32 offset_ = (code) - (font)->extMapOffset;                                       \
                                                                                                   \
            if (offset_ >= 0 && offset_ < (font)->extMapSize) {                                    \
                (index) = (font)->extMap[offset_];                                                 \
            } else {                                                                               \
                (index) = -1;                                                                      \
            }                                                                                      \
        } else {                                                                                   \
            (index) = (font)->map[code];                                                           \
        }                                                                                          \
    } while (0)

/* Sets up the character code reading for a font's character size */
#define FONTCHARSETUP(font)                                                                        \
    do {                                                                                           \
        if ((font)->flag & rt2dFONTFLAGWIDECHARS) {                                                \
            charSize = 2;                                                                          \
            lowMask = 0xFF;                                                                        \
            lowIndex = 1;                                                                          \
            highShift = 8;                                                                         \
            lowShift = 0;                                                                          \
        } else {                                                                                   \
            charSize = 1;                                                                          \
            lowMask = 0;                                                                           \
            lowIndex = 0;                                                                          \
            highShift = 0;                                                                         \
            lowShift = 0;                                                                          \
        }                                                                                          \
    } while (0)

/* Shows a string with outline characters: each character's path is filled in turn */
Rt2dFont *fn_80470AC4(Rt2dFont *font, const RwChar *string, RwReal height, RwV2d *anchor,
                      Rt2dBrush *brush) {
    if (font != NULL) {
        rt2dCharRect *charRects = fn_8047DFAC(font->charRects);
        RwInt32 charSize;
        RwUInt8 lowMask;
        RwInt32 lowIndex;
        RwInt32 highShift;
        RwInt32 lowShift;
        RwInt32 code;

        FONTCHARSETUP(font);

        fn_804747A0();
        fn_80474964(anchor->x, anchor->y);
        fn_804748FC(height, height);
        fn_80474964(0.0f, -font->baseline);

        for (code = FONTCHARCODE((const RwUInt8 *)string); code > 0;
             code = FONTCHARCODE((const RwUInt8 *)string)) {
            RwInt32 index;

            FONTCHARINDEX(index, font, code);

            string += charSize;
            if (index >= 0) {
                rt2dCharRect *charRect = &charRects[index];

                if (charRect->texture == NULL) {
                    fn_804771C0(charRect->path, brush);
                    fn_80474964(charRect->width + font->intergap, 0.0f);
                }
            }
        }
        fn_804748C8();
    }
    return font;
}

/* Interpolates a brush's colour along its edge at a fraction of the string */
#define BRUSHEDGECOLOR(vertex, base, delta, t)                                                     \
    do {                                                                                           \
        RwRGBAReal col_;                                                                           \
                                                                                                   \
        col_.red = (delta).red * (t);                                                              \
        col_.green = (delta).green * (t);                                                          \
        col_.blue = (delta).blue * (t);                                                            \
        col_.alpha = (delta).alpha * (t);                                                          \
        col_.red = (base).red + col_.red;                                                          \
        col_.green = (base).green + col_.green;                                                    \
        col_.blue = (base).blue + col_.blue;                                                       \
        col_.alpha = (base).alpha + col_.alpha;                                                    \
        (vertex)->color.red = col_.red;                                                            \
        (vertex)->color.green = col_.green;                                                        \
        (vertex)->color.blue = col_.blue;                                                          \
        (vertex)->color.alpha = col_.alpha;                                                        \
    } while (0)

/* Sets a vertex's colour from the brush: interpolated along an edge, or uniform */
#define BRUSHVERTEXCOLOR(vertex, edge, dedge, t)                                                   \
    do {                                                                                           \
        if (brush->flag & FIELDRGBA) {                                                             \
            BRUSHEDGECOLOR(vertex, brush->edge.col, brush->dedge.col, t);                          \
        } else {                                                                                   \
            (vertex)->color.red = brush->colorCache.red;                                           \
            (vertex)->color.green = brush->colorCache.green;                                       \
            (vertex)->color.blue = brush->colorCache.blue;                                         \
            (vertex)->color.alpha = brush->colorCache.alpha;                                       \
        }                                                                                          \
    } while (0)

/* Draws the quads of the characters so far */
#define FLUSHCHARS()                                                                               \
    do {                                                                                           \
        if (numIndices != 0 &&                                                                     \
            fn_8049A82C(lbl_805E1950.vertexBuffer, vertex - lbl_805E1950.vertexBuffer, ltm,        \
                        rwIM3D_VERTEXUV | rwIM3D_NOCLIP) != NULL) {                                \
            fn_8049A95C(rwPRIMTYPETRILIST, indices, numIndices);                                   \
            fn_8049A90C();                                                                         \
        }                                                                                          \
    } while (0)

/* Shows a string with texture characters: a quad per character, batched while the texture stays
 * the same, coloured across the string by the brush */
Rt2dFont *fn_80470C54(Rt2dFont *font, const RwChar *string, RwReal height, RwV2d *anchor,
                      Rt2dBrush *brush) {
    RwReal layerDepth = lbl_805E1950.layerDepth;

    if (font != NULL) {
        rt2dCharRect *charRects = fn_8047DFAC(font->charRects);
        RwInt32 charSize;
        RwUInt8 lowMask;
        RwInt32 lowIndex;
        RwInt32 highShift;
        RwInt32 lowShift;
        RwMatrix *ltm;
        RwReal x;
        RwReal t;
        RwReal invWidth;
        RwIm3DVertex *vertex;
        RwImVertexIndex *indices;
        RwTexture *texture;
        RwInt32 numIndices;
        RwInt32 code;

        FONTCHARSETUP(font);

        fn_804747A0();
        fn_80474964(anchor->x, anchor->y);
        fn_804748FC(height, height);
        fn_80474964(0.0f, -font->baseline);
        ltm = fn_804749CC();

        invWidth = 1.0f / fn_80473584(font, string, 1.0f);
        t = 0.0f;
        vertex = lbl_805E1950.vertexBuffer;
        x = t;
        indices = lbl_805E1950.fontIndices;
        texture = NULL;
        numIndices = 0;

        for (code = FONTCHARCODE((const RwUInt8 *)string); code > 0;
             code = FONTCHARCODE((const RwUInt8 *)string)) {
            RwInt32 index;

            FONTCHARINDEX(index, font, code);

            string += charSize;
            if (index >= 0) {
                rt2dCharRect *charRect = &charRects[index];

                if (charRect->texture != texture || numIndices > (rt2dFONTMAXQUADS - 1) * 6) {
                    FLUSHCHARS();
                    if (charRect->texture != texture) {
                        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER,
                                         (void *)(charRect->texture->filterAddressing & 0xFF));
                        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, charRect->texture->raster);
                        texture = charRect->texture;
                    }
                    numIndices = 0;
                    vertex = lbl_805E1950.vertexBuffer;
                    if (brush->texture != NULL && (brush->flag & FIELDUV)) {
                        brush->flag &= ~FIELDUV;
                    }
                }

                vertex[0].objVertex.x = x;
                vertex[0].objVertex.y = 0.0f;
                vertex[0].objVertex.z = layerDepth;
                vertex[0].u = charRect->uvTopLeft.x;
                vertex[0].v = charRect->uvBottomRight.y;
                BRUSHVERTEXCOLOR(&vertex[0], bottom, dbottom, t * invWidth);

                vertex[1].objVertex.x = x;
                vertex[1].objVertex.y = 1.0f;
                vertex[1].objVertex.z = layerDepth;
                vertex[1].u = charRect->uvTopLeft.x;
                vertex[1].v = charRect->uvTopLeft.y;
                BRUSHVERTEXCOLOR(&vertex[1], top, dtop, t * invWidth);

                vertex[2].objVertex.x = x + charRect->width;
                vertex[2].objVertex.y = 0.0f;
                vertex[2].objVertex.z = layerDepth;
                vertex[2].u = charRect->uvBottomRight.x;
                vertex[2].v = charRect->uvBottomRight.y;
                BRUSHVERTEXCOLOR(&vertex[2], bottom, dbottom, invWidth * (t + charRect->width));

                vertex[3].objVertex.x = x + charRect->width;
                vertex[3].objVertex.y = 1.0f;
                vertex[3].objVertex.z = layerDepth;
                vertex[3].u = charRect->uvBottomRight.x;
                vertex[3].v = charRect->uvTopLeft.y;
                BRUSHVERTEXCOLOR(&vertex[3], top, dtop, invWidth * (t + charRect->width));

                vertex += 4;
                numIndices += 6;
                x += charRect->width + font->intergap;
                t += charRect->width + font->intergap;
            }
        }
        FLUSHCHARS();
        anchor->x += x * height;
        fn_804748C8();
    }
    return font;
}

/* Builds a font's extended map from the codes of its characters at and above 0x80 */
Rt2dFont *fn_80471300(Rt2dFont *font) {
    RwInt32 *codes = lbl_805E1950.charCodes;
    RwInt32 minCode = 0xFFFF;
    RwInt32 maxCode = 0;
    RwInt32 found = 0;
    RwInt32 i;

    for (i = 0; i < font->numChars; i++) {
        RwInt32 code = codes[i];

        if (code > 0) {
            if (code < minCode) {
                minCode = code;
            }
            if (code > maxCode) {
                maxCode = code;
            }
            found = 1;
        }
    }
    if (found > 0) {
        RwInt32 range = maxCode - minCode;
        RwInt32 size = range + 1;
        RwInt16 *extMap = RwMalloc(size * sizeof(RwInt16), rwMEMHINTDUR_EVENT | RT2DMEMID);

        if (extMap == NULL) {
            return NULL;
        }
        for (i = 0; i < size; i++) {
            extMap[i] = -1;
        }
        for (i = 0; i < font->numChars; i++) {
            RwInt32 code = lbl_805E1950.charCodes[i];

            if (code > 0) {
                extMap[code - minCode] = i;
            }
        }
        font->extMapSize = size;
        font->extMapOffset = minCode;
        font->extMap = extMap;
    }
    return font;
}

/* Makes room for another code in the character codes, 32 at a time, with unused codes -1 */
#define GROWCHARCODES(codes)                                                                       \
    do {                                                                                           \
        if (numChars >= lbl_805E1950.numCharCodes) {                                               \
            RwInt32 newSize = lbl_805E1950.numCharCodes + 32;                                      \
            RwInt32 *newCodes =                                                                    \
                RwMalloc(newSize * sizeof(RwInt32), rwMEMHINTDUR_EVENT | RT2DMEMID);               \
            RwInt32 j;                                                                             \
                                                                                                   \
            memcpy(newCodes, codes, lbl_805E1950.numCharCodes * sizeof(RwInt32));                  \
            for (j = lbl_805E1950.numCharCodes; j < newSize; j++) {                                \
                newCodes[j] = -1;                                                                  \
            }                                                                                      \
            if (codes != NULL) {                                                                   \
                RwFree(codes);                                                                     \
            }                                                                                      \
            lbl_805E1950.charCodes = newCodes;                                                     \
            codes = newCodes;                                                                      \
            lbl_805E1950.numCharCodes = newSize;                                                   \
        }                                                                                          \
    } while (0)

/* Maps a code to the next character: through the extended map at and above 0x80 */
#define MAPCHARCODE(font, codes, code)                                                             \
    do {                                                                                           \
        if ((code) >= rt2dFONTMAPSIZE) {                                                           \
            (font)->flag |= rt2dFONTFLAGWIDECHARS;                                                 \
            (codes)[numChars] = (code);                                                            \
        } else {                                                                                   \
            (font)->map[code] = numChars;                                                          \
        }                                                                                          \
    } while (0)

/* Makes room for 32 more characters when they are used up */
#define GROWCHARRECTS(font)                                                                        \
    do {                                                                                           \
        if (numChars >= maxChars) {                                                                \
            fn_8047DEA0((font)->charRects, 32, rwMEMHINTDUR_EVENT | RT2DMEMID);                    \
            charRects = fn_8047DFAC((font)->charRects);                                            \
            maxChars += 32;                                                                        \
            memset(&charRects[numChars], 0, 32 * sizeof(rt2dCharRect));                            \
        }                                                                                          \
    } while (0)

/* Reads the characters of a metrics file of the first version: a texture with an optional mask,
 * the baseline, then a rectangle of the texture per character */
Rt2dFont *fn_8047149C(Rt2dFont *font, RwChar *textureName, void *fp, RwChar *line) {
    RwTexDictionary *oldDict = fn_8048D0D8();
    Rt2dFont *result;
    RwInt16 numChars;
    RwInt32 maxChars;
    RwInt32 *codes;
    rt2dCharRect *charRects;
    RwInt32 numNames;
    RwChar *maskName;

    fn_8048D0C4(lbl_805E1950.fontTexDictCurrent);
    result = font;
    font->isOutline = FALSE;
    numChars = 0;
    maxChars = font->numChars;
    if (maxChars == 0) {
        maxChars = 0x100;
    }
    codes = lbl_805E1950.charCodes;
    fn_8047DEA0(font->charRects, maxChars, rwMEMHINTDUR_EVENT | RT2DMEMID);
    charRects = fn_8047DFAC(font->charRects);
    memset(charRects, 0, maxChars * sizeof(rt2dCharRect));

    fn_8047A518(line, 0x100, fp);
    maskName = lbl_805E1950.fontMaskName;
    numNames = rwsscanf(line, "%s %s\n", textureName, maskName);
    if (numNames == 1 || numNames == 2) {
        RwTexture *texture;

        fn_804885DC(lbl_805E1950.fontPath);
        if (numNames == 1) {
            rwsprintf(maskName, "%sm%s", lbl_805E1950.fontPath, textureName);
            if (fn_8047A67C(maskName)) {
                rwsprintf(maskName, "m%s", textureName);
                texture = fn_8048D130(textureName, maskName);
            } else {
                texture = fn_8048D130(textureName, NULL);
            }
        } else {
            texture = fn_8048D130(textureName, maskName);
        }

        if (texture != NULL) {
            RwInt32 width;
            RwInt32 height;
            RwInt32 baseline;
            RwInt32 code;
            RwInt32 left;
            RwInt32 right;
            RwInt32 top;
            RwInt32 bottom;
            RwReal invWidth;
            RwReal invHeight;

            font->textures[0] = texture;
            width = texture->raster->width;
            height = texture->raster->height;
            fn_8047A518(line, 0x100, fp);
            rwsscanf(line, "%d", &baseline);
            invWidth = 1.0f / width;
            invHeight = 1.0f / height;

            while (result == font && fn_8047A518(line, 0x100, fp) != NULL) {
                if (rwsscanf(line, "%i %i %i %i %i\n", &code, &left, &top, &right, &bottom) == 5) {
                    if (code > 0xFFFF) {
                        result = NULL;
                    } else {
                        rt2dCharRect *charRect;

                        GROWCHARCODES(codes);
                        MAPCHARCODE(font, codes, code);
                        charRect = &charRects[numChars];
                        numChars++;
                        charRect->uvTopLeft.x = (0.5f + left) * invWidth;
                        charRect->uvTopLeft.y = (0.5f + top) * invHeight;
                        charRect->uvBottomRight.x = (0.5f + right) * invWidth;
                        charRect->uvBottomRight.y = (0.5f + bottom) * invHeight;
                        charRect->texture = texture;
                        charRect->width = (RwReal)(right - left) / (RwReal)(bottom - top);
                        charRect->textureIndex = 0;
                        GROWCHARRECTS(font);
                    }
                }
            }
            if (result == font) {
                font->numChars = numChars;
                fn_80471300(font);
                if (lbl_805E1950.charCodes != NULL) {
                    memset(lbl_805E1950.charCodes, -1, lbl_805E1950.numCharCodes * sizeof(RwInt32));
                }
                font->height = bottom - top;
                font->baseline = baseline / font->height;
                font->intergap = 0.0f;
            }
        } else {
            result = NULL;
        }
    }
    fn_8048D0C4(oldDict);
    return result;
}

/* Decodes a UTF-8 character at a string position into code and returns its length, or -1 */
#define UTF8DECODE(string, code, length)                                                           \
    do {                                                                                           \
        RwChar lead = (string)[0];                                                                 \
                                                                                                   \
        if ((lead & 0x80) == 0) {                                                                  \
            (code) = lead;                                                                         \
            (length) = 1;                                                                          \
        } else if ((lead & 0xE0) == 0xC0) {                                                        \
            (length) = 2;                                                                          \
            (code) = ((lead << 6) & 0x7C0) | ((string)[1] & 0x3F);                                 \
        } else if ((lead & 0xF0) == 0xE0) {                                                        \
            (length) = 3;                                                                          \
            (code) =                                                                               \
                ((lead << 12) & 0xF000) | (((string)[1] << 6) & 0xFC0) | ((string)[2] & 0x3F);     \
        } else if ((lead & 0xF8) == 0xF0) {                                                        \
            (code) = ((lead << 18) & 0x1C0000) | (((string)[1] << 12) & 0x3F000) |                 \
                     (((string)[2] << 6) & 0xFC0) | ((string)[3] & 0x3F);                          \
            (length) = 4;                                                                          \
        } else if ((lead & 0xFC) == 0xF8) {                                                        \
            (code) = ((lead << 24) & 0x3000000) | (((string)[1] << 18) & 0xFC0000) |               \
                     (((string)[2] << 12) & 0x3F000) | (((string)[3] << 6) & 0xFC0) |              \
                     ((string)[4] & 0x3F);                                                         \
            (length) = 5;                                                                          \
        } else if ((lead & 0xFE) == 0xFC) {                                                        \
            (code) = ((lead << 30) & 0x40000000) | (((string)[1] << 24) & 0x3F000000) |            \
                     (((string)[2] << 18) & 0xFC0000) | (((string)[3] << 12) & 0x3F000) |          \
                     (((string)[4] << 6) & 0xFC0) | ((string)[5] & 0x3F);                          \
            (length) = 6;                                                                          \
        } else {                                                                                   \
            (length) = -1;                                                                         \
        }                                                                                          \
    } while (0)

/* Moves the rest of the line to the start of the buffer and reads more after it */
#define SHIFTLINE()                                                                                \
    do {                                                                                           \
        length = rwstrlen(p);                                                                      \
        memcpy(line, p, length + 1);                                                               \
        p = line;                                                                                  \
        if (line[length - 1] == '\n') {                                                            \
            length--;                                                                              \
        }                                                                                          \
        fn_8047A518(&line[length], 0x100 - length, fp);                                            \
    } while (0)

/* Reads the characters of a metrics file of the second version: up to four textures, each with
 * its characters in rows of cells separated by marker pixels, named by UTF-8 text */
Rt2dFont *fn_80471A54(Rt2dFont *font, RwChar *textureName, void *fp, RwChar *line) {
    RwTexDictionary *oldDict;
    Rt2dFont *result;
    RwUInt32 textureIndex;
    RwInt32 charOffset;
    RwInt16 numChars;
    RwInt32 maxChars;
    Rt2dGlobalVars *globals;
    RwInt32 *codes;
    RwInt16 *map;
    RwChar *p;
    rt2dCharRect *charRects;
    RwInt32 codeOffset;
    RwImage *image;
    RwInt32 rectOffset;
    RwInt32 imageWidth;
    RwInt32 imageHeight;
    RwChar *maskName;
    RwInt32 code;

    textureIndex = 0;
    oldDict = fn_8048D0D8();
    fn_8048D0C4(lbl_805E1950.fontTexDictCurrent);
    result = font;
    numChars = 0;
    font->isOutline = FALSE;
    charOffset = 0;
    maxChars = font->numChars;
    if (maxChars == 0) {
        maxChars = 0x100;
    }
    globals = &lbl_805E1950;
    codes = lbl_805E1950.charCodes;
    map = font->map;
    p = NULL;
    fn_8047DEA0(font->charRects, maxChars, rwMEMHINTDUR_EVENT | RT2DMEMID);
    charRects = fn_8047DFAC(font->charRects);
    memset(charRects, 0, maxChars * sizeof(rt2dCharRect));
    codeOffset = 0;
    image = NULL;
    rectOffset = 0;
    imageHeight = 0;
    maskName = lbl_805E1950.fontMaskName;
    imageWidth = 0;

    while (textureIndex < 4 && fn_8047A518(line, 0x100, fp) != NULL) {
        RwUInt32 numNames = rwsscanf(line, "%s %s\n", textureName, maskName);
        RwTexture *texture;

        if (numNames == 0) {
            break;
        }
        if (numNames != 1 && numNames != 2) {
            break;
        }

        fn_804885DC(lbl_805E1950.fontPath);
        if (numNames == 1) {
            texture = fn_8048D130(textureName, NULL);
        } else {
            texture = fn_8048D130(textureName, maskName);
        }

        if (texture != NULL) {
            RwRaster *raster;
            RwInt32 width;
            RwInt32 height;
            RwReal invWidth;
            RwReal invHeight;
            RwInt32 baseline;
            RwInt32 cellHeight;
            RwInt32 row;
            RwUInt8 *pixels;
            RwInt32 pixelSize;

            font->textures[textureIndex] = texture;
            raster = texture->raster;
            width = raster->width;
            height = raster->height;
            if (image != NULL) {
                if (imageWidth != width) {
                    fn_80487DB0(image);
                    image = NULL;
                } else if (imageHeight != height) {
                    fn_80487DB0(image);
                    image = NULL;
                }
            }
            if (image == NULL) {
                image = fn_80487D08(width, height, 32);
                fn_80487E18(image);
            }
            fn_804899A8(image, raster);
            imageWidth = width;
            invWidth = 1.0f / width;
            imageHeight = height;
            invHeight = 1.0f / height;
            fn_8047A518(line, 0x100, fp);
            rwsscanf(line, "%d", &baseline);

            /* the cell height: the first row that starts with the marker pixel */
            cellHeight = 0;
            pixels = image->cpPixels;
            pixelSize = image->depth >> 3;
            for (row = 1; row < height; row++) {
                if (memcmp(pixels, pixels + image->stride * row, pixelSize) == 0) {
                    cellHeight = row;
                    break;
                }
            }

            if (cellHeight > 0) {
                RwInt32 cellTop;

                for (cellTop = 0; cellTop < height && result == font; cellTop += cellHeight) {
                    RwInt32 left = 0;
                    RwInt32 x;
                    RwInt32 xOffset = pixelSize;
                    RwInt32 codeIndex = codeOffset;
                    RwInt32 rectIndex = rectOffset;
                    RwInt32 cellBottom = cellTop + cellHeight;

                    for (x = 1; x < width && result == font; x++, xOffset += pixelSize) {
                        RwInt32 cellWidth;
                        RwInt32 length;

                        if (memcmp(pixels, pixels + image->stride * cellTop + xOffset, pixelSize) !=
                            0) {
                            continue;
                        }
                        cellWidth = x - left;
                        if (cellWidth == 1) {
                            continue;
                        }

                        if (p == NULL || *p == '\n' || *p == '\0') {
                            fn_8047A518(line, 0x100, fp);
                            p = line;
                        } else if (p - line >= 0x80) {
                            length = rwstrlen(p);
                            if ((p + length) - line == 0xFF) {
                                SHIFTLINE();
                            }
                        } else if (p - line >= 0x80) { /* (sic) */
                            SHIFTLINE();
                        }

                        UTF8DECODE(p, code, length);
                        if (length > 0 && code <= 0xFFFF) {
                            rt2dCharRect *charRect;

                            p += length;
                            GROWCHARCODES(codes);
                            globals->charCodes = codes;
                            if (code >= rt2dFONTMAPSIZE) {
                                font->flag |= rt2dFONTFLAGWIDECHARS;
                                codes[codeIndex] = code;
                            } else {
                                map[code] = numChars;
                            }
                            numChars++;
                            codeIndex++;
                            rectIndex++;
                            charRect = &charRects[charOffset];
                            codeOffset++;
                            rectOffset++;
                            charRect->uvTopLeft.x = (2.5f + left) * invWidth;
                            charRect->uvTopLeft.y = (2.5f + cellTop) * invHeight;
                            charRect->uvBottomRight.x = (x - 0.5f) * invWidth;
                            charRect->uvBottomRight.y = (cellBottom - 0.5f) * invHeight;
                            charRect->texture = texture;
                            charRect->width =
                                (RwReal)(cellWidth - 3) / (RwReal)(cellBottom - cellTop - 3);
                            charRect->textureIndex = textureIndex;
                            charOffset++;
                            GROWCHARRECTS(font);
                        } else {
                            textureIndex = 4;
                            result = NULL;
                        }
                        left = x;
                    }
                }
                p = NULL;
                textureIndex++;
                font->height = cellHeight - 3.0f;
                font->baseline = baseline / font->height;
                font->intergap = 0.0f;
            } else {
                textureIndex = 4;
                result = NULL;
            }
        } else {
            textureIndex = 4;
            result = NULL;
        }
    }
    if (image != NULL) {
        fn_80487DB0(image);
    }
    if (result == font) {
        font->numChars = numChars;
        fn_80471300(font);
        if (globals->charCodes != NULL) {
            memset(lbl_805E1950.charCodes, -1, lbl_805E1950.numCharCodes * sizeof(RwInt32));
        }
    }
    fn_8048D0C4(oldDict);
    return result;
}

/* Reads the characters of a metrics file of the third version: each named by a UTF-8 line and
 * outlined by path commands between "begin" and "end" */
Rt2dFont *fn_804724AC(Rt2dFont *font, RwChar *textureName, void *fp, RwChar *line) {
    Rt2dFont *result;
    Rt2dPath *path;
    RwInt32 maxChars;
    RwInt32 numChars;
    RwInt32 rectOffset;
    RwInt32 *codes;
    RwInt16 *map;
    rt2dCharRect *charRects;
    RwChar name[128];
    RwInt32 code;

    font->isOutline = TRUE;
    font->height = 1.0f;
    result = font;
    font->baseline = 0.0f;
    path = NULL;
    font->intergap = 0.0f;
    fn_8047A518(line, 0x100, fp);
    maxChars = font->numChars;
    numChars = 0;
    rectOffset = 0;
    if (maxChars == 0) {
        maxChars = 0x100;
    }
    codes = lbl_805E1950.charCodes;
    map = font->map;
    fn_8047DEA0(font->charRects, maxChars, rwMEMHINTDUR_EVENT | RT2DMEMID);
    charRects = fn_8047DFAC(font->charRects);
    memset(charRects, 0, maxChars * sizeof(rt2dCharRect));

    if (rwsscanf(line, "%[^\n]", name) == 1) {
        RwInt32 codeOffset = 0;
        RwInt32 rectBytes = 0;
        RwReal x3;
        RwReal x2;
        RwReal x1;
        RwReal y3;
        RwReal y2;
        RwReal y1;

        code = 0;
        while (result == font && fn_8047A518(line, 0x100, fp) != NULL) {
            if (code == 0) {
                RwInt32 length;

                UTF8DECODE(&line[1], code, length);
                if (length < 0 || code > 0xFFFF) {
                    result = NULL;
                }
            } else if (rwstrcmp(line, "begin\n") == 0) {
                path = fn_80476490();
                fn_80476510(path);
            } else if (rwstrcmp(line, "end\n") == 0) {
                Rt2dPath *flattened = fn_80475B4C(path);
                rt2dCharRect *charRect;

                fn_80476830(path);
                GROWCHARCODES(codes);
                if (code >= rt2dFONTMAPSIZE) {
                    font->flag |= rt2dFONTFLAGWIDECHARS;
                    codes[codeOffset] = code;
                } else {
                    map[code] = numChars;
                }
                charRect = &charRects[rectOffset];
                numChars++;
                charRect->path = flattened;
                code = 0;
                path = NULL;
                codeOffset++;
                rectBytes += sizeof(rt2dCharRect);
                rectOffset++;
                charRect->width = x1;
                if (numChars >= maxChars) {
                    fn_8047DEA0(font->charRects, 32, rwMEMHINTDUR_EVENT | RT2DMEMID);
                    charRects = fn_8047DFAC(font->charRects);
                    maxChars += 32;
                    memset((RwUInt8 *)charRects + rectBytes, 0, 32 * sizeof(rt2dCharRect));
                }
            } else if (path != NULL) {
                if (rwsscanf(line, "moveto %f %f\n", &x1, &y1) == 2) {
                    fn_80474C7C(path, x1, y1);
                } else if (rwsscanf(line, "lineto %f %f\n", &x1, &y1) == 2) {
                    fn_80474EA0(path, x1, y1);
                } else if (rwsscanf(line, "curveto %f %f %f %f %f %f\n", &x1, &y1, &x2, &y2, &x3,
                                    &y3) == 6) {
                    fn_80474FF8(path, x1, y1, x2, y2, x3, y3);
                } else if (rwstrcmp(line, "closepath\n") == 0) {
                    fn_80475260(path);
                }
            }
        }
    }
    if (result == font) {
        font->numChars = numChars;
        fn_80471300(font);
        if (lbl_805E1950.charCodes != NULL) {
            memset(lbl_805E1950.charCodes, -1, lbl_805E1950.numCharCodes * sizeof(RwInt32));
        }
        font->showCB = fn_80470AC4;
    }
    return result;
}

/* Reads the texture characters of a font stream: their rectangles and widths, then the textures
 * by name */
Rt2dFont *fn_80472A60(Rt2dFont *font, RwStream *stream) {
    rt2dCharRect *charRects = fn_8047DFAC(font->charRects);
    RwInt32 numTextures;
    RwInt32 i;

    for (i = 0; i < font->numChars; i++) {
        if (fn_8047D784(stream, &charRects[i].uvTopLeft.x, sizeof(RwV2d) * 2) == NULL) {
            return NULL;
        }
        if (fn_8047D784(stream, &charRects[i].width, sizeof(RwReal)) == NULL) {
            return NULL;
        }
        if (fn_80480D4C(stream, &charRects[i].textureIndex, sizeof(RwInt8)) != sizeof(RwInt8)) {
            return NULL;
        }
        charRects[i].path = NULL;
    }

    if (fn_8047D868(stream, &numTextures, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    for (i = 0; i < numTextures; i++) {
        RwChar name[32];
        RwTexture *texture;

        if (fn_80480D4C(stream, name, sizeof(name)) != sizeof(name)) {
            return NULL;
        }
        texture = fn_8048D130(name, NULL);
        font->textures[i] = texture;
        if (texture == NULL) {
            return NULL;
        }
    }

    for (i = 0; i < font->numChars; i++) {
        charRects[i].texture = font->textures[charRects[i].textureIndex];
        if (charRects[i].texture->raster == NULL) {
            return NULL;
        }
    }
    font->field_0x13C = NULL;
    font->field_0x140 = NULL;
    font->showCB = fn_80470C54;
    return font;
}

/* Creates an empty font of texture characters */
static inline Rt2dFont *FontCreate(void) {
    Rt2dFont *font = RwFreeListAlloc(lbl_805E1950.fontFreeList, rwMEMHINTDUR_EVENT | RT2DFONTMEMID);
    RwInt32 i;

    if (font == NULL) {
        return NULL;
    }
    font->isOutline = TRUE;
    font->height = 1.0f;
    font->baseline = 0.0f;
    font->intergap = 0.0f;
    font->flag = 0;
    font->textures[0] = NULL;
    font->textures[1] = NULL;
    font->textures[2] = NULL;
    font->textures[3] = NULL;
    font->extMapSize = 0;
    font->numChars = 0;
    font->extMapOffset = 0;
    font->extMap = NULL;
    font->charRects = fn_8047DC90(sizeof(rt2dCharRect), rwMEMHINTDUR_EVENT | RT2DMEMID);
    font->showCB = fn_80470C54;
    font->field_0x13C = NULL;
    font->field_0x140 = NULL;
    for (i = 0; i < rt2dFONTMAPSIZE; i++) {
        font->map[i] = -1;
    }
    return font;
}

/* Reads the outline characters of a font stream: each one's width and optional path */
static inline Rt2dFont *FontStreamReadOutlines(Rt2dFont *font, RwStream *stream) {
    rt2dCharRect *charRect = fn_8047DFAC(font->charRects);
    RwInt32 i;

    for (i = 0; i < font->numChars; i++) {
        Rt2dPath *path = NULL;
        RwInt32 type;

        if (fn_8047D868(stream, &type, sizeof(RwInt32)) == NULL) {
            return NULL;
        }
        if (fn_8047D784(stream, &charRect->width, sizeof(RwReal)) == NULL) {
            return NULL;
        }
        if (fn_8047D868(stream, &type, sizeof(RwInt32)) == NULL) {
            return NULL;
        }
        if (type == 5) {
            if (!fn_8047CF50(stream, rwID_2DPATH, NULL, NULL)) {
                return NULL;
            }
            path = fn_80476C4C(stream);
            if (fn_8047D868(stream, &type, sizeof(RwInt32)) == NULL) {
                return NULL;
            }
        }
        if (fn_8047D868(stream, &type, sizeof(RwInt32)) == NULL) {
            return NULL;
        }
        charRect->path = path;
        charRect++;
    }
    font->showCB = fn_80470AC4;
    return font;
}

Rt2dFont *fn_80472C14(RwStream *stream) {
    Rt2dFont *font = FontCreate();
    RwInt32 version;
    RwInt32 major;

    if (font == NULL) {
        return NULL;
    }
    if (fn_8047D868(stream, &version, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    major = (RwUInt32)version >> 24;
    if (major < 1) {
        return NULL;
    }
    if (fn_8047D868(stream, &font->isOutline, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (fn_8047D784(stream, &font->height, sizeof(RwReal)) == NULL) {
        return NULL;
    }
    if (fn_8047D784(stream, &font->baseline, sizeof(RwReal)) == NULL) {
        return NULL;
    }
    if (fn_8047D784(stream, &font->intergap, sizeof(RwReal)) == NULL) {
        return NULL;
    }
    if (fn_8047D868(stream, &font->flag, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (fn_8047D868(stream, &font->extMapOffset, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (fn_8047D868(stream, &font->numChars, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (fn_8047D868(stream, &font->extMapSize, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (font->extMapSize > 0) {
        font->extMap = RwMalloc(font->extMapSize * sizeof(RwInt16), rwMEMHINTDUR_EVENT | RT2DMEMID);
        memset(font->extMap, 0, font->extMapSize * sizeof(RwInt16));
        if (font->extMap == NULL) {
            fn_804736CC(font);
            return NULL;
        }
        if (fn_8047D684(stream, font->extMap, font->extMapSize * sizeof(RwInt16)) == NULL) {
            return NULL;
        }
    }
    if (fn_8047D684(stream, font->map, sizeof(font->map)) == NULL) {
        return NULL;
    }
    fn_8047DEA0(font->charRects, font->numChars, rwMEMHINTDUR_EVENT | RT2DMEMID);
    memset(fn_8047DFAC(font->charRects), 0, font->numChars * sizeof(rt2dCharRect));

    if (font->isOutline) {
        if (FontStreamReadOutlines(font, stream) == NULL) {
            fn_804736CC(font);
            font = NULL;
        }
    } else {
        RwTexDictionary *oldDict = fn_8048D0D8();

        fn_8048D0C4(lbl_805E1950.fontTexDictCurrent);
        if (fn_80472A60(font, stream) == NULL) {
            fn_804736CC(font);
            font = NULL;
        }
        fn_8048D0C4(oldDict);
    }
    return font;
}

void fn_80473118(void) {
    rt2dFontDictNode *node = lbl_805E1950.fontDict;

    while (node != NULL) {
        rt2dFontDictNode *freed;

        RwFree(node->name);
        fn_804736CC(node->font);
        freed = node;
        node = node->next;
        RwFreeListFree(lbl_805E1950.fontDictNodeFreeList, freed);
    }
    lbl_805E1950.fontDict = NULL;

    if (lbl_805E1950.fontTexDict != NULL) {
        fn_8048CB38(lbl_805E1950.fontTexDict);
    }
    lbl_805E1950.fontTexDict = NULL;
    lbl_805E1950.fontTexDictCurrent = NULL;

    fn_8047FCDC(lbl_805E1950.fontDictNodeFreeList);
    lbl_805E1950.fontDictNodeFreeList = NULL;
    fn_8047FCDC(lbl_805E1950.fontFreeList);
    lbl_805E1950.fontFreeList = NULL;

    if (lbl_805E1950.charCodes != NULL) {
        RwFree(lbl_805E1950.charCodes);
    }
    lbl_805E1950.charCodes = NULL;
    if (lbl_805E1950.fontPath != NULL) {
        RwFree(lbl_805E1950.fontPath);
    }
    lbl_805E1950.fontPath = NULL;
    lbl_805E1950.fontPathSize = 0;
    lbl_805F1538 = FALSE;
}

RwInt32 lbl_805EDE28 = 8;  /* font free list entries per block */
RwInt32 lbl_805EDE2C = 1;  /* font free list blocks to preallocate */
RwInt32 lbl_805EDE30 = 16; /* font dictionary node free list entries per block */
RwInt32 lbl_805EDE34 = 1;  /* font dictionary node free list blocks to preallocate */

RwBool fn_80473260(void) {
    RwImVertexIndex *index;
    RwInt32 i;

    lbl_805E1950.fontFreeList = fn_8047FC04(sizeof(Rt2dFont), lbl_805EDE28, 4, lbl_805EDE2C,
                                            &lbl_805E1908, rwMEMHINTDUR_GLOBAL | RT2DFONTMEMID);
    if (lbl_805E1950.fontFreeList == NULL) {
        return FALSE;
    }
    lbl_805E1950.fontDictNodeFreeList =
        fn_8047FC04(sizeof(rt2dFontDictNode), lbl_805EDE30, 4, lbl_805EDE34, &lbl_805E192C,
                    rwMEMHINTDUR_GLOBAL | RT2DMEMID);
    if (lbl_805E1950.fontDictNodeFreeList == NULL) {
        return FALSE;
    }
    lbl_805E1950.fontPath =
        RwMalloc(0x51, rwMEMHINTDUR_GLOBAL | rwMEMHINTFLAG_RESIZABLE | RT2DMEMID);
    if (lbl_805E1950.fontPath == NULL) {
        return FALSE;
    }
    lbl_805E1950.fontPathSize = 0x51;
    fn_80473460("fonts\\");

    /* two triangles per character quad */
    index = lbl_805E1950.fontIndices;
    for (i = 0; i < rt2dFONTMAXQUADS * 4; i += 4) {
        index[0] = i;
        index[1] = (RwImVertexIndex)i + 2;
        index[2] = (RwImVertexIndex)i + 3;
        index[3] = i;
        index[4] = (RwImVertexIndex)i + 3;
        index[5] = (RwImVertexIndex)i + 1;
        index += 6;
    }

    lbl_805E1950.fontDict = NULL;
    lbl_805E1950.fontTexDict = fn_8048CA64();
    lbl_805E1950.fontTexDictCurrent = lbl_805E1950.fontTexDict;
    lbl_805E1950.charCodes = NULL;
    lbl_805E1950.numCharCodes = 0;
    fn_80473528(fn_804737E4);
    lbl_805F1538 = TRUE;
    return TRUE;
}

const RwChar *fn_80473460(const RwChar *path) {
    if (path != NULL) {
        RwUInt32 size = rwstrlen(path) + 1;

        if (size > lbl_805E1950.fontPathSize) {
            RwChar *fontPath = RwRealloc(lbl_805E1950.fontPath, size,
                                         rwMEMHINTDUR_GLOBAL | rwMEMHINTFLAG_RESIZABLE | RT2DMEMID);

            if (fontPath == NULL) {
                return NULL;
            }
            lbl_805E1950.fontPath = fontPath;
            lbl_805E1950.fontPathSize = size;
        }
        rwstrcpy(lbl_805E1950.fontPath, path);
        return path;
    }
    return NULL;
}

RwBool fn_80473528(Rt2dFontReadCallBack readCB) {
    lbl_805E1950.fontReadCB = readCB;
    return TRUE;
}

RwReal fn_8047353C(Rt2dFont *font) {
    RwV2d xStep;
    RwV2d yStep;
    RwV2d origin;

    fn_80473FD0(&xStep, &yStep, &origin);
    return font->height * fn_804823F0(&yStep);
}

RwReal fn_80473584(Rt2dFont *font, const RwChar *string, RwReal height) {
    RwReal width = 0.0f;
    rt2dCharRect *charRects = fn_8047DFAC(font->charRects);
    RwInt32 charSize;
    RwUInt8 lowMask;
    RwInt32 lowIndex;
    RwInt32 highShift;
    RwInt32 lowShift;
    RwInt32 code;
    const RwUInt8 *p = (const RwUInt8 *)string;

    FONTCHARSETUP(font);
    for (code = FONTCHARCODE(p); code > 0; code = FONTCHARCODE(p)) {
        RwInt32 index;

        FONTCHARINDEX(index, font, code);

        p += charSize;
        if (index >= 0) {
            width += charRects[index].width + font->intergap;
        }
    }
    return width * height;
}

RwBool fn_804736CC(Rt2dFont *font) {
    if (font != NULL) {
        RwUInt32 i;

        if (font->charRects != NULL) {
            rt2dCharRect *charRects = fn_8047DFAC(font->charRects);

            for (i = 0; i < font->numChars; i++) {
                if (charRects[i].path != NULL) {
                    fn_80475E2C(charRects[i].path);
                    charRects[i].path = NULL;
                }
            }
            fn_8047DFC8(font->charRects);
        }
        font->charRects = NULL;
        if (font->extMap != NULL) {
            RwFree(font->extMap);
        }
        font->extMap = NULL;
        for (i = 0; i < 4; i++) {
            if (font->textures[i] != NULL) {
                fn_8048CD8C(font->textures[i]);
            }
            font->textures[i] = NULL;
        }
        RwFreeListFree(lbl_805E1950.fontFreeList, font);
        return TRUE;
    }
    return FALSE;
}

/* Reads a font's metrics file "<name>.met" from the first font path that has it */
Rt2dFont *fn_804737E4(const RwChar *name) {
    Rt2dFont *font = FontCreate();
    RwChar *fileName;
    RwChar textureName[128];
    RwChar line[256];

    if (font == NULL) {
        return NULL;
    }
    fileName = RwMalloc(rwstrlen(name) + rwstrlen(lbl_805E1950.fontPath) + 5,
                        rwMEMHINTDUR_FUNCTION | RT2DMEMID);
    if (fileName != NULL) {
        Rt2dFont *result;
        RwInt32 length = 0;
        const RwChar *path = lbl_805E1950.fontPath;
        void *fp = NULL;
        RwUInt8 c;

        do {
            c = *path;
            if (c == ';' || c == '\0') {
                fileName[length] = '\0';
                rwstrcat(fileName, name);
                rwstrcat(fileName, ".met");
                length = 0;
                fp = fn_8047A178(fileName, "r");
            } else {
                fileName[length] = c;
                length++;
            }
            path++;
        } while (c != '\0' && fp == NULL);

        result = NULL;
        if (fp != NULL) {
            fn_8047A518(line, sizeof(line), fp);
            if (rwstrcmp(line, "METRICS1\n") == 0) {
                result = fn_8047149C(font, textureName, fp, line);
            } else if (rwstrcmp(line, "METRICS2\n") == 0) {
                result = fn_80471A54(font, textureName, fp, line);
            } else if (rwstrcmp(line, "METRICS3\n") == 0) {
                result = fn_804724AC(font, textureName, fp, line);
            }
            fn_8047A388(fp);
        }
        RwFree(fileName);
        if (result != font) {
            fn_804736CC(font);
            font = NULL;
        }
    } else {
        fn_804736CC(font);
        font = NULL;
    }
    return font;
}

Rt2dFont *fn_80473CE0(const RwChar *name) {
    return lbl_805E1950.fontReadCB(name);
}

Rt2dFont *fn_80473D10(Rt2dFont *font, const RwChar *string, RwReal height, RwV2d *anchor,
                      Rt2dBrush *brush) {
    return font->showCB(font, string, height, anchor, brush);
}

RwInt32 fn_80473D38(Rt2dFont *font, RwInt32 *codes) {
    RwInt32 count = 0;
    RwUInt32 i;

    for (i = 0; i < rt2dFONTMAPSIZE; i++) {
        if (font->map[i] >= 0) {
            codes[count] = i;
            count++;
        }
    }
    for (i = 0; i < font->extMapSize; i++) {
        if (font->extMap[i] >= 0) {
            codes[count] = i + font->extMapOffset;
            count++;
        }
    }
    return count;
}

void fn_80473E8C(RwTexDictionary *dict) {
    lbl_805E1950.fontTexDictCurrent = dict;
}
