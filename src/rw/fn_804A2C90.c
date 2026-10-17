/* RenderWare material effects plugin (plugin ID 0x120), GameCube: rendering a mesh with its
 * material's effects (a second texture pass, texture coordinate transforms, environment and bump
 * mapping, or a GameCube multi-texture effect), the atomic and world sector instance and render
 * callbacks, and the material effects pipelines. The original file name is unknown. */
#include "rw/rpmatfx.h"
#include <dolphin/gx.h>
#include <dolphin/mtx.h>
#include <math.h>
#include "layout_check.h"

/* the pipelines' plugin data (inferred) */
#define rpMATFXATOMICPIPELINE 1
#define rpMATFXWORLDSECTORPIPELINE 2

/* white and opaque black */
static const GXColor lbl_805FB450 = {255, 255, 255, 255};
static const GXColor lbl_805FB454 = {0, 0, 0, 255};

/* The GX state the material effects renderers share between the meshes of an object: whether the
 * colour channel is set up, the material whose colours are set, and whether the single texture
 * stage is set up (inferred) */
typedef struct MatFXRenderState {
    /* 0x0 */ RwBool channelsSet;
    /* 0x4 */ RpMaterial *material;
    /* 0x8 */ RwBool tevSet;
} MatFXRenderState;
CHECK_SIZEOF(MatFXRenderState, 0xC);

MatFXRenderState lbl_805E32C8;

/* declared in reverse address order (the compiler places them in .sbss last first) */
RxPipeline *lbl_805F16C0; /* the world sector pipeline */
RxPipeline *lbl_805F16BC; /* the atomic pipeline */
RpGameCubeVertexFormat *lbl_805F16B8;

/* Sets the GX channel colours of a mesh for a material: with lights, the ambient light scaled by
 * the material's ambient coefficient and the material colour; without, the material colour lit by
 * the ambient light */
void fn_804A2C90(RpGameCubeRenderMesh *renderMesh, RpMaterial *material) {
    GXColor color;

    if (renderMesh->numLights > 0) {
        if (renderMesh->hasAmbient && !(renderMesh->geometryFlags & rpGEOMETRYPRELIT)) {
            RwReal scale = 255.0f * material->surfaceProps.ambient;

            color.a = 255;
            color.r = scale * renderMesh->ambientColor.red;
            color.g = scale * renderMesh->ambientColor.green;
            color.b = scale * renderMesh->ambientColor.blue;
            GXSetChanAmbColor(GX_COLOR0A0, color);
        }
        if (renderMesh->geometryFlags & rpGEOMETRYMODULATEMATERIALCOLOR) {
            GXSetChanMatColor(GX_COLOR0A0, *(GXColor *)&material->color);
        }
    } else if (renderMesh->geometryFlags & rpGEOMETRYPRELIT) {
        if (renderMesh->geometryFlags & rpGEOMETRYMODULATEMATERIALCOLOR) {
            GXSetChanMatColor(GX_COLOR0A0, *(GXColor *)&material->color);
        }
    } else if (renderMesh->hasAmbient) {
        RwReal ambientScale = material->surfaceProps.ambient;

        if (renderMesh->geometryFlags & rpGEOMETRYMODULATEMATERIALCOLOR) {
            color.r = material->color.red * (ambientScale * renderMesh->ambientColor.red);
            color.g = material->color.green * (ambientScale * renderMesh->ambientColor.green);
            color.b = material->color.blue * (ambientScale * renderMesh->ambientColor.blue);
            color.a = material->color.alpha;
        } else {
            /* the ambient light as a byte colour */
            ambientScale *= 255.0f;
            color.a = 255;
            color.r = ambientScale * renderMesh->ambientColor.red;
            color.g = ambientScale * renderMesh->ambientColor.green;
            color.b = ambientScale * renderMesh->ambientColor.blue;
        }
        GXSetChanMatColor(GX_COLOR0A0, color);
    } else if (renderMesh->geometryFlags & rpGEOMETRYMODULATEMATERIALCOLOR) {
        color.b = 0;
        color.g = 0;
        color.r = 0;
        color.a = material->color.alpha;
        GXSetChanMatColor(GX_COLOR0A0, color);
    }
}

void fn_804A2F08(void) {
    lbl_805E32C8.tevSet = FALSE;
    lbl_805E32C8.channelsSet = FALSE;
    lbl_805E32C8.material = NULL;
}

