/* RenderWare mesh generation: sorting a build mesh's triangles by material, and turning them into
 * triangle lists or triangle strips. The original file name is unknown. */
#include "rw/rpworld.h"
#include <stdlib.h>
#include <string.h>

#define rwMEMHINTDUR_FUNCTION 0x10000
#define rwMEMHINTDUR_EVENT 0x30000
#define rwMEMHINTFLAG_RESIZABLE 0x1000000

#define rwID_MESHMODULE 0x502

/* the plugin ID the world module reports errors with (inferred) */
#define rwWORLDERRORPLUGIN 2

/* Reports an error: the arguments are the error code and its parameters */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwWORLDERRORPLUGIN;                                                      \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

/* raster pixel formats with alpha (rwRASTERFORMAT*) */
#define rwRASTERFORMATPIXELFORMATMASK 0x0F00
#define rwRASTERFORMAT1555 0x0100
#define rwRASTERFORMAT4444 0x0300
#define rwRASTERFORMAT8888 0x0500

/* The first trial at which the strip builder commits polygons; earlier trials only test them */
#define rpTRISTRIPCOMMIT 4

typedef struct TriStripPolygon TriStripPolygon;
typedef struct TriStripEdge TriStripEdge;

/* A triangle while strips are built (0x24 bytes) */
struct TriStripPolygon {
    /* 0x00 */ RwUInt32 index; /* of the build mesh triangle */
    /* 0x04 */ TriStripEdge *edges[3];
    /* 0x10 */ TriStripPolygon *next; /* in the list of polygons with as many neighbours */
    /* 0x14 */ TriStripPolygon *prev;
    /* 0x18 */ RwInt32 done;     /* in a strip */
    /* 0x1C */ RwInt32 testDone; /* in a trial strip */
    /* 0x20 */ RwUInt8 numNeighbours;
};

/* An edge shared by up to two triangles (0x10 bytes) */
struct TriStripEdge {
    /* 0x0 */ RxVertexIndex vert0;
    /* 0x2 */ RxVertexIndex vert1;
    /* 0x4 */ TriStripPolygon *poly0;
    /* 0x8 */ TriStripPolygon *poly1;
    /* 0xC */ TriStripEdge *next;
};

/* A triangle strip (0x10 bytes) */
typedef struct TriStripList {
    /* 0x0 */ RxVertexIndex *vertices;
    /* 0x4 */ RwUInt32 numVertices;
    /* 0x8 */ RwUInt32 size; /* in vertices */
    /* 0xC */ struct TriStripList *next;
} TriStripList;

/* The free lists of the strip builder's polygons and edges */
typedef struct TriStripFreeLists {
    /* 0x0 */ RwFreeList *polygons;
    /* 0x4 */ RwFreeList *edges;
} TriStripFreeLists;

/* A run of sorted triangles that share a material (0xC bytes) */
typedef struct MaterialRun {
    /* 0x0 */ void *field_0x0;
    /* 0x4 */ RwUInt32 numTriangles; /* first the index of the run's first triangle */
    /* 0x8 */ RpMaterial *material;
} MaterialRun;

typedef RpMeshHeader *(*RpTriStripMeshCallBack)(RpBuildMesh *buildMesh, void *data);

/* The triangle strip generator (inferred) */
typedef struct TriStripMethod {
    /* 0x0 */ RpTriStripMeshCallBack callBack;
    /* 0x4 */ void *data;
} TriStripMethod;

RpMeshHeader *fn_8045EF3C(RpBuildMesh *buildMesh, void *data);

/* The unit's statics: one object, which the code addresses from a single base (inferred) */
typedef struct MeshGenerationStatics {
    /* 0x00 */ RpMeshHeader emptyMeshHeader; /* of a build mesh without triangles; not freed */
    /* 0x10 */ TriStripMethod triStripMethod;
} MeshGenerationStatics;

MeshGenerationStatics lbl_8056D868 = {{0, 0, 0, 0, 0}, {fn_8045EF3C, NULL}};

/* The material sort key bits. The original defines them as a constant aggregate, which puts them in
 * .sdata2 and makes the comparison load them (inferred). */
typedef struct MaterialSortKeys {
    /* 0x0 */ RwUInt32 transparent; /* transparent triangles sort after the opaque ones */
} MaterialSortKeys;

const MaterialSortKeys lbl_805FB170 = {16};

/* The sort key of a triangle's material: whether it is transparent */
#define MATERIALSORTKEY(key, material)                                                             \
    do {                                                                                           \
        if ((material) != NULL) {                                                                  \
            RwTexture *texture_ = (material)->texture;                                             \
                                                                                                   \
            if (texture_ != NULL) {                                                                \
                RwInt32 format_ =                                                                  \
                    (texture_->raster->cFormat << 8) & rwRASTERFORMATPIXELFORMATMASK;              \
                                                                                                   \
                if (format_ == rwRASTERFORMAT1555 || format_ == rwRASTERFORMAT4444 ||              \
                    format_ == rwRASTERFORMAT8888) {                                               \
                    (key) = lbl_805FB170.transparent;                                              \
                }                                                                                  \
            }                                                                                      \
            if ((material)->color.alpha != 0xFF) {                                                 \
                (key) |= lbl_805FB170.transparent;                                                 \
            }                                                                                      \
        }                                                                                          \
    } while (0)

/* The triangle that a sorted triangle pointer points at */
#define SORTEDTRIANGLE(element) (*(const RpBuildMeshTriangle *const *)(element))

