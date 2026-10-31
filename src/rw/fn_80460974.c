/* RenderWare world objects: the world object module and its tie free lists, the world plugin
 * data of cameras, atomics, clumps and lights, tying atomics and lights to the world sectors they
 * touch, finding the sectors in a camera's view frustum, the binary mesh and native data stream
 * plugins, and adding objects to and removing them from worlds. The original file name is
 * unknown. */
#include "rw/rpworld.h"
#include <stddef.h>
#include "layout_check.h"

/* engine and object plugin IDs (inferred) */
#define rwID_WORLDMODULE 0x507
#define rwID_WORLDOBJMODULE 0x509
#define rwID_BINMESHPLUGIN 0x50E
#define rwID_NATIVEDATAPLUGIN 0x510
#define rwID_RIGHTTORENDER 0x1F

/* errors */
#define E_RW_ERROR16 0x80000016      /* an engine error code; its name is unknown */
#define E_RP_WORLD_CLUMPNOTINWORLD 4 /* the world plugin's error code (inferred) */

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

/* the deepest sector tree that the tree walks handle */
#define rpWORLDMAXBSPDEPTH 64

/* how many sectors a camera's frustum sector list grows by */
#define rpFRUSTUMSECTORGRANULARITY 50

/* world render orders */
#define rpWORLDRENDERFRONT2BACK 1
#define rpWORLDRENDERBACK2FRONT 2

/* the engine standard function that hints the world sector render order */
#define rwSTANDARDHINTRENDERF2B 22

/* light types with a position (rpLIGHTPOINT and up) */
#define rpLIGHTPOSITIONINGSTART 0x80

/* A coordinate of a vector, by its byte offset (a plane sector's type) */
#define GETCOORD(vector, offset) (*(RwReal *)((RwUInt8 *)&(vector) + (offset)))

/* The world data of a camera */
typedef struct CameraExt {
    /* 0x00 */ RpWorldSector **frustumSectors; /* the sectors in its view frustum */
    /* 0x04 */ RwInt32 space;                  /* of the frustum sector list */
    /* 0x08 */ RwInt32 numFrustumSectors;
    /* 0x0C */ RpWorld *world;
    /* 0x10 */ RwCameraBeginUpdateFunc oldBeginUpdate;
    /* 0x14 */ RwCameraEndUpdateFunc oldEndUpdate;
    /* 0x18 */ RwObjectHasFrameSyncFunction oldSync;
} CameraExt;
CHECK_SIZEOF(CameraExt, 0x1C);

/* The world data of an atomic */
typedef struct AtomicExt {
    /* 0x0 */ RpWorld *world;
    /* 0x4 */ RwObjectHasFrameSyncFunction oldSync;
} AtomicExt;
CHECK_SIZEOF(AtomicExt, 0x8);

/* The world data of a clump */
typedef struct ClumpExt {
    /* 0x0 */ RpWorld *world;
    /* 0x4 */ RwInt32 field_0x4; /* the module's field_0x8 when the clump is created or added */
} ClumpExt;
CHECK_SIZEOF(ClumpExt, 0x8);

/* The world data of a light */
typedef struct LightExt {
    /* 0x0 */ RpWorld *world;
    /* 0x4 */ RwObjectHasFrameSyncFunction oldSync;
} LightExt;
CHECK_SIZEOF(LightExt, 0x8);

/* The world object module's globals in the engine (0xC bytes) */
typedef struct WorldObjGlobals {
    /* 0x0 */ RwFreeList *tieFreeList;
    /* 0x4 */ RwFreeList *lightTieFreeList;
    /* 0x8 */ RwInt32 field_0x8;
} WorldObjGlobals;
CHECK_SIZEOF(WorldObjGlobals, 0xC);

/* The world object module (inferred) */
typedef struct WorldObjModule {
    /* 0x0 */ RwInt32 globalsOffset; /* engine plugin offset of the globals */
    /* 0x4 */ RwInt32 numInstances;  /* the open engine instances */
} WorldObjModule;
CHECK_SIZEOF(WorldObjModule, 0x8);

#define WORLDOBJGLOBAL(var)                                                                        \
    (RWPLUGINOFFSET(WorldObjGlobals, lbl_805F15B4, lbl_805F1518.globalsOffset)->var)

#define CAMERAEXT(camera) RWPLUGINOFFSET(CameraExt, camera, lbl_805F1508)
#define ATOMICEXT(atomic) RWPLUGINOFFSET(AtomicExt, atomic, lbl_805F150C)
#define CLUMPEXT(clump) RWPLUGINOFFSET(ClumpExt, clump, lbl_805F1510)
#define LIGHTEXT(light) RWPLUGINOFFSET(LightExt, light, lbl_805F1514)

