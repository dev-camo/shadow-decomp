/* RenderWare delta morph plugin (plugin ID 0x122), GameCube: the delta morph pipeline. Its
 * reinstance callback writes an atomic's morphed vertices, the geometry's base streams plus each
 * target's weighted deltas, into a vertex buffer that replaces the instance's streams and is
 * double buffered against the GPU. The positions and normals are written by three kernels that
 * are hand-written assembly rather than compiler output: they have no prologue or frame, return
 * early with hinted conditional returns, count down with raw bc encodings and compute in paired
 * singles with quantized update-form loads and stores. The original file name is unknown. */
#include "rw/rpdmorph.h"
#include <dolphin/gx.h>
#include <dolphin/os.h>

#define rpATOMIC 1 /* RwObject type of an atomic */

/* the plugin's pipelines, by the index fn_80445CE0 takes; only the second is ever created */
RxPipeline *lbl_805F1438[2];

/* the delta kernels (hand-written assembly) */
void fn_804459C0(void *dst, const void *base, const RwV3d *deltas, const RwReal *weight,
                 const rpDMorphRLE *rle, RwInt32 size, RwInt32 padding);
void fn_80445AE8(void *dst, const void *base, const RwV3d *deltas, const RwReal *weight,
                 const rpDMorphRLE *rle, RwInt32 size, RwInt32 padding);
void fn_80445C00(void *dst, const void *base, RwInt32 size, RwInt32 padding, RwInt32 numVertices);

/* Destroy callback of a delta morphed geometry's instance data, run before its resource entry is
 * freed: frees the vertex buffer the morphed streams are written to, or waits for the GPU when
 * there is none, and gives the instance back its base streams */
void fn_804448B8(RwResEntry *repEntry) {
    RpGameCubeInstanceData *instance = (RpGameCubeInstanceData *)(repEntry + 1);
    void *owner;
    RpGeometry *geometry;
    RpDMorphGeometry *dmorphGeometry;

    if (instance->vertexDescriptors[0].data != NULL) {
        fn_8048071C(DMORPHVERTEXBUFFERENTRY(instance->vertexDescriptors[0].data));
    } else {
        RPGAMECUBEWAITDRAWSYNC(instance->syncToken);
    }

    owner = repEntry->owner;
    if (((RwObject *)owner)->type == rpATOMIC) {
        geometry = ((RpAtomic *)owner)->geometry;
        dmorphGeometry = DMORPHGEOMETRY(geometry);
    } else {
        geometry = owner;
        dmorphGeometry = DMORPHGEOMETRY(geometry);
    }
    instance->vertexDescriptors[0].data = dmorphGeometry->baseStreams[0];
    if (geometry->flags & rpGEOMETRYNORMALS) {
        instance->vertexDescriptors[1].data = dmorphGeometry->baseStreams[1];
    }
    dmorphGeometry->baseStreams[0] = NULL;
}

/* Destroy callback of a vertex buffer: waits until the GPU is done with it */
void fn_804449A4(RwResEntry *repEntry) {
    DMorphVertexBuffer *buffer = (DMorphVertexBuffer *)(repEntry + 1);

    RPGAMECUBEWAITDRAWSYNC(buffer->syncToken);
}

/* Prepares the vertex buffer the morphed streams are written to: the instance's streams become the
 * base streams the first time, a buffer the GPU is still reading is replaced, and a new buffer is
 * allocated as a resource entry when there is none. Only the streams some target morphs get a
 * place in the buffer. */