/* qsort comparison of two build mesh triangle pointers: by material, transparency last */
int fn_8045CA38(const void *a, const void *b) {
    RwUInt32 keyA = 0;
    RwUInt32 keyB = 0;
    RpMaterial *materialA = SORTEDTRIANGLE(a)->material;
    RpMaterial *materialB = SORTEDTRIANGLE(b)->material;

    if (materialA == materialB) {
        return 0;
    }
    MATERIALSORTKEY(keyA, materialA);
    MATERIALSORTKEY(keyB, materialB);

    keyA |= (SORTEDTRIANGLE(a)->rasterIndex > SORTEDTRIANGLE(b)->rasterIndex) ? 8 : 0;
    keyB |= (SORTEDTRIANGLE(a)->rasterIndex < SORTEDTRIANGLE(b)->rasterIndex) ? 8 : 0;
    keyA |= (SORTEDTRIANGLE(a)->pipelineIndex > SORTEDTRIANGLE(b)->pipelineIndex) ? 4 : 0;
    keyB |= (SORTEDTRIANGLE(a)->pipelineIndex < SORTEDTRIANGLE(b)->pipelineIndex) ? 4 : 0;
    keyA |= (SORTEDTRIANGLE(a)->textureIndex > SORTEDTRIANGLE(b)->textureIndex) ? 2 : 0;
    keyB |= (SORTEDTRIANGLE(a)->textureIndex < SORTEDTRIANGLE(b)->textureIndex) ? 2 : 0;
    keyA |= SORTEDTRIANGLE(a)->matIndex > SORTEDTRIANGLE(b)->matIndex;
    keyB |= SORTEDTRIANGLE(a)->matIndex < SORTEDTRIANGLE(b)->matIndex;
    return keyA - keyB;
}

/* Whether a vertex is in a bit set of vertices */
#define VERTEXUSED(used, vertex) (((RwUInt32 *)(used))[(vertex) >> 5] & (1 << ((vertex) & 0x1F)))
#define VERTEXUSE(used, vertex) (((RwUInt32 *)(used))[(vertex) >> 5] |= 1 << ((vertex) & 0x1F))

/* Reorders a triangle list mesh in passes, each taking only triangles whose vertices no earlier
 * triangle of the pass uses (inferred) */
RpMesh *fn_8045CBD4(RpMesh *mesh, RpMeshHeader *meshHeader, void *data) {
    RwUInt32 numIndices = mesh->numIndices;
    RwUInt32 maxVertex = 0;
    RwUInt32 i;
    RwUInt32 usedSize;
    RwUInt8 *used;
    RwUInt32 numTriangles = numIndices / 3;
    RxVertexIndex *indices = mesh->indices;

    for (i = 0; i < numIndices; i++) {
        maxVertex |= indices[i];
    }
    usedSize = (maxVertex + 0x1F) >> 3;
    used = RwMalloc(usedSize, rwMEMHINTDUR_FUNCTION | rwID_MESHMODULE);
    if (used != NULL) {
        RwUInt32 copySize = mesh->numIndices * sizeof(RxVertexIndex);
        RxVertexIndex *copy = RwMalloc(copySize, rwMEMHINTDUR_FUNCTION | rwID_MESHMODULE);

        if (copy != NULL) {
            RwUInt32 numDone;

            memcpy(copy, indices, copySize);
            numDone = 0;
            while (numDone < numTriangles) {
                RwUInt8 *clear = used;
                RwUInt32 count = usedSize;
                RxVertexIndex *triangle;

                while (count--) {
                    *clear++ = 0;
                }
                triangle = copy;
                for (count = 0; count < numTriangles; count++) {
                    RwInt32 vert0 = triangle[0];
                    RwInt32 vert1 = triangle[1];
                    RwInt32 vert2 = triangle[2];

                    if ((vert0 & vert1 & vert2) != 0xFFFF && !VERTEXUSED(used, vert0) &&
                        !VERTEXUSED(used, vert1) && !VERTEXUSED(used, vert2)) {
                        VERTEXUSE(used, vert0);
                        numDone++;
                        VERTEXUSE(used, triangle[1]);
                        VERTEXUSE(used, triangle[2]);
                        indices[0] = triangle[0];
                        indices[1] = triangle[1];
                        indices[2] = triangle[2];
                        indices += 3;
                        triangle[0] = triangle[1] = triangle[2] = 0xFFFF;
                    }
                    triangle += 3;
                }
            }
            RwFree(copy);
        }
        RwFree(used);
        return mesh;
    }
    return mesh;
}

/* Finds the edge from vert0 to vert1 that another polygon added, or adds it for *polygon
 * (inferred) */
static inline TriStripEdge *TriStripAddEdge(TriStripEdge **edgeList, TriStripPolygon **polygon,
                                            RwFreeList *edgeFreeList, RxVertexIndex vert0,
                                            RxVertexIndex vert1) {
    TriStripEdge *edge;

    for (edge = *edgeList; edge != NULL; edge = edge->next) {
        if (vert0 == edge->vert1 && vert1 == edge->vert0 && edge->poly1 == NULL) {
            edge->poly0->numNeighbours++;
            (*polygon)->numNeighbours++;
            edge->poly1 = *polygon;
            return edge;
        }
    }
    edge = RwFreeListAlloc(edgeFreeList, rwMEMHINTDUR_EVENT | rwID_MESHMODULE);
    edge->vert0 = vert0;
    edge->vert1 = vert1;
    edge->poly0 = *polygon;
    edge->poly1 = NULL;
    edge->next = *edgeList;
    *edgeList = edge;
    return edge;
}