/* The tie of a link in a light's sector list */
#define LIGHTTIEFROMLINK(link)                                                                     \
    ((RpLightTie *)((RwUInt8 *)(link) - offsetof(RpLightTie, WorldSectorInLight)))

/* Unlinks a tie from its sector and atomic and frees it */
#define TIEDESTROY(tie)                                                                            \
    do {                                                                                           \
        rwLinkListRemoveLLLink(&(tie)->lWorldSectorInAtomic);                                      \
        rwLinkListRemoveLLLink(&(tie)->lAtomicInWorldSector);                                      \
        RwFreeListFree(WORLDOBJGLOBAL(tieFreeList), (tie));                                        \
    } while (0)

/* Unlinks a light tie from its sector and light and frees it */
#define LIGHTTIEDESTROY(tie)                                                                       \
    do {                                                                                           \
        rwLinkListRemoveLLLink(&(tie)->WorldSectorInLight);                                        \
        rwLinkListRemoveLLLink(&(tie)->lightInWorldSector);                                        \
        RwFreeListFree(WORLDOBJGLOBAL(lightTieFreeList), (tie));                                   \
    } while (0)

RwFreeList lbl_805E1640; /* the tie free list */
RwFreeList lbl_805E1664; /* the light tie free list */

RwInt32 lbl_805EDDC8 = 256; /* tie free list entries per block */
RwInt32 lbl_805EDDCC = 1;   /* tie free list blocks to preallocate */
RwInt32 lbl_805EDDD0 = 32;  /* light tie free list entries per block */
RwInt32 lbl_805EDDD4 = 1;   /* light tie free list blocks to preallocate */

/* declared in reverse address order (the compiler places them in .sbss last first) */
WorldObjModule lbl_805F1518;
RwInt32 lbl_805F1514; /* light plugin offset */
RwInt32 lbl_805F1510; /* clump plugin offset */
RwInt32 lbl_805F150C; /* atomic plugin offset */
RwInt32 lbl_805F1508; /* camera plugin offset */

/* engine plugin constructor: the tie free lists */
void *fn_80460974(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F1518.globalsOffset = offset;
    WORLDOBJGLOBAL(tieFreeList) =
        fn_8047FC04(sizeof(RpTie), lbl_805EDDC8, 4, lbl_805EDDCC, &lbl_805E1640,
                    rwMEMHINTDUR_GLOBAL | rwID_WORLDMODULE);
    if (WORLDOBJGLOBAL(tieFreeList) == NULL) {
        return NULL;
    }
    WORLDOBJGLOBAL(lightTieFreeList) =
        fn_8047FC04(sizeof(RpLightTie), lbl_805EDDD0, 4, lbl_805EDDD4, &lbl_805E1664,
                    rwMEMHINTDUR_GLOBAL | rwID_WORLDMODULE);
    if (WORLDOBJGLOBAL(lightTieFreeList) == NULL) {
        fn_8047FCDC(WORLDOBJGLOBAL(tieFreeList));
        WORLDOBJGLOBAL(tieFreeList) = NULL;
        return NULL;
    }
    lbl_805F15B4->renderFrame = 1;
    WORLDOBJGLOBAL(field_0x8) = 0;
    lbl_805F1518.numInstances++;
    return instance;
}

/* engine plugin destructor */
void *fn_80460A88(void *instance, RwInt32 offset, RwInt32 size) {
    if (WORLDOBJGLOBAL(lightTieFreeList) != NULL) {
        fn_8047FCDC(WORLDOBJGLOBAL(lightTieFreeList));
        WORLDOBJGLOBAL(lightTieFreeList) = NULL;
    }
    if (WORLDOBJGLOBAL(tieFreeList) != NULL) {
        fn_8047FCDC(WORLDOBJGLOBAL(tieFreeList));
        WORLDOBJGLOBAL(tieFreeList) = NULL;
    }
    lbl_805F1518.numInstances--;
    return instance;
}

/* A camera's begin update: makes its world current and starts a render frame */
RwCamera *fn_80460B1C(RwCamera *camera) {
    CameraExt *ext = CAMERAEXT(camera);

    lbl_805F15B4->curWorld = ext->world;
    lbl_805F15B4->renderFrame++;
    return ext->oldBeginUpdate(camera);
}

