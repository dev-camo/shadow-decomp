/* RenderWare collision plugin (plugin ID 0x11D): collision trees of geometries and world sectors,
 * their stream format (chunk 0x2C, and the older format that came before version 0x36001), the
 * plugin registration and the intersection tests of a world's triangles, which walk the world's
 * sector tree and then the collision trees of its sectors. The original file name is unknown. */
#include "rw/rpcollis.h"
#include "rw/rpworld.h"
#include "rw/rwmath.h"
#include "layout_check.h"

#define rwMEMHINT_COLLTREE (rwMEMHINTDUR_EVENT | rwID_COLLTREE)

/* An intersection primitive (0x1C bytes). */
typedef struct RpIntersection {
    /* 0x00 */ union {
        RwLine line;
        RwSphere sphere;
        RwBBox box;
        void *object;
    } t;
    /* 0x18 */ RwInt32 type; /* rpINTERSECT* */
} RpIntersection;
CHECK_SIZEOF(RpIntersection, 0x1C);

#define rpINTERSECTLINE 1
#define rpINTERSECTSPHERE 3
#define rpINTERSECTBOX 4
#define rpINTERSECTATOMIC 5

typedef RpCollisionTriangle *(*RpIntersectionCallBackWorldTriangle)(RpIntersection *intersection,
                                                                    RpWorldSector *sector,
                                                                    RpCollisionTriangle *triangle,
                                                                    RwReal distance, void *data);

/* the layout of the tree's STRUCT chunk */
typedef struct RpCollTreeStreamHeader {
    RwUInt32 flags;
    RwV3d inf;
    RwV3d sup;
    RwUInt32 numTriangles;
    RwUInt32 numSplits;
} RpCollTreeStreamHeader;
CHECK_SIZEOF(RpCollTreeStreamHeader, 0x24);

/* the plugin's offsets and open count, defined from the highest address down (the compiler places
 * the last definition first) */
RwInt32 lbl_805F1420;
RwInt32 lbl_805F141C;
RwInt32 lbl_805F1418;
RwInt32 lbl_805F1414;
RwInt32 lbl_805F1410;

/* Allocates a tree with room for numSplits splits and, with rpCOLLTREE_TRIANGLEMAP, a triangle map
 * after it */
static inline RpCollTree *CollTreeCreate(RwUInt16 numTriangles, RwUInt16 numSplits, RwUInt32 flags,
                                         const RwBBox *bbox) {
    RpCollTree *tree;
    RwUInt32 size = sizeof(RpCollTree);
    RwUInt8 *mem;

    if (numSplits != 0) {
        size += numSplits * sizeof(RpCollTreeSplit) + 15;
    }
    if (flags & rpCOLLTREE_TRIANGLEMAP) {
        size += numTriangles * sizeof(RwUInt16) + 1;
    }
    mem = RwMalloc(size, rwMEMHINT_COLLTREE);
    if (mem == NULL) {
        RWERRORPLUGIN(rpID_COLLISPLUGIN, (E_RW_NOMEM, size));
        return NULL;
    }
    tree = (RpCollTree *)mem;
    tree->flags = flags;
    tree->numTriangles = numTriangles;
    tree->numSplits = numSplits;
    tree->bbox = *bbox;
    mem += sizeof(RpCollTree);
    if (numSplits != 0) {
        mem = (RwUInt8 *)(((RwUInt32)mem + 15) & ~15);
        tree->splits = (RpCollTreeSplit *)mem;
        mem += numSplits * sizeof(RpCollTreeSplit);
    } else {
        tree->splits = NULL;
    }
    if (flags & rpCOLLTREE_TRIANGLEMAP) {
        tree->map = (RwUInt16 *)(((RwUInt32)mem + 1) & ~1);
    } else {
        tree->map = NULL;
    }
    return tree;
}

/* Frees a tree */
void fn_80440410(RpCollTree *tree) {
    RwFree(tree);
}

/* The stream size of a tree, chunk headers included */
static inline RwInt32 CollTreeStreamGetSize(const RpCollTree *tree) {
    RwInt32 size = tree->numSplits * sizeof(RpCollTreeSplit) + 0x30;

    if (tree->flags & rpCOLLTREE_TRIANGLEMAP) {
        size += tree->numTriangles * sizeof(RwUInt16);
    }
    return size;
}

RwInt32 fn_8044043C(const RpCollTree *tree) {
    return CollTreeStreamGetSize(tree);
}

/* Writes a tree */
const RpCollTree *fn_80440468(const RpCollTree *tree, RwStream *stream) {
    RpCollTreeStreamHeader header;
    RwInt32 size;
    RwUInt32 i;

    size = CollTreeStreamGetSize(tree);
    if (fn_8047CEF0(stream, rwID_COLLTREE, size, RW_STREAM_VERSION, RW_STREAM_BUILD) == NULL) {
        return NULL;
    }
    if (fn_8047CEF0(stream, rwID_STRUCT, size - 12, RW_STREAM_VERSION, RW_STREAM_BUILD) == NULL) {
        return NULL;
    }
    header.inf = tree->bbox.inf;
    header.sup = tree->bbox.sup;
    header.flags = tree->flags;
    header.numTriangles = tree->numTriangles;
    header.numSplits = tree->numSplits;
    fn_8047D158(&header, sizeof(header));
    if (fn_80480ED0(stream, &header, sizeof(header)) == NULL) {
        return NULL;
    }
    if (tree->numSplits != 0) {
        for (i = 0; i < tree->numSplits; i++) {
            RpCollTreeSplit split = tree->splits[i];

            fn_8047D1C8(&split.left.index, sizeof(RwUInt16));
            fn_8047D1C8(&split.right.index, sizeof(RwUInt16));
            fn_8047D158(&split.left.value, sizeof(RwReal));
            fn_8047D158(&split.right.value, sizeof(RwReal));
            if (fn_80480ED0(stream, &split, sizeof(split)) == NULL) {
                return NULL;
            }
        }
    }
    if (tree->flags & rpCOLLTREE_TRIANGLEMAP) {
        if (fn_8047D560(stream, (RwInt16 *)tree->map, tree->numTriangles * sizeof(RwUInt16)) ==
            NULL) {
            return NULL;
        }
    }
    return tree;
}

