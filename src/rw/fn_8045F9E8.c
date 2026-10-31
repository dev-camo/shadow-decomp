/* RenderWare worlds: the world module, its list of worlds, the default sector render callback,
 * sector bounding boxes, building and destroying sector meshes, creating and destroying worlds,
 * iteration, plugin registration and attaching the world plugins. The original file name is
 * unknown. */
#include "rw/rpworld.h"
#include <stddef.h>
#include <string.h>
#include "layout_check.h"

/* An entry in the world module's list of worlds: a world and the size of its allocation */
typedef struct WorldListEntry {
    /* 0x0 */ RpWorld *world;
    /* 0x4 */ RwUInt32 size;
    /* 0x8 */ RwLLLink link;
} WorldListEntry;
CHECK_SIZEOF(WorldListEntry, 0x10);

/* The world module's globals in the engine (0xC bytes) */
typedef struct WorldGlobals {
    /* 0x0 */ RwFreeList *worldListFreeList;
    /* 0x4 */ RwLinkList worldList;
} WorldGlobals;
CHECK_SIZEOF(WorldGlobals, 0xC);

/* The world list entry of a link */
#define WORLDLISTENTRY(link)                                                                       \
    ((WorldListEntry *)((RwUInt8 *)(link) - offsetof(WorldListEntry, link)))

#define WORLDGLOBAL(var)                                                                           \
    (RWPLUGINOFFSET(WorldGlobals, lbl_805F15B4, lbl_805F1500.globalsOffset)->var)

RwPluginRegistry lbl_8056D898 = {sizeof(RpWorld), sizeof(RpWorld), 0, 0, NULL, NULL};

RwFreeList lbl_805E1618; /* the world list free list */

RwModuleInfo lbl_805F1500;

/* engine plugin destructor */
void *fn_8045F9E8(void *instance, RwInt32 offset, RwInt32 size) {
    if (WORLDGLOBAL(worldListFreeList) != NULL) {
        fn_8047FCDC(WORLDGLOBAL(worldListFreeList));
        WORLDGLOBAL(worldListFreeList) = NULL;
    }
    fn_80462770();
    lbl_805F1500.numInstances--;
    return instance;
}

/* engine plugin constructor: the pipelines, the world list and its free list */
void *fn_8045FA50(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F1500.globalsOffset = offset;
    if (!fn_80462798()) {
        return NULL;
    }
    WORLDGLOBAL(worldListFreeList) = fn_8047FC04(sizeof(WorldListEntry), 8, 4, 1, &lbl_805E1618,
                                                 rwMEMHINTDUR_GLOBAL | rwID_WORLD);
    if (WORLDGLOBAL(worldListFreeList) == NULL) {
        return NULL;
    }
    rwLinkListInitialize(&WORLDGLOBAL(worldList));
    lbl_805F1500.numInstances++;
    return instance;
}

/* The default sector render callback: runs the sector's pipeline, or the world's, or the default
 * one */
RpWorldSector *fn_8045FB10(RpWorldSector *sector) {
    RxPipeline *pipeline;

    if (sector->numTriangles == 0) {
        return sector;
    }
    if (sector->pipeline != NULL) {
        pipeline = sector->pipeline;
    } else if (((RpWorld *)lbl_805F15B4->curWorld)->pipeline != NULL) {
        pipeline = ((RpWorld *)lbl_805F15B4->curWorld)->pipeline;
    } else {
        pipeline = RXPIPELINEGLOBAL(currentWorldSectorPipeline);
    }
    if (fn_8049B674(pipeline, sector, TRUE) != NULL) {
        return sector;
    }
    return NULL;
}

