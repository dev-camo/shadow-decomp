/* RenderWare UV animation plugin (plugin ID 0x135): the engine plugin (the custom data free list,
 * the dictionary schema and the keyframe schemes), the material plugin data and its stream, the
 * animations' custom data, and playing the animations on materials. The original file name is
 * unknown. */
#include "rw/rpmatfx.h"
#include "rw/rpuvanim.h"
#include <string.h>
#include "layout_check.h"

/* The plugin's engine globals (inferred) */
typedef struct UVAnimEngineGlobals {
    /* 0x0 */ RwInt32 field_0x0;
    /* 0x4 */ RwInt32 numInstances; /* the open engine instances */
} UVAnimEngineGlobals;
CHECK_SIZEOF(UVAnimEngineGlobals, 0x8);

UVAnimEngineGlobals lbl_805EDD88 = {0, 0};
RwInt32 lbl_805EDD90 = 128; /* custom data free list entries per block */
RwInt32 lbl_805EDD94 = 1;   /* custom data free list blocks to preallocate */

/* declared in reverse address order (the compiler places them in .sbss last first) */
RwInt32 lbl_805F149C;
RwFreeList *lbl_805F1498; /* the custom data free list */

const RwV3d lbl_8051BD40 = {-0.5f, -0.5f, 0.0f};
const RwV3d lbl_8051BD4C = {0.5f, 0.5f, 0.0f};

RtDictSchema lbl_8056D748 = {
    "UVAnim",
    0x2B, /* dictionary chunk */
    0x1B, /* animation chunk */
    0x36000,
    NULL,
    NULL,
    (RtDictEntryAddRefCallBack)fn_80454618,
    (RtDictEntryDestroyCallBack)fn_8045459C,
    (RtDictEntryGetNameCallBack)fn_804548E4,
    (RtDictEntryStreamGetSizeCallBack)fn_80477978,
    (RtDictEntryStreamReadCallBack)fn_804775CC,
    NULL,
    (RtDictEntryStreamWriteCallBack)fn_80477860,
};

RtAnimInterpolatorInfo lbl_8056D77C = {
    rpUVANIMLINEARKEYFRAMES,
    sizeof(RpUVAnimInterpFrame),
    sizeof(RpUVAnimKeyFrame),
    fn_804532E4,
    fn_804533E0,
    fn_80453350,
    fn_80453500,
    fn_8045345C,
    fn_804530CC,
    fn_804531B0,
    fn_804532A4,
    sizeof(RpUVAnimCustomData),
};

RtAnimInterpolatorInfo lbl_8056D7AC = {
    rpUVANIMPARAMKEYFRAMES,
    sizeof(RpUVAnimInterpFrame),
    sizeof(RpUVAnimKeyFrame),
    fn_804535C8,
    fn_80453770,
    fn_804536B4,
    fn_8045381C,
    fn_80453818,
    fn_804530CC,
    fn_804531B0,
    fn_804532A4,
    sizeof(RpUVAnimCustomData),
};

/* the material plugin callbacks, defined after the attachment */
void *fn_80453B24(void *object, RwInt32 offset, RwInt32 size);
void *fn_80453B64(void *object, RwInt32 offset, RwInt32 size);
void *fn_80453C04(void *dst, const void *src, RwInt32 offset, RwInt32 size);
RwStream *fn_80453E24(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size);
RwStream *fn_80453FB0(RwStream *stream, RwInt32 length, void *object, RwInt32 offset, RwInt32 size);
RwInt32 fn_80454294(const void *object, RwInt32 offset, RwInt32 size);

/* engine plugin constructor */
void *fn_80453980(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F1498 = fn_8047FC04(sizeof(RpUVAnimCustomData), lbl_805EDD90, 4, lbl_805EDD94,
                               lbl_805F1498, rwMEMHINTDUR_GLOBAL | rpID_UVANIMPLUGIN);
    if (lbl_805F1498 == NULL) {
        instance = NULL;
    }
    fn_80479040(&lbl_8056D748);
    fn_80477344(&lbl_8056D77C);
    fn_80477344(&lbl_8056D7AC);
    lbl_805EDD88.numInstances++;
    return instance;
}

