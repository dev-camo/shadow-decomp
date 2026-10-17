/* RenderWare GameCube render states: the GX state set up when the device starts (including the 2D
 * immediate mode texture and a white raster), getting and setting render states through a cache of
 * the GX state, fog, blending and alpha testing. The original file name is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>
#include <string.h>

/* raster lock modes */
#define rwRASTERLOCKWRITE 0x1
#define rwRASTERLOCKRAW 0x8

/* The render state cache */
RwGameCubeRenderState lbl_805E3210;

/* The texture 2D immediate mode draws with and a white 4x4 raster (inferred use); declared in
 * reverse address order, as .sbss is laid out */
RwTexture *lbl_805F1664;
RwRaster *lbl_805F1660;

/* The GX fog type of each RenderWare fog type */
GXFogType lbl_8056E958[4] = {GX_FOG_NONE, GX_FOG_LIN, GX_FOG_EXP, GX_FOG_EXP2};

/* The GX blend factor of each RenderWare blend function */
GXBlendFactor lbl_8056E968[12] = {
    GX_BL_ZERO,        GX_BL_ZERO,     GX_BL_ONE,         GX_BL_SRCCLR,
    GX_BL_INVSRCCLR,   GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_BL_DSTALPHA,
    GX_BL_INVDSTALPHA, GX_BL_SRCCLR,   GX_BL_INVSRCCLR,   GX_BL_ZERO,
};

/* Finishes starting the device: the initial GX state and its cache, the 2D immediate mode texture
 * and a white raster */
void fn_80497198(void) {
    GXColor white = {255, 255, 255, 255};

    lbl_805E3210.fogEnable = FALSE;
    lbl_805E3210.fogType = rwFOGTYPELINEAR;
    lbl_805E3210.fogColor = 0;
    lbl_805E3210.fogColorGX.r = 0;
    lbl_805E3210.fogColorGX.g = 0;
    lbl_805E3210.fogColorGX.b = 0;
    lbl_805E3210.fogColorGX.a = 0;
    lbl_805E3210.fogStart = 5.0f;
    lbl_805E3210.fogEnd = 10.0f;
    lbl_805E3210.fogNear = 0.05f;
    lbl_805E3210.fogFar = 10.0f;
    lbl_805E3210.fogEndSet = FALSE;
    lbl_805E3210.zWriteEnable = TRUE;
    lbl_805E3210.zTestEnable = TRUE;
    lbl_805E3210.opaque = TRUE;
    lbl_805E3210.srcBlend = rwBLENDSRCALPHA;
    lbl_805E3210.destBlend = rwBLENDINVSRCALPHA;
    lbl_805E3210.cullMode = rwCULLMODECULLBACK;
    lbl_805E3210.zCompare = GX_LEQUAL;
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetZCompLoc(GX_TRUE);

    lbl_805E3210.alphaComp0 = GX_GREATER;
    lbl_805E3210.alphaRef0 = 0;
    lbl_805E3210.alphaOp = GX_AOP_AND;
    lbl_805E3210.alphaComp1 = GX_ALWAYS;
    lbl_805E3210.alphaRef1 = 0;
    lbl_805E3210.alphaCompareSet = FALSE;
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetCullMode(GX_CULL_FRONT);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

    GXSetChanCtrl(GX_ALPHA0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);
    GXSetChanCtrl(GX_ALPHA1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);
    GXSetChanMatColor(GX_ALPHA0, white);
    GXSetChanMatColor(GX_ALPHA1, white);
    GXSetChanMatColor(GX_COLOR0, white);
    GXSetChanMatColor(GX_COLOR1, white);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetCopyClear(white, 0xFFFFFF);
    GXSetCurrentMtx(GX_PNMTX0);
    fn_80499BC8();

    lbl_805F1664 = fn_8048CCDC(NULL);
    RwTextureSetFilterMode(lbl_805F1664, rwFILTERLINEAR);
    RwTextureSetAddressing(lbl_805F1664, rwTEXTUREADDRESSWRAP);
    lbl_805F1660 = fn_80489E74(4, 4, 16, rwRASTERTYPETEXTURE | rwRASTERFORMAT565);
    memset(fn_80489F70(lbl_805F1660, 0, rwRASTERLOCKWRITE | rwRASTERLOCKRAW), 0xFF, 32);
    fn_80489B40(lbl_805F1660);
}

