/* RenderWare GameCube render state: setting a mesh's material colour through the GX channel or TEV
 * colours, and choosing the GX channels, TEV stages and material colour callback of a render mesh.
 * The original file name is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>

/* opaque white and opaque black */
static const GXColor lbl_805FB1D8 = {255, 255, 255, 255};
static const GXColor lbl_805FB1DC = {0, 0, 0, 255};

/* Material colour callbacks: they receive the mesh's ambient colour, the material colour and the
 * material's ambient coefficient */

/* the material colour in TEV register 0, and the material colour scaled by the ambient light in
 * TEV register 1 */
void fn_8046528C(RwRGBAReal *ambient, RwRGBA *color, RwReal ambientCoef) {
    GXColor ambientColor;

    ambientColor.r = color->red * (ambient->red * ambientCoef);
    ambientColor.g = color->green * (ambient->green * ambientCoef);
    ambientColor.b = color->blue * (ambient->blue * ambientCoef);
    ambientColor.a = 0;
    GXSetTevColor(GX_TEVREG0, *(GXColor *)color);
    GXSetTevColor(GX_TEVREG1, ambientColor);
}

/* the ambient light in TEV register 1 */
void fn_80465370(RwRGBAReal *ambient, RwRGBA *color, RwReal ambientCoef) {
    GXColor ambientColor;
    RwReal scale = 255.0f * ambientCoef;

    ambientColor.r = ambient->red * scale;
    ambientColor.g = ambient->green * scale;
    ambientColor.b = ambient->blue * scale;
    ambientColor.a = 0;
    GXSetTevColor(GX_TEVREG1, ambientColor);
}

/* the ambient light as the channel ambient colour */
void fn_804653F8(RwRGBAReal *ambient, RwRGBA *color, RwReal ambientCoef) {
    GXColor ambientColor;
    RwReal scale = 255.0f * ambientCoef;

    ambientColor.r = ambient->red * scale;
    ambientColor.g = ambient->green * scale;
    ambientColor.b = ambient->blue * scale;
    ambientColor.a = 0;
    GXSetChanAmbColor(GX_COLOR0, ambientColor);
}

/* the material colour as the channel material colour, and the ambient light as the channel
 * ambient colour */
void fn_80465480(RwRGBAReal *ambient, RwRGBA *color, RwReal ambientCoef) {
    GXColor ambientColor;
    RwReal scale = 255.0f * ambientCoef;

    ambientColor.r = ambient->red * scale;
    ambientColor.g = ambient->green * scale;
    ambientColor.b = ambient->blue * scale;
    ambientColor.a = 0;
    GXSetChanMatColor(GX_COLOR0A0, *(GXColor *)color);
    GXSetChanAmbColor(GX_COLOR0, ambientColor);
}

/* the material colour as the channel material colour */
void fn_8046551C(RwRGBAReal *ambient, RwRGBA *color, RwReal ambientCoef) {
    GXSetChanMatColor(GX_COLOR0A0, *(GXColor *)color);
}

/* the ambient light as the channel material colour */
void fn_8046554C(RwRGBAReal *ambient, RwRGBA *color, RwReal ambientCoef) {
    GXColor ambientColor;
    RwReal scale = 255.0f * ambientCoef;

    ambientColor.r = ambient->red * scale;
    ambientColor.g = ambient->green * scale;
    ambientColor.b = ambient->blue * scale;
    ambientColor.a = 0;
    GXSetChanMatColor(GX_COLOR0, ambientColor);
}

/* Sets up the GX colour channels and TEV stages for a mesh with the geometry flags, the hardware
 * lights in lightMask, whether it has ambient light and whether its prelit colours have alpha.
 * Returns the callback that sets each material's colour, or NULL. */
