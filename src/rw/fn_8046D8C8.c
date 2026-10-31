/* RenderWare GameCube display list filling: splitting a mesh's indices into triangle strips at
 * degenerate triangles, and writing a mesh's display list: a 3-byte header per strip followed by
 * each vertex's attributes, as indices or as direct data. The original file name is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>

/* Whether the triangle of three indices has distinct vertices */
#define TRIANGLEVALID(a, b, c) ((a) != (b) && (a) != (c) && (b) != (c))

/* Counts the triangle strips of a mesh's indices and the indices they take: a strip continues
 * while its triangles are valid, and with the flag a degenerate triangle between two strips of
 * the same winding is kept */
void fn_8046D8C8(RxVertexIndex *indices, RwUInt32 numIndices, RwInt32 *numStrips,
                 RwUInt32 *numStripIndices, RwInt32 flags) {
    RwUInt32 run;
    RwUInt32 i;

    *numStrips = 0;
    *numStripIndices = 0;
    run = 0;
    for (i = 0; i < numIndices - 2; indices++, i++) {
        if (TRIANGLEVALID(indices[0], indices[1], indices[2])) {
            if (run == 0 && (i & 1) && flags) {
                run = 2;
            } else {
                run++;
            }
        } else if (run != 0) {
            if (flags && i < numIndices - 3) {
                if (TRIANGLEVALID(indices[1], indices[2], indices[3])) {
                    if (((i + 1) & 1) == 0) {
                        *numStripIndices += run + 2;
                        run = 0;
                        (*numStrips)++;
                    } else {
                        run++;
                    }
                } else {
                    *numStripIndices += run + 2;
                    run = 0;
                    (*numStrips)++;
                }
            } else {
                *numStripIndices += run + 2;
                run = 0;
                (*numStrips)++;
            }
        }
    }
    if (run != 0) {
        *numStripIndices += run + 2;
        (*numStrips)++;
    }
}

/* Writes count indices of the GX index type at a stride */
#define WRITEINDICES(destination, source, count, stride, type)                                     \
    do {                                                                                           \
        RwUInt8 *dst_ = (destination);                                                             \
        RxVertexIndex *src_ = (source);                                                            \
        RwUInt32 k_;                                                                               \
                                                                                                   \
        if ((type) == GX_INDEX8) {                                                                 \
            for (k_ = 0; k_ < (count); k_++) {                                                     \
                *dst_ = *src_;                                                                     \
                src_++;                                                                            \
                dst_ += (stride);                                                                  \
            }                                                                                      \
        } else if ((type) == GX_INDEX16) {                                                         \
            for (k_ = 0; k_ < (count); k_++) {                                                     \
                *(RwUInt16 *)dst_ = *src_;                                                         \
                src_++;                                                                            \
                dst_ += (stride);                                                                  \
            }                                                                                      \
        }                                                                                          \
    } while (0)

/* Ends the current strip at index i: writes its indices after its header */
#define ENDSTRIPINDICES()                                                                          \
    do {                                                                                           \
        WRITEINDICES(data + (headers + written * stride), attributeIndices + (i - run), run + 2,   \
                     stride, type);                                                                \
        written += run + 2;                                                                        \
        run = 0;                                                                                   \
        numStrips++;                                                                               \
        headers += 3;                                                                              \
    } while (0)

/* Writes one attribute's indices into a mesh's display list, split into the triangle strips that
 * the position indices give, each after its 3-byte header */
void fn_8046DA24(RxVertexIndex *stripIndices, RxVertexIndex *attributeIndices, RwUInt32 numIndices,
                 RwInt32 triStrip, RwInt32 stride, RwUInt32 type, RwInt32 flag, RwUInt8 *data) {
    RxVertexIndex *indices = stripIndices;
    RwInt32 numStrips = 0;
    RwUInt32 headers = 0;
    RwUInt32 written = 0;
    RwUInt32 run = 0;
    RwUInt32 i;

    if (triStrip) {
        for (i = 0; i < numIndices - 2; indices++, i++) {
            if (TRIANGLEVALID(indices[0], indices[1], indices[2])) {
                if (run == 0 && (i & 1) && flag) {
                    run = 2;
                } else {
                    run++;
                }
            } else if (run != 0) {
                if (flag && i < numIndices - 3) {
                    if (TRIANGLEVALID(indices[1], indices[2], indices[3])) {
                        if (((i + 1) & 1) == 0) {
                            ENDSTRIPINDICES();
                        } else {
                            run++;
                        }
                    } else {
                        ENDSTRIPINDICES();
                    }
                } else {
                    ENDSTRIPINDICES();
                }
            }
        }
        if (run != 0) {
            WRITEINDICES(data + (numStrips * 3 + written * stride), attributeIndices + (i - run),
                         run + 2, stride, type);
        }
    } else {
        WRITEINDICES(data, attributeIndices, numIndices, stride, type);
    }
}