/* A camera's end update */
RwCamera *fn_80460B68(RwCamera *camera) {
    CameraExt *ext = CAMERAEXT(camera);

    lbl_805F15B4->curWorld = NULL;
    return ext->oldEndUpdate(camera);
}

/* A camera's frame sync: finds the sectors in its view frustum */
RwCamera *fn_80460BA4(RwCamera *camera) {
    CameraExt *ext = CAMERAEXT(camera);

    if (ext->oldSync(camera) != NULL) {
        RpWorld *world = ext->world;

        if (world != NULL) {
            RwBool (*sortSectors)(void *out, void *inOut, RwInt32 numIn) =
                lbl_805F15B4->stdFunc[rwSTANDARDHINTRENDERF2B];

            fn_80461658(world, camera);
            sortSectors(NULL, NULL, world->renderOrder == rpWORLDRENDERFRONT2BACK);
        }
        return camera;
    }
    return NULL;
}

/* camera plugin constructor: hooks the camera's updates and sync */
void *fn_80460C48(void *object, RwInt32 offset, RwInt32 size) {
    RwCamera *camera = object;
    CameraExt *ext = CAMERAEXT(camera);

    ext->frustumSectors = NULL;
    ext->space = 0;
    ext->numFrustumSectors = 0;
    ext->oldBeginUpdate = camera->beginUpdate;
    ext->oldEndUpdate = camera->endUpdate;
    ext->oldSync = camera->sync;
    camera->sync = (RwObjectHasFrameSyncFunction)fn_80460BA4;
    camera->beginUpdate = fn_80460B1C;
    camera->endUpdate = fn_80460B68;
    ext->world = NULL;
    return object;
}

/* camera plugin copy: the copy joins the original's world */
void *fn_80460CA4(void *dstObject, const void *srcObject, RwInt32 offset, RwInt32 size) {
    CameraExt *ext = CAMERAEXT(dstObject);
    const CameraExt *srcExt = CAMERAEXT(srcObject);

    ext->frustumSectors = NULL;
    ext->space = 0;
    ext->numFrustumSectors = 0;
    if (srcExt->world != NULL) {
        fn_80462130(srcExt->world, dstObject);
    }
    return dstObject;
}

/* camera plugin destructor: restores the camera's updates and sync */
void *fn_80460D00(void *object, RwInt32 offset, RwInt32 size) {
    RwCamera *camera = object;
    CameraExt *ext = CAMERAEXT(camera);

    if (ext->frustumSectors != NULL) {
        RwFree(ext->frustumSectors);
    }
    ext->frustumSectors = NULL;
    ext->space = 0;
    ext->numFrustumSectors = 0;
    camera->beginUpdate = ext->oldBeginUpdate;
    camera->endUpdate = ext->oldEndUpdate;
    camera->sync = ext->oldSync;
    return object;
}

RwBool fn_80460D80(RpLightTie *tie) {
    LIGHTTIEDESTROY(tie);
    return TRUE;
}

RwBool fn_80460DF0(RpTie *tie) {
    if (tie->apAtom != NULL && tie->worldSector != NULL) {
        TIEDESTROY(tie);
    }
    return TRUE;
}

/* Ties an atomic to each world sector its bounding sphere's box meets */
void fn_80460E74(RpWorld *world, RpAtomic *atomic) {
    RpSector *sectorStack[rpWORLDMAXBSPDEPTH];
    RpSector *sector;
    RwInt32 depth = 0;
    const RwSphere *sphere = fn_804566D0(atomic);
    RwV3d inf;
    RwV3d sup;

    inf = sphere->center;
    sup = sphere->center;
    inf.x -= sphere->radius;
    sector = world->rootSector;
    inf.y -= sphere->radius;
    inf.z -= sphere->radius;
    sup.x += sphere->radius;
    sup.y += sphere->radius;
    sup.z += sphere->radius;

    do {
        RwInt32 type = sector->type;

        if (type < 0) {
            RpTie *tie =
                RwFreeListAlloc(WORLDOBJGLOBAL(tieFreeList), rwMEMHINTDUR_EVENT | rwID_WORLDMODULE);

            tie->worldSector = (RpWorldSector *)sector;
            tie->apAtom = atomic;
            rwLinkListAddLLLink(&((RpWorldSector *)sector)->collAtomicsInWorldSector,
                                &tie->lAtomicInWorldSector);
            rwLinkListAddLLLink(&atomic->llWorldSectorsInAtomic, &tie->lWorldSectorInAtomic);
            sector = sectorStack[depth--];
        } else {
            RpPlaneSector *plane = (RpPlaneSector *)sector;

            if (GETCOORD(inf, type) < plane->leftValue) {
                sector = plane->leftSubTree;
                if (plane->rightValue < GETCOORD(sup, type)) {
                    sectorStack[++depth] = plane->rightSubTree;
                }
            } else if (plane->rightValue < GETCOORD(sup, type)) {
                sector = plane->rightSubTree;
            } else {
                sector = sectorStack[depth--];
            }
        }
    } while (depth >= 0);
}

