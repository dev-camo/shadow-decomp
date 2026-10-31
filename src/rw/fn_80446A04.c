/* RenderWare hierarchical animation plugin (plugin ID 0x11E): the hierarchy data the plugin keeps in
 * frames, its stream format, the hierarchies themselves (created from a free list), and the
 * registration of the standard keyframe scheme. The original file name is unknown. */
#include "rw/rphanim.h"
#include <string.h>
#include "layout_check.h"

#define rwMEMHINT_HANIM (rwMEMHINTDUR_EVENT | rpID_HANIMPLUGIN)

/* The plugin's globals (inferred) */
typedef struct HAnimGlobals {
    /* 0x0 */ RwInt32 frameOffset;           /* frame plugin offset */
    /* 0x4 */ RwFreeList *hierarchyFreeList; /* RpHAnimHierarchy entries */
} HAnimGlobals;
CHECK_SIZEOF(HAnimGlobals, 0x8);

HAnimGlobals lbl_805F1440;
RwFreeList lbl_805E1510;    /* the hierarchy free list */
RwInt32 lbl_805EDD70 = 128; /* hierarchies per free list block */
RwInt32 lbl_805EDD74 = 1;   /* free list blocks allocated up front */

#define HANIMFRAME(frame) RWPLUGINOFFSET(RpHAnimFrameExtension, frame, lbl_805F1440.frameOffset)

/* Finds the nodes of hierarchy that belong to frame and its descendants (an RwFrame callback) */
RwFrame *fn_80446A04(RwFrame *frame, void *data) {
    RwInt32 i;
    RpHAnimHierarchy *hierarchy = data;

    for (i = 0; i < hierarchy->numNodes; i++) {
        if (hierarchy->pNodeInfo[i].nodeID == fn_8044758C(frame)) {
            hierarchy->pNodeInfo[i].pFrame = frame;
        }
    }
    fn_8048722C(frame);
    fn_8048752C(frame, fn_80446A04, hierarchy);
    return frame;
}

/* engine plugin constructor: creates the hierarchy free list and registers the standard keyframe
 * scheme */
void *fn_80446AA8(void *instance, RwInt32 offset, RwInt32 size) {
    RtAnimInterpolatorInfo info;

    lbl_805F1440.hierarchyFreeList =
        fn_8047FC04(sizeof(RpHAnimHierarchy), lbl_805EDD70, 4, lbl_805EDD74, &lbl_805E1510,
                    0x40000 | rpID_HANIMPLUGIN);
    if (lbl_805F1440.hierarchyFreeList == NULL) {
        instance = NULL;
    }

    info.typeID = 1;
    info.interpKeyFrameSize = sizeof(RpHAnimInterpFrame);
    info.animKeyFrameSize = sizeof(RpHAnimKeyFrame);
    /* the toolkit passes the scheme's frames untyped */
    info.keyFrameApplyCB = (RtAnimKeyFrameApplyCallBack)fn_80445CF4;
    info.keyFrameBlendCB = (RtAnimKeyFrameBlendCallBack)fn_8044621C;
    info.keyFrameInterpolateCB = (RtAnimKeyFrameInterpolateCallBack)fn_80445E08;
    info.keyFrameAddCB = (RtAnimKeyFrameAddCallBack)fn_804468D0;
    info.keyFrameMulRecipCB = (RtAnimKeyFrameMulRecipCallBack)fn_80446790;
    info.keyFrameStreamReadCB = fn_80446620;
    info.keyFrameStreamWriteCB = fn_804466DC;
    info.keyFrameStreamGetSizeCB = fn_80446780;
    info.customDataSize = 0;
    fn_80477344(&info);
    return instance;
}

/* engine plugin destructor: destroys the hierarchy free list */
void *fn_80446B90(void *instance, RwInt32 offset, RwInt32 size) {
    if (lbl_805F1440.hierarchyFreeList != NULL) {
        fn_8047FCDC(lbl_805F1440.hierarchyFreeList);
        lbl_805F1440.hierarchyFreeList = NULL;
    }
    return instance;
}

/* frame plugin constructor */
void *fn_80446BE0(void *object, RwInt32 offset, RwInt32 size) {
    RpHAnimFrameExtension *extension = HANIMFRAME(object);

    extension->hierarchy = NULL;
    extension->id = -1;
    return object;
}