/* engine plugin destructor */
void *fn_80453A14(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805EDD88.numInstances--;
    fn_8047909C(&lbl_8056D748);
    if (lbl_805F1498 != NULL) {
        fn_8047FCDC(lbl_805F1498);
        lbl_805F1498 = NULL;
    }
    return instance;
}

RwBool fn_80453A78(void) {
    RwInt32 streamOffset;

    if (fn_80485D78(0, rpID_UVANIMPLUGIN, fn_80453980, fn_80453A14) < 0) {
        return FALSE;
    }
    lbl_805F149C = fn_8045B2CC(sizeof(RpUVAnimMaterialExtension), rpID_UVANIMPLUGIN, fn_80453B24,
                               fn_80453B64, fn_80453C04);
    streamOffset = fn_8045B30C(rpID_UVANIMPLUGIN, fn_80453FB0, fn_80453E24, fn_80454294);
    return streamOffset >= 0 && lbl_805F149C >= 0;
}

/* material plugin constructor */
void *fn_80453B24(void *object, RwInt32 offset, RwInt32 size) {
    memset(RPUVANIMMATERIALGETEXT(object), 0, sizeof(RpUVAnimMaterialExtension));
    return object;
}

/* material plugin destructor */
void *fn_80453B64(void *object, RwInt32 offset, RwInt32 size) {
    RpUVAnimMaterialExtension *extension = RPUVANIMMATERIALGETEXT(object);
    RwUInt32 i;
    RtAnimInterpolator *interp;

    for (i = 0; i < rpUVANIMNUMCHANNELS; i++) {
        if (extension->uv[i] != NULL) {
            fn_8047F958(extension->uv[i]);
        }
        extension->uv[i] = NULL;
    }
    for (i = 0; i < rpUVANIMMAXSLOTS; i++) {
        interp = extension->interp[i];
        if (interp != NULL) {
            fn_8045459C(interp->pCurrentAnim);
            fn_80477A8C(interp);
        }
    }
    return object;
}

/* material plugin copy: allocates an interpolator for each slot that has one, copies the source's
 * interpolated frame data into it and adds a reference to the source animation. The animation
 * association, time and callbacks are not copied: the new interpolators have no current
 * animation, so unless one is set later, the destructor and this function's failure path pass
 * that NULL to fn_8045459C. */
void *fn_80453C04(void *dst, const void *src, RwInt32 offset, RwInt32 size) {
    RpUVAnimMaterialExtension *srcExtension = RPUVANIMMATERIALGETEXT(src);
    RpUVAnimMaterialExtension *dstExtension = RPUVANIMMATERIALGETEXT(dst);
    RwUInt32 i;
    RwBool copied = TRUE;
    RtAnimInterpolator *srcInterp;
    RtAnimAnimation *animation;
    RwUInt32 j;

    for (i = 0; i < rpUVANIMMAXSLOTS; i++) {
        srcInterp = srcExtension->interp[i];
        if (srcInterp != NULL) {
            animation = srcInterp->pCurrentAnim;
            dstExtension->interp[i] =
                fn_804779DC(fn_804779A8(animation), animation->interpInfo->interpKeyFrameSize);
            if (dstExtension->interp[i] != NULL) {
                fn_80477BF0(dstExtension->interp[i], srcInterp);
                fn_80454618(animation);
            } else {
                copied = FALSE;
                break;
            }
        } else {
            dstExtension->interp[i] = NULL;
        }
    }

    if (!copied) {
        for (j = 0; j < i; j++) {
            if (dstExtension->interp[j] != NULL) {
                fn_8045459C(dstExtension->interp[j]->pCurrentAnim);
                fn_80477A8C(dstExtension->interp[j]);
            }
        }
        return NULL;
    }
    return dst;
}