/* Removes an atomic's ties to world sectors */
static inline void AtomicUntie(RpAtomic *atomic) {
    RwLLLink *link = atomic->llWorldSectorsInAtomic.link.next;
    RwLLLink *end = &atomic->llWorldSectorsInAtomic.link;

    while (link != end) {
        RpTie *tie = TIEFROMATOMICLINK(link);

        link = link->next;
        if (tie->apAtom != NULL && tie->worldSector != NULL) {
            TIEDESTROY(tie);
        }
    }
}

/* An atomic's frame sync: reties it to the sectors it meets */
RpAtomic *fn_80461030(RpAtomic *atomic) {
    AtomicExt *ext = ATOMICEXT(atomic);

    if (ext->oldSync(atomic) != NULL) {
        RpWorld *world = ext->world;

        if (world != NULL) {
            AtomicUntie(atomic);
            fn_80460E74(world, atomic);
        }
        return atomic;
    }
    return NULL;
}

/* atomic plugin constructor: hooks the atomic's sync */
void *fn_8046112C(void *object, RwInt32 offset, RwInt32 size) {
    RpAtomic *atomic = object;
    AtomicExt *ext = ATOMICEXT(atomic);

    ext->world = NULL;
    atomic->renderFrame = lbl_805F15B4->renderFrame - 1;
    ext->oldSync = atomic->sync;
    atomic->sync = (RwObjectHasFrameSyncFunction)fn_80461030;
    return object;
}

/* atomic plugin copy: nothing to copy */
void *fn_80461164(void *dstObject, const void *srcObject, RwInt32 offset, RwInt32 size) {
    return dstObject;
}

/* atomic plugin destructor: unties the atomic and restores its sync */
void *fn_80461168(void *object, RwInt32 offset, RwInt32 size) {
    RpAtomic *atomic = object;
    AtomicExt *ext = ATOMICEXT(atomic);

    AtomicUntie(atomic);
    atomic->sync = ext->oldSync;
    return object;
}

/* clump plugin constructor */
void *fn_80461238(void *object, RwInt32 offset, RwInt32 size) {
    ClumpExt *ext = CLUMPEXT(object);

    ext->world = NULL;
    ext->field_0x4 = WORLDOBJGLOBAL(field_0x8);
    return object;
}

/* clump plugin copy: the copy joins the original's world */
void *fn_80461260(void *dstObject, const void *srcObject, RwInt32 offset, RwInt32 size) {
    if (CLUMPEXT(srcObject)->world != NULL) {
        fn_804622FC(CLUMPEXT(srcObject)->world, dstObject);
    }
    return dstObject;
}

/* clump plugin destructor: nothing to release */
void *fn_804612A4(void *object, RwInt32 offset, RwInt32 size) {
    return object;
}

/* Removes a light's ties to world sectors */
static inline void LightUntie(RpLight *light) {
    RwLLLink *link = light->WorldSectorsInLight.link.next;
    RwLLLink *end = &light->WorldSectorsInLight.link;

    while (link != end) {
        RpLightTie *tie = LIGHTTIEFROMLINK(link);

        link = link->next;
        LIGHTTIEDESTROY(tie);
    }
}

