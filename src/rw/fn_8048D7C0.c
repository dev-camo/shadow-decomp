/* RenderWare objects attached to frames: attaching an object to a frame and detaching it. The
 * original file name is unknown. */
#include "rw/rwcore.h"

/* Attaches an object to a frame (or to none): it leaves its old frame's object list and joins the
 * new frame's, whose hierarchy is marked for synchronizing */
void fn_8048D7C0(void *object, RwFrame *frame) {
    RwObjectHasFrame *objectHasFrame = (RwObjectHasFrame *)object;

    if (objectHasFrame->object.parent != NULL) {
        rwLinkListRemoveLLLink(&objectHasFrame->lFrame);
    }
    objectHasFrame->object.parent = frame;
    if (frame != NULL) {
        rwLinkListAddLLLink(&frame->objectList, &objectHasFrame->lFrame);
        fn_8048722C(frame);
    }
}

/* Detaches an object from its frame's object list */
void fn_8048D834(void *object) {
    RwObjectHasFrame *objectHasFrame = (RwObjectHasFrame *)object;

    if (objectHasFrame->object.parent != NULL) {
        rwLinkListRemoveLLLink(&objectHasFrame->lFrame);
    }
}