/* Sets up the GX colour channel for a mesh's lighting, once per object (name inferred) */
static inline void MatFXSetupChannels(RpGameCubeRenderMesh *renderMesh) {
    if (!lbl_805E32C8.channelsSet) {
        RwInt32 flags = renderMesh->geometryFlags;

        if (renderMesh->numLights > 0) {
            GXColorSrc ambientSource;

            if (flags & rpGEOMETRYPRELIT) {
                ambientSource = GX_SRC_VTX;
            } else {
                ambientSource = GX_SRC_REG;
                if (!renderMesh->hasAmbient) {
                    GXSetChanAmbColor(GX_COLOR0A0, lbl_805FB454);
                }
            }
            GXSetNumChans(1);
            GXSetChanCtrl(GX_COLOR0A0, GX_TRUE, ambientSource, GX_SRC_REG, renderMesh->lightMask,
                          GX_DF_CLAMP, GX_AF_SPOT);
            if (!(flags & rpGEOMETRYMODULATEMATERIALCOLOR)) {
                GXSetChanMatColor(GX_COLOR0A0, lbl_805FB450);
            }
        } else if (flags & rpGEOMETRYPRELIT) {
            if (flags & rpGEOMETRYMODULATEMATERIALCOLOR) {
                GXSetNumChans(1);
                GXSetChanCtrl(GX_COLOR0A0, GX_TRUE, GX_SRC_VTX, GX_SRC_REG, GX_LIGHT_NULL,
                              GX_DF_NONE, GX_AF_NONE);
            } else {
                GXSetNumChans(1);
                GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL,
                              GX_DF_NONE, GX_AF_NONE);
            }
        } else {
            if (!renderMesh->hasAmbient && !(flags & rpGEOMETRYMODULATEMATERIALCOLOR)) {
                GXSetChanMatColor(GX_COLOR0A0, lbl_805FB454);
            }
            GXSetNumChans(1);
            GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                          GX_AF_NONE);
        }
        lbl_805E32C8.channelsSet = TRUE;
    }
}

/* Sets the channel colours of a mesh's material when the material changes (name inferred) */
static inline void MatFXSetupMaterialColor(RpGameCubeRenderMesh *renderMesh, RpMesh *mesh) {
    if (lbl_805E32C8.material != mesh->material) {
        fn_804A2C90(renderMesh, mesh->material);
        lbl_805E32C8.material = mesh->material;
    }
}

/* Sets up a single texture stage, modulating the texture or passing the colour through, once per
 * object (name inferred) */
static inline void MatFXSetupTev(RpGameCubeRenderMesh *renderMesh) {
    if (!lbl_805E32C8.tevSet) {
        GXSetNumTevStages(1);
        if (renderMesh->geometryFlags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
            GXSetNumTexGens(1);
            GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE,
                              GX_PTIDENTITY);
            GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
            GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
        } else {
            GXSetNumTexGens(0);
            GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
            GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
        }
        lbl_805E32C8.tevSet = TRUE;
    }
}

/* Sets texture map 0, with alpha testing unless its raster is opaque (name inferred) */
static inline void MatFXSetTexture(RwTexture *texture) {
    fn_80499C38(texture, 0);
    if (texture != NULL && texture->raster != NULL) {
        fn_80497778(
            (RWPLUGINOFFSET(RwGameCubeRasterExt, texture->raster->parent, lbl_805F1658)->flags &
             1) ^
            1);
    } else {
        fn_80497778(TRUE);
    }
}

/* The matrix that brings normals into the space an environment map is projected in: the inverse of
 * its frame's LTM, or the camera's view transform without a frame, applied after the object's
 * LTM (name inferred) */
static inline RwMatrix *MatFXEnvMapMatrix(RwFrame *frame, RwMatrix *ltm, RwMatrix *frameMatrix,
                                          RwMatrix *objectMatrix) {
    RwMatrix *matrix;

    if (frame != NULL) {
        fn_8047F0B0(frameMatrix, RwFrameGetLTM(frame));
        fn_8047ECB4(frameMatrix, frameMatrix);
        matrix = frameMatrix;
    } else {
        matrix = &lbl_805E31CC;
    }
    if (ltm != NULL) {
        fn_8047EBB8(objectMatrix, ltm, matrix);
        fn_8047ECB4(objectMatrix, objectMatrix);
        matrix = objectMatrix;
    }
    return matrix;
}

