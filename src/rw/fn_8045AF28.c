/* RenderWare materials: the material rights stream extension, the material module's free list,
 * creating and destroying materials, their textures, and reading a material from a stream. The
 * original file name is unknown. */
#include "rw/rpworld.h"
#include <string.h>
#include "layout_check.h"

#define rwMEMHINTDUR_EVENT 0x30000
#define rwMEMHINTDUR_GLOBAL 0x40000

/* chunk IDs */
#define rwID_TEXTURE 0x6
#define rwID_MATERIAL 0x7

/* the plugin ID the world module reports errors with (inferred) */
#define rwWORLDERRORPLUGIN 2

/* Reports an error: the arguments are the error code and its parameters */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwWORLDERRORPLUGIN;                                                      \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

/* The stream header of a material (0x1C bytes) */
typedef struct MaterialChunk {
    /* 0x00 */ RwInt32 flags;
    /* 0x04 */ RwRGBA color; /* bytes, not converted */
    /* 0x08 */ RwInt32 unused;
    /* 0x0C */ RwBool textured;
    /* 0x10 */ RwSurfaceProperties surfaceProps;
} MaterialChunk;
CHECK_SIZEOF(MaterialChunk, 0x1C);

/* The material module's globals in the engine (inferred) */
typedef struct MaterialGlobals {
    /* 0x0 */ RwFreeList *materialFreeList;
} MaterialGlobals;
CHECK_SIZEOF(MaterialGlobals, 0x4);

/* The material module (inferred) */
typedef struct MaterialModule {
    /* 0x0 */ RwInt32 globalsOffset; /* engine plugin offset of the globals */
    /* 0x4 */ RwInt32 numInstances;  /* the open engine instances */
} MaterialModule;
CHECK_SIZEOF(MaterialModule, 0x8);

#define MATERIALGLOBALS()                                                                          \
    ((MaterialGlobals *)((RwUInt8 *)lbl_805F15B4 + lbl_805F14E8.globalsOffset))
#define MATERIALGLOBAL(var) (MATERIALGLOBALS()->var)

RwPluginRegistry lbl_8056D840 = {sizeof(RpMaterial), sizeof(RpMaterial), 0, 0, NULL, NULL};

/* the surface properties of a new material */
RwSurfaceProperties lbl_8056D858 = {1.0f, 1.0f, 1.0f};

RwFreeList lbl_805E15F0; /* the material free list */

RwInt32 lbl_805EDDB8 = 256; /* material free list entries per block */
RwInt32 lbl_805EDDBC = 1;   /* material free list blocks to preallocate */

/* declared in reverse address order (the compiler places them in .sbss last first) */
MaterialModule lbl_805F14E8;
RwUInt32 lbl_805F14E4; /* material pipeline plugin, from the last material stream's rights */
RwUInt32 lbl_805F14E0; /* material pipeline plugin data */

