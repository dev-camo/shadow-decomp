/* RenderWare frames: the frame module, creating, cloning and destroying frames and hierarchies,
 * adding and removing children, transforming frames and marking them dirty. The original file
 * name is unknown. */
#include "rw/rwcore.h"
#include "layout_check.h"

/* object type and free list ID */
#define rwFRAME 0

/* The engine globals of the frame module */
typedef struct rwFrameGlobals {
    /* 0x0 */ RwFreeList *frameFreeList;
} rwFrameGlobals;
CHECK_SIZEOF(rwFrameGlobals, 0x4);

#define RWFRAMEGLOBAL(var)                                                                         \
    (RWPLUGINOFFSET(rwFrameGlobals, lbl_805F15B4, lbl_805F15B8.globalsOffset)->var)

/* The frame plugin registry */
RwPluginRegistry lbl_8056DFE8 = {sizeof(RwFrame), sizeof(RwFrame), 0, 0, NULL, NULL};

RwFreeList lbl_805E2C78; /* the frame free list */

RwInt32 lbl_805EDEE0 = 50; /* frames per free list block */
RwInt32 lbl_805EDEE4 = 1;  /* free list blocks to preallocate */

RwModuleInfo lbl_805F15B8; /* the frame module */

/* Opens the frame module: its free list of frames with their plugins' data, and the dirty frame
 * list */
void *fn_80486888(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F15B8.globalsOffset = offset;
    RWFRAMEGLOBAL(frameFreeList) =
        fn_8047FC04(lbl_8056DFE8.sizeOfStruct, lbl_805EDEE0, 4, lbl_805EDEE4, &lbl_805E2C78,
                    rwMEMHINTDUR_GLOBAL | rwID_FRAMELIST);
    if (RWFRAMEGLOBAL(frameFreeList) == NULL) {
        return NULL;
    }
    rwLinkListInitialize(&lbl_805F15B4->dirtyFrameList);
    lbl_805F15B8.numInstances++;
    return instance;
}

void *fn_8048692C(void *instance, RwInt32 offset, RwInt32 size) {
    if (RWFRAMEGLOBAL(frameFreeList) != NULL) {
        fn_8047FCDC(RWFRAMEGLOBAL(frameFreeList));
        RWFRAMEGLOBAL(frameFreeList) = NULL;
    }
    lbl_805F15B8.numInstances--;
    return instance;
}

/* Sets the root of a frame and its descendants */
void fn_80486990(RwFrame *frame, RwFrame *root) {
    frame->root = root;
    frame = frame->child;
    while (frame != NULL) {
        fn_80486990(frame, root);
        frame = frame->next;
    }
}

/* Clones a frame and its descendants into the hierarchy under root (or a new hierarchy when root
 * is NULL). Each original frame's root is left pointing at its clone, which fn_80486CA4 undoes. */
RwFrame *fn_80486AA4(RwFrame *oldFrame, RwFrame *root) {
    RwFrame *newFrame;
    RwFrame *child;

    newFrame = (RwFrame *)RwFreeListAlloc(RWFRAMEGLOBAL(frameFreeList),
                                          rwMEMHINTDUR_EVENT | rwID_FRAMELIST);
    if (newFrame == NULL) {
        return NULL;
    }
    if (root == NULL) {
        root = newFrame;
    }

    RWOBJECTCOPY(&newFrame->object, &oldFrame->object);
    rwLinkListInitialize(&newFrame->objectList);
    newFrame->object.parent = NULL; /* stored again, as in the original */
    newFrame->modelling = oldFrame->modelling;
    newFrame->child = NULL;
    newFrame->next = NULL;
    newFrame->root = root;
    oldFrame->root = newFrame;

    child = oldFrame->child;
    while (child != NULL) {
        RwFrame *newChild = fn_80486AA4(child, root);

        if (newChild == NULL) {
            fn_80487208(newFrame);
            return NULL;
        }
        newChild->next = newFrame->child;
        newFrame->child = newChild;
        newChild->object.parent = newFrame;
        child = child->next;
    }

    fn_8048209C(&lbl_8056DFE8, newFrame);
    fn_804821B0(&lbl_8056DFE8, newFrame, oldFrame);
    return newFrame;
}