/* Starts stopping the device: destroys the white raster and the 2D immediate mode texture */
void fn_8049742C(void) {
    fn_80489BDC(lbl_805F1660);
    lbl_805F1660 = NULL;
    fn_8048CA00(lbl_805F1664, NULL);
    fn_8048CD8C(lbl_805F1664);
    lbl_805F1664 = NULL;
}

/* Gets a render state */
RwBool fn_80497474(RwInt32 state, void *value) {
    switch (state) {
    case rwRENDERSTATEFOGENABLE:
        *(RwInt32 *)value = lbl_805E3210.fogEnable;
        return TRUE;
    case rwRENDERSTATEFOGTYPE:
        *(RwInt32 *)value = lbl_805E3210.fogType;
        return TRUE;
    case rwRENDERSTATEFOGCOLOR:
        *(RwUInt32 *)value = lbl_805E3210.fogColor;
        return TRUE;
    case rwRENDERSTATEFOGDENSITY:
        return FALSE;
    case rwRENDERSTATETEXTUREADDRESS:
        if (RwTextureGetAddressingU(lbl_805F1664) == RwTextureGetAddressingV(lbl_805F1664)) {
            *(RwInt32 *)value = RwTextureGetAddressingU(lbl_805F1664);
            return TRUE;
        }
        return FALSE;
    case rwRENDERSTATETEXTUREADDRESSU:
        *(RwInt32 *)value = RwTextureGetAddressingU(lbl_805F1664);
        return TRUE;
    case rwRENDERSTATETEXTUREADDRESSV:
        *(RwInt32 *)value = RwTextureGetAddressingV(lbl_805F1664);
        return TRUE;
    case rwRENDERSTATETEXTUREFILTER:
        *(RwInt32 *)value = RwTextureGetFilterMode(lbl_805F1664);
        return TRUE;
    case rwRENDERSTATETEXTURERASTER:
        *(RwRaster **)value = lbl_805F1664->raster;
        return TRUE;
    case rwRENDERSTATEZWRITEENABLE:
        *(RwInt32 *)value = lbl_805E3210.zWriteEnable;
        return TRUE;
    case rwRENDERSTATEZTESTENABLE:
        *(RwInt32 *)value = lbl_805E3210.zTestEnable;
        return TRUE;
    case rwRENDERSTATESRCBLEND:
        *(RwInt32 *)value = lbl_805E3210.srcBlend;
        return TRUE;
    case rwRENDERSTATEDESTBLEND:
        *(RwInt32 *)value = lbl_805E3210.destBlend;
        return TRUE;
    case rwRENDERSTATESHADEMODE:
        *(RwInt32 *)value = rwSHADEMODEGOURAUD;
        return TRUE;
    case rwRENDERSTATEBORDERCOLOR:
        return FALSE;
    case rwRENDERSTATETEXTUREPERSPECTIVE:
        *(RwInt32 *)value = TRUE;
        return TRUE;
    case rwRENDERSTATECULLMODE:
        *(RwInt32 *)value = lbl_805E3210.cullMode;
        return TRUE;
    case rwRENDERSTATEALPHATESTFUNCTION:
        /* Only a single comparison is a RenderWare alpha test function */
        if (lbl_805E3210.alphaOp == GX_AOP_AND && lbl_805E3210.alphaComp1 == GX_ALWAYS) {
            *(RwInt32 *)value = lbl_805E3210.alphaComp0 + 1;
        } else {
            *(RwInt32 *)value = 0;
        }
        return TRUE;
    case rwRENDERSTATEALPHATESTFUNCTIONREF:
        *(RwInt32 *)value = lbl_805E3210.alphaRef0;
        return TRUE;
    }
    return FALSE;
}

