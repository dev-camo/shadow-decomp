/* RenderWare GameCube world device: waiting for instanced data, the default render callback of
 * atomics and world sectors, the all-in-one node callbacks, the vertex descriptor setup, vertex
 * formats and their geometry and world plugin, and the platform pipelines. The original file name
 * is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>
#include <math.h>

#define rwMEMHINTDUR_EVENT 0x30000

/* engine and object plugin IDs (inferred) */
#define rwID_VTXFMTPLUGIN 0x50D
#define rwID_GAMECUBEWORLDPLUGIN 0x511

/* the plugin ID the platform pipelines report (inferred) */
#define rwPLATFORMPIPELINEPLUGIN 2

/* RwMatrix flags */
#define rwMATRIXTYPEORTHONORMAL 0x3
#define rwMATRIXINTERNALIDENTITY 0x20000

/* The vertex format module (inferred) */
typedef struct VtxFmtModule {
    /* 0x0 */ RwInt32 globalsOffset;
    /* 0x4 */ RwInt32 numInstances; /* the open engine instances */
} VtxFmtModule;

#define PIPELINEGLOBAL(var) (RWPLUGINOFFSET(RpPipelineGlobals, lbl_805F15B4, lbl_805F1690)->var)

RpGameCubeVertexFormat lbl_805E1688; /* the default vertex format */

RwInt32 lbl_805EDDD8 = 1; /* the instancing optimisation mode (inferred) */

/* declared in reverse address order (the compiler places them in .sbss last first) */
VtxFmtModule lbl_805F1528;
RwInt32 lbl_805F1524; /* world plugin offset of the vertex format */
RwInt32 lbl_805F1520; /* geometry plugin offset of the vertex format */

void fn_8046288C(RwResEntry *repEntry) {
    RpGameCubeInstanceData *instance = (RpGameCubeInstanceData *)(repEntry + 1);

    RPGAMECUBEWAITDRAWSYNC(instance->syncToken);
}

/* Renders an atomic's or world sector's meshes from their display lists. A world sector whose
 * positions are fixed point is drawn relative to its box's corner, truncated toward zero to a
 * multiple of the fixed point step (x - fmod(x, step)). */
void *fn_80462914(void *object, RpGameCubeRenderMesh *renderMesh) {
    RpGameCubeInstanceData *instance = (RpGameCubeInstanceData *)(renderMesh->repEntry + 1);
    RpGameCubeDisplayList *displayList;
    RwMatrix *ltm;
    RpGameCubeVertexFormat *format;
    RwMatrix origin;
    RpGameCubeMaterialColorCallBack setMaterialColor;
    RpMesh *mesh;
    RwUInt32 numMeshes;

    displayList =
        (RpGameCubeDisplayList *)instance->vertexDescriptors + instance->numVertexDescriptors;
    instance->syncToken = lbl_805EDF40;
    if (((RwObject *)object)->type == rpATOMIC) {
        ltm = RwFrameGetLTM(((RpAtomic *)object)->object.parent);
        format = GAMECUBEVERTEXFORMAT(((RpAtomic *)object)->geometry);
    } else {
        format = GAMECUBEWORLDVERTEXFORMAT(lbl_805F15B4->curWorld);
        if (format != NULL && format->posType != GX_F32) {
            RpWorldSector *sector = object;
            RwReal step;

            origin.at.z = 1.0f;
            origin.up.x = 0.0f;
            origin.up.y = 1.0f;
            origin.right.x = 1.0f;
            origin.right.z = 0.0f;
            origin.right.y = 0.0f;
            origin.at.y = 0.0f;
            origin.at.x = 0.0f;
            origin.up.z = 0.0f;
            origin.pos.z = 0.0f;
            origin.pos.y = 0.0f;
            origin.pos.x = 0.0f;
            origin.flags |= rwMATRIXINTERNALIDENTITY | rwMATRIXTYPEORTHONORMAL;
            step = 1.0f / (1 << format->posFrac);
            origin.pos.x =
                sector->boundingBox.inf.x - (RwReal)fmod(sector->boundingBox.inf.x, step);
            origin.pos.y =
                sector->boundingBox.inf.y - (RwReal)fmod(sector->boundingBox.inf.y, step);
            origin.pos.z =
                sector->boundingBox.inf.z - (RwReal)fmod(sector->boundingBox.inf.z, step);
            ltm = &origin;
        } else {
            ltm = NULL;
        }
    }

    fn_80462C30(format, renderMesh);
    if (renderMesh->geometryFlags & rpGEOMETRYNORMALS) {
        fn_804950AC(ltm, TRUE);
    } else {
        fn_804950AC(ltm, FALSE);
    }
    setMaterialColor = fn_804655D4(renderMesh->geometryFlags, renderMesh->lightMask,
                                   renderMesh->hasAmbient, instance->flags & 1);
    numMeshes = renderMesh->meshHeader->numMeshes;
    mesh = (RpMesh *)(renderMesh->meshHeader + 1);

    if (renderMesh->geometryFlags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
        while (numMeshes-- != 0) {
            RwTexture *texture = mesh->material->texture;

            fn_80499C38(texture, 0);
            if (texture != NULL && texture->raster != NULL) {
                fn_80497778(
                    (RWPLUGINOFFSET(RwGameCubeRasterExt, texture->raster->parent, lbl_805F1658)
                         ->flags &
                     1) ^
                    1);
            } else {
                fn_80497778(TRUE);
            }
            if (setMaterialColor != NULL) {
                setMaterialColor(&renderMesh->ambientColor, &mesh->material->color,
                                 mesh->material->surfaceProps.ambient);
            }
            GXCallDisplayList(displayList->data, displayList->size);
            displayList++;
            mesh++;
        }
    } else {
        fn_80497778(TRUE);
        while (numMeshes-- != 0) {
            if (setMaterialColor != NULL) {
                setMaterialColor(&renderMesh->ambientColor, &mesh->material->color,
                                 mesh->material->surfaceProps.ambient);
            }
            GXCallDisplayList(displayList->data, displayList->size);
            displayList++;
            mesh++;
        }
    }
    return object;
}

