/* RenderWare GameCube driver: the texture plugin, which keeps a GX texture object for every texture,
 * setting textures on GX texture maps, and a texture's GX LOD parameters. The original file name is
 * unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>

/* The number of GX texture maps */
#define NUMTEXTUREMAPS 8

/* The GX minification and magnification filters of each RenderWare filter mode */
GXTexFilter lbl_8056EBD0[7][2] = {
    {GX_NEAR, GX_NEAR},           /* none */
    {GX_NEAR, GX_NEAR},           /* rwFILTERNEAREST */
    {GX_LINEAR, GX_LINEAR},       /* rwFILTERLINEAR */
    {GX_NEAR_MIP_NEAR, GX_NEAR},  /* rwFILTERMIPNEAREST */
    {GX_LIN_MIP_NEAR, GX_LINEAR}, /* rwFILTERMIPLINEAR */
    {GX_NEAR_MIP_LIN, GX_NEAR},   /* rwFILTERLINEARMIPNEAREST */
    {GX_LIN_MIP_LIN, GX_LINEAR},  /* rwFILTERLINEARMIPLINEAR */
};

/* The GX wrap mode of each RenderWare texture addressing mode */
GXTexWrapMode lbl_8056EC08[5] = {
    GX_CLAMP,  /* none */
    GX_REPEAT, /* rwTEXTUREADDRESSWRAP */
    GX_MIRROR, /* rwTEXTUREADDRESSMIRROR */
    GX_CLAMP,  /* rwTEXTUREADDRESSCLAMP */
    GX_CLAMP,  /* rwTEXTUREADDRESSBORDER */
};

/* The texture last set on each GX texture map */
RwTexture *lbl_8056EC1C[NUMTEXTUREMAPS] = {NULL};

/* The texture plugin's offset */
RwInt32 lbl_805F1670;

/* The texture plugin data of a texture */
#define TEXTUREEXT(texture) RWPLUGINOFFSET(RwGameCubeTextureExt, (texture), lbl_805F1670)

/* The texture plugin constructor: the texture object is built when the texture is first set */
void *fn_80499B24(void *object, RwInt32 offset, RwInt32 size) {
    TEXTUREEXT((RwTexture *)object)->flags = rwGAMECUBETEXTURENOTBUILT;
    return object;
}

/* The texture plugin destructor: forgets the texture on the texture maps it was set on */
void *fn_80499B38(void *object, RwInt32 offset, RwInt32 size) {
    RwInt32 i;

    for (i = 0; i < NUMTEXTUREMAPS; i++) {
        if (object == lbl_8056EC1C[i]) {
            lbl_8056EC1C[i] = NULL;
        }
    }
    return object;
}

/* Forgets the textures set on all texture maps */
void fn_80499BC8(void) {
    RwUInt32 i = NUMTEXTUREMAPS;

    while (i--) {
        lbl_8056EC1C[i] = NULL;
    }
}

/* Registers the texture plugin; returns its offset */
RwInt32 fn_80499BF8(void) {
    return lbl_805F1670 = fn_8048D280(sizeof(RwGameCubeTextureExt), rwID_DEVICEMODULE, fn_80499B24,
                                      fn_80499B38, NULL);
}

/* Builds a texture's GX texture object from its raster with the given LOD parameters, recording
 * the filter mode and addressing it was built with. tlut is the TLUT name of a palettized texture
 * that does not keep its own. */
static inline void BuildTexObj(RwTexture *texture, RwUInt32 tlut, RwReal lodBias, RwBool biasClamp,
                               RwBool edgeLOD, RwInt32 maxAniso) {
    RwRaster *raster = texture->raster;
    RwGameCubeTextureExt *textureExt = TEXTUREEXT(texture);
    RwGameCubeRasterExt *rasterExt =
        RWPLUGINOFFSET(RwGameCubeRasterExt, raster->parent, lbl_805F1658);
    RwInt32 format = RwRasterGetFormat(raster);
    RwInt32 filter;
    GXTexFilter minFilter;
    GXTexFilter magFilter;

    if (format & rwRASTERFORMATPALMASK) {
        GXInitTexObjCI(&textureExt->texObj, rasterExt->texels, raster->width, raster->height,
                       rasterExt->texFormat, lbl_8056EC08[RwTextureGetAddressingU(texture)],
                       lbl_8056EC08[RwTextureGetAddressingV(texture)],
                       (format & rwRASTERFORMATMIPMAP) ? GX_TRUE : GX_FALSE,
                       (textureExt->flags & rwGAMECUBETEXTUREPRELOADED)
                           ? GXGetTexObjTlut(&textureExt->texObj)
                           : tlut);

        /* palettized textures do not interpolate between mip levels: the filter modes that do use
         * the filters of rwFILTERMIPLINEAR */
        filter = RwTextureGetFilterMode(texture);
        if (filter == rwFILTERLINEARMIPLINEAR || filter == rwFILTERLINEARMIPNEAREST) {
            minFilter = lbl_8056EBD0[rwFILTERMIPLINEAR][0];
            magFilter = lbl_8056EBD0[rwFILTERMIPLINEAR][1];
        } else {
            minFilter = lbl_8056EBD0[filter][0];
            magFilter = lbl_8056EBD0[filter][1];
        }
    } else {
        GXInitTexObj(&textureExt->texObj, rasterExt->texels, raster->width, raster->height,
                     rasterExt->texFormat, lbl_8056EC08[RwTextureGetAddressingU(texture)],
                     lbl_8056EC08[RwTextureGetAddressingV(texture)],
                     (format & rwRASTERFORMATMIPMAP) ? GX_TRUE : GX_FALSE);
        minFilter = lbl_8056EBD0[RwTextureGetFilterMode(texture)][0];
        magFilter = lbl_8056EBD0[RwTextureGetFilterMode(texture)][1];
    }

    GXInitTexObjLOD(&textureExt->texObj, minFilter, magFilter, 0.0f, rasterExt->maxLevel, lodBias,
                    biasClamp, edgeLOD, maxAniso);
    textureExt->flags = ((textureExt->flags & ~rwGAMECUBETEXTUREFILTERADDRESSINGMASK) |
                         (texture->filterAddressing & rwGAMECUBETEXTUREFILTERADDRESSINGMASK)) &
                        ~rwGAMECUBETEXTURENOTBUILT;
}

