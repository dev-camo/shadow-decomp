/* RenderWare frame lists in streams. The original file name is unknown. */
#include "rw/rwcore.h"
#include "layout_check.h"

#define rwMEMHINTDUR_EVENT 0x30000

/* chunk IDs */
#define rwID_FRAMELIST 0xE

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

/* A frame's stream data (0x38 bytes) */
typedef struct rwStreamFrame {
    /* 0x00 */ RwV3d right;
    /* 0x0C */ RwV3d up;
    /* 0x18 */ RwV3d at;
    /* 0x24 */ RwV3d pos;
    /* 0x30 */ RwInt32 parentIndex; /* -1 for a root */
    /* 0x34 */ RwUInt32 data;
} rwStreamFrame;
CHECK_SIZEOF(rwStreamFrame, 0x38);

/* Whether frames read are added to their parents with fn_80487348, then updated from each root
 * (inferred); otherwise fn_804872E0 adds them */
RwInt32 lbl_805EDED0 = 1;

/* Registers a frame plugin's stream chunk callbacks */
RwInt32 fn_804836EC(RwUInt32 pluginID, RwPluginDataChunkReadCallBack readCB,
                    RwPluginDataChunkWriteCallBack writeCB,
                    RwPluginDataChunkGetSizeCallBack getSizeCB) {
    return fn_804815F0(&lbl_8056DFE8, pluginID, readCB, writeCB, getSizeCB);
}

/* Frees a frame list's array of frames */
RwFrameList *fn_80483728(RwFrameList *frameList) {
    if (frameList->numFrames != 0) {
        RwFree(frameList->frames);
    }
    return frameList;
}

/* Reads a frame list: its structure chunk with each frame's matrix and parent, then each frame's
 * plugins' data */
RwFrameList *fn_80483774(RwStream *stream, RwFrameList *frameList) {
    RwInt32 numFrames;
    RwUInt32 size;
    RwUInt32 version;
    RwInt32 i;

    if (!fn_8047CF50(stream, rwID_STRUCT, &size, &version)) {
        return NULL;
    }
    if (version >= rwLIBRARYBASEVERSION && version <= RW_STREAM_VERSION) {
        if (fn_80480D4C(stream, &numFrames, sizeof(numFrames)) != sizeof(numFrames)) {
            return NULL;
        }
        fn_8047D254(&numFrames, sizeof(numFrames));
        frameList->numFrames = numFrames;
        frameList->frames =
            RwMalloc(numFrames * sizeof(RwFrame *), rwMEMHINTDUR_EVENT | rwID_FRAMELIST);
        if (frameList->frames == NULL) {
            RWERROR((E_RW_NOMEM, numFrames * sizeof(RwFrame *)));
            return NULL;
        }

        for (i = 0; i < numFrames; i++) {
            rwStreamFrame frameData;
            RwFrame *frame;
            RwMatrix *matrix;

            if (fn_80480D4C(stream, &frameData, sizeof(frameData)) != sizeof(frameData)) {
                RwFree(frameList->frames);
                return NULL;
            }
            fn_8047D254(&frameData, sizeof(frameData));

            frame = fn_80486D00();
            if (frame == NULL) {
                RwFree(frameList->frames);
                return NULL;
            }
            matrix = &frame->modelling;
            matrix->right = frameData.right;
            matrix->up = frameData.up;
            matrix->at = frameData.at;
            matrix->pos = frameData.pos;

            /* An orthonormal matrix keeps its orthonormal flags */
            if (0.01f >= fn_8047E7A8(matrix) && 0.01f >= fn_8047E750(matrix) &&
                0.99f <= fn_8047E704(matrix)) {
                matrix->flags &= ~rwMATRIXINTERNALIDENTITY;
            } else {
                matrix->flags &= ~(rwMATRIXINTERNALIDENTITY | rwMATRIXTYPEORTHONORMAL);
            }

            frameList->frames[i] = frame;
            if (frameData.parentIndex >= 0) {
                if (lbl_805EDED0) {
                    fn_80487348(frameList->frames[frameData.parentIndex], frame);
                } else {
                    fn_804872E0(frameList->frames[frameData.parentIndex], frame);
                }
            }
        }
    } else {
        RWERROR((E_RW_BADVERSION));
        return NULL;
    }

    for (i = 0; i < numFrames; i++) {
        RwFrame *frame = frameList->frames[i];

        if (fn_804816AC(&lbl_8056DFE8, stream, frame) == NULL) {
            fn_80487208(frameList->frames[0]);
            RwFree(frameList->frames);
            return NULL;
        }
        if (frame == fn_804872D8(frame) && lbl_805EDED0 == 1) {
            fn_8048722C(frame);
        }
    }
    return frameList;
}