/* Creates the polygons of a build mesh's triangles and the edges between them */
TriStripPolygon **fn_8045CED8(RwUInt32 numTriangles, TriStripFreeLists *freeLists,
                              TriStripEdge **edgeList, RpBuildMeshTriangle *triangles) {
    TriStripPolygon **polygons =
        RwMalloc(numTriangles * sizeof(TriStripPolygon *), rwMEMHINTDUR_EVENT | rwID_MESHMODULE);
    TriStripPolygon **polygon;
    RpBuildMeshTriangle *triangle;
    RwUInt32 i;

    freeLists->polygons =
        fn_8047FBD8(sizeof(TriStripPolygon), numTriangles, 4, rwMEMHINTDUR_EVENT | rwID_MESHMODULE);
    freeLists->edges = fn_8047FBD8(sizeof(TriStripEdge), (numTriangles >> 1) + 1, 4,
                                   rwMEMHINTDUR_EVENT | rwID_MESHMODULE);

    polygon = polygons;
    triangle = triangles;
    for (i = 0; i < numTriangles; i++) {
        *polygon = RwFreeListAlloc(freeLists->polygons, rwMEMHINTDUR_EVENT | rwID_MESHMODULE);
        (*polygon)->numNeighbours = 0;
        (*polygon)->index = i;
        (*polygon)->prev = NULL;
        (*polygon)->next = NULL;
        (*polygon)->done = FALSE;
        (*polygon)->edges[0] = TriStripAddEdge(edgeList, polygon, freeLists->edges,
                                               triangle->vertIndex[0], triangle->vertIndex[1]);
        (*polygon)->edges[1] = TriStripAddEdge(edgeList, polygon, freeLists->edges,
                                               triangle->vertIndex[1], triangle->vertIndex[2]);
        (*polygon)->edges[2] = TriStripAddEdge(edgeList, polygon, freeLists->edges,
                                               triangle->vertIndex[2], triangle->vertIndex[0]);
        polygon++;
        triangle++;
    }
    return polygons;
}

/* Takes a polygon out of the list of polygons with as many neighbours */
#define TRISTRIPUNLINK(buckets, polygon)                                                           \
    do {                                                                                           \
        if ((buckets)[(polygon)->numNeighbours] == (polygon)) {                                    \
            (buckets)[(polygon)->numNeighbours] = (buckets)[(polygon)->numNeighbours]->next;       \
            if ((buckets)[(polygon)->numNeighbours] != NULL) {                                     \
                (buckets)[(polygon)->numNeighbours]->prev = NULL;                                  \
            }                                                                                      \
        } else {                                                                                   \
            if ((polygon)->next != NULL) {                                                         \
                (polygon)->next->prev = (polygon)->prev;                                           \
            }                                                                                      \
            if ((polygon)->prev != NULL) {                                                         \
                (polygon)->prev->next = (polygon)->next;                                           \
            }                                                                                      \
        }                                                                                          \
    } while (0)

/* Puts a polygon at the head of the list of polygons with as many neighbours */
#define TRISTRIPLINK(buckets, polygon)                                                             \
    do {                                                                                           \
        (polygon)->next = (buckets)[(polygon)->numNeighbours];                                     \
        if ((polygon)->next != NULL) {                                                             \
            (polygon)->next->prev = (polygon);                                                     \
        }                                                                                          \
        (buckets)[(polygon)->numNeighbours] = (polygon);                                           \
        (polygon)->prev = NULL;                                                                    \
    } while (0)

/* Marks a polygon as stripped. A trial only marks it as tested; a commit also takes it out of its
 * list and moves each neighbour still free to the list with one neighbour fewer. */
void fn_8045D20C(TriStripPolygon *polygon, TriStripPolygon **buckets, RwInt32 trial) {
    RwUInt8 i;

    if (trial < rpTRISTRIPCOMMIT) {
        polygon->testDone = TRUE;
        return;
    }
    polygon->done = TRUE;
    TRISTRIPUNLINK(buckets, polygon);
    for (i = 0; i < 3; i++) {
        TriStripEdge *edge = polygon->edges[i];
        TriStripPolygon *neighbour = NULL;

        if (edge->poly0 != NULL && edge->poly0 != polygon && !edge->poly0->done) {
            neighbour = edge->poly0;
        } else if (edge->poly1 != NULL && !edge->poly1->done) {
            neighbour = edge->poly1;
        }
        if (neighbour != NULL) {
            TRISTRIPUNLINK(buckets, neighbour);
            neighbour->numNeighbours--;
            TRISTRIPLINK(buckets, neighbour);
        }
    }
}

/* Whether a polygon is free for the strip: with tested, not tested in this trial yet; otherwise
 * not in a strip yet */
static inline RwBool TriStripPolygonFree(const TriStripPolygon *polygon, RwBool tested) {
    RwBool isFree = FALSE;

    if (polygon != NULL && !(tested ? polygon->testDone : polygon->done)) {
        isFree = TRUE;
    }
    return isFree;
}

/* How many free polygons share an edge */
static inline RwInt32 TriStripNumFree(const TriStripEdge *edge, RwBool tested) {
    RwInt32 numFree1 = TriStripPolygonFree(edge->poly1, tested);
    RwInt32 numFree0 = TriStripPolygonFree(edge->poly0, tested);

    return numFree0 + numFree1;
}

/* Whether no free polygon shares an edge, or there is no edge */
static inline RwBool TriStripEdgeClosed(const TriStripEdge *edge, RwBool tested) {
    RwBool closed = TRUE;

    if (edge != NULL && TriStripNumFree(edge, tested) != 0) {
        closed = FALSE;
    }
    return closed;
}

/* Whether a free polygon shares an edge */
static inline RwBool TriStripEdgeOpen(const TriStripEdge *edge, RwBool tested) {
    RwBool open = FALSE;

    if (edge != NULL && TriStripNumFree(edge, tested) != 0) {
        open = TRUE;
    }
    return open;
}

/* Whether a strip ends at an edge */
static inline RwBool TriStripEdgeEnds(const TriStripEdge *edge, RwBool tested) {
    RwBool ends = TRUE;

    if (!TriStripEdgeClosed(edge, tested)) {
        ends = FALSE;
    }
    return ends;
}

/* Whether a strip can go on across an edge */
static inline RwBool TriStripEdgeLeadsOn(const TriStripEdge *edge, RwBool tested) {
    RwBool leadsOn = FALSE;

    if (TriStripEdgeOpen(edge, tested)) {
        leadsOn = TRUE;
    }
    return leadsOn;
}