void fn_80462BF0(RxPipelineNode *node, RxGameCubeAllInOneCallBack callBack) {
    ((RxGameCubeAllInOneData *)node->privateData)->instanceCallBack = callBack;
}

void fn_80462BFC(RxPipelineNode *node, RxGameCubeAllInOneCallBack callBack) {
    ((RxGameCubeAllInOneData *)node->privateData)->reinstanceCallBack = callBack;
}

void fn_80462C08(RxPipelineNode *node, RxGameCubeAllInOneCallBack callBack) {
    ((RxGameCubeAllInOneData *)node->privateData)->lightingCallBack = callBack;
}

void fn_80462C14(RxPipelineNode *node, RxGameCubeAllInOneCallBack callBack) {
    ((RxGameCubeAllInOneData *)node->privateData)->renderCallBack = callBack;
}

void fn_80462C20(RwInt32 mode) {
    lbl_805EDDD8 = mode;
}

RwInt32 fn_80462C28(void) {
    return lbl_805EDDD8;
}

/* Sets the GX vertex descriptors, formats and arrays of a mesh's instanced data */
void fn_80462C30(RpGameCubeVertexFormat *format, RpGameCubeRenderMesh *renderMesh) {
    RpGameCubeInstanceData *instance = (RpGameCubeInstanceData *)(renderMesh->repEntry + 1);
    RwUInt32 index;

    if (format == NULL) {
        format = &lbl_805E1688;
    }
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, instance->vertexDescriptors[0].type);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, format->posType, format->posFrac);
    GXSetArray(GX_VA_POS, instance->vertexDescriptors[0].data,
               instance->vertexDescriptors[0].stride);
    index = 1;

    if (renderMesh->geometryFlags & rpGEOMETRYNORMALS) {
        if (format->normalNbt) {
            GXSetVtxDesc(GX_VA_NBT, instance->vertexDescriptors[index].type);
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NBT, GX_NRM_NBT, format->normalType, 0);
            GXSetArray(GX_VA_NBT, instance->vertexDescriptors[index].data,
                       instance->vertexDescriptors[index].stride);
            index = 2;
        } else {
            GXSetVtxDesc(GX_VA_NRM, instance->vertexDescriptors[index].type);
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, format->normalType, 0);
            GXSetArray(GX_VA_NRM, instance->vertexDescriptors[index].data,
                       instance->vertexDescriptors[index].stride);
            index = 2;
        }
    }
    if (renderMesh->geometryFlags & rpGEOMETRYPRELIT) {
        GXSetVtxDesc(GX_VA_CLR0, instance->vertexDescriptors[index].type);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0,
                        format->colorType > GX_RGBX8 ? GX_CLR_RGBA : GX_CLR_RGB, format->colorType,
                        0);
        GXSetArray(GX_VA_CLR0, instance->vertexDescriptors[index].data,
                   instance->vertexDescriptors[index].stride);
        index++;
    }
    if (renderMesh->geometryFlags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
        GXAttr attribute = GX_VA_TEX0;

        while (index < instance->numVertexDescriptors) {
            GXSetVtxDesc(attribute, instance->vertexDescriptors[index].type);
            GXSetVtxAttrFmt(GX_VTXFMT0, attribute, GX_TEX_ST,
                            format->texCoordType[attribute - GX_VA_TEX0],
                            format->texCoordFrac[attribute - GX_VA_TEX0]);
            GXSetArray(attribute, instance->vertexDescriptors[index].data,
                       instance->vertexDescriptors[index].stride);
            index++;
            attribute++;
        }
    }
}