RwBool fn_80444A2C(RpAtomic *atomic, RpGameCubeInstanceData *instance,
                   RpGameCubeRenderMesh *renderMesh, void **baseStreams, RwUInt32 geometryFlags,
                   RwUInt32 targetFlags) {
    RwInt32 numVertices;
    RpGeometry *geometry;
    RwUInt32 sizes[4];
    RwUInt32 size;
    RwInt32 index;
    RwResEntry *buffer;
    RwResEntry *ownerEntry;

    geometry = atomic->geometry;
    numVertices = geometry->numVertices;
    RwResourcesUseResEntry(renderMesh->repEntry);
    renderMesh->repEntry->destroyNotify = fn_804448B8;

    if (baseStreams[0] == NULL) {
        index = 0;
        baseStreams[index] = instance->vertexDescriptors[index].data;
        instance->vertexDescriptors[index].data = NULL;
        index++;
        if (geometryFlags & rpGEOMETRYNORMALS) {
            if (targetFlags & rpGEOMETRYNORMALS) {
                baseStreams[index] = instance->vertexDescriptors[index].data;
            }
            index++;
        }
        if (geometryFlags & rpGEOMETRYPRELIT) {
            if (targetFlags & rpGEOMETRYPRELIT) {
                baseStreams[index] = instance->vertexDescriptors[index].data;
            }
            index++;
        }
        if ((geometryFlags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) &&
            (targetFlags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2))) {
            baseStreams[index] = instance->vertexDescriptors[index].data;
        }
    }

    /* a buffer the GPU may still read is left to its entry */
    if (instance->vertexDescriptors[0].data != NULL &&
        !fn_8049A78C(DMORPHVERTEXBUFFER(instance->vertexDescriptors[0].data)->syncToken)) {
        DMORPHVERTEXBUFFERENTRY(instance->vertexDescriptors[0].data)->ownerRef = NULL;
        instance->vertexDescriptors[0].data = NULL;
    }

    if (instance->vertexDescriptors[0].data == NULL) {
        /* the buffer header, then each morphed stream 32-byte aligned */
        index = 0;
        sizes[index] = (instance->vertexDescriptors[index].stride * numVertices + 31) & ~31;
        size = sizes[index] + 6;
        index++;
        if (geometryFlags & rpGEOMETRYNORMALS) {
            if (targetFlags & rpGEOMETRYNORMALS) {
                sizes[index] = (instance->vertexDescriptors[index].stride * numVertices + 31) & ~31;
                size += sizes[index];
            }
            index++;
        }
        if (geometryFlags & rpGEOMETRYPRELIT) {
            if (targetFlags & rpGEOMETRYPRELIT) {
                sizes[index] = (instance->vertexDescriptors[index].stride * numVertices + 31) & ~31;
                size += sizes[index];
            }
            index++;
        }
        if ((geometryFlags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) &&
            (targetFlags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2))) {
            sizes[index] = (instance->vertexDescriptors[index].stride * numVertices + 31) & ~31;
            size += sizes[index];
        }

        buffer = fn_80480878(atomic, (RwResEntry **)&instance->vertexDescriptors[0].data, size,
                             fn_804449A4);
        if (geometry->numMorphTargets == 1) {
            ownerEntry = geometry->repEntry;
        } else {
            ownerEntry = atomic->repEntry;
        }
        if (buffer == NULL || ownerEntry == NULL) {
            if (buffer != NULL) {
                fn_8048071C(buffer);
            }
            return FALSE;
        }

        index = 0;
        instance->vertexDescriptors[index].data =
            (void *)(((RwUInt32)instance->vertexDescriptors[index].data + sizeof(RwResEntry) + 6 +
                      31) &
                     ~31);
        index++;
        if (geometryFlags & rpGEOMETRYNORMALS) {
            if (targetFlags & rpGEOMETRYNORMALS) {
                instance->vertexDescriptors[index].data =
                    (RwUInt8 *)instance->vertexDescriptors[index - 1].data + sizes[index - 1];
            }
            index++;
        }
        /* each stream follows the previous descriptor's; with normals that no target morphs, the
         * prelit colors are placed from the base normals and the unset sizes[1], as the original
         * does */
        if (geometryFlags & rpGEOMETRYPRELIT) {
            if (targetFlags & rpGEOMETRYPRELIT) {
                instance->vertexDescriptors[index].data =
                    (RwUInt8 *)instance->vertexDescriptors[index - 1].data + sizes[index - 1];
            }
            index++;
        }
        if ((geometryFlags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) &&
            (targetFlags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2))) {
            instance->vertexDescriptors[index].data =
                (RwUInt8 *)instance->vertexDescriptors[index - 1].data + sizes[index - 1];
        }
        DMORPHVERTEXBUFFERENTRY(instance->vertexDescriptors[0].data) = buffer;
    } else {
        RwResourcesUseResEntry(DMORPHVERTEXBUFFERENTRY(instance->vertexDescriptors[0].data));
    }

    DMORPHVERTEXBUFFER(instance->vertexDescriptors[0].data)->syncToken = lbl_805EDF40;
    return TRUE;
}