/* The trial's view of TriStripNumFree and the edge tests */
#define TRISTRIPNUMFREE(edge, trial)                                                               \
    ((trial) < rpTRISTRIPCOMMIT ? TriStripNumFree(edge, TRUE) : TriStripNumFree(edge, FALSE))

/* Whether an edge joins two vertices, in either direction */
#define TRISTRIPEDGEJOINS(edge, a, b)                                                              \
    (((edge)->vert0 == (a) && (edge)->vert1 == (b)) ||                                             \
     ((edge)->vert0 == (b) && (edge)->vert1 == (a)))

/* Extends a strip across edge: the number of polygons it adds */
RwInt32 fn_8045D3A8(TriStripList *strip, TriStripEdge *edge, TriStripPolygon **buckets,
                    RpBuildMeshTriangle *triangles, RwInt32 trial) {
    TriStripPolygon *polygon;
    RwInt32 side;
    RwInt32 numAdded = 0;
    TriStripEdge *otherEdge = NULL;

    while (edge != NULL) {
        TriStripEdge *nextEdge;
        TriStripEdge *entryEdge;
        RxVertexIndex vertex;
        RxVertexIndex *end;
        RxVertexIndex last;
        RwUInt32 numVertices;
        RwBool stuck;

        polygon = NULL;
        side = -1;
        if (trial < rpTRISTRIPCOMMIT) {
            if (edge->poly0 != NULL && !edge->poly0->testDone) {
                polygon = edge->poly0;
            } else if (edge->poly1 != NULL && !edge->poly1->testDone) {
                polygon = edge->poly1;
            }
        } else {
            if (edge->poly0 != NULL && !edge->poly0->done) {
                polygon = edge->poly0;
            } else if (edge->poly1 != NULL && !edge->poly1->done) {
                polygon = edge->poly1;
            }
        }
        if (polygon == NULL) {
            return numAdded;
        }
        numAdded++;
        fn_8045D20C(polygon, buckets, trial);

        if (edge == polygon->edges[0]) {
            side = 0;
        } else if (edge == polygon->edges[1]) {
            side = 1;
        } else if (edge == polygon->edges[2]) {
            side = 2;
        }
        /* the new vertex: the far end of the edge after the entry edge */
        nextEdge = polygon->edges[(side + 1) % 3];
        if (nextEdge->poly0 == polygon) {
            vertex = nextEdge->vert1;
        } else {
            vertex = nextEdge->vert0;
        }
        entryEdge = edge;

        /* the edge across which the strip continues: from its last vertex to the new one */
        numVertices = strip->numVertices;
        end = &strip->vertices[numVertices];
        last = end[-1];
        if (TRISTRIPEDGEJOINS(polygon->edges[0], last, vertex)) {
            nextEdge = polygon->edges[0];
        } else if (TRISTRIPEDGEJOINS(polygon->edges[1], last, vertex)) {
            nextEdge = polygon->edges[1];
        } else if (TRISTRIPEDGEJOINS(polygon->edges[2], last, vertex)) {
            nextEdge = polygon->edges[2];
        } else {
            nextEdge = NULL;
        }
        edge = nextEdge;
        if (trial < rpTRISTRIPCOMMIT) {
            stuck = TriStripEdgeEnds(nextEdge, TRUE);
        } else {
            stuck = TriStripEdgeEnds(nextEdge, FALSE);
        }

        if (stuck) {
            /* turn across the polygon's third edge, which costs a repeated vertex */
            if (polygon->edges[0] != entryEdge && polygon->edges[0] != nextEdge) {
                otherEdge = polygon->edges[0];
            } else if (polygon->edges[1] != entryEdge && polygon->edges[1] != nextEdge) {
                otherEdge = polygon->edges[1];
            } else if (polygon->edges[2] != entryEdge && polygon->edges[2] != nextEdge) {
                otherEdge = polygon->edges[2];
            }
            if (trial < rpTRISTRIPCOMMIT ? TriStripEdgeLeadsOn(otherEdge, TRUE)
                                         : TriStripEdgeLeadsOn(otherEdge, FALSE)) {
                if (numVertices & 1) {
                    *end = vertex;
                    edge = NULL;
                    strip->numVertices++;
                } else {
                    edge = otherEdge;
                    *end = end[-2];
                    strip->numVertices++;
                    strip->vertices[strip->numVertices] = vertex;
                    strip->numVertices++;
                }
            } else {
                *end = vertex;
                edge = NULL;
                strip->numVertices++;
            }
        } else {
            *end = vertex;
            strip->numVertices++;
        }
    }
    return numAdded;
}

/* Builds the strips of numTriangles triangles: the longest of three trial strips from each
 * starting polygon, or the first when trial is 0 (inferred) */