RpGameCubeVertexFormat *fn_80462E1C(void) {
    return &lbl_805E1688;
}

/* geometry and world plugin constructor and destructor of the vertex format */
void *fn_80462E28(void *object, RwInt32 offset, RwInt32 size) {
    *RWPLUGINOFFSET(RpGameCubeVertexFormat *, object, offset) = NULL;
    return object;
}

void *fn_80462E34(void *object, RwInt32 offset, RwInt32 size) {
    if (*RWPLUGINOFFSET(RpGameCubeVertexFormat *, object, offset) != NULL) {
        fn_804630C0(*RWPLUGINOFFSET(RpGameCubeVertexFormat *, object, offset));
    }
    return object;
}

/* engine plugin constructor: the default vertex format */
void *fn_80462E70(void *instance, RwInt32 offset, RwInt32 size) {
    if (++lbl_805F1528.numInstances == 1) {
        fn_80462FAC(&lbl_805E1688);
    }
    return instance;
}

/* engine plugin destructor */
void *fn_80462EC0(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F1528.numInstances--;
    return instance;
}

/* Attaches the vertex format plugin: TRUE on success */
RwBool fn_80462ED4(void) {
    if (fn_80485D78(0, rwID_GAMECUBEWORLDPLUGIN, fn_80462E70, fn_80462EC0) < 0) {
        return FALSE;
    }
    lbl_805F1520 = fn_8045994C(sizeof(RpGameCubeVertexFormat *), rwID_GAMECUBEWORLDPLUGIN,
                               fn_80462E28, fn_80462E34, NULL);
    if (lbl_805F1520 < 0) {
        return FALSE;
    }
    return (lbl_805F1524 = fn_80460790(sizeof(RpGameCubeVertexFormat *), rwID_GAMECUBEWORLDPLUGIN,
                                       fn_80462E28, fn_80462E34, NULL)) >= 0;
}

void fn_80462F7C(RpGameCubeVertexFormat *format, RwUInt8 type, RwUInt8 frac) {
    format->posType = type;
    format->posFrac = frac;
}

void fn_80462F88(RpGameCubeVertexFormat *format, RwUInt8 type, RwUInt8 nbt) {
    format->normalType = type;
    format->normalNbt = nbt;
}

void fn_80462F94(RpGameCubeVertexFormat *format, RwInt32 index, RwUInt8 type, RwUInt8 frac) {
    format->texCoordType[index - 1] = type;
    format->texCoordFrac[index - 1] = frac;
}

void fn_80462FA4(RpGameCubeVertexFormat *format, RwUInt8 type) {
    format->colorType = type;
}