/* Enables or disables fog; fog takes its range from the current camera */
RwBool fn_80497668(RwBool enable) {
    if (enable) {
        if (!lbl_805E3210.fogEnable) {
            RwCamera *camera = lbl_805F15B4->curCamera;

            if (camera != NULL) {
                if (!lbl_805E3210.fogEndSet) {
                    lbl_805E3210.fogEnd = camera->farPlane;
                }
                lbl_805E3210.fogStart = camera->fogPlane;
                lbl_805E3210.fogNear = camera->nearPlane;
                lbl_805E3210.fogFar = camera->farPlane;
            }
            GXSetFog(lbl_8056E958[lbl_805E3210.fogType], lbl_805E3210.fogStart, lbl_805E3210.fogEnd,
                     lbl_805E3210.fogNear, lbl_805E3210.fogFar, lbl_805E3210.fogColorGX);
            lbl_805E3210.fogEnable = TRUE;
        }
    } else if (lbl_805E3210.fogEnable) {
        GXSetFog(GX_FOG_NONE, 5.0f, 10.0f, 0.05f, 10.0f, lbl_805E3210.fogColorGX);
        lbl_805E3210.fogEnable = FALSE;
    }
    return TRUE;
}

/* Turns alpha testing off for opaque drawing, or back to the cached alpha compare */
void fn_80497778(RwBool opaque) {
    if (lbl_805E3210.opaque != opaque) {
        if (opaque == TRUE) {
            GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
        } else {
            GXSetAlphaCompare(lbl_805E3210.alphaComp0, lbl_805E3210.alphaRef0, lbl_805E3210.alphaOp,
                              lbl_805E3210.alphaComp1, lbl_805E3210.alphaRef1);
        }
        GXSetZCompLoc(opaque);
        lbl_805E3210.opaque = opaque;
    }
}

/* Sets up the 2D immediate mode texture, testing alpha only when its raster has alpha */
void fn_80497804(void) {
    RwGameCubeRasterExt *rasterExt;

    if (lbl_805F1664->raster != NULL) {
        fn_80499C38(lbl_805F1664, 0);
        rasterExt = RWPLUGINOFFSET(RwGameCubeRasterExt, lbl_805F1664->raster, lbl_805F1658);

        /* bit 0 of the flags: the raster's format has alpha */
        fn_80497778((rasterExt->flags & 1) ? FALSE : TRUE);
    }
}

/* Sets the GX blend mode for RenderWare source and destination blend functions */
void fn_804978C0(RwInt32 srcBlend, RwInt32 destBlend) {
    GXSetBlendMode(GX_BM_BLEND, lbl_8056E968[srcBlend], lbl_8056E968[destBlend], GX_LO_CLEAR);
}

/* Sets the fog color from a packed ARGB color (inferred) */
static inline RwBool SetFogColor(RwUInt32 color) {
    if (color != lbl_805E3210.fogColor) {
        RwCamera *camera = lbl_805F15B4->curCamera;

        lbl_805E3210.fogColorGX.a = color >> 24;
        lbl_805E3210.fogColorGX.r = (color >> 16) & 0xFF;
        lbl_805E3210.fogColorGX.g = (color >> 8) & 0xFF;
        lbl_805E3210.fogColorGX.b = color;
        GXSetFog(lbl_8056E958[lbl_805E3210.fogType], camera->fogPlane, camera->farPlane,
                 camera->nearPlane, camera->farPlane, lbl_805E3210.fogColorGX);
        lbl_805E3210.fogColor = color;
    }
    return TRUE;
}

/* Sets the fog type; GX fog here is linear only (inferred) */
static inline RwBool SetFogType(RwInt32 type) {
    if (type != lbl_805E3210.fogType) {
        if (type != rwFOGTYPELINEAR) {
            return FALSE;
        } else {
            RwCamera *camera = lbl_805F15B4->curCamera;

            GXSetFog(lbl_8056E958[type], camera->fogPlane, camera->farPlane, camera->nearPlane,
                     camera->farPlane, lbl_805E3210.fogColorGX);
            lbl_805E3210.fogType = type;
        }
    }
    return TRUE;
}

/* Sets the 2D immediate mode texture's addressing; there is no border addressing (inferred) */
static inline RwBool SetTextureAddress(RwInt32 addressing) {
    if (addressing == rwTEXTUREADDRESSBORDER) {
        return FALSE;
    }
    RwTextureSetAddressing(lbl_805F1664, addressing);
    return TRUE;
}