/* Reinstance callback: writes the atomic's morphed vertices into its vertex buffer. Each stream
 * that some target morphs starts from the geometry's base stream; the first target with a nonzero
 * weight writes base + weight * delta, the others add their weighted deltas in place, and with no
 * such target the base stream is copied. */
RpAtomic *fn_80444E7C(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh) {
    RwInt32 atomicOffset; /* the atomic plugin offset, read once on entry */
    RwUInt32 targetFlags;
    RpDMorphGeometry *dmorphGeometry;
    RpGameCubeInstanceData *instance;
    RwInt32 numVertices;
    RwReal weight;
    RwInt32 size;
    RwInt32 padding;
    RwInt32 index;
    RwUInt32 i;
    RpGameCubeVertexFormat *format;
    RwResEntry *buffer;
    RwUInt32 geometryFlags;
    RpGeometry *geometry;

    geometry = atomic->geometry;
    atomicOffset = lbl_805F1430;
    dmorphGeometry = DMORPHGEOMETRY(geometry);
    if (dmorphGeometry == NULL) {
        return atomic;
    }

    /* the streams some target morphs; every target has positions */
    targetFlags = rpGEOMETRYPOSITIONS;
    instance = (RpGameCubeInstanceData *)(renderMesh->repEntry + 1);
    geometryFlags = geometry->flags;
    for (i = 0; i < dmorphGeometry->numDMorphTargets; i++) {
        targetFlags |= dmorphGeometry->dmorphTargets[i].flags;
    }

    if (!fn_80444A2C(atomic, instance, renderMesh, dmorphGeometry->baseStreams, geometryFlags,
                     targetFlags)) {
        return NULL;
    }
    numVertices = geometry->numVertices;

    index = 0;
    if (targetFlags & rpGEOMETRYPOSITIONS) {
        register RwUInt32 gqr6 = 0; /* asm operands need registers */
        RwBool first;
        void *base;

        format = GAMECUBEVERTEXFORMAT(geometry);
        if (format != NULL) {
            /* GX component type to quantization type, and the component sizes */
            RwUInt8 quantizeType[5] = {4, 6, 5, 7, 0};
            RwUInt8 typeSize[5] = {1, 1, 2, 2, 4};

            gqr6 = quantizeType[format->posType] | (format->posFrac << 8);
            gqr6 |= gqr6 << 16;
            size = typeSize[format->posType];
        } else {
            size = 4;
        }
        asm { mtspr GQR6, gqr6 }

        first = TRUE;
        base = dmorphGeometry->baseStreams[index];
        for (i = 0; i < dmorphGeometry->numDMorphTargets; i++) {
            RpDMorphTarget *target = &dmorphGeometry->dmorphTargets[i];

            if (target->flags & rpGEOMETRYPOSITIONS) {
                RwReal value = RWPLUGINOFFSET(RpDMorphAtomic, atomic, atomicOffset)->values[i];

                if (value != 0.0f) {
                    rpDMorphRLE *rle;
                    RwV3d *deltas;
                    void *dst;

                    weight = value;
                    rle = &target->rle;
                    deltas = target->positions;
                    dst = instance->vertexDescriptors[index].data;
                    if (first) {
                        fn_804459C0(dst, base, deltas, &weight, rle, size, 0);
                        first = FALSE;
                    } else {
                        fn_80445AE8(dst, dst, deltas, &weight, rle, size, 0);
                    }
                }
            }
        }
        if (first) {
            fn_80445C00(instance->vertexDescriptors[index].data, base, size, 0, numVertices);
        }
    }

    index++;

    if (targetFlags & rpGEOMETRYNORMALS) {
        register RwUInt32 gqr6 = 0;
        RwBool first;
        void *base;

        format = GAMECUBEVERTEXFORMAT(geometry);
        if (format != NULL) {
            /* GX component type to quantization type, component size, and the scale of normals;
             * binormals and tangents follow each normal */
            RwUInt8 quantizeType[5] = {4, 6, 5, 7, 0};
            RwUInt8 typeSize[5] = {1, 1, 2, 2, 4};
            RwUInt8 normalScale[5] = {0, 6, 0, 14, 0};

            size = typeSize[format->normalType];
            padding = format->normalNbt ? size * 6 : 0;
            gqr6 = quantizeType[format->normalType] | (normalScale[format->normalType] << 8);
            gqr6 |= gqr6 << 16;
        } else {
            size = 4;
            padding = 0;
        }
        asm { mtspr GQR6, gqr6 }

        first = TRUE;
        base = dmorphGeometry->baseStreams[index];
        for (i = 0; i < dmorphGeometry->numDMorphTargets; i++) {
            RpDMorphTarget *target = &dmorphGeometry->dmorphTargets[i];

            if (target->flags & rpGEOMETRYNORMALS) {
                RwReal value = RWPLUGINOFFSET(RpDMorphAtomic, atomic, atomicOffset)->values[i];

                if (value != 0.0f) {
                    rpDMorphRLE *rle;
                    RwV3d *deltas;
                    void *dst;

                    weight = value;
                    rle = &target->rle;
                    deltas = target->normals;
                    dst = instance->vertexDescriptors[index].data;
                    if (first) {
                        fn_804459C0(dst, base, deltas, &weight, rle, size, padding);
                        first = FALSE;
                    } else {
                        fn_80445AE8(dst, dst, deltas, &weight, rle, size, padding);
                    }
                }
            }
        }
        if (first) {
            fn_80445C00(instance->vertexDescriptors[index].data, base, size, padding, numVertices);
        }
    }
    if (geometryFlags & rpGEOMETRYNORMALS) {
        index++;
    }

    if (targetFlags & rpGEOMETRYPRELIT) {
        RwBool first;
        RwRGBA *base = dmorphGeometry->baseStreams[index];

        first = TRUE;
        for (i = 0; i < dmorphGeometry->numDMorphTargets; i++) {
            RpDMorphTarget *target = &dmorphGeometry->dmorphTargets[i];

            if (target->flags & rpGEOMETRYPRELIT) {
                RwReal value = RWPLUGINOFFSET(RpDMorphAtomic, atomic, atomicOffset)->values[i];

                if (value != 0.0f) {
                    RwRGBA *delta = target->preLitLum;
                    RwRGBA *dst = instance->vertexDescriptors[index].data;
                    RwUInt32 run;
                    RwUInt32 j;

                    weight = value;
                    if (first) {
                        for (run = 0; run < target->rle.numCodeElements; run++) {
                            if (target->rle.code[run] & 0x80) {
                                for (j = 0; j < (target->rle.code[run] & 0x7F); j++) {
                                    dst->red = base->red + (RwInt32)(weight * delta->red);
                                    dst->green = base->green + (RwInt32)(weight * delta->green);
                                    dst->blue = base->blue + (RwInt32)(weight * delta->blue);
                                    dst->alpha = base->alpha + (RwInt32)(weight * delta->alpha);
                                    delta++;
                                    base++;
                                    dst++;
                                }
                            } else {
                                for (j = 0; j < (target->rle.code[run] & 0x7F); j++) {
                                    dst->red = base->red;
                                    dst->green = base->green;
                                    dst->blue = base->blue;
                                    dst->alpha = base->alpha;
                                    base++;
                                    dst++;
                                }
                            }
                        }
                        first = FALSE;
                    } else {
                        for (run = 0; run < target->rle.numCodeElements; run++) {
                            if (target->rle.code[run] & 0x80) {
                                for (j = 0; j < (target->rle.code[run] & 0x7F); j++) {
                                    dst->red += (RwInt32)(weight * delta->red);
                                    dst->green += (RwInt32)(weight * delta->green);
                                    dst->blue += (RwInt32)(weight * delta->blue);
                                    dst->alpha += (RwInt32)(weight * delta->alpha);
                                    delta++;
                                    dst++;
                                }
                            } else {
                                dst += target->rle.code[run] & 0x7F;
                            }
                        }
                    }
                }
            }
        }
        if (first) {
            RwRGBA *dst = instance->vertexDescriptors[index].data;

            for (i = 0; i < numVertices; i++) {
                *dst++ = *base++;
            }
        }
    }
    if (geometryFlags & rpGEOMETRYPRELIT) {
        index++;
    }

    if (targetFlags & rpGEOMETRYTEXTURED) {
        RwBool first;
        RwTexCoords *base = dmorphGeometry->baseStreams[index];

        first = TRUE;
        for (i = 0; i < dmorphGeometry->numDMorphTargets; i++) {
            RpDMorphTarget *target = &dmorphGeometry->dmorphTargets[i];

            if (target->flags & rpGEOMETRYTEXTURED) {
                RwReal value = RWPLUGINOFFSET(RpDMorphAtomic, atomic, atomicOffset)->values[i];

                if (value != 0.0f) {
                    RwTexCoords *delta = target->texCoords;
                    RwTexCoords *dst = instance->vertexDescriptors[index].data;
                    RwUInt32 run;
                    RwUInt32 j;

                    weight = value;
                    if (first) {
                        for (run = 0; run < target->rle.numCodeElements; run++) {
                            if (target->rle.code[run] & 0x80) {
                                for (j = 0; j < (target->rle.code[run] & 0x7F); j++) {
                                    dst->u = base->u + weight * delta->u;
                                    dst->v = base->v + weight * delta->v;
                                    delta++;
                                    base++;
                                    dst++;
                                }
                            } else {
                                for (j = 0; j < (target->rle.code[run] & 0x7F); j++) {
                                    dst->u = base->u;
                                    dst->v = base->v;
                                    base++;
                                    dst++;
                                }
                            }
                        }
                        first = FALSE;
                    } else {
                        for (run = 0; run < target->rle.numCodeElements; run++) {
                            if (target->rle.code[run] & 0x80) {
                                for (j = 0; j < (target->rle.code[run] & 0x7F); j++) {
                                    dst->u += weight * delta->u;
                                    dst->v += weight * delta->v;
                                    delta++;
                                    dst++;
                                }
                            } else {
                                dst += target->rle.code[run] & 0x7F;
                            }
                        }
                    }
                }
            }
        }
        if (first) {
            RwTexCoords *dst = instance->vertexDescriptors[index].data;

            for (i = 0; i < numVertices; i++) {
                *dst++ = *base++;
            }
        }
    }

    buffer = DMORPHVERTEXBUFFERENTRY(instance->vertexDescriptors[0].data);
    DCStoreRange(buffer + 1, buffer->size - 6);
    GXInvalidateVtxCache();
    return atomic;
}

