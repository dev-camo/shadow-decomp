/* RenderWare delta morph plugin (plugin ID 0x122), atomic part: the delta morph weights of an atomic,
 * the render callback that applies them to the geometry, and the delta morph animation player. The
 * original file name is unknown. */
#include "rw/rpdmorph.h"
#include <string.h>

RwInt32 lbl_805F1430;

/* The render callback of a delta morphed atomic: applies the change of each target's weight since
 * the last render to the geometry's vertex streams, then calls the atomic's own render callback.
 * A target's runs list its vertices: a run with the top bit set covers that many (low 7 bits)
 * morphed vertices, which have target data, otherwise it skips that many. */
RpAtomic *fn_80443A74(RpAtomic *atomic) {
    RpDMorphAtomic *dmorphAtomic;
    RpDMorphGeometry *dmorphGeometry;
    RpGeometry *geometry;
    RpDMorphTarget *target;
    RwV3d *verts;
    RwV3d *normals;
    RwRGBA *preLitLum;
    RwTexCoords *texCoords;
    RwV3d *targetNormals;
    RwRGBA *targetPreLitLum;
    RwTexCoords *targetTexCoords;
    RwUInt32 i;
    RwV3d *targetPositions;

    dmorphAtomic = DMORPHATOMIC(atomic);
    dmorphGeometry = DMORPHGEOMETRY(atomic->geometry);
    if (dmorphGeometry != NULL && dmorphAtomic->values != NULL) {
        geometry = atomic->geometry;
        verts = geometry->morphTarget->verts;
        normals = geometry->morphTarget->normals;
        preLitLum = geometry->preLitLum;
        texCoords = geometry->texCoords[0];

        for (i = 0; i < dmorphGeometry->numDMorphTargets; i++) {
            if (dmorphAtomic->values[i] != dmorphGeometry->values[i]) {
                RwReal delta = dmorphAtomic->values[i] - dmorphGeometry->values[i];
                RwUInt32 run;
                RwUInt32 vertex;

                target = &dmorphGeometry->dmorphTargets[i];
                targetPositions = target->positions;
                targetNormals = target->normals;
                targetPreLitLum = target->preLitLum;
                targetTexCoords = target->texCoords;

                fn_8045916C(geometry, target->field_0xC);

                if (target->flags & rpDMORPHTARGETPOSITIONS) {
                    RwV3d *next = targetPositions;

                    for (run = 0, vertex = 0; run < target->rle.numCodeElements; run++) {
                        if (target->rle.code[run] & 0x80) {
                            RwV3d *dst = &verts[vertex];
                            RwV3d *src = next;
                            RwUInt8 j;

                            for (j = 0; j < (target->rle.code[run] & 0x7F); j++) {
                                vertex++;
                                next++;
                                dst->x += delta * src->x;
                                dst->y += delta * src->y;
                                dst->z += delta * src->z;
                                src++;
                                dst++;
                            }
                        } else {
                            vertex += target->rle.code[run] & 0x7F;
                        }
                    }
                }
                if (target->flags & rpDMORPHTARGETNORMALS) {
                    RwV3d *next = targetNormals;

                    for (run = 0, vertex = 0; run < target->rle.numCodeElements; run++) {
                        if (target->rle.code[run] & 0x80) {
                            RwV3d *dst = &normals[vertex];
                            RwV3d *src = next;
                            RwUInt8 j;

                            for (j = 0; j < (target->rle.code[run] & 0x7F); j++) {
                                vertex++;
                                next++;
                                dst->x += delta * src->x;
                                dst->y += delta * src->y;
                                dst->z += delta * src->z;
                                src++;
                                dst++;
                            }
                        } else {
                            vertex += target->rle.code[run] & 0x7F;
                        }
                    }
                }
                if (target->flags & rpDMORPHTARGETPRELIT) {
                    /* colours take out the old weight's share and put in the new one */
                    RwRGBA *next = targetPreLitLum;

                    for (run = 0, vertex = 0; run < target->rle.numCodeElements; run++) {
                        if (target->rle.code[run] & 0x80) {
                            RwRGBA *dst = &preLitLum[vertex];
                            RwRGBA *src = next;
                            RwUInt8 j;

                            for (j = 0; j < (target->rle.code[run] & 0x7F); j++) {
                                RwReal oldValue = dmorphGeometry->values[i];
                                RwReal newValue = dmorphAtomic->values[i];

                                dst->alpha -= (RwUInt8)(RwInt32)(oldValue * src->alpha);
                                dst->blue -= (RwUInt8)(RwInt32)(oldValue * src->blue);
                                dst->green -= (RwUInt8)(RwInt32)(oldValue * src->green);
                                dst->red -= (RwUInt8)(RwInt32)(oldValue * src->red);
                                dst->alpha += (RwUInt8)(RwInt32)(newValue * src->alpha);
                                dst->blue += (RwUInt8)(RwInt32)(newValue * src->blue);
                                dst->green += (RwUInt8)(RwInt32)(newValue * src->green);
                                dst->red += (RwUInt8)(RwInt32)(newValue * src->red);
                                vertex++;
                                next++;
                                src++;
                                dst++;
                            }
                        } else {
                            vertex += target->rle.code[run] & 0x7F;
                        }
                    }
                }
                if (target->flags & rpDMORPHTARGETTEXCOORDS) {
                    RwTexCoords *next = targetTexCoords;

                    for (run = 0, vertex = 0; run < target->rle.numCodeElements; run++) {
                        if (target->rle.code[run] & 0x80) {
                            RwTexCoords *dst = &texCoords[vertex];
                            RwTexCoords *src = next;
                            RwUInt8 j;

                            for (j = 0; j < (target->rle.code[run] & 0x7F); j++) {
                                vertex++;
                                next++;
                                dst->u += delta * src->u;
                                dst->v += delta * src->v;
                                src++;
                                dst++;
                            }
                        } else {
                            vertex += target->rle.code[run] & 0x7F;
                        }
                    }
                }

                fn_804591C4(geometry);
            }
            dmorphGeometry->values[i] = dmorphAtomic->values[i];
        }
    }
    dmorphAtomic->renderCallBack(atomic);
    return atomic;
}