/* material rights stream read */
RwStream *fn_8045AF28(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    if (fn_8047D868(stream, (RwInt32 *)&lbl_805F14E4, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (length == 8 && fn_8047D868(stream, (RwInt32 *)&lbl_805F14E0, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    return stream;
}

/* material rights stream write */
RwStream *fn_8045AFA4(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {
    const RpMaterial *material = object;
    const RxPipeline *pipeline = material->pipeline;

    if (fn_8047D458(stream, (RwInt32 *)&pipeline->pluginId, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    pipeline = material->pipeline;
    if (fn_8047D458(stream, (RwInt32 *)&pipeline->pluginData, sizeof(RwInt32)) != NULL) {
        return stream;
    }
    return NULL;
}

/* material rights stream size: only for a pipeline that a plugin owns */
RwInt32 fn_8045B020(const void *object, RwInt32 offset, RwInt32 size) {
    const RpMaterial *material = object;
    const RxPipeline *pipeline = material->pipeline;

    if (pipeline != NULL && pipeline->pluginId != 0) {
        return 8;
    }
    return 0;
}

/* engine plugin constructor: the material free list */
void *fn_8045B048(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F14E8.globalsOffset = offset;
    MATERIALGLOBAL(materialFreeList) =
        fn_8047FC04(lbl_8056D840.sizeOfStruct, lbl_805EDDB8, 4, lbl_805EDDBC, &lbl_805E15F0,
                    rwMEMHINTDUR_GLOBAL | rwID_MATERIAL);
    if (MATERIALGLOBAL(materialFreeList) == NULL) {
        return NULL;
    }
    lbl_805F14E8.numInstances++;
    return instance;
}

/* engine plugin destructor */
void *fn_8045B0D8(void *instance, RwInt32 offset, RwInt32 size) {
    if (MATERIALGLOBAL(materialFreeList) != NULL) {
        fn_8047FCDC(MATERIALGLOBAL(materialFreeList));
        MATERIALGLOBAL(materialFreeList) = NULL;
    }
    lbl_805F14E8.numInstances--;
    return instance;
}

RpMaterial *fn_8045B13C(void) {
    RpMaterial *material =
        RwFreeListAlloc(MATERIALGLOBAL(materialFreeList), rwMEMHINTDUR_EVENT | rwID_MATERIAL);
    RwRGBA white;

    if (material == NULL) {
        return NULL;
    }
    material->refCount = 1;
    white.red = 255;
    white.green = 255;
    white.blue = 255;
    white.alpha = 255;
    material->color = white;
    material->texture = NULL;
    material->pipeline = NULL;
    material->surfaceProps = lbl_8056D858;
    fn_8048209C(&lbl_8056D840, material);
    return material;
}

/* Releases a material; the last reference drops its texture and frees it */
RwBool fn_8045B1F4(RpMaterial *material) {
    if (material->refCount == 1) {
        fn_80482144(&lbl_8056D840, material);
        fn_8045B270(material, NULL);
        RwFreeListFree(MATERIALGLOBAL(materialFreeList), material);
    } else {
        material->refCount--;
    }
    return TRUE;
}

/* Sets a material's texture, taking a reference to the new one and releasing the old one */
RpMaterial *fn_8045B270(RpMaterial *material, RwTexture *texture) {
    if (texture != NULL) {
        texture->refCount++;
    }
    if (material->texture != NULL) {
        fn_8048CD8C(material->texture);
    }
    material->texture = texture;
    return material;
}

/* plugin registration with materials */
RwInt32 fn_8045B2CC(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor, RwPluginObjectCopy copy) {
    return fn_80481DC8(&lbl_8056D840, size, pluginID, constructor, destructor, copy);
}

RwInt32 fn_8045B30C(RwUInt32 pluginID, RwPluginDataChunkReadCallBack read,
                    RwPluginDataChunkWriteCallBack write,
                    RwPluginDataChunkGetSizeCallBack getSize) {
    return fn_804815F0(&lbl_8056D840, pluginID, read, write, getSize);
}

/* Reads a material: its header, its texture, then its plugin data and rights */
RpMaterial *fn_8045B348(RwStream *stream) {
    RwUInt32 length;
    RwUInt32 version;
    MaterialChunk header;
    RwRGBA color;
    RpMaterial *material;

    if (!fn_8047CF50(stream, rwID_STRUCT, &length, &version)) {
        return NULL;
    }
    if (version >= rwLIBRARYBASEVERSION && version <= RW_STREAM_VERSION) {
        memset(&header, 0, sizeof(header));
        if (length != fn_80480D4C(stream, &header, length)) {
            return NULL;
        }
        /* the colour is bytes, which the conversion must not touch */
        color = header.color;
        fn_8047D254(&header, sizeof(header));
        header.color = color;

        material = fn_8045B13C();
        if (material == NULL) {
            return NULL;
        }
        material->color = header.color;
        material->surfaceProps = header.surfaceProps;
        material->texture = NULL;

        if (header.textured) {
            if (!fn_8047CF50(stream, rwID_TEXTURE, NULL, &version)) {
                fn_8045B1F4(material);
                return NULL;
            }
            if (version >= rwLIBRARYBASEVERSION && version <= RW_STREAM_VERSION) {
                material->texture = fn_80483D28(stream);
            } else {
                fn_8045B1F4(material);
                RWERROR((E_RW_BADVERSION));
                return NULL;
            }
        }

        lbl_805F14E4 = 0;
        lbl_805F14E0 = 0;
        if (fn_804816AC(&lbl_8056D840, stream, material) == NULL) {
            fn_8045B1F4(material);
            return NULL;
        }
        if (lbl_805F14E4 != 0) {
            fn_80481870(&lbl_8056D840, lbl_805F14E4, material, lbl_805F14E0);
        }
        return material;
    }
    RWERROR((E_RW_BADVERSION));
    return NULL;
}
