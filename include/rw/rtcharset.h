#ifndef RW_RTCHARSET_H
#define RW_RTCHARSET_H

#include "rw/rwcore.h"
#include "layout_check.h"

/* RenderWare character set toolkit: a raster holding a built-in bitmap font, and printing strings
 * with it as 2D immediate mode quads. The type and field names follow the RenderWare 3 RtCharset
 * toolkit, whose layout and behaviour the code matches (inferred). */

#ifdef __cplusplus
extern "C" {
#endif

/* A character set is a raster holding the font's characters */
typedef RwRaster RtCharset;

/* The metrics of the font and its layout in the raster (0x1C bytes) */
typedef struct RtCharsetDesc {
    /* 0x00 */ RwInt32 width;           /* of a character */
    /* 0x04 */ RwInt32 height;          /* of a character */
    /* 0x08 */ RwInt32 width_internal;  /* of a character's cell in the raster */
    /* 0x0C */ RwInt32 height_internal; /* of a character's cell in the raster */
    /* 0x10 */ RwInt32 count;           /* the characters, from the space up */
    /* 0x14 */ RwInt32 tile_width;      /* the cells across the raster */
    /* 0x18 */ RwInt32 tile_height;     /* the rows of cells in the raster */
} RtCharsetDesc;
CHECK_SIZEOF(RtCharsetDesc, 0x1C);

/* The buffered printing state, held between opening and closing the toolkit (0x18 bytes) */
typedef struct rtCharsetGlobals {
    /* 0x00 */ RwBool open;
    /* 0x04 */ RwInt32 field_0x4;
    /* 0x08 */ RwInt32 field_0x8;
    /* 0x0C */ RwIm2DVertex *vertices;
    /* 0x10 */ RwImVertexIndex *indices;
    /* 0x14 */ RwInt32 field_0x14;
} rtCharsetGlobals;
CHECK_SIZEOF(rtCharsetGlobals, 0x18);

extern rtCharsetGlobals lbl_8056DD68;

/* the toolkit (inferred) */
RwInt32 fn_8047832C(const RwChar *string, RwUInt32 count, RtCharset *charSet, RwInt32 x, RwInt32 y,
                    RwIm2DVertex *vertices, RwImVertexIndex *indices, RwImVertexIndex baseIndex,
                    RwBool hideSpaces); /* fills a string's quads */
RtCharset *fn_80478700(RtCharset *charSet, const RwChar *string, RwInt32 x,
                       RwInt32 y); /* prints a string */
RtCharset *fn_80478AC8(RtCharset *charSet, const RwChar *string, RwInt32 x,
                       RwInt32 y); /* prints a string */
const RwUInt8 *fn_80478AE8(RtCharsetDesc *desc,
                           RwInt32 *rasterSize); /* the font, its metrics and raster size */
RtCharset *fn_80478B5C(RtCharset *charSet, RtCharsetDesc *desc); /* gets the metrics */
RtCharset *fn_80478C14(RtCharset *charSet, const RwRGBA *foreground,
                       const RwRGBA *background); /* draws the font in two colours */
RtCharset *fn_80478DE0(const RwRGBA *foreground, const RwRGBA *background); /* creates */
void fn_80478F00(void);                                                     /* closes */
RwBool fn_80478F74(void);                                                   /* opens */

#ifdef __cplusplus
}
#endif

#endif
