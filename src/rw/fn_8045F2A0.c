/* RenderWare world sectors: the sector module, rendering a sector and its atomics, sector plugin
 * registration, building a sector's mesh, and destroying a sector tree. The original file name is
 * unknown. */
#include "rw/rpworld.h"

#define rwMEMHINTDUR_FUNCTION 0x10000

/* chunk IDs */
#define rwID_TEXTURE 0x6
#define rwID_WORLDMODULE 0x507 /* (inferred) */

/* a sector type that is freed without being deinitialised (inferred) */
#define rpSECTORTYPEEMPTY -2

/* The world sector module (inferred) */
typedef struct WorldSectorModule {
    /* 0x0 */ RwInt32 globalsOffset; /* engine plugin offset of the globals */
    /* 0x4 */ RwInt32 numInstances;  /* the open engine instances */
} WorldSectorModule;

/* A search of a list for one object (inferred) */
typedef struct ObjectSearch {
    /* 0x0 */ void *object;
    /* 0x4 */ RwBool found;
} ObjectSearch;

RwPluginRegistry lbl_8056D880 = {sizeof(RpWorldSector), sizeof(RpWorldSector), 0, 0, NULL, NULL};

WorldSectorModule lbl_805F14F8;

/* engine plugin constructor */
void *fn_8045F2A0(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F14F8.numInstances++;
    return instance;
}

/* engine plugin destructor */
void *fn_8045F2B4(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F14F8.numInstances--;
    return instance;
}

/* Renders a world sector with the current world's sector render callback */
RpWorldSector *fn_8045F2C8(RpWorldSector *sector) {
    RpWorld *world = lbl_805F15B4->curWorld;

    return world->renderCallBack(sector);
}

/* plugin registration with world sectors */
RwInt32 fn_8045F2F8(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor, RwPluginObjectCopy copy) {
    return fn_80481DC8(&lbl_8056D880, size, pluginID, constructor, destructor, copy);
}

RwInt32 fn_8045F338(RwUInt32 pluginID, RwPluginDataChunkReadCallBack read,
                    RwPluginDataChunkWriteCallBack write,
                    RwPluginDataChunkGetSizeCallBack getSize) {
    return fn_804815F0(&lbl_8056D880, pluginID, read, write, getSize);
}

/* A search callback: stops at the object searched for */
void *fn_8045F374(void *object, void *data) {
    ObjectSearch *search = data;

    if (search->object == object) {
        search->found = TRUE;
        return NULL;
    }
    return object;
}

/* Renders a world sector, then each of its atomics that is visible and not yet rendered this
 * frame */
RpWorldSector *fn_8045F390(RpWorldSector *sector) {
    if (fn_8045F2C8(sector) != NULL) {
        RpAtomic *atomic;
        RwLLLink *end = &sector->collAtomicsInWorldSector.link;
        RwLLLink *link = sector->collAtomicsInWorldSector.link.next;

        while (link != end) {
            atomic = ((RpTie *)link)->apAtom;

            if ((atomic->object.flags & rpATOMICRENDER) &&
                atomic->renderFrame != lbl_805F15B4->renderFrame) {
                if (fn_804856C8(lbl_805F15B4->curCamera, fn_804566D0(atomic))) {
                    atomic->renderCallBack(atomic);
                }
                atomic->renderFrame = lbl_805F15B4->renderFrame;
            }
            link = link->next;
        }
    }
    return sector;
}

/* Builds a world sector's mesh from its triangles, numbering the textures, rasters and
 * pipelines of their materials */
RpWorld *fn_8045F458(RpWorld *world, RpBuildMesh *buildMesh, RpWorldSector *sector,
                     RpMaterial **materials) {
    RwUInt16 numTextures = 0;
    RwUInt16 numRasters = 0;
    RwUInt16 numPipelines = 0;
    RpMeshHeader *mesh;
    RwUInt32 numMaterials = world->matList.numMaterials;

    if (numMaterials != 0) {
        RwUInt32 size = numMaterials * sizeof(void *);
        RwTexture **textures = RwMalloc(size, rwMEMHINTDUR_FUNCTION | rwID_TEXTURE);
        RwRaster **rasters = RwMalloc(size, rwMEMHINTDUR_FUNCTION | rwID_WORLDMODULE);
        RxPipeline **pipelines = RwMalloc(size, rwMEMHINTDUR_FUNCTION | rwID_WORLDMODULE);
        RwInt32 i;

        for (i = 0; i < sector->numTriangles; i++) {
            RpTriangle *triangle = &sector->triangles[i];
            RpMaterial *material = materials[triangle->matIndex];
            RwTexture *texture = material->texture;
            RwRaster *raster = NULL;
            RxPipeline *pipeline;
            RwUInt16 textureIndex;
            RwUInt16 rasterIndex;
            RwUInt16 pipelineIndex;

            for (textureIndex = 0; textureIndex < numTextures; textureIndex++) {
                if (textures[textureIndex] == texture) {
                    break;
                }
            }
            if (textureIndex == numTextures) {
                textures[numTextures++] = texture;
            }

            if (texture != NULL) {
                raster = texture->raster;
            }
            for (rasterIndex = 0; rasterIndex < numRasters; rasterIndex++) {
                if (rasters[rasterIndex] == raster) {
                    break;
                }
            }
            if (rasterIndex == numRasters) {
                rasters[numRasters++] = raster;
            }

            pipeline = material->pipeline;
            for (pipelineIndex = 0; pipelineIndex < numPipelines; pipelineIndex++) {
                if (pipelines[pipelineIndex] == pipeline) {
                    break;
                }
            }
            if (pipelineIndex == numPipelines) {
                pipelines[numPipelines++] = pipeline;
            }

            fn_8045C340(buildMesh, material, triangle->vertIndex[0], triangle->vertIndex[1],
                        triangle->vertIndex[2], triangle->matIndex, textureIndex, rasterIndex,
                        pipelineIndex);
        }
        RwFree(textures);
        RwFree(rasters);
        RwFree(pipelines);
    }

    if (world->flags & rpWORLDTRISTRIP) {
        mesh = fn_8045F1FC(buildMesh, rpGEOMETRYTRISTRIP);
    } else {
        mesh = fn_8045F1FC(buildMesh, 0);
    }
    if (mesh != NULL) {
        sector->mesh = mesh;
    } else {
        fn_8045C26C(buildMesh);
        return NULL;
    }
    return world;
}

