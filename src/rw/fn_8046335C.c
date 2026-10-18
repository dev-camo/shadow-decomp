/* RenderWare GameCube atomic all-in-one pipeline: creating the pipeline, the default lighting,
 * instancing and reinstancing callbacks of atomics, and the node that runs the callbacks. The
 * original file name is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>
#include <dolphin/os.h>

/* RpGeometry lock flags */
#define rpGEOMETRYLOCKPRELIGHT 0x8
#define rpGEOMETRYLOCKTEXCOORDS1 0x10

/* RpInterpolator flags */
#define rpINTERPOLATORDIRTY 0x1

/* the instanced data flag of prelit colours with alpha (inferred) */
#define rpGAMECUBEINSTANCEPRELITALPHA 0x1

/* Creates an atomic all-in-one pipeline: one node that calls the given callbacks */
RxPipeline *fn_8046335C(RwUInt32 pluginID, RwUInt32 pluginData,
                        RpGameCubeInstanceCallBack instanceCallBack,
                        RpGameCubeInstanceCallBack reinstanceCallBack,
                        RpGameCubeLightingCallBack lightingCallBack,
                        RpGameCubeRenderCallBack renderCallBack) {
    RxPipeline *pipeline = fn_8049B76C();
    RxPipeline *lockedPipeline;
    RxNodeDefinition *nodeDef;
    RxPipelineNode *node;

    pipeline->pluginId = pluginID;
    pipeline->pluginData = pluginData;
    lockedPipeline = fn_8049CCB8(pipeline);
    nodeDef = fn_80463D1C();
    lockedPipeline = fn_8049D0D0(lockedPipeline, NULL, nodeDef, NULL);
    fn_8049C1E0(lockedPipeline);

    node = fn_8049CFCC(pipeline, nodeDef->name, NULL, NULL);
    fn_80462BF0(node, (RxGameCubeAllInOneCallBack)instanceCallBack);
    fn_80462BFC(node, (RxGameCubeAllInOneCallBack)reinstanceCallBack);
    fn_80462C08(node, (RxGameCubeAllInOneCallBack)lightingCallBack);
    fn_80462C14(node, (RxGameCubeAllInOneCallBack)renderCallBack);
    return pipeline;
}

/* Lights an atomic that is lit in a world: by the world's directional and ambient lights that
 * light atomics, then by each local light that lights atomics in the sectors the atomic is in and
 * whose range reaches the atomic's bounding sphere, once per light. The ambient colour is clamped
 * to 1. */