/* Ends the current strip at index i: writes its header */
#define ENDSTRIPHEADER()                                                                           \
    do {                                                                                           \
        RwUInt8 *header_ = data + headers + written * stride;                                      \
                                                                                                   \
        written += run + 2;                                                                        \
        header_[0] = primitive;                                                                    \
        *(RwUInt16 *)(header_ + 1) = run + 2;                                                      \
        run = 0;                                                                                   \
        numStrips++;                                                                               \
        headers += 3;                                                                              \
    } while (0)

/* Writes the 3-byte headers of a mesh's display list: the GX primitive and its vertex count, for
 * each triangle strip or for the whole mesh */
void fn_8046E3E0(RxVertexIndex *indices, RwUInt32 numIndices, RwInt32 triStrip, RwInt32 stride,
                 RwInt32 primitive, RwInt32 flag, RwUInt8 *data) {
    RwUInt32 run = 0;
    RwUInt32 headers = 0;
    RwInt32 numStrips = 0;
    RwUInt32 written = 0;
    RwUInt32 i;

    if (triStrip) {
        for (i = 0; i < numIndices - 2; indices++, i++) {
            if (TRIANGLEVALID(indices[0], indices[1], indices[2])) {
                if (run == 0 && (i & 1) && flag) {
                    run = 2;
                } else {
                    run++;
                }
            } else if (run != 0) {
                if (flag && i < numIndices - 3) {
                    if (TRIANGLEVALID(indices[1], indices[2], indices[3])) {
                        if (((i + 1) & 1) == 0) {
                            ENDSTRIPHEADER();
                        } else {
                            run++;
                        }
                    } else {
                        ENDSTRIPHEADER();
                    }
                } else {
                    ENDSTRIPHEADER();
                }
            }
        }
        if (run != 0) {
            RwUInt8 *header = data + numStrips * 3 + written * stride;

            header[0] = primitive;
            *(RwUInt16 *)(header + 1) = run + 2;
        }
    } else {
        data[0] = primitive;
        *(RwUInt16 *)(data + 1) = numIndices;
    }
}

/* Writes the indices of an indexed attribute into the display list and adds the bytes they take
 * per vertex (one for GX_INDEX8, two for GX_INDEX16) to offset; the direct attributes are written
 * by the callers */
#define FILLINDEXED(offset, type)                                                                  \
    do {                                                                                           \
        if ((type) != GX_NONE) {                                                                   \
            fn_8046DA24(indexSource->indices[GX_VA_POS], indexSource->indices[attribute],          \
                        numIndices, triStrip, stride, (type), flag, position);                     \
            if ((type) == GX_INDEX8) {                                                             \
                (offset) += 1;                                                                     \
            } else if ((type) == GX_INDEX16) {                                                     \
                (offset) += 2;                                                                     \
            }                                                                                      \
        }                                                                                          \
    } while (0)

/* Fills texture coordinate set n, whose descriptor type is at bit 2 * n of the VCD high register,
 * with its component type and the fixed point scale from the VAT */
#define FILLTEXCOORDS(offset, n, compType, scale)                                                  \
    do {                                                                                           \
        RwUInt32 type = (builder->vcd[1] >> (2 * (n))) & 3;                                        \
                                                                                                   \
        if (type == GX_DIRECT) {                                                                   \
            fn_8046A270(position, indexSource->indices[attribute], (compType), numIndices, stride, \
                        (scale));                                                                  \
            (offset) += fn_80466A54(builder, n);                                                   \
        } else {                                                                                   \
            FILLINDEXED(offset, type);                                                             \
        }                                                                                          \
    } while (0)

/* The fixed point scale of a texture coordinate set from its VAT fraction field. Except for set 0
 * the original masks the scale instead of the field, (1 << (vat >> shift)) & 0x1F (sic). */
#define TEXSCALE(vat, shift) ((1 << ((vat) >> (shift))) & 0x1F)

