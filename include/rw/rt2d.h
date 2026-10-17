#ifndef RW_RT2D_H
#define RW_RT2D_H

#include "rw/rwcore.h"
#include "layout_check.h"

/* RenderWare 2D toolkit. The type and field names follow the RenderWare 3 Rt2d brush, whose layout
 * and behaviour the code matches (inferred). */

#ifdef __cplusplus
extern "C" {
#endif

/* The colour and texture coordinates at one edge of a brush */
typedef struct rt2dShadeParameters {
    /* 0x00 */ RwRGBAReal col;
    /* 0x10 */ RwV2d uv;
} rt2dShadeParameters;
CHECK_SIZEOF(rt2dShadeParameters, 0x18);

/* The brush flags */
#define FIELDRGBA 0x1 /* the colours are not uniform */
/* Written independently by two setters, so it does not track one property: the texture
 * coordinate setter (fn_80470998) sets it when either edge's coordinates differ along the edge and
 * clears it otherwise, whatever the texture; the texture setter (fn_80470A4C) sets it for a
 * texture and clears it for none, whatever the coordinates */
#define FIELDUV 0x2

/* A brush (0x78 bytes): the colours and texture coordinates at the top edge and at the bottom
 * edge, each as a base and the difference to the second value (inferred) */
typedef struct Rt2dBrush {
    /* 0x00 */ rt2dShadeParameters top;
    /* 0x18 */ rt2dShadeParameters dtop;
    /* 0x30 */ rt2dShadeParameters bottom;
    /* 0x48 */ rt2dShadeParameters dbottom;
    /* 0x60 */ RwRGBA colorCache; /* the uniform colour */
    /* 0x64 */ RwInt32 flag;
    /* 0x68 */ RwTexture *texture;
    /* 0x6C */ RwUInt8 field_0x6C[0x4];
    /* 0x70 */ RwReal halfwidth;
    /* 0x74 */ RwInt32 refCount;
} Rt2dBrush;
CHECK_SIZEOF(Rt2dBrush, 0x78);

typedef struct Rt2dPath Rt2dPath;

/* The kinds of path segment */
#define rt2dMOVETO 0
#define rt2dLINETO 1
#define rt2dCURVETO 2

/* A point of a path (0x18 bytes): its kind, position, the averaged normal of its segments and the
 * path length up to it (inferred) */
typedef struct rt2dPathNode {
    /* 0x00 */ RwInt32 type;
    /* 0x04 */ RwV2d pos;
    /* 0x0C */ RwV2d normal;
    /* 0x14 */ RwReal dist;
} rt2dPathNode;
CHECK_SIZEOF(rt2dPathNode, 0x18);

/* A path (0x1C bytes): a chain of subpaths. While locked, the points are a list and numPoints is
 * -1; unlocked, they are an array of numPoints (inferred) */
struct Rt2dPath {
    /* 0x00 */ void *points;
    /* 0x04 */ RwInt32 numPoints;
    /* 0x08 */ RwBool closed;
    /* 0x0C */ RwBool flat; /* no curves */
    /* 0x10 */ RwReal field_0x10;
    /* 0x14 */ Rt2dPath *next;
    /* 0x18 */ Rt2dPath *curr; /* the subpath being added to */
};
CHECK_SIZEOF(struct Rt2dPath, 0x1C);
typedef struct Rt2dFont Rt2dFont;

/* Shows a string in a font at a height, from an anchor that it advances, with a brush */
typedef Rt2dFont *(*Rt2dFontShowCallBack)(Rt2dFont *font, const RwChar *string, RwReal height,
                                          RwV2d *anchor, Rt2dBrush *brush);

/* Reads a font by name */
typedef Rt2dFont *(*Rt2dFontReadCallBack)(const RwChar *name);

/* A character of a font (0x20 bytes): its width relative to the font height, and its rectangle
 * in a texture or its outline (inferred) */
typedef struct rt2dCharRect {
    /* 0x00 */ RwReal width;
    /* 0x04 */ RwTexture *texture;
    /* 0x08 */ RwV2d uvTopLeft;
    /* 0x10 */ RwV2d uvBottomRight;
    /* 0x18 */ Rt2dPath *path;
    /* 0x1C */ RwInt8 textureIndex; /* of the font's textures */
    /* 0x1D */ RwUInt8 field_0x1D[0x3];
} rt2dCharRect;
CHECK_SIZEOF(rt2dCharRect, 0x20);

/* The font flags */
#define rt2dFONTFLAGWIDECHARS 0x2 /* characters are two bytes (inferred) */

/* The number of codes a font maps directly; others go through its extended map */
#define rt2dFONTMAPSIZE 0x80

/* A font (0x144 bytes) (inferred) */
struct Rt2dFont {
    /* 0x000 */ RwInt32 isOutline; /* the characters are paths rather than textures */
    /* 0x004 */ RwReal height;
    /* 0x008 */ RwReal baseline; /* relative to the height */
    /* 0x00C */ RwReal intergap;
    /* 0x010 */ RwInt32 flag;
    /* 0x014 */ RwTexture *textures[4];
    /* 0x024 */ RwInt16 map[rt2dFONTMAPSIZE]; /* the character of each code, or -1 */
    /* 0x124 */ RwInt32 extMapOffset;         /* the first code of the extended map */
    /* 0x128 */ RwInt32 extMapSize;
    /* 0x12C */ RwInt16 *extMap;
    /* 0x130 */ RwInt32 numChars;
    /* 0x134 */ RwSList *charRects;
    /* 0x138 */ Rt2dFontShowCallBack showCB;
    /* 0x13C */ void *field_0x13C;
    /* 0x140 */ void *field_0x140;
};
CHECK_SIZEOF(struct Rt2dFont, 0x144);

/* A named font in the font dictionary (inferred) */
typedef struct rt2dFontDictNode {
    /* 0x0 */ RwChar *name;
    /* 0x4 */ Rt2dFont *font;
    /* 0x8 */ struct rt2dFontDictNode *next;
} rt2dFontDictNode;
CHECK_SIZEOF(rt2dFontDictNode, 0xC);

/* The depth of the transformation stack */
#define rt2dCTMSTACKSIZE 32

/* The vertices and indices of the characters drawn at once */
#define rt2dFONTMAXQUADS 16

/* The 2D toolkit's globals (0xD58 bytes); only the mapped members are used */
typedef struct Rt2dGlobalVars {
    /* 0x000 */ RwImVertexIndex indices[64];                       /* 0 to 63 */
    /* 0x080 */ RwImVertexIndex fontIndices[rt2dFONTMAXQUADS * 6]; /* two triangles per quad */
    /* 0x140 */ RwFreeList *brushFreeList;
    /* 0x144 */ RwIm3DVertex *vertexBuffer; /* 0x900 bytes (inferred) */
    /* 0x148 */ RwFreeList *fontFreeList;
    /* 0x14C */ RwChar *fontPath; /* paths separated by ';' */
    /* 0x150 */ RwUInt32 fontPathSize;
    /* 0x154 */ RwFreeList *fontDictNodeFreeList;
    /* 0x158 */ rt2dFontDictNode *fontDict;
    /* 0x15C */ RwTexDictionary *fontTexDict;        /* created when fonts open */
    /* 0x160 */ RwTexDictionary *fontTexDictCurrent; /* font textures are read into it */
    /* 0x164 */ RwFreeList *pathFreeList;
    /* 0x168 */ Rt2dPath *scratchPath;
    /* 0x16C */ RwFreeList *field_0x16C; /* of 0x100-byte objects (inferred) */
    /* 0x170 */ RwUInt8 field_0x170[0x4];
    /* 0x174 */ RwFreeList *field_0x174; /* of 8-byte objects (inferred) */
    /* 0x178 */ RwFreeList *field_0x178; /* of 0x14-byte objects (inferred) */
    /* 0x17C */ RwUInt8 field_0x17C[0x8];
    /* 0x184 */ RwMatrix baseMatrix; /* applied after the current transformation */
    /* 0x1C4 */ RwMatrix *ctmStack[rt2dCTMSTACKSIZE];
    /* 0x244 */ RwInt32 ctmIndex;
    /* 0x248 */ RwV2d viewWindow;      /* twice the camera's */
    /* 0x250 */ RwV2d rasterSize;      /* of the camera's frame buffer */
    /* 0x258 */ RwMatrix deviceMatrix; /* from the transformation to the camera's view */
    /* 0x298 */ RwMatrix inverseDeviceMatrix;
    /* 0x2D8 */ RwMatrix ltm; /* the device matrix in the camera frame */
    /* 0x318 */ RwBool deviceMatrixValid;
    /* 0x31C */ RwUInt32 field_0x31C; /* immediate mode flags for strokes */
    /* 0x320 */ RwReal layerDepth;
    /* 0x324 */ RwReal field_0x324;
    /* 0x328 */ RwReal pixelSizeSq; /* the squared length of a pixel in the transformation */
    /* 0x32C */ RwBool pixelSizeValid;
    /* 0x330 */ RwInt32 field_0x330;
    /* 0x334 */ RwUInt8 field_0x334[0x4];
    /* 0x338 */ RwV2d field_0x338;
    /* 0x340 */ RwV2d aspect; /* the view window's larger side over its smaller one */
    /* 0x348 */ RwUInt8 field_0x348[0x4];
    /* 0x34C */ RwCamera *camera;
    /* 0x350 */ RwUInt8 field_0x350[0x48];
    /* 0x398 */ RwCameraEndUpdateFunc cameraEndUpdate; /* the camera's own, which Rt2d wraps */
    /* 0x39C */ RwUInt8 field_0x39C[0x4A4];
    /* 0x840 */ Rt2dFontReadCallBack fontReadCB;
    /* 0x844 */ RwUInt8 field_0x844[0x100];
    /* 0x944 */ void *field_0x944;
    /* 0x948 */ RwInt32 field_0x948;
    /* 0x94C */ RwChar fontMaskName[0x400]; /* the mask texture name read with a font */
    /* 0xD4C */ RwInt32 *charCodes;         /* the extended codes of the characters read */
    /* 0xD50 */ RwInt32 numCharCodes;
    /* 0xD54 */ RwUInt8 field_0xD54[0x4];
} Rt2dGlobalVars;
CHECK_SIZEOF(Rt2dGlobalVars, 0xD58);

extern Rt2dGlobalVars lbl_805E1950;

/* brushes (inferred) */
void fn_80470474(void);               /* close */
RwBool fn_804704D4(void);             /* open */
Rt2dBrush *fn_8047054C(void);         /* create */
RwBool fn_80470620(Rt2dBrush *brush); /* destroy (releases a reference) */
Rt2dBrush *fn_80470694(Rt2dBrush *brush, RwRGBA *col0, RwRGBA *col1, RwRGBA *col2,
                       RwRGBA *col3); /* set the colours */
Rt2dBrush *fn_80470998(Rt2dBrush *brush, RwV2d *uv0, RwV2d *uv1, RwV2d *uv2,
                       RwV2d *uv3);                           /* set the texture coordinates */
Rt2dBrush *fn_80470A4C(Rt2dBrush *brush, RwTexture *texture); /* set the texture */

/* opening and closing (inferred) */
void fn_80474258(RwCamera *camera);      /* open */
void fn_804743AC(void);                  /* close */
RwCamera *fn_80473E9C(RwCamera *camera); /* the camera's end update */
RwBool fn_80474498(void);                /* opens the transformations */
void fn_804743F0(void);                  /* closes the transformations */
RwBool fn_80474228(RwReal depth);        /* sets the layer depth */
RwReal fn_80474168(void);                /* the squared pixel size */
RwBool fn_80473ECC(void);                /* resets the transformation */
RwBool fn_804772C4(void);
RwBool fn_8047721C(void);
void fn_804771E0(void);
void fn_8047726C(void);

/* the current transformation (inferred) */
RwBool fn_804747A0(void);               /* push */
RwBool fn_804748C8(void);               /* pop */
RwBool fn_804748FC(RwReal x, RwReal y); /* scale */
RwBool fn_80474964(RwReal x, RwReal y); /* translate */
RwMatrix *fn_804749CC(void);            /* the matrix */

/* the device's steps in the current transformation (inferred) */
RwBool fn_80473FD0(RwV2d *xStep, RwV2d *yStep, RwV2d *origin);

/* paths (inferred) */
Rt2dPath *fn_80476490(void);                               /* create */
RwBool fn_80475E2C(Rt2dPath *path);                        /* destroy */
Rt2dPath *fn_80476510(Rt2dPath *path);                     /* lock */
Rt2dPath *fn_80476830(Rt2dPath *path);                     /* unlock */
Rt2dPath *fn_80474C7C(Rt2dPath *path, RwReal x, RwReal y); /* moveto */
Rt2dPath *fn_80474BEC(Rt2dPath *path); /* destroys one subpath and returns the next */
Rt2dPath *fn_8047548C(void);           /* the emptied scratch path */
void fn_804754F4(Rt2dPath *path, RwV2d *bezier, RwInt32 depth); /* flattens a curve */
Rt2dPath *fn_804758C8(Rt2dPath *flat, Rt2dPath *path);          /* adds a flattened subpath */
void fn_804760C8(Rt2dPath *path);                               /* frees the points */
void fn_80476A0C(void);                                         /* closes the paths */
RwBool fn_80476A58(void);                                       /* opens the paths */
Rt2dPath *fn_80474EA0(Rt2dPath *path, RwReal x, RwReal y);      /* lineto */
Rt2dPath *fn_80474FF8(Rt2dPath *path, RwReal x1, RwReal y1, RwReal x2, RwReal y2, RwReal x3,
                      RwReal y3);      /* curveto */
Rt2dPath *fn_80475260(Rt2dPath *path); /* closepath */
/* drops the subpaths that hold a single segment, freeing them, and returns the rest, which may
 * be NULL (inferred) */
Rt2dPath *fn_80475B4C(Rt2dPath *path);
Rt2dPath *fn_80476C4C(RwStream *stream); /* stream read */

/* A path in a stream (0x10 bytes): its kind, points, flags and inset (inferred) */
typedef struct rt2dStreamPath {
    /* 0x0 */ RwInt32 type; /* 1 for points */
    /* 0x4 */ RwInt32 numPoints;
    /* 0x8 */ RwInt32 flags; /* closed, flat, another subpath follows */
    /* 0xC */ RwReal inset;
} rt2dStreamPath;
CHECK_SIZEOF(rt2dStreamPath, 0x10);

Rt2dPath *fn_80476ABC(Rt2dPath *path, rt2dStreamPath *header, RwStream *stream);
Rt2dPath *fn_804771C0(Rt2dPath *path, Rt2dBrush *brush); /* stroke */
Rt2dPath *fn_80476CF8(Rt2dPath *path, Rt2dBrush *brush); /* strokes each subpath */

/* fonts (inferred) */
Rt2dFont *fn_80470AC4(Rt2dFont *font, const RwChar *string, RwReal height, RwV2d *anchor,
                      Rt2dBrush *brush); /* shows outline characters */
Rt2dFont *fn_80470C54(Rt2dFont *font, const RwChar *string, RwReal height, RwV2d *anchor,
                      Rt2dBrush *brush);         /* shows texture characters */
Rt2dFont *fn_80472C14(RwStream *stream);         /* stream read */
void fn_80473118(void);                          /* close */
RwBool fn_80473260(void);                        /* open */
const RwChar *fn_80473460(const RwChar *path);   /* sets the path */
RwBool fn_80473528(Rt2dFontReadCallBack readCB); /* sets the read callback */
RwReal fn_8047353C(Rt2dFont *font);              /* the height on the device */
RwReal fn_80473584(Rt2dFont *font, const RwChar *string, RwReal height); /* string width */
RwBool fn_804736CC(Rt2dFont *font);                                      /* destroy */
Rt2dFont *fn_804737E4(const RwChar *name); /* reads a metrics file (the default read callback) */
Rt2dFont *fn_80473CE0(const RwChar *name); /* read */
Rt2dFont *fn_80473D10(Rt2dFont *font, const RwChar *string, RwReal height, RwV2d *anchor,
                      Rt2dBrush *brush);             /* show */
RwInt32 fn_80473D38(Rt2dFont *font, RwInt32 *codes); /* the codes of its characters */
void fn_80473E8C(RwTexDictionary *dict);             /* sets the texture dictionary */

extern RwInt32 lbl_805F1538; /* whether the fonts are open (inferred) */

#ifdef __cplusplus
}
#endif

#endif