/* Loads a texture matrix that maps normals into an environment map projected from a frame (or the
 * camera) (name inferred). Not inline: its out-of-line copy, which the linker strips, comes first in
 * the unit and so places its literals first. */
static void MatFXLoadEnvMapMatrix(Mtx texMatrix, RwFrame *frame, RwMatrix *ltm, u32 id) {
    RwMatrix objectMatrix;
    RwMatrix frameMatrix;
    RwMatrix *matrix = MatFXEnvMapMatrix(frame, ltm, &frameMatrix, &objectMatrix);

    texMatrix[0][0] = -0.5f * matrix->right.x;
    texMatrix[0][1] = -0.5f * matrix->up.x;
    texMatrix[0][2] = -0.5f * matrix->at.x;
    texMatrix[0][3] = 0.5f;
    texMatrix[1][0] = -0.5f * matrix->right.y;
    texMatrix[1][1] = -0.5f * matrix->up.y;
    texMatrix[1][2] = -0.5f * matrix->at.y;
    texMatrix[1][3] = 0.5f;
    GXLoadTexMtxImm(texMatrix, id, GX_MTX2x4);
}

/* Loads a texture matrix from a texture coordinate transform (name inferred) */
static inline void MatFXLoadUVTransform(Mtx texMatrix, const RwMatrix *transform) {
    texMatrix[0][0] = transform->right.x;
    texMatrix[0][1] = transform->up.x;
    texMatrix[0][2] = 0.0f;
    texMatrix[0][3] = transform->pos.x;
    texMatrix[1][0] = transform->right.y;
    texMatrix[1][1] = transform->up.y;
    texMatrix[1][2] = 0.0f;
    texMatrix[1][3] = transform->pos.y;
    GXLoadTexMtxImm(texMatrix, GX_TEXMTX0, GX_MTX2x4);
}

/* Loads a texture matrix that offsets texture coordinates along the light direction, from a frame
 * (or the camera), by scale (name inferred) */
static inline void MatFXLoadBumpMapMatrix(Mtx texMatrix, RwFrame *frame, RwMatrix *ltm,
                                          RwReal scale) {
    RwMatrix objectMatrix;
    RwMatrix frameMatrix;
    RwMatrix *matrix = MatFXEnvMapMatrix(frame, ltm, &frameMatrix, &objectMatrix);

    texMatrix[0][0] = scale * matrix->right.x;
    texMatrix[0][1] = scale * matrix->up.x;
    texMatrix[0][2] = scale * matrix->at.x;
    texMatrix[0][3] = 0.0f;
    texMatrix[1][0] = scale * matrix->right.y;
    texMatrix[1][1] = scale * matrix->up.y;
    texMatrix[1][2] = scale * matrix->at.y;
    texMatrix[1][3] = 0.0f;
    GXLoadTexMtxImm(texMatrix, GX_TEXMTX0, GX_MTX2x4);
}

/* Renders a mesh with a second texture pass blended over it */
void fn_804A2F24(RpMesh *mesh, RpGameCubeDisplayList *displayList,
                 RpGameCubeRenderMesh *renderMesh) {
    MatFXDualData *dual = &(*MATFXMATERIALGETDATA(mesh->material))->data[0].data.dual;
    RwInt32 srcBlend;
    RwInt32 dstBlend;

    MatFXSetupChannels(renderMesh);
    MatFXSetupMaterialColor(renderMesh, mesh);
    MatFXSetupTev(renderMesh);
    MatFXSetTexture(mesh->material->texture);
    GXCallDisplayList(displayList->data, displayList->size);

    RwRenderStateGet(rwRENDERSTATESRCBLEND, &srcBlend);
    RwRenderStateGet(rwRENDERSTATEDESTBLEND, &dstBlend);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)dual->srcBlendMode);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)dual->dstBlendMode);
    if (renderMesh->geometryFlags & rpGEOMETRYTEXTURED2) {
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, GX_FALSE,
                          GX_PTIDENTITY);
        lbl_805E32C8.tevSet = FALSE;
    }
    MatFXSetTexture(dual->texture);
    GXCallDisplayList(displayList->data, displayList->size);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)srcBlend);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)dstBlend);
}

/* Renders a mesh with its base texture coordinates transformed, then a second texture pass with
 * its own transform */