RwBool fn_8045D898(RpBuildMeshTriangle *triangles, RwUInt32 numTriangles, TriStripList **stripList,
                   RwInt32 firstTrial) {
    TriStripEdge *edgeList = NULL;
    TriStripFreeLists freeLists;
    TriStripPolygon *buckets[4];
    TriStripPolygon **polygons;
    TriStripList *forward;
    TriStripList *backward;
    TriStripEdge *aheadEdge = NULL;
    TriStripEdge *behindEdge = NULL;
    RwUInt32 numDone = 0;
    RwUInt32 i;

    buckets[0] = NULL;
    buckets[1] = NULL;
    buckets[2] = NULL;
    buckets[3] = NULL;
    freeLists.polygons = NULL;
    freeLists.edges = NULL;
    polygons = fn_8045CED8(numTriangles, &freeLists, &edgeList, triangles);
    for (i = 0; i < numTriangles; i++) {
        TRISTRIPLINK(buckets, polygons[i]);
    }

    forward = RwFreeListAlloc(RPMESHGLOBAL(triStripListEntryFreeList),
                              rwMEMHINTDUR_FUNCTION | rwID_MESHMODULE);
    forward->size = numTriangles * 2 + 2;
    forward->numVertices = 0;
    forward->vertices =
        RwMalloc(forward->size * sizeof(RwUInt32), rwMEMHINTDUR_FUNCTION | rwID_MESHMODULE);
    backward = RwFreeListAlloc(RPMESHGLOBAL(triStripListEntryFreeList),
                               rwMEMHINTDUR_FUNCTION | rwID_MESHMODULE);
    backward->size = numTriangles * 2 + 2;
    backward->numVertices = 0;
    backward->vertices =
        RwMalloc(backward->size * sizeof(RwUInt32), rwMEMHINTDUR_FUNCTION | rwID_MESHMODULE);

    while (numDone < numTriangles) {
        RwUInt32 bestLength = 0;
        RwInt32 trial = firstTrial ? 0 : 3;

        if (buckets[0] != NULL) {
            /* a polygon without neighbours is a strip of its own */
            RwUInt32 index = buckets[0]->index;
            TriStripList *strip = RwFreeListAlloc(RPMESHGLOBAL(triStripListEntryFreeList),
                                                  rwMEMHINTDUR_EVENT | rwID_MESHMODULE);

            strip->next = *stripList;
            *stripList = strip;
            strip->size = 3;
            strip->numVertices = 3;
            strip->vertices = RwMalloc(3 * sizeof(RwUInt32), rwMEMHINTDUR_EVENT | rwID_MESHMODULE);
            strip->vertices[0] = triangles[index].vertIndex[0];
            strip->vertices[1] = triangles[index].vertIndex[1];
            strip->vertices[2] = triangles[index].vertIndex[2];
            buckets[0]->done = TRUE;
            buckets[0]->testDone = TRUE;
            buckets[0] = buckets[0]->next;
            if (buckets[0] != NULL) {
                buckets[0]->prev = NULL;
            }
            numDone++;
        } else {
            TriStripPolygon **bucket = &buckets[1];
            RwInt32 first = 1;
            TriStripPolygon *start;
            TriStripEdge *edge0;
            TriStripEdge *edge1;
            TriStripEdge *edge2;
            RwUInt32 direction;
            RwUInt32 bestDirection;
            RwUInt32 firstDirection;
            RwUInt32 startDone;

            while (*bucket == NULL) {
                bucket++;
                first++;
            }
            start = buckets[first];
            edge0 = start->edges[0];
            edge1 = start->edges[1];
            edge2 = start->edges[2];

            /* start along two edges that both lead on, or else the most promising one */
            if (TRISTRIPNUMFREE(edge2, rpTRISTRIPCOMMIT) >= 2 &&
                TRISTRIPNUMFREE(edge1, rpTRISTRIPCOMMIT) >= 2) {
                direction = 1;
            } else if (TRISTRIPNUMFREE(edge0, rpTRISTRIPCOMMIT) >= 2 &&
                       TRISTRIPNUMFREE(edge2, rpTRISTRIPCOMMIT) >= 2) {
                direction = 2;
            } else if (TRISTRIPNUMFREE(edge1, rpTRISTRIPCOMMIT) >= 2 &&
                       TRISTRIPNUMFREE(edge0, rpTRISTRIPCOMMIT) >= 2) {
                direction = 0;
            } else if (TRISTRIPNUMFREE(edge0, rpTRISTRIPCOMMIT) >
                       TRISTRIPNUMFREE(edge1, rpTRISTRIPCOMMIT)) {
                if (TRISTRIPNUMFREE(edge0, rpTRISTRIPCOMMIT) >
                    TRISTRIPNUMFREE(edge2, rpTRISTRIPCOMMIT)) {
                    direction = 2;
                } else {
                    direction = 1;
                }
            } else {
                if (TRISTRIPNUMFREE(edge1, rpTRISTRIPCOMMIT) >
                    TRISTRIPNUMFREE(edge2, rpTRISTRIPCOMMIT)) {
                    direction = 0;
                } else {
                    direction = 1;
                }
            }

            startDone = numDone;
            bestDirection = direction;
            firstDirection = direction % 3;
            for (;;) {
                RwUInt32 side;
                RwBool extendsBack;
                RpBuildMeshTriangle *triangle;

                for (i = 0; i < numTriangles; i++) {
                    polygons[i]->testDone = polygons[i]->done;
                }
                switch (trial++) {
                case 0:
                    side = firstDirection;
                    break;
                case 1:
                    side = (direction + 1) % 3;
                    break;
                case 2:
                    side = (direction + 2) % 3;
                    break;
                default:
                    side = bestDirection;
                    break;
                }
                switch (side) {
                case 0:
                    aheadEdge = buckets[first]->edges[1];
                    behindEdge = buckets[first]->edges[0];
                    break;
                case 1:
                    aheadEdge = buckets[first]->edges[2];
                    behindEdge = buckets[first]->edges[1];
                    break;
                case 2:
                    aheadEdge = buckets[first]->edges[0];
                    behindEdge = buckets[first]->edges[2];
                    break;
                }

                triangle = &triangles[buckets[first]->index];
                forward->vertices[0] = triangle->vertIndex[side % 3];
                forward->vertices[1] = triangle->vertIndex[(side + 1) % 3];
                forward->vertices[2] = triangle->vertIndex[(side + 2) % 3];
                forward->numVertices = 3;
                fn_8045D20C(buckets[first], buckets, trial);
                numDone =
                    startDone + 1 + fn_8045D3A8(forward, aheadEdge, buckets, triangles, trial);

                if (trial < rpTRISTRIPCOMMIT) {
                    extendsBack = TriStripNumFree(behindEdge, TRUE) > 0;
                } else {
                    extendsBack = TriStripNumFree(behindEdge, FALSE) > 0;
                }
                if (extendsBack) {
                    RwUInt32 numBackward;

                    /* the strip also extends backwards from the starting polygon */
                    backward->vertices[0] = forward->vertices[1];
                    backward->vertices[1] = forward->vertices[0];
                    backward->numVertices = 2;
                    numDone += fn_8045D3A8(backward, behindEdge, buckets, triangles, trial);
                    numBackward = backward->numVertices;
                    if (numBackward & 1) {
                        backward->vertices[numBackward] = backward->vertices[numBackward - 2];
                        backward->numVertices++;
                    }
                    if (numDone > bestLength) {
                        bestLength = numDone;
                        bestDirection = side;
                    }
                    if (trial >= rpTRISTRIPCOMMIT) {
                        TriStripList *strip =
                            RwFreeListAlloc(RPMESHGLOBAL(triStripListEntryFreeList),
                                            rwMEMHINTDUR_EVENT | rwID_MESHMODULE);

                        strip->next = *stripList;
                        *stripList = strip;
                        strip->size = forward->numVertices + (backward->numVertices - 2);
                        strip->numVertices = 0;
                        strip->vertices = RwMalloc(strip->size * sizeof(RwUInt32),
                                                   rwMEMHINTDUR_EVENT | rwID_MESHMODULE);
                        /* the backward strip reversed, without the starting edge */
                        while (backward->numVertices > 2) {
                            strip->vertices[strip->numVertices] =
                                backward->vertices[backward->numVertices - 1];
                            strip->numVertices++;
                            backward->numVertices--;
                        }
                        /* copies twice the forward vertices' bytes, as the original does */
                        memcpy(&strip->vertices[strip->numVertices], forward->vertices,
                               forward->numVertices * sizeof(RwUInt32));
                        strip->numVertices = strip->size;
                        break;
                    }
                } else {
                    if (numDone > bestLength) {
                        bestLength = numDone;
                        bestDirection = side;
                    }
                    if (trial >= rpTRISTRIPCOMMIT) {
                        TriStripList *strip =
                            RwFreeListAlloc(RPMESHGLOBAL(triStripListEntryFreeList),
                                            rwMEMHINTDUR_EVENT | rwID_MESHMODULE);

                        strip->next = *stripList;
                        *stripList = strip;
                        strip->size = forward->numVertices;
                        strip->numVertices = forward->numVertices;
                        strip->vertices = RwMalloc(forward->numVertices * sizeof(RwUInt32),
                                                   rwMEMHINTDUR_EVENT | rwID_MESHMODULE);
                        memcpy(strip->vertices, forward->vertices,
                               forward->numVertices * sizeof(RwUInt32));
                        break;
                    }
                }
            }
        }
    }

    RwFree(backward->vertices);
    backward->vertices = NULL;
    RwFreeListFree(RPMESHGLOBAL(triStripListEntryFreeList), backward);
    RwFree(forward->vertices);
    forward->vertices = NULL;
    RwFreeListFree(RPMESHGLOBAL(triStripListEntryFreeList), forward);

    while (edgeList != NULL) {
        TriStripEdge *next = edgeList->next;

        RwFreeListFree(freeLists.edges, edgeList);
        edgeList = next;
    }
    fn_8047FCDC(freeLists.edges);
    freeLists.edges = NULL;
    for (i = 0; i < numTriangles; i++) {
        RwFreeListFree(freeLists.polygons, polygons[i]);
        polygons[i] = NULL;
    }
    fn_8047FCDC(freeLists.polygons);
    freeLists.polygons = NULL;
    RwFree(polygons);
    return TRUE;
}

