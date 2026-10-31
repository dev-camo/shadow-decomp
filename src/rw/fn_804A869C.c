/* RenderWare multi-texturing (part of the material effects plugin, plugin ID 0x120): the engine
 * plugin (plugin ID 0x12C), each platform's material plugin with a material's texture set for the
 * platform and its stream, and the texture set accessors. The original file name is unknown. */
#include "rw/rpmatfx.h"
#include <string.h>

#define rwMEMHINT_MULTITEXTURE (rwMEMHINTDUR_EVENT | rwMEMHINTFLAG_RESIZABLE | rpID_MULTITEXPLUGIN)

/* The bytes after the library version in a texture set's stream */
typedef struct MultiTextureStreamHeader {
    /* 0x0 */ RwUInt8 platformID;
    /* 0x1 */ RwUInt8 numTextures;
    /* 0x2 */ RwUInt8 hasEffect;
    /* 0x3 */ RwUInt8 pad;
} MultiTextureStreamHeader;
CHECK_SIZEOF(MultiTextureStreamHeader, 0x4);

RwModuleInfo lbl_805EDF68 = {0, 0};
rpMultiTexturePlatform lbl_805E32D8[rpMTNUMPLATFORMS]; /* each platform's material plugin */

/* Creates an empty texture set of numTextures textures for a platform (name inferred) */
static inline RpMultiTexture *MultiTextureCreate(rpMultiTexturePlatform *platform,
                                                 RwUInt32 numTextures) {
    RwInt32 size;
    RpMultiTexture *multiTexture;

    size = sizeof(RpMultiTexture) + platform->dataSize;
    multiTexture = RwMalloc(size, rwMEMHINT_MULTITEXTURE);

    if (multiTexture == NULL) {
        RWERRORPLUGIN(rpID_MATFXPLUGIN, (E_RW_NOMEM, size));
        return NULL;
    }
    memset(multiTexture, 0, size);
    multiTexture->platform = platform;
    multiTexture->numTextures = numTextures;
    if (platform->dataSize != 0) {
        multiTexture->platformData = multiTexture + 1;
    }
    return multiTexture;
}

/* Releases a texture set's textures and effect and frees it (name inferred) */
static inline void MultiTextureDestroy(RpMultiTexture *multiTexture) {
    RwUInt32 i;

    for (i = 0; i < multiTexture->numTextures; i++) {
        if (multiTexture->textures[i] != NULL) {
            fn_8048CD8C(multiTexture->textures[i]);
            multiTexture->textures[i] = NULL;
        }
    }
    if (multiTexture->effect != NULL) {
        fn_804A9674(multiTexture->effect);
        multiTexture->effect = NULL;
    }
    RwFree(multiTexture);
}

void *fn_804A869C(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805EDF68.numInstances++;
    fn_804A9158();
    return instance;
}

void *fn_804A86DC(void *instance, RwInt32 offset, RwInt32 size) {
    fn_804A9274();
    lbl_805EDF68.numInstances--;
    return instance;
}

void *fn_804A871C(void *object, RwInt32 offset, RwInt32 size) {
    *RWPLUGINOFFSET(RpMultiTexture *, object, offset) = NULL;
    return object;
}

void *fn_804A8728(void *object, RwInt32 offset, RwInt32 size) {
    RpMultiTexture *multiTexture = *RWPLUGINOFFSET(RpMultiTexture *, object, offset);

    if (multiTexture != NULL) {
        MultiTextureDestroy(multiTexture);
        *RWPLUGINOFFSET(RpMultiTexture *, object, offset) = NULL;
    }
    return object;
}

void *fn_804A87D8(void *dstObject, const void *srcObject, RwInt32 offset, RwInt32 size) {
    const RpMultiTexture *src = *RWPLUGINOFFSET(RpMultiTexture *, srcObject, offset);
    RpMultiTexture *dst;
    RwUInt32 i;

    if (src == NULL) {
        return dstObject;
    }
    dst = MultiTextureCreate(src->platform, src->numTextures);
    if (dst == NULL) {
        return NULL;
    }
    *RWPLUGINOFFSET(RpMultiTexture *, dstObject, offset) = dst;

    for (i = 0; i < src->numTextures; i++) {
        fn_804A9030(dst, i, fn_804A90A0(src, i));
        fn_804A90B0(dst, i, fn_804A90BC(src, i));
    }
    fn_804A8FCC(dst, fn_804A9028(src));
    return dstObject;
}

RwInt32 fn_804A8924(const void *object, RwInt32 offset, RwInt32 size) {
    const RpMultiTexture *multiTexture = *RWPLUGINOFFSET(RpMultiTexture *, object, offset);
    RwInt32 streamSize = 0;
    RwUInt32 i;

    if (multiTexture != NULL) {
        /* the library version, the header and the coordinate sets */
        streamSize = multiTexture->numTextures + sizeof(RwInt32) + sizeof(MultiTextureStreamHeader);
        for (i = 0; i < multiTexture->numTextures; i++) {
            streamSize += fn_804A0DB8(multiTexture->textures[i]);
        }
        if (multiTexture->effect != NULL) {
            streamSize += fn_80482F64(multiTexture->effect->name) + rwCHUNKHEADERSIZE;
        }
    }
    return streamSize;
}

