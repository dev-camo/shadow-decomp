/* RenderWare cameras in streams. The original file name is unknown. */
#include "rw/rwcore.h"
#include "layout_check.h"

/* Reports an error of the core; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwID_COREPLUGIN;                                                         \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

/* A camera's stream data (0x20 bytes) */
typedef struct rwStreamCamera {
    /* 0x00 */ RwV2d viewWindow;
    /* 0x08 */ RwV2d viewOffset;
    /* 0x10 */ RwReal nearPlane;
    /* 0x14 */ RwReal farPlane;
    /* 0x18 */ RwReal fogPlane;
    /* 0x1C */ RwInt32 projection;
} rwStreamCamera;
CHECK_SIZEOF(rwStreamCamera, 0x20);

/* Registers a camera plugin's stream chunk callbacks */
RwInt32 fn_80483554(RwUInt32 pluginID, RwPluginDataChunkReadCallBack readCB,
                    RwPluginDataChunkWriteCallBack writeCB,
                    RwPluginDataChunkGetSizeCallBack getSizeCB) {
    return fn_804815F0(&lbl_8056DF70, pluginID, readCB, writeCB, getSizeCB);
}

/* Reads a camera: its structure chunk, then its plugins' data */
RwCamera *fn_80483590(RwStream *stream) {
    RwUInt32 size;
    RwUInt32 version;

    if (!fn_8047CF50(stream, rwID_STRUCT, &size, &version)) {
        return NULL;
    }
    if (version >= rwLIBRARYBASEVERSION && version <= RW_STREAM_VERSION) {
        rwStreamCamera cam;
        RwCamera *camera;

        memset(&cam, 0, sizeof(cam));
        if (fn_80480D4C(stream, &cam, size) != size) {
            return NULL;
        }
        fn_8047D254(&cam, sizeof(cam));

        camera = fn_80485A58();
        if (camera == NULL) {
            return NULL;
        }
        if (fn_804816AC(&lbl_8056DF70, stream, camera) == NULL) {
            return NULL;
        }
        fn_80485950(camera, &cam.viewWindow);
        fn_80485504(camera, &cam.viewOffset);
        fn_80485550(camera, cam.nearPlane);
        fn_8048560C(camera, cam.farPlane);
        camera->fogPlane = cam.fogPlane;
        fn_8048584C(camera, cam.projection);
        return camera;
    }
    RWERROR((E_RW_BADVERSION));
    return NULL;
}