/* Instance callback: instances the atomic with the optimised instancing mode turned off for
 * geometries instanced natively (inferred) */
RpAtomic *fn_80445934(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh) {
    if (atomic->geometry->flags & rpGEOMETRYNATIVEINSTANCE) {
        RwInt32 mode = fn_80462C28();

        if (mode == 1) {
            fn_80462C20(0);
        }
        fn_804639AC(atomic, renderMesh);
        if (mode == 1) {
            fn_80462C20(1);
        }
    } else {
        fn_804639AC(atomic, renderMesh);
    }
    return atomic;
}

/* Applies the first target with a nonzero weight to a stream: for each run of the target's run-
 * length code, the vertices with deltas get base + weight * delta and the others a copy of base. A
 * vertex is three components of size bytes in the GQR6 quantization followed by padding bytes; the
 * deltas are an RwV3d for each vertex that has one. */
asm void fn_804459C0(register void *dst, register const void *base, register const RwV3d *deltas,
                     register const RwReal *weight, register const rpDMorphRLE *rle,
                     register RwInt32 size, register RwInt32 padding) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* nothing to do without runs */
    lwz      r10, rpDMorphRLE.numCodeElements(rle)
    cmpwi    r10, 0
    blelr+
    /* r11 walks the code, f0 = the weight and r12 = twice the component size; the vertex pointers
     * start one component back for the update-form loads and stores */
    lwz      r11, rpDMorphRLE.code(rle)
    subi     r11, r11, 0x1
    lfs      f0, 0(weight)
    rotlwi   r12, size, 1
    subf     dst, size, dst
    subf     base, size, base
    /* the next run: below 0x80 it has no deltas, and its low seven bits count its vertices; an
     * empty run with deltas is passed over */