RpGameCubeMaterialColorCallBack fn_804655D4(RwInt32 geometryFlags, RwUInt32 lightMask,
                                            RwBool hasAmbient, RwBool prelitAlpha) {
    RwUInt32 textured = geometryFlags & (rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2);
    RpGameCubeMaterialColorCallBack setMaterialColor = NULL;
    GXBool colorLighting;
    GXBool alphaLighting;
    GXColorSrc colorMaterialSource;
    GXColorSrc alphaMaterialSource;
    GXColorSrc colorAmbientSource;
    GXColorSrc alphaAmbientSource;
    RwUInt8 numStages;

    if (textured) {
        RwUInt32 prelit = geometryFlags & rpGEOMETRYPRELIT;

        if (prelit && hasAmbient == TRUE) {
            if (lightMask != 0) {
                if (geometryFlags & rpGEOMETRYMODULATEMATERIALCOLOR) {
                    setMaterialColor = fn_8046528C;
                } else {
                    GXSetTevColor(GX_TEVREG0, lbl_805FB1D8);
                    setMaterialColor = fn_80465370;
                }
                colorMaterialSource = GX_SRC_REG;
                alphaMaterialSource = GX_SRC_REG;
                GXSetChanMatColor(GX_COLOR0A0, lbl_805FB1D8);
                colorLighting = GX_TRUE;
                colorAmbientSource = GX_SRC_VTX;
                if (prelitAlpha == TRUE) {
                    alphaAmbientSource = GX_SRC_VTX;
                    alphaLighting = GX_TRUE;
                } else {
                    alphaAmbientSource = GX_SRC_REG;
                    GXSetChanAmbColor(GX_ALPHA0, lbl_805FB1DC);
                    alphaLighting = GX_FALSE;
                }
            } else {
                if (geometryFlags & rpGEOMETRYMODULATEMATERIALCOLOR) {
                    setMaterialColor = fn_8046528C;
                } else {
                    GXSetTevColor(GX_TEVREG0, lbl_805FB1D8);
                    setMaterialColor = fn_80465370;
                }
                colorAmbientSource = GX_SRC_REG;
                alphaAmbientSource = GX_SRC_REG;
                colorMaterialSource = GX_SRC_VTX;
                if (prelitAlpha == TRUE) {
                    alphaMaterialSource = GX_SRC_VTX;
                } else {
                    alphaMaterialSource = GX_SRC_REG;
                    GXSetChanMatColor(GX_ALPHA0, lbl_805FB1DC);
                }
                colorLighting = GX_FALSE;
                alphaLighting = GX_FALSE;
            }
            /* the vertex colour lit by the light colour, plus the ambient light */
            numStages = 2;
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_C0, GX_CC_C1);
            GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_RASA, GX_CA_A0, GX_CA_A1);
        } else {
            if (lightMask != 0) {
                if (geometryFlags & rpGEOMETRYMODULATEMATERIALCOLOR) {
                    colorMaterialSource = GX_SRC_REG;
                    alphaMaterialSource = GX_SRC_REG;
                    colorLighting = GX_TRUE;
                    if (prelit) {
                        colorAmbientSource = GX_SRC_VTX;
                        if (prelitAlpha == TRUE) {
                            alphaAmbientSource = GX_SRC_VTX;
                            alphaLighting = GX_TRUE;
                        } else {
                            alphaAmbientSource = GX_SRC_REG;
                            GXSetChanAmbColor(GX_ALPHA0, lbl_805FB1DC);
                            alphaLighting = GX_FALSE;
                        }
                        setMaterialColor = fn_8046551C;
                    } else {
                        colorAmbientSource = GX_SRC_REG;
                        alphaAmbientSource = GX_SRC_REG;
                        alphaLighting = GX_FALSE;
                        GXSetChanAmbColor(GX_ALPHA0, lbl_805FB1DC);
                        setMaterialColor = fn_80465480;
                    }
                } else {
                    colorMaterialSource = GX_SRC_REG;
                    alphaMaterialSource = GX_SRC_REG;
                    GXSetChanMatColor(GX_COLOR0A0, lbl_805FB1D8);
                    colorLighting = GX_TRUE;
                    if (prelit) {
                        colorAmbientSource = GX_SRC_VTX;
                        if (prelitAlpha == TRUE) {
                            alphaAmbientSource = GX_SRC_VTX;
                            alphaLighting = GX_TRUE;
                        } else {
                            alphaAmbientSource = GX_SRC_REG;
                            alphaLighting = GX_FALSE;
                        }
                    } else {
                        colorAmbientSource = GX_SRC_REG;
                        alphaAmbientSource = GX_SRC_REG;
                        setMaterialColor = fn_804653F8;
                        alphaLighting = GX_FALSE;
                    }
                }
            } else if (geometryFlags & rpGEOMETRYMODULATEMATERIALCOLOR) {
                colorMaterialSource = GX_SRC_REG;
                alphaMaterialSource = GX_SRC_REG;
                colorLighting = GX_TRUE;
                if (prelit) {
                    colorAmbientSource = GX_SRC_VTX;
                    if (prelitAlpha == TRUE) {
                        alphaAmbientSource = GX_SRC_VTX;
                        alphaLighting = GX_TRUE;
                    } else {
                        alphaAmbientSource = GX_SRC_REG;
                        GXSetChanAmbColor(GX_ALPHA0, lbl_805FB1DC);
                        alphaLighting = GX_FALSE;
                    }
                    setMaterialColor = fn_8046551C;
                } else {
                    colorAmbientSource = GX_SRC_REG;
                    alphaAmbientSource = GX_SRC_REG;
                    alphaLighting = GX_FALSE;
                    GXSetChanAmbColor(GX_ALPHA0, lbl_805FB1DC);
                    setMaterialColor = fn_80465480;
                }
            } else {
                if (prelit) {
                    colorMaterialSource = GX_SRC_VTX;
                    if (prelitAlpha == TRUE) {
                        alphaMaterialSource = GX_SRC_VTX;
                    } else {
                        alphaMaterialSource = GX_SRC_REG;
                        GXSetChanMatColor(GX_ALPHA0, lbl_805FB1DC);
                    }
                } else {
                    colorMaterialSource = GX_SRC_REG;
                    alphaMaterialSource = GX_SRC_REG;
                    GXSetChanMatColor(GX_ALPHA0, lbl_805FB1DC);
                    setMaterialColor = fn_8046554C;
                }
                colorLighting = GX_FALSE;
                alphaLighting = GX_FALSE;
                colorAmbientSource = GX_SRC_REG;
                alphaAmbientSource = GX_SRC_REG;
            }
            /* the lit colour modulates the texture */
            numStages = 1;
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
            GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO);
        }
    } else {
        if (lightMask != 0) {
            colorMaterialSource = GX_SRC_REG;
            alphaMaterialSource = GX_SRC_REG;
            GXSetChanMatColor(GX_COLOR0A0, lbl_805FB1D8);
            colorLighting = GX_TRUE;
            if (geometryFlags & rpGEOMETRYPRELIT) {
                colorAmbientSource = GX_SRC_VTX;
                if (prelitAlpha == TRUE) {
                    alphaAmbientSource = GX_SRC_VTX;
                    alphaLighting = GX_TRUE;
                } else {
                    alphaAmbientSource = GX_SRC_REG;
                    alphaLighting = GX_FALSE;
                }
            } else {
                colorAmbientSource = GX_SRC_REG;
                alphaAmbientSource = GX_SRC_REG;
                alphaLighting = GX_FALSE;
                GXSetChanAmbColor(GX_COLOR0A0, lbl_805FB1DC);
            }
        } else {
            if (geometryFlags & rpGEOMETRYPRELIT) {
                colorMaterialSource = GX_SRC_VTX;
                if (prelitAlpha == TRUE) {
                    alphaMaterialSource = GX_SRC_VTX;
                } else {
                    alphaMaterialSource = GX_SRC_REG;
                    GXSetChanMatColor(GX_ALPHA0, lbl_805FB1DC);
                }
            } else {
                colorMaterialSource = GX_SRC_REG;
                alphaMaterialSource = GX_SRC_REG;
                GXSetChanMatColor(GX_COLOR0A0, lbl_805FB1DC);
            }
            colorAmbientSource = GX_SRC_REG;
            alphaAmbientSource = GX_SRC_REG;
            colorLighting = GX_FALSE;
            alphaLighting = GX_FALSE;
        }
        numStages = 1;
        if (geometryFlags & rpGEOMETRYMODULATEMATERIALCOLOR) {
            setMaterialColor = fn_8046528C;
        } else {
            GXSetTevColor(GX_TEVREG0, lbl_805FB1D8);
            setMaterialColor = fn_80465370;
        }
        /* the lit colour times the material colour, plus the ambient light */
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_C0, GX_CC_C1);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_RASA, GX_CA_A0, GX_CA_A1);
    }

    GXSetNumTevStages(numStages);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    if (numStages > 1) {
        /* a second stage modulates the texture */
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetNumTexGens(1);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    } else if (textured) {
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GXSetNumTexGens(1);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    } else {
        GXSetNumTexGens(0);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    }
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0, colorLighting, colorAmbientSource, colorMaterialSource, lightMask,
                  GX_DF_CLAMP, GX_AF_SPOT);
    GXSetChanCtrl(GX_ALPHA0, alphaLighting, alphaAmbientSource, alphaMaterialSource, 0, GX_DF_NONE,
                  GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_ALPHA1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    return setMaterialColor;
}