/* Sets the 2D immediate mode texture's U addressing (inferred) */
static inline RwBool SetTextureAddressU(RwInt32 addressing) {
    if (addressing == rwTEXTUREADDRESSBORDER) {
        return FALSE;
    }
    RwTextureSetAddressingU(lbl_805F1664, addressing);
    return TRUE;
}

/* Sets the 2D immediate mode texture's V addressing (inferred) */
static inline RwBool SetTextureAddressV(RwInt32 addressing) {
    if (addressing == rwTEXTUREADDRESSBORDER) {
        return FALSE;
    }
    RwTextureSetAddressingV(lbl_805F1664, addressing);
    return TRUE;
}

/* Sets the source blend function; only those GX has as source factors (inferred) */
static inline RwBool SetSrcBlend(RwInt32 blend) {
    if (blend != lbl_805E3210.srcBlend) {
        switch (blend) {
        case rwBLENDZERO:
        case rwBLENDONE:
        case rwBLENDSRCALPHA:
        case rwBLENDINVSRCALPHA:
        case rwBLENDDESTALPHA:
        case rwBLENDINVDESTALPHA:
        case rwBLENDDESTCOLOR:
        case rwBLENDINVDESTCOLOR:
            GXSetBlendMode(GX_BM_BLEND, lbl_8056E968[blend], lbl_8056E968[lbl_805E3210.destBlend],
                           GX_LO_CLEAR);
            lbl_805E3210.srcBlend = blend;
            break;
        case rwBLENDSRCALPHASAT:
        default:
            return FALSE;
        }
    }
    return TRUE;
}

/* Sets the destination blend function; only those GX has as destination factors (inferred) */
static inline RwBool SetDestBlend(RwInt32 blend) {
    if (blend != lbl_805E3210.destBlend) {
        switch (blend) {
        case rwBLENDZERO:
        case rwBLENDONE:
        case rwBLENDSRCCOLOR:
        case rwBLENDINVSRCCOLOR:
        case rwBLENDSRCALPHA:
        case rwBLENDINVSRCALPHA:
        case rwBLENDDESTALPHA:
        case rwBLENDINVDESTALPHA:
            GXSetBlendMode(GX_BM_BLEND, lbl_8056E968[lbl_805E3210.srcBlend], lbl_8056E968[blend],
                           GX_LO_CLEAR);
            lbl_805E3210.destBlend = blend;
            break;
        case rwBLENDDESTCOLOR:
        case rwBLENDINVDESTCOLOR:
        default:
            return FALSE;
        }
    }
    return TRUE;
}