run:
    lbzu     r0, 0x1(r11)
    cmpwi    cr2, r0, 0x80
    andi.    r0, r0, 0x7f
    blt      cr2, copy_run
    bgt+     delta_run
    subi     r10, r10, 0x1
    cmpwi    r10, 0
    bgt-     run
    blr
    /* an odd count does one vertex first, then the vertices go in pairs; r6 is free for the odd
     * bit, as the weight is in f0 */
delta_run:
    andi.    r6, r0, 0x1
    srawi    r0, r0, 1
    beq      delta_pairs
    psq_l    f1, 0(deltas), 0, 0
    psq_l    f2, 0x8(deltas), 1, 0
    addi     deltas, deltas, 0xc
    psq_lux  f3, base, size, 0, 6
    psq_lux  f4, base, r12, 1, 6
    add      base, base, padding
    ps_madd  f5, f1, f0, f3
    ps_madd  f6, f2, f0, f4
    psq_stux f5, dst, size, 0, 6
    psq_stux f6, dst, r12, 1, 6
    add      dst, dst, padding
    cmpwi    r0, 0
    ble      next_run
delta_pairs:
    mtctr    r0
delta_pair:
    psq_l    f1, 0(deltas), 0, 0
    psq_l    f2, 0x8(deltas), 1, 0
    psq_lux  f3, base, size, 0, 6
    psq_lux  f4, base, r12, 1, 6
    add      base, base, padding
    psq_l    f5, 0xc(deltas), 0, 0
    psq_l    f6, 0x14(deltas), 1, 0
    psq_lux  f7, base, size, 0, 6
    psq_lux  f8, base, r12, 1, 6
    add      base, base, padding
    addi     deltas, deltas, 0x18
    ps_madd  f9, f1, f0, f3
    ps_madd  f10, f2, f0, f4
    ps_madd  f11, f5, f0, f7
    ps_madd  f12, f6, f0, f8
    psq_stux f9, dst, size, 0, 6
    psq_stux f10, dst, r12, 1, 6
    add      dst, dst, padding
    psq_stux f11, dst, size, 0, 6
    psq_stux f12, dst, r12, 1, 6
    add      dst, dst, padding
    /* count the pairs down (the condition bit is not tested) */
    bc       17, gt, delta_pair
    subi     r10, r10, 0x1
    cmpwi    r10, 0
    bgt-     run
    blr
    /* copy the base vertices of a run without deltas; an empty one goes back to run without being
     * counted */
