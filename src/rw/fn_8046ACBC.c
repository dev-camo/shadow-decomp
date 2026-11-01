/* RenderWare GameCube display lists: the bytes of a vertex in a display list for a vertex
 * descriptor, the size of a display list, and preparing a mesh's display list record. The
 * original file name is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>
#include <string.h>

/* The bytes of a vertex attribute sent as the GX attribute type: its direct size, or an index */
#define ATTRIBUTEBYTES(stride, type, directSize)                                                   \
    do {                                                                                           \
        if ((type) == GX_DIRECT) {                                                                 \
            (stride) += (directSize);                                                              \
        } else if ((type) == GX_INDEX8) {                                                          \
            (stride) += 1;                                                                         \
        } else if ((type) == GX_INDEX16) {                                                         \
            (stride) += 2;                                                                         \
        }                                                                                          \
    } while (0)

/* The bytes of texture coordinate set n, whose type is at bit 2 * n of the VCD high register */
#define TEXCOORDBYTES(stride, builder, n)                                                          \
    do {                                                                                           \
        RwUInt32 type = ((builder)->vcd[1] >> (2 * (n))) & 3;                                      \
                                                                                                   \
        ATTRIBUTEBYTES(stride, type, fn_80466A54((builder), (n)));                                 \
    } while (0)

RwInt32 fn_8046ACBC(const RpGameCubeVertexDescBuilder *builder) {
    RwInt32 attribute;
    RwInt32 stride = 0;

    /* the vertex attributes, which come before the array attributes */
    for (attribute = GX_VA_PNMTXIDX; attribute < GX_POS_MTX_ARRAY; attribute++) {
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
            /* the matrix indices are one bit each and sent as a byte */
            if ((1 << attribute) & builder->vcd[0]) {
                stride++;
            }
            break;
        case GX_VA_POS: {
            RwUInt32 type = (builder->vcd[0] >> 9) & 3;

            ATTRIBUTEBYTES(stride, type, fn_8046693C(builder));
            break;
        }
        case GX_VA_NRM: {
            RwUInt32 vatA = builder->vat[0];
            RwUInt32 nbt = (vatA >> 9) & 1;
            RwUInt32 type = (builder->vcd[0] >> 11) & 3;

            if (nbt == GX_NRM_NBT) {
                /* binormals and tangents are indexed separately with the NBT3 index flag */
                RwInt32 count = 3;

                if ((vatA >> 31) == 0) {
                    count = 1;
                }
                if (type == GX_DIRECT) {
                    stride += count * fn_804669A0(builder);
                } else if (type == GX_INDEX8) {
                    stride += count;
                } else if (type == GX_INDEX16) {
                    stride += count * 2;
                }
            } else {
                ATTRIBUTEBYTES(stride, type, fn_804669A0(builder));
            }
            break;
        }
        case GX_VA_CLR0:
        case GX_VA_CLR1: {
            RwUInt32 type = (builder->vcd[0] >> ((attribute - GX_VA_CLR0) * 2 + 13)) & 3;

            ATTRIBUTEBYTES(stride, type, fn_804669EC(builder, attribute - GX_VA_CLR0));
            break;
        }
        case GX_VA_TEX0:
            TEXCOORDBYTES(stride, builder, 0);
            break;
        case GX_VA_TEX1:
            TEXCOORDBYTES(stride, builder, 1);
            break;
        case GX_VA_TEX2:
            TEXCOORDBYTES(stride, builder, 2);
            break;
        case GX_VA_TEX3:
            TEXCOORDBYTES(stride, builder, 3);
            break;
        case GX_VA_TEX4:
            TEXCOORDBYTES(stride, builder, 4);
            break;
        case GX_VA_TEX5:
            TEXCOORDBYTES(stride, builder, 5);
            break;
        case GX_VA_TEX6:
            TEXCOORDBYTES(stride, builder, 6);
            break;
        case GX_VA_TEX7:
            TEXCOORDBYTES(stride, builder, 7);
            break;
        }
    }
    return stride;
}

/* The size of a display list of the primitives and indices, rounded up to 32 bytes: each
 * primitive has a 3-byte header and each index a vertex */
RwUInt32 fn_8046B094(const RpGameCubeVertexDescBuilder *builder, RwInt32 numPrimitives,
                     RwUInt32 numIndices) {
    RwUInt32 stride = fn_8046ACBC(builder);
    RwUInt32 size = numPrimitives * 3 + numIndices * stride;

    size = (size + 31) & ~31;
    return size;
}

/* Prepares a mesh's display list record over cleared data; the mesh index is not used */
void fn_8046B0E0(void *displayListRecord, RwInt32 meshIndex, RwUInt32 size, void *data) {
    RpGameCubeDisplayList *record = displayListRecord;

    record->data = data;
    record->size = size;
    memset(data, 0, size);
}