/* Joins a list of strips into one, linking each to the next with repeated vertices. With
 * keepWinding, a join keeps the winding of the strip it adds. */
RwBool fn_8045E51C(TriStripList **stripList, RwBool keepWinding) {
    TriStripList *joined;
    TriStripList *strip;
    TriStripList *rest;
    RwUInt32 i;

    if (*stripList == NULL) {
        return FALSE;
    }
    joined = RwFreeListAlloc(RPMESHGLOBAL(triStripListEntryFreeList),
                             rwMEMHINTDUR_EVENT | rwID_MESHMODULE);
    joined->numVertices = 0;
    joined->size = 0;
    for (strip = *stripList; strip != NULL; strip = strip->next) {
        joined->size += strip->numVertices + 6;
    }
    joined->vertices =
        RwMalloc(joined->size * sizeof(RwUInt32), rwMEMHINTDUR_EVENT | rwID_MESHMODULE);

    strip = *stripList;
    for (i = 0; i < strip->numVertices; i++) {
        joined->vertices[joined->numVertices] = strip->vertices[i];
        joined->numVertices++;
    }
    RwFree(strip->vertices);
    strip->vertices = NULL;
    rest = strip->next;
    RwFreeListFree(RPMESHGLOBAL(triStripListEntryFreeList), strip);

    while (rest != NULL) {
        /* prefer a strip that starts where the joined strip ends */
        for (strip = rest; strip != NULL; strip = strip->next) {
            RxVertexIndex *end = &joined->vertices[joined->numVertices];

            if (strip->vertices[0] == end[-1]) {
                if ((joined->numVertices & 1) && keepWinding) {
                    *end = strip->vertices[0];
                    joined->numVertices++;
                }
                break;
            }
        }
        if (strip == NULL) {
            for (strip = rest; strip != NULL; strip = strip->next) {
                RxVertexIndex *end = &joined->vertices[joined->numVertices];

                if (strip->vertices[0] == end[-2]) {
                    if (!(joined->numVertices & 1) && keepWinding) {
                        *end = strip->vertices[0];
                        joined->vertices[joined->numVertices + 1] = strip->vertices[0];
                        joined->numVertices += 2;
                    } else {
                        *end = strip->vertices[0];
                        joined->numVertices++;
                    }
                    break;
                }
            }
        }
        if (strip == NULL) {
            /* no strip continues the joined one: link to the first with a degenerate triangle */
            strip = rest;
            if (!(joined->numVertices & 1) || !keepWinding) {
                RxVertexIndex *end = &joined->vertices[joined->numVertices];

                *end = end[-1];
                joined->vertices[joined->numVertices + 1] = rest->vertices[0];
                joined->numVertices += 2;
            } else {
                RxVertexIndex *end = &joined->vertices[joined->numVertices];

                *end = end[-1];
                joined->vertices[joined->numVertices + 1] = rest->vertices[0];
                joined->vertices[joined->numVertices + 2] = rest->vertices[0];
                joined->numVertices += 3;
            }
        }

        for (i = 0; i < strip->numVertices; i++) {
            joined->vertices[joined->numVertices] = strip->vertices[i];
            joined->numVertices++;
        }
        RwFree(strip->vertices);
        strip->vertices = NULL;
        if (rest == strip) {
            rest = rest->next;
            RwFreeListFree(RPMESHGLOBAL(triStripListEntryFreeList), strip);
        } else {
            TriStripList *before = rest;

            while (before->next != strip) {
                before = before->next;
            }
            before->next = strip->next;
            RwFreeListFree(RPMESHGLOBAL(triStripListEntryFreeList), strip);
        }
    }
    *stripList = joined;
    joined->next = NULL;
    return TRUE;
}