copy_run:
    ble-     run
    mtctr    r0
copy_vertex:
    psq_lux  f3, base, size, 0, 6
    psq_lux  f4, base, r12, 1, 6
    add      base, base, padding
    psq_stux f3, dst, size, 0, 6
    psq_stux f4, dst, r12, 1, 6
    add      dst, dst, padding
    bc       17, gt, copy_vertex
next_run:
    subi     r10, r10, 0x1
    cmpwi    r10, 0
    bgt-     run
    blr
#endif // clang-format on
}

/* Applies a further target to a stream in place (dst and base are the same buffer): the vertices
 * with deltas get weight * delta added, the runs without deltas are skipped. */
asm void fn_80445AE8(register void *dst, register const void *base, register const RwV3d *deltas,
                     register const RwReal *weight, register const rpDMorphRLE *rle,
                     register RwInt32 size, register RwInt32 padding) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* nothing to do without runs */
    lwz      r10, rpDMorphRLE.numCodeElements(rle)
    cmpwi    r10, 0
    blelr
    /* r11 walks the code, f0 = the weight and r12 = twice the component size; the vertex pointers
     * start one component back for the update-form loads and stores */
    lwz      r11, rpDMorphRLE.code(rle)
    subi     r11, r11, 0x1
    lfs      f0, 0(weight)
    rotlwi   r12, size, 1
    subf     dst, size, dst
    subf     base, size, base
    /* the next run: below 0x80 it has no deltas, and its low seven bits count its vertices; an
     * empty run with deltas is passed over */
run:
    lbzu     r0, 0x1(r11)
    cmpwi    cr2, r0, 0x80
    andi.    r0, r0, 0x7f
    blt      cr2, skip_run
    bgt+     delta_run
    subi     r10, r10, 0x1
    cmpwi    r10, 0
    bgt      run
    blr
    /* an odd count does one vertex first, then the vertices go in pairs; r6 is free for the odd
     * bit, as the weight is in f0 */
