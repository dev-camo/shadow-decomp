/* RenderWare clumps and atomics: the atomic rights stream extension, the atomic's bounding spheres,
 * the clump module's free lists, the clump plugin data of cameras and lights, creating,
 * destroying and cloning atomics and clumps, and reading them from a stream. The original file
 * name is unknown. */
#include "rw/rpworld.h"
#include <string.h>

#define rwMEMHINTDUR_EVENT 0x30000
#define rwMEMHINTDUR_GLOBAL 0x40000

/* chunk IDs */
#define rwID_CAMERA 0x5
#define rwID_FRAMELIST 0xE
#define rwID_GEOMETRY 0xF
#define rwID_CLUMP 0x10
#define rwID_LIGHT 0x12
#define rwID_ATOMIC 0x14
#define rwID_GEOMETRYLIST 0x1A

/* errors */
#define E_RW_READ 0x8000001A
#define E_RW_ATOMICNOFRAME 0x5 /* cloning an atomic without a frame (inferred) */

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

#define rwSTREAMOLDESTVERSION 0x35000

/* The stream header of a clump (0xC bytes) */
typedef struct ClumpChunk {
    /* 0x0 */ RwInt32 numAtomics;
    /* 0x4 */ RwInt32 numLights;
    /* 0x8 */ RwInt32 numCameras;
} ClumpChunk;

/* The stream header of an atomic (0x10 bytes) */
typedef struct AtomicChunk {
    /* 0x0 */ RwInt32 frameIndex;
    /* 0x4 */ RwInt32 geomIndex;
    /* 0x8 */ RwInt32 flags;
    /* 0xC */ RwInt32 unused;
} AtomicChunk;

/* The clump module's globals in the engine (inferred) */
typedef struct ClumpGlobals {
    /* 0x0 */ RwFreeList *atomicFreeList;
    /* 0x4 */ RwFreeList *clumpFreeList;
} ClumpGlobals;

/* The clump module (inferred) */
typedef struct ClumpModule {
    /* 0x0 */ RwInt32 globalsOffset; /* engine plugin offset of the globals */
    /* 0x4 */ RwInt32 numInstances;  /* the open engine instances */
} ClumpModule;

#define CLUMPGLOBALS() ((ClumpGlobals *)((RwUInt8 *)lbl_805F15B4 + lbl_805F14C8.globalsOffset))
#define CLUMPGLOBAL(var) (CLUMPGLOBALS()->var)

#define PIPELINEGLOBAL(var) (((RpPipelineGlobals *)((RwUInt8 *)lbl_805F15B4 + lbl_805F1690))->var)

RwPluginRegistry lbl_8056D7E0 = {sizeof(RpAtomic), sizeof(RpAtomic), 0, 0, NULL, NULL};
RwPluginRegistry lbl_8056D7F8 = {sizeof(RpClump), sizeof(RpClump), 0, 0, NULL, NULL};

RwFreeList lbl_805E1580; /* the atomic free list */
RwFreeList lbl_805E15A4; /* the clump free list */

RwInt32 lbl_805EDDA0 = 128; /* atomic free list entries per block */
RwInt32 lbl_805EDDA4 = 1;   /* atomic free list blocks to preallocate */
RwInt32 lbl_805EDDA8 = 128; /* clump free list entries per block */
RwInt32 lbl_805EDDAC = 1;   /* clump free list blocks to preallocate */

/* declared in reverse address order (the compiler places them in .sbss last first) */
ClumpModule lbl_805F14C8;
RwUInt32 lbl_805F14C4; /* atomic pipeline plugin, from the last atomic stream's rights */
RwUInt32 lbl_805F14C0; /* atomic pipeline plugin data */
RwInt32 lbl_805F14BC;  /* light plugin offset of the clump data */
RwInt32 lbl_805F14B8;  /* camera plugin offset of the clump data */

#define CLUMPLIGHTEXT(light) ((RpClumpObjectExtension *)((RwUInt8 *)(light) + lbl_805F14BC))
#define CLUMPCAMERAEXT(camera) ((RpClumpObjectExtension *)((RwUInt8 *)(camera) + lbl_805F14B8))

/* the light or camera of a clump list link */
#define CLUMPLIGHTFROMLINK(link)                                                                   \
    ((RpLight *)((RwUInt8 *)(link) - offsetof(RpClumpObjectExtension, inClumpLink) - lbl_805F14BC))
#define CLUMPCAMERAFROMLINK(link)                                                                  \
    ((RwCamera *)((RwUInt8 *)(link) - offsetof(RpClumpObjectExtension, inClumpLink) - lbl_805F14B8))

/* the atomic of a clump list link */
#define CLUMPATOMICFROMLINK(link)                                                                  \
    ((RpAtomic *)((RwUInt8 *)(link) - offsetof(RpAtomic, inClumpLink)))

typedef RpAtomic *(*RpAtomicCallBack)(RpAtomic *atomic, void *data);
typedef RpLight *(*RpLightCallBack)(RpLight *light, void *data);
typedef RwCamera *(*RwCameraCallBack)(RwCamera *camera, void *data);