/* A light's frame sync: reties a positioned light to the sectors its sphere's box meets */
RpLight *fn_804612A8(RpLight *light) {
    LightExt *ext = LIGHTEXT(light);

    if (ext->oldSync(light) != NULL) {
        RpWorld *world;
        RwFrame *frame;

        if (RpLightGetType(light) < rpLIGHTPOSITIONINGSTART) {
            return light;
        }
        world = ext->world;
        frame = light->object.parent;
        if (world != NULL && frame != NULL) {
            RwReal radius = light->radius;
            RpSector *sectorStack[rpWORLDMAXBSPDEPTH];
            RwInt32 depth = 0;
            const RwV3d *position;
            RwV3d inf;
            RwV3d sup;
            RpSector *sector;

            LightUntie(light);
            position = &RwFrameGetLTM(frame)->pos;
            inf = *position;
            sector = world->rootSector;
            sup = *position;
            sup.x += radius;
            sup.y += radius;
            sup.z += radius;
            inf.x -= radius;
            inf.y -= radius;
            inf.z -= radius;

            do {
                RwInt32 type = sector->type;

                if (type < 0) {
                    RpLightTie *tie = RwFreeListAlloc(WORLDOBJGLOBAL(lightTieFreeList),
                                                      rwMEMHINTDUR_EVENT | rwID_WORLDMODULE);

                    tie->sect = (RpWorldSector *)sector;
                    tie->light = light;
                    rwLinkListAddLLLink(&((RpWorldSector *)sector)->lightsInWorldSector,
                                        &tie->lightInWorldSector);
                    rwLinkListAddLLLink(&light->WorldSectorsInLight, &tie->WorldSectorInLight);
                    sector = sectorStack[depth--];
                } else {
                    RpPlaneSector *plane = (RpPlaneSector *)sector;

                    if (GETCOORD(inf, type) < plane->leftValue) {
                        sector = plane->leftSubTree;
                        if (plane->rightValue < GETCOORD(sup, type)) {
                            sectorStack[++depth] = plane->rightSubTree;
                        }
                    } else if (plane->rightValue < GETCOORD(sup, type)) {
                        sector = plane->rightSubTree;
                    } else {
                        sector = sectorStack[depth--];
                    }
                }
            } while (depth >= 0);
        }
    } else {
        RWERROR((E_RW_ERROR16));
    }
    return light;
}

/* light plugin constructor: hooks the light's sync */
void *fn_80461548(void *object, RwInt32 offset, RwInt32 size) {
    RpLight *light = object;
    LightExt *ext = LIGHTEXT(light);

    ext->world = NULL;
    ext->oldSync = light->sync;
    light->sync = (RwObjectHasFrameSyncFunction)fn_804612A8;
    return object;
}

/* light plugin copy: the copy joins the original's world */
void *fn_80461570(void *dstObject, const void *srcObject, RwInt32 offset, RwInt32 size) {
    if (LIGHTEXT(srcObject)->world != NULL) {
        fn_80462578(LIGHTEXT(srcObject)->world, dstObject);
    }
    return dstObject;
}

/* light plugin destructor: unties the light */
void *fn_804615B4(void *object, RwInt32 offset, RwInt32 size) {
    RpLight *light = object;

    LightUntie(light);
    return object;
}

/* Makes room for more sectors in a camera's frustum sector list (inferred) */
static inline RwBool FrustumSectorsAddSpace(CameraExt *ext, RwInt32 amount) {
    RwInt32 bytes = (ext->space + amount) * sizeof(RpWorldSector *);
    RpWorldSector **sectors;

    if (ext->frustumSectors != NULL) {
        sectors = RwRealloc(ext->frustumSectors, bytes,
                            rwMEMHINTDUR_EVENT | rwID_WORLDMODULE | rwMEMHINTFLAG_RESIZABLE);
    } else {
        sectors = RwMalloc(bytes, rwMEMHINTDUR_EVENT | rwID_WORLDMODULE | rwMEMHINTFLAG_RESIZABLE);
    }
    if (sectors != NULL) {
        ext->frustumSectors = sectors;
        ext->space += amount;
        return TRUE;
    }
    RWERROR((E_RW_NOMEM, bytes));
    return FALSE;
}

/* Collects the world sectors in a camera's view frustum, nearer or farther first by the world's
 * render order */
