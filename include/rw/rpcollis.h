#ifndef RW_RPCOLLIS_H
#define RW_RPCOLLIS_H

#include "rw/rwcore.h"

/* The RenderWare collision plugin (plugin ID 0x11D): a tree that splits the triangles of a
 * geometry or world sector along the axes, read and written as chunk 0x2C. The type and field
 * names are inferred from the stream format and the code that uses them. */

#ifdef __cplusplus
extern "C" {
#endif

#define rpID_COLLISPLUGIN 0x11D
#define rwID_COLLTREE 0x2C

/* rpCollTreeFlags */
#define rpCOLLTREE_TRIANGLEMAP 0x1 /* the tree has its own triangle index map */

/* One side of a split (8 bytes): either another split or a range of triangles (a leaf), with the
 * extent of that side along the split axis. */
typedef struct RpCollTreeBranch {
    /* 0x0 */ RwUInt8 field_0x0; /* bits 2-3 of the left branch's: the split axis (byte offset) */
    /* 0x1 */ RwUInt8 type;      /* 0xFF: index is a split, else the leaf's triangle count */
    /* 0x2 */ RwUInt16 index;    /* the split, or the leaf's first triangle */
    /* 0x4 */ RwReal value;
} RpCollTreeBranch;

/* One split of the tree (0x10 bytes). */
typedef struct RpCollTreeSplit {
    /* 0x0 */ RpCollTreeBranch left;
    /* 0x8 */ RpCollTreeBranch right;
} RpCollTreeSplit;

typedef struct RpCollTree {
    /* 0x00 */ RwUInt32 flags; /* rpCOLLTREE_* */
    /* 0x04 */ RwBBox bbox;
    /* 0x1C */ RwUInt16 numTriangles;
    /* 0x1E */ RwUInt16 numSplits;
    /* 0x20 */ RpCollTreeSplit *splits;
    /* 0x24 */ RwUInt16 *map;
} RpCollTree;

/* A triangle met by an intersection test, as passed to the callback. */
typedef struct RpCollisionTriangle {
    /* 0x00 */ RwV3d normal; /* unit length */
    /* 0x0C */ RwV3d point;  /* the first vertex */
    /* 0x18 */ RwInt32 index;
    /* 0x1C */ RwV3d *vertices[3];
} RpCollisionTriangle;

/* the plugin's registration with the engine, atomics, world sectors and geometries */
extern RwInt32 lbl_805F1410; /* open count */
extern RwInt32 lbl_805F1414; /* engine plugin offset */
extern RwInt32 lbl_805F1418; /* atomic plugin offset */
extern RwInt32 lbl_805F141C; /* geometry plugin offset */
extern RwInt32 lbl_805F1420; /* world sector plugin offset (the intersection tests read it there) */

#ifdef __cplusplus
}
#endif

#endif