/* frame plugin destructor: detaches the frame from its hierarchy's nodes, and destroys the
 * hierarchy if the frame holds it */
void *fn_80446BFC(void *object, RwInt32 offset, RwInt32 size) {
    RpHAnimFrameExtension *extension = HANIMFRAME(object);
    RpHAnimHierarchy *hierarchy = extension->hierarchy;

    if (hierarchy != NULL) {
        RwInt32 i;

        for (i = 0; i < hierarchy->numNodes; i++) {
            hierarchy->pNodeInfo[i].pFrame = NULL;
        }
        if (hierarchy->parentFrame == object) {
            fn_80447418(hierarchy);
        }
        extension->hierarchy = NULL;
    }
    extension->id = -1;
    return object;
}

/* frame plugin copy: copies the ID, and a hierarchy the frame holds (unless it is a sub
 * hierarchy) without its frames */
void *fn_80446C94(void *dstObject, const void *srcObject, RwInt32 offset, RwInt32 size) {
    RpHAnimHierarchy *srcHierarchy;
    RpHAnimFrameExtension *src = HANIMFRAME(srcObject);
    RpHAnimFrameExtension *dst = HANIMFRAME(dstObject);

    srcHierarchy = src->hierarchy;

    dst->id = src->id;
    if (srcHierarchy != NULL && !(srcHierarchy->flags & rpHANIMHIERARCHYSUBHIERARCHY)) {
        RpHAnimHierarchy *hierarchy =
            fn_804472AC(srcHierarchy->numNodes, NULL, NULL, srcHierarchy->flags,
                        srcHierarchy->currentAnim->maxInterpKeyFrameSize);
        RwInt32 i;

        for (i = 0; i < hierarchy->numNodes; i++) {
            hierarchy->pNodeInfo[i].pFrame = NULL;
            hierarchy->pNodeInfo[i].flags = srcHierarchy->pNodeInfo[i].flags;
            hierarchy->pNodeInfo[i].nodeIndex = srcHierarchy->pNodeInfo[i].nodeIndex;
            hierarchy->pNodeInfo[i].nodeID = srcHierarchy->pNodeInfo[i].nodeID;
        }
        dst->hierarchy = hierarchy;
        hierarchy->parentFrame = dstObject;
    }
    return dstObject;
}

/* frame plugin stream write: version 0x100, the ID, then the node count (0 for none) and, for a
 * hierarchy the frame holds, its flags, keyframe size and nodes */