RwStream *fn_804A89C4(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {
    const RpMultiTexture *multiTexture = *RWPLUGINOFFSET(RpMultiTexture *, object, offset);
    RwInt32 version;
    MultiTextureStreamHeader header;
    RwUInt32 i;

    if (multiTexture != NULL) {
        version = fn_80485D6C();
        if (fn_8047D458(stream, &version, sizeof(version)) == NULL) {
            return NULL;
        }

        header.platformID = multiTexture->platform->platformID;
        header.numTextures = multiTexture->numTextures;
        header.hasEffect = multiTexture->effect != NULL;
        header.pad = 0;
        if (fn_80480ED0(stream, &header, sizeof(header)) == NULL) {
            return NULL;
        }

        if (multiTexture->numTextures != 0) {
            if (fn_80480ED0(stream, multiTexture->coords, multiTexture->numTextures) == NULL) {
                return NULL;
            }
            for (i = 0; i < multiTexture->numTextures; i++) {
                fn_804A0C50(stream, multiTexture->textures[i]);
            }
        }

        if (multiTexture->effect != NULL) {
            if (fn_80482FA4(multiTexture->effect->name, stream) == NULL) {
                return NULL;
            }
        }
    }
    return stream;
}

RwStream *fn_804A8B08(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    RpMultiTexture *multiTexture;
    rpMultiTexturePlatform *platform;
    RwInt32 version;
    MultiTextureStreamHeader header;
    RwChar effectName[rpMTEFFECTNAMELENGTH];
    RpMTEffect *effect;
    RwUInt32 i;

    if (fn_8047D868(stream, &version, sizeof(version)) == NULL) {
        return NULL;
    }
    if (fn_80480D4C(stream, &header, sizeof(header)) == 0) {
        return NULL;
    }

    platform = &lbl_805E32D8[header.platformID];
    multiTexture = MultiTextureCreate(platform, header.numTextures);
    if (multiTexture == NULL) {
        return NULL;
    }

    if (header.numTextures != 0) {
        if (fn_80480D4C(stream, multiTexture->coords, multiTexture->numTextures) == 0) {
            MultiTextureDestroy(multiTexture);
            return NULL;
        }
        for (i = 0; i < multiTexture->numTextures; i++) {
            if (fn_804A0CDC(stream, &multiTexture->textures[i]) == NULL) {
                MultiTextureDestroy(multiTexture);
                return NULL;
            }
        }
    }

    if (header.hasEffect & 1) {
        if (fn_80483060(effectName, stream) == NULL) {
            MultiTextureDestroy(multiTexture);
            return NULL;
        }

        /* an effect that cannot be found is replaced by a dummy of its name */
        effect = fn_804A9914(effectName);
        if (effect == NULL) {
            effect = fn_804A95D4();
            if (effect == NULL) {
                MultiTextureDestroy(multiTexture);
                return NULL;
            }
            fn_804A9BA4(effect, effectName);
        }
        fn_804A8FCC(multiTexture, effect);
        fn_804A9674(effect);
    }

    *RWPLUGINOFFSET(RpMultiTexture *, object, offset) = multiTexture;
    return stream;
}

RwBool fn_804A8E90(void) {
    RwInt32 offset;

    if (!fn_804A90F8()) {
        return FALSE;
    }
    memset(lbl_805E32D8, 0, sizeof(lbl_805E32D8));
    offset =
        fn_80485D78(sizeof(rpMultiTextureGlobals), rpID_MULTITEXPLUGIN, fn_804A869C, fn_804A86DC);
    lbl_805EDF68.globalsOffset = offset;
    if (offset < 0) {
        return FALSE;
    }
    return TRUE;
}

RwBool fn_804A8EFC(RwPlatformID platformID, RwUInt32 pluginID, RwUInt32 size) {
    rpMultiTexturePlatform *platform;
    RwInt32 offset;

    offset = fn_8045B2CC(sizeof(RpMultiTexture *), pluginID, fn_804A871C, fn_804A8728, fn_804A87D8);
    if (offset < 0) {
        return FALSE;
    }
    platform = &lbl_805E32D8[platformID];
    platform->offset = offset;

    offset = fn_8045B30C(pluginID, fn_804A8B08, fn_804A89C4, fn_804A8924);
    if (offset < 0) {
        return FALSE;
    }
    platform->platformID = platformID;
    platform->pluginID = pluginID;
    platform->dataSize = size;
    return TRUE;
}

/* Sets the effect, which the texture set holds a reference to */
RpMultiTexture *fn_804A8FCC(RpMultiTexture *multiTexture, RpMTEffect *effect) {
    if (multiTexture->effect != NULL) {
        fn_804A9674(multiTexture->effect);
    }
    multiTexture->effect = effect;
    if (effect != NULL) {
        fn_804A9BE8(multiTexture->effect);
    }
    return multiTexture;
}

RpMTEffect *fn_804A9028(const RpMultiTexture *multiTexture) {
    return multiTexture->effect;
}

/* Sets a texture, which the texture set holds a reference to */
RpMultiTexture *fn_804A9030(RpMultiTexture *multiTexture, RwUInt32 index, RwTexture *texture) {
    if (multiTexture->textures[index] != NULL) {
        fn_8048CD8C(multiTexture->textures[index]);
    }
    multiTexture->textures[index] = texture;
    if (texture != NULL) {
        texture->refCount++;
    }
    return multiTexture;
}

RwTexture *fn_804A90A0(const RpMultiTexture *multiTexture, RwUInt32 index) {
    return multiTexture->textures[index];
}

RpMultiTexture *fn_804A90B0(RpMultiTexture *multiTexture, RwUInt32 index, RwUInt32 texCoordIndex) {
    multiTexture->coords[index] = texCoordIndex;
    return multiTexture;
}

RwUInt32 fn_804A90BC(const RpMultiTexture *multiTexture, RwUInt32 index) {
    return multiTexture->coords[index];
}

RpMultiTexture *fn_804A90C8(const RpMaterial *material, RwPlatformID platformID) {
    rpMultiTexturePlatform *platform = &lbl_805E32D8[platformID];

    if (platform->pluginID != 0) {
        return *RWPLUGINOFFSET(RpMultiTexture *, material, platform->offset);
    }
    return NULL;
}