/* Builds triangle strips from a build mesh: one mesh per material, joined into a strip each */
RpMeshHeader *fn_8045E904(RpBuildMesh *buildMesh, RwInt32 firstTrial, RwBool keepWinding) {
    RpBuildMeshTriangle **sorted;
    RwUInt16 numMeshes = 0;
    MaterialRun *run;
    RwUInt32 numMaterials;
    RwUInt32 numRuns;
    RwUInt32 i;
    RwUInt32 j;
    RpMesh **meshes;
    MaterialRun *runs;
    RwUInt32 numIndices;
    RwUInt32 size;
    RpMeshHeader *meshHeader;
    RpMesh *mesh;
    RxVertexIndex *indices;
    TriStripList *stripList;
    RpBuildMeshTriangle **source;

    sorted = RwMalloc(buildMesh->numTriangles * sizeof(RpBuildMeshTriangle *),
                      rwMEMHINTDUR_FUNCTION | rwID_MESHMODULE);
    if (sorted == NULL) {
        return NULL;
    }
    for (i = 0; i < buildMesh->numTriangles; i++) {
        sorted[i] = &buildMesh->meshTriangles[i];
    }
    qsort(sorted, buildMesh->numTriangles, sizeof(RpBuildMeshTriangle *), fn_8045CA38);

    numMaterials = 1;
    if (buildMesh->numTriangles >= 2) {
        RpMaterial *material = sorted[0]->material;

        for (i = 1; i < buildMesh->numTriangles; i++) {
            if (sorted[i]->material != material) {
                material = sorted[i]->material;
                numMaterials++;
            }
        }
    }
    meshes = RwMalloc(numMaterials * sizeof(RpMesh *),
                      rwMEMHINTFLAG_RESIZABLE | rwMEMHINTDUR_FUNCTION | rwID_MESHMODULE);
    runs = RwMalloc(numMaterials * sizeof(MaterialRun), rwMEMHINTDUR_FUNCTION | rwID_MESHMODULE);

    numRuns = 1;
    runs[0].material = sorted[0]->material;
    runs[0].numTriangles = 0;
    runs[0].field_0x0 = NULL;
    if (buildMesh->numTriangles >= 2) {
        run = &runs[1];
        for (i = 0; i < buildMesh->numTriangles - 1; i++) {
            if (sorted[i]->material != sorted[i + 1]->material) {
                run->material = sorted[i + 1]->material;
                numRuns++;
                run->numTriangles = i + 1;
                run->field_0x0 = NULL;
                run[-1].numTriangles = (i + 1) - run[-1].numTriangles;
                run++;
            }
        }
    }
    runs[numRuns - 1].numTriangles = buildMesh->numTriangles - runs[numRuns - 1].numTriangles;

    RPMESHGLOBAL(triStripListEntryFreeList) =
        fn_8047FBD8(sizeof(TriStripList), buildMesh->numTriangles / 10 + 5, 4,
                    rwMEMHINTDUR_FUNCTION | rwID_MESHMODULE);
    source = sorted;
    run = runs;
    stripList = NULL;
    for (i = 0; i < numRuns; i++) {
        RpBuildMeshTriangle *triangles = RwMalloc(run->numTriangles * sizeof(RpBuildMeshTriangle),
                                                  rwMEMHINTDUR_FUNCTION | rwID_MESHMODULE);
        TriStripList *strip;

        for (j = 0; j < run->numTriangles; j++) {
            triangles[j] = **source;
            source++;
        }
        fn_8045D898(triangles, run->numTriangles, &stripList, firstTrial);
        fn_8045E51C(&stripList, keepWinding);

        for (strip = stripList; strip != NULL; strip = strip->next) {
            RpMesh *stripMesh =
                RwMalloc(sizeof(RpMesh) + strip->numVertices * sizeof(RxVertexIndex),
                         rwMEMHINTDUR_FUNCTION | rwID_MESHMODULE);

            stripMesh->material = run->material;
            stripMesh->numIndices = strip->numVertices;
            stripMesh->indices = (RxVertexIndex *)(stripMesh + 1);
            for (j = 0; j < stripMesh->numIndices; j++) {
                stripMesh->indices[j] = strip->vertices[j];
            }
            meshes[numMeshes++] = stripMesh;
        }
        while (stripList != NULL) {
            strip = stripList;
            stripList = strip->next;
            RwFree(strip->vertices);
            strip->vertices = NULL;
            RwFreeListFree(RPMESHGLOBAL(triStripListEntryFreeList), strip);
        }
        RwFree(triangles);
        run++;
    }
    fn_8047FCDC(RPMESHGLOBAL(triStripListEntryFreeList));
    RPMESHGLOBAL(triStripListEntryFreeList) = NULL;

    size = sizeof(RpMeshHeader);
    numIndices = 0;
    for (i = 0; i < numMeshes; i++) {
        numIndices += meshes[i]->numIndices;
        size += sizeof(RpMesh) + meshes[i]->numIndices * sizeof(RxVertexIndex);
    }
    meshHeader = fn_8045BF30(size);
    meshHeader->flags = rpMESHHEADERTRISTRIP;
    meshHeader->numMeshes = numMeshes;
    meshHeader->serialNum = RPMESHGLOBAL(nextSerialNum);
    meshHeader->firstMeshOffset = 0;
    meshHeader->totalIndicesInMesh = numIndices;
    RPMESHGLOBAL(nextSerialNum)++;

    mesh = (RpMesh *)(meshHeader + 1);
    indices = (RxVertexIndex *)(mesh + numMeshes);
    for (i = 0; i < numMeshes; i++) {
        mesh->indices = indices;
        mesh->numIndices = meshes[i]->numIndices;
        mesh->material = meshes[i]->material;
        memcpy(indices, meshes[i]->indices, meshes[i]->numIndices * sizeof(RxVertexIndex));
        indices += mesh->numIndices;
        mesh++;
        RwFree(meshes[i]);
        meshes[i] = NULL;
    }
    RwFree(sorted);
    RwFree(meshes);
    RwFree(runs);
    return meshHeader;
}