RpWorld *fn_80461658(RpWorld *world, RwCamera *camera) {
    RpSector *sectorStack[rpWORLDMAXBSPDEPTH];
    RpSector *sector;
    RwV3d position;
    RwV3d inf;
    RwV3d sup;
    const RwFrustumPlane *frustumPlanes;
    CameraExt *ext;
    RwBool backToFront;
    RwInt32 depth = 0;
    RwInt32 numSectors = 0;

    ext = CAMERAEXT(camera);
    position = RwFrameGetLTM(camera->object.parent)->pos;
    inf = camera->frustumBoundBox.inf;
    sup = camera->frustumBoundBox.sup;
    frustumPlanes = camera->frustumPlanes;
    backToFront = world->renderOrder == rpWORLDRENDERBACK2FRONT;

    sector = ext->world->rootSector;
    do {
        RwInt32 type = sector->type;

        if (type < 0) {
            RwBool outside;
            RwInt32 i;
            RpWorldSector *worldSector = (RpWorldSector *)sector;
            /* The box's two corners, indexed by each plane's closestX/Y/Z */
            const RwV3d *const corners = (const RwV3d *)&worldSector->field_0x48;

            for (i = 0; i < 6; i++) {
                RwV3d corner;
                RwSplitBits side;

                corner.x = corners[frustumPlanes[i].closestX].x;
                corner.y = corners[frustumPlanes[i].closestY].y;
                corner.z = corners[frustumPlanes[i].closestZ].z;
                side.nReal = corner.x * frustumPlanes[i].plane.normal.x +
                             corner.y * frustumPlanes[i].plane.normal.y +
                             corner.z * frustumPlanes[i].plane.normal.z -
                             frustumPlanes[i].plane.distance;

                outside = side.nInt > 0;
                if (outside) {
                    break;
                }
            }
            if (!outside) {
                if (numSectors >= ext->space) {
                    if (!FrustumSectorsAddSpace(ext, rpFRUSTUMSECTORGRANULARITY)) {
                        ext->numFrustumSectors = numSectors;
                        return world;
                    }
                }
                ext->frustumSectors[numSectors++] = worldSector;
            }
            sector = sectorStack[depth--];
        } else {
            RpPlaneSector *planeSector = (RpPlaneSector *)sector;
            RwSplitBits left;
            RwSplitBits right;

            left.nReal = GETCOORD(inf, type) - planeSector->leftValue;
            right.nReal = planeSector->rightValue - GETCOORD(sup, type);

            if (left.nInt < 0 && right.nInt < 0) {
                /* the frustum meets both sides: the camera's side first, or last */
                RwBool beyond = GETCOORD(position, type) > planeSector->value;

                if ((backToFront && beyond) || (!backToFront && !beyond)) {
                    sector = planeSector->leftSubTree;
                    sectorStack[++depth] = planeSector->rightSubTree;
                } else {
                    sector = planeSector->rightSubTree;
                    sectorStack[++depth] = planeSector->leftSubTree;
                }
            } else {
                /* the frustum is on one side */
                sector = left.nInt < 0 ? planeSector->leftSubTree : planeSector->rightSubTree;
            }
        }
    } while (depth >= 0);
    ext->numFrustumSectors = numSectors;
    return world;
}

/* Clump object callbacks that add an object to a world, or remove it from one */
RpAtomic *fn_804619C8(RpAtomic *atomic, void *data) {
    fn_804621C0(data, atomic);
    return atomic;
}

RpAtomic *fn_80461A00(RpAtomic *atomic, void *data) {
    AtomicExt *ext = ATOMICEXT(atomic);

    if (ext != NULL && data == ext->world) {
        fn_8046221C(data, atomic);
    }
    return atomic;
}

RpLight *fn_80461A50(RpLight *light, void *data) {
    fn_80462578(data, light);
    return light;
}

RpLight *fn_80461A88(RpLight *light, void *data) {
    LightExt *ext = LIGHTEXT(light);

    if (ext != NULL && data == ext->world) {
        fn_80462618(data, light);
    }
    return light;
}

RwCamera *fn_80461AD8(RwCamera *camera, void *data) {
    fn_80462130(data, camera);
    return camera;
}

RwCamera *fn_80461B10(RwCamera *camera, void *data) {
    CameraExt *ext = CAMERAEXT(camera);

    if (ext != NULL && data == ext->world) {
        fn_80462184(data, camera);
    }
    return camera;
}