/* Reads a tree */
RpCollTree *fn_80440670(RwStream *stream) {
    RpCollTreeStreamHeader header;
    RwBBox bbox;
    RpCollTree *tree;
    RwUInt32 version;

    if (!fn_8047CF50(stream, rwID_STRUCT, NULL, &version)) {
        return NULL;
    }
    if (fn_80480D4C(stream, &header, sizeof(header)) != sizeof(header)) {
        return NULL;
    }
    fn_8047D254(&header, sizeof(header));
    bbox.inf = header.inf;
    bbox.sup = header.sup;
    tree = CollTreeCreate(header.numTriangles, header.numSplits, header.flags, &bbox);
    if (tree == NULL) {
        return NULL;
    }
    if (tree->numSplits != 0) {
        RwUInt32 length = tree->numSplits * sizeof(RpCollTreeSplit);
        RwUInt32 i;
        RpCollTreeSplit *split;

        if (length != fn_80480D4C(stream, tree->splits, length)) {
            RwFree(tree);
            return NULL;
        }
        split = tree->splits;
        for (i = 0; i < tree->numSplits; i++, split++) {
            fn_8047D2C4(&split->left.index, sizeof(RwUInt16));
            fn_8047D2C4(&split->right.index, sizeof(RwUInt16));
            fn_8047D254(&split->left.value, sizeof(RwReal));
            fn_8047D254(&split->right.value, sizeof(RwReal));
        }
    }
    if (tree->flags & rpCOLLTREE_TRIANGLEMAP) {
        if (fn_8047D684(stream, (RwInt16 *)tree->map, tree->numTriangles * sizeof(RwUInt16)) ==
            NULL) {
            RwFree(tree);
            return NULL;
        }
    }
    return tree;
}

/* Reads a tree in the format older than version 0x36001: each split is two words of branch types
 * and indices and the two extents, and the leaves are rebuilt by walking the tree. *failed is set
 * when a leaf is too large for this format. */
RpCollTree *fn_80440920(RwStream *stream, RwInt32 numTriangles, RwInt32 numSplits, RwBBox *bbox,
                        RwInt32 *failed) {
    RpCollTreeBranch *stack[32];
    RpCollTreeBranch root;
    RpCollTree *tree;
    RpCollTreeSplit *split;
    RpCollTreeBranch *branch;
    RwUInt32 i;
    RwUInt32 depth;
    RwUInt32 triIndex;

    *failed = 0;
    tree = CollTreeCreate(numTriangles, numSplits, rpCOLLTREE_TRIANGLEMAP, bbox);
    if (tree == NULL) {
        return NULL;
    }
    if (tree->numSplits != 0) {
        split = tree->splits;
        for (i = 0; i < tree->numSplits; i++, split++) {
            RwUInt32 types;
            RwInt32 indices;

            if (fn_8047D868(stream, (RwInt32 *)&types, sizeof(types)) == NULL ||
                fn_8047D868(stream, &indices, sizeof(indices)) == NULL ||
                fn_8047D784(stream, &split->left.value, sizeof(RwReal)) == NULL ||
                fn_8047D784(stream, &split->right.value, sizeof(RwReal)) == NULL) {
                RwFree(tree);
                return NULL;
            }
            split->right.field_0x0 = types >> 16;
            split->left.field_0x0 = split->right.field_0x0 | 1;
            split->right.index = indices;
            split->left.index = (RwUInt32)indices >> 16;
            /* a branch of type 2 is another split; the leaves are read below */
            if (((types >> 8) & 0xFF) == 2) {
                split->left.type = 0xFF;
            } else {
                split->left.type = 0;
            }
            if ((types & 0xFF) == 2) {
                split->right.type = 0xFF;
            } else {
                split->right.type = 0;
            }
        }

        /* the leaves follow in tree order: give each its triangle count and first index */
        root.type = 0xFF;
        root.index = 0;
        branch = &root;
        depth = 1;
        while (depth != 0) {
            if (branch->type != 0xFF) {
                RwInt32 leaf;

                if (fn_8047D868(stream, &leaf, sizeof(leaf)) == NULL) {
                    RwFree(tree);
                    return NULL;
                }
                if ((RwUInt32)leaf >> 16 > 0xEF) {
                    *failed = 1;
                    RwFree(tree);
                    RWERRORPLUGIN(rpID_COLLISPLUGIN, (1));
                    return NULL;
                }
                branch->type = (RwUInt32)leaf >> 16;
                depth--;
                branch->index = leaf;
                branch = stack[depth];
            } else {
                RpCollTreeSplit *split = &tree->splits[branch->index];

                stack[depth++] = &split->right;
                branch = &split->left;
            }
        }
    } else {
        if (fn_804810FC(stream, sizeof(RwInt32)) == NULL) {
            RwFree(tree);
            return NULL;
        }
    }
    if (tree->flags & rpCOLLTREE_TRIANGLEMAP) {
        for (triIndex = 0; triIndex < tree->numTriangles; triIndex++) {
            RwInt32 index;

            if (fn_8047D868(stream, &index, sizeof(index)) == NULL) {
                RwFree(tree);
                return NULL;
            }
            tree->map[triIndex] = index;
        }
    }
    return tree;
}

/* engine plugin constructor */
void *fn_80440D28(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F1410++;
    *RWPLUGINOFFSET(void *, lbl_805F15B4, lbl_805F1414) = NULL;
    return instance;
}

/* engine plugin destructor */
void *fn_80440D48(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F1410--;
    return instance;
}

/* object plugin destructor: frees the object's tree */
void *fn_80440D58(void *object, RwInt32 offset, RwInt32 size) {
    if (*RWPLUGINOFFSET(RpCollTree *, object, offset) != NULL) {
        fn_80440410(*RWPLUGINOFFSET(RpCollTree *, object, offset));
        *RWPLUGINOFFSET(RpCollTree *, object, offset) = NULL;
    }
    return object;
}

