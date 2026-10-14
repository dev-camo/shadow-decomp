/* RenderWare GameCube morph targets: writing an atomic's interpolated positions and normals into
 * its instanced data with quantized paired-single loads and stores. The original file name is
 * unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>
#include <dolphin/os.h>

/* GX component type to paired-single quantization type, and the component sizes */
RwUInt8 lbl_805EDDE0[5] = {4, 6, 5, 7, 0};
RwUInt8 lbl_805EDDE8[5] = {1, 1, 2, 2, 4};

asm void fn_804649C0(register void *dst, register const RwV3d *start, register const RwV3d *end,
                     register const RwReal *t, register RwInt32 numVertices,
                     register RwInt32 srcSize, register RwInt32 dstSize, register RwInt32 padding);

/* Writes an atomic's positions, and its normals, interpolated between its interpolator's start
 * and end morph targets into its instanced data, in the vertex format's component types */
void fn_80464814(RpGeometry *geometry, RpGameCubeInstanceData *instance,
                 RpInterpolator *interpolator) {
    void *data = instance->vertexDescriptors[0].data;
    RpMorphTarget *start = &geometry->morphTarget[interpolator->startMorphTarget];
    RpMorphTarget *end = &geometry->morphTarget[interpolator->endMorphTarget];
    RwReal t = interpolator->recipTime * interpolator->position;
    RpGameCubeVertexFormat *format = GAMECUBEVERTEXFORMAT(geometry);
    register RwUInt32 gqr6 = 0; /* asm operands need registers */
    RwInt32 size;
    RwInt32 padding;

    if (format != NULL) {
        gqr6 = lbl_805EDDE0[format->posType] | (format->posFrac << 8);
        size = lbl_805EDDE8[format->posType];
    } else {
        size = 4;
    }
    asm { mtspr GQR6, gqr6 }
    fn_804649C0(data, start->verts, end->verts, &t, geometry->numVertices, 4, size, 0);
    DCFlushRange(instance->vertexDescriptors[0].data, geometry->numVertices * (size * 3));

    if (geometry->flags & rpGEOMETRYNORMALS) {
        void *normals = instance->vertexDescriptors[1].data;
        register RwUInt32 normalGqr6 = 0;

        format = GAMECUBEVERTEXFORMAT(geometry);
        if (format != NULL) {
            /* GX component type to quantization type, component size, and the scale of
             * normals; binormals and tangents follow each normal */
            RwUInt8 quantizeType[5] = {4, 6, 5, 7, 0};
            RwUInt8 typeSize[5] = {1, 1, 2, 2, 4};
            RwUInt8 normalScale[5] = {0, 6, 0, 14, 0};

            normalGqr6 = quantizeType[format->normalType] | (normalScale[format->normalType] << 8);
            size = typeSize[format->normalType];
            padding = format->normalNbt ? size * 6 : 0;
        } else {
            size = 4;
            padding = 0;
        }
        asm { mtspr GQR6, normalGqr6 }
        fn_804649C0(normals, start->normals, end->normals, &t, geometry->numVertices, 4, size,
                    padding);
        DCFlushRange(instance->vertexDescriptors[1].data,
                     geometry->numVertices * (padding + size * 3));
    }
    GXInvalidateVtxCache();
}

/* Writes numVertices three-component vectors interpolated between start and end by *t to dst,
 * reading floats and writing in the GQR6 quantization. srcSize is the size of a source component,
 * dstSize that of a destination component, and padding is skipped after each written vector. Two
 * vectors are interpolated per loop iteration, after a single one for an odd count. */
asm void fn_804649C0(register void *dst, register const RwV3d *start, register const RwV3d *end,
                     register const RwReal *t, register RwInt32 numVertices,
                     register RwInt32 srcSize, register RwInt32 dstSize, register RwInt32 padding) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    andi. r11, numVertices, 0x1
    srawi numVertices, numVertices, 1
    lfs f0, 0(t)
    rotlwi r11, srcSize, 1
    rotlwi r12, dstSize, 1
    subf dst, r12, dst
    subf start, r11, start
    subf end, r11, end
    beq pairs
    psq_lux f1, start, r11, 1, 6
    psq_lux f2, start, srcSize, 0, 6
    psq_lux f3, end, r11, 1, 6
    psq_lux f4, end, srcSize, 0, 6
    ps_sub f3, f3, f1
    ps_sub f4, f4, f2
    ps_madd f1, f3, f0, f1
    ps_madd f2, f4, f0, f2
    psq_stux f1, dst, r12, 1, 6
    psq_stux f2, dst, dstSize, 0, 6
    add dst, dst, padding
pairs:
    cmpwi numVertices, 0
    mtctr numVertices
    blelr
    psq_lux f1, start, r11, 0, 6
    psq_lux f2, start, r11, 0, 6
    psq_lux f3, start, r11, 0, 6
    psq_lux f4, end, r11, 0, 6
    psq_lux f5, end, r11, 0, 6
    psq_lux f6, end, r11, 0, 6
    ps_sub f4, f4, f1
    ps_sub f5, f5, f2
    ps_sub f6, f6, f3
loop:
    ps_madd f7, f4, f0, f1
    psq_lux f1, start, r11, 0, 6
    psq_lux f4, end, r11, 0, 6
    psq_stux f7, dst, r12, 0, 6
    ps_madd f8, f5, f0, f2
    psq_lux f2, start, r11, 0, 6
    psq_lux f5, end, r11, 0, 6
    ps_madd f9, f6, f0, f3
    psq_lux f3, start, r11, 0, 6
    psq_lux f6, end, r11, 0, 6
    psq_stux f8, dst, r12, 1, 6
    add dst, dst, padding
    ps_merge10 f8, f8, f8
    psq_stux f8, dst, dstSize, 1, 6
    psq_stux f9, dst, dstSize, 0, 6
    add dst, dst, padding
    ps_sub f4, f4, f1
    ps_sub f5, f5, f2
    ps_sub f6, f6, f3
    bdnz loop
    blr
#endif // clang-format on
}