/* binary mesh stream plugin of geometries */
RwStream *fn_80461B60(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {
    const RpGeometry *geometry = object;

    return fn_8045C56C(geometry->mesh, geometry, stream, &geometry->matList);
}

RwStream *fn_80461B94(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    RpGeometry *geometry = object;

    geometry->mesh = fn_8045C770(stream, geometry, &geometry->matList);
    if (geometry->mesh != NULL) {
        return stream;
    }
    return NULL;
}

RwInt32 fn_80461BF0(const void *object, RwInt32 offset, RwInt32 size) {
    const RpGeometry *geometry = object;

    return fn_8045C9C8(geometry->mesh, geometry);
}

/* native data stream plugin of geometries */
RwStream *fn_80461C18(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {
    return fn_80465138(stream, object);
}

RwStream *fn_80461C3C(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    if (fn_80465180(stream, object) != NULL) {
        return stream;
    }
    return NULL;
}

RwInt32 fn_80461C80(const void *object, RwInt32 offset, RwInt32 size) {
    return fn_8046510C(object);
}

/* native data stream plugin of world sectors */
RwStream *fn_80461CA0(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {
    return fn_80465200(stream, object);
}

RwStream *fn_80461CC4(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    if (fn_80465260(stream, object) != NULL) {
        return stream;
    }
    return NULL;
}

RwInt32 fn_80461D08(const void *object, RwInt32 offset, RwInt32 size) {
    return fn_804651AC(object);
}

/* binary mesh stream plugin of world sectors */
RwStream *fn_80461D28(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {
    const RpWorldSector *sector = object;
    RpWorld *world = fn_8045FEA0((RpWorldSector *)sector);

    return fn_8045C56C(sector->mesh, world, stream, &world->matList);
}

RwStream *fn_80461D7C(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    RpWorldSector *sector = object;
    RpWorld *world = fn_8045FEA0(sector);

    sector->mesh = fn_8045C770(stream, world, &world->matList);
    if (sector->mesh != NULL) {
        return stream;
    }
    return NULL;
}

RwInt32 fn_80461DE8(const void *object, RwInt32 offset, RwInt32 size) {
    const RpWorldSector *sector = object;

    return fn_8045C9C8(sector->mesh, fn_8045FEA0((RpWorldSector *)sector));
}

/* Attaches the world object plugins to the engine and objects: TRUE on success */
RwBool fn_80461E20(void) {
    RwInt32 result =
        fn_80485D78(sizeof(WorldObjGlobals), rwID_WORLDOBJMODULE, fn_80460974, fn_80460A88);

    lbl_805F1508 =
        fn_804859B8(sizeof(CameraExt), rwID_WORLDOBJMODULE, fn_80460C48, fn_80460D00, fn_80460CA4);
    result |= lbl_805F1508;
    lbl_805F150C =
        fn_80458B6C(sizeof(AtomicExt), rwID_WORLDOBJMODULE, fn_8046112C, fn_80461168, fn_80461164);
    result |= lbl_805F150C;
    lbl_805F1510 =
        fn_80458BAC(sizeof(ClumpExt), rwID_WORLDOBJMODULE, fn_80461238, fn_804612A4, fn_80461260);
    result |= lbl_805F1510;
    lbl_805F1514 =
        fn_8045AAA0(sizeof(LightExt), rwID_WORLDOBJMODULE, fn_80461548, fn_804615B4, fn_80461570);
    result |= lbl_805F1514;
    result |= fn_8045994C(0, rwID_BINMESHPLUGIN, NULL, NULL, NULL);
    result |= fn_8045F2F8(0, rwID_BINMESHPLUGIN, NULL, NULL, NULL);
    result |= fn_8045998C(rwID_BINMESHPLUGIN, fn_80461B94, fn_80461B60, fn_80461BF0);
    result |= fn_8045F338(rwID_BINMESHPLUGIN, fn_80461D7C, fn_80461D28, fn_80461DE8);
    result |= fn_8045994C(0, rwID_NATIVEDATAPLUGIN, NULL, NULL, NULL);
    result |= fn_8045F2F8(0, rwID_NATIVEDATAPLUGIN, NULL, NULL, NULL);
    result |= fn_8045998C(rwID_NATIVEDATAPLUGIN, fn_80461C3C, fn_80461C18, fn_80461C80);
    result |= fn_8045F338(rwID_NATIVEDATAPLUGIN, fn_80461CC4, fn_80461CA0, fn_80461D08);
    result |= fn_80458B6C(0, rwID_RIGHTTORENDER, NULL, NULL, NULL);
    result |= fn_80458BEC(rwID_RIGHTTORENDER, fn_80455D4C, fn_80455DC8, fn_80455E44);
    result |= fn_80460790(0, rwID_RIGHTTORENDER, NULL, NULL, NULL);
    result |= fn_804607D0(rwID_RIGHTTORENDER, fn_80454AA4, fn_80454B20, fn_80454B9C);
    result |= fn_8045F2F8(0, rwID_RIGHTTORENDER, NULL, NULL, NULL);
    result |= fn_8045F338(rwID_RIGHTTORENDER, fn_80454BC4, fn_80454C40, fn_80454CBC);
    result |= fn_8045B2CC(0, rwID_RIGHTTORENDER, NULL, NULL, NULL);
    result |= fn_8045B30C(rwID_RIGHTTORENDER, fn_8045AF28, fn_8045AFA4, fn_8045B020);
    if (result < 0) {
        return FALSE;
    }
    return fn_80462884() != FALSE;
}

/* Adds a camera to a world */
RpWorld *fn_80462130(RpWorld *world, RwCamera *camera) {
    RwFrame *frame = camera->object.parent;
    CameraExt *ext = CAMERAEXT(camera);

    if (frame != NULL) {
        fn_8048722C(frame);
    }
    ext->world = world;
    return world;
}

/* Removes a camera from its world */
RpWorld *fn_80462184(RpWorld *world, RwCamera *camera) {
    CameraExt *ext = CAMERAEXT(camera);

    if (ext->world != NULL) {
        ext->world = NULL;
        ext->numFrustumSectors = 0;
        return world;
    }
    return NULL;
}

/* The world a camera is in */
RpWorld *fn_804621B0(const RwCamera *camera) {
    return CAMERAEXT(camera)->world;
}

/* Adds an atomic to a world */
RpWorld *fn_804621C0(RpWorld *world, RpAtomic *atomic) {
    RwFrame *frame = atomic->object.parent;
    AtomicExt *ext = ATOMICEXT(atomic);

    if (frame != NULL) {
        fn_8048722C(frame);
    }
    ext->world = world;
    return world;
}

/* Removes an atomic from its world */
RpWorld *fn_8046221C(RpWorld *world, RpAtomic *atomic) {
    AtomicExt *ext = ATOMICEXT(atomic);

    if (atomic->repEntry != NULL) {
        fn_8048071C(atomic->repEntry);
    }
    AtomicUntie(atomic);
    ext->world = NULL;
    return world;
}

/* The world an atomic is in */
RpWorld *fn_804622F0(RpAtomic *atomic) {
    return ATOMICEXT(atomic)->world;
}

/* Adds a clump and its atomics, lights and cameras to a world */
RpWorld *fn_804622FC(RpWorld *world, RpClump *clump) {
    ClumpExt *ext = CLUMPEXT(clump);
    RwFrame *frame = clump->object.parent;

    rwLinkListAddLLLink(&world->clumpList, &clump->inWorldLink);
    world->numClumpsInWorld++;
    ext->world = world;
    fn_80456C54(clump, fn_804619C8, world);
    fn_80456D54(clump, fn_80461A50, world);
    fn_80456CD0(clump, fn_80461AD8, world);
    if (frame != NULL) {
        fn_8047E990(&frame->modelling, NULL);
        fn_8048722C(frame);
    }
    ext->field_0x4 = WORLDOBJGLOBAL(field_0x8);
    return world;
}

/* Removes a clump and its atomics, lights and cameras from its world */
RpWorld *fn_804623EC(RpWorld *world, RpClump *clump) {
    ClumpExt *ext = CLUMPEXT(clump);

    if (ext->world != NULL) {
        ext->world->numClumpsInWorld--;
        if (&clump->inWorldLink == ext->world->currentClumpLink) {
            ext->world->currentClumpLink = clump->inWorldLink.next;
        }
        rwLinkListRemoveLLLink(&clump->inWorldLink);
        fn_80456C54(clump, fn_80461A00, world);
        fn_80456D54(clump, fn_80461A88, world);
        fn_80456CD0(clump, fn_80461B10, world);
        ext->world = NULL;
        return world;
    }
    RWERROR((E_RP_WORLD_CLUMPNOTINWORLD));
    return NULL;
}

/* Calls back for each world sector in a camera's view frustum until the callback returns NULL */
RwCamera *fn_804624E8(RwCamera *camera, RpWorldSectorCallBack callBack, void *data) {
    CameraExt *ext = CAMERAEXT(camera);
    RpWorldSector **sectors = ext->frustumSectors;
    RwInt32 numSectors = ext->numFrustumSectors;

    while (numSectors != 0) {
        if (callBack(*sectors, data) == NULL) {
            return camera;
        }
        sectors++;
        numSectors--;
    }
    return camera;
}

/* The world a clump is in */
RpWorld *fn_8046256C(const RpClump *clump) {
    return CLUMPEXT(clump)->world;
}

/* Adds a light to a world: directional lights to one list, positioned ones to the other */
RpWorld *fn_80462578(RpWorld *world, RpLight *light) {
    LIGHTEXT(light)->world = world;
    if (light->object.subType < rpLIGHTPOSITIONINGSTART) {
        rwLinkListAddLLLink(&world->directionalLightList, &light->inWorld);
    } else {
        if (light->object.parent != NULL) {
            fn_8048722C(light->object.parent);
        }
        rwLinkListAddLLLink(&world->lightList, &light->inWorld);
    }
    return world;
}

/* Removes a light from its world */
RpWorld *fn_80462618(RpWorld *world, RpLight *light) {
    LIGHTEXT(light)->world = NULL;
    LightUntie(light);
    rwLinkListRemoveLLLink(&light->inWorld);
    return world;
}

/* The world a light is in */
RpWorld *fn_804626EC(const RpLight *light) {
    return LIGHTEXT(light)->world;
}