/* The default triangle strip generator */
RpMeshHeader *fn_8045EF3C(RpBuildMesh *buildMesh, void *data) {
    return fn_8045E904(buildMesh, 0, TRUE);
}

/* Builds triangle lists from a build mesh: one mesh per material */
RpMeshHeader *fn_8045EF64(RpBuildMesh *buildMesh, void *data) {
    RpBuildMeshTriangle **sorted;
    RwUInt32 numMeshes;
    RwUInt32 size;
    RpMeshHeader *meshHeader;
    RpMesh *mesh;
    RxVertexIndex *indices;
    RwUInt32 i;

    sorted = RwMalloc(buildMesh->numTriangles * sizeof(RpBuildMeshTriangle *),
                      rwMEMHINTDUR_FUNCTION | rwID_MESHMODULE);
    if (sorted != NULL) {
        for (i = 0; i < buildMesh->numTriangles; i++) {
            sorted[i] = &buildMesh->meshTriangles[i];
        }
        qsort(sorted, buildMesh->numTriangles, sizeof(RpBuildMeshTriangle *), fn_8045CA38);

        numMeshes = 1;
        if (buildMesh->numTriangles >= 2) {
            RpMaterial *material = sorted[0]->material;

            for (i = 1; i < buildMesh->numTriangles; i++) {
                if (sorted[i]->material != material) {
                    material = sorted[i]->material;
                    numMeshes++;
                }
            }
        }

        size = sizeof(RpMeshHeader) + numMeshes * sizeof(RpMesh) +
               buildMesh->numTriangles * (3 * sizeof(RxVertexIndex));
        meshHeader = fn_8045BF30(size);
        if (meshHeader == NULL) {
            RwFree(sorted);
            RWERROR((E_RW_NOMEM, size));
            return NULL;
        }
        meshHeader->flags = 0;
        meshHeader->numMeshes = 1;
        meshHeader->serialNum = RPMESHGLOBAL(nextSerialNum);
        meshHeader->firstMeshOffset = 0;
        meshHeader->totalIndicesInMesh = buildMesh->numTriangles * 3;
        RPMESHGLOBAL(nextSerialNum)++;

        mesh = (RpMesh *)(meshHeader + 1);
        indices = (RxVertexIndex *)(mesh + numMeshes);
        mesh->numIndices = 0;
        mesh->material = sorted[0]->material;
        mesh->indices = indices;
        i = 0;
        do {
            if (sorted[i]->material != mesh->material) {
                mesh[1].numIndices = 0;
                mesh[1].material = sorted[i]->material;
                mesh++;
                mesh->indices = indices;
                meshHeader->numMeshes++;
            }
            i++;
            indices[0] = sorted[i - 1]->vertIndex[0];
            indices[1] = sorted[i - 1]->vertIndex[1];
            indices[2] = sorted[i - 1]->vertIndex[2];
            indices += 3;
            mesh->numIndices += 3;
        } while (i < buildMesh->numTriangles);

        fn_8045C4E4(meshHeader, fn_8045CBD4, NULL);
        RwFree(sorted);
        return meshHeader;
    }
    RWERROR((E_RW_NOMEM, buildMesh->numTriangles * sizeof(RpBuildMeshTriangle *)));
    return NULL;
}

/* The mesh header of a build mesh, as triangle strips or a triangle list; it destroys the build
 * mesh */
RpMeshHeader *fn_8045F1FC(RpBuildMesh *buildMesh, RwUInt32 flags) {
    if (buildMesh != NULL) {
        RpTriStripMeshCallBack callBack;
        void *data;
        RpMeshHeader *meshHeader;

        if (buildMesh->numTriangles == 0) {
            fn_8045C26C(buildMesh);
            return &lbl_8056D868.emptyMeshHeader;
        }
        if (flags & rpGEOMETRYTRISTRIP) {
            callBack = lbl_8056D868.triStripMethod.callBack;
            data = lbl_8056D868.triStripMethod.data;
        } else {
            callBack = fn_8045EF64;
            data = NULL;
        }
        meshHeader = callBack(buildMesh, data);
        if (meshHeader != NULL) {
            fn_8045C26C(buildMesh);
            return meshHeader;
        }
    }
    return NULL;
}