RpClump *fn_80456230(RpClump *clump, void *data);
RpClump *fn_80457630(RpClump *clump, RpLight *light);
RpClump *fn_80457664(RpClump *clump, RwCamera *camera);
void fn_80456560(RpAtomic *atomic);

/* free list callbacks that destroy the clumps and atomics still in use when the module closes */
void fn_80455D0C(void *clump, void *data) {
    fn_80457468(clump);
}

void fn_80455D2C(void *atomic, void *data) {
    fn_80456FD8(atomic);
}

/* atomic rights stream read */
RwStream *fn_80455D4C(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    if (fn_8047D868(stream, (RwInt32 *)&lbl_805F14C4, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (length == 8 && fn_8047D868(stream, (RwInt32 *)&lbl_805F14C0, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    return stream;
}

/* atomic rights stream write */
RwStream *fn_80455DC8(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {
    const RpAtomic *atomic = object;

    if (fn_8047D458(stream, (RwInt32 *)&atomic->pipeline->pluginId, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (fn_8047D458(stream, (RwInt32 *)&atomic->pipeline->pluginData, sizeof(RwInt32)) != NULL) {
        return stream;
    }
    return NULL;
}

/* atomic rights stream size: only for a pipeline that a plugin owns */
RwInt32 fn_80455E44(const void *object, RwInt32 offset, RwInt32 size) {
    const RpAtomic *atomic = object;

    if (atomic->pipeline != NULL && atomic->pipeline->pluginId != 0) {
        return 8;
    }
    return 0;
}

/* the atomic's frame sync: brings its bounding sphere up to date and marks its world bounding
 * sphere out of date */
void *fn_80455E6C(void *object) {
    RpAtomic *atomic = object;

    if (atomic->interpolator.flags & rpINTERPOLATORDIRTYSPHERE) {
        fn_80456560(atomic);
    }
    atomic->object.privateFlags |= rpATOMICSPHEREDIRTY;
    return atomic;
}

/* Renders an atomic with its pipeline, or the default atomic pipeline */
RpAtomic *fn_80455EB4(RpAtomic *atomic) {
    RxPipeline *pipeline = atomic->pipeline;

    if (pipeline == NULL) {
        pipeline = PIPELINEGLOBAL(defaultAtomicPipeline);
    }
    if (fn_8049B674(pipeline, atomic, TRUE) != NULL) {
        return atomic;
    }
    return NULL;
}

/* Destroys the geometries of a geometry list and frees it */
static inline void GeometryListDeinitialize(RpGeometryList *geomList) {
    RwInt32 i;

    for (i = 0; i < geomList->numGeoms; i++) {
        fn_8045985C(geomList->geometries[i]);
    }
    if (geomList->geometries != NULL) {
        RwFree(geomList->geometries);
        geomList->geometries = NULL;
    }
}

/* Reads the geometries of a clump stream */
RpGeometryList *fn_80455F18(RwStream *stream, RpGeometryList *geomList) {
    RwInt32 numGeoms;
    RwUInt32 length;
    RwUInt32 version;
    RpGeometry *geometry;
    RwInt32 i;

    if (!fn_8047CF50(stream, rwID_STRUCT, &length, &version)) {
        return NULL;
    }
    if (version >= rwSTREAMOLDESTVERSION && version <= RW_STREAM_VERSION) {
        if (fn_80480D4C(stream, &numGeoms, sizeof(RwInt32)) != sizeof(RwInt32)) {
            return NULL;
        }
        fn_8047D254(&numGeoms, sizeof(RwInt32));

        geomList->numGeoms = 0;
        if (numGeoms > 0) {
            geomList->geometries =
                RwMalloc(numGeoms * sizeof(RpGeometry *), rwMEMHINTDUR_EVENT | rwID_GEOMETRY);
            if (geomList->geometries == NULL) {
                RWERROR((E_RW_NOMEM, numGeoms * sizeof(RpGeometry *)));
                return NULL;
            }
        } else {
            geomList->geometries = NULL;
        }

        for (i = 0; i < numGeoms; i++) {
            if (!fn_8047CF50(stream, rwID_GEOMETRY, NULL, &version)) {
                GeometryListDeinitialize(geomList);
                return NULL;
            }
            if (version >= rwSTREAMOLDESTVERSION && version <= RW_STREAM_VERSION) {
                geometry = fn_804599C8(stream);
                geomList->geometries[i] = geometry;
                if (geometry == NULL) {
                    GeometryListDeinitialize(geomList);
                    return NULL;
                }
                geomList->numGeoms++;
            } else {
                GeometryListDeinitialize(geomList);
                return NULL;
            }
        }
    } else {
        RWERROR((E_RW_BADVERSION));
        return NULL;
    }
    return geomList;
}

/* camera plugin constructor: not in a clump */
void *fn_804561F0(void *object, RwInt32 offset, RwInt32 size) {
    RpClumpObjectExtension *extension = CLUMPCAMERAEXT(object);

    extension->inClumpLink.prev = NULL;
    extension->inClumpLink.next = NULL;
    extension->clump = NULL;
    return object;
}

/* camera plugin destructor */
void *fn_8045620C(void *object, RwInt32 offset, RwInt32 size) {
    return object;
}

/* light plugin constructor: not in a clump */
void *fn_80456210(void *object, RwInt32 offset, RwInt32 size) {
    RpClumpObjectExtension *extension = CLUMPLIGHTEXT(object);

    extension->inClumpLink.prev = NULL;
    extension->inClumpLink.next = NULL;
    extension->clump = NULL;
    return object;
}

/* light plugin destructor */
void *fn_8045622C(void *object, RwInt32 offset, RwInt32 size) {
    return object;
}

/* the default clump callback */
RpClump *fn_80456230(RpClump *clump, void *data) {
    return clump;
}

/* Reads an atomic: its header, its frame and geometry from the clump's lists (or its own
 * geometry), then its plugin data and rights */
RpAtomic *fn_80456234(RwStream *stream, RwFrameList *frameList, RpGeometryList *geomList) {
    RwUInt32 length;
    RwUInt32 version;
    RwBool status;
    AtomicChunk header;
    RpAtomic *atomic;
    RpGeometry *geometry;

    if (!fn_8047CF50(stream, rwID_STRUCT, &length, &version)) {
        RWERROR((E_RW_READ));
        return NULL;
    }
    if (version >= rwSTREAMOLDESTVERSION && version <= RW_STREAM_VERSION) {
        memset(&header, 0, sizeof(header));
        status = fn_80480D4C(stream, &header, length) == length;
        if (!status) {
            RWERROR((E_RW_READ));
            return NULL;
        }
        fn_8047D254(&header, sizeof(header));

        atomic = fn_80456DD8();
        if (atomic == NULL) {
            return NULL;
        }
        atomic->object.flags = header.flags;
        if (frameList->numFrames != 0) {
            fn_80458CBC(atomic, frameList->frames[header.frameIndex]);
        }

        if (geomList->numGeoms != 0) {
            fn_80456F0C(atomic, geomList->geometries[header.geomIndex], 0);
        } else {
            if (!fn_8047CF50(stream, rwID_GEOMETRY, NULL, &version)) {
                fn_80456FD8(atomic);
                RWERROR((E_RW_READ));
                return NULL;
            }
            if (version >= rwSTREAMOLDESTVERSION && version <= RW_STREAM_VERSION) {
                geometry = fn_804599C8(stream);
                if (geometry == NULL) {
                    fn_80456FD8(atomic);
                    RWERROR((E_RW_READ));
                    return NULL;
                }
            } else {
                fn_80456FD8(atomic);
                RWERROR((E_RW_BADVERSION));
                return NULL;
            }
            /* the atomic keeps its own reference to the geometry */
            fn_80456F0C(atomic, geometry, 0);
            fn_8045985C(geometry);
        }

        lbl_805F14C4 = 0;
        lbl_805F14C0 = 0;
        status = fn_804816AC(&lbl_8056D7E0, stream, atomic) != NULL;
        if (!status) {
            RWERROR((E_RW_READ));
            return NULL;
        }
        if (lbl_805F14C4 != 0) {
            fn_80481870(&lbl_8056D7E0, lbl_805F14C4, atomic, lbl_805F14C0);
        }
        return atomic;
    }
    RWERROR((E_RW_BADVERSION));
    return NULL;
}

/* Brings an atomic's bounding sphere up to date with its morph targets: interpolated between two,
 * one, or the first */
void fn_80456560(RpAtomic *atomic) {
    RpGeometry *geometry = atomic->geometry;
    RpInterpolator *interpolator;
    RpMorphTarget *start;
    RpMorphTarget *end;
    RwReal scale;

    if (geometry == NULL) {
        return;
    }
    interpolator = &atomic->interpolator;
    if (interpolator->startMorphTarget == interpolator->endMorphTarget ||
        interpolator->startMorphTarget >= geometry->numMorphTargets ||
        interpolator->endMorphTarget >= geometry->numMorphTargets) {
        if (interpolator->startMorphTarget >= geometry->numMorphTargets ||
            interpolator->endMorphTarget >= geometry->numMorphTargets) {
            atomic->boundingSphere = geometry->morphTarget[0].boundingSphere;
        } else {
            atomic->boundingSphere =
                geometry->morphTarget[interpolator->startMorphTarget].boundingSphere;
        }
    } else {
        start = &geometry->morphTarget[interpolator->startMorphTarget];
        end = &geometry->morphTarget[interpolator->endMorphTarget];
        scale = interpolator->recipTime * interpolator->position;
        atomic->boundingSphere.radius =
            start->boundingSphere.radius +
            scale * (end->boundingSphere.radius - start->boundingSphere.radius);
        atomic->boundingSphere.center.x =
            end->boundingSphere.center.x - start->boundingSphere.center.x;
        atomic->boundingSphere.center.y =
            end->boundingSphere.center.y - start->boundingSphere.center.y;
        atomic->boundingSphere.center.z =
            end->boundingSphere.center.z - start->boundingSphere.center.z;
        atomic->boundingSphere.center.x = atomic->boundingSphere.center.x * scale;
        atomic->boundingSphere.center.y = atomic->boundingSphere.center.y * scale;
        atomic->boundingSphere.center.z = atomic->boundingSphere.center.z * scale;
        atomic->boundingSphere.center.x =
            atomic->boundingSphere.center.x + start->boundingSphere.center.x;
        atomic->boundingSphere.center.y =
            atomic->boundingSphere.center.y + start->boundingSphere.center.y;
        atomic->boundingSphere.center.z =
            atomic->boundingSphere.center.z + start->boundingSphere.center.z;
    }
    interpolator->flags &= ~rpINTERPOLATORDIRTYSPHERE;
    atomic->object.privateFlags |= rpATOMICSPHEREDIRTY;
}

/* The largest of three values */
#define RwRealMax2(a, b) ((a) >= (b) ? (a) : (b))
#define RwRealMax3(a, b, c) RwRealMax2(a, RwRealMax2(b, c))

/* Brings an atomic's world bounding sphere up to date with its frame and returns it */
RwSphere *fn_804566D0(RpAtomic *atomic) {
    RwFrame *frame = atomic->object.parent;
    RwMatrix *ltm;
    RwReal rightLength2;
    RwReal upLength2;
    RwReal atLength2;

    if (atomic->interpolator.flags & rpINTERPOLATORDIRTYSPHERE) {
        fn_80456560(atomic);
    }
    if (fn_80486CF0(frame) || (atomic->object.privateFlags & rpATOMICSPHEREDIRTY)) {
        ltm = RwFrameGetLTM(frame);
        fn_80482458(&atomic->worldBoundingSphere.center, &atomic->boundingSphere.center, ltm);
        if ((ltm->flags & rwMATRIXTYPEORTHONORMAL) != rwMATRIXTYPEORTHONORMAL) {
            /* a scaled frame scales the radius by its longest axis */
            rightLength2 = ltm->right.x * ltm->right.x + ltm->right.y * ltm->right.y +
                           ltm->right.z * ltm->right.z;
            upLength2 = ltm->up.x * ltm->up.x + ltm->up.y * ltm->up.y + ltm->up.z * ltm->up.z;
            atLength2 = ltm->at.x * ltm->at.x + ltm->at.y * ltm->at.y + ltm->at.z * ltm->at.z;
            atomic->worldBoundingSphere.radius =
                atomic->boundingSphere.radius *
                fn_80482340(RwRealMax3(rightLength2, upLength2, atLength2));
        } else {
            atomic->worldBoundingSphere.radius = atomic->boundingSphere.radius;
        }
        atomic->object.privateFlags &= ~rpATOMICSPHEREDIRTY;
    }
    return &atomic->worldBoundingSphere;
}

/* engine plugin destructor: destroys the clumps and atomics still in use, then the free lists */
void *fn_80456994(void *instance, RwInt32 offset, RwInt32 size) {
    fn_804800DC(CLUMPGLOBAL(clumpFreeList), fn_80455D0C, NULL);
    fn_804800DC(CLUMPGLOBAL(atomicFreeList), fn_80455D2C, NULL);
    fn_8047FCDC(CLUMPGLOBAL(atomicFreeList));
    fn_8047FCDC(CLUMPGLOBAL(clumpFreeList));
    CLUMPGLOBAL(atomicFreeList) = NULL;
    CLUMPGLOBAL(clumpFreeList) = NULL;
    lbl_805F14C8.numInstances--;
    return instance;
}

/* engine plugin constructor: the atomic and clump free lists */
void *fn_80456A50(void *instance, RwInt32 offset, RwInt32 size) {
    ClumpGlobals *globals;

    lbl_805F14C8.globalsOffset = offset;
    CLUMPGLOBAL(atomicFreeList) =
        fn_8047FC04(lbl_8056D7E0.sizeOfStruct, lbl_805EDDA0, 4, lbl_805EDDA4, &lbl_805E1580,
                    rwMEMHINTDUR_GLOBAL | rwID_CLUMP);
    if (CLUMPGLOBAL(atomicFreeList) != NULL) {
        CLUMPGLOBAL(clumpFreeList) =
            fn_8047FC04(lbl_8056D7F8.sizeOfStruct, lbl_805EDDA8, 4, lbl_805EDDAC, &lbl_805E15A4,
                        rwMEMHINTDUR_GLOBAL | rwID_ATOMIC);
        globals = CLUMPGLOBALS();
        if (globals->clumpFreeList != NULL) {
            lbl_805F14C8.numInstances++;
            return instance;
        }
        fn_8047FCDC(globals->atomicFreeList);
        CLUMPGLOBAL(atomicFreeList) = NULL;
    }
    return NULL;
}

/* Registers the clump plugin data of cameras and lights */
RwBool fn_80456B48(void) {
    lbl_805F14B8 =
        fn_804859B8(sizeof(RpClumpObjectExtension), rwID_CLUMP, fn_804561F0, fn_8045620C, NULL);
    if (lbl_805F14B8 < 0) {
        return FALSE;
    }
    lbl_805F14BC =
        fn_8045AAA0(sizeof(RpClumpObjectExtension), rwID_CLUMP, fn_80456210, fn_8045622C, NULL);
    if (lbl_805F14BC < 0) {
        return FALSE;
    }
    return TRUE;
}

/* Renders a clump's atomics that are flagged for rendering */
RpClump *fn_80456BC4(RpClump *clump) {
    RpClump *result = clump;
    RwLLLink *cur = clump->atomicList.link.next;
    RwLLLink *end = &clump->atomicList.link;
    RpAtomic *atomic;

    while (cur != end) {
        atomic = CLUMPATOMICFROMLINK(cur);
        if (atomic->object.flags & rpATOMICRENDER) {
            RwFrameGetLTM(atomic->object.parent);
            if (atomic->renderCallBack(atomic) == NULL) {
                result = NULL;
            }
        }
        cur = cur->next;
    }
    return result;
}

/* Calls a function for each atomic of a clump until it returns NULL */
RpClump *fn_80456C54(RpClump *clump, RpAtomicCallBack callBack, void *data) {
    RwLLLink *end = &clump->atomicList.link;
    RwLLLink *cur = clump->atomicList.link.next;
    RwLLLink *next;

    while (cur != end) {
        next = cur->next;
        if (callBack(CLUMPATOMICFROMLINK(cur), data) == NULL) {
            return clump;
        }
        cur = next;
    }
    return clump;
}

/* Calls a function for each camera of a clump until it returns NULL */
RpClump *fn_80456CD0(RpClump *clump, RwCameraCallBack callBack, void *data) {
    RwLLLink *end = &clump->cameraList.link;
    RwLLLink *cur = clump->cameraList.link.next;
    RwLLLink *next;

    while (cur != end) {
        next = cur->next;
        if (callBack(CLUMPCAMERAFROMLINK(cur), data) == NULL) {
            return clump;
        }
        cur = next;
    }
    return clump;
}

/* Calls a function for each light of a clump until it returns NULL */
RpClump *fn_80456D54(RpClump *clump, RpLightCallBack callBack, void *data) {
    RwLLLink *end = &clump->lightList.link;
    RwLLLink *cur = clump->lightList.link.next;
    RwLLLink *next;

    while (cur != end) {
        next = cur->next;
        if (callBack(CLUMPLIGHTFROMLINK(cur), data) == NULL) {
            return clump;
        }
        cur = next;
    }
    return clump;
}

RpAtomic *fn_80456DD8(void) {
    RpAtomic *atomic =
        RwFreeListAlloc(CLUMPGLOBAL(atomicFreeList), rwMEMHINTDUR_EVENT | rwID_ATOMIC);

    if (atomic == NULL) {
        return NULL;
    }

    atomic->object.type = rpATOMIC;
    atomic->object.subType = 0;
    atomic->object.flags = 0;
    atomic->object.privateFlags = 0;
    atomic->object.parent = NULL;
    atomic->sync = fn_80455E6C;
    atomic->repEntry = NULL;
    atomic->object.flags = rpATOMICCOLLISIONTEST | rpATOMICRENDER;
    atomic->object.privateFlags = rpATOMICSPHEREDIRTY;
    fn_80458CBC(atomic, NULL);

    atomic->geometry = NULL;
    atomic->boundingSphere.radius = 0.0f;
    atomic->boundingSphere.center.x = 0.0f;
    atomic->boundingSphere.center.y = 0.0f;
    atomic->boundingSphere.center.z = 0.0f;
    atomic->worldBoundingSphere.radius = 0.0f;
    atomic->worldBoundingSphere.center.x = 0.0f;
    atomic->worldBoundingSphere.center.y = 0.0f;
    atomic->worldBoundingSphere.center.z = 0.0f;
    RpAtomicSetRenderCallBack(atomic, fn_80455EB4);

    atomic->interpolator.startMorphTarget = 0;
    atomic->interpolator.endMorphTarget = 0;
    atomic->interpolator.time = 1.0f;
    atomic->interpolator.recipTime = 1.0f;
    atomic->interpolator.position = 0.0f;
    atomic->interpolator.flags = rpINTERPOLATORDIRTYINSTANCE | rpINTERPOLATORDIRTYSPHERE;
    atomic->inClumpLink.prev = NULL;
    atomic->inClumpLink.next = NULL;
    atomic->clump = NULL;
    atomic->pipeline = NULL;
    rwLinkListInitialize(&atomic->llWorldSectorsInAtomic);

    fn_8048209C(&lbl_8056D7E0, atomic);
    return atomic;
}

RpAtomic *fn_80456F0C(RpAtomic *atomic, RpGeometry *geometry, RwUInt32 flags) {
    RwFrame *frame;

    if (geometry != atomic->geometry) {
        if (geometry != NULL) {
            fn_8045984C(geometry);
        }
        if (atomic->geometry != NULL) {
            fn_8045985C(atomic->geometry);
        }
        atomic->geometry = geometry;
        if (!(flags & rpATOMICSAMEBOUNDINGSPHERE)) {
            if (geometry != NULL) {
                atomic->boundingSphere = geometry->morphTarget[0].boundingSphere;
            }
            frame = atomic->object.parent;
            if (frame != NULL && fn_804622F0(atomic)) {
                fn_8048722C(frame);
            }
        }
    }
    return atomic;
}

RwBool fn_80456FD8(RpAtomic *atomic) {
    fn_80482144(&lbl_8056D7E0, atomic);
    if (atomic->repEntry != NULL) {
        fn_8048071C(atomic->repEntry);
    }
    fn_80456F0C(atomic, NULL, 0);
    fn_8048D834(atomic);
    RwFreeListFree(CLUMPGLOBAL(atomicFreeList), atomic);
    return TRUE;
}

/* Clones an atomic: its object flags, geometry, bounding sphere, render callback, pipeline,
 * interpolator and plugin data, without a frame */
static inline RpAtomic *AtomicClone(RpAtomic *atomic) {
    RpAtomic *newAtomic = fn_80456DD8();

    if (newAtomic == NULL) {
        return NULL;
    }
    newAtomic->object.type = atomic->object.type;
    newAtomic->object.subType = atomic->object.subType;
    newAtomic->object.flags = atomic->object.flags;
    newAtomic->object.privateFlags = atomic->object.privateFlags;
    newAtomic->object.parent = NULL;
    newAtomic->object.flags = atomic->object.flags;
    fn_80458CBC(newAtomic, NULL);
    fn_80456F0C(newAtomic, atomic->geometry, 0);
    newAtomic->boundingSphere = atomic->boundingSphere;
    newAtomic->renderCallBack = atomic->renderCallBack;
    newAtomic->pipeline = atomic->pipeline;
    newAtomic->interpolator = atomic->interpolator;
    fn_804821B0(&lbl_8056D7E0, newAtomic, atomic);
    return newAtomic;
}

/* The state of a clump clone (inferred) */
typedef struct ClumpCloneData {
    RpClump *clump;
    RpClump *newClump;
    RwBool cloned;
} ClumpCloneData;

/* Clones an atomic of the clump onto the clone of its frame; NULL stops the cloning. As in the
 * original, an atomic without a frame stops it without marking the clone failed. */
static inline RpAtomic *ClumpCloneAtomic(RpAtomic *atomic, void *data) {
    ClumpCloneData *cloneData = data;
    RpAtomic *newAtomic = AtomicClone(atomic);
    RwFrame *frame;

    if (newAtomic == NULL) {
        cloneData->cloned = FALSE;
        return NULL;
    }
    frame = atomic->object.parent;
    if (frame == NULL) {
        fn_80456FD8(newAtomic);
        RWERROR((E_RW_ATOMICNOFRAME));
        return NULL;
    }
    /* the clone of the atomic's frame, which the cloned hierarchy keeps in its root */
    fn_80458CBC(newAtomic, frame->root);
    fn_80457608(cloneData->newClump, newAtomic);
    return atomic;
}

/* Clones a clump: its frame hierarchy, then each atomic on the matching cloned frame */
RpClump *fn_80457090(RpClump *clump) {
    ClumpCloneData cloneData;
    RpAtomic *atomic;
    RwLLLink *end;
    RwLLLink *cur;
    RwLLLink *next;

    cloneData.clump = clump;
    cloneData.newClump = fn_804573A8();
    if (cloneData.newClump == NULL) {
        return NULL;
    }
    cloneData.newClump->object.parent = fn_80486C5C(cloneData.clump->object.parent);
    if (cloneData.newClump->object.parent == NULL) {
        fn_80457468(cloneData.newClump);
        return NULL;
    }

    cloneData.cloned = TRUE;
    end = &cloneData.clump->atomicList.link;
    cur = cloneData.clump->atomicList.link.next;
    while (cur != end) {
        next = cur->next;
        atomic = CLUMPATOMICFROMLINK(cur);
        if (ClumpCloneAtomic(atomic, &cloneData) == NULL) {
            break;
        }
        cur = next;
    }

    fn_80486CA4(cloneData.clump->object.parent);
    if (!cloneData.cloned) {
        fn_80457468(cloneData.newClump);
        return NULL;
    }
    cloneData.newClump->callback = cloneData.clump->callback;
    fn_804821B0(&lbl_8056D7F8, cloneData.newClump, cloneData.clump);
    return cloneData.newClump;
}

RpClump *fn_804573A8(void) {
    RpClump *clump = RwFreeListAlloc(CLUMPGLOBAL(clumpFreeList), rwMEMHINTDUR_EVENT | rwID_CLUMP);

    if (clump == NULL) {
        return clump;
    }
    clump->object.type = rpCLUMP;
    clump->object.subType = 0;
    clump->object.flags = 0;
    clump->object.privateFlags = 0;
    clump->object.parent = NULL;
    clump->object.parent = NULL;
    rwLinkListInitialize(&clump->atomicList);
    rwLinkListInitialize(&clump->lightList);
    rwLinkListInitialize(&clump->cameraList);
    clump->inWorldLink.prev = NULL;
    clump->inWorldLink.next = NULL;
    clump->callback = fn_80456230;
    fn_8048209C(&lbl_8056D7F8, clump);
    return clump;
}

/* clump destroy callbacks for its atomics, lights and cameras */
static inline RpAtomic *ClumpDestroyAtomic(RpAtomic *atomic) {
    fn_80456FD8(atomic);
    return atomic;
}

static inline RpLight *ClumpDestroyLight(RpLight *light) {
    fn_80457630(CLUMPLIGHTEXT(light)->clump, light);
    fn_8045ACD4(light);
    return light;
}

static inline RwCamera *ClumpDestroyCamera(RwCamera *camera) {
    fn_80457664(CLUMPCAMERAEXT(camera)->clump, camera);
    fn_804859F8(camera);
    return camera;
}

/* Destroys a clump with its atomics, lights, cameras and frame hierarchy */
RwBool fn_80457468(RpClump *clump) {
    fn_80482144(&lbl_8056D7F8, clump);

    {
        RwLLLink *cur = clump->atomicList.link.next;
        RwLLLink *end = &clump->atomicList.link;

        while (cur != end) {
            RpAtomic *atomic = CLUMPATOMICFROMLINK(cur);

            cur = cur->next;
            if (ClumpDestroyAtomic(atomic) == NULL) {
                break;
            }
        }
    }
    {
        RwLLLink *cur = clump->lightList.link.next;
        RwLLLink *end = &clump->lightList.link;

        while (cur != end) {
            RpLight *light = CLUMPLIGHTFROMLINK(cur);

            cur = cur->next;
            if (ClumpDestroyLight(light) == NULL) {
                break;
            }
        }
    }
    {
        RwLLLink *cur = clump->cameraList.link.next;
        RwLLLink *end = &clump->cameraList.link;

        while (cur != end) {
            RwCamera *camera = CLUMPCAMERAFROMLINK(cur);

            cur = cur->next;
            if (ClumpDestroyCamera(camera) == NULL) {
                break;
            }
        }
    }

    if (clump->object.parent != NULL) {
        fn_80487208(clump->object.parent);
    }
    RwFreeListFree(CLUMPGLOBAL(clumpFreeList), clump);
    return TRUE;
}

RpClump *fn_80457608(RpClump *clump, RpAtomic *atomic) {
    rwLinkListAddLLLink(&clump->atomicList, &atomic->inClumpLink);
    atomic->clump = clump;
    return clump;
}

/* Takes a light out of its clump */
RpClump *fn_80457630(RpClump *clump, RpLight *light) {
    RpClumpObjectExtension *extension = CLUMPLIGHTEXT(light);

    rwLinkListRemoveLLLink(&extension->inClumpLink);
    extension->inClumpLink.prev = NULL;
    extension->inClumpLink.next = NULL;
    extension->clump = NULL;
    return clump;
}

/* Takes a camera out of its clump */
RpClump *fn_80457664(RpClump *clump, RwCamera *camera) {
    RpClumpObjectExtension *extension = CLUMPCAMERAEXT(camera);

    rwLinkListRemoveLLLink(&extension->inClumpLink);
    extension->inClumpLink.prev = NULL;
    extension->inClumpLink.next = NULL;
    extension->clump = NULL;
    return clump;
}

/* Reads a clump: its header, frame list and geometry list, then its atomics, lights (each on a
 * frame from the list) and cameras, then its plugin data */
RpClump *fn_80457698(RwStream *stream) {
    RwUInt32 length;
    RwUInt32 version;
    RwUInt32 chunkVersion;
    RwInt32 lightFrameIndex;
    RwInt32 cameraFrameIndex;
    ClumpChunk header;
    RpClump *clump;
    RpAtomic *atomic;
    RpLight *light;
    RwCamera *camera;
    RpClumpObjectExtension *extension;
    RwBool status;
    RwInt32 i;

    if (!fn_8047CF50(stream, rwID_STRUCT, &length, &version)) {
        RWERROR((E_RW_READ));
        return NULL;
    }
    if (version >= rwSTREAMOLDESTVERSION && version <= RW_STREAM_VERSION) {
        RwFrameList frameList;
        RpGeometryList geomList;

        status = fn_80480D4C(stream, &header, sizeof(header)) == sizeof(header);
        if (!status) {
            RWERROR((E_RW_READ));
            return NULL;
        }
        fn_8047D254(&header, sizeof(header));

        clump = fn_804573A8();
        if (clump == NULL) {
            return NULL;
        }

        if (!fn_8047CF50(stream, rwID_FRAMELIST, NULL, &chunkVersion)) {
            fn_80457468(clump);
            RWERROR((E_RW_READ));
            return NULL;
        }
        status = fn_80483774(stream, &frameList) != NULL;
        if (!status) {
            fn_80457468(clump);
            RWERROR((E_RW_READ));
            return NULL;
        }
        clump->object.parent = frameList.frames[0];

        if (!fn_8047CF50(stream, rwID_GEOMETRYLIST, NULL, &chunkVersion)) {
            fn_80483728(&frameList);
            fn_80457468(clump);
            RWERROR((E_RW_READ));
            return NULL;
        }
        status = fn_80455F18(stream, &geomList) != NULL;
        if (!status) {
            fn_80483728(&frameList);
            fn_80457468(clump);
            RWERROR((E_RW_READ));
            return NULL;
        }

        for (i = 0; i < header.numAtomics; i++) {
            status = fn_8047CF50(stream, rwID_ATOMIC, NULL, &version);
            if (status) {
                atomic = fn_80456234(stream, &frameList, &geomList);
                status = atomic != NULL;
            }
            if (!status) {
                GeometryListDeinitialize(&geomList);
                fn_80483728(&frameList);
                fn_80457468(clump);
                RWERROR((E_RW_READ));
                return NULL;
            }
            rwLinkListAddLLLink(&clump->atomicList, &atomic->inClumpLink);
            atomic->clump = clump;
        }

        for (i = 0; i < header.numLights; i++) {
            status = FALSE;
            if (fn_8047CF50(stream, rwID_STRUCT, NULL, NULL) &&
                fn_8047D868(stream, &lightFrameIndex, sizeof(RwInt32)) != NULL) {
                status = TRUE;
            }
            if (status) {
                status = fn_8047CF50(stream, rwID_LIGHT, NULL, NULL);
            }
            if (status) {
                light = fn_8045AB1C(stream);
                status = light != NULL;
            }
            if (!status) {
                GeometryListDeinitialize(&geomList);
                fn_80483728(&frameList);
                fn_80457468(clump);
                RWERROR((E_RW_READ));
                return NULL;
            }
            fn_8048D7C0(light, frameList.frames[lightFrameIndex]);
            extension = CLUMPLIGHTEXT(light);
            rwLinkListAddLLLink(&clump->lightList, &extension->inClumpLink);
            extension->clump = clump;
        }

        for (i = 0; i < header.numCameras; i++) {
            status = FALSE;
            if (fn_8047CF50(stream, rwID_STRUCT, NULL, NULL) &&
                fn_8047D868(stream, &cameraFrameIndex, sizeof(RwInt32)) != NULL) {
                status = TRUE;
            }
            if (status) {
                status = fn_8047CF50(stream, rwID_CAMERA, NULL, NULL);
            }
            if (status) {
                camera = fn_80483590(stream);
                status = camera != NULL;
            }
            if (!status) {
                GeometryListDeinitialize(&geomList);
                fn_80483728(&frameList);
                fn_80457468(clump);
                RWERROR((E_RW_READ));
                return NULL;
            }
            fn_8048D7C0(camera, frameList.frames[cameraFrameIndex]);
            extension = CLUMPCAMERAEXT(camera);
            rwLinkListAddLLLink(&clump->cameraList, &extension->inClumpLink);
            extension->clump = clump;
        }

        GeometryListDeinitialize(&geomList);
        fn_80483728(&frameList);
        status = fn_804816AC(&lbl_8056D7F8, stream, clump) != NULL;
        if (!status) {
            fn_80457468(clump);
            RWERROR((E_RW_READ));
            return NULL;
        }
        return clump;
    }
    RWERROR((E_RW_BADVERSION));
    return NULL;
}

/* plugin registration with atomics and clumps */
RwInt32 fn_80458B6C(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor, RwPluginObjectCopy copy) {
    return fn_80481DC8(&lbl_8056D7E0, size, pluginID, constructor, destructor, copy);
}

RwInt32 fn_80458BAC(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor, RwPluginObjectCopy copy) {
    return fn_80481DC8(&lbl_8056D7F8, size, pluginID, constructor, destructor, copy);
}

RwInt32 fn_80458BEC(RwUInt32 pluginID, RwPluginDataChunkReadCallBack read,
                    RwPluginDataChunkWriteCallBack write,
                    RwPluginDataChunkGetSizeCallBack getSize) {
    return fn_804815F0(&lbl_8056D7E0, pluginID, read, write, getSize);
}

RwInt32 fn_80458C28(RwUInt32 pluginID,
                    RwBool (*callBack)(void *object, RwInt32 offset, RwInt32 size)) {
    return fn_80481634(&lbl_8056D7E0, pluginID, callBack);
}

RwInt32 fn_80458C5C(RwUInt32 pluginID, RwBool (*callBack)(void *object, RwInt32 offset,
                                                          RwInt32 size, RwUInt32 extraData)) {
    return fn_80481670(&lbl_8056D7E0, pluginID, callBack);
}

RwInt32 fn_80458C90(RwUInt32 pluginID) {
    return fn_80481D98(&lbl_8056D7E0, pluginID);
}

RpAtomic *fn_80458CBC(RpAtomic *atomic, RwFrame *frame) {
    fn_8048D7C0(atomic, frame);
    atomic->object.privateFlags |= rpATOMICSPHEREDIRTY;
    return atomic;
}