/* Creates the UV channel matrices that some animation node drives and destroys the others */
RpUVAnimMaterialExtension *fn_80453D08(RpUVAnimMaterialExtension *extension) {
    RpUVAnimCustomData *customData;
    RwUInt32 channel;
    RwUInt32 slot;
    RwUInt32 node;
    RwBool used;
    RtAnimInterpolator *interp;

    for (channel = 0; channel < rpUVANIMNUMCHANNELS; channel++) {
        /* look through every node of every animation for one that drives the channel */
        slot = 0;
        node = 0;
        used = FALSE;
        while (slot < rpUVANIMMAXSLOTS) {
            interp = extension->interp[slot];
            if (interp != NULL) {
                customData = interp->pCurrentAnim->customData;
                if (channel == customData->nodeToUVChannelMap[node]) {
                    used = TRUE;
                    break;
                }
                node++;
                if (node >= interp->numNodes) {
                    node = 0;
                    slot++;
                }
            } else {
                slot++;
            }
        }

        if (used && extension->uv[channel] == NULL) {
            extension->uv[channel] = fn_8047F994();
            if (extension->uv[channel] == NULL) {
                return NULL;
            }
        } else if (!used && extension->uv[channel] != NULL) {
            fn_8047F958(extension->uv[channel]);
            extension->uv[channel] = NULL;
        }
    }
    return extension;
}

/* material stream write: a struct chunk with a mask of the playing slots, then each slot's
 * animation name */