/* Clones a hierarchy; the clone needs synchronizing */
RwFrame *fn_80486C5C(RwFrame *root) {
    RwFrame *clone;

    clone = fn_80486AA4(root, NULL);
    if (clone != NULL) {
        clone->object.privateFlags &=
            ~(rwFRAMEPRIVATEHIERARCHYSYNCLTM | rwFRAMEPRIVATEHIERARCHYSYNCOBJ);
        fn_8048722C(clone);
    }
    return clone;
}

/* Points the frames of a cloned hierarchy at their real root again */
RwFrame *fn_80486CA4(RwFrame *root) {
    RwFrame *parent = (RwFrame *)root->object.parent;

    if (parent != NULL) {
        fn_80486990(root, parent->root);
    } else {
        fn_80486990(root, root);
    }
    return root;
}

RwBool fn_80486CF0(RwFrame *frame) {
    return frame->root->object.privateFlags &
           (rwFRAMEPRIVATEHIERARCHYSYNCLTM | rwFRAMEPRIVATEHIERARCHYSYNCOBJ);
}

/* Creates a frame: an identity modelling matrix and LTM, the root of its own hierarchy */
RwFrame *fn_80486D00(void) {
    RwFrame *frame;

    frame = (RwFrame *)RwFreeListAlloc(RWFRAMEGLOBAL(frameFreeList),
                                       rwMEMHINTDUR_EVENT | rwID_FRAMELIST);
    if (frame == NULL) {
        return NULL;
    }

    RWOBJECTINITIALIZE(&frame->object, rwFRAME, 0);
    rwLinkListInitialize(&frame->objectList);

    frame->modelling.flags = rwMATRIXTYPEORTHONORMAL;
    RwMatrixSetIdentity(&frame->modelling);
    frame->ltm.flags = rwMATRIXTYPEORTHONORMAL;
    RwMatrixSetIdentity(&frame->ltm);

    frame->child = NULL;
    frame->next = NULL;
    frame->root = frame;

    fn_8048209C(&lbl_8056DFE8, frame);
    return frame;
}

/* Destroys a frame: it leaves its parent and the dirty list, and its children's parent pointers
 * are cleared. The children's cached root pointers, and their descendants', are left unchanged. */
RwBool fn_80486E2C(RwFrame *frame) {
    RwFrame *child;

    fn_80482144(&lbl_8056DFE8, frame);
    if (frame->object.parent != NULL) {
        fn_80487448(frame);
    }
    if (frame->object.privateFlags &
        (rwFRAMEPRIVATEHIERARCHYSYNCLTM | rwFRAMEPRIVATEHIERARCHYSYNCOBJ)) {
        rwLinkListRemoveLLLink(&frame->inDirtyListLink);
    }
    for (child = frame->child; child != NULL; child = child->next) {
        child->object.parent = NULL;
    }
    RwFreeListFree(RWFRAMEGLOBAL(frameFreeList), frame);
    return TRUE;
}

/* Deinitializes a frame of a hierarchy being destroyed */
void fn_80486ED8(RwFrame *frame) {
    fn_80482144(&lbl_8056DFE8, frame);
    if (frame->object.privateFlags &
        (rwFRAMEPRIVATEHIERARCHYSYNCLTM | rwFRAMEPRIVATEHIERARCHYSYNCOBJ)) {
        rwLinkListRemoveLLLink(&frame->inDirtyListLink);
    }
}

/* Deinitializes and frees a frame of a hierarchy being destroyed */
void fn_80486F34(RwFrame *frame) {
    fn_80486ED8(frame);
    RwFreeListFree(RWFRAMEGLOBAL(frameFreeList), frame);
}

/* Destroys a frame and its descendants */
void fn_80486FAC(RwFrame *frame) {
    if (frame != NULL) {
        RwFrame *child = frame->child;

        while (child != NULL) {
            RwFrame *next = child->next;

            fn_80486FAC(child);
            child = next;
        }
        fn_80486F34(frame);
    }
}

RwBool fn_80487208(RwFrame *frame) {
    fn_80486FAC(frame);
    return TRUE;
}

/* Marks a frame's hierarchy for synchronizing: its root joins the dirty list, and the frame's
 * subtree is marked */
