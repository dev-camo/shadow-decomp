/* RenderWare material effects plugin (plugin ID 0x120): the engine plugin (the material data free
 * list and the platform pipelines), the material, atomic and world sector plugin data and their
 * streams, the bumped texture made for a bump map, the plugin attachment, and the effect setters
 * and getters. The original file name is unknown. */
#include "rw/rpmatfx.h"
#include <string.h>

#define rwMEMHINT_MATFX (rwMEMHINTDUR_EVENT | rpID_MATFXPLUGIN)

/* The addressing in a texture's filterAddressing word when both directions share one, else 0
 * (name inferred) */
static inline RwUInt32 MatFXTextureAddressing(RwUInt32 filterAddressing) {
    RwUInt32 addressing = 0;

    if (((filterAddressing & rwTEXTUREADDRESSINGUMASK) >> 8) ==
        ((filterAddressing & rwTEXTUREADDRESSINGVMASK) >> 12)) {
        addressing = (filterAddressing & rwTEXTUREADDRESSINGVMASK) >> 12;
    }
    return addressing;
}

/* The plugin's globals (inferred) */
typedef struct rpMatFXGlobals {
    /* 0x0 */ RwModuleInfo module;
    /* 0x8 */ RwFreeList *freeList; /* of rpMatFXMaterialData */
} rpMatFXGlobals;
CHECK_SIZEOF(rpMatFXGlobals, 0xC);

rpMatFXGlobals lbl_8056ED00 = {{0, 0}, NULL};
RwInt32 lbl_805EDF60 = 128; /* material data free list entries per block */
RwInt32 lbl_805EDF64 = 1;   /* material data free list blocks to preallocate */
RwFreeList lbl_805E32A0;    /* the material data free list */

/* declared in reverse address order (the compiler places them in .sbss last first) */
RwInt32 lbl_805F16B0;
RwInt32 lbl_805F16AC;
RwInt32 lbl_805F16A8;

/* The slot of a material's effects that has an effect, or NULL (name inferred) */
static inline MatFXEffectData *MatFXMaterialDataGetSlot(rpMatFXMaterialData *data, RwInt32 flag) {
    RwUInt8 i;

    for (i = 0; i < rpMATFXNUMSLOTS; i++) {
        if (data->data[i].flag == flag) {
            return &data->data[i];
        }
    }
    return NULL;
}

/* A material's effects, made empty when it has none, or NULL when they cannot be allocated
 * (name inferred) */
static inline rpMatFXMaterialData *MatFXMaterialDataCreate(RpMaterial *material) {
    rpMatFXMaterialData *data = *MATFXMATERIALGETDATA(material);

    if (data == NULL) {
        data = RwFreeListAlloc(lbl_8056ED00.freeList, rwMEMHINT_MATFX);
        if (data == NULL) {
            return NULL;
        }
        memset(data, 0, sizeof(rpMatFXMaterialData));
        *MATFXMATERIALGETDATA(material) = data;
    }
    return data;
}

/* Releases the textures of a material's effects and clears them (name inferred) */
static inline void MatFXMaterialDataClean(rpMatFXMaterialData *data) {
    MatFXEffectData *effect;
    RwUInt8 i;

    for (i = 0; i < rpMATFXNUMSLOTS; i++) {
        effect = &data->data[i];
        switch (effect->flag) {
        case rpMATFXEFFECTBUMPMAP:
            if (effect->data.bumpMap.bumpedTexture != NULL) {
                fn_8048CD8C(effect->data.bumpMap.bumpedTexture);
            }
            if (effect->data.bumpMap.texture != NULL) {
                fn_8048CD8C(effect->data.bumpMap.texture);
            }
            break;
        case rpMATFXEFFECTENVMAP:
            if (effect->data.envMap.texture != NULL) {
                fn_8048CD8C(effect->data.envMap.texture);
            }
            break;
        case rpMATFXEFFECTDUAL:
            if (effect->data.dual.texture != NULL) {
                fn_8048CD8C(effect->data.dual.texture);
            }
            break;
        case rpMATFXEFFECTUVTRANSFORM:
            effect->data.uvAnim.baseTransform = NULL;
            effect->data.uvAnim.dualTransform = NULL;
            break;
        }
    }
    memset(data, 0, sizeof(rpMatFXMaterialData));
}

/* Writes the name of the bumped texture made from a base texture (or none) and a bump map texture:
 * the characters of the two textures' names interleaved, up to 30 of them (name inferred) */
