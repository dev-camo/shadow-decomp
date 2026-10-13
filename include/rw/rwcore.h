#ifndef RW_RWCORE_H
#define RW_RWCORE_H

#include <dolphin/types.h>

/* RenderWare Graphics 3.7 (stream chunk version 0x37002), linked into the game. Only what the
 * recovered RenderWare code uses is declared. The functions keep placeholder names; their roles
 * are inferred from their behavior and from the RenderWare stream format. */

#ifdef __cplusplus
extern "C" {
#endif

/* RenderWare's basic types */
typedef int RwInt32;
typedef unsigned int RwUInt32;
typedef short RwInt16;
typedef unsigned short RwUInt16;
typedef unsigned char RwUInt8;
typedef float RwReal;
typedef RwInt32 RwBool;

#define RW_STREAM_VERSION 0x37002
#define RW_STREAM_BUILD 0x37

/* chunk IDs */
#define rwID_STRUCT 0x1

/* The engine globals; only the memory functions are mapped. */
typedef struct RwGlobals {
    /* 0x000 */ RwUInt8 field_0x0[0x108];
    /* 0x108 */ void *(*rwmalloc)(RwUInt32 size, RwUInt32 hint);
    /* 0x10C */ void (*rwfree)(void *mem);
} RwGlobals;

extern RwGlobals *lbl_805F15B4; /* the engine instance */

#define RwMalloc(size, hint) (lbl_805F15B4->rwmalloc((size), (hint)))
#define RwFree(mem) (lbl_805F15B4->rwfree(mem))

/* plugin data at offset in an object (offsets are assigned at plugin registration) */
#define RWPLUGINOFFSET(type, base, offset) ((type *)((RwUInt8 *)(base) + (offset)))

typedef struct RwStream RwStream;

/* An error reported with fn_8047DB84: the reporting plugin and its error code. */
typedef struct RwError {
    RwInt32 pluginID;
    RwInt32 errorCode;
} RwError;

typedef struct RwV3d {
    RwReal x;
    RwReal y;
    RwReal z;
} RwV3d;

typedef struct RwBBox {
    RwV3d sup;
    RwV3d inf;
} RwBBox;

typedef struct RwLine {
    RwV3d start;
    RwV3d end;
} RwLine;

typedef struct RwSphere {
    RwV3d center;
    RwReal radius;
} RwSphere;

/* streams */
RwStream *fn_8047CEF0(RwStream *stream, RwInt32 type, RwInt32 size, RwUInt32 version,
                      RwUInt32 build);
RwInt32 fn_8047CF50(RwStream *stream, RwUInt32 type, RwUInt32 *lengthOut, RwUInt32 *versionOut);
void *fn_8047D158(void *mem, RwUInt32 size);
void *fn_8047D1C8(void *mem, RwUInt32 size);
void *fn_8047D254(void *mem, RwUInt32 size);
void *fn_8047D2C4(void *mem, RwUInt32 size);
RwStream *fn_8047D458(RwStream *stream, const RwInt32 *ints, RwUInt32 size);
RwStream *fn_8047D560(RwStream *stream, const RwInt16 *ints, RwUInt32 size);
RwStream *fn_8047D684(RwStream *stream, RwInt16 *ints, RwUInt32 size);
RwStream *fn_8047D784(RwStream *stream, RwReal *reals, RwUInt32 size);
RwStream *fn_8047D868(RwStream *stream, RwInt32 *ints, RwUInt32 size);
RwUInt32 fn_80480D4C(RwStream *stream, void *buffer, RwUInt32 length);
RwStream *fn_80480ED0(RwStream *stream, const void *buffer, RwUInt32 length);
RwStream *fn_804810FC(RwStream *stream, RwUInt32 offset);

/* maths and intersection tests (inferred from their use) */
RwReal fn_80482394(RwReal x); /* 1 / sqrt(x) */

/* whether a triangle meets a box */
RwBool fn_8047BDD0(const RwBBox *box, const RwV3d *v0, const RwV3d *v1, const RwV3d *v2);

/* whether a triangle meets a sphere, with the triangle's normal and the distance of the sphere's
 * center from its plane */
RwBool fn_8047B720(const RwSphere *sphere, const RwV3d *v0, const RwV3d *v1, const RwV3d *v2,
                   RwV3d *normal, RwReal *distance);

/* errors */
RwError *fn_8047DB84(const RwError *error);
RwInt32 fn_8047DC28(RwInt32 code, ...);

/* the engine and its plugins */
RwInt32 fn_80485D6C(void);
RwInt32 fn_80485D78(RwInt32 size, RwUInt32 pluginID, void *(*constructor)(void *, RwInt32, RwInt32),
                    void *(*destructor)(void *, RwInt32, RwInt32));

#ifdef __cplusplus
}
#endif

#endif