/* atomic plugin constructor */
void *fn_80443FE0(void *object, RwInt32 offset, RwInt32 size) {
    RpDMorphAtomic *dmorphAtomic = DMORPHATOMIC(object);

    dmorphAtomic->values = NULL;
    dmorphAtomic->interpolator = NULL;
    dmorphAtomic->renderCallBack = NULL;
    return object;
}

/* atomic plugin destructor */
void *fn_80443FFC(void *object, RwInt32 offset, RwInt32 size) {
    RpDMorphAtomic *dmorphAtomic = DMORPHATOMIC(object);

    if (dmorphAtomic->values != NULL) {
        RwFree(dmorphAtomic->values);
        dmorphAtomic->values = NULL;
    }
    if (dmorphAtomic->interpolator != NULL) {
        RwFree(dmorphAtomic->interpolator);
        dmorphAtomic->interpolator = NULL;
    }
    dmorphAtomic->renderCallBack = NULL;
    return object;
}

/* Allocates the zeroed weights of an atomic, one per target of its geometry, in its plugin data */
static inline RpAtomic *DMorphAtomicCreateValues(RpAtomic *atomic) {
    RpDMorphAtomic *dmorphAtomic = DMORPHATOMIC(atomic);
    RwInt32 size = fn_80443A54(atomic->geometry) * sizeof(RwReal);

    dmorphAtomic->values = RwMalloc(size, rwMEMHINT_DMORPH);
    if (dmorphAtomic->values == NULL) {
        RWERRORPLUGIN(rpID_DMORPHPLUGIN, (E_RW_NOMEM, size));
        return NULL;
    }
    memset(dmorphAtomic->values, 0, size);
    return atomic;
}

