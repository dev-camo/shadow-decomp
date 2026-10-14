#ifndef RW_RT2D_H
#define RW_RT2D_H

#include "rw/rwcore.h"

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

/* The brush flags */
#define FIELDRGBA 0x1 /* the colours are not uniform */
#define FIELDUV 0x2   /* the brush is textured or its texture coordinates vary */

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

/* The 2D toolkit's globals (0xD58 bytes); only the mapped members are used */
typedef struct Rt2dGlobalVars {
    /* 0x000 */ RwUInt8 field_0x0[0x140];
    /* 0x140 */ RwFreeList *brushFreeList;
    /* 0x144 */ void *vertexBuffer; /* 0x900 bytes of vertices (inferred) */
    /* 0x148 */ RwUInt8 field_0x148[0xC10];
} Rt2dGlobalVars;

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

#ifdef __cplusplus
}
#endif

#endif