RpAtomic *fn_80463428(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh) {
    RwUInt32 geometryFlags = atomic->geometry->flags;

    renderMesh->lightMask = 0;
    renderMesh->ambientColor.red = 0.0f;
    renderMesh->ambientColor.green = 0.0f;
    renderMesh->ambientColor.blue = 0.0f;
    renderMesh->ambientColor.alpha = 1.0f;
    renderMesh->hasAmbient = FALSE;
    renderMesh->numLights = 0;

    if ((geometryFlags & rpGEOMETRYLIGHT) && lbl_805F15B4->curWorld != NULL) {
        RwLLLink *tieEnd;
        RwLLLink *tieLink;

        fn_804641CC(rpLIGHTLIGHTATOMICS, renderMesh);
        lbl_805F15B4->lightFrame++;

        tieEnd = &atomic->llWorldSectorsInAtomic.link;
        tieLink = atomic->llWorldSectorsInAtomic.link.next;
        while (tieLink != tieEnd) {
            RpWorldSector *sector = TIEFROMATOMICLINK(tieLink)->worldSector;
            RwLLLink *lightEnd = &sector->lightsInWorldSector.link;
            RwLLLink *lightLink = sector->lightsInWorldSector.link.next;

            while (lightLink != lightEnd) {
                RpLight *light = ((RpLightTie *)lightLink)->light;

                if (light != NULL && light->lightFrame != lbl_805F15B4->lightFrame &&
                    (light->object.flags & rpLIGHTLIGHTATOMICS)) {
                    RwMatrix *ltm;
                    RwSphere *sphere;
                    RwV3d delta;
                    RwReal reach;

                    light->lightFrame = lbl_805F15B4->lightFrame;
                    ltm = RwFrameGetLTM(light->object.parent);
                    sphere = fn_804566D0(atomic);
                    delta.x = sphere->center.x - ltm->pos.x;
                    delta.y = sphere->center.y - ltm->pos.y;
                    delta.z = sphere->center.z - ltm->pos.z;
                    reach = sphere->radius + light->radius;
                    if (delta.x * delta.x + delta.y * delta.y + delta.z * delta.z < reach * reach) {
                        fn_804643E8(((RpLightTie *)lightLink)->light, renderMesh);
                    }
                }
                lightLink = lightLink->next;
            }
            tieLink = tieLink->next;
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
    return atomic;
}

/* Writes the locked vertex streams of a geometry's first morph target into its instanced data, in
 * the order of the vertex descriptors: positions, normals, prelit colours, then texture
 * coordinates. The node data is not used. */
void fn_804635D4(RpGeometry *geometry, RpGameCubeInstanceData *instance,
                 struct RxGameCubeAllInOneData *nodeData) {
    RwUInt16 locked = geometry->lockedSinceLastInst;
    RwUInt32 flags = geometry->flags | rpGEOMETRYPOSITIONS;
    RwInt32 index = 0;
    RwUInt32 numVertices = geometry->numVertices;
    RpGameCubeVertexFormat *format = GAMECUBEVERTEXFORMAT(geometry);

    if (format == NULL) {
        format = fn_80462E1C();
    }

    if (flags & rpGEOMETRYPOSITIONS) {
        if (locked & rpGEOMETRYLOCKVERTICES) {
            RwUInt32 size =
                fn_80466B58(instance->vertexDescriptors[0].data, geometry->morphTarget->verts,
                            format->posType, geometry->numVertices,
                            instance->vertexDescriptors[0].stride, NULL, 1 << format->posFrac);

            DCFlushRange(instance->vertexDescriptors[0].data, size);
        }
        index = 1;
    }
    if (flags & rpGEOMETRYNORMALS) {
        if (locked & rpGEOMETRYLOCKNORMALS) {
            DCFlushRange(instance->vertexDescriptors[index].data,
                         fn_80467BC8(instance->vertexDescriptors[index].data,
                                     geometry->morphTarget->normals, format->normalType,
                                     geometry->numVertices,
                                     instance->vertexDescriptors[index].stride));
        }
        index++;
    }
    if (flags & rpGEOMETRYPRELIT) {
        if (locked & rpGEOMETRYLOCKPRELIGHT) {
            /* without a vertex format, the colours have alpha if any vertex is translucent */
            if (GAMECUBEVERTEXFORMAT(geometry) == NULL) {
                RwUInt32 i;

                instance->flags &= ~rpGAMECUBEINSTANCEPRELITALPHA;
                for (i = 0; i < numVertices; i++) {
                    if (geometry->preLitLum[i].alpha < 0xFF) {
                        instance->flags |= rpGAMECUBEINSTANCEPRELITALPHA;
                        break;
                    }
                }
            } else if (format->colorType > GX_RGBX8) {
                instance->flags |= rpGAMECUBEINSTANCEPRELITALPHA;
            } else {
                instance->flags &= ~rpGAMECUBEINSTANCEPRELITALPHA;
            }

            DCFlushRange(instance->vertexDescriptors[index].data,
                         fn_804699F0(instance->vertexDescriptors[index].data, geometry->preLitLum,
                                     format->colorType, geometry->numVertices,
                                     instance->vertexDescriptors[index].stride));
        }
        index++;
    }
    if (flags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
        RwInt32 i;

        for (i = 0; i < geometry->numTexCoordSets; i++) {
            if ((rpGEOMETRYLOCKTEXCOORDS1 << i) & locked) {
                DCFlushRange(instance->vertexDescriptors[index].data,
                             fn_8046A270(instance->vertexDescriptors[index].data,
                                         geometry->texCoords[i], format->texCoordType[i],
                                         geometry->numVertices,
                                         instance->vertexDescriptors[index].stride,
                                         1 << format->texCoordFrac[i]));
            }
            index++;
        }
    }
    GXInvalidateVtxCache();
}

/* Rewrites the instanced data of an atomic whose geometry was locked, and of an atomic with
 * several morph targets whose interpolation changed, once the GPU is done with it */
RpAtomic *fn_80463840(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh) {
    RpGeometry *geometry = atomic->geometry;
    RpGameCubeInstanceData *instance = (RpGameCubeInstanceData *)(renderMesh->repEntry + 1);

    if (geometry->lockedSinceLastInst != 0) {
        RPGAMECUBEWAITDRAWSYNC(instance->syncToken);
        fn_804635D4(geometry, instance, renderMesh->nodeData);
        geometry->lockedSinceLastInst = 0;
    }
    if (geometry->numMorphTargets != 1 && (atomic->interpolator.flags & rpINTERPOLATORDIRTY)) {
        RPGAMECUBEWAITDRAWSYNC(instance->syncToken);
        fn_80464814(geometry, instance, &atomic->interpolator);
        atomic->interpolator.flags &= ~rpINTERPOLATORDIRTY;
    }
    return atomic;
}

/* Creates the instanced data of an atomic's geometry, owned by the atomic when it has several
 * morph targets */
void *fn_804639AC(void *object, RpGameCubeRenderMesh *renderMesh) {
    RpAtomic *atomic = object;
    RpGeometry *geometry = atomic->geometry;
    void *owner;
    RwResEntry **ownerRef;

    if (geometry->numMorphTargets != 1) {
        owner = atomic;
        ownerRef = &atomic->repEntry;
    } else {
        owner = geometry;
        ownerRef = &geometry->repEntry;
    }

    if (geometry->flags & rpGEOMETRYNATIVEINSTANCE) {
        if (lbl_805EDDD8 == 1) {
            renderMesh->repEntry = fn_8046B79C(geometry, owner, ownerRef);
        } else {
            renderMesh->repEntry = fn_8046C0D4(geometry, owner, ownerRef);
        }
    } else {
        renderMesh->repEntry = fn_8046C0D4(geometry, owner, ownerRef);
    }
    geometry->lockedSinceLastInst = 0;
    return object;
}

/* The atomic all-in-one node body: instances the atomic's geometry, or reinstances and reuses its
 * existing instanced data, then lights and renders it */
RwBool fn_80463A5C(RxPipelineNode *self, const RxPipelineNodeParam *params) {
    RpAtomic *atomic = params->dataParam;
    RxGameCubeAllInOneData *nodeData = self->privateData;
    RpGeometry *geometry;
    RpGameCubeRenderMesh renderMesh;

    geometry = atomic->geometry;
    renderMesh.nodeData = nodeData;
    if (!(geometry->flags & rpGEOMETRYNATIVE)) {
        RpMeshHeader *meshHeader;

        if (geometry->numVertices == 0) {
            return TRUE;
        }
        meshHeader = geometry->mesh;
        if (meshHeader->numMeshes == 0) {
            return TRUE;
        }

        if (geometry->numMorphTargets != 1) {
            renderMesh.repEntry = atomic->repEntry;
        } else {
            renderMesh.repEntry = geometry->repEntry;
        }
        renderMesh.meshHeader = geometry->mesh;
        renderMesh.geometryFlags = geometry->flags;

        /* instanced data of an older mesh is destroyed */
        if (renderMesh.repEntry != NULL &&
            ((RpGameCubeInstanceData *)(renderMesh.repEntry + 1))->serialNum !=
                meshHeader->serialNum) {
            fn_8048071C(renderMesh.repEntry);
            renderMesh.repEntry = NULL;
        }

        if (renderMesh.repEntry != NULL) {
            if (!RXGAMECUBEALLINONECALL(nodeData->reinstanceCallBack, atomic, &renderMesh)) {
                return FALSE;
            }
            RwResourcesUseResEntry(renderMesh.repEntry);
        } else {
            if (!RXGAMECUBEALLINONECALL(nodeData->instanceCallBack, atomic, &renderMesh)) {
                return FALSE;
            }
            if (!RXGAMECUBEALLINONECALL(nodeData->reinstanceCallBack, atomic, &renderMesh)) {
                return FALSE;
            }
        }
    } else {
        renderMesh.repEntry = geometry->repEntry;
        renderMesh.meshHeader = geometry->mesh;
        renderMesh.geometryFlags = geometry->flags;
        if (!RXGAMECUBEALLINONECALL(nodeData->reinstanceCallBack, atomic, &renderMesh)) {
            return FALSE;
        }
    }

    if (!RXGAMECUBEALLINONECALL(nodeData->lightingCallBack, atomic, &renderMesh)) {
        return FALSE;
    }
    if (!RXGAMECUBEALLINONECALL(nodeData->renderCallBack, atomic, &renderMesh)) {
        return FALSE;
    }
    return TRUE;
}

/* Installs the default callbacks in a new atomic all-in-one node */
RwBool fn_80463CE0(RxPipelineNode *self) {
    RxGameCubeAllInOneData *nodeData = self->privateData;

    nodeData->instanceCallBack = fn_804639AC;
    nodeData->reinstanceCallBack = (RxGameCubeAllInOneCallBack)fn_80463840;
    nodeData->lightingCallBack = (RxGameCubeAllInOneCallBack)fn_80463428;
    nodeData->renderCallBack = fn_80462914;
    return TRUE;
}

RwChar lbl_8056D8B0[] = "GameCubeAtomicAllInOne.csl";

RxNodeDefinition lbl_8056D8CC = {
    lbl_8056D8B0,
    {fn_80463A5C, NULL, NULL, fn_80463CE0, NULL, NULL, NULL},
    {0, NULL, NULL, 0, NULL},
    sizeof(RxGameCubeAllInOneData),
    rxNODEDEFCONST,
    0,
};

RxNodeDefinition *fn_80463D1C(void) {
    return &lbl_8056D8CC;
}