void fn_804A3308(RpMesh *mesh, RpGameCubeDisplayList *displayList,
                 RpGameCubeRenderMesh *renderMesh) {
    rpMatFXMaterialData *data = *MATFXMATERIALGETDATA(mesh->material);
    MatFXUVAnimData *uvAnim = &data->data[0].data.uvAnim;
    MatFXDualData *dual = &data->data[1].data.dual;
    RwInt32 srcBlend;
    RwInt32 dstBlend;
    Mtx texMatrix;

    MatFXSetupChannels(renderMesh);
    MatFXSetupMaterialColor(renderMesh, mesh);
    MatFXSetupTev(renderMesh);
    if (uvAnim->baseTransform != NULL) {
        MatFXLoadUVTransform(texMatrix, uvAnim->baseTransform);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE,
                          GX_PTIDENTITY);
    }
    MatFXSetTexture(mesh->material->texture);
    GXCallDisplayList(displayList->data, displayList->size);

    /* the second pass uses the second texture coordinates when the geometry has them */
    if (renderMesh->geometryFlags & rpGEOMETRYTEXTURED2) {
        if (uvAnim->dualTransform != NULL) {
            MatFXLoadUVTransform(texMatrix, uvAnim->dualTransform);
            GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX1, GX_TEXMTX0, GX_FALSE,
                              GX_PTIDENTITY);
            lbl_805E32C8.tevSet = FALSE;
        } else {
            GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, GX_FALSE,
                              GX_PTIDENTITY);
            lbl_805E32C8.tevSet = FALSE;
        }
    } else if (uvAnim->dualTransform != NULL) {
        MatFXLoadUVTransform(texMatrix, uvAnim->dualTransform);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE,
                          GX_PTIDENTITY);
        lbl_805E32C8.tevSet = FALSE;
    } else if (uvAnim->baseTransform != NULL) {
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE,
                          GX_PTIDENTITY);
    }

    RwRenderStateGet(rwRENDERSTATESRCBLEND, &srcBlend);
    RwRenderStateGet(rwRENDERSTATEDESTBLEND, &dstBlend);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)dual->srcBlendMode);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)dual->dstBlendMode);
    MatFXSetTexture(dual->texture);
    GXCallDisplayList(displayList->data, displayList->size);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)srcBlend);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)dstBlend);
}

/* Renders a mesh, then its environment map added over it: the map is looked up by the vertex
 * normals in the space of the map's frame, scaled by its coefficient, and weighted by the frame
 * buffer alpha if the material asks for it */
void fn_804A388C(RpMesh *mesh, RpGameCubeDisplayList *displayList, RwMatrix *ltm,
                 RpGameCubeRenderMesh *renderMesh) {
    RwInt32 srcBlend;
    RwInt32 dstBlend;
    GXColor color = {255, 255, 255, 255};
    MatFXEnvMapData *envMap = &(*MATFXMATERIALGETDATA(mesh->material))->data[0].data.envMap;
    Mtx texMatrix;

    MatFXSetupChannels(renderMesh);
    MatFXSetupMaterialColor(renderMesh, mesh);
    MatFXSetupTev(renderMesh);
    fn_80499C38(mesh->material->texture, 0);
    fn_80497778(TRUE);
    GXCallDisplayList(displayList->data, displayList->size);

    RwRenderStateGet(rwRENDERSTATESRCBLEND, &srcBlend);
    RwRenderStateGet(rwRENDERSTATEDESTBLEND, &dstBlend);
    RwRenderStateSet(rwRENDERSTATESRCBLEND,
                     (void *)(envMap->useFrameBufferAlpha ? rwBLENDDESTALPHA : rwBLENDONE));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);

    /* unlit, in the material colour's place the coefficient as a grey */
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_CLAMP,
                  GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_CLAMP,
                  GX_AF_NONE);
    if (envMap->coef < 1.0f) {
        RwUInt8 intensity = 255.9f * envMap->coef;

        color.a = 255;
        color.b = intensity;
        color.g = intensity;
        color.r = intensity;
    } else {
        color.a = 255;
        color.b = 255;
        color.g = 255;
        color.r = 255;
    }
    GXSetChanMatColor(GX_COLOR0A0, color);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    lbl_805E32C8.material = NULL;
    lbl_805E32C8.channelsSet = FALSE;
    lbl_805E32C8.tevSet = FALSE;

    fn_80499C38(envMap->texture, 0);
    MatFXLoadEnvMapMatrix(texMatrix, envMap->frame, ltm, GX_TEXMTX0);
    GXCallDisplayList(displayList->data, displayList->size);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)srcBlend);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)dstBlend);
}