delta_run:
    andi.    r6, r0, 0x1
    srawi    r0, r0, 1
    beq      delta_pairs
    psq_l    f1, 0(deltas), 0, 0
    psq_l    f2, 0x8(deltas), 1, 0
    addi     deltas, deltas, 0xc
    psq_lux  f3, base, size, 0, 6
    psq_lux  f4, base, r12, 1, 6
    add      base, base, padding
    ps_madd  f5, f1, f0, f3
    ps_madd  f6, f2, f0, f4
    psq_stux f5, dst, size, 0, 6
    psq_stux f6, dst, r12, 1, 6
    add      dst, dst, padding
    cmpwi    r0, 0
    ble      next_run
delta_pairs:
    mtctr    r0
delta_pair:
    psq_l    f1, 0(deltas), 0, 0
    psq_l    f2, 0x8(deltas), 1, 0
    psq_lux  f3, base, size, 0, 6
    psq_lux  f4, base, r12, 1, 6
    add      base, base, padding
    psq_l    f5, 0xc(deltas), 0, 0
    psq_l    f6, 0x14(deltas), 1, 0
    psq_lux  f7, base, size, 0, 6
    psq_lux  f8, base, r12, 1, 6
    add      base, base, padding
    ps_madd  f9, f1, f0, f3
    ps_madd  f10, f2, f0, f4
    ps_madd  f11, f5, f0, f7
    ps_madd  f12, f6, f0, f8
    psq_stux f9, dst, size, 0, 6
    psq_stux f10, dst, r12, 1, 6
    add      dst, dst, padding
    psq_stux f11, dst, size, 0, 6
    psq_stux f12, dst, r12, 1, 6
    add      dst, dst, padding
    addi     deltas, deltas, 0x18
    /* count the pairs down (the condition bit is not tested) */
    bc       16, gt, delta_pair
    subi     r10, r10, 0x1
    cmpwi    r10, 0
    bgt      run
    blr
    /* skip the vertices of a run without deltas */
skip_run:
    add      r6, size, r12
    add      r6, r6, padding
    mullw    r6, r0, r6
    add      base, base, r6
    add      dst, dst, r6
next_run:
    subi     r10, r10, 0x1
    cmpwi    r10, 0
    bgt      run
    blr
#endif // clang-format on
}

/* Copies numVertices base vertices to dst, for a stream that no target changes. */
asm void fn_80445C00(register void *dst, register const void *base, register RwInt32 size,
                     register RwInt32 padding, register RwInt32 numVertices){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    cmpwi    numVertices, 0
    blelr+
    /* r8 = twice the component size; the pointers start one component back for the update-form
     * loads and stores */
    mtctr    numVertices
    rotlwi   r8, size, 1
    subf     dst, size, dst
    subf     base, size, base
vertex:
    psq_lux  f0, base, size, 0, 6
    psq_lux  f1, base, r8, 1, 6
    add      base, base, padding
    psq_stux f0, dst, size, 0, 6
    psq_stux f1, dst, r8, 1, 6
    add      dst, dst, padding
    bc       16, gt, vertex
    blr
#endif // clang-format on
}

/* Creates the delta morph pipeline */
RwBool fn_80445C38(RwInt32 pluginData) {
    RxPipeline *pipeline = fn_8046335C(rpID_DMORPHPLUGIN, pluginData, fn_80445934, fn_80444E7C,
                                       fn_80463428, (RpGameCubeRenderCallBack)fn_80462914);

    if (pipeline == NULL) {
        return FALSE;
    }
    lbl_805F1438[1] = pipeline;
    return TRUE;
}

/* Destroys it */
RwBool fn_80445C9C(void) {
    if (lbl_805F1438[1] != NULL) {
        fn_8049B800(lbl_805F1438[1]);
        lbl_805F1438[1] = NULL;
    }
    return TRUE;
}

/* Sets an atomic's pipeline to one of the plugin's */
RpAtomic *fn_80445CE0(RpAtomic *atomic, RwInt32 pipelineIndex) {
    atomic->pipeline = lbl_805F1438[pipelineIndex];
    return atomic;
}
