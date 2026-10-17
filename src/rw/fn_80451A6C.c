/* RenderWare skinning plugin (plugin ID 0x116), GameCube: the skin pipelines (the plain one and the
 * material effects one), the instance, reinstance and render callbacks of the material effects
 * pipeline, and the choice of an atomic's skin pipeline. The original file name is unknown. */
#include "rw/rpmatfx.h"
#include "rw/rpskin.h"
#include <dolphin/gx.h>

/* skin pipeline types (inferred) */
#define SKINPIPELINEAUTO 0 /* chosen by whether the atomic has material effects */
#define SKINPIPELINEGENERIC 1
#define SKINPIPELINEMATFX 2

/* Instances a skinned geometry for the material effects pipeline */
RpAtomic *fn_80451A6C(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh) {
    RwInt32 mode;

    if (fn_804A8478(renderMesh)) {
        if (GAMECUBEVERTEXFORMAT(atomic->geometry) == NULL) {
            fn_80463104(atomic->geometry, lbl_805F16B8);
        }
        mode = fn_80462C28();
        if (mode == 1) {
            fn_80462C20(0);
        }
        atomic = fn_8044D924(atomic, renderMesh);
        if (mode == 1) {
            fn_80462C20(1);
        }
        return atomic;
    }
    return fn_8044D924(atomic, renderMesh);
}

/* Reinstances a skinned geometry for the material effects pipeline, rebuilding the material effects
 * data of geometries with binormals and tangents */
RpAtomic *fn_80451B20(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh) {
    RpGeometry *geometry;
    RpGameCubeVertexFormat *format;

    if (fn_8044DA3C(atomic, renderMesh) == NULL) {
        return NULL;
    }
    geometry = atomic->geometry;
    format = GAMECUBEVERTEXFORMAT(geometry);
    if (fn_8044CCAC(geometry)->maxNumWeightsForVertex > 1 && format != NULL &&
        format->normalNbt != 0 && (geometry->flags & rpGEOMETRYNORMALS)) {
        fn_804A8120(renderMesh, format, geometry->numVertices);
    }
    return atomic;
}

/* Renders a skinned atomic with material effects */
RpAtomic *fn_80451BCC(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh) {
    RpGameCubeInstanceData *instance = (RpGameCubeInstanceData *)(renderMesh->repEntry + 1);
    RpGameCubeDisplayList *displayList;
    RpMeshHeader *meshHeader;
    RpSkin *skin;
    RwMatrix *ltm;
    RpMesh *mesh;
    RwInt32 numMeshes;
    RwUInt32 index;
    RwUInt32 i;

    displayList = RPGAMECUBEINSTANCEDISPLAYLISTS(instance);
    instance->syncToken = lbl_805EDF40;
    meshHeader = atomic->geometry->mesh;
    ltm = RwFrameGetLTM(atomic->object.parent);
    fn_80462C30(GAMECUBEVERTEXFORMAT(atomic->geometry), renderMesh);

    skin = fn_8044CCAC(atomic->geometry);
    if (skin->maxNumWeightsForVertex > 1) {
        fn_804950AC(ltm, (renderMesh->geometryFlags >> 4) & 1);
    } else {
        GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
    }

    /* rigid skins load all their bone matrices up front */
    if (skin->maxNumWeightsForVertex == 1 && skin->skinSplitData.numMeshes == 0) {
        for (i = 0, index = 0; i < skin->numBoneIds; index += 3, i++) {
            fn_8044DC4C(&lbl_8056D708.matrixCache[skin->boneIds[i]], index,
                        (renderMesh->geometryFlags >> 4) & 1);
        }
    }

    fn_804A2F08();
    numMeshes = meshHeader->numMeshes;
    mesh = (RpMesh *)(meshHeader + 1);
    while (numMeshes-- != 0) {
        if (skin->maxNumWeightsForVertex == 1 && skin->skinSplitData.numMeshes != 0) {
            fn_8044DD1C(skin, renderMesh->meshHeader->numMeshes - (numMeshes + 1),
                        (renderMesh->geometryFlags >> 4) & 1);
        }
        fn_804A457C(mesh, displayList, atomic, ltm, renderMesh);
        displayList++;
        mesh++;
    }
    return atomic;
}

/* Opens the skin pipelines for the plugins present (flags: 0x1 skinning, 0x2 material effects) */
RwBool fn_80451D60(RwUInt32 flags) {
    SkinGlobals *globals = &lbl_8056D708;

    if (flags & 0x1) {
        globals->pipelines[SKINPIPELINEGENERIC] =
            fn_8046335C(rpID_SKINPLUGIN, SKINPIPELINEGENERIC, fn_8044D924, fn_8044DA3C, fn_80463428,
                        fn_8044DE74);
    }
    if (flags & 0x2) {
        globals->pipelines[SKINPIPELINEMATFX] = fn_8046335C(
            rpID_SKINPLUGIN, SKINPIPELINEMATFX, fn_80451A6C, fn_80451B20, fn_80463428, fn_80451BCC);
    }
    return TRUE;
}

/* Closes the skin pipelines */
RwBool fn_80451E0C(void) {
    SkinGlobals *globals = &lbl_8056D708;

    if (globals->pipelines[SKINPIPELINEGENERIC] != NULL) {
        fn_8049B800(globals->pipelines[SKINPIPELINEGENERIC]);
        globals->pipelines[SKINPIPELINEGENERIC] = NULL;
    }
    if (globals->pipelines[SKINPIPELINEMATFX] != NULL) {
        fn_8049B800(globals->pipelines[SKINPIPELINEMATFX]);
        globals->pipelines[SKINPIPELINEMATFX] = NULL;
    }
    return TRUE;
}

/* Puts a skinned atomic in a skin pipeline: the given type, or by whether it has material
 * effects */
RpAtomic *fn_80451E6C(RpAtomic *atomic, RwInt32 type) {
    RpSkin *skin = fn_8044CCAC(atomic->geometry);
    RwInt32 pipeline;

    switch (type) {
    case SKINPIPELINEGENERIC:
        skin->pipelineType = SKINPIPELINEGENERIC;
        pipeline = SKINPIPELINEGENERIC;
        break;
    case SKINPIPELINEMATFX:
        skin->pipelineType = SKINPIPELINEMATFX;
        pipeline = SKINPIPELINEMATFX;
        break;
    case SKINPIPELINEAUTO:
    default:
        if (fn_80458C90(rpID_MATFXPLUGIN) > 0 && fn_804A2064(atomic)) {
            skin->pipelineType = SKINPIPELINEMATFX;
            pipeline = SKINPIPELINEMATFX;
        } else {
            skin->pipelineType = SKINPIPELINEGENERIC;
            pipeline = SKINPIPELINEGENERIC;
        }
        break;
    }
    atomic->pipeline = lbl_8056D708.pipelines[pipeline];
    return atomic;
}