/* Sets a render state */
RwBool fn_80497904(RwInt32 state, void *value) {
    RwInt32 intValue = (RwInt32)value;
    RwBool result = FALSE;

    switch (state) {
    case rwRENDERSTATEFOGENABLE:
        fn_80497668(intValue);
        result = TRUE;
        break;
    case rwRENDERSTATEFOGCOLOR:
        result = SetFogColor(intValue);
        break;
    case rwRENDERSTATEFOGTYPE:
        result = SetFogType(intValue);
        break;
    case rwRENDERSTATEFOGDENSITY:
        result = FALSE;
        break;
    case rwRENDERSTATETEXTUREADDRESS:
        result = SetTextureAddress(intValue);
        break;
    case rwRENDERSTATETEXTUREADDRESSU:
        result = SetTextureAddressU(intValue);
        break;
    case rwRENDERSTATETEXTUREADDRESSV:
        result = SetTextureAddressV(intValue);
        break;
    case rwRENDERSTATETEXTUREFILTER:
        RwTextureSetFilterMode(lbl_805F1664, intValue);
        result = TRUE;
        break;
    case rwRENDERSTATETEXTURERASTER:
        if ((RwRaster *)intValue != lbl_805F1664->raster) {
            fn_8049714C(lbl_805F1664, (RwRaster *)intValue, 0);
        }
        result = TRUE;
        break;
    case rwRENDERSTATEZWRITEENABLE:
        if (intValue) {
            if (!lbl_805E3210.zWriteEnable) {
                GXSetZMode(GX_TRUE, lbl_805E3210.zCompare, GX_TRUE);
                lbl_805E3210.zWriteEnable = TRUE;
            }
        } else if (lbl_805E3210.zWriteEnable) {
            GXSetZMode(GX_TRUE, lbl_805E3210.zCompare, GX_FALSE);
            lbl_805E3210.zWriteEnable = FALSE;
        }
        result = TRUE;
        break;
    case rwRENDERSTATEZTESTENABLE:
        if (intValue) {
            if (!lbl_805E3210.zTestEnable) {
                GXSetZMode(GX_TRUE, GX_LEQUAL, lbl_805E3210.zWriteEnable);
                lbl_805E3210.zCompare = GX_LEQUAL;
                lbl_805E3210.zTestEnable = TRUE;
            }
        } else if (lbl_805E3210.zTestEnable) {
            GXSetZMode(GX_TRUE, GX_ALWAYS, lbl_805E3210.zWriteEnable);
            lbl_805E3210.zCompare = GX_ALWAYS;
            lbl_805E3210.zTestEnable = FALSE;
        }
        result = TRUE;
        break;
    case rwRENDERSTATESRCBLEND:
        result = SetSrcBlend(intValue);
        break;
    case rwRENDERSTATEDESTBLEND:
        result = SetDestBlend(intValue);
        break;
    case rwRENDERSTATESHADEMODE:
        result = intValue == rwSHADEMODEGOURAUD;
        break;
    case rwRENDERSTATEBORDERCOLOR:
        result = FALSE;
        break;
    case rwRENDERSTATETEXTUREPERSPECTIVE:
        result = intValue;
        break;
    case rwRENDERSTATECULLMODE:
        if (intValue != lbl_805E3210.cullMode) {
            GXSetCullMode(intValue - 1);
            lbl_805E3210.cullMode = intValue;
        }
        result = TRUE;
        break;
    case rwRENDERSTATEALPHATESTFUNCTION:
        if (lbl_805E3210.alphaCompareSet) {
            /* Back to a single comparison */
            if (lbl_805E3210.opaque != TRUE) {
                GXSetAlphaCompare(intValue - 1, lbl_805E3210.alphaRef0, GX_AOP_AND, GX_ALWAYS, 0);
            }
            lbl_805E3210.alphaComp0 = intValue - 1;
            lbl_805E3210.alphaOp = GX_AOP_AND;
            lbl_805E3210.alphaComp1 = GX_ALWAYS;
            lbl_805E3210.alphaRef1 = 0;
        } else {
            RwInt32 compare = intValue - 1;

            if (compare != lbl_805E3210.alphaComp0) {
                if (lbl_805E3210.opaque != TRUE) {
                    GXSetAlphaCompare(compare, lbl_805E3210.alphaRef0, lbl_805E3210.alphaOp,
                                      lbl_805E3210.alphaComp1, lbl_805E3210.alphaRef1);
                }
                lbl_805E3210.alphaComp0 = compare;
            }
        }
        result = TRUE;
        break;
    case rwRENDERSTATEALPHATESTFUNCTIONREF:
        if (intValue != lbl_805E3210.alphaRef0) {
            if (lbl_805E3210.opaque != TRUE) {
                GXSetAlphaCompare(lbl_805E3210.alphaComp0, intValue, lbl_805E3210.alphaOp,
                                  lbl_805E3210.alphaComp1, lbl_805E3210.alphaRef1);
            }
            lbl_805E3210.alphaRef0 = intValue;
        }
        result = TRUE;
        break;
    }
    return result;
}

/* Sets the alpha compare directly; the GX state keeps the previous compare until the next change
 * (as in the original) */
void fn_80497F24(RwInt32 comp0, RwUInt8 ref0, RwInt32 op, RwInt32 comp1, RwUInt8 ref1) {
    if (lbl_805E3210.opaque != TRUE) {
        GXSetAlphaCompare(lbl_805E3210.alphaComp0, lbl_805E3210.alphaRef0, lbl_805E3210.alphaOp,
                          lbl_805E3210.alphaComp1, lbl_805E3210.alphaRef1);
    }
    lbl_805E3210.alphaComp0 = comp0;
    lbl_805E3210.alphaRef0 = ref0;
    lbl_805E3210.alphaOp = op;
    lbl_805E3210.alphaComp1 = comp1;
    lbl_805E3210.alphaRef1 = ref1;
    lbl_805E3210.alphaCompareSet = TRUE;
}
