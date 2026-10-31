/* RenderWare 2D toolkit: reading paths from streams, stroking paths with a brush, and the free
 * lists of three more kinds of toolkit object. The original file name is unknown. */
#include "rw/rt2d.h"

/* The object IDs of the memory hints (inferred) */
#define RT2DMEMID 0x190
#define RT2DPATHMEMID 0x1A1
#define RT2DOBJECTMEMID 0x1A3

/* The chunk ID of a path */
#define rwID_2DPATH 0x1A1

/* The stream path flags */
#define rt2dSTREAMPATHCLOSED 0x1
#define rt2dSTREAMPATHFLAT 0x2
#define rt2dSTREAMPATHNEXT 0x4

RwFreeList lbl_805E26D0; /* the free list of 0x100-byte objects */
RwFreeList lbl_805E26F8; /* the free list of 8-byte objects */
RwFreeList lbl_805E271C; /* the free list of 0x14-byte objects */

RwInt32 lbl_805EDE58 = 32; /* entries per block of the 0x100-byte objects */
RwInt32 lbl_805EDE5C = 1;  /* blocks to preallocate of the 0x100-byte objects */
RwInt32 lbl_805EDE60 = 32; /* entries per block of the 0x14-byte objects */
RwInt32 lbl_805EDE64 = 1;  /* blocks to preallocate of the 0x14-byte objects */
RwInt32 lbl_805EDE68 = 32; /* entries per block of the 8-byte objects */
RwInt32 lbl_805EDE6C = 1;  /* blocks to preallocate of the 8-byte objects */

/* Reads a subpath's points after its stream header, then the subpaths that follow it */
Rt2dPath *fn_80476ABC(Rt2dPath *path, rt2dStreamPath *header, RwStream *stream) {
    rt2dPathNode *points;
    RwInt32 i;
    Rt2dPath *subpath;

    fn_804760C8(path);
    path->curr = path;
    path->numPoints = header->numPoints;
    path->closed = header->flags & rt2dSTREAMPATHCLOSED;
    path->flat = (header->flags & rt2dSTREAMPATHFLAT) != 0;
    path->field_0x10 = header->inset;
    points = RwMalloc(path->numPoints * sizeof(rt2dPathNode), rwMEMHINTDUR_EVENT | RT2DPATHMEMID);
    if (points == NULL) {
        return NULL;
    }
    if (path->numPoints != 0) {
        if (path->numPoints * sizeof(rt2dPathNode) !=
            fn_80480D4C(stream, points, path->numPoints * sizeof(rt2dPathNode))) {
            return NULL;
        }
        fn_8047D158(points, path->numPoints * sizeof(rt2dPathNode));
    }
    for (i = 0; i < header->numPoints; i++) {
        rt2dPathNode *node = &points[i];

        RwMemFloat32ToReal(&node->pos, sizeof(node->pos));
        RwMemFloat32ToReal(&node->normal, sizeof(node->normal));
        RwMemFloat32ToReal(&node->dist, sizeof(node->dist));
    }
    path->points = points;
    if (header->flags & rt2dSTREAMPATHNEXT) {
        if (fn_8047CF50(stream, rwID_2DPATH, NULL, NULL)) {
            path->next = fn_80476C4C(stream);
        } else {
            return NULL;
        }
    }
    for (subpath = path->next; subpath != NULL; subpath = subpath->next) {
        subpath->curr = path;
    }
    return path;
}

/* Reads a path and the subpaths that follow it from a stream,
 * discarding the new path if reading fails */
Rt2dPath *fn_80476C4C(RwStream *stream) {
    Rt2dPath *result;
    Rt2dPath *path;
    rt2dStreamPath header;

    path = fn_80476490();
    result = path;
    if (fn_80480D4C(stream, &header, sizeof(header)) != sizeof(header)) {
        result = NULL;
    } else {
        fn_8047D254(&header, sizeof(header));
        switch (header.type) {
        case 1:
            result = fn_80476ABC(path, &header, stream);
            break;
        }
    }
    if (result == NULL) {
        fn_80475E2C(path);
    }
    return result;
}

/* Interpolates a brush edge's colour at a fraction of the path length */
#define STROKECOLOR(vertex, base, delta, t)                                                        \
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

/* Sets a stroke vertex's colour and texture coordinates from a brush edge */
#define STROKEVERTEX(vertex, edge, dedge, dist)                                                    \
    do {                                                                                           \
        if (brush->flag & FIELDRGBA) {                                                             \
            STROKECOLOR(vertex, brush->edge.col, brush->dedge.col, (dist) * invLength);            \
        } else {                                                                                   \
            RwUInt8 red_ = brush->colorCache.red;                                                  \
            RwUInt8 green_ = brush->colorCache.green;                                              \
            RwUInt8 blue_ = brush->colorCache.blue;                                                \
            RwUInt8 alpha_ = brush->colorCache.alpha;                                              \
                                                                                                   \
            (vertex)->color.red = red_;                                                            \
            (vertex)->color.green = green_;                                                        \
            (vertex)->color.blue = blue_;                                                          \
            (vertex)->color.alpha = alpha_;                                                        \
        }                                                                                          \
        if (brush->flag & FIELDUV) {                                                               \
            RwReal t_ = (dist) * invLength;                                                        \
            RwV2d uv_;                                                                             \
                                                                                                   \
            uv_.x = brush->dedge.uv.x * t_;                                                        \
            uv_.y = brush->dedge.uv.y * t_;                                                        \
            (vertex)->u = brush->edge.uv.x + uv_.x;                                                \
            (vertex)->v = brush->edge.uv.y + uv_.y;                                                \
        }                                                                                          \
    } while (0)