/* atomic plugin copy: copies the weights, but not the animation */
void *fn_80444088(void *dstObject, const void *srcObject, RwInt32 offset, RwInt32 size) {
    RpAtomic *dst = dstObject;
    const RpAtomic *src = srcObject;
    RwUInt32 numDMorphTargets;

    if (src->geometry != NULL) {
        numDMorphTargets = fn_80443A54(src->geometry);
        if (numDMorphTargets != 0) {
            RpDMorphAtomic *srcData = DMORPHATOMIC(src);
            RpDMorphAtomic *dstData = DMORPHATOMIC(dst);

            if (DMorphAtomicCreateValues(dst) != NULL) {
                if (srcData->values != NULL) {
                    memcpy(dstData->values, srcData->values, numDMorphTargets * sizeof(RwReal));
                } else {
                    memset(dstData->values, 0, numDMorphTargets * sizeof(RwReal));
                }
            }
            dstData->interpolator = NULL;
            dstData->renderCallBack = srcData->renderCallBack;
        }
    }
    return dstObject;
}

/* atomic stream callback run on every load: installs the render callback of delta morphed atomics */
RwBool fn_804441A0(void *object, RwInt32 offset, RwInt32 size) {
    RpAtomic *atomic = object;

    if (fn_80443A54(atomic->geometry) != 0) {
        fn_80444284(atomic);
    }
    return TRUE;
}

/* atomic stream rights callback */
RwBool fn_804441E4(void *object, RwInt32 offset, RwInt32 size, RwUInt32 extraData) {
    fn_804442A4(object, 1);
    return TRUE;
}

/* Attaches the plugin to atomics */
RwBool fn_8044420C(void) {
    lbl_805F1430 = fn_80458B6C(sizeof(RpDMorphAtomic), rpID_DMORPHPLUGIN, fn_80443FE0, fn_80443FFC,
                               fn_80444088);
    if (lbl_805F1430 < 0) {
        return FALSE;
    }
    fn_80458C28(rpID_DMORPHPLUGIN, fn_804441A0);
    fn_80458C5C(rpID_DMORPHPLUGIN, fn_804441E4);
    return TRUE;
}

/* Puts the delta morph render callback in front of the atomic's own */
RpAtomic *fn_80444284(RpAtomic *atomic) {
    DMORPHATOMIC(atomic)->renderCallBack = atomic->renderCallBack;
    atomic->renderCallBack = fn_80443A74;
    return atomic;
}

/* Restores the atomic's own render callback and sets its pipeline */
RpAtomic *fn_804442A4(RpAtomic *atomic, RwInt32 pipelineIndex) {
    RpDMorphAtomic *dmorphAtomic = DMORPHATOMIC(atomic);

    if (dmorphAtomic->renderCallBack != NULL) {
        atomic->renderCallBack = dmorphAtomic->renderCallBack;
        dmorphAtomic->renderCallBack = NULL;
    }
    return fn_80445CE0(atomic, pipelineIndex);
}

/* The weights of an atomic, allocated (zeroed) on first use; NULL if that fails */
RwReal *fn_804442E4(RpAtomic *atomic) {
    RpDMorphAtomic *dmorphAtomic = DMORPHATOMIC(atomic);

    if (dmorphAtomic->values == NULL) {
        RwInt32 size = fn_80443A54(atomic->geometry) * sizeof(RwReal);

        dmorphAtomic->values = RwMalloc(size, rwMEMHINT_DMORPH);
        if (dmorphAtomic->values == NULL) {
            RWERRORPLUGIN(rpID_DMORPHPLUGIN, (E_RW_NOMEM, size));
        } else {
            memset(dmorphAtomic->values, 0, size);
        }
    }
    return dmorphAtomic->values;
}

