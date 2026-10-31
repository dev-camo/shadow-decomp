/* RenderWare lights: the light module's free list, creating, destroying and reading lights, and
 * their radius, colour and cone angle. The original file name is unknown. */
#include "rw/rpworld.h"
#include "rw/rwmath.h"
#include <math.h>
#include <string.h>
#include "layout_check.h"

/* Reports an error: the arguments are the error code and its parameters */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwID_WORLDPLUGIN;                                                        \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

/* the first stream version that stores minusCosAngle itself (inferred) */
#define rwSTREAMLIGHTCONEVERSION 0x30300

/* The stream header of a light (0x18 bytes) */
typedef struct LightChunk {
    /* 0x00 */ RwReal radius;
    /* 0x04 */ RwReal red;
    /* 0x08 */ RwReal green;
    /* 0x0C */ RwReal blue;
    /* 0x10 */ RwReal minusCosAngle;
    /* 0x14 */ RwUInt32 typeAndFlags; /* the type in bits 16-23, the flags in the low byte */
} LightChunk;
CHECK_SIZEOF(LightChunk, 0x18);

/* The light module's globals in the engine (inferred) */
typedef struct LightGlobals {
    /* 0x0 */ RwFreeList *lightFreeList;
} LightGlobals;
CHECK_SIZEOF(LightGlobals, 0x4);

/* The light module (inferred) */
typedef struct LightModule {
    /* 0x0 */ RwInt32 globalsOffset; /* engine plugin offset of the globals */
    /* 0x4 */ RwInt32 numInstances;  /* the open engine instances */
} LightModule;
CHECK_SIZEOF(LightModule, 0x8);

#define LIGHTGLOBALS() ((LightGlobals *)((RwUInt8 *)lbl_805F15B4 + lbl_805F14D8.globalsOffset))
#define LIGHTGLOBAL(var) (LIGHTGLOBALS()->var)

RwPluginRegistry lbl_8056D828 = {sizeof(RpLight), sizeof(RpLight), 0, 0, NULL, NULL};

RwFreeList lbl_805E15C8; /* the light free list */

RwInt32 lbl_805EDDB0 = 32; /* light free list entries per block */
RwInt32 lbl_805EDDB4 = 1;  /* light free list blocks to preallocate */

LightModule lbl_805F14D8;

/* the free list callback that destroys the lights still in use when the module closes */
void fn_8045A760(void *light, void *data) {
    fn_8045ACD4(light);
}

/* the light's frame sync: nothing to do */
void *fn_8045A780(void *object) {
    return object;
}

/* Sets a light's radius */
RpLight *fn_8045A784(RpLight *light, RwReal radius) {
    RwFrame *frame = light->object.parent;

    light->radius = radius;
    if (frame != NULL) {
        fn_8048722C(frame);
    }
    return light;
}

/* Sets a light's colour, noting whether it is grey */
RpLight *fn_8045A7C4(RpLight *light, const RwRGBAReal *color) {
    light->color = *color;
    if (light->color.red == light->color.green && light->color.red == light->color.blue) {
        light->object.privateFlags = rpLIGHTPRIVATENOCHROMA;
    } else {
        light->object.privateFlags = 0;
    }
    return light;
}

/* The cone angle of a spot light, in radians */
RwReal fn_8045A818(const RpLight *light) {
    RwReal angle;

    RwACosMacro(angle, -light->minusCosAngle);
    return angle;
}

/* Sets the cone angle of a spot light: 0 to pi/2 radians */
RpLight *fn_8045AA44(RpLight *light, RwReal angle) {
    if (angle < 0.0f || angle > 1.5707964f) {
        return NULL;
    }
    light->minusCosAngle = -(RwReal)cos(angle);
    return light;
}

/* plugin registration with lights */
RwInt32 fn_8045AAA0(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor, RwPluginObjectCopy copy) {
    return fn_80481DC8(&lbl_8056D828, size, pluginID, constructor, destructor, copy);
}

RwInt32 fn_8045AAE0(RwUInt32 pluginID, RwPluginDataChunkReadCallBack read,
                    RwPluginDataChunkWriteCallBack write,
                    RwPluginDataChunkGetSizeCallBack getSize) {
    return fn_804815F0(&lbl_8056D828, pluginID, read, write, getSize);
}

