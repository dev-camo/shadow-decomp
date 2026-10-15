/* RenderWare frame synchronization: the LTMs of dirty hierarchies are rebuilt from their modelling
 * matrices, and the objects attached to their frames are synchronized. The original file name is
 * unknown. */
#include "rw/rwcore.h"
#include <stddef.h>

/* The object a link in a frame's object list belongs to */
#define OBJECTFROMFRAMELINK(link)                                                                  \
    ((RwObjectHasFrame *)((RwUInt8 *)(link) - offsetof(RwObjectHasFrame, lFrame)))

/* Calls the sync function of each object attached to a frame */
#define FRAMESYNCOBJECTS(frame)                                                                    \
    do {                                                                                           \
        RwLLLink *link_;                                                                           \
        RwLLLink *end_ = &(frame)->objectList.link;                                                \
                                                                                                   \
        if ((frame)->objectList.link.next != end_) {                                               \
            link_ = (frame)->objectList.link.next;                                                 \
            while (link_ != end_) {                                                                \
                RwObjectHasFrame *object_ = OBJECTFROMFRAMELINK(link_);                            \
                                                                                                   \
                object_->sync(object_);                                                            \
                link_ = link_->next;                                                               \
            }                                                                                      \
        }                                                                                          \
    } while (0)

/* Synchronizes a list of sibling frames and their descendants: the LTMs where they or an ancestor
 * are marked, and every attached object */
void fn_8048ADE0(RwFrame *frame, RwInt32 flags) {
    while (frame != NULL) {
        RwInt32 accumFlags = flags | frame->object.privateFlags;

        if (accumFlags & rwFRAMEPRIVATESUBTREESYNCLTM) {
            RwFrame *parent = (RwFrame *)frame->object.parent;

            fn_8047EBB8(&frame->ltm, &frame->modelling, &parent->ltm);
        }
        FRAMESYNCOBJECTS(frame);
        frame->object.privateFlags &=
            ~(rwFRAMEPRIVATESUBTREESYNCLTM | rwFRAMEPRIVATESUBTREESYNCOBJ);
        fn_8048ADE0(frame->child, accumFlags);
        frame = frame->next;
    }
}

/* Synchronizes the objects attached to a list of sibling frames and their descendants */
void fn_8048AFF0(RwFrame *frame) {
    while (frame != NULL) {
        FRAMESYNCOBJECTS(frame);
        frame->object.privateFlags &= ~rwFRAMEPRIVATESUBTREESYNCOBJ;
        fn_8048AFF0(frame->child);
        frame = frame->next;
    }
}

/* Synchronizes a dirty hierarchy from its root */
void fn_8048B208(RwFrame *root) {
    RwUInt8 flags = root->object.privateFlags;

    if (flags & rwFRAMEPRIVATEHIERARCHYSYNCLTM) {
        RwInt32 rootFlags = flags & rwFRAMEPRIVATESUBTREESYNCLTM;

        /* The root's LTM is its modelling matrix */
        if (rootFlags) {
            root->ltm = root->modelling;
        }
        FRAMESYNCOBJECTS(root);
        fn_8048ADE0(root->child, rootFlags);
    } else {
        FRAMESYNCOBJECTS(root);
        fn_8048AFF0(root->child);
    }
    root->object.privateFlags =
        flags & ~(rwFRAMEPRIVATEHIERARCHYSYNCLTM | rwFRAMEPRIVATEHIERARCHYSYNCOBJ |
                  rwFRAMEPRIVATESUBTREESYNCLTM | rwFRAMEPRIVATESUBTREESYNCOBJ);
}

/* Synchronizes every dirty hierarchy and empties the dirty list */
RwBool fn_8048B354(void) {
    RwLLLink *end = &lbl_805F15B4->dirtyFrameList.link;
    RwLLLink *link;

    for (link = end->next; link != end; link = link->next) {
        fn_8048B208((RwFrame *)((RwUInt8 *)link - offsetof(RwFrame, inDirtyListLink)));
    }
    rwLinkListInitialize(&lbl_805F15B4->dirtyFrameList);
    return TRUE;
}

/* Rebuilds the LTMs of a list of sibling frames and their descendants where they or an ancestor
 * are marked */
void fn_8048B3C0(RwFrame *frame, RwInt32 flags) {
    while (frame != NULL) {
        RwInt32 accumFlags = flags | frame->object.privateFlags;

        if (accumFlags & rwFRAMEPRIVATESUBTREESYNCLTM) {
            RwFrame *parent = (RwFrame *)frame->object.parent;

            fn_8047EBB8(&frame->ltm, &frame->modelling, &parent->ltm);
            frame->object.privateFlags &= ~rwFRAMEPRIVATESUBTREESYNCLTM;
        }
        fn_8048B3C0(frame->child, accumFlags);
        frame = frame->next;
    }
}

void fn_8048B5D8(RwFrame *root) {
    RwUInt8 flags = root->object.privateFlags;

    if (flags & rwFRAMEPRIVATESUBTREESYNCLTM) {
        root->ltm = root->modelling;
    }
    fn_8048B3C0(root->child, flags);
    root->object.privateFlags =
        flags & ~(rwFRAMEPRIVATEHIERARCHYSYNCLTM | rwFRAMEPRIVATESUBTREESYNCLTM);
}
