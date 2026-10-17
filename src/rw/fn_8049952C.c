/* RenderWare GameCube driver: the size, writing and reading of native textures in streams. The
 * original file name is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>
#include <dolphin/os.h>
#include <string.h>
#include "layout_check.h"

/* The texture part of a native texture's struct chunk (names inferred): the platform, the filter
 * mode and addressing, the GX LOD parameters and the names */
typedef struct RwGameCubeNativeTexture {
    /* 0x00 */ RwPlatformID platformID;
    /* 0x04 */ RwUInt32 filterAddressing;
    /* 0x08 */ RwInt32 maxAniso;
    /* 0x0C */ RwBool biasClamp;
    /* 0x10 */ RwBool edgeLOD;
    /* 0x14 */ RwReal lodBias;
    /* 0x18 */ RwChar name[rwTEXTUREBASENAMELENGTH];
    /* 0x38 */ RwChar mask[rwTEXTUREBASENAMELENGTH];
} RwGameCubeNativeTexture;
CHECK_SIZEOF(RwGameCubeNativeTexture, 0x58);

/* The raster part that follows it (names inferred): the raster create flags (type and format), the
 * size, depth and mip levels, the GX texture and TLUT formats and the raster plugin data flags.
 * The palette, the byte size of the texels and the texels follow. */
typedef struct RwGameCubeNativeRaster {
    /* 0x00 */ RwInt32 format;
    /* 0x04 */ RwUInt16 width;
    /* 0x06 */ RwUInt16 height;
    /* 0x08 */ RwUInt8 depth;
    /* 0x09 */ RwUInt8 numMipLevels;
    /* 0x0A */ RwUInt8 texFormat;
    /* 0x0B */ RwUInt8 tlutFormat;
    /* 0x0C */ RwUInt32 flags;
} RwGameCubeNativeRaster;
CHECK_SIZEOF(RwGameCubeNativeRaster, 0x10);

/* The bytes of a palettized raster's palette */
#define PALETTESIZE(raster) ((1 << (raster)->depth) * sizeof(RwUInt16))

/* Gets the stream size of a native texture: out is the size and inOut the texture. A texture
 * without a raster is counted with the texture part only. */
RwBool fn_8049952C(void *out, void *inOut, RwInt32 numIn) {
    RwInt32 *size = (RwInt32 *)out;
    RwRaster *raster = ((RwTexture *)inOut)->raster;
    RwInt32 total;

    if (raster == NULL) {
        *size = rwCHUNKHEADERSIZE + sizeof(RwGameCubeNativeTexture);
        return TRUE;
    }

    total = rwCHUNKHEADERSIZE + sizeof(RwGameCubeNativeTexture) + sizeof(RwGameCubeNativeRaster);
    if (RwRasterGetFormat(raster) & rwRASTERFORMATPALMASK) {
        total += PALETTESIZE(raster);
    }
    total += sizeof(RwUInt32);
    total += fn_80495558(raster);
    *size = total;
    return TRUE;
}

/* Writes a native texture to a stream: inOut is the texture */
RwBool fn_804995B4(void *stream, void *inOut, RwInt32 numIn) {
    RwTexture *texture = (RwTexture *)inOut;
    RwUInt32 texelsSize;
    RwInt32 size;
    RwGameCubeNativeRaster nativeRaster;
    RwGameCubeNativeTexture nativeTexture;
    RwGameCubeTextureExt *textureExt;
    RwRaster *raster;
    RwGameCubeRasterExt *rasterExt;
    RwUInt32 paletteSize;

    /* size counts down the bytes of the struct chunk left to write; it is updated after every
     * write but never checked */
    fn_8049952C(&size, texture, 0);
    size -= rwCHUNKHEADERSIZE;
    if (fn_8047CEF0(stream, rwID_STRUCT, size, RW_STREAM_VERSION, RW_STREAM_BUILD) == NULL) {
        return FALSE;
    }

    nativeTexture.platformID = rwID_GAMECUBE;
    textureExt = RWPLUGINOFFSET(RwGameCubeTextureExt, texture, lbl_805F1670);
    nativeTexture.filterAddressing =
        (RwTextureGetFilterMode(texture) & rwTEXTUREFILTERMODEMASK) |
        ((RwTextureGetAddressingU(texture) << 8) & rwTEXTUREADDRESSINGUMASK) |
        ((RwTextureGetAddressingV(texture) << 12) & rwTEXTUREADDRESSINGVMASK);
    if (textureExt->flags & rwGAMECUBETEXTURENOTBUILT) {
        nativeTexture.maxAniso = GX_ANISO_1;
        nativeTexture.biasClamp = TRUE;
        nativeTexture.edgeLOD = TRUE;
        nativeTexture.lodBias = 0.0f;
    } else {
        nativeTexture.maxAniso = GXGetTexObjMaxAniso(&textureExt->texObj);
        nativeTexture.biasClamp = GXGetTexObjBiasClamp(&textureExt->texObj);
        nativeTexture.edgeLOD = GXGetTexObjEdgeLOD(&textureExt->texObj);
        nativeTexture.lodBias = GXGetTexObjLODBias(&textureExt->texObj);
    }
    memcpy(nativeTexture.name, texture->name, rwTEXTUREBASENAMELENGTH);
    memcpy(nativeTexture.mask, texture->mask, rwTEXTUREBASENAMELENGTH);
    if (fn_80480ED0(stream, &nativeTexture, sizeof(nativeTexture)) == NULL) {
        return FALSE;
    }
    size -= sizeof(nativeTexture);

    raster = texture->raster;
    rasterExt = RWPLUGINOFFSET(RwGameCubeRasterExt, raster->parent, lbl_805F1658);
    nativeRaster.format = (raster->cFormat << 8) | raster->cType;
    nativeRaster.width = raster->width;
    nativeRaster.height = raster->height;
    nativeRaster.depth = raster->depth;
    nativeRaster.numMipLevels = fn_80489CE4(raster);
    nativeRaster.texFormat = rasterExt->texFormat;
    nativeRaster.tlutFormat = rasterExt->tlutFormat;
    nativeRaster.flags = rasterExt->flags & rwGAMECUBERASTERHASALPHA;
    if (fn_80480ED0(stream, &nativeRaster, sizeof(nativeRaster)) == NULL) {
        return FALSE;
    }
    size -= sizeof(nativeRaster);

    if (RwRasterGetFormat(raster) & rwRASTERFORMATPALMASK) {
        paletteSize = PALETTESIZE(raster);
        if (fn_80480ED0(stream, rasterExt->palette, paletteSize) == NULL) {
            return FALSE;
        }
        size -= paletteSize;
    }

    texelsSize = fn_80495558(raster);
    if (fn_80480ED0(stream, &texelsSize, sizeof(texelsSize)) == NULL) {
        return FALSE;
    }
    size -= sizeof(texelsSize);
    if (fn_80480ED0(stream, rasterExt->texels, texelsSize) == NULL) {
        return FALSE;
    }
    size -= texelsSize;
    return TRUE;
}