/* Sets each world sector's box from the world's box and the planes above it; returns the world */
RpWorld *fn_8045FBA4(RpWorld *world) {
    RwBBox box = world->boundingBox;
    RwInt32 depth = 0;
    RpSector *sector = world->rootSector;
    RpSector *sectorStack[rpWORLDMAXBSPDEPTH];
    RwBBox boxStack[rpWORLDMAXBSPDEPTH];

    while (depth >= 0) {
        if (sector->type < 0) {
            ((RpWorldSector *)sector)->field_0x48 = box;
            sector = sectorStack[depth];
            box = boxStack[depth];
            depth--;
        } else {
            RpPlaneSector *plane = (RpPlaneSector *)sector;

            depth++;
            sectorStack[depth] = plane->rightSubTree;
            boxStack[depth] = box;
            GETCOORD(boxStack[depth].inf, plane->type) = plane->rightValue;
            GETCOORD(box.sup, plane->type) = plane->leftValue;
            sector = plane->leftSubTree;
        }
    }
    return world;
}

/* Adds a world to the world list with the size of its allocation */
void fn_8045FCFC(RpWorld *world, RwUInt32 size) {
    WorldListEntry *entry =
        RwFreeListAlloc(WORLDGLOBAL(worldListFreeList), rwMEMHINTDUR_GLOBAL | rwID_WORLDMODULE);

    if (entry != NULL) {
        entry->world = world;
        entry->size = size;
        rwLinkListAddLLLink(&WORLDGLOBAL(worldList), &entry->link);
    }
}

/* Builds the mesh of each world sector that has none */
RpWorld *fn_8045FDB0(RpWorld *world) {
    RpSector *sectorStack[rpWORLDMAXBSPDEPTH];
    RpSector *sector = world->rootSector;
    RwInt32 depth = 0;

    if (sector == NULL) {
        return NULL;
    }
    do {
        if (sector->type < 0) {
            RpWorldSector *worldSector = (RpWorldSector *)sector;
            RpMaterial **materials = world->matList.materials + worldSector->matListWindowBase;

            if (worldSector->mesh == NULL) {
                RpBuildMesh *buildMesh = fn_8045C144(worldSector->numTriangles);

                if (buildMesh != NULL) {
                    world = fn_8045F458(world, buildMesh, worldSector, materials);
                    if (world == NULL) {
                        return NULL;
                    }
                } else {
                    return NULL;
                }
            }
            sector = sectorStack[depth--];
        } else {
            RpPlaneSector *plane = (RpPlaneSector *)sector;

            depth++;
            sector = plane->leftSubTree;
            sectorStack[depth] = plane->rightSubTree;
        }
    } while (depth >= 0);
    return world;
}

/* The world that a world sector belongs to, or NULL */
RpWorld *fn_8045FEA0(RpWorldSector *sector) {
    RwLLLink *end = &WORLDGLOBAL(worldList).link;
    RwLLLink *link = end->next;

    while (link != end) {
        WorldListEntry *entry = WORLDLISTENTRY(link);
        RpWorld *world = entry->world;

        if (world->object.privateFlags & rpWORLDSINGLEMALLOC) {
            /* the sectors are part of the world's allocation */
            if ((RwUInt8 *)sector >= (RwUInt8 *)world &&
                (RwUInt8 *)sector < (RwUInt8 *)world + entry->size) {
                return world;
            }
        } else {
            RpWorldSectorSearch search;

            search.sector = sector;
            search.found = FALSE;
            fn_804606F0(world, fn_8045F374, &search);
            if (search.found) {
                return entry->world;
            }
        }
        link = link->next;
    }
    return NULL;
}

/* Renders the world sectors in the current camera's view frustum */
RpWorld *fn_8045FF5C(RpWorld *world) {
    /* the sector render function ignores the callback data */
    fn_804624E8(lbl_805F15B4->curCamera, (RpWorldSectorCallBack)fn_8045F390, world);
    return world;
}

/* Destroys a world: its list entry, sector meshes, materials, sectors and plugin data. World
 * creation inlines it on failure (inferred). */