RwStream *fn_80446D84(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {
    RwInt32 version = 0x100;
    RpHAnimFrameExtension *extension;
    RwInt32 i;
    RpHAnimNodeInfo *node;
    RpHAnimHierarchy *hierarchy;

    if (fn_8047D458(stream, &version, sizeof(version)) == NULL) {
        return NULL;
    }
    extension = HANIMFRAME(object);
    if (fn_8047D458(stream, &extension->id, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    hierarchy = extension->hierarchy;
    if (hierarchy != NULL && !(hierarchy->flags & rpHANIMHIERARCHYSUBHIERARCHY)) {
        if (fn_8047D458(stream, &hierarchy->numNodes, sizeof(RwInt32)) == NULL) {
            return NULL;
        }
        if (fn_8047D458(stream, &hierarchy->flags, sizeof(RwInt32)) == NULL) {
            return NULL;
        }
        if (fn_8047D458(stream, &hierarchy->currentAnim->maxInterpKeyFrameSize, sizeof(RwInt32)) ==
            NULL) {
            return NULL;
        }
        node = hierarchy->pNodeInfo;
        for (i = 0; i < hierarchy->numNodes; node++, i++) {
            if (fn_8047D458(stream, &node->nodeID, sizeof(RwInt32)) == NULL) {
                return NULL;
            }
            if (fn_8047D458(stream, &node->nodeIndex, sizeof(RwInt32)) == NULL) {
                return NULL;
            }
            if (fn_8047D458(stream, &node->flags, sizeof(RwInt32)) == NULL) {
                return NULL;
            }
        }
    } else {
        RwInt32 numNodes = 0;

        if (fn_8047D458(stream, &numNodes, sizeof(numNodes)) == NULL) {
            return NULL;
        }
    }
    return stream;
}

/* frame plugin stream read */
RwStream *fn_80446F40(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    RpHAnimFrameExtension *extension = HANIMFRAME(object);
    RwInt32 numNodes;
    RwInt32 version;
    RwInt32 flags;
    RwInt32 maxInterpKeyFrameSize;

    if (fn_8047D868(stream, &version, sizeof(version)) == NULL) {
        return NULL;
    }
    if (version != 0x100) {
        return NULL;
    }
    if (fn_8047D868(stream, &extension->id, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (fn_8047D868(stream, &numNodes, sizeof(numNodes)) == NULL) {
        return NULL;
    }
    if (numNodes > 0) {
        RpHAnimHierarchy *hierarchy;

        if (fn_8047D868(stream, &flags, sizeof(flags)) == NULL) {
            return NULL;
        }
        if (fn_8047D868(stream, &maxInterpKeyFrameSize, sizeof(maxInterpKeyFrameSize)) == NULL) {
            return NULL;
        }
        hierarchy = RwFreeListAlloc(lbl_805F1440.hierarchyFreeList, rwMEMHINT_HANIM);
        memset(hierarchy, 0, sizeof(RpHAnimHierarchy));
        if (hierarchy != NULL) {
            RwInt32 i;
            RpHAnimNodeInfo *node;

            hierarchy->currentAnim = fn_804779DC(numNodes, maxInterpKeyFrameSize);
            hierarchy->flags = flags;
            hierarchy->parentFrame = object;
            hierarchy->numNodes = numNodes;
            hierarchy->parentHierarchy = hierarchy;
            if (hierarchy->flags & rpHANIMHIERARCHYNOMATRICES) {
                hierarchy->pMatrixArray = NULL;
                hierarchy->pMatrixArrayUnaligned = NULL;
            } else {
                void *matrices = RwMalloc(numNodes * sizeof(RwMatrix) + 15, rwMEMHINT_HANIM);

                hierarchy->pMatrixArray = (RwMatrix *)(((RwUInt32)matrices + 15) & ~15);
                hierarchy->pMatrixArrayUnaligned = matrices;
            }
            hierarchy->pNodeInfo = RwMalloc(numNodes * sizeof(RpHAnimNodeInfo), rwMEMHINT_HANIM);

            node = hierarchy->pNodeInfo;
            for (i = 0; i < hierarchy->numNodes; node++, i++) {
                if (fn_8047D868(stream, &node->nodeID, sizeof(RwInt32)) == NULL) {
                    return NULL;
                }
                if (fn_8047D868(stream, &node->nodeIndex, sizeof(RwInt32)) == NULL) {
                    return NULL;
                }
                if (fn_8047D868(stream, &node->flags, sizeof(RwInt32)) == NULL) {
                    return NULL;
                }
                node->pFrame = NULL;
            }
        }
        extension->hierarchy = hierarchy;
    }
    return stream;
}

/* frame plugin stream size: nothing for a frame without an ID or hierarchy */
RwInt32 fn_8044719C(const void *object, RwInt32 offset, RwInt32 size) {
    RpHAnimFrameExtension *extension = HANIMFRAME(object);
    RwBool hasData = extension->id != -1 || extension->hierarchy != NULL;

    if (hasData) {
        RpHAnimHierarchy *hierarchy = extension->hierarchy;
        RwInt32 streamSize = 3 * sizeof(RwInt32);

        if (hierarchy != NULL && !(hierarchy->flags & rpHANIMHIERARCHYSUBHIERARCHY)) {
            streamSize += 2 * sizeof(RwInt32) + hierarchy->numNodes * (3 * sizeof(RwInt32));
        }
        return streamSize;
    }
    return 0;
}

/* Attaches the plugin to the engine and to frames and registers its stream callbacks */
RwBool fn_80447200(void) {
    RwInt32 streamOffset;

    if (fn_80485D78(0, rpID_HANIMPLUGIN, fn_80446AA8, fn_80446B90) < 0) {
        return FALSE;
    }
    lbl_805F1440.frameOffset = fn_804878D4(sizeof(RpHAnimFrameExtension), rpID_HANIMPLUGIN,
                                           fn_80446BE0, fn_80446BFC, fn_80446C94);
    streamOffset = fn_804836EC(rpID_HANIMPLUGIN, fn_80446F40, fn_80446D84, fn_8044719C);
    return streamOffset >= 0 && lbl_805F1440.frameOffset >= 0;
}

/* Creates a hierarchy of numNodes nodes with the given node flags and IDs (either may be NULL) */
RpHAnimHierarchy *fn_804472AC(RwInt32 numNodes, RwUInt32 *nodeFlags, RwInt32 *nodeIDs,
                              RwInt32 flags, RwInt32 maxInterpKeyFrameSize) {
    RpHAnimHierarchy *hierarchy = RwFreeListAlloc(lbl_805F1440.hierarchyFreeList, rwMEMHINT_HANIM);
    RwInt32 i;

    hierarchy->currentAnim = fn_804779DC(numNodes, maxInterpKeyFrameSize);
    hierarchy->flags = flags;
    hierarchy->numNodes = numNodes;
    hierarchy->parentFrame = NULL;
    if (!(flags & rpHANIMHIERARCHYNOMATRICES)) {
        void *matrices = RwMalloc(numNodes * sizeof(RwMatrix) + 15, rwMEMHINT_HANIM);

        hierarchy->pMatrixArray = (RwMatrix *)(((RwUInt32)matrices + 15) & ~15);
        hierarchy->pMatrixArrayUnaligned = matrices;
    } else {
        hierarchy->pMatrixArray = NULL;
        hierarchy->pMatrixArrayUnaligned = NULL;
    }
    hierarchy->pNodeInfo = RwMalloc(numNodes * sizeof(RpHAnimNodeInfo), rwMEMHINT_HANIM);
    for (i = 0; i < numNodes; i++) {
        hierarchy->pNodeInfo[i].pFrame = NULL;
        if (nodeIDs != NULL) {
            hierarchy->pNodeInfo[i].nodeID = nodeIDs[i];
        }
        hierarchy->pNodeInfo[i].nodeIndex = i;
        if (nodeFlags != NULL) {
            hierarchy->pNodeInfo[i].flags = nodeFlags[i];
        }
    }
    hierarchy->parentHierarchy = hierarchy;
    return hierarchy;
}

/* Destroys a hierarchy and detaches it from the frame that holds it */
RpHAnimHierarchy *fn_80447418(RpHAnimHierarchy *hierarchy) {
    RwFrame *parentFrame = hierarchy->parentFrame;

    if (!(hierarchy->flags & rpHANIMHIERARCHYSUBHIERARCHY)) {
        if (!(hierarchy->flags & rpHANIMHIERARCHYNOMATRICES)) {
            RwFree(hierarchy->pMatrixArrayUnaligned);
        }
        RwFree(hierarchy->pNodeInfo);
    }
    hierarchy->pMatrixArrayUnaligned = NULL;
    hierarchy->pMatrixArray = NULL;
    hierarchy->pNodeInfo = NULL;
    fn_80477A8C(hierarchy->currentAnim);
    RwFreeListFree(lbl_805F1440.hierarchyFreeList, hierarchy);
    if (parentFrame != NULL) {
        HANIMFRAME(parentFrame)->hierarchy = NULL;
    }
    return NULL;
}

/* Attaches a hierarchy to the frames below its parent frame by their IDs */
RpHAnimHierarchy *fn_804474D8(RpHAnimHierarchy *hierarchy) {
    RwFrame *frame = hierarchy->parentFrame;
    RwInt32 i;

    for (i = 0; i < hierarchy->numNodes; i++) {
        if (hierarchy->pNodeInfo[i].nodeID == fn_8044758C(frame)) {
            hierarchy->pNodeInfo[i].pFrame = frame;
        }
    }
    fn_8048722C(frame);
    fn_8048752C(frame, fn_80446A04, hierarchy);
    return hierarchy;
}

/* The hierarchy a frame holds */
RpHAnimHierarchy *fn_8044757C(RwFrame *frame) {
    return HANIMFRAME(frame)->hierarchy;
}

/* The ID of a frame */
RwInt32 fn_8044758C(RwFrame *frame) {
    return HANIMFRAME(frame)->id;
}
