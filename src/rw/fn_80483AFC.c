/* RenderWare textures and texture dictionaries in streams. The original file name is unknown. */
#include "rw/rwcore.h"
#include "layout_check.h"

/* chunk IDs */
#define rwID_TEXTURE 0x6
#define rwID_TEXTURENATIVE 0x15

/* The oldest stream version textures are read from */
#define rwSTREAMOLDESTVERSION 0x35000

/* The size of a chunk header in a stream */
#define rwCHUNKHEADERSIZE 12

/* rwRASTERFORMATAUTOMIPMAP in a raster's cFormat */
#define rwRASTERFORMATAUTOMIPMAP_CFORMAT 0x10

/* A texture's flags in a stream: its filtering and addressing, and above them whether its
 * mipmaps were not generated (inferred) */
#define rwTEXTURESTREAMUSERMIPMAPS 0x10000

/* the filter modes that use mipmaps */
#define rwFILTERMIPNEAREST 3
#define rwFILTERLINEARMIPLINEAR 6

/* The system request for the device's ID (inferred) */
#define rwDEVICESYSTEMGETID 0x16

/* The standard function that reads a native texture */
#define rwSTANDARDNATIVETEXTUREREAD 26

/* The characters a texture name is read into */
#define rwTEXTURENAMEBUFFERLENGTH (rwTEXTUREBASENAMELENGTH * 4)

/* The core's error plugin ID (inferred) */
#define rwCOREERRORPLUGIN 1

/* Reports an error of the core; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwCOREERRORPLUGIN;                                                       \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

/* A texture dictionary's stream data, as it is in memory once read */
typedef struct rwStreamTexDictionary {
    /* 0x0 */ RwUInt16 deviceId; /* 0 for any device */
    /* 0x2 */ RwUInt16 numTextures;
} rwStreamTexDictionary;
CHECK_SIZEOF(rwStreamTexDictionary, 0x4);

/* Registers a texture plugin's stream chunk callbacks */
RwInt32 fn_80483AFC(RwUInt32 pluginID, RwPluginDataChunkReadCallBack readCB,
                    RwPluginDataChunkWriteCallBack writeCB,
                    RwPluginDataChunkGetSizeCallBack getSizeCB) {
    return fn_804815F0(&lbl_8056E030, pluginID, readCB, writeCB, getSizeCB);
}

/* The size of a texture's chunk data: its flags, name and mask, and its plugins' data */
RwUInt32 fn_80483B38(const RwTexture *texture) {
    RwUInt32 size = rwCHUNKHEADERSIZE + sizeof(RwUInt32);

    size += rwCHUNKHEADERSIZE + fn_80482F64(texture->name);
    size += rwCHUNKHEADERSIZE + fn_80482F64(texture->mask);
    size += rwCHUNKHEADERSIZE + fn_804818FC(&lbl_8056E030, texture);
    return size;
}

/* Writes a texture chunk: its flags, name and mask, and its plugins' data */
const RwTexture *fn_80483BA0(const RwTexture *texture, RwStream *stream) {
    RwUInt32 flags;
    RwUInt32 userMipmaps;

    if (!fn_8047CEF0(stream, rwID_TEXTURE, fn_80483B38(texture), RW_STREAM_VERSION,
                     RW_STREAM_BUILD)) {
        return NULL;
    }
    if (!fn_8047CEF0(stream, rwID_STRUCT, sizeof(flags), RW_STREAM_VERSION, RW_STREAM_BUILD)) {
        return NULL;
    }
    if (texture->raster != NULL && !(texture->raster->cFormat & rwRASTERFORMATAUTOMIPMAP_CFORMAT)) {
        userMipmaps = 1;
    } else {
        userMipmaps = 0;
    }
    flags = (texture->filterAddressing & 0xFFFF) | ((userMipmaps << 16) & 0xFF0000);
    fn_8047D158(&flags, sizeof(flags));
    if (!fn_80480ED0(stream, &flags, sizeof(flags))) {
        return NULL;
    }
    if (!fn_80482FA4(texture->name, stream)) {
        return NULL;
    }
    if (!fn_80482FA4(texture->mask, stream)) {
        return NULL;
    }
    if (fn_80481980(&lbl_8056E030, stream, texture)) {
        return texture;
    }
    return NULL;
}

/* Reads a texture chunk, finding or loading the texture by its name and mask with the mipmap
 * settings it was written with */