/* Reads a native texture from a stream; the texture is returned through inOut */
RwBool fn_8049988C(void *stream, void *inOut, RwInt32 numIn) {
    RwUInt32 length;
    RwUInt32 version;
    RwGameCubeNativeTexture nativeTexture;
    RwGameCubeNativeRaster nativeRaster;
    RwRaster *raster;
    RwGameCubeRasterExt *rasterExt;
    RwUInt32 paletteSize;
    RwUInt32 autoMipmap;
    RwUInt32 texelsSize;
    RwTexture *texture;

    if (!fn_8047CF50(stream, rwID_STRUCT, &length, &version)) {
        return FALSE;
    }
    if (version < rwLIBRARYBASEVERSION || version > RW_STREAM_VERSION) {
        return FALSE;
    }
    if (fn_80480D4C(stream, &nativeTexture, sizeof(nativeTexture)) != sizeof(nativeTexture)) {
        return FALSE;
    }
    if (nativeTexture.platformID != rwID_GAMECUBE) {
        return FALSE;
    }
    if (fn_80480D4C(stream, &nativeRaster, sizeof(nativeRaster)) != sizeof(nativeRaster)) {
        return FALSE;
    }

    /* The raster is created without pixels, which are allocated with its mip levels below */
    raster = fn_80489E74(nativeRaster.width, nativeRaster.height, nativeRaster.depth,
                         nativeRaster.format | rwRASTERDONTALLOCATE);
    if (raster == NULL) {
        return FALSE;
    }
    rasterExt = RWPLUGINOFFSET(RwGameCubeRasterExt, raster, lbl_805F1658);
    rasterExt->texFormat = nativeRaster.texFormat;
    rasterExt->tlutFormat = nativeRaster.tlutFormat;
    rasterExt->flags = nativeRaster.flags != 0;
    if (!fn_80496A20(raster, nativeRaster.numMipLevels)) {
        fn_80489BDC(raster);
        return FALSE;
    }
    raster->cFlags &= ~rwRASTERDONTALLOCATE;

    if (RwRasterGetFormat(raster) & rwRASTERFORMATPALMASK) {
        paletteSize = PALETTESIZE(raster);
        if (fn_80480D4C(stream, rasterExt->palette, paletteSize) != paletteSize) {
            return FALSE;
        }
        DCFlushRange(rasterExt->palette, PALETTESIZE(raster));
    }

    /* The texels are read as they are, so the raster must not regenerate its mip levels meanwhile */
    autoMipmap = raster->cFormat & (rwRASTERFORMATAUTOMIPMAP >> 8);
    raster->cFormat &= ~autoMipmap;
    if (fn_80480D4C(stream, &texelsSize, sizeof(texelsSize)) != sizeof(texelsSize)) {
        return FALSE;
    }
    if (fn_80480D4C(stream, rasterExt->texels, texelsSize) != texelsSize) {
        return FALSE;
    }
    DCFlushRange(rasterExt->texels, texelsSize);
    GXInvalidateTexAll();
    raster->cFormat |= autoMipmap;

    texture = fn_8048CCDC(raster);
    if (texture == NULL) {
        fn_80489BDC(raster);
        return FALSE;
    }
    /* the filter mode is the low byte */
    RwTextureSetFilterMode(texture, (RwUInt8)nativeTexture.filterAddressing);
    RwTextureSetAddressingU(texture,
                            (nativeTexture.filterAddressing & rwTEXTUREADDRESSINGUMASK) >> 8);
    RwTextureSetAddressingV(texture,
                            (nativeTexture.filterAddressing & rwTEXTUREADDRESSINGVMASK) >> 12);
    fn_8048CE54(texture, nativeTexture.name);
    fn_8048CF00(texture, nativeTexture.mask);
    fn_8049A518(texture, nativeTexture.lodBias, nativeTexture.biasClamp, nativeTexture.edgeLOD,
                nativeTexture.maxAniso);
    *(RwTexture **)inOut = texture;
    return TRUE;
}