/* Renders a mesh with its bump map, and its environment map when envMapped is set, in one pass:
 * the bumped texture's alpha is looked up again with texture coordinates offset along the light
 * direction (an indirect stage) to shade the bumps, and the environment map is added weighted by
 * its coefficient or the frame buffer alpha */
void fn_804A3DB0(RpMesh *mesh, RpGameCubeDisplayList *displayList, RwMatrix *ltm,
                 RpGameCubeRenderMesh *renderMesh, RwBool envMapped) {
    MatFXBumpMapData *bumpMap = &(*MATFXMATERIALGETDATA(mesh->material))->data[0].data.bumpMap;
    RwReal scale;
    RwUInt8 numStages;
    Mtx texMatrix;
    Mtx envTexMatrix;

    MatFXSetupChannels(renderMesh);
    MatFXSetupMaterialColor(renderMesh, mesh);
    lbl_805E32C8.tevSet = FALSE;
    fn_80499C38(bumpMap->bumpedTexture, 0);
    fn_80497778(TRUE);

    /* offsets of the bump lookup: the light direction scaled to the bumpiness in texels */
    scale = -bumpMap->invBumpWidth * bumpMap->coef;
    MatFXLoadBumpMapMatrix(texMatrix, bumpMap->frame, ltm, scale);

    if (envMapped) {
        MatFXEnvMapData *envMap = &(*MATFXMATERIALGETDATA(mesh->material))->data[1].data.envMap;

        fn_80499C38(envMap->texture, 1);
        MatFXLoadEnvMapMatrix(envTexMatrix, envMap->frame, ltm, GX_TEXMTX1);
        GXSetNumTexGens(3);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE,
                          GX_PTIDENTITY);
        GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX0, GX_FALSE,
                          GX_PTIDENTITY);
        GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX1, GX_FALSE,
                          GX_PTIDENTITY);
    } else {
        GXSetNumTexGens(2);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE,
                          GX_PTIDENTITY);
        GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX0, GX_FALSE,
                          GX_PTIDENTITY);
    }

    /* stage 0: the lit texture into register 0; the alpha is 1 minus the bump alpha */
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_KONST, GX_CA_ZERO, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    /* stage 1: register 0 weighted by that alpha; the alpha is replaced by the bump alpha at the
     * offset coordinates */
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevIndirect(GX_TEVSTAGE1, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE, GX_ITM_OFF, GX_ITW_OFF,
                     GX_ITW_OFF, GX_TRUE, GX_FALSE, GX_ITBA_OFF);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_APREV, GX_CC_C0, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    if (envMapped) {
        MatFXEnvMapData *envMap = &(*MATFXMATERIALGETDATA(mesh->material))->data[1].data.envMap;
        GXColor kColor;

        /* stage 2: adds register 0 weighted by the offset bump alpha; the alpha is the environment
         * map's weight */
        if (envMap->coef < 1.0f) {
            kColor.a = 255.9f * envMap->coef;
        } else {
            kColor.a = 255;
        }
        GXSetTevKColor(GX_KCOLOR0, kColor);
        GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
        GXSetTevKAlphaSel(GX_TEVSTAGE2, GX_TEV_KASEL_K0_A);
        GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_APREV, GX_CC_C0, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        if (envMap->useFrameBufferAlpha) {
            GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_KONST, GX_CA_APREV, GX_CA_ZERO);
        } else {
            GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        }
        GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

        /* stage 3: the environment map added by that weight */
        numStages = 3;
        GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD2, GX_TEXMAP1, GX_COLOR0A0);
        GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_APREV, GX_CC_TEXC, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_RASA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    } else {
        /* stage 2: adds register 0 weighted by the offset bump alpha */
        numStages = 2;
        GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_APREV, GX_CC_C0, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_RASA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    }
    GXSetNumTevStages(numStages + 1);
    GXCallDisplayList(displayList->data, displayList->size);
    GXSetTevDirect(GX_TEVSTAGE1);
}

/* Renders a mesh without material effects (name inferred) */
static inline void MatFXRenderPlain(RpMesh *mesh, RpGameCubeDisplayList *displayList,
                                    RpGameCubeRenderMesh *renderMesh) {
    MatFXSetupChannels(renderMesh);
    MatFXSetupMaterialColor(renderMesh, mesh);
    MatFXSetupTev(renderMesh);
    MatFXSetTexture(mesh->material->texture);
    GXCallDisplayList(displayList->data, displayList->size);
}