/* Writes a mesh's display list: the strip headers, then each vertex's attributes in GX order */
void fn_8046E594(RpGameCubeVertexDescBuilder *builder, void *displayListRecord,
                 RpGameCubeIndexSource *indexSource, RwUInt32 numIndices, RwInt32 triStrip,
                 RwInt32 stride, RwInt32 flag, RwInt32 primitive, const RwV3d *origin) {
    RpGameCubeDisplayList *record = displayListRecord;
    RwUInt32 offset;
    RwUInt32 attribute;

    fn_8046E3E0(indexSource->indices[GX_VA_POS], numIndices, triStrip, stride, primitive, flag,
                record->data);
    offset = 3;
    /* the vertex attributes, which come before the array attributes */
    for (attribute = GX_VA_PNMTXIDX; attribute < GX_POS_MTX_ARRAY; attribute++) {
        RwUInt8 *position = (RwUInt8 *)record->data + offset;

        switch (attribute) {
        case GX_VA_PNMTXIDX:
        case GX_VA_TEX0MTXIDX:
        case GX_VA_TEX1MTXIDX:
        case GX_VA_TEX2MTXIDX:
        case GX_VA_TEX3MTXIDX:
        case GX_VA_TEX4MTXIDX:
        case GX_VA_TEX5MTXIDX:
        case GX_VA_TEX6MTXIDX:
        case GX_VA_TEX7MTXIDX:
            if ((1 << attribute) & builder->vcd[0]) {
                fn_8046DA24(indexSource->indices[GX_VA_POS], indexSource->indices[attribute],
                            numIndices, triStrip, stride, GX_INDEX8, flag, position);
                offset++;
            }
            break;
        case GX_VA_POS: {
            RwUInt32 type = (builder->vcd[0] >> 9) & 3;

            if (type == GX_DIRECT) {
                RwUInt32 vat = builder->vat[0];

                if ((vat & 1) == GX_POS_XYZ) {
                    fn_80466B58(position, indexSource->indices[attribute], (vat >> 1) & 7,
                                numIndices, stride, origin, 1 << ((vat >> 4) & 0x1F));
                    offset += fn_8046693C(builder);
                }
            } else {
                FILLINDEXED(offset, type);
            }
            break;
        }
        case GX_VA_NRM: {
            RwUInt32 vat = builder->vat[0];
            RwUInt32 type = (builder->vcd[0] >> 11) & 3;

            if (((vat >> 9) & 1) == GX_NRM_NBT) {
                if (type == GX_DIRECT) {
                    offset += fn_804669A0(builder) * 3;
                } else if (type != GX_NONE) {
                    RwInt32 count;

                    if ((vat >> 31) == 0) {
                        count = 1;
                        fn_8046DA24(indexSource->indices[GX_VA_POS],
                                    indexSource->indices[attribute], numIndices, triStrip, stride,
                                    type, flag, position);
                    } else {
                        count = 3;
                    }
                    if (type == GX_INDEX8) {
                        offset += count;
                    } else if (type == GX_INDEX16) {
                        offset += count * 2;
                    }
                }
            } else if (type == GX_DIRECT) {
                fn_80467BC8(position, indexSource->indices[attribute], (vat >> 10) & 7, numIndices,
                            stride);
                offset += fn_804669A0(builder);
            } else {
                FILLINDEXED(offset, type);
            }
            break;
        }
        case GX_VA_CLR0:
        case GX_VA_CLR1: {
            RwInt32 channel = attribute - GX_VA_CLR0;
            RwUInt32 type = (builder->vcd[0] >> (channel * 2 + 13)) & 3;

            /* Matching workaround: the channel is recomputed from the attribute for the size
             * below. Passing `channel` makes fn_8046E594 an instruction shorter (0x8F4 against
             * 0x8F8 bytes) and it then differs in 496 words instead of 322. Revisit when the
             * function matches or the original form of the size call is found. */
            if (type == GX_DIRECT) {
                fn_804699F0(position, indexSource->indices[attribute],
                            (builder->vat[0] >> (channel * 4 + 14)) & 7, numIndices, stride);
                offset += fn_804669EC(builder, attribute - GX_VA_CLR0);
            } else {
                FILLINDEXED(offset, type);
            }
            break;
        }
        case GX_VA_TEX0:
            FILLTEXCOORDS(offset, 0, (builder->vat[0] >> 22) & 7,
                          1 << ((builder->vat[0] >> 25) & 0x1F));
            break;
        case GX_VA_TEX1:
            FILLTEXCOORDS(offset, 1, (builder->vat[1] >> 1) & 7, TEXSCALE(builder->vat[1], 4));
            break;
        case GX_VA_TEX2:
            FILLTEXCOORDS(offset, 2, (builder->vat[1] >> 10) & 7, TEXSCALE(builder->vat[1], 13));
            break;
        case GX_VA_TEX3:
            FILLTEXCOORDS(offset, 3, (builder->vat[1] >> 19) & 7, TEXSCALE(builder->vat[1], 22));
            break;
        case GX_VA_TEX4:
            FILLTEXCOORDS(offset, 4, (builder->vat[1] >> 28) & 7, TEXSCALE(builder->vat[2], 0));
            break;
        case GX_VA_TEX5:
            FILLTEXCOORDS(offset, 5, (builder->vat[2] >> 6) & 7, TEXSCALE(builder->vat[2], 9));
            break;
        case GX_VA_TEX6:
            FILLTEXCOORDS(offset, 6, (builder->vat[2] >> 15) & 7, TEXSCALE(builder->vat[2], 18));
            break;
        case GX_VA_TEX7:
            FILLTEXCOORDS(offset, 7, (builder->vat[2] >> 24) & 7, TEXSCALE(builder->vat[2], 27));
            break;
        }
    }
}
