/* RenderWare delta morph animations (plugin ID 0x122): destroying an animation and reading one
 * from its stream chunk (0x1E). The original file name is unknown. */
#include "rw/rpdmorph.h"
#include <string.h>

#define rwMEMHINT_DMORPHANIMATION 0x3001E
#define rwMEMHINT_DMORPHFRAMES 0x30122

/* Reports an error of this plugin; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rpID_DMORPHPLUGIN;                                                       \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

/* Allocates an animation with numDMorphTargets empty frame lists after it */
static inline RpDMorphAnimation *DMorphAnimationCreate(RwInt32 numDMorphTargets) {
    RpDMorphAnimation *animation;
    RwUInt32 size = numDMorphTargets * sizeof(RpDMorphFrameList) + sizeof(RpDMorphAnimation);
    RwUInt32 i;
    RpDMorphFrameList *frameList;

    animation = RwMalloc(size, rwMEMHINT_DMORPHANIMATION);
    if (animation == NULL) {
        RWERROR((0x80000013, size));
        return NULL;
    }
    animation->numDMorphTargets = numDMorphTargets;
    animation->frameLists = (RpDMorphFrameList *)(animation + 1);
    frameList = animation->frameLists;
    for (i = 0; i < numDMorphTargets; frameList++, i++) {
        frameList->numFrames = 0;
        frameList->frames = NULL;
    }
    return animation;
}

/* Allocates numFrames unlinked frames for the frame list of target index */
static inline RpDMorphAnimation *DMorphAnimationCreateFrames(RpDMorphAnimation *animation,
                                                             RwUInt32 index, RwUInt32 numFrames) {
    RpDMorphFrameList *frameList = &animation->frameLists[index];
    RwUInt32 size = numFrames * sizeof(RpDMorphFrame);
    RwUInt32 i;

    frameList->frames = RwMalloc(size, rwMEMHINT_DMORPHFRAMES);
    if (frameList->frames == NULL) {
        RWERROR((0x80000013, size));
        return NULL;
    }
    memset(frameList->frames, 0, size);
    for (i = 0; i < numFrames; i++) {
        frameList->frames[i].nextFrame = rpDMORPHNOFRAME;
    }
    frameList->numFrames = numFrames;
    return animation;
}

/* Destroys an animation */
RpDMorphAnimation *fn_80442B98(RpDMorphAnimation *animation) {
    RwUInt32 i;
    RpDMorphFrameList *frameList = animation->frameLists;

    for (i = 0; i < animation->numDMorphTargets; frameList++, i++) {
        if (frameList->frames != NULL) {
            RwFree(frameList->frames);
        }
    }
    RwFree(animation);
    return NULL;
}

/* Reads an animation */
RpDMorphAnimation *fn_80442C24(RwStream *stream) {
    RwUInt32 length;
    RwUInt32 version;
    RwUInt32 formatVersion;
    RwInt32 field_0x4;
    RwUInt32 numDMorphTargets;
    RwInt32 field_0xC;
    RwUInt32 numFrames;
    RpDMorphAnimation *animation;
    RwUInt32 i;

    if (!fn_8047CF50(stream, rwID_STRUCT, &length, &version)) {
        return NULL;
    }
    if (fn_8047D868(stream, (RwInt32 *)&formatVersion, sizeof(formatVersion)) == NULL ||
        fn_8047D868(stream, &field_0x4, sizeof(field_0x4)) == NULL ||
        fn_8047D868(stream, (RwInt32 *)&numDMorphTargets, sizeof(numDMorphTargets)) == NULL ||
        fn_8047D868(stream, &field_0xC, sizeof(field_0xC)) == NULL) {
        return NULL;
    }
    if (formatVersion > 0x100) {
        RWERROR((0x80000004));
        return NULL;
    }
    animation = DMorphAnimationCreate(numDMorphTargets);
    if (animation == NULL) {
        return NULL;
    }
    for (i = 0; i < numDMorphTargets; i++) {
        RpDMorphFrame *frame;
        RwUInt32 j;

        if (fn_8047D868(stream, (RwInt32 *)&numFrames, sizeof(numFrames)) == NULL) {
            return NULL;
        }
        if (numFrames == 0) {
            continue;
        }
        if (DMorphAnimationCreateFrames(animation, i, numFrames) == NULL) {
            return NULL;
        }
        frame = animation->frameLists[i].frames;
        for (j = 0; j < numFrames; frame++, j++) {
            if (fn_8047D784(stream, &frame->startValue, 4 * sizeof(RwReal)) == NULL ||
                fn_8047D868(stream, (RwInt32 *)&frame->nextFrame, sizeof(frame->nextFrame)) ==
                    NULL) {
                return NULL;
            }
        }
    }
    return animation;
}
