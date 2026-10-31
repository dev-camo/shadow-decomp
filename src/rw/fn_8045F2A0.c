/* RenderWare world sectors: the sector module, rendering a sector and its atomics, sector plugin
 * registration, building a sector's mesh, and destroying a sector tree. The original file name is
 * unknown. */
#include "rw/rpworld.h"
#include "layout_check.h"

/* a sector type that is freed without being deinitialised (inferred) */
#define rpSECTORTYPEEMPTY (-2)

RwPluginRegistry lbl_8056D880 = {sizeof(RpWorldSector), sizeof(RpWorldSector), 0, 0, NULL, NULL};

RwModuleInfo lbl_805F14F8;

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

/* A world sector callback that stops at the sector searched for */
RpWorldSector *fn_8045F374(RpWorldSector *sector, void *data) {
    RpWorldSectorSearch *search = data;

    if (search->sector == sector) {
        search->found = TRUE;
        return NULL;
    }
    return sector;
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
    RwUInt16 pipelineIndex;
    RwUInt16 rasterIndex;
    RwUInt16 textureIndex;
    RxPipeline *pipeline;
    RwTexture *texture;
    RpMaterial *material;
    RpTriangle *triangle;
    RwInt32 i;
    RwTexture **textures;
    RwRaster **rasters;
    RxPipeline **pipelines;
    RwUInt32 size;
    RwUInt32 numMaterials = world->matList.numMaterials;
    RpMeshHeader *mesh;
    RwUInt16 numTextures = 0;
    RwUInt16 numRasters = 0;
    RwUInt16 numPipelines = 0;
    RwRaster *raster;

    if (numMaterials != 0) {
        size = numMaterials * sizeof(void *);
        textures = RwMalloc(size, rwMEMHINTDUR_FUNCTION | rwID_TEXTURE);
        rasters = RwMalloc(size, rwMEMHINTDUR_FUNCTION | rwID_WORLDMODULE);
        pipelines = RwMalloc(size, rwMEMHINTDUR_FUNCTION | rwID_WORLDMODULE);

        for (i = 0; i < sector->numTriangles; i++) {
            triangle = &sector->triangles[i];
            material = materials[triangle->matIndex];
            texture = material->texture;
            raster = NULL;

            for (textureIndex = 0; textureIndex < numTextures; textureIndex++) {
                if (textures[textureIndex] == texture) {
                    break;
                }
            }
            if (textureIndex == numTextures) {
                textures[textureIndex] = texture;
                numTextures++;
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
                rasters[rasterIndex] = raster;
                numRasters++;
            }

            pipeline = material->pipeline;
            for (pipelineIndex = 0; pipelineIndex < numPipelines; pipelineIndex++) {
                if (pipelines[pipelineIndex] == pipeline) {
                    break;
                }
            }
            if (pipelineIndex == numPipelines) {
                pipelines[pipelineIndex] = pipeline;
                numPipelines++;
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

/* Destroys a sector tree whose sectors are part of the world's allocation.
 *
 * Matching workaround: the sector release is written out here and in fn_8045F7D0 instead of
 * sharing a macro or an inline, which compiles fn_8045F7D0 differently (measured). Revisit when
 * the original's release helper is found. */
void fn_8045F6E0(RpSector *sector) {
    RpWorldSector *worldSector;
    RwLLLink *link;
    RwLLLink *current;
    RwLLLink *end;
    RpPlaneSector *planeSector;

    switch (sector->type) {
    case rpWORLDSECTORTYPE:
        worldSector = (RpWorldSector *)sector;

        /* releases the sector's instance data, ties and plugin data */
        if (worldSector->repEntry != NULL) {
            fn_8048071C(worldSector->repEntry);
        }
        link = worldSector->collAtomicsInWorldSector.link.next;
        end = &worldSector->collAtomicsInWorldSector.link;
        while (link != end) {
            current = link;
            link = link->next;
            fn_80460DF0((RpTie *)current);
        }
        link = worldSector->lightsInWorldSector.link.next;
        end = &worldSector->lightsInWorldSector.link;
        while (link != end) {
            current = link;
            link = link->next;
            fn_80460D80((RpLightTie *)current);
        }
        fn_80482144(&lbl_8056D880, worldSector);
        if (worldSector->mesh != NULL) {
            fn_8045C2D4(worldSector->mesh);
            worldSector->mesh = NULL;
        }
        break;
    case rpSECTORTYPEEMPTY:
        break;
    default:
        planeSector = (RpPlaneSector *)sector;

        if (planeSector->leftSubTree != NULL) {
            fn_8045F6E0(planeSector->leftSubTree);
        }
        if (planeSector->rightSubTree != NULL) {
            fn_8045F6E0(planeSector->rightSubTree);
        }
        break;
    }
}

/* Destroys a sector tree whose sectors and their data are separate allocations */
void fn_8045F7D0(RpSector *sector) {
    RpWorldSector *worldSector;
    RwLLLink *link;
    RwLLLink *current;
    RwLLLink *end;
    RwInt32 i;
    RpPlaneSector *planeSector;

    switch (sector->type) {
    case rpWORLDSECTORTYPE:
        worldSector = (RpWorldSector *)sector;

        /* releases the sector's instance data, ties and plugin data */
        if (worldSector->repEntry != NULL) {
            fn_8048071C(worldSector->repEntry);
        }
        link = worldSector->collAtomicsInWorldSector.link.next;
        end = &worldSector->collAtomicsInWorldSector.link;
        while (link != end) {
            current = link;
            link = link->next;
            fn_80460DF0((RpTie *)current);
        }
        link = worldSector->lightsInWorldSector.link.next;
        end = &worldSector->lightsInWorldSector.link;
        while (link != end) {
            current = link;
            link = link->next;
            fn_80460D80((RpLightTie *)current);
        }
        fn_80482144(&lbl_8056D880, worldSector);
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
    case rpSECTORTYPEEMPTY:
        RwFree(sector);
        break;
    default:
        planeSector = (RpPlaneSector *)sector;

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