/* Allocates the animation state of an atomic */
static inline RpAtomic *DMorphAtomicCreateInterpolator(RpAtomic *atomic) {
    RwUInt32 numDMorphTargets = fn_80443A54(atomic->geometry);
    RwInt32 size = numDMorphTargets * (sizeof(RwReal) + sizeof(RpDMorphFrame *)) +
                   sizeof(RpDMorphInterpolator);
    RpDMorphInterpolator *interpolator = RwMalloc(size, rwMEMHINT_DMORPH);

    if (interpolator == NULL) {
        RWERRORPLUGIN(rpID_DMORPHPLUGIN, (E_RW_NOMEM, size));
        return NULL;
    }
    memset(interpolator, 0, size);
    interpolator->frameTimes = (RwReal *)(interpolator + 1);
    interpolator->frames = (RpDMorphFrame **)(interpolator->frameTimes + numDMorphTargets);
    DMORPHATOMIC(atomic)->interpolator = interpolator;
    return atomic;
}

/* Sets the delta morph animation of an atomic and starts it at each target's first frame; NULL
 * stops it */
RpAtomic *fn_804443A0(RpAtomic *atomic, RpDMorphAnimation *animation) {
    RpDMorphAtomic *dmorphAtomic = DMORPHATOMIC(atomic);

    if (animation == NULL) {
        RwUInt32 i;

        if (dmorphAtomic->interpolator != NULL) {
            for (i = 0; i < fn_80443A54(atomic->geometry); i++) {
                dmorphAtomic->interpolator->frames[i] = NULL;
            }
            dmorphAtomic->interpolator->animation = NULL;
        }
    } else {
        RwUInt32 i;
        RpDMorphInterpolator *interpolator;
        RwReal *values;

        if (dmorphAtomic->interpolator == NULL) {
            if (DMorphAtomicCreateInterpolator(atomic) == NULL) {
                return NULL;
            }
        }
        if (dmorphAtomic->values == NULL) {
            if (DMorphAtomicCreateValues(atomic) == NULL) {
                return NULL;
            }
        }

        values = dmorphAtomic->values;
        interpolator = dmorphAtomic->interpolator;
        interpolator->animation = animation;
        interpolator->time = 0.0f;
        for (i = 0; i < fn_80443A54(atomic->geometry); i++) {
            if (i < animation->numDMorphTargets && animation->frameLists[i].frames != NULL) {
                interpolator->frames[i] = animation->frameLists[i].frames;
                interpolator->frameTimes[i] = 0.0f;
                values[i] = animation->frameLists[i].frames->startValue;
            } else {
                interpolator->frames[i] = NULL;
                interpolator->frameTimes[i] = 0.0f;
                values[i] = 0.0f;
            }
        }
    }
    return atomic;
}

/* Advances an atomic's delta morph animation by delta: moves each target along its frames and sets
 * its weight; calls the animation callback when a target's frames loop back */
RpAtomic *fn_80444630(RpAtomic *atomic, RwReal delta) {
    RpDMorphAtomic *dmorphAtomic = DMORPHATOMIC(atomic);
    RpDMorphInterpolator *interpolator = dmorphAtomic->interpolator;
    RpDMorphFrame **frames;
    RwReal *frameTimes;
    RwBool looped = FALSE;
    RwUInt32 i;

    interpolator->time += delta;
    frames = interpolator->frames;
    frameTimes = interpolator->frameTimes;
    for (i = 0; i < fn_80443A54(atomic->geometry); frames++, frameTimes++, i++) {
        if (*frames != NULL) {
            *frameTimes += delta;
            while (*frameTimes > (*frames)->duration) {
                RpDMorphFrame *frame;
                RpDMorphFrame *next;

                *frameTimes -= (*frames)->duration;
                frame = *frames;
                if (frame->nextFrame == rpDMORPHNOFRAME) {
                    *frames = NULL;
                    break;
                }
                next = &interpolator->animation->frameLists[i].frames[frame->nextFrame];
                looped |= next < frame;
                *frames = next;
            }
            if (*frames != NULL) {
                RpDMorphFrame *frame = *frames;

                dmorphAtomic->values[i] =
                    *frameTimes * frame->recipDuration * (frame->endValue - frame->startValue) +
                    frame->startValue;
            }
        }
    }
    if (looped && interpolator->callBack != NULL) {
        interpolator->callBack(atomic, interpolator->callBackData);
    }
    return atomic;
}