RwFrame *fn_8048722C(RwFrame *frame) {
    RwUInt8 oldFlags = frame->root->object.privateFlags;

    if (!(oldFlags & (rwFRAMEPRIVATEHIERARCHYSYNCLTM | rwFRAMEPRIVATEHIERARCHYSYNCOBJ))) {
        rwLinkListAddLLLink(&lbl_805F15B4->dirtyFrameList, &frame->root->inDirtyListLink);
    }
    frame->root->object.privateFlags =
        oldFlags | rwFRAMEPRIVATEHIERARCHYSYNCLTM | rwFRAMEPRIVATEHIERARCHYSYNCOBJ;
    frame->object.privateFlags |= rwFRAMEPRIVATESUBTREESYNCLTM | rwFRAMEPRIVATESUBTREESYNCOBJ;
    return frame;
}

/* The frame's LTM, after synchronizing its hierarchy's LTMs if they are dirty */
RwMatrix *RwFrameGetLTM(RwFrame *frame) {
    if (frame->root->object.privateFlags & rwFRAMEPRIVATEHIERARCHYSYNCLTM) {
        fn_8048B5D8(frame->root);
    }
    return &frame->ltm;
}

RwFrame *fn_804872D8(RwFrame *frame) {
    return frame->root;
}

/* Adds a child without marking it dirty; the child leaves its old parent first */
RwFrame *fn_804872E0(RwFrame *parent, RwFrame *child) {
    if (child->object.parent != NULL) {
        fn_80487448(child);
    }
    child->next = parent->child;
    parent->child = child;
    child->object.parent = parent;
    fn_80486990(child, parent->root);
    return parent;
}

/* Adds a child; the child leaves its old parent and the dirty list, and is marked dirty in its new
 * hierarchy */
RwFrame *fn_80487348(RwFrame *parent, RwFrame *child) {
    if (child->object.parent != NULL) {
        fn_80487448(child);
    }
    child->next = parent->child;
    parent->child = child;
    child->object.parent = parent;
    fn_80486990(child, parent->root);

    if (child->object.privateFlags &
        (rwFRAMEPRIVATEHIERARCHYSYNCLTM | rwFRAMEPRIVATEHIERARCHYSYNCOBJ)) {
        rwLinkListRemoveLLLink(&child->inDirtyListLink);
        child->object.privateFlags &=
            ~(rwFRAMEPRIVATEHIERARCHYSYNCLTM | rwFRAMEPRIVATEHIERARCHYSYNCOBJ);
    }
    fn_8048722C(child);
    return parent;
}

/* Removes a frame from its parent's children; it becomes the dirty root of its own hierarchy */
RwFrame *fn_80487448(RwFrame *oldChild) {
    RwFrame *parent = (RwFrame *)oldChild->object.parent;
    RwFrame *current = parent->child;

    if (current == oldChild) {
        parent->child = oldChild->next;
    } else {
        while (current->next != oldChild) {
            current = current->next;
        }
        current->next = oldChild->next;
    }

    oldChild->object.parent = NULL;
    oldChild->next = NULL;
    fn_80486990(oldChild, oldChild);
    fn_8048722C(oldChild);
    return oldChild;
}

RwFrame *fn_8048752C(RwFrame *frame, RwFrameCallBack callBack, void *data) {
    RwFrame *child = frame->child;

    while (child != NULL) {
        RwFrame *next = child->next;

        if (callBack(child, data) == NULL) {
            return frame;
        }
        child = next;
    }
    return frame;
}

RwFrame *fn_804875B0(RwFrame *frame, const RwV3d *translation, RwInt32 combine) {
    fn_8047F59C(&frame->modelling, translation, combine);
    fn_8048722C(frame);
    return frame;
}

RwFrame *fn_8048764C(RwFrame *frame, const RwV3d *scale, RwInt32 combine) {
    fn_8047F398(&frame->modelling, scale, combine);
    fn_8048722C(frame);
    return frame;
}

RwFrame *fn_804876E8(RwFrame *frame, const RwMatrix *transform, RwInt32 combine) {
    fn_8047F72C(&frame->modelling, transform, combine);
    fn_8048722C(frame);
    return frame;
}

RwFrame *fn_80487784(RwFrame *frame, const RwV3d *axis, RwReal angle, RwInt32 combine) {
    fn_8047EFC8(&frame->modelling, axis, angle, combine);
    fn_8048722C(frame);
    return frame;
}

RwFrame *fn_80487820(RwFrame *frame) {
    RwMatrixSetIdentity(&frame->modelling);
    fn_8048722C(frame);
    return frame;
}

RwInt32 fn_804878D4(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB,
                    RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB) {
    return fn_80481DC8(&lbl_8056DFE8, size, pluginID, constructCB, destructCB, copyCB);
}