/* Sets a texture on a GX texture map (stage); a NULL texture sets lbl_805F1664 with the white
 * raster. The texture object is built the first time and rebuilt when the texture's filter mode or
 * addressing changed, or when a palettized texture is set on another texture map, whose TLUT it
 * then uses; a rebuild keeps the texture object's LOD parameters. */
void fn_80499C38(RwTexture *texture, RwInt32 stage) {
    RwGameCubeTextureExt *textureExt;
    RwGameCubeRasterExt *rasterExt;
    RwRaster *raster;

    if (texture == NULL) {
        texture = lbl_805F1664;
        fn_8049714C(lbl_805F1664, lbl_805F1660, 0);
    }
    raster = texture->raster;
    textureExt = TEXTUREEXT(texture);
    rasterExt = RWPLUGINOFFSET(RwGameCubeRasterExt, raster->parent, lbl_805F1658);
    rasterExt->syncToken = lbl_805EDF40;

    if (RwRasterGetFormat(raster) & rwRASTERFORMATPALMASK) {
        if (textureExt->flags & rwGAMECUBETEXTURENOTBUILT) {
            BuildTexObj(texture, stage, 0.0f, TRUE, TRUE, GX_ANISO_1);
        } else if ((texture->filterAddressing & rwGAMECUBETEXTUREFILTERADDRESSINGMASK) !=
                   (textureExt->flags & rwGAMECUBETEXTUREFILTERADDRESSINGMASK)) {
            BuildTexObj(texture, stage, GXGetTexObjLODBias(&textureExt->texObj),
                        GXGetTexObjBiasClamp(&textureExt->texObj),
                        GXGetTexObjEdgeLOD(&textureExt->texObj),
                        GXGetTexObjMaxAniso(&textureExt->texObj));
        } else if (!(textureExt->flags & rwGAMECUBETEXTUREPRELOADED) &&
                   GXGetTexObjTlut(&textureExt->texObj) != stage) {
            BuildTexObj(texture, stage, GXGetTexObjLODBias(&textureExt->texObj),
                        GXGetTexObjBiasClamp(&textureExt->texObj),
                        GXGetTexObjEdgeLOD(&textureExt->texObj),
                        GXGetTexObjMaxAniso(&textureExt->texObj));
        }
        if (!(textureExt->flags & rwGAMECUBETEXTUREPRELOADED)) {
            GXLoadTlut(&rasterExt->tlutObj, stage);
        }
    } else if (textureExt->flags & rwGAMECUBETEXTURENOTBUILT) {
        BuildTexObj(texture, 0, 0.0f, TRUE, TRUE, GX_ANISO_1);
    } else if ((texture->filterAddressing & rwGAMECUBETEXTUREFILTERADDRESSINGMASK) !=
               (textureExt->flags & rwGAMECUBETEXTUREFILTERADDRESSINGMASK)) {
        BuildTexObj(texture, 0, GXGetTexObjLODBias(&textureExt->texObj),
                    GXGetTexObjBiasClamp(&textureExt->texObj),
                    GXGetTexObjEdgeLOD(&textureExt->texObj),
                    GXGetTexObjMaxAniso(&textureExt->texObj));
    }

    if (!(textureExt->flags & rwGAMECUBETEXTUREPRELOADED)) {
        GXLoadTexObj(&textureExt->texObj, stage);
    } else {
        GXLoadTexObjPreLoaded(&textureExt->texObj, rasterExt->texRegion, stage);
    }
    lbl_8056EC1C[stage] = texture;
}

/* Sets a texture's GX LOD parameters, building its texture object */
void fn_8049A518(RwTexture *texture, RwReal lodBias, RwBool biasClamp, RwBool edgeLOD,
                 RwInt32 maxAniso) {
    BuildTexObj(texture, 0, lodBias, biasClamp, edgeLOD, maxAniso);
}

/* Gets a texture's GX LOD parameters; a texture whose object is not built yet reports the ones it
 * will be built with */
void fn_8049A6E4(RwTexture *texture, RwReal *lodBias, RwBool *biasClamp, RwBool *edgeLOD,
                 RwInt32 *maxAniso) {
    RwGameCubeTextureExt *textureExt = TEXTUREEXT(texture);

    if (textureExt->flags & rwGAMECUBETEXTURENOTBUILT) {
        *lodBias = 0.0f;
        *biasClamp = TRUE;
        *edgeLOD = TRUE;
        *maxAniso = GX_ANISO_1;
    } else {
        *lodBias = GXGetTexObjLODBias(&textureExt->texObj);
        *biasClamp = GXGetTexObjBiasClamp(&textureExt->texObj);
        *edgeLOD = GXGetTexObjEdgeLOD(&textureExt->texObj);
        *maxAniso = GXGetTexObjMaxAniso(&textureExt->texObj);
    }
}