void fn_804A457C(RpMesh *mesh, RpGameCubeDisplayList *displayList, RpAtomic *atomic, RwMatrix *ltm,
                 RpGameCubeRenderMesh *renderMesh) {
    rpMatFXMaterialData *data;

    if (fn_804A90C8(mesh->material, rwID_GAMECUBE) != NULL) {
        MatFXSetupChannels(renderMesh);
        MatFXSetupMaterialColor(renderMesh, mesh);
        lbl_805E32C8.tevSet = FALSE;
        fn_804A6A08(displayList, mesh->material, atomic, renderMesh, ltm);
        return;
    }

    data = *MATFXMATERIALGETDATA(mesh->material);
    if (data != NULL) {
        switch (data->flags) {
        case rpMATFXEFFECTDUAL:
            fn_804A2F24(mesh, displayList, renderMesh);
            break;
        case rpMATFXEFFECTUVTRANSFORM: {
            MatFXUVAnimData *uvAnim = &data->data[0].data.uvAnim;
            Mtx texMatrix;

            MatFXSetupChannels(renderMesh);
            MatFXSetupMaterialColor(renderMesh, mesh);
            MatFXSetupTev(renderMesh);
            if (uvAnim->baseTransform != NULL) {
                MatFXLoadUVTransform(texMatrix, uvAnim->baseTransform);
                GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE,
                                  GX_PTIDENTITY);
                lbl_805E32C8.tevSet = FALSE;
            }
            MatFXSetTexture(mesh->material->texture);
            GXCallDisplayList(displayList->data, displayList->size);
            break;
        }
        case rpMATFXEFFECTDUALUVTRANSFORM:
            fn_804A3308(mesh, displayList, renderMesh);
            break;
        case rpMATFXEFFECTENVMAP:
            fn_804A388C(mesh, displayList, ltm, renderMesh);
            break;
        case rpMATFXEFFECTBUMPMAP:
            fn_804A3DB0(mesh, displayList, ltm, renderMesh, FALSE);
            break;
        case rpMATFXEFFECTBUMPENVMAP:
            fn_804A3DB0(mesh, displayList, ltm, renderMesh, TRUE);
            break;
        default:
            MatFXRenderPlain(mesh, displayList, renderMesh);
            break;
        }

    } else {
        MatFXRenderPlain(mesh, displayList, renderMesh);
    }
}

/* Instances an atomic, with binormals and tangents in a vertex format that holds them when its
 * meshes' multi-texture effects need them */
RpAtomic *fn_804A5058(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh) {
    if (fn_804A8478(renderMesh)) {
        RpGeometry *geometry = atomic->geometry;
        RpGameCubeVertexFormat *format = GAMECUBEVERTEXFORMAT(geometry);
        RwInt32 mode;

        if (format == NULL) {
            fn_80463104(geometry, lbl_805F16B8);
            format = lbl_805F16B8;
        }
        mode = fn_80462C28();
        if (mode == 1) {
            fn_80462C20(0);
        }
        atomic = fn_804639AC(atomic, renderMesh);
        if (mode == 1) {
            fn_80462C20(1);
        }
        if (atomic != NULL) {
            fn_804A8120(renderMesh, format, geometry->numVertices);
        }
        return atomic;
    }
    return fn_804639AC(atomic, renderMesh);
}

/* Reinstances an atomic, rebuilding its binormals and tangents when its vertices, normals or
 * texture coordinates changed or it is morphing */
RpAtomic *fn_804A5124(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh) {
    RpGeometry *geometry = atomic->geometry;
    RpGameCubeVertexFormat *format = GAMECUBEVERTEXFORMAT(geometry);
    RwBool rebuild = FALSE;

    if (format != NULL && format->normalNbt != 0 && (geometry->flags & rpGEOMETRYNORMALS)) {
        RwUInt32 locked =
            geometry->lockedSinceLastInst & (rpGEOMETRYLOCKPOLYGONS | rpGEOMETRYLOCKVERTICES |
                                             rpGEOMETRYLOCKNORMALS | rpGEOMETRYLOCKTEXCOORDS);
        RwUInt32 morphing = geometry->numMorphTargets != 1 &&
                            (atomic->interpolator.flags & rpINTERPOLATORDIRTYINSTANCE);
        rebuild = locked || morphing;
    }
    if (fn_80463840(atomic, renderMesh) == NULL) {
        return NULL;
    }
    if (rebuild) {
        fn_804A8120(renderMesh, format, geometry->numVertices);
    }
    return atomic;
}