/* Draws the triangle strip so far */
#define FLUSHSTROKE()                                                                              \
    do {                                                                                           \
        if (fn_8049A82C(lbl_805E1950.vertexBuffer, numVertices, ltm,                               \
                        lbl_805E1950.field_0x31C | (brush->texture != NULL)) != NULL) {            \
            fn_8049A95C(rwPRIMTYPETRISTRIP, lbl_805E1950.indices, numVertices);                    \
            fn_8049A90C();                                                                         \
        }                                                                                          \
    } while (0)

/* Strokes each subpath with the brush as a triangle strip along it, the brush's half width out
 * from the path's inset on each side, coloured and textured along the path length */
Rt2dPath *fn_80476CF8(Rt2dPath *path, Rt2dBrush *brush) {
    if (path != NULL) {
        RwMatrix *ltm = fn_804749CC();
        RwReal halfwidth = brush->halfwidth;
        RwReal layerDepth = lbl_805E1950.layerDepth;
        RwReal inset = path->field_0x10;
        Rt2dPath *flat = path;
        rt2dPathNode *node;
        RwInt32 numPoints;
        RwIm3DVertex *vertex;
        RwInt32 numVertices;
        RwInt32 i;
        RwReal outer;
        RwReal inner;
        RwReal invLength;
        RwV2d offset;

        if (!path->flat) {
            flat = fn_8047548C();
            fn_804758C8(flat, path);
        }
        if (flat->numPoints == -1) {
            node = fn_8047DFAC(flat->points);
        } else {
            node = flat->points;
        }
        numPoints = flat->numPoints;
        if (numPoints == -1) {
            numPoints = fn_8047E03C(flat->points);
        }

        if (brush->texture != NULL) {
            RwUInt32 filterAddressing = brush->texture->filterAddressing;
            RwUInt32 filter = filterAddressing & 0xFF;
            RwUInt32 address = 0;
            RwUInt32 addressV = (filterAddressing >> 12) & 0xF;

            if (((filterAddressing >> 8) & 0xF) == addressV) {
                address = addressV;
            }
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, brush->texture->raster);
            RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)filter);
            RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void *)address);
        } else {
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
        }

        outer = halfwidth + inset;
        inner = halfwidth - inset;
        vertex = lbl_805E1950.vertexBuffer;
        numVertices = 0;
        invLength = 1.0f / node[numPoints - 1].dist;
        for (i = 0; i < numPoints; i++, node++) {
            if (numVertices >= 62) {
                FLUSHSTROKE();
                vertex = lbl_805E1950.vertexBuffer;
                numVertices = 0;
                /* the next strip starts again from the previous point */
                i--;
                node--;
            }
            offset.x = node->normal.x * outer;
            offset.y = node->normal.y * outer;
            vertex[0].objVertex.x = node->pos.x - offset.x;
            vertex[0].objVertex.y = node->pos.y - offset.y;
            vertex[0].objVertex.z = layerDepth;
            STROKEVERTEX(&vertex[0], top, dtop, node->dist);
            offset.x = node->normal.x * inner;
            offset.y = node->normal.y * inner;
            vertex[1].objVertex.x = node->pos.x + offset.x;
            vertex[1].objVertex.y = node->pos.y + offset.y;
            vertex[1].objVertex.z = layerDepth;
            STROKEVERTEX(&vertex[1], bottom, dbottom, node->dist);
            vertex += 2;
            numVertices += 2;
        }
        if (numVertices > 0) {
            FLUSHSTROKE();
        }
        fn_80476CF8(path->next, brush);
    }
    return path;
}

Rt2dPath *fn_804771C0(Rt2dPath *path, Rt2dBrush *brush) {
    return fn_80476CF8(path, brush);
}

void fn_804771E0(void) {
    fn_8047FCDC(lbl_805E1950.field_0x16C);
    lbl_805E1950.field_0x16C = NULL;
}

RwBool fn_8047721C(void) {
    lbl_805E1950.field_0x16C = fn_8047FC04(0x100, lbl_805EDE58, 16, lbl_805EDE5C, &lbl_805E26D0,
                                           rwMEMHINTDUR_GLOBAL | RT2DOBJECTMEMID);
    return TRUE;
}

void fn_8047726C(void) {
    fn_8047FCDC(lbl_805E1950.field_0x174);
    fn_8047FCDC(lbl_805E1950.field_0x178);
    lbl_805E1950.field_0x174 = NULL;
    lbl_805E1950.field_0x178 = NULL;
}

RwBool fn_804772C4(void) {
    lbl_805E1950.field_0x174 = fn_8047FC04(8, lbl_805EDE68, 4, lbl_805EDE6C, &lbl_805E26F8,
                                           rwMEMHINTDUR_GLOBAL | RT2DMEMID);
    lbl_805E1950.field_0x178 = fn_8047FC04(0x14, lbl_805EDE60, 4, lbl_805EDE64, &lbl_805E271C,
                                           rwMEMHINTDUR_GLOBAL | RT2DMEMID);
    return TRUE;
}