/* Releases a world sector's instance data, ties and plugin data */
#define WORLDSECTORDEINIT(sector)                                                                  \
    do {                                                                                           \
        RwLLLink *link_;                                                                           \
                                                                                                   \
        if ((sector)->repEntry != NULL) {                                                          \
            fn_8048071C((sector)->repEntry);                                                       \
        }                                                                                          \
        link_ = (sector)->collAtomicsInWorldSector.link.next;                                      \
        while (link_ != &(sector)->collAtomicsInWorldSector.link) {                                \
            RwLLLink *current_ = link_;                                                            \
                                                                                                   \
            link_ = link_->next;                                                                   \
            fn_80460DF0((RpTie *)current_);                                                        \
        }                                                                                          \
        link_ = (sector)->lightsInWorldSector.link.next;                                           \
        while (link_ != &(sector)->lightsInWorldSector.link) {                                     \
            RwLLLink *current_ = link_;                                                            \
                                                                                                   \
            link_ = link_->next;                                                                   \
            fn_80460D80((RpLightTie *)current_);                                                   \
        }                                                                                          \
        fn_80482144(&lbl_8056D880, (sector));                                                      \
    } while (0)

/* Destroys a sector tree whose sectors are part of the world's allocation */
void fn_8045F6E0(RpSector *sector) {
    switch (sector->type) {
    case rpWORLDSECTORTYPE: {
        RpWorldSector *worldSector = (RpWorldSector *)sector;

        WORLDSECTORDEINIT(worldSector);
        if (worldSector->mesh != NULL) {
            fn_8045C2D4(worldSector->mesh);
            worldSector->mesh = NULL;
        }
        break;
    }
    case rpSECTORTYPEEMPTY:
        break;
    default: {
        RpPlaneSector *planeSector = (RpPlaneSector *)sector;

        if (planeSector->leftSubTree != NULL) {
            fn_8045F6E0(planeSector->leftSubTree);
        }
        if (planeSector->rightSubTree != NULL) {
            fn_8045F6E0(planeSector->rightSubTree);
        }
        break;
    }
    }
}

/* Destroys a sector tree whose sectors and their data are separate allocations */
void fn_8045F7D0(RpSector *sector) {
    switch (sector->type) {
    case rpWORLDSECTORTYPE: {
        RpWorldSector *worldSector = (RpWorldSector *)sector;
        RwInt32 i;

        WORLDSECTORDEINIT(worldSector);
        if (worldSector->vertices != NULL) {
            RwFree(worldSector->vertices);
            worldSector->vertices = NULL;
        }
        if (worldSector->normals != NULL) {
            RwFree(worldSector->normals);
            worldSector->normals = NULL;
        }
        if (worldSector->preLitLum != NULL) {
            RwFree(worldSector->preLitLum);
            worldSector->preLitLum = NULL;
        }
        if (worldSector->triangles != NULL) {
            RwFree(worldSector->triangles);
            worldSector->triangles = NULL;
        }
        for (i = 0; i < 8; i++) {
            if (worldSector->texCoords[i] != NULL) {
                RwFree(worldSector->texCoords[i]);
                worldSector->texCoords[i] = NULL;
            }
        }
        if (worldSector->mesh != NULL) {
            fn_8045C2D4(worldSector->mesh);
            worldSector->mesh = NULL;
        }
        RwFree(worldSector);
        break;
    }
    case rpSECTORTYPEEMPTY:
        RwFree(sector);
        break;
    default: {
        RpPlaneSector *planeSector = (RpPlaneSector *)sector;

        if (planeSector->leftSubTree != NULL) {
            fn_8045F7D0(planeSector->leftSubTree);
        }
        planeSector->leftSubTree = NULL;
        if (planeSector->rightSubTree != NULL) {
            fn_8045F7D0(planeSector->rightSubTree);
        }
        planeSector->rightSubTree = NULL;
        RwFree(planeSector);
        break;
    }
    }
}
