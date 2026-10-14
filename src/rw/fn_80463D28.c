/* RenderWare GameCube world sector all-in-one pipeline: creating the pipeline, the default
 * lighting and instancing callbacks of world sectors, and the node that runs the callbacks. The
 * original file name is unknown. */
#include "rw/rpworld.h"

/* Creates a world sector all-in-one pipeline: one node that calls the given callbacks */
RxPipeline *fn_80463D28(RwUInt32 pluginID, RwUInt32 pluginData,
                        RpGameCubeSectorCallBack instanceCallBack,
                        RpGameCubeSectorCallBack reinstanceCallBack,
                        RpGameCubeSectorCallBack lightingCallBack,
                        RpGameCubeSectorCallBack renderCallBack) {
    RxPipeline *pipeline = fn_8049B76C();
    RxPipeline *lockedPipeline;
    RxNodeDefinition *nodeDef;
    RxPipelineNode *node;

    pipeline->pluginId = pluginID;
    pipeline->pluginData = pluginData;
    lockedPipeline = fn_8049CCB8(pipeline);
    nodeDef = fn_804641C0();
    lockedPipeline = fn_8049D0D0(lockedPipeline, NULL, nodeDef, NULL);
    fn_8049C1E0(lockedPipeline);

    node = fn_8049CFCC(pipeline, nodeDef->name, NULL, NULL);
    fn_80462BF0(node, (RxGameCubeAllInOneCallBack)instanceCallBack);
    fn_80462BFC(node, (RxGameCubeAllInOneCallBack)reinstanceCallBack);
    fn_80462C08(node, (RxGameCubeAllInOneCallBack)lightingCallBack);
    fn_80462C14(node, (RxGameCubeAllInOneCallBack)renderCallBack);
    return pipeline;
}

/* Lights a world sector when the world is lit: by the world's directional and ambient lights
 * that light the world, then by each local light in the sector that lights the world. The ambient
 * colour is clamped to 1. */
RpWorldSector *fn_80463DF4(RpWorldSector *sector, RpGameCubeRenderMesh *renderMesh) {
    renderMesh->lightMask = 0;
    renderMesh->ambientColor.red = 0.0f;
    renderMesh->ambientColor.green = 0.0f;
    renderMesh->ambientColor.blue = 0.0f;
    renderMesh->ambientColor.alpha = 1.0f;
    renderMesh->hasAmbient = FALSE;
    renderMesh->numLights = 0;

    if (((RpWorld *)lbl_805F15B4->curWorld)->flags & rpWORLDLIGHT) {
        RwLLLink *link;
        RwLLLink *end;

        fn_804641CC(rpLIGHTLIGHTWORLD, renderMesh);
        lbl_805F15B4->lightFrame++;

        end = &sector->lightsInWorldSector.link;
        link = sector->lightsInWorldSector.link.next;
        while (link != end) {
            RpLight *light = ((RpLightTie *)link)->light;

            if (light != NULL && (light->object.flags & rpLIGHTLIGHTWORLD)) {
                fn_804643E8(light, renderMesh);
            }
            link = link->next;
        }
    }

    if (renderMesh->hasAmbient) {
        if (renderMesh->ambientColor.red > 1.0f) {
            renderMesh->ambientColor.red = 1.0f;
        }
        if (renderMesh->ambientColor.green > 1.0f) {
            renderMesh->ambientColor.green = 1.0f;
        }
        if (renderMesh->ambientColor.blue > 1.0f) {
            renderMesh->ambientColor.blue = 1.0f;
        }
    }
    return sector;
}

/* Creates the instanced data of a world sector, owned by the sector. The sector is passed on as
 * the generic object the all-in-one callbacks receive. */
RpWorldSector *fn_80463F10(RpWorldSector *sector, RpGameCubeRenderMesh *renderMesh) {
    void *object = sector;
    RpWorld *world = lbl_805F15B4->curWorld;

    if (world->flags & rpWORLDNATIVEINSTANCE) {
        if (lbl_805EDDD8 == 1) {
            renderMesh->repEntry =
                fn_8046CB94(world, object, object, &((RpWorldSector *)object)->repEntry);
        } else {
            renderMesh->repEntry =
                fn_8046D4C8(world, object, object, &((RpWorldSector *)object)->repEntry);
        }
    } else {
        renderMesh->repEntry =
            fn_8046D4C8(world, object, object, &((RpWorldSector *)object)->repEntry);
    }
    return sector;
}

/* The world sector all-in-one node body: instances the sector, or reuses its existing instanced
 * data, then lights and renders it */
RwBool fn_80463FA4(RxPipelineNode *self, const RxPipelineNodeParam *params) {
    RpWorldSector *sector = params->dataParam;
    RxGameCubeAllInOneData *nodeData = self->privateData;
    RpWorld *world;
    RpGameCubeRenderMesh renderMesh;

    renderMesh.nodeData = nodeData;
    world = lbl_805F15B4->curWorld;
    if (!(world->flags & rpWORLDNATIVE)) {
        RpMeshHeader *meshHeader;

        if (sector->numVertices == 0) {
            return TRUE;
        }
        meshHeader = sector->mesh;
        if (meshHeader->numMeshes == 0) {
            return TRUE;
        }

        renderMesh.repEntry = sector->repEntry;
        renderMesh.meshHeader = meshHeader;
        renderMesh.geometryFlags = world->flags;

        /* instanced data of an older mesh is destroyed */
        if (renderMesh.repEntry != NULL &&
            ((RpGameCubeInstanceData *)(renderMesh.repEntry + 1))->serialNum !=
                meshHeader->serialNum) {
            fn_8048071C(renderMesh.repEntry);
            renderMesh.repEntry = NULL;
        }

        if (renderMesh.repEntry != NULL) {
            RwResourcesUseResEntry(renderMesh.repEntry);
        } else if (!RXGAMECUBEALLINONECALL(nodeData->instanceCallBack, sector, &renderMesh)) {
            return FALSE;
        }
    } else {
        renderMesh.repEntry = sector->repEntry;
        renderMesh.meshHeader = sector->mesh;
        renderMesh.geometryFlags = world->flags;
    }

    if (!RXGAMECUBEALLINONECALL(nodeData->lightingCallBack, sector, &renderMesh)) {
        return FALSE;
    }
    if (!RXGAMECUBEALLINONECALL(nodeData->renderCallBack, sector, &renderMesh)) {
        return FALSE;
    }
    return TRUE;
}

/* Installs the default callbacks in a new world sector all-in-one node; world sectors are not
 * reinstanced */
RwBool fn_80464188(RxPipelineNode *self) {
    RxGameCubeAllInOneData *nodeData = self->privateData;

    nodeData->instanceCallBack = (RxGameCubeAllInOneCallBack)fn_80463F10;
    nodeData->reinstanceCallBack = NULL;
    nodeData->lightingCallBack = (RxGameCubeAllInOneCallBack)fn_80463DF4;
    nodeData->renderCallBack = fn_80462914;
    return TRUE;
}

/* the node name, misspelt as in the original */
RwChar lbl_8056D910[] = "GamerCubeWorldSectorAllInOne.csl";

RxNodeDefinition lbl_8056D934 = {
    lbl_8056D910, fn_80463FA4, NULL, NULL, fn_80464188,
    NULL,         NULL,        NULL, {0},  sizeof(RxGameCubeAllInOneData),
};

RxNodeDefinition *fn_804641C0(void) {
    return &lbl_8056D934;
}