/* Instances a world sector, with binormals and tangents when the world's vertex format holds
 * them */
RpWorldSector *fn_804A5204(RpWorldSector *sector, RpGameCubeRenderMesh *renderMesh) {
    RpGameCubeVertexFormat *format;

    if (fn_80463F10(sector, renderMesh) == NULL) {
        return NULL;
    }
    format = GAMECUBEWORLDVERTEXFORMAT(lbl_805F15B4->curWorld);
    if (format != NULL && format->normalNbt != 0) {
        fn_804A8120(renderMesh, format, sector->numVertices);
    }
    return sector;
}

/* Renders an atomic's or world sector's meshes with their material effects. A world sector whose
 * positions are fixed point is drawn relative to its box's corner, truncated toward zero to a
 * multiple of the fixed point step (x - fmod(x, step)). */
void *fn_804A5280(void *object, RpGameCubeRenderMesh *renderMesh) {
    RpGameCubeInstanceData *instance = (RpGameCubeInstanceData *)(renderMesh->repEntry + 1);
    RpGameCubeDisplayList *displayList;
    RwInt32 numMeshes;
    RpMesh *mesh;
    RwMatrix origin;
    RwMatrix *ltm;
    RpGameCubeVertexFormat *format;

    displayList = RPGAMECUBEINSTANCEDISPLAYLISTS(instance);
    instance->syncToken = lbl_805EDF40;
    if (((RwObject *)object)->type == rpATOMIC) {
        ltm = RwFrameGetLTM(((RpAtomic *)object)->object.parent);
        format = GAMECUBEVERTEXFORMAT(((RpAtomic *)object)->geometry);
    } else {
        format = GAMECUBEWORLDVERTEXFORMAT(lbl_805F15B4->curWorld);
        if (format != NULL && format->posType != GX_F32) {
            RpWorldSector *sector = object;
            RwReal step;

            origin.up.x = 0.0f;
            origin.at.z = 1.0f;
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
    fn_804A2F08();

    numMeshes = renderMesh->meshHeader->numMeshes;
    mesh = (RpMesh *)(renderMesh->meshHeader + 1);
    while (numMeshes-- != 0) {
        fn_804A457C(mesh, displayList, object, ltm, renderMesh);
        displayList++;
        mesh++;
    }
    return object;
}

RwBool fn_804A5494(void) {
    lbl_805F16B8 = fn_8046301C();
    fn_80462F88(lbl_805F16B8, GX_F32, TRUE);
    lbl_805F16BC = fn_8046335C(rpID_MATFXPLUGIN, rpMATFXATOMICPIPELINE, fn_804A5058, fn_804A5124,
                               fn_80463428, (RpGameCubeRenderCallBack)fn_804A5280);
    lbl_805F16C0 = fn_80463D28(rpID_MATFXPLUGIN, rpMATFXWORLDSECTORPIPELINE, fn_804A5204, NULL,
                               fn_80463DF4, (RpGameCubeSectorCallBack)fn_804A5280);
    return TRUE;
}

RwBool fn_804A5524(void) {
    if (lbl_805F16B8 != NULL) {
        fn_804630C0(lbl_805F16B8);
        lbl_805F16B8 = NULL;
    }
    if (lbl_805F16BC != NULL) {
        fn_8049B800(lbl_805F16BC);
        lbl_805F16BC = NULL;
    }
    if (lbl_805F16C0 != NULL) {
        fn_8049B800(lbl_805F16C0);
        lbl_805F16C0 = NULL;
    }
    return TRUE;
}

RpAtomic *fn_804A558C(RpAtomic *atomic) {
    atomic->pipeline = lbl_805F16BC;
    return atomic;
}

RpWorldSector *fn_804A5598(RpWorldSector *sector) {
    sector->pipeline = lbl_805F16C0;
    return sector;
}

RwBool fn_804A55A4(MatFXDualData *dualData, RwInt32 renderState) {
    return TRUE;
}

RwTexture *fn_804A55AC(RwTexture *baseTexture, RwTexture *bumpTexture) {
    return fn_804A1A08(baseTexture, bumpTexture);
}