RwStream *fn_80453E24(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {
    RpUVAnimMaterialExtension *extension = RPUVANIMMATERIALGETEXT(object);
    RwInt32 slots = 0;
    RwInt32 chunkSize = sizeof(RwInt32);
    RwChar *name;
    RwUInt32 i;

    for (i = 0; i < rpUVANIMMAXSLOTS; i++) {
        if (extension->interp[i] != NULL) {
            slots |= 1 << i;
            chunkSize += rpUVANIMNAMELENGTH;
        }
    }
    if (fn_8047CEF0(stream, rwID_STRUCT, chunkSize, RW_STREAM_VERSION, RW_STREAM_BUILD) == NULL) {
        return NULL;
    }
    if (fn_8047D458(stream, &slots, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    for (i = 0; i < rpUVANIMMAXSLOTS; i++) {
        if (extension->interp[i] != NULL) {
            name = fn_804548E4(extension->interp[i]->pCurrentAnim);
            fn_80480ED0(stream, name, rpUVANIMNAMELENGTH);
        }
    }
    return stream;
}

/* Creates a one-node linear animation that holds the identity for two keyframes, standing in for
 * an animation missing from the dictionary */
static inline RtAnimAnimation *UVAnimCreateIdentity(const RwChar *name) {
    RwUInt32 nodeToUVChannelMap[1] = {0};
    RtAnimAnimation *animation;
    RpUVAnimKeyFrame *frames;
    RpUVAnimKeyFrame *second;
    RwMatrix identity;

    animation = fn_8045449C(name, 1, 2, 1.0f, nodeToUVChannelMap, rpUVANIMLINEAR);
    if (animation == NULL) {
        return NULL;
    }
    frames = animation->pFrames;
    second = (RpUVAnimKeyFrame *)((RwUInt8 *)frames + animation->interpInfo->animKeyFrameSize);
    RwMatrixSetIdentity(&identity);
    fn_80453018(animation, frames, NULL, 0.0f, &identity);
    fn_80453018(animation, second, frames, 1.0f, &identity);
    return animation;
}

/* material stream read: finds each named animation in the current dictionary, or stands in an
 * identity animation for it, and plays it in its slot */
RwStream *fn_80453FB0(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    RpUVAnimMaterialExtension *extension = RPUVANIMMATERIALGETEXT(object);
    RwChar name[rpUVANIMNAMELENGTH];
    RtAnimAnimation *animation;
    RwStream *result;
    void *dict;
    RtAnimInterpolator *interp;
    RwInt32 slots;
    RwUInt32 i;

    if (!fn_8047CF50(stream, rwID_STRUCT, NULL, NULL) ||
        fn_8047D868(stream, &slots, sizeof(RwInt32)) == NULL) {
        return NULL;
    }

    for (i = 0; i < rpUVANIMMAXSLOTS; i++) {
        if (!(slots & (1 << i))) {
            continue;
        }

        if (fn_80480D4C(stream, name, rpUVANIMNAMELENGTH) == 0) {
            result = NULL;
        } else {
            dict = fn_804790C4(&lbl_8056D748);
            if (dict != NULL) {
                animation = fn_804790D4(dict, name);
                if (animation != NULL) {
                    fn_80454618(animation);
                } else {
                    animation = UVAnimCreateIdentity(name);
                }
            } else {
                animation = UVAnimCreateIdentity(name);
            }
            result = animation == NULL ? NULL : stream;
        }
        if (result == NULL) {
            return NULL;
        }

        if (animation != NULL) {
            interp = fn_804779DC(fn_804779A8(animation), animation->interpInfo->interpKeyFrameSize);
            extension->interp[i] = interp;
            if (interp == NULL || !fn_80477AB8(extension->interp[i], animation)) {
                fn_8045459C(animation);
                return NULL;
            }
        }
    }

    if (fn_80453D08(extension) != NULL) {
        return stream;
    }
    return NULL;
}

/* material stream size: nothing without animations */
RwInt32 fn_80454294(const void *object, RwInt32 offset, RwInt32 size) {
    RpUVAnimMaterialExtension *extension;
    RwInt32 chunkSize;
    RwUInt32 i;

    if (fn_804549DC(object)) {
        extension = RPUVANIMMATERIALGETEXT(object);
        chunkSize = rwCHUNKHEADERSIZE + sizeof(RwInt32);
        for (i = 0; i < rpUVANIMMAXSLOTS; i++) {
            if (extension->interp[i] != NULL) {
                chunkSize += rpUVANIMNAMELENGTH;
            }
        }
        return chunkSize;
    }
    return 0;
}

/* Reads an animation's custom data: its name and node to UV channel map */
RpUVAnimCustomData *fn_80454374(RwStream *stream) {
    RpUVAnimCustomData *customData =
        RwFreeListAlloc(lbl_805F1498, rwMEMHINTDUR_EVENT | rpID_UVANIMPLUGIN);

    if (customData == NULL) {
        return NULL;
    }
    if (fn_80480D4C(stream, customData->name, rpUVANIMNAMELENGTH) == 0) {
        return NULL;
    }
    if (fn_8047D868(stream, (RwInt32 *)customData->nodeToUVChannelMap,
                    sizeof(customData->nodeToUVChannelMap)) == NULL) {
        return NULL;
    }
    customData->refCount = 1;
    return customData;
}

/* Writes an animation's custom data */
RpUVAnimCustomData *fn_8045441C(RpUVAnimCustomData *customData, RwStream *stream) {
    if (fn_80480ED0(stream, customData->name, rpUVANIMNAMELENGTH) == NULL) {
        return NULL;
    }
    if (fn_8047D458(stream, (RwInt32 *)customData->nodeToUVChannelMap,
                    sizeof(customData->nodeToUVChannelMap)) != NULL) {
        return customData;
    }
    return NULL;
}

/* The stream size of an animation's custom data */
RwInt32 fn_80454494(RpUVAnimCustomData *customData) {
    return sizeof(customData->name) + sizeof(customData->nodeToUVChannelMap);
}

/* Creates an animation of the given keyframe type with its custom data */
RtAnimAnimation *fn_8045449C(const RwChar *name, RwUInt32 numNodes, RwUInt32 numFrames,
                             RwReal duration, RwUInt32 *nodeToUVChannelMap, RwInt32 keyFrameType) {
    RtAnimAnimation *animation = NULL;
    RpUVAnimCustomData *customData;

    switch (keyFrameType) {
    case rpUVANIMLINEAR:
        animation = fn_80477468(lbl_8056D77C.typeID, numFrames, 0, duration);
        break;
    case rpUVANIMPARAM:
        animation = fn_80477468(lbl_8056D7AC.typeID, numFrames, 0, duration);
        break;
    }
    if (animation != NULL) {
        customData = RwFreeListAlloc(lbl_805F1498, rwMEMHINTDUR_EVENT | rpID_UVANIMPLUGIN);
        if (customData == NULL) {
            return NULL;
        }
        strncpy(customData->name, name, rpUVANIMNAMELENGTH);
        customData->name[rpUVANIMNAMELENGTH - 1] = '\0';
        memcpy(customData->nodeToUVChannelMap, nodeToUVChannelMap, numNodes * sizeof(RwUInt32));
        customData->refCount = 1;
        animation->customData = customData;
        return animation;
    }
    return NULL;
}

/* Releases an animation; the last reference frees its custom data and destroys it */
RwBool fn_8045459C(RtAnimAnimation *animation) {
    RpUVAnimCustomData *customData = animation->customData;
    RwBool lastReference;

    customData->refCount--;
    if (customData->refCount != 0) {
        lastReference = FALSE;
    } else {
        RwFreeListFree(lbl_805F1498, animation->customData);
        lastReference = TRUE;
    }
    if (lastReference) {
        fn_8047759C(animation);
    }
    return TRUE;
}

void fn_80454618(RtAnimAnimation *animation) {
    RpUVAnimCustomData *customData = animation->customData;

    customData->refCount++;
}

/* Sets the material's UV channel matrices to the product of the transforms the playing animations'
 * nodes drive them with, and hands them to the material's UV transform effect */
RpMaterial *fn_8045462C(RpMaterial *material) {
    RpUVAnimMaterialExtension *extension = RPUVANIMMATERIALGETEXT(material);
    RwMatrix transform;
    RtAnimInterpolator *interp;
    RpUVAnimCustomData *customData;
    RwInt32 frameSize;
    RtAnimKeyFrameApplyCallBack apply;
    RwUInt32 *channel;
    RwUInt8 *frame;
    RwMatrix *uv;
    RwInt32 i;
    RwInt32 node;

    for (i = 0; i < rpUVANIMNUMCHANNELS; i++) {
        uv = extension->uv[i];
        if (uv != NULL) {
            RwMatrixSetIdentity(uv);
        }
    }

    for (i = 0; i < rpUVANIMMAXSLOTS; i++) {
        interp = extension->interp[i];
        if (interp == NULL) {
            continue;
        }
        frame = RTANIMINTERPOLATORFRAMES(interp);
        frameSize = interp->currentInterpKeyFrameSize;
        customData = interp->pCurrentAnim->customData;
        apply = interp->keyFrameApplyCB;
        channel = customData->nodeToUVChannelMap;
        for (node = 0; node < interp->numNodes; channel++, node++) {
            /* as in the original, a node without a channel matrix keeps the frame pointer, so the
             * nodes after it read the wrong frames */
            if (*channel >= rpUVANIMNUMCHANNELS) {
                continue;
            }
            uv = extension->uv[*channel];
            if (uv == NULL) {
                continue;
            }
            if (fn_804532E4 == apply) {
                UVAnimLinearKeyFrameApply(&transform, (RpUVAnimInterpFrame *)frame);
            } else if (fn_804535C8 == apply) {
                UVANIMPARAMKEYFRAMEAPPLY(&transform, (RpUVAnimInterpFrame *)frame);
            } else {
                apply(&transform, frame);
            }
            fn_8047F72C(uv, &transform, rwCOMBINEPRECONCAT);
            frame += frameSize;
        }
    }

    fn_804A2BE0(material, extension->uv[0], extension->uv[1]);
    return material;
}

RwChar *fn_804548E4(RtAnimAnimation *animation) {
    RpUVAnimCustomData *customData = animation->customData;

    return customData->name;
}

/* Sets the current time of the animations playing on a material */
RpMaterial *fn_804548EC(RpMaterial *material, RwReal time) {
    RpUVAnimMaterialExtension *extension = RPUVANIMMATERIALGETEXT(material);
    RwUInt32 i;

    for (i = 0; i < rpUVANIMMAXSLOTS; i++) {
        if (extension->interp[i] != NULL) {
            fn_804782E8(extension->interp[i], time);
        }
    }
    return material;
}

/* Advances the animations playing on a material */
RpMaterial *fn_80454964(RpMaterial *material, RwReal deltaTime) {
    RpUVAnimMaterialExtension *extension = RPUVANIMMATERIALGETEXT(material);
    RwUInt32 i;

    for (i = 0; i < rpUVANIMMAXSLOTS; i++) {
        if (extension->interp[i] != NULL) {
            fn_80477ED4(extension->interp[i], deltaTime);
        }
    }
    return material;
}

RwBool fn_804549DC(const RpMaterial *material) {
    RpUVAnimMaterialExtension *extension = RPUVANIMMATERIALGETEXT(material);
    RwUInt32 i;

    for (i = 0; i < rpUVANIMMAXSLOTS; i++) {
        if (extension->interp[i] != NULL) {
            return TRUE;
        }
    }
    return FALSE;
}