/* object plugin constructor */
void *fn_80440DA8(void *object, RwInt32 offset, RwInt32 size) {
    *RWPLUGINOFFSET(RpCollTree *, object, offset) = NULL;
    return object;
}

/* object plugin stream size */
RwInt32 fn_80440DB4(const void *object, RwInt32 offset, RwInt32 size) {
    RpCollTree *tree = *RWPLUGINOFFSET(RpCollTree *, object, offset);

    if (tree != NULL) {
        return fn_8044043C(tree) + 0x10;
    }
    return 0;
}

/* object plugin stream write: the engine version, then the tree */
RwStream *fn_80440DEC(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {
    if (*RWPLUGINOFFSET(RpCollTree *, object, offset) != NULL) {
        RwInt32 version = fn_80485D6C();

        if (fn_8047D458(stream, &version, sizeof(version)) == NULL) {
            return NULL;
        }
        if (fn_80440468(*RWPLUGINOFFSET(RpCollTree *, object, offset), stream) == NULL) {
            return NULL;
        }
    }
    return stream;
}

/* object plugin stream read: a version (new format) or a split count (old format) comes first */
RwStream *fn_80440E80(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    RwUInt32 first;

    if (fn_8047D868(stream, (RwInt32 *)&first, sizeof(first)) == NULL) {
        return NULL;
    }
    if (first < 0x36001) {
        RwBBox bbox;
        RwInt32 numTriangles;
        RwInt32 failed;

        if (fn_8047D868(stream, &numTriangles, sizeof(numTriangles)) == NULL) {
            return NULL;
        }
        if (((RpGeometry *)object)->object.type == rpGEOMETRY) {
            RwSphere *sphere = &((RpGeometry *)object)->morphTarget->boundingSphere;

            bbox.inf.x = sphere->center.x - sphere->radius;
            bbox.inf.y = sphere->center.y - sphere->radius;
            bbox.inf.z = sphere->center.z - sphere->radius;
            bbox.sup.x = sphere->center.x + sphere->radius;
            bbox.sup.y = sphere->center.y + sphere->radius;
            bbox.sup.z = sphere->center.z + sphere->radius;
        } else {
            bbox = ((RpWorldSector *)object)->boundingBox;
        }
        failed = 0;
        *RWPLUGINOFFSET(RpCollTree *, object, offset) =
            fn_80440920(stream, numTriangles, first - 1, &bbox, &failed);
        if (failed != 0) {
            return stream;
        }
    } else {
        if (!fn_8047CF50(stream, rwID_COLLTREE, NULL, NULL)) {
            return NULL;
        }
        *RWPLUGINOFFSET(RpCollTree *, object, offset) = fn_80440670(stream);
    }
    if (*RWPLUGINOFFSET(RpCollTree *, object, offset) != NULL) {
        return stream;
    }
    return NULL;
}

/* atomic plugin constructor */
void *fn_80441018(void *object, RwInt32 offset, RwInt32 size) {
    *RWPLUGINOFFSET(void *, object, lbl_805F1418) = NULL;
    return object;
}

/* Registers the plugin with the engine, atomics, world sectors and geometries */
RwInt32 fn_80441028(void) {
    RwInt32 result;
    RwInt32 ok;
    RwInt32 attached;

    lbl_805F1414 = fn_80485D78(sizeof(void *), rpID_COLLISPLUGIN, fn_80440D28, fn_80440D48);
    result = lbl_805F1414 > 0;
    if (result) {
        lbl_805F1418 = fn_80458B6C(sizeof(void *), rpID_COLLISPLUGIN, fn_80441018, NULL, NULL);
        ok = lbl_805F1418 > 0;
        if (ok) {
            lbl_805F1420 =
                fn_8045F2F8(sizeof(void *), rpID_COLLISPLUGIN, fn_80440DA8, fn_80440D58, NULL);
            ok = lbl_805F1420 >= 0;
            if (ok) {
                ok = fn_8045F338(rpID_COLLISPLUGIN, fn_80440E80, fn_80440DEC, fn_80440DB4) >= 0;
            }
            ok = ok != 0;
        }
        attached = ok != 0;
        if (attached) {
            lbl_805F141C =
                fn_8045994C(sizeof(void *), rpID_COLLISPLUGIN, fn_80440DA8, fn_80440D58, NULL);
            ok = lbl_805F141C >= 0;
            if (ok) {
                ok = fn_8045998C(rpID_COLLISPLUGIN, fn_80440E80, fn_80440DEC, fn_80440DB4) >= 0;
            }
            attached = ok != 0;
        }
        result = attached;
    }
    return result;
}

/* A component of a vector chosen by an axis stored as a byte offset (0, 4 or 8), the form the sector
 * and collision trees keep their split axes in */
#define RwV3dGetCoord(vector, offset) (*(RwReal *)((RwUInt8 *)&(vector) + (offset)))

#define RwRealMax2(a, b) ((a) >= (b) ? (a) : (b))

/* A line in the form the line walks use (inferred): its direction and the reciprocals of the
 * direction's components (0 for a zero component), and a per-axis tolerance for the split tests. */
typedef struct CollLine {
    /* 0x00 */ RwV3d start;
    /* 0x0C */ RwV3d delta;
    /* 0x18 */ RwV3d recip;
    /* 0x24 */ RwV3d pad;
} CollLine;
CHECK_SIZEOF(CollLine, 0x30);

/* A pending part of the line walk of a world's sector tree: a sector and the part of the line,
 * between parameters t0 and t1, that may meet it */
typedef struct CollWorldLineEntry {
    /* 0x0 */ RpSector *sector;
    /* 0x4 */ RwReal t0;
    /* 0x8 */ RwReal t1;
} CollWorldLineEntry;
CHECK_SIZEOF(CollWorldLineEntry, 0xC);

typedef struct CollWorldLineIt {
    /* 0x000 */ CollLine line;
    /* 0x030 */ CollWorldLineEntry stack[65];
    /* 0x33C */ CollWorldLineEntry *top; /* the current entry; those below it are pending */
} CollWorldLineIt;
CHECK_SIZEOF(CollWorldLineIt, 0x340);

/* The same for a collision tree */
typedef struct CollTreeLineEntry {
    /* 0x0 */ RpCollTreeBranch *branch;
    /* 0x4 */ RwReal t0;
    /* 0x8 */ RwReal t1;
} CollTreeLineEntry;
CHECK_SIZEOF(CollTreeLineEntry, 0xC);

typedef struct CollTreeLineIt {
    /* 0x000 */ RpCollTree *tree;
    /* 0x004 */ CollLine line;
    /* 0x034 */ RpCollTreeBranch root; /* the whole tree */
    /* 0x03C */ CollTreeLineEntry stack[33];
    /* 0x1C8 */ CollTreeLineEntry *top;
} CollTreeLineIt;
CHECK_SIZEOF(CollTreeLineIt, 0x1CC);

/* Moves to the next world sector the line may meet, pushing the far side of each split the line
 * crosses; call while there are entries left */
static inline void CollWorldLineItNext(CollWorldLineIt *it) {
    it->top--;
    while (it->top->sector->type >= 0) {
        RpPlaneSector *plane = (RpPlaneSector *)it->top->sector;
        /* the line's coordinates along the split axis at the walk's two parameters, and the
         * split's two extents widened by the tolerance */
        RwReal leftHi;
        RwReal rightLo;
        RwReal pad;
        RwReal p0;
        RwReal p1;
        RwReal lineDelta;
        RwReal lineStart;

        lineStart = RwV3dGetCoord(it->line.start, plane->type);
        lineDelta = RwV3dGetCoord(it->line.delta, plane->type);
        p1 = it->top->t1 * lineDelta + lineStart;
        p0 = it->top->t0 * lineDelta + lineStart;
        pad = RwV3dGetCoord(it->line.pad, plane->type);
        rightLo = plane->rightValue - pad;
        leftHi = plane->leftValue + pad;

        if (lineDelta > 0.0f) {
            if (rightLo > p1) {
                it->top->sector = plane->leftSubTree;
            } else if (leftHi < p0) {
                it->top->sector = plane->rightSubTree;
            } else {
                RwReal recip;

                it->top++;
                recip = RwV3dGetCoord(it->line.recip, plane->type);
                it->top->sector = plane->leftSubTree;
                it->top->t0 = it->top[-1].t0;
                it->top->t1 = it->top[-1].t1;
                if (leftHi < p1) {
                    it->top->t1 += recip * (leftHi - p1);
                }
                it->top[-1].sector = plane->rightSubTree;
                if (rightLo > p0) {
                    it->top[-1].t0 += recip * (rightLo - p0);
                }
            }
        } else {
            if (leftHi < p1) {
                it->top->sector = plane->rightSubTree;
            } else if (rightLo > p0) {
                it->top->sector = plane->leftSubTree;
            } else {
                RwReal recip;

                it->top++;
                recip = RwV3dGetCoord(it->line.recip, plane->type);
                it->top->sector = plane->rightSubTree;
                it->top->t0 = it->top[-1].t0;
                it->top->t1 = it->top[-1].t1;
                if (rightLo > p1) {
                    it->top->t1 += recip * (rightLo - p1);
                }
                it->top[-1].sector = plane->leftSubTree;
                if (leftHi < p0) {
                    it->top[-1].t0 += recip * (leftHi - p0);
                }
            }
        }
    }
}

/* Calls callback for the triangles of a world that a line meets, with the distance along the line
 * (0 at its start, 1 at its end); stops when callback returns NULL */
void fn_80441188(RpWorld *world, RpIntersection *intersection,
                 RpIntersectionCallBackWorldTriangle callback, void *data) {
    CollWorldLineIt worldIt;
    CollTreeLineIt treeIt;
    RpCollisionTriangle collTri;

    worldIt.line.start = intersection->t.line.start;
    worldIt.line.delta.x = intersection->t.line.end.x - intersection->t.line.start.x;
    worldIt.line.delta.y = intersection->t.line.end.y - intersection->t.line.start.y;
    worldIt.line.delta.z = intersection->t.line.end.z - intersection->t.line.start.z;
    worldIt.line.recip.x = (0.0f != worldIt.line.delta.x) ? 1.0f / worldIt.line.delta.x : 0.0f;
    worldIt.line.recip.y = (0.0f != worldIt.line.delta.y) ? 1.0f / worldIt.line.delta.y : 0.0f;
    worldIt.line.recip.z = (0.0f != worldIt.line.delta.z) ? 1.0f / worldIt.line.delta.z : 0.0f;
    worldIt.line.pad.x =
        RwRealMax2(RwRealAbs(intersection->t.line.start.x), RwRealAbs(intersection->t.line.end.x));
    worldIt.line.pad.y =
        RwRealMax2(RwRealAbs(intersection->t.line.start.y), RwRealAbs(intersection->t.line.end.y));
    worldIt.line.pad.z =
        RwRealMax2(RwRealAbs(intersection->t.line.start.z), RwRealAbs(intersection->t.line.end.z));
    worldIt.line.pad.x = 1e-6f * worldIt.line.pad.x;
    worldIt.line.pad.y = 1e-6f * worldIt.line.pad.y;
    worldIt.line.pad.z = 1e-6f * worldIt.line.pad.z;

    worldIt.stack[0].sector = world->rootSector;
    worldIt.stack[0].t0 = 0.0f;
    worldIt.stack[0].t1 = 1.0f;
    worldIt.top = &worldIt.stack[1];

    while (worldIt.top > worldIt.stack) {
        RpWorldSector *sector;
        RpCollTree *tree;
        RpTriangle *triangles;
        RwV3d *vertices;

        CollWorldLineItNext(&worldIt);
        sector = (RpWorldSector *)worldIt.top->sector;
        tree = *RWPLUGINOFFSET(RpCollTree *, sector, lbl_805F1420);
        if (tree == NULL) {
            continue;
        }
        triangles = sector->triangles;
        vertices = sector->vertices;

        treeIt.tree = tree;
        treeIt.root.index = 0;
        treeIt.root.type = 0xFF;
        if (tree->numSplits == 0) {
            treeIt.root.type = tree->numTriangles;
        }
        treeIt.line.start = worldIt.line.start;
        treeIt.line.delta = worldIt.line.delta;
        treeIt.line.recip = worldIt.line.recip;
        treeIt.line.pad = worldIt.line.pad;
        treeIt.stack[0].branch = &treeIt.root;
        treeIt.stack[0].t0 = worldIt.top->t0;
        treeIt.stack[0].t1 = worldIt.top->t1;
        treeIt.top = &treeIt.stack[1];

        while (treeIt.top > treeIt.stack) {
            RpCollTreeBranch *branch;
            RwInt32 axis;
            CollTreeLineEntry *curr; /* the entry being descended */
            RwUInt16 i;
            RwUInt16 start;
            RwUInt8 count;

            treeIt.top--;

            /* descend the same way to the leaves of the sector's tree */
            while ((branch = (curr = treeIt.top)->branch)->type == 0xFF) {
                RpCollTreeSplit *split;
                RwReal lineStart;
                RwReal lineDelta;
                RwReal p1;
                RwReal p0;
                RwReal pad;
                RwReal rightLo;
                RwReal leftHi;
                RwReal recip;

                split = &treeIt.tree->splits[branch->index];
                axis = split->left.field_0x0 & 0xC;
                lineStart = RwV3dGetCoord(treeIt.line.start, axis);
                lineDelta = RwV3dGetCoord(treeIt.line.delta, axis);
                p1 = curr->t1 * lineDelta + lineStart;
                p0 = curr->t0 * lineDelta + lineStart;
                pad = RwV3dGetCoord(treeIt.line.pad, axis);
                rightLo = split->right.value - pad;
                leftHi = split->left.value + pad;

                if (lineDelta > 0.0f) {
                    if (rightLo > p1) {
                        curr->branch = &split->left;
                    } else if (leftHi < p0) {
                        curr->branch = &split->right;
                    } else {
                        treeIt.top++;
                        recip = RwV3dGetCoord(treeIt.line.recip, axis);
                        treeIt.top->branch = &split->left;
                        treeIt.top->t0 = treeIt.top[-1].t0;
                        treeIt.top->t1 = treeIt.top[-1].t1;
                        if (leftHi < p1) {
                            treeIt.top->t1 += recip * (leftHi - p1);
                        }
                        treeIt.top[-1].branch = &split->right;
                        if (rightLo > p0) {
                            treeIt.top[-1].t0 += recip * (rightLo - p0);
                        }
                    }
                } else {
                    if (leftHi < p1) {
                        curr->branch = &split->right;
                    } else if (rightLo > p0) {
                        curr->branch = &split->left;
                    } else {
                        treeIt.top++;
                        recip = RwV3dGetCoord(treeIt.line.recip, axis);
                        treeIt.top->branch = &split->right;
                        treeIt.top->t0 = treeIt.top[-1].t0;
                        treeIt.top->t1 = treeIt.top[-1].t1;
                        if (rightLo > p1) {
                            treeIt.top->t1 += recip * (rightLo - p1);
                        }
                        treeIt.top[-1].branch = &split->left;
                        if (leftHi < p0) {
                            treeIt.top[-1].t0 += recip * (leftHi - p0);
                        }
                    }
                }
            }

            /* test the leaf's triangles */
            start = branch->index;
            count = branch->type;
            for (i = start; i < start + count; i++) {
                RwInt32 triIndex;
                RpTriangle *triangle;
                RwV3d *v0;
                RwV3d *v1;
                RwV3d *v2;
                RwV3d edge1;
                RwV3d edge2;
                RwV3d tVec;
                RwV3d pVec;
                RwV3d qVec;
                RwReal det;
                RwReal distance;
                RwBool result;

                triIndex = (tree->map != NULL) ? tree->map[i] : i;
                triangle = &triangles[triIndex];
                v0 = &vertices[triangle->vertIndex[0]];
                v1 = &vertices[triangle->vertIndex[1]];
                v2 = &vertices[triangle->vertIndex[2]];

                /* the line-triangle test of RenderWare's intersection toolkit */
                edge1.x = v1->x - v0->x;
                edge1.y = v1->y - v0->y;
                edge1.z = v1->z - v0->z;
                edge2.x = v2->x - v0->x;
                edge2.y = v2->y - v0->y;
                edge2.z = v2->z - v0->z;
                pVec.x = worldIt.line.delta.y * edge2.z - worldIt.line.delta.z * edge2.y;
                pVec.y = worldIt.line.delta.z * edge2.x - worldIt.line.delta.x * edge2.z;
                pVec.z = worldIt.line.delta.x * edge2.y - worldIt.line.delta.y * edge2.x;
                det = edge1.x * pVec.x + edge1.y * pVec.y + edge1.z * pVec.z;
                result = det > 1e-8f;
                if (result) {
                    RwReal lo;
                    RwReal hi;
                    RwReal u;

                    lo = -det * 1e-5f;
                    hi = det - lo;
                    tVec.x = worldIt.line.start.x - v0->x;
                    tVec.y = worldIt.line.start.y - v0->y;
                    tVec.z = worldIt.line.start.z - v0->z;
                    u = tVec.x * pVec.x + tVec.y * pVec.y + tVec.z * pVec.z;
                    result = (u >= lo) && (u <= hi);
                    if (result) {
                        RwReal v;

                        qVec.x = tVec.y * edge1.z - tVec.z * edge1.y;
                        qVec.y = tVec.z * edge1.x - tVec.x * edge1.z;
                        qVec.z = tVec.x * edge1.y - tVec.y * edge1.x;
                        v = worldIt.line.delta.x * qVec.x + worldIt.line.delta.y * qVec.y +
                            worldIt.line.delta.z * qVec.z;
                        result = (v >= lo) && (u + v <= hi);
                        if (result) {
                            distance = edge2.x * qVec.x + edge2.y * qVec.y + edge2.z * qVec.z;
                            result = (distance >= lo) && (distance <= hi);
                            if (result) {
                                distance /= det;
                            }
                        }
                    }
                }

                if (result) {
                    RwV3d a;
                    RwV3d b;
                    RwReal recipLength;

                    collTri.point = *v0;
                    collTri.index = triIndex;
                    collTri.vertices[0] = v0;
                    collTri.vertices[1] = v1;
                    collTri.vertices[2] = v2;
                    a.x = v1->x - v0->x;
                    a.y = v1->y - v0->y;
                    a.z = v1->z - v0->z;
                    b.x = v2->x - v0->x;
                    b.y = v2->y - v0->y;
                    b.z = v2->z - v0->z;
                    collTri.normal.x = a.y * b.z - a.z * b.y;
                    collTri.normal.y = a.z * b.x - a.x * b.z;
                    collTri.normal.z = a.x * b.y - a.y * b.x;
                    recipLength = fn_80482394(collTri.normal.x * collTri.normal.x +
                                              collTri.normal.y * collTri.normal.y +
                                              collTri.normal.z * collTri.normal.z);
                    collTri.normal.x *= recipLength;
                    collTri.normal.y *= recipLength;
                    collTri.normal.z *= recipLength;
                    if (callback(intersection, sector, &collTri, distance, data) == NULL) {
                        return;
                    }
                }
            }
        }
    }
}

/* A box walk of a world's sector tree (inferred): the sectors still to visit */
typedef struct CollWorldBoxIt {
    /* 0x000 */ RwBBox box;
    /* 0x018 */ RpSector *stack[65];
    /* 0x11C */ RpSector **top; /* the current entry; those below it are pending */
} CollWorldBoxIt;
CHECK_SIZEOF(CollWorldBoxIt, 0x120);

/* A pending branch of a box walk of a collision tree, with the faces of the clip box that may cut
 * its triangles (bit 2 * axis: the low face, the next bit: the high face) */
typedef struct CollTreeBoxEntry {
    /* 0x0 */ RpCollTreeBranch *branch;
    /* 0x4 */ RwUInt32 clipFlags;
} CollTreeBoxEntry;
CHECK_SIZEOF(CollTreeBoxEntry, 0x8);

typedef struct CollTreeBoxIt {
    /* 0x000 */ RpCollTree *tree;
    /* 0x004 */ RwBBox box;     /* the branches that meet this box are visited */
    /* 0x01C */ RwBBox clipBox; /* the triangles of a branch inside this box need no test */
    /* 0x034 */ RpCollTreeBranch root;
    /* 0x03C */ CollTreeBoxEntry stack[33];
    /* 0x144 */ CollTreeBoxEntry *top;
} CollTreeBoxIt;
CHECK_SIZEOF(CollTreeBoxIt, 0x148);

static inline void CollWorldBoxItStart(CollWorldBoxIt *it, RpWorld *world, const RwBBox *box) {
    it->box = *box;
    it->stack[0] = world->rootSector;
    it->top = &it->stack[1];
}

/* Moves to the next world sector the box may meet, pushing the left side of each split the box
 * crosses; call while there are entries left */
static inline void CollWorldBoxItNext(CollWorldBoxIt *it) {
    it->top--;
    while ((*it->top)->type >= 0) {
        RpPlaneSector *plane = (RpPlaneSector *)*it->top;

        if (plane->rightValue > RwV3dGetCoord(it->box.sup, plane->type)) {
            *it->top = plane->leftSubTree;
        } else {
            *it->top = plane->rightSubTree;
            if (plane->leftValue >= RwV3dGetCoord(it->box.inf, plane->type)) {
                it->top++;
                *it->top = plane->leftSubTree;
            }
        }
    }
}

/* Starts a box walk of a collision tree: none if the tree is outside box */
static inline void CollTreeBoxItStart(CollTreeBoxIt *it, RpCollTree *tree, const RwBBox *box,
                                      const RwBBox *clipBox) {
    if (tree->bbox.inf.x <= box->sup.x && tree->bbox.inf.y <= box->sup.y &&
        tree->bbox.inf.z <= box->sup.z && tree->bbox.sup.x >= box->inf.x &&
        tree->bbox.sup.y >= box->inf.y && tree->bbox.sup.z >= box->inf.z) {
        it->tree = tree;
        it->root.index = 0;
        it->root.type = 0xFF;
        if (tree->numSplits == 0) {
            it->root.type = tree->numTriangles;
        }
        it->box = *box;
        it->clipBox = *clipBox;
        it->stack[0].branch = &it->root;
        it->stack[0].clipFlags = ((tree->bbox.inf.x < clipBox->inf.x) ? 0x1 : 0) |
                                 ((tree->bbox.inf.y < clipBox->inf.y) ? 0x4 : 0) |
                                 ((tree->bbox.inf.z < clipBox->inf.z) ? 0x10 : 0) |
                                 ((tree->bbox.sup.x > clipBox->sup.x) ? 0x2 : 0) |
                                 ((tree->bbox.sup.y > clipBox->sup.y) ? 0x8 : 0) |
                                 ((tree->bbox.sup.z > clipBox->sup.z) ? 0x20 : 0);
        it->top = &it->stack[1];
    } else {
        it->top = it->stack;
    }
}

/* Moves to the next leaf the box may meet, or to a branch inside the clip box (the triangles of its
 * leaves are a single range), and gives its triangles and clip flags; use while there are entries
 * left */
#define CollTreeBoxItNextMacro(it, start, count, clipFlags)                                        \
    do {                                                                                           \
        (it)->top--;                                                                               \
        for (;;) {                                                                                 \
            RpCollTreeBranch *branch_ = (it)->top->branch;                                         \
            RpCollTreeSplit *split_;                                                               \
            RwUInt32 axis_;                                                                        \
                                                                                                   \
            if (branch_->type != 0xFF) {                                                           \
                (start) = branch_->index;                                                          \
                (clipFlags) = (it)->top->clipFlags;                                                \
                (count) = branch_->type;                                                           \
                break;                                                                             \
            }                                                                                      \
            if ((it)->top->clipFlags == 0) {                                                       \
                RpCollTreeBranch *first_;                                                          \
                RpCollTreeBranch *last_;                                                           \
                                                                                                   \
                split_ = &(it)->tree->splits[branch_->index];                                      \
                first_ = &split_->left;                                                            \
                while (first_->type == 0xFF) {                                                     \
                    first_ = &(it)->tree->splits[first_->index].left;                              \
                }                                                                                  \
                last_ = &split_->right;                                                            \
                while (last_->type == 0xFF) {                                                      \
                    last_ = &(it)->tree->splits[last_->index].right;                               \
                }                                                                                  \
                (clipFlags) = 0;                                                                   \
                (start) = first_->index;                                                           \
                (count) = last_->index + last_->type - first_->index;                              \
                break;                                                                             \
            }                                                                                      \
                                                                                                   \
            split_ = &(it)->tree->splits[branch_->index];                                          \
            axis_ = split_->left.field_0x0 & 0xC;                                                  \
            if (split_->right.value > RwV3dGetCoord((it)->box.sup, axis_)) {                       \
                (it)->top->branch = &split_->left;                                                 \
                continue;                                                                          \
            }                                                                                      \
            (it)->top->branch = &split_->right;                                                    \
            if (split_->left.value >= RwV3dGetCoord((it)->box.inf, axis_)) {                       \
                (it)->top++;                                                                       \
                (it)->top->branch = &split_->left;                                                 \
                (it)->top->clipFlags = (it)->top[-1].clipFlags;                                    \
                if (split_->left.value <= RwV3dGetCoord((it)->clipBox.sup, axis_)) {               \
                    (it)->top->clipFlags &= ~(0x2 << (axis_ >> 1));                                \
                }                                                                                  \
                if (split_->right.value >= RwV3dGetCoord((it)->clipBox.inf, axis_)) {              \
                    (it)->top[-1].clipFlags &= ~(0x1 << (axis_ >> 1));                             \
                }                                                                                  \
            }                                                                                      \
        }                                                                                          \
    } while (0)

/* Calls callback for the triangles of a world that meet a box, with distance 0; stops when callback
 * returns NULL */
void fn_80441BF8(RpWorld *world, RpIntersection *intersection,
                 RpIntersectionCallBackWorldTriangle callback, void *data) {
    CollWorldBoxIt worldIt;
    CollTreeBoxIt treeIt;
    RpCollisionTriangle collTri;

    CollWorldBoxItStart(&worldIt, world, &intersection->t.box);
    while (worldIt.top > worldIt.stack) {
        RpWorldSector *sector;
        RpCollTree *tree;
        RpTriangle *triangles;
        RwV3d *vertices;

        CollWorldBoxItNext(&worldIt);
        sector = (RpWorldSector *)*worldIt.top;
        tree = *RWPLUGINOFFSET(RpCollTree *, sector, lbl_805F1420);
        if (tree == NULL) {
            continue;
        }
        triangles = sector->triangles;
        vertices = sector->vertices;

        CollTreeBoxItStart(&treeIt, tree, &intersection->t.box, &intersection->t.box);
        while (treeIt.top > treeIt.stack) {
            RwUInt16 start;
            RwUInt16 count;
            RwUInt16 i;
            RwUInt32 clipFlags;

            CollTreeBoxItNextMacro(&treeIt, start, count, clipFlags);

            /* the triangles; those of a branch inside the clip box need no test */
            for (i = start; i < start + count; i++) {
                RwInt32 triIndex;
                RpTriangle *triangle;
                RwV3d *v0;
                RwV3d *v1;
                RwV3d *v2;
                RwV3d a;
                RwV3d b;
                RwReal recipLength;

                triIndex = (tree->map != NULL) ? tree->map[i] : i;
                triangle = &triangles[triIndex];
                v0 = &vertices[triangle->vertIndex[0]];
                v1 = &vertices[triangle->vertIndex[1]];
                v2 = &vertices[triangle->vertIndex[2]];
                if (clipFlags != 0 && !fn_8047BDD0(&intersection->t.box, v0, v1, v2)) {
                    continue;
                }

                collTri.point = *v0;
                collTri.index = triIndex;
                collTri.vertices[0] = v0;
                collTri.vertices[1] = v1;
                collTri.vertices[2] = v2;
                a.x = v1->x - v0->x;
                a.y = v1->y - v0->y;
                a.z = v1->z - v0->z;
                b.x = v2->x - v0->x;
                b.y = v2->y - v0->y;
                b.z = v2->z - v0->z;
                collTri.normal.x = a.y * b.z - a.z * b.y;
                collTri.normal.y = a.z * b.x - a.x * b.z;
                collTri.normal.z = a.x * b.y - a.y * b.x;
                recipLength = fn_80482394(collTri.normal.x * collTri.normal.x +
                                          collTri.normal.y * collTri.normal.y +
                                          collTri.normal.z * collTri.normal.z);
                collTri.normal.x *= recipLength;
                collTri.normal.y *= recipLength;
                collTri.normal.z *= recipLength;
                if (callback(intersection, sector, &collTri, 0.0f, data) == NULL) {
                    return;
                }
            }
        }
    }
}

/* The box around center that reaches radius along each axis */
static inline void CollBBoxAround(RwBBox *box, const RwV3d *center, RwReal radius) {
    box->sup = *center;
    box->inf = *center;
    box->inf.x -= radius;
    box->inf.y -= radius;
    box->inf.z -= radius;
    box->sup.x += radius;
    box->sup.y += radius;
    box->sup.z += radius;
}

/* Calls callback for the triangles of a world that meet a sphere, with the distance of the sphere's
 * center from the triangle's plane in units of the radius; stops when callback returns NULL */
void fn_80442210(RpWorld *world, const RwSphere *sphere, RpIntersection *intersection,
                 RpIntersectionCallBackWorldTriangle callback, void *data) {
    CollWorldBoxIt worldIt;
    CollTreeBoxIt treeIt;
    RpCollisionTriangle collTri;
    RwBBox box;
    RwBBox clipBox;
    RwV3d center;
    RwReal distance;
    RwReal recipRadius;
    RwReal clipRadius;

    /* the sphere's bounding box, and a box inside the sphere (the cube in it, made a little
     * smaller): triangles inside it need no test */
    center = sphere->center;
    recipRadius = 1.0f / sphere->radius;
    clipRadius = 0.57f * sphere->radius;
    CollBBoxAround(&box, &center, sphere->radius);
    CollBBoxAround(&clipBox, &center, clipRadius);

    CollWorldBoxItStart(&worldIt, world, &box);
    while (worldIt.top > worldIt.stack) {
        RpWorldSector *sector;
        RpCollTree *tree;
        RpTriangle *triangles;
        RwV3d *vertices;
        RwReal axisDistance;
        RwReal distance2;

        CollWorldBoxItNext(&worldIt);
        sector = (RpWorldSector *)*worldIt.top;
        tree = *RWPLUGINOFFSET(RpCollTree *, sector, lbl_805F1420);
        if (tree == NULL) {
            continue;
        }

        /* skip trees whose bounding box is outside the sphere */
        axisDistance = (sphere->center.x < tree->bbox.inf.x)   ? tree->bbox.inf.x - sphere->center.x
                       : (sphere->center.x > tree->bbox.sup.x) ? sphere->center.x - tree->bbox.sup.x
                                                               : 0.0f;
        distance2 = axisDistance * axisDistance;
        axisDistance = (sphere->center.y < tree->bbox.inf.y)   ? tree->bbox.inf.y - sphere->center.y
                       : (sphere->center.y > tree->bbox.sup.y) ? sphere->center.y - tree->bbox.sup.y
                                                               : 0.0f;
        distance2 += axisDistance * axisDistance;
        axisDistance = (sphere->center.z < tree->bbox.inf.z)   ? tree->bbox.inf.z - sphere->center.z
                       : (sphere->center.z > tree->bbox.sup.z) ? sphere->center.z - tree->bbox.sup.z
                                                               : 0.0f;
        distance2 += axisDistance * axisDistance;
        if (!(distance2 <= sphere->radius * sphere->radius)) {
            continue;
        }
        triangles = sector->triangles;
        vertices = sector->vertices;

        CollTreeBoxItStart(&treeIt, tree, &box, &clipBox);
        while (treeIt.top > treeIt.stack) {
            RwUInt16 start;
            RwUInt16 count;
            RwUInt16 i;
            RwUInt32 clipFlags;

            CollTreeBoxItNextMacro(&treeIt, start, count, clipFlags);

            /* the triangles; those of a branch inside the clip box are inside the sphere */
            for (i = start; i < start + count; i++) {
                RwInt32 triIndex;
                RpTriangle *triangle;
                RwV3d *v0;
                RwV3d *v1;
                RwV3d *v2;

                triIndex = (tree->map != NULL) ? tree->map[i] : i;
                triangle = &triangles[triIndex];
                v0 = &vertices[triangle->vertIndex[0]];
                v1 = &vertices[triangle->vertIndex[1]];
                v2 = &vertices[triangle->vertIndex[2]];
                if (clipFlags == 0) {
                    RwV3d a;
                    RwV3d b;
                    RwReal recipLength;
                    RwV3d offset;

                    a.x = v1->x - v0->x;
                    a.y = v1->y - v0->y;
                    a.z = v1->z - v0->z;
                    b.x = v2->x - v0->x;
                    b.y = v2->y - v0->y;
                    b.z = v2->z - v0->z;
                    collTri.normal.x = a.y * b.z - a.z * b.y;
                    collTri.normal.y = a.z * b.x - a.x * b.z;
                    collTri.normal.z = a.x * b.y - a.y * b.x;
                    recipLength = fn_80482394(collTri.normal.x * collTri.normal.x +
                                              collTri.normal.y * collTri.normal.y +
                                              collTri.normal.z * collTri.normal.z);
                    collTri.normal.x *= recipLength;
                    collTri.normal.y *= recipLength;
                    collTri.normal.z *= recipLength;
                    offset.x = sphere->center.x - v0->x;
                    offset.y = sphere->center.y - v0->y;
                    offset.z = sphere->center.z - v0->z;
                    distance = offset.x * collTri.normal.x + offset.y * collTri.normal.y +
                               offset.z * collTri.normal.z;
                } else if (!fn_8047B720(sphere, v0, v1, v2, &collTri.normal, &distance)) {
                    continue;
                }

                distance *= recipRadius;
                collTri.point = *v0;
                collTri.index = triIndex;
                collTri.vertices[0] = v0;
                collTri.vertices[1] = v1;
                collTri.vertices[2] = v2;
                if (callback(intersection, sector, &collTri, distance, data) == NULL) {
                    return;
                }
            }
        }
    }
}

/* Calls callback for the triangles of a world that meet the intersection primitive */
RpWorld *fn_80442AA4(RpWorld *world, RpIntersection *intersection,
                     RpIntersectionCallBackWorldTriangle callback, void *data) {
    switch (intersection->type) {
    case rpINTERSECTLINE:
        fn_80441188(world, intersection, callback, data);
        return world;
    case rpINTERSECTSPHERE:
        fn_80442210(world, &intersection->t.sphere, intersection, callback, data);
        return world;
    case rpINTERSECTBOX:
        fn_80441BF8(world, intersection, callback, data);
        return world;
    case rpINTERSECTATOMIC: {
        RwSphere sphere = *fn_804566D0(intersection->t.object);

        fn_80442210(world, &sphere, intersection, callback, data);
        return world;
    }
    }
    return world;
}
