#ifndef GAME_RWCLUMPVIEW_H
#define GAME_RWCLUMPVIEW_H

#include <dolphin/types.h>
#include "layout_check.h"

/* The few RenderWare objects that game C++ touches, as the game code sees them. include/rw/ holds
 * the full C declarations, but those headers include the C library headers by their C names and
 * so cannot be included from the game C++ units (which search MSL's headers). The records mirror
 * the layouts of the RenderWare types they stand for. fn_804856C8 and fn_804875B0 are declared in
 * include/rw/rwcore.h; fn_80456BC4 is defined in src/rw/fn_80455D0C.c. */

/* A RenderWare frame (the object a clump hangs from); not mapped. */
struct UnkRwFrame;

/* A camera; not mapped. */
struct UnkRwCamera;

/* A position (RwV3d); plain, so that a local one is not constructed. */
struct UnkRwV3d {
    /* 0x0 */ float x;
    /* 0x4 */ float y;
    /* 0x8 */ float z;
};
CHECK_SIZEOF(UnkRwV3d, 0xC);

/* A sphere (centre and radius), RwSphere. */
struct UnkRwSphere {
    /* 0x0 */ UnkRwV3d center;
    /* 0xC */ float radius;
};
CHECK_SIZEOF(UnkRwSphere, 0x10);

/* A clump (RpClump); only its object header, whose parent is the clump's frame, is mapped. */
struct UnkRwClump {
    /* 0x0 */ u8 type;
    /* 0x1 */ u8 subType;
    /* 0x2 */ u8 flags;
    /* 0x3 */ u8 privateFlags;
    /* 0x4 */ UnkRwFrame *frame;
    /* 0x8 */ u8 field_0x8[0x2C - 0x8];
};
CHECK_SIZEOF(UnkRwClump, 0x2C);

/* The engine globals, RwGlobals; only the current camera is mapped. include/rw/rwcore.h declares
 * lbl_805F15B4 with the full RwGlobals type; this is a second, deliberately partial view of the
 * same object, so the two headers must not be included together. The object's size is not mapped,
 * so there is no size check. */
struct UnkRwGlobals {
    /* 0x0 */ UnkRwCamera *curCamera;
};

extern UnkRwGlobals *lbl_805F15B4;

/* How a frame operation combines with the frame's current matrix: replace it, or apply the new
 * transformation after it. The values are the same as in include/rw/rwcore.h. */
#define rwCOMBINEREPLACE 0
#define rwCOMBINEPOSTCONCAT 2

extern "C" {
/* Whether a sphere is in a camera's view frustum (inferred; zero when it is outside). */
int fn_804856C8(const UnkRwCamera *camera, const UnkRwSphere *sphere);
/* Translates the frame by translation with the combine operation (inferred). */
UnkRwFrame *fn_804875B0(UnkRwFrame *frame, const UnkRwV3d *translation, int combine);
/* Renders the atomics of the clump that are flagged for rendering (inferred). */
UnkRwClump *fn_80456BC4(UnkRwClump *clump);
}

#endif