RwTexture *fn_80483D28(RwStream *stream) {
    RwUInt32 size;
    RwUInt32 version;
    RwUInt32 flags;
    RwChar name[rwTEXTURENAMEBUFFERLENGTH];
    RwChar mask[rwTEXTURENAMEBUFFERLENGTH];
    RwInt32 filterMode;
    RwBool mipmapping;
    RwBool autoMipmapping;
    RwTexture *texture;
    RwUInt32 addressingU;
    RwUInt32 mipmapFlags;

    if (!fn_8047CF50(stream, rwID_STRUCT, &size, &version)) {
        return NULL;
    }
    if (version >= rwSTREAMOLDESTVERSION && version <= RW_STREAM_VERSION) {
        memset(&flags, 0, sizeof(flags));
        if (size != fn_80480D4C(stream, &flags, size)) {
            return NULL;
        }
        fn_8047D254(&flags, sizeof(flags));

        filterMode = flags & 0xFF;
        addressingU = (flags >> 8) & 0xF;

        /* Without a V addressing mode, V uses U's */
        if (((flags >> 12) & 0xF) == 0) {
            flags |= (addressingU << 12) & 0xF000;
        }
        mipmapFlags = (flags >> 16) & 0xFF;

        mipmapping = fn_8048C9C0();
        autoMipmapping = fn_8048C9EC();
        if ((RwUInt32)(filterMode - rwFILTERMIPNEAREST) <= 2 ||
            filterMode == rwFILTERLINEARMIPLINEAR) {
            fn_8048C9A8(TRUE);
            if (mipmapFlags & 1) {
                fn_8048C9D4(FALSE);
            } else {
                fn_8048C9D4(TRUE);
            }
        } else {
            fn_8048C9A8(FALSE);
            fn_8048C9D4(FALSE);
        }

        if (!fn_80483060(name, stream)) {
            fn_8048C9A8(mipmapping);
            fn_8048C9D4(autoMipmapping);
            return NULL;
        }
        if (!fn_80483060(mask, stream)) {
            fn_8048C9A8(mipmapping);
            fn_8048C9D4(autoMipmapping);
            return NULL;
        }

        texture = fn_8048D130(name, mask);
        if (texture == NULL) {
            fn_80481AD4(&lbl_8056E030, stream);
            fn_8048C9A8(mipmapping);
            fn_8048C9D4(autoMipmapping);
            return NULL;
        }
        fn_8048C9A8(mipmapping);
        fn_8048C9D4(autoMipmapping);

        /* A texture already loaded keeps its flags and plugins' data */
        if (texture->refCount == 1) {
            texture->filterAddressing = flags & 0xFFFF;
            if (!fn_804816AC(&lbl_8056E030, stream, texture)) {
                return NULL;
            }
        } else if (!fn_80481AD4(&lbl_8056E030, stream)) {
            return NULL;
        }
        return texture;
    }
    RWERROR((E_RW_BADVERSION));
    return NULL;
}

/* Destroys a texture of a texture dictionary being freed */
RwTexture *fn_80483F8C(RwTexture *texture, void *data) {
    fn_8048CD8C(texture);
    return texture;
}

/* Reads a texture dictionary: its native textures, then its plugins' data */
RwTexDictionary *fn_80483FBC(RwStream *stream) {
    RwUInt32 size;
    RwUInt32 version;
    rwStreamTexDictionary binDict;
    RwTexture *texture;
    RwUInt16 deviceId;
    RwDevice *device;
    RwTexDictionary *dict;

    if (!fn_8047CF50(stream, rwID_STRUCT, &size, &version)) {
        return NULL;
    }
    if (version >= rwSTREAMOLDESTVERSION && version <= RW_STREAM_VERSION) {
        device = &lbl_805F15B4->dOpenDevice;
        memset(&binDict, 0, sizeof(binDict));
        if (size != fn_80480D4C(stream, &binDict, size)) {
            return NULL;
        }
        fn_8047D254(&binDict, sizeof(binDict));

        /* Textures for another device cannot be read */
        deviceId = 0;
        fn_80485C3C(device, rwDEVICESYSTEMGETID, &deviceId, NULL, 0);
        deviceId &= 0xFF;
        if (deviceId != 0 && binDict.deviceId != 0 && deviceId != binDict.deviceId) {
            return NULL;
        }

        dict = fn_8048CA64();
        if (dict == NULL) {
            return NULL;
        }
        while (binDict.numTextures--) {
            if (!fn_8047CF50(stream, rwID_TEXTURENATIVE, &size, &version)) {
                fn_8048CBE0(dict, fn_80483F8C, NULL);
                fn_8048CB38(dict);
                return NULL;
            }
            if (!(version >= rwSTREAMOLDESTVERSION && version <= RW_STREAM_VERSION)) {
                fn_8048CBE0(dict, fn_80483F8C, NULL);
                fn_8048CB38(dict);
                RWERROR((E_RW_BADVERSION));
                return NULL;
            }
            if (!lbl_805F15B4->stdFunc[rwSTANDARDNATIVETEXTUREREAD](stream, &texture, size)) {
                fn_8048CBE0(dict, fn_80483F8C, NULL);
                fn_8048CB38(dict);
                return NULL;
            }
            if (texture == NULL) {
                fn_8048CBE0(dict, fn_80483F8C, NULL);
                fn_8048CB38(dict);
                return NULL;
            }
            if (!fn_804816AC(&lbl_8056E030, stream, texture)) {
                fn_8048CBE0(dict, fn_80483F8C, NULL);
                fn_8048CB38(dict);
                return NULL;
            }
            fn_8048CFAC(dict, texture);
        }

        if (!fn_804816AC(&lbl_8056E048, stream, dict)) {
            fn_8048CBE0(dict, fn_80483F8C, NULL);
            fn_8048CB38(dict);
            return NULL;
        }
        return dict;
    }
    RWERROR((E_RW_BADVERSION));
    return NULL;
}