/* Initialises a vertex format: floats, RGBA8 colors */
#define VTXFMTINIT(format)                                                                         \
    do {                                                                                           \
        (format)->posType = GX_F32;                                                                \
        (format)->normalType = GX_F32;                                                             \
        (format)->texCoordType[0] = GX_F32;                                                        \
        (format)->texCoordType[1] = GX_F32;                                                        \
        (format)->texCoordType[2] = GX_F32;                                                        \
        (format)->texCoordType[3] = GX_F32;                                                        \
        (format)->texCoordType[4] = GX_F32;                                                        \
        (format)->texCoordType[5] = GX_F32;                                                        \
        (format)->texCoordType[6] = GX_F32;                                                        \
        (format)->texCoordType[7] = GX_F32;                                                        \
        (format)->colorType = GX_RGBA8;                                                            \
        (format)->field_0xB = 1;                                                                   \
        (format)->posFrac = 0;                                                                     \
        (format)->texCoordFrac[0] = 0;                                                             \
        (format)->texCoordFrac[1] = 0;                                                             \
        (format)->texCoordFrac[2] = 0;                                                             \
        (format)->texCoordFrac[3] = 0;                                                             \
        (format)->texCoordFrac[4] = 0;                                                             \
        (format)->texCoordFrac[5] = 0;                                                             \
        (format)->texCoordFrac[6] = 0;                                                             \
        (format)->texCoordFrac[7] = 0;                                                             \
        (format)->normalNbt = 0;                                                                   \
        (format)->refCount = 1;                                                                    \
    } while (0)

void fn_80462FAC(RpGameCubeVertexFormat *format) {
    VTXFMTINIT(format);
}

RpGameCubeVertexFormat *fn_8046301C(void) {
    RpGameCubeVertexFormat *format =
        RwMalloc(sizeof(RpGameCubeVertexFormat), rwMEMHINTDUR_EVENT | rwID_VTXFMTPLUGIN);

    VTXFMTINIT(format);
    return format;
}

/* Releases a vertex format: frees it with the last reference */
#define VTXFMTRELEASE(format)                                                                      \
    do {                                                                                           \
        if ((format)->refCount == 1) {                                                             \
            RwFree(format);                                                                        \
        } else {                                                                                   \
            (format)->refCount--;                                                                  \
        }                                                                                          \
    } while (0)

void fn_804630C0(RpGameCubeVertexFormat *format) {
    VTXFMTRELEASE(format);
}

void fn_80463104(RpGeometry *geometry, RpGameCubeVertexFormat *format) {
    RpGameCubeVertexFormat **slot = &GAMECUBEVERTEXFORMAT(geometry);

    if (*slot != NULL) {
        VTXFMTRELEASE(*slot);
    }
    *slot = format;
    (*slot)->refCount++;
}

/* Attaches the world device plugins: TRUE on success */
RwBool fn_8046318C(void) {
    if (!fn_80462ED4()) {
        return FALSE;
    }
    if (!fn_804647C8()) {
        return FALSE;
    }
    return TRUE;
}

/* the platform pipeline open and close steps */
RwBool fn_804631CC(void) {
    return TRUE;
}

void fn_804631D4(void) {}

RwBool fn_804631D8(void) {
    void *pipeline = fn_80463D28(rwPLATFORMPIPELINEPLUGIN, 0, fn_80463F10, NULL, fn_80463DF4,
                                 (RpGameCubeSectorCallBack)fn_80462914);

    if (pipeline != NULL) {
        PIPELINEGLOBAL(platformWorldSectorPipeline) = pipeline;
        fn_804626F8(pipeline);
        return TRUE;
    }
    return FALSE;
}

void fn_80463244(void) {
    fn_804626F8(NULL);
    if (PIPELINEGLOBAL(platformWorldSectorPipeline) != NULL) {
        fn_8049B800(PIPELINEGLOBAL(platformWorldSectorPipeline));
        PIPELINEGLOBAL(platformWorldSectorPipeline) = NULL;
    }
}

RwBool fn_80463298(void) {
    void *pipeline = fn_8046335C(rwPLATFORMPIPELINEPLUGIN, 0, fn_804639AC, fn_80463840, fn_80463428,
                                 (RpGameCubeRenderCallBack)fn_80462914);

    if (pipeline != NULL) {
        PIPELINEGLOBAL(platformAtomicPipeline) = pipeline;
        fn_80462734(pipeline);
        return TRUE;
    }
    return FALSE;
}

void fn_80463308(void) {
    fn_80462734(NULL);
    if (PIPELINEGLOBAL(platformAtomicPipeline) != NULL) {
        fn_8049B800(PIPELINEGLOBAL(platformAtomicPipeline));
        PIPELINEGLOBAL(platformAtomicPipeline) = NULL;
    }
}