static inline RwBool WorldDestroy(RpWorld *world) {
    RwLLLink *end = &WORLDGLOBAL(worldList).link;
    RwLLLink *link = end->next;
    RpSector *sectorStack[rpWORLDMAXBSPDEPTH];
    RpSector *sector;
    RwInt32 depth;

    while (link != end) {
        WorldListEntry *entry = WORLDLISTENTRY(link);

        if (entry->world == world) {
            rwLinkListRemoveLLLink(&entry->link);
            RwFreeListFree(WORLDGLOBAL(worldListFreeList), entry);
            break;
        }
        link = link->next;
    }

    sector = world->rootSector;
    depth = 0;
    if (sector != NULL) {
        do {
            if (sector->type < 0) {
                RpWorldSector *worldSector = (RpWorldSector *)sector;

                if (worldSector->mesh != NULL) {
                    fn_8045C2D4(worldSector->mesh);
                    worldSector->mesh = NULL;
                }
                sector = sectorStack[depth--];
            } else {
                RpPlaneSector *plane = (RpPlaneSector *)sector;

                depth++;
                sectorStack[depth] = plane->rightSubTree;
                sector = plane->leftSubTree;
            }
        } while (depth >= 0);
    }

    fn_8045B6F0(&world->matList);
    if (world->object.privateFlags & rpWORLDSINGLEMALLOC) {
        if (world->rootSector != NULL) {
            fn_8045F6E0(world->rootSector);
        }
        fn_80482144(&lbl_8056D898, world);
        RwFree(world);
    } else {
        if (world->rootSector != NULL) {
            fn_8045F7D0(world->rootSector);
        }
        fn_80482144(&lbl_8056D898, world);
        RwFree(world);
    }
    return TRUE;
}

RwBool fn_8045FFA0(RpWorld *world) {
    return WorldDestroy(world);
}

/* Sets the world sector render callback, or the default one for NULL */
RpWorld *fn_80460120(RpWorld *world, RpWorldSectorCallBackRender callBack) {
    if (callBack == NULL) {
        callBack = fn_8045FB10;
    }
    world->renderCallBack = callBack;
    return world;
}

/* Creates an empty world with one world sector filling its box */
RpWorld *fn_80460138(RwBBox *boundingBox) {
    RpWorld *world = RwMalloc(lbl_8056D898.sizeOfStruct, rwMEMHINTDUR_EVENT | rwID_WORLD);
    RpWorldSector *sector;

    if (world == NULL) {
        RPWORLDERROR((E_RW_NOMEM, lbl_8056D898.sizeOfStruct));
        return NULL;
    }
    world->object.type = rpWORLD;
    world->object.subType = 0;
    world->object.flags = 0;
    world->object.privateFlags = 0;
    world->object.parent = NULL;
    fn_8045B788(&world->matList);
    world->renderOrder = rpWORLDRENDERBACK2FRONT;
    world->flags = 0;

    sector = RwMalloc(lbl_8056D880.sizeOfStruct, rwMEMHINTDUR_EVENT | rwID_WORLD);
    if (sector == NULL) {
        /* the original reports the size of a pointer */
        RPWORLDERROR((E_RW_NOMEM, sizeof(RpWorldSector *)));
        RwFree(world);
        return NULL;
    }
    sector->type = rpWORLDSECTORTYPE;
    sector->repEntry = NULL;
    sector->mesh = NULL;
    rwLinkListInitialize(&sector->collAtomicsInWorldSector);
    rwLinkListInitialize(&sector->lightsInWorldSector);
    sector->numVertices = 0;
    sector->numTriangles = 0;
    sector->vertices = NULL;
    sector->triangles = NULL;
    sector->normals = NULL;
    memset(sector->texCoords, 0, sizeof(sector->texCoords));
    sector->preLitLum = NULL;
    sector->field_0x48.inf = boundingBox->inf;
    sector->field_0x48.sup = boundingBox->sup;
    sector->boundingBox.inf = boundingBox->inf;
    sector->boundingBox.sup = boundingBox->sup;
    sector->pipeline = NULL;

    world->rootSector = (RpSector *)sector;
    world->numTexCoordSets = 0;
    world->worldOrigin.x = world->worldOrigin.y = world->worldOrigin.z = 0.0f;
    world->boundingBox.inf = boundingBox->inf;
    world->boundingBox.sup = boundingBox->sup;
    rwLinkListInitialize(&world->clumpList);
    world->numClumpsInWorld = 0;
    world->currentClumpLink = &world->clumpList.link;
    rwLinkListInitialize(&world->lightList);
    rwLinkListInitialize(&world->directionalLightList);
    world->renderCallBack = fn_8045FB10;
    world->pipeline = NULL;

    fn_8045FCFC(world, lbl_8056D898.sizeOfStruct);
    fn_8048209C(&lbl_8056D898, world);
    fn_8048209C(&lbl_8056D880, sector);
    if (fn_8045FDB0(world) == NULL) {
        WorldDestroy(world);
        return NULL;
    }
    return world;
}