/* Reads a light: its header, then its plugin data */
RpLight *fn_8045AB1C(RwStream *stream) {
    RwUInt32 length;
    RwUInt32 version;
    LightChunk header;
    RpLight *light;

    if (!fn_8047CF50(stream, rwID_STRUCT, &length, &version)) {
        return NULL;
    }
    if (version >= rwLIBRARYBASEVERSION && version <= RW_STREAM_VERSION) {
        memset(&header, 0, sizeof(header));
        if (length != fn_80480D4C(stream, &header, length)) {
            return NULL;
        }
        fn_8047D254(&header, sizeof(header));

        light = fn_8045AD34((header.typeAndFlags >> 16) & 0xFF);
        if (light == NULL) {
            return NULL;
        }
        light->radius = header.radius;
        light->color.red = header.red;
        light->color.green = header.green;
        light->color.blue = header.blue;
        if (version < rwSTREAMLIGHTCONEVERSION) {
            /* older streams store the tangent of the cone angle */
            light->minusCosAngle =
                -fn_80482340(1.0f / (1.0f + header.minusCosAngle * header.minusCosAngle));
        } else {
            light->minusCosAngle = header.minusCosAngle;
        }
        if (light->color.red == light->color.green && light->color.red == light->color.blue) {
            light->object.privateFlags = rpLIGHTPRIVATENOCHROMA;
        } else {
            light->object.privateFlags = 0;
        }
        light->object.flags = header.typeAndFlags;

        if (fn_804816AC(&lbl_8056D828, stream, light) == NULL) {
            return NULL;
        }
        return light;
    }
    RWERROR((E_RW_BADVERSION));
    return NULL;
}

RwBool fn_8045ACD4(RpLight *light) {
    fn_80482144(&lbl_8056D828, light);
    fn_8048D834(light);
    RwFreeListFree(LIGHTGLOBAL(lightFreeList), light);
    return TRUE;
}

RpLight *fn_8045AD34(RwInt32 type) {
    RpLight *light = RwFreeListAlloc(LIGHTGLOBAL(lightFreeList), rwMEMHINTDUR_EVENT | rwID_LIGHT);

    if (light == NULL) {
        return NULL;
    }
    light->object.type = rpLIGHT;
    light->object.subType = type;
    light->object.flags = 0;
    light->object.privateFlags = 0;
    light->object.parent = NULL;
    light->sync = fn_8045A780;
    light->radius = 0.0f;
    light->minusCosAngle = 0.0f;
    light->color.red = 1.0f;
    light->color.green = 1.0f;
    light->color.blue = 1.0f;
    light->color.alpha = 1.0f;
    light->object.privateFlags = rpLIGHTPRIVATENOCHROMA;
    rwLinkListInitialize(&light->WorldSectorsInLight);
    light->inWorld.prev = NULL;
    light->inWorld.next = NULL;
    light->lightFrame = lbl_805F15B4->lightFrame - 1;
    light->object.flags = rpLIGHTLIGHTATOMICS | rpLIGHTLIGHTWORLD;
    fn_8048209C(&lbl_8056D828, light);
    return light;
}

/* engine plugin destructor: destroys the lights still in use, then the free list */
void *fn_8045AE20(void *instance, RwInt32 offset, RwInt32 size) {
    fn_804800DC(LIGHTGLOBAL(lightFreeList), fn_8045A760, NULL);
    fn_8047FCDC(LIGHTGLOBAL(lightFreeList));
    LIGHTGLOBAL(lightFreeList) = NULL;
    lbl_805F14D8.numInstances--;
    return instance;
}

/* engine plugin constructor: the light free list */
void *fn_8045AE98(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F14D8.globalsOffset = offset;
    LIGHTGLOBAL(lightFreeList) =
        fn_8047FC04(lbl_8056D828.sizeOfStruct, lbl_805EDDB0, 4, lbl_805EDDB4, &lbl_805E15C8,
                    rwMEMHINTDUR_GLOBAL | rwID_LIGHT);
    if (LIGHTGLOBAL(lightFreeList) != NULL) {
        lbl_805F14D8.numInstances++;
        return instance;
    }
    return NULL;
}