static inline void MatFXBumpedTextureName(RwChar *name, const RwTexture *baseTexture,
                                          const RwTexture *bumpTexture) {
    const RwChar *names[2];
    RwInt32 count = 0;
    RwChar *out = name;
    RwInt32 j;

    if (baseTexture != NULL) {
        names[0] = baseTexture->name;
        names[1] = bumpTexture->name;
    } else {
        names[0] = names[1] = bumpTexture->name;
    }
    while (count < rwTEXTUREBASENAMELENGTH - 2 && (*names[0] != '\0' || *names[1] != '\0')) {
        for (j = 0; j < 2; j++) {
            if (*names[j] != '\0') {
                *out++ = *names[j]++;
                count++;
            }
        }
    }
    *out = '\0';
}

void *fn_804A076C(void *instance, RwInt32 offset, RwInt32 size) {
    if (--lbl_8056ED00.module.numInstances == 0) {
        fn_804A5524();
    }
    if (lbl_8056ED00.freeList != NULL) {
        fn_8047FCDC(lbl_8056ED00.freeList);
        lbl_8056ED00.freeList = NULL;
    }
    return instance;
}

void *fn_804A07DC(void *instance, RwInt32 offset, RwInt32 size) {
    if (lbl_8056ED00.module.numInstances == 0) {
        lbl_8056ED00.freeList =
            fn_8047FC04(sizeof(rpMatFXMaterialData), lbl_805EDF60, 4, lbl_805EDF64, &lbl_805E32A0,
                        rwMEMHINTDUR_GLOBAL | rpID_MATFXPLUGIN);
        if (lbl_8056ED00.freeList == NULL) {
            return NULL;
        }
        if (!fn_804A5494()) {
            return NULL;
        }
    }
    lbl_8056ED00.module.numInstances++;
    return instance;
}

void *fn_804A0884(void *object, RwInt32 offset, RwInt32 size) {
    *MATFXMATERIALGETDATA(object) = NULL;
    return object;
}

void *fn_804A0894(void *object, RwInt32 offset, RwInt32 size) {
    rpMatFXMaterialData *data = *MATFXMATERIALGETDATA(object);

    if (data != NULL) {
        MatFXMaterialDataClean(data);
        RwFreeListFree(lbl_8056ED00.freeList, data);
        *MATFXMATERIALGETDATA(object) = NULL;
    }
    return object;
}

void *fn_804A09D0(void *dstObject, const void *srcObject, RwInt32 offset, RwInt32 size) {
    const RpMaterial *srcMaterial;
    rpMatFXMaterialData *srcData;
    RpMaterial *dstMaterial;
    MatFXEffectData *srcEffect;
    RwUInt8 i;
    rpMatFXMaterialData *dstData;

    srcMaterial = srcObject;
    dstMaterial = dstObject;
    srcData = *MATFXMATERIALGETDATA(srcMaterial);

    if (srcData == NULL) {
        return NULL;
    }
    dstData = MatFXMaterialDataCreate(dstMaterial);
    if (dstData == NULL) {
        return NULL;
    }

    fn_804A20D0(dstMaterial, srcData->flags);
    for (i = 0; i < rpMATFXNUMSLOTS; i++) {
        srcEffect = &srcData->data[i];
        switch (srcEffect->flag) {
        case rpMATFXEFFECTBUMPMAP: {
            MatFXEffectData *dstEffect = &dstData->data[i];
            RwFrame *frame = fn_804A26BC(srcMaterial);
            RwReal coef = fn_804A2704(srcMaterial);

            fn_804A25E0(dstMaterial, frame);
            fn_804A2628(dstMaterial, coef);
            dstEffect->data.bumpMap.bumpedTexture = srcEffect->data.bumpMap.bumpedTexture;
            dstEffect->data.bumpMap.texture = srcEffect->data.bumpMap.texture;
            if (dstEffect->data.bumpMap.bumpedTexture != NULL) {
                dstEffect->data.bumpMap.bumpedTexture->refCount++;
            }
            if (dstEffect->data.bumpMap.texture != NULL) {
                dstEffect->data.bumpMap.texture->refCount++;
            }
            break;
        }
        case rpMATFXEFFECTENVMAP: {
            RwTexture *texture = fn_804A28D0(srcMaterial);
            RwFrame *frame = fn_804A2918(srcMaterial);
            RwReal coef = fn_804A29A8(srcMaterial);
            RwBool useFrameBufferAlpha = fn_804A2960(srcMaterial);

            if (texture != NULL) {
                fn_804A2750(dstMaterial, texture);
            }
            fn_804A27F8(dstMaterial, frame);
            fn_804A2840(dstMaterial, useFrameBufferAlpha);
            fn_804A2888(dstMaterial, coef);
            break;
        }
        case rpMATFXEFFECTDUAL: {
            RwTexture *texture = fn_804A2B44(srcMaterial);
            RwInt32 srcBlendMode;
            RwInt32 dstBlendMode;

            fn_804A2B8C(srcMaterial, &srcBlendMode, &dstBlendMode);
            if (texture != NULL) {
                fn_804A29F0(dstMaterial, texture);
            }
            fn_804A2AB0(dstMaterial, srcBlendMode, dstBlendMode);
            break;
        }
        case rpMATFXEFFECTUVTRANSFORM: {
            RwMatrix *baseTransform;
            RwMatrix *dualTransform;

            /* the returned materials replace both pointers for the following slots */
            srcMaterial = fn_804A2C2C(srcMaterial, &baseTransform, &dualTransform);
            dstMaterial = fn_804A2BE0(dstMaterial, baseTransform, dualTransform);
            break;
        }
        }
    }
    return dstObject;
}