/* Calls back for each material of a world until the callback returns NULL */
RpWorld *fn_8046066C(RpWorld *world, RpMaterialCallBack callBack, void *data) {
    RwInt32 numMaterials = world->matList.numMaterials;
    RwInt32 i;

    for (i = 0; i < numMaterials; i++) {
        if (callBack(world->matList.materials[i], data) == NULL) {
            return world;
        }
    }
    return world;
}

/* Calls back for each world sector until the callback returns NULL */
RpWorld *fn_804606F0(RpWorld *world, RpWorldSectorCallBack callBack, void *data) {
    RpSector *sectorStack[rpWORLDMAXBSPDEPTH];
    RpSector *sector = world->rootSector;
    RwInt32 depth = 0;

    do {
        if (sector->type < 0) {
            if (callBack((RpWorldSector *)sector, data) == NULL) {
                return world;
            }
            sector = sectorStack[depth--];
        } else {
            RpPlaneSector *plane = (RpPlaneSector *)sector;

            depth++;
            sectorStack[depth] = plane->rightSubTree;
            sector = plane->leftSubTree;
        }
    } while (depth >= 0);
    return world;
}

/* plugin registration with worlds */
RwInt32 fn_80460790(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor, RwPluginObjectCopy copy) {
    return fn_80481DC8(&lbl_8056D898, size, pluginID, constructor, destructor, copy);
}

RwInt32 fn_804607D0(RwUInt32 pluginID, RwPluginDataChunkReadCallBack read,
                    RwPluginDataChunkWriteCallBack write,
                    RwPluginDataChunkGetSizeCallBack getSize) {
    return fn_804815F0(&lbl_8056D898, pluginID, read, write, getSize);
}

/* Attaches the world plugins to the engine: TRUE on success */
RwBool fn_8046080C(void) {
    RwInt32 result =
        fn_80485D78(sizeof(RwFreeList *), rwID_MATERIALMODULE, fn_8045B048, fn_8045B0D8);

    result |= fn_80485D78(sizeof(RpMeshGlobals), rwID_MESHMODULE, fn_8045BFBC, fn_8045BF64);
    result |= fn_80485D78(sizeof(void *), rwID_GEOMETRYMODULE, fn_80458CF8, fn_80458D10);
    result |= fn_80485D78(8, rwID_CLUMPMODULE, fn_80456A50, fn_80456994);
    result |= fn_80485D78(sizeof(void *), rwID_LIGHTMODULE, fn_8045AE98, fn_8045AE20);
    result |= fn_80485D78(0, rwID_SECTORMODULE, fn_8045F2A0, fn_8045F2B4);
    result |= fn_80485D78(sizeof(WorldGlobals), rwID_WORLDMODULE, fn_8045FA50, fn_8045F9E8);
    result |= fn_80485D78(0, rwID_BINWORLDMODULE, fn_80455CF8, fn_80455CE4);
    if (result < 0) {
        return FALSE;
    }
    if (!fn_80461E20()) {
        return FALSE;
    }
    if (!fn_80456B48()) {
        return FALSE;
    }
    if (fn_8046318C()) {
        return TRUE;
    }
    return FALSE;
}
