/* RenderWare particle tank plugin (plugin ID 0x12F), GameCube: the lighting and render callbacks of
 * the tank pipeline, which set up GX for the tank's streams and run its particle kernel
 * (hand-written assembly at 0x8044920C). The original file name is unknown. */
#include "rw/rpptank.h"
#include <dolphin/gx.h>

/* The GameCube raster plugin data; only the flags are mapped */
typedef struct RwGameCubeRasterExt {
    /* 0x00 */ RwUInt8 field_0x0[0x14];
    /* 0x14 */ RwUInt32 flags;
} RwGameCubeRasterExt;

static const GXColor lbl_805FB078 = {255, 255, 255, 255}; /* white */
static const GXColor lbl_805FB07C = {0, 0, 0, 255};       /* black */

/* The lighting callback of the tank pipeline: only tanks with normals are lit */
RpAtomic *fn_80448CB0(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh) {
    if ((renderMesh->geometryFlags & rpGEOMETRYNORMALS) == rpGEOMETRYNORMALS) {
        fn_80463428(atomic, renderMesh);
    } else {
        renderMesh->lightMask = 0;
        renderMesh->hasAmbient = FALSE;
        renderMesh->numLights = 0;
    }
    return atomic;
}

/* Draws the active particles of a tank as quads with its kernel (inferred) */
static inline void PTankDrawParticles(RpAtomic *atomic) {
    RpPTankAtomicExtPrv *ptank = PTANKEXT(atomic);
    RpGeometry *geometry = atomic->geometry;
    RpPTankData *ptankGlobal = &ptank->publicData;
    PTankKernel kernel = ptank->kernel;
    RwUInt16 count = ptank->actPCount;
    RwUInt32 platFlags = ptank->platFlags;
    RpGameCubeVertexFormat *format;
    register RwUInt32 gqr6; /* asm operands need registers */
    register RwUInt32 gqr7;

    GXBegin(GX_QUADS, GX_VTXFMT0, ptank->actPCount * 4);

    /* the kernels load positions and normals with paired-single quantized loads */
    gqr6 = 0;
    gqr7 = gqr6;
    format = GAMECUBEVERTEXFORMAT(geometry);
    if (format != NULL) {
        /* GX component type to quantization type, and the scale of normals */
        RwUInt8 quantizeType[5] = {4, 6, 5, 7, 0};
        RwUInt8 normalScale[5] = {0, 6, 0, 14, 0};

        gqr6 = (format->posFrac << 8) | quantizeType[format->posType];
        gqr6 |= gqr6 << 16;
        gqr7 = quantizeType[format->normalType] | (normalScale[format->normalType] << 8);
        gqr7 |= gqr7 << 16;
    }
    asm {
        mtspr GQR6, gqr6
        mtspr GQR7, gqr7
    }

    kernel(count, platFlags, ptankGlobal);
}

/* The render callback of the tank pipeline: sets up GX for the tank's streams and runs its
 * particle kernel */
RpAtomic *fn_80448D04(RpAtomic *atomic, RpGameCubeRenderMesh *renderMesh) {
    GXColorSrc ambSrc;
    GXColorSrc matSrc;
    GXBool enable;
    RpGameCubeVertexFormat *format;
    RwUInt32 platFlags;
    RpPTankAtomicExtPrv *ptank;
    RwRGBA ambient;
    RpGameCubeMeshInstance *instance;
    RwTexture *texture;

    ptank = PTANKEXT(atomic);
    platFlags = ptank->platFlags;
    format = GAMECUBEVERTEXFORMAT(atomic->geometry);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, format->posType, format->posFrac);
    if (renderMesh->geometryFlags & rpGEOMETRYNORMALS) {
        GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, format->normalType, 0);
    }
    if (renderMesh->geometryFlags & rpGEOMETRYPRELIT) {
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, format->colorType, 0);
    }
    if (renderMesh->geometryFlags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, format->texCoordType[0],
                        format->texCoordFrac[0]);
    }
    fn_804950AC(NULL, (renderMesh->geometryFlags >> 4) & 1);

    /* RwRGBA and GXColor share their layout (four bytes, red first), so colors are passed as is */
    if (renderMesh->numLights > 0) {
        enable = GX_TRUE;
        if (platFlags & PTANKPLATFLAGCOLORSTREAMS) {
            ambSrc = GX_SRC_VTX;
        } else {
            ambSrc = GX_SRC_REG;
            if (platFlags & PTANKPLATFLAGCNSCOLOR) {
                GXSetChanAmbColor(GX_COLOR0A0, *(GXColor *)&ptank->cColor);
            } else if (renderMesh->hasAmbient) {
                RwRGBAFromRwRGBAReal(&ambient, &renderMesh->ambientColor);
                GXSetChanAmbColor(GX_COLOR0A0, *(GXColor *)&ambient);
            } else {
                GXSetChanAmbColor(GX_COLOR0A0, lbl_805FB07C);
            }
        }
        matSrc = GX_SRC_REG;
        GXSetChanMatColor(GX_COLOR0A0, lbl_805FB078);
    } else {
        enable = GX_FALSE;
        ambSrc = GX_SRC_REG;
        if (platFlags & PTANKPLATFLAGCNSCOLOR) {
            matSrc = GX_SRC_REG;
            GXSetChanMatColor(GX_COLOR0A0, *(GXColor *)&ptank->cColor);
        } else if (platFlags & PTANKPLATFLAGCOLORSTREAMS) {
            matSrc = GX_SRC_VTX;
        } else {
            matSrc = GX_SRC_REG;
            if (renderMesh->hasAmbient) {
                RwRGBAFromRwRGBAReal(&ambient, &renderMesh->ambientColor);
                GXSetChanMatColor(GX_COLOR0A0, *(GXColor *)&ambient);
            } else {
                GXSetChanMatColor(GX_COLOR0A0, lbl_805FB07C);
            }
        }
    }

    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0, enable, ambSrc, matSrc, renderMesh->lightMask, GX_DF_CLAMP,
                  GX_AF_SPOT);
    GXSetChanCtrl(GX_ALPHA0, enable, ambSrc, matSrc, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);
    GXSetNumTevStages(1);

    if (renderMesh->geometryFlags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
        GXSetNumTexGens(1);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    } else {
        GXSetNumTexGens(0);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    }

    instance = renderMesh->meshInstance;
    if (renderMesh->geometryFlags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) {
        texture = instance->material->texture;
        fn_80499C38(texture, 0);
        if (texture != NULL && texture->raster != NULL) {
            fn_80497778(
                (RWPLUGINOFFSET(RwGameCubeRasterExt, texture->raster->parent, lbl_805F1658)->flags &
                 1) ^
                1);
        } else {
            fn_80497778(TRUE);
        }
    } else {
        fn_80497778(TRUE);
    }

    PTankDrawParticles(atomic);
    return atomic;
}