RwStream *fn_804A0C50(RwStream *stream, const RwTexture *texture) {
    RwBool hasTexture = texture != NULL;

    if (fn_8047D458(stream, &hasTexture, sizeof(hasTexture)) == NULL) {
        return NULL;
    }
    if (hasTexture) {
        if (fn_80483BA0(texture, stream) == NULL) {
            return NULL;
        }
    }
    return stream;
}

RwStream *fn_804A0CDC(RwStream *stream, RwTexture **texture) {
    RwBool hasTexture;
    RwError error;

    if (fn_8047D868(stream, &hasTexture, sizeof(hasTexture)) == NULL) {
        return NULL;
    }
    if (hasTexture) {
        if (!fn_8047CF50(stream, rwID_TEXTURE, NULL, NULL)) {
            return NULL;
        }

        /* a texture that cannot be read is no error, unless reading it reported one other than a
         * missing mask */
        fn_8047DBE4(&error);
        *texture = fn_80483D28(stream);
        if (*texture == NULL) {
            fn_8047DBE4(&error);
            if (error.errorCode != E_RW_NOERROR && error.errorCode != E_RW_READTEXMASK) {
                fn_8047DB84(&error);
                return NULL;
            }
        }
    } else {
        *texture = NULL;
    }
    return stream;
}

RwUInt32 fn_804A0DB8(const RwTexture *texture) {
    RwUInt32 size = sizeof(RwBool);

    if (texture != NULL) {
        size = fn_80483B38(texture) + rwCHUNKHEADERSIZE + sizeof(RwBool);
    }
    return size;
}

RwStream *fn_804A0DEC(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {
    const rpMatFXMaterialData *data = *MATFXMATERIALGETDATA(object);
    RwInt32 value;
    RwInt32 flag;
    RwReal coef;
    RwUInt8 i;

    value = data->flags;
    if (fn_8047D458(stream, &value, sizeof(value)) == NULL) {
        return NULL;
    }

    for (i = 0; i < rpMATFXNUMSLOTS; i++) {
        const MatFXEffectData *effect = &data->data[i];

        flag = effect->flag;
        if (fn_8047D458(stream, &flag, sizeof(flag)) == NULL) {
            return NULL;
        }

        switch (flag) {
        case rpMATFXEFFECTBUMPMAP:
            coef = -effect->data.bumpMap.coef;
            if (fn_8047D350(stream, &coef, sizeof(coef)) == NULL) {
                return NULL;
            }
            if (fn_804A0C50(stream, effect->data.bumpMap.bumpedTexture) == NULL) {
                return NULL;
            }
            if (fn_804A0C50(stream, effect->data.bumpMap.texture) == NULL) {
                return NULL;
            }
            break;
        case rpMATFXEFFECTENVMAP:
            if (fn_8047D350(stream, &effect->data.envMap.coef, sizeof(RwReal)) == NULL) {
                return NULL;
            }
            value = effect->data.envMap.useFrameBufferAlpha;
            if (fn_8047D458(stream, &value, sizeof(value)) == NULL) {
                return NULL;
            }
            if (fn_804A0C50(stream, effect->data.envMap.texture) == NULL) {
                return NULL;
            }
            break;
        case rpMATFXEFFECTDUAL:
            value = effect->data.dual.srcBlendMode;
            if (fn_8047D458(stream, &value, sizeof(value)) == NULL) {
                return NULL;
            }
            value = effect->data.dual.dstBlendMode;
            if (fn_8047D458(stream, &value, sizeof(value)) == NULL) {
                return NULL;
            }
            if (fn_804A0C50(stream, effect->data.dual.texture) == NULL) {
                return NULL;
            }
            break;
        case rpMATFXEFFECTUVTRANSFORM:
            break;
        }
    }
    return stream;
}

RwStream *fn_804A114C(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    RpMaterial *material;
    rpMatFXMaterialData *data;
    RwInt32 flags;
    RwInt32 flag;
    RwUInt8 i;

    material = object;
    data = MatFXMaterialDataCreate(material);

    if (data == NULL) {
        return NULL;
    }
    if (fn_8047D868(stream, &flags, sizeof(flags)) == NULL) {
        return NULL;
    }
    fn_804A20D0(material, flags);

    for (i = 0; i < rpMATFXNUMSLOTS; i++) {
        if (fn_8047D868(stream, &flag, sizeof(flag)) == NULL) {
            return NULL;
        }

        switch (flag) {
        case rpMATFXEFFECTBUMPMAP: {
            RwTexture *bumpedTexture = NULL;
            RwTexture *texture = NULL;
            MatFXEffectData *effect = &data->data[i];
            RwReal coef;

            if (fn_8047D784(stream, &coef, sizeof(coef)) == NULL) {
                return NULL;
            }
            if (fn_804A0CDC(stream, &bumpedTexture) == NULL) {
                return NULL;
            }
            if (fn_804A0CDC(stream, &texture) == NULL) {
                if (bumpedTexture != NULL) {
                    fn_8048CD8C(bumpedTexture);
                }
                return NULL;
            }

            /* without the bumped texture, it is made again from the texture */
            if (bumpedTexture != NULL) {
                effect->data.bumpMap.bumpedTexture = bumpedTexture;
                effect->data.bumpMap.texture = texture;
                effect->data.bumpMap.invBumpWidth =
                    1.0f / effect->data.bumpMap.bumpedTexture->raster->width;
            } else if (texture != NULL) {
                fn_804A2304(material, texture);
                fn_8048CD8C(texture);
            } else {
                effect->data.bumpMap.bumpedTexture = NULL;
                effect->data.bumpMap.texture = NULL;
            }
            fn_804A2628(material, coef);
            break;
        }
        case rpMATFXEFFECTENVMAP: {
            RwTexture *texture = NULL;
            RwReal coef;
            RwBool useFrameBufferAlpha;

            if (fn_8047D784(stream, &coef, sizeof(coef)) == NULL) {
                return NULL;
            }
            if (fn_8047D868(stream, &useFrameBufferAlpha, sizeof(useFrameBufferAlpha)) == NULL) {
                return NULL;
            }
            if (fn_804A0CDC(stream, &texture) == NULL) {
                return NULL;
            }
            if (texture != NULL) {
                fn_804A2750(material, texture);
                fn_8048CD8C(texture);
            }
            fn_804A2888(material, coef);
            fn_804A2840(material, useFrameBufferAlpha);
            break;
        }
        case rpMATFXEFFECTDUAL: {
            RwTexture *texture = NULL;
            RwInt32 blendModes[2]; /* source, destination */

            if (fn_8047D868(stream, blendModes, sizeof(blendModes)) == NULL) {
                return NULL;
            }
            if (fn_804A0CDC(stream, &texture) == NULL) {
                return NULL;
            }
            if (texture != NULL) {
                fn_804A29F0(material, texture);
                fn_8048CD8C(texture);
            }
            fn_804A2AB0(material, blendModes[0], blendModes[1]);
            break;
        }
        case rpMATFXEFFECTUVTRANSFORM:
            break;
        }
    }
    return stream;
}

RwInt32 fn_804A16D4(const void *object, RwInt32 offset, RwInt32 size) {
    const MatFXEffectData *effect;
    const rpMatFXMaterialData *data;
    RwInt32 streamSize;
    RwUInt8 i;

    data = *MATFXMATERIALGETDATA(object);
    if (data == NULL || data->flags == rpMATFXEFFECTNULL) {
        return 0;
    }

    streamSize = sizeof(RwInt32);
    for (i = 0; i < rpMATFXNUMSLOTS; i++) {
        effect = &data->data[i];
        streamSize += sizeof(RwInt32);
        switch (effect->flag) {
        case rpMATFXEFFECTBUMPMAP:
            streamSize += sizeof(RwReal);
            streamSize += fn_804A0DB8(effect->data.bumpMap.bumpedTexture);
            streamSize += fn_804A0DB8(effect->data.bumpMap.texture);
            break;
        case rpMATFXEFFECTENVMAP:
            streamSize += sizeof(RwReal) + sizeof(RwBool);
            streamSize += fn_804A0DB8(effect->data.envMap.texture);
            break;
        case rpMATFXEFFECTDUAL:
            streamSize += sizeof(RwInt32) + sizeof(RwInt32);
            streamSize += fn_804A0DB8(effect->data.dual.texture);
            break;
        case rpMATFXEFFECTUVTRANSFORM:
            break;
        }
    }
    return streamSize;
}

void *fn_804A1818(void *object, RwInt32 offset, RwInt32 size) {
    *MATFXATOMICGETDATA(object) = FALSE;
    return object;
}

void *fn_804A1828(void *object, RwInt32 offset, RwInt32 size) {
    *MATFXATOMICGETDATA(object) = FALSE;
    return object;
}

void *fn_804A1838(void *dstObject, const void *srcObject, RwInt32 offset, RwInt32 size) {
    if (*MATFXATOMICGETDATA(srcObject)) {
        *MATFXATOMICGETDATA(dstObject) = TRUE;
    }
    return dstObject;
}

RwStream *fn_804A1854(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {
    RwBool enabled = *MATFXATOMICGETDATA(object);

    return fn_8047D458(stream, &enabled, sizeof(enabled));
}

RwStream *fn_804A1888(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    RwBool enabled;

    if (fn_8047D868(stream, &enabled, sizeof(enabled)) == NULL) {
        return NULL;
    }
    if (enabled) {
        fn_804A2004(object);
    }
    return stream;
}

RwInt32 fn_804A18F0(const void *object, RwInt32 offset, RwInt32 size) {
    return *MATFXATOMICGETDATA(object) == FALSE ? 0 : sizeof(RwBool);
}

void *fn_804A1910(void *object, RwInt32 offset, RwInt32 size) {
    *MATFXWORLDSECTORGETDATA(object) = FALSE;
    return object;
}

void *fn_804A1920(void *object, RwInt32 offset, RwInt32 size) {
    *MATFXWORLDSECTORGETDATA(object) = FALSE;
    return object;
}

void *fn_804A1930(void *dstObject, const void *srcObject, RwInt32 offset, RwInt32 size) {
    if (*MATFXWORLDSECTORGETDATA(srcObject)) {
        *MATFXWORLDSECTORGETDATA(dstObject) = TRUE;
    }
    return dstObject;
}

RwStream *fn_804A194C(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {
    RwBool enabled = *MATFXWORLDSECTORGETDATA(object);

    return fn_8047D458(stream, &enabled, sizeof(enabled));
}

RwStream *fn_804A1980(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    RwBool enabled;

    if (fn_8047D868(stream, &enabled, sizeof(enabled)) == NULL) {
        return NULL;
    }
    if (enabled) {
        fn_804A2070(object);
    }
    return stream;
}

RwInt32 fn_804A19E8(const void *object, RwInt32 offset, RwInt32 size) {
    return *MATFXWORLDSECTORGETDATA(object) == FALSE ? 0 : sizeof(RwBool);
}

/* Makes the texture rendered for a bump map: the base texture (white without one) with the bump
 * map's brightness, resampled to the base texture's size, as its alpha. It takes the base
 * texture's filtering and addressing and both textures' names. */
RwTexture *fn_804A1A08(RwTexture *baseTexture, RwTexture *bumpTexture) {
    RwRaster *baseRaster;
    RwRaster *bumpRaster;
    RwInt32 bumpWidth;
    RwInt32 bumpHeight;
    RwInt32 width;
    RwInt32 height;
    RwImage *bumpImage;
    RwImage *image;
    RwImage *resampled;
    RwInt32 rasterWidth;
    RwInt32 rasterHeight;
    RwInt32 rasterDepth;
    RwInt32 rasterFormat;
    RwRaster *raster;
    RwTexture *texture;
    RwUInt32 addressing;
    RwUInt32 filtering;

    bumpRaster = bumpTexture->raster;
    baseRaster = NULL;
    bumpWidth = bumpRaster->width;
    bumpHeight = bumpRaster->height;
    bumpImage = fn_80487D08(bumpWidth, bumpHeight, 32);
    fn_80487E18(bumpImage);
    fn_804899A8(bumpImage, bumpRaster);

    if (baseTexture != NULL) {
        baseRaster = baseTexture->raster;
        width = baseRaster->width;
        height = baseRaster->height;
        image = fn_80487D08(width, height, 32);
        fn_80487E18(image);
        fn_804899A8(image, baseRaster);
    } else {
        RwInt32 x;
        RwInt32 y;

        width = bumpRaster->width;
        height = bumpRaster->height;
        image = fn_80487D08(width, height, 32);
        fn_80487E18(image);
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                *(RwUInt32 *)(image->cpPixels + image->stride * y + (x << 2)) = 0xFFFFFFFF;
            }
        }
    }

    if (width != bumpWidth || height != bumpHeight) {
        resampled = fn_80487D08(width, height, 32);
        fn_80487E18(resampled);
        fn_8048A790(resampled, bumpImage);
        fn_80487DB0(bumpImage);
        bumpImage = resampled;
    }
    fn_80487FA8(bumpImage);
    fn_80488088(image, bumpImage);

    fn_80489A90(image, rwRASTERTYPETEXTURE, &rasterWidth, &rasterHeight, &rasterDepth,
                &rasterFormat);
    if ((baseTexture != NULL ? RwRasterGetFormat(baseRaster) : RwRasterGetFormat(bumpRaster)) &
        rwRASTERFORMATMIPMAP) {
        rasterFormat |= rwRASTERFORMATMIPMAP | rwRASTERFORMATAUTOMIPMAP;
    }
    raster = fn_80489E74(rasterWidth, rasterHeight, rasterDepth, rasterFormat);
    fn_80489A1C(raster, image);
    texture = fn_8048CCDC(raster);

    if (baseTexture != NULL) {
        filtering = RwTextureGetFilterMode(baseTexture);
        addressing = MatFXTextureAddressing(baseTexture->filterAddressing);
    } else {
        filtering = RwTextureGetFilterMode(bumpTexture);
        addressing = MatFXTextureAddressing(bumpTexture->filterAddressing);
    }
    RwTextureSetAddressing(texture, addressing);
    RwTextureSetFilterMode(texture, filtering);

    fn_80487DB0(image);
    fn_80487DB0(bumpImage);

    /* the name buffer's initializer runs after the images are destroyed, so it is declared in this
     * block */
    {
        RwChar name[rwTEXTUREBASENAMELENGTH] = {0};

        MatFXBumpedTextureName(name, baseTexture, bumpTexture);
        fn_8048CE54(texture, name);
    }
    return texture;
}

RwBool fn_804A1E74(void) {
    if (fn_80485D78(0, rpID_MATFXPLUGIN, fn_804A07DC, fn_804A076C) < 0) {
        return FALSE;
    }

    lbl_805F16A8 = fn_8045B2CC(sizeof(rpMatFXMaterialData *), rpID_MATFXPLUGIN, fn_804A0884,
                               fn_804A0894, fn_804A09D0);
    if (lbl_805F16A8 < 0) {
        return FALSE;
    }
    if (fn_8045B30C(rpID_MATFXPLUGIN, fn_804A114C, fn_804A0DEC, fn_804A16D4) < 0) {
        return FALSE;
    }

    lbl_805F16AC =
        fn_80458B6C(sizeof(RwBool), rpID_MATFXPLUGIN, fn_804A1818, fn_804A1828, fn_804A1838);
    if (lbl_805F16AC < 0) {
        return FALSE;
    }
    if (fn_80458BEC(rpID_MATFXPLUGIN, fn_804A1888, fn_804A1854, fn_804A18F0) < 0) {
        return FALSE;
    }

    lbl_805F16B0 =
        fn_8045F2F8(sizeof(RwBool), rpID_MATFXPLUGIN, fn_804A1910, fn_804A1920, fn_804A1930);
    if (lbl_805F16B0 < 0) {
        return FALSE;
    }
    if (fn_8045F338(rpID_MATFXPLUGIN, fn_804A1980, fn_804A194C, fn_804A19E8) < 0) {
        return FALSE;
    }

    return fn_804A6C74() != FALSE;
}

RpAtomic *fn_804A2004(RpAtomic *atomic) {
    RwBool *enabled = MATFXATOMICGETDATA(atomic);

    if (!*enabled) {
        if (fn_804A558C(atomic) == NULL) {
            return NULL;
        }
        *enabled = TRUE;
    }
    return atomic;
}

RwBool fn_804A2064(RpAtomic *atomic) {
    return *MATFXATOMICGETDATA(atomic);
}

RpWorldSector *fn_804A2070(RpWorldSector *worldSector) {
    RwBool *enabled = MATFXWORLDSECTORGETDATA(worldSector);

    if (!*enabled) {
        if (fn_804A5598(worldSector) == NULL) {
            return NULL;
        }
        *enabled = TRUE;
    }
    return worldSector;
}

/* Sets the effects, clearing the slots when they change; a dual pass blends by source alpha */
RpMaterial *fn_804A20D0(RpMaterial *material, RwInt32 flags) {
    rpMatFXMaterialData *data = MatFXMaterialDataCreate(material);

    if (data == NULL) {
        return NULL;
    }
    if (flags == rpMATFXEFFECTNULL || (data->flags != rpMATFXEFFECTNULL && data->flags != flags)) {
        MatFXMaterialDataClean(data);
    }

    data->flags = flags;
    switch (data->flags) {
    case rpMATFXEFFECTNULL:
        break;
    case rpMATFXEFFECTBUMPMAP:
        data->data[0].flag = rpMATFXEFFECTBUMPMAP;
        break;
    case rpMATFXEFFECTENVMAP:
        data->data[0].flag = rpMATFXEFFECTENVMAP;
        break;
    case rpMATFXEFFECTBUMPENVMAP:
        data->data[0].flag = rpMATFXEFFECTBUMPMAP;
        data->data[1].flag = rpMATFXEFFECTENVMAP;
        break;
    case rpMATFXEFFECTDUAL:
        data->data[0].flag = rpMATFXEFFECTDUAL;
        fn_804A2AB0(material, rwBLENDSRCALPHA, rwBLENDINVSRCALPHA);
        break;
    case rpMATFXEFFECTUVTRANSFORM:
        data->data[0].flag = rpMATFXEFFECTUVTRANSFORM;
        break;
    case rpMATFXEFFECTDUALUVTRANSFORM:
        data->data[0].flag = rpMATFXEFFECTUVTRANSFORM;
        data->data[1].flag = rpMATFXEFFECTDUAL;
        fn_804A2AB0(material, rwBLENDSRCALPHA, rwBLENDINVSRCALPHA);
        break;
    }
    return material;
}

RwInt32 fn_804A22E4(const RpMaterial *material) {
    const rpMatFXMaterialData *data = *MATFXMATERIALGETDATA(material);

    if (data == NULL) {
        return rpMATFXEFFECTNULL;
    }
    return data->flags;
}

/* Sets the bump map texture, and the bumped texture made from it and the material's texture,
 * which is shared through the current texture dictionary */
RpMaterial *fn_804A2304(RpMaterial *material, RwTexture *texture) {
    MatFXBumpMapData *bumpMap =
        &MatFXMaterialDataGetSlot(*MATFXMATERIALGETDATA(material), rpMATFXEFFECTBUMPMAP)
             ->data.bumpMap;
    RwTexture *baseTexture;

    if (bumpMap->texture != NULL) {
        fn_8048CD8C(bumpMap->texture);
        bumpMap->texture = NULL;
    }
    if (bumpMap->bumpedTexture != NULL) {
        fn_8048CD8C(bumpMap->bumpedTexture);
        bumpMap->bumpedTexture = NULL;
        bumpMap->invBumpWidth = 0.0f;
    }

    if (texture != NULL) {
        RwBool unsized;

        bumpMap->texture = texture;
        bumpMap->texture->refCount++;

        baseTexture = material->texture;
        unsized = texture->raster->width == 0;
        if (!unsized && baseTexture != NULL) {
            unsized = baseTexture->raster->width == 0;
        }
        if (!unsized) {
            RwChar name[rwTEXTUREBASENAMELENGTH] = {0};
            RwTexDictionary *dict;

            MatFXBumpedTextureName(name, baseTexture, texture);

            dict = fn_8048D0D8();
            bumpMap->bumpedTexture = NULL;
            if (dict != NULL) {
                bumpMap->bumpedTexture = fn_8048CFFC(dict, name);
            }
            if (bumpMap->bumpedTexture == NULL) {
                bumpMap->bumpedTexture = fn_804A55AC(baseTexture, texture);
                if (bumpMap->bumpedTexture == NULL) {
                    return NULL;
                }
                if (dict != NULL) {
                    fn_8048CFAC(dict, bumpMap->bumpedTexture);
                }
            } else {
                bumpMap->bumpedTexture->refCount++;
            }
            bumpMap->invBumpWidth = 1.0f / bumpMap->bumpedTexture->raster->width;
        }
    } else {
        /* the material must have a texture here: a NULL one is dereferenced, as in the original */
        bumpMap->invBumpWidth = 1.0f / material->texture->raster->width;
    }
    return material;
}

/* the slot of a material's effects with an effect */
#define MATFXSLOT(material, flag)                                                                  \
    (&MatFXMaterialDataGetSlot(*MATFXMATERIALGETDATA(material), flag)->data)

RpMaterial *fn_804A25E0(RpMaterial *material, RwFrame *frame) {
    MATFXSLOT(material, rpMATFXEFFECTBUMPMAP)->bumpMap.frame = frame;
    return material;
}

RpMaterial *fn_804A2628(RpMaterial *material, RwReal coef) {
    MATFXSLOT(material, rpMATFXEFFECTBUMPMAP)->bumpMap.coef = -coef;
    return material;
}

RwTexture *fn_804A2674(const RpMaterial *material) {
    return MATFXSLOT(material, rpMATFXEFFECTBUMPMAP)->bumpMap.texture;
}

RwFrame *fn_804A26BC(const RpMaterial *material) {
    return MATFXSLOT(material, rpMATFXEFFECTBUMPMAP)->bumpMap.frame;
}

RwReal fn_804A2704(const RpMaterial *material) {
    return -MATFXSLOT(material, rpMATFXEFFECTBUMPMAP)->bumpMap.coef;
}

RpMaterial *fn_804A2750(RpMaterial *material, RwTexture *texture) {
    MatFXEnvMapData *envMap = &MATFXSLOT(material, rpMATFXEFFECTENVMAP)->envMap;

    texture->refCount++;
    if (envMap->texture != NULL) {
        fn_8048CD8C(envMap->texture);
        envMap->texture = NULL;
    }
    envMap->texture = texture;
    return material;
}

RpMaterial *fn_804A27F8(RpMaterial *material, RwFrame *frame) {
    MATFXSLOT(material, rpMATFXEFFECTENVMAP)->envMap.frame = frame;
    return material;
}

RpMaterial *fn_804A2840(RpMaterial *material, RwBool useFrameBufferAlpha) {
    MATFXSLOT(material, rpMATFXEFFECTENVMAP)->envMap.useFrameBufferAlpha = useFrameBufferAlpha;
    return material;
}

RpMaterial *fn_804A2888(RpMaterial *material, RwReal coef) {
    MATFXSLOT(material, rpMATFXEFFECTENVMAP)->envMap.coef = coef;
    return material;
}

RwTexture *fn_804A28D0(const RpMaterial *material) {
    return MATFXSLOT(material, rpMATFXEFFECTENVMAP)->envMap.texture;
}

RwFrame *fn_804A2918(const RpMaterial *material) {
    return MATFXSLOT(material, rpMATFXEFFECTENVMAP)->envMap.frame;
}

RwBool fn_804A2960(const RpMaterial *material) {
    return MATFXSLOT(material, rpMATFXEFFECTENVMAP)->envMap.useFrameBufferAlpha;
}

RwReal fn_804A29A8(const RpMaterial *material) {
    return MATFXSLOT(material, rpMATFXEFFECTENVMAP)->envMap.coef;
}

RpMaterial *fn_804A29F0(RpMaterial *material, RwTexture *texture) {
    MatFXDualData *dual = &MATFXSLOT(material, rpMATFXEFFECTDUAL)->dual;

    texture->refCount++;
    if (dual->texture != NULL) {
        fn_8048CD8C(dual->texture);
        dual->texture = NULL;
    }
    dual->texture = texture;
    fn_804A55A4(dual, rwRENDERSTATETEXTUREADDRESS);
    fn_804A55A4(dual, rwRENDERSTATETEXTURERASTER);
    return material;
}

RpMaterial *fn_804A2AB0(RpMaterial *material, RwInt32 srcBlendMode, RwInt32 dstBlendMode) {
    MatFXDualData *dual = &MATFXSLOT(material, rpMATFXEFFECTDUAL)->dual;

    dual->srcBlendMode = srcBlendMode;
    dual->dstBlendMode = dstBlendMode;
    fn_804A55A4(dual, rwRENDERSTATESRCBLEND);
    fn_804A55A4(dual, rwRENDERSTATEDESTBLEND);
    return material;
}

RwTexture *fn_804A2B44(const RpMaterial *material) {
    return MATFXSLOT(material, rpMATFXEFFECTDUAL)->dual.texture;
}

const RpMaterial *fn_804A2B8C(const RpMaterial *material, RwInt32 *srcBlendMode,
                              RwInt32 *dstBlendMode) {
    const MatFXDualData *dual = &MATFXSLOT(material, rpMATFXEFFECTDUAL)->dual;

    *srcBlendMode = dual->srcBlendMode;
    *dstBlendMode = dual->dstBlendMode;
    return material;
}

RpMaterial *fn_804A2BE0(RpMaterial *material, RwMatrix *baseTransform, RwMatrix *dualTransform) {
    MatFXUVAnimData *uvAnim = &MATFXSLOT(material, rpMATFXEFFECTUVTRANSFORM)->uvAnim;

    uvAnim->baseTransform = baseTransform;
    uvAnim->dualTransform = dualTransform;
    return material;
}

const RpMaterial *fn_804A2C2C(const RpMaterial *material, RwMatrix **baseTransform,
                              RwMatrix **dualTransform) {
    const MatFXUVAnimData *uvAnim = &MATFXSLOT(material, rpMATFXEFFECTUVTRANSFORM)->uvAnim;

    if (baseTransform != NULL) {
        *baseTransform = uvAnim->baseTransform;
    }
    if (dualTransform != NULL) {
        *dualTransform = uvAnim->dualTransform;
    }
    return material;
}
