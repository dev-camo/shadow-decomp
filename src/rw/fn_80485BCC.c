/* RenderWare engine: initializing, opening, starting, stopping, closing and terminating it, its
 * device requests, video modes and plugins. The original file name is unknown. */
#include "rw/rwcore.h"
#include <string.h>
#include "layout_check.h"

/* RwEngineInitFlag: free list entries are allocated and freed with the memory functions */
#define rwENGINEINITNOFREELISTS 0x1

/* RenderWare's core module IDs, which the engine registers its modules' plugins under */
#define rwID_VECTORMODULE 0x401
#define rwID_MATRIXMODULE 0x402
#define rwID_FRAMEMODULE 0x403
#define rwID_STREAMMODULE 0x404
#define rwID_CAMERAMODULE 0x405
#define rwID_IMAGEMODULE 0x406
#define rwID_RASTERMODULE 0x407
#define rwID_TEXTUREMODULE 0x408
#define rwID_IMMEDIATEMODULE 0x40A
#define rwID_RESOURCESMODULE 0x40B
#define rwID_COLORMODULE 0x40D
#define rwID_ERRORMODULE 0x40F
#define rwID_STRINGMODULE 0x412

/* device system requests (RwCoreDeviceSystemFn) */
#define rwDEVICESYSTEMOPEN 0x00
#define rwDEVICESYSTEMCLOSE 0x01
#define rwDEVICESYSTEMSTART 0x02
#define rwDEVICESYSTEMSTOP 0x03
#define rwDEVICESYSTEMREGISTER 0x04
#define rwDEVICESYSTEMGETMODEINFO 0x06
#define rwDEVICESYSTEMGETMODE 0x0A
#define rwDEVICESYSTEMSTANDARDS 0x0B
#define rwDEVICESYSTEMGETNUMSUBSYSTEMS 0x0D
#define rwDEVICESYSTEMGETSUBSYSTEMINFO 0x0E
#define rwDEVICESYSTEMGETCURRENTSUBSYSTEM 0x0F
#define rwDEVICESYSTEMSETSUBSYSTEM 0x10
#define rwDEVICESYSTEMFINALIZESTART 0x11
#define rwDEVICESYSTEMINITIATESTOP 0x12

/* the number of standard functions (rwSTANDARDNUMOFSTANDARD) */
#define rwSTANDARDNUMOFSTANDARD 0x1D

/* errors (inferred) */
#define E_RW_BADENGINESTATE 0x80000001 /* the engine is not in the status the call needs */
#define E_RW_DEVICEERROR 0x18          /* reported with the device request that failed */

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

/* A device sub system's description; only its name is written here, and the name's size is
 * RenderWare's */
typedef struct RwSubSystemInfo {
    RwChar name[80];
} RwSubSystemInfo;
CHECK_SIZEOF(RwSubSystemInfo, 0x50);

/* The engine plugin registry: the engine globals with their plugins' data */
RwPluginRegistry lbl_8056DFB0 = {sizeof(RwGlobals), sizeof(RwGlobals), 0, 0, NULL, NULL};

/* The engine globals while no engine is open */
RwGlobals lbl_805E2B48;

RwGlobals *lbl_805F15B4; /* the engine instance */
RwUInt32 lbl_805F15B0;   /* the number of engine instances */

/* Allocates a free list entry with the memory functions, when free lists are disabled */
void *fn_80485BCC(RwFreeList *freeList, RwUInt32 hint) {
    return RwMalloc(freeList->entrySize, hint);
}

/* Frees a free list entry with the memory functions, when free lists are disabled */
RwFreeList *fn_80485BFC(RwFreeList *freeList, void *mem) {
    RwFree(mem);
    return freeList;
}

/* Makes a device system request. When the device fails it, the engine answers the sub system
 * requests for its one rendering sub system and accepts the start and stop notifications. */
RwBool fn_80485C3C(RwDevice *device, RwInt32 requestID, void *out, void *inOut, RwInt32 numIn) {
    RwBool result;

    result = device->fpSystem(requestID, out, inOut, numIn);
    if (!result) {
        switch (requestID) {
        case rwDEVICESYSTEMFINALIZESTART:
        case rwDEVICESYSTEMINITIATESTOP:
            result = TRUE;
            break;
        case rwDEVICESYSTEMGETNUMSUBSYSTEMS:
            *(RwInt32 *)out = 1;
            result = TRUE;
            break;
        case rwDEVICESYSTEMGETSUBSYSTEMINFO:
            result = (numIn == 0);
            if (result) {
                rwstrcpy(((RwSubSystemInfo *)out)->name, "Only rendering sub system");
            }
            break;
        case rwDEVICESYSTEMGETCURRENTSUBSYSTEM:
            *(RwInt32 *)out = 0;
            result = TRUE;
            break;
        case rwDEVICESYSTEMSETSUBSYSTEM:
            result = (numIn == 0);
            break;
        }
    }
    if (!result) {
        RWERROR((E_RW_DEVICEERROR, requestID));
    }
    return result;
}

RwUInt32 fn_80485D64(void) {
    return lbl_805F15B0;
}

/* The library version: 3.7.0.2 */
RwInt32 fn_80485D6C(void) {
    return 0x37002;
}

RwInt32 fn_80485D78(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor) {
    return fn_80481DC8(&lbl_8056DFB0, size, pluginID, constructor, destructor, NULL);
}

RwInt32 fn_80485DB8(RwUInt32 pluginID) {
    return fn_80481D98(&lbl_8056DFB0, pluginID);
}

RwVideoMode *fn_80485DE4(RwVideoMode *modeinfo, RwInt32 modeIndex) {
    if (!fn_80485C3C(&lbl_805F15B4->dOpenDevice, rwDEVICESYSTEMGETMODEINFO, modeinfo, NULL,
                     modeIndex)) {
        modeinfo = NULL;
    }
    return modeinfo;
}

/* The current video mode, or -1 */
RwInt32 fn_80485E70(void) {
    RwInt32 mode;

    if (fn_80485C3C(&lbl_805F15B4->dOpenDevice, rwDEVICESYSTEMGETMODE, &mode, NULL, 0)) {
        return mode;
    }
    return -1;
}

/* Stops a started engine: the device is told, the engine plugins are deinitialized and the
 * device stops */
RwBool fn_80485EF4(void) {
    RwDevice *device;
    RwBool result;

    device = &lbl_805F15B4->dOpenDevice;
    fn_80485C3C(device, rwDEVICESYSTEMINITIATESTOP, NULL, NULL, 0);
    fn_80482144(&lbl_8056DFB0, lbl_805F15B4);
    result = fn_80485C3C(device, rwDEVICESYSTEMSTOP, NULL, NULL, 0);
    if (result) {
        lbl_805F15B4->engineStatus = rwENGINESTATUSOPENED;
    }
    return result;
}

/* Starts an opened engine: the device starts, the engine plugins are initialized, the gamma is
 * set and the device is told. If the plugins fail, the device stops again. */
RwBool fn_80485FE0(void) {
    RwDevice *device;

    device = &lbl_805F15B4->dOpenDevice;
    if (fn_80485C3C(device, rwDEVICESYSTEMSTART, NULL, NULL, 0)) {
        if (fn_8048209C(&lbl_8056DFB0, lbl_805F15B4) != NULL) {
            fn_80489830(lbl_805F15B4->dOpenDevice.gammaCorrection);
            fn_80485C3C(device, rwDEVICESYSTEMFINALIZESTART, NULL, NULL, 0);
            lbl_805F15B4->engineStatus = rwENGINESTATUSSTARTED;
            return TRUE;
        }
        fn_80485C3C(device, rwDEVICESYSTEMSTOP, NULL, NULL, 0);
    }
    return FALSE;
}

/* Closes an opened engine: the device closes, and the engine globals go back to the static copy */
RwBool fn_80486138(void) {
    RwBool result;

    result = fn_80485C3C(&lbl_805F15B4->dOpenDevice, rwDEVICESYSTEMCLOSE, NULL, NULL, 0);
    if (result) {
        RwGlobals *engine = lbl_805F15B4;

        lbl_805F15B4 = &lbl_805E2B48;
        memcpy(lbl_805F15B4, engine, sizeof(RwGlobals));
        RwFree(engine);
        lbl_805F15B0--;
        lbl_805F15B4->engineStatus = rwENGINESTATUSINITED;
    }
    return result;
}

/* Opens the engine on a device (inferred helper, inlined into fn_80486204): allocates the engine
 * globals with their plugins' data and registers, opens and asks the device for its standard
 * functions. On failure the static engine globals are the instance again. */
static inline RwBool EngineOpenOnDevice(RwDevice *device, RwEngineOpenParams *initParams) {
    RwGlobals *engine;

    lbl_805F15B4 = (RwGlobals *)RwMalloc(lbl_8056DFB0.sizeOfStruct, rwMEMHINTDUR_GLOBAL);
    engine = lbl_805F15B4;
    if (engine != NULL) {
        memcpy(engine, &lbl_805E2B48, sizeof(RwGlobals));
        fn_80485C3C(device, rwDEVICESYSTEMREGISTER, &lbl_805F15B4->dOpenDevice,
                    &lbl_805F15B4->memoryFuncs, 0);
        if (fn_80485C3C(device, rwDEVICESYSTEMOPEN, NULL, initParams, 0)) {
            fn_80485C3C(device, rwDEVICESYSTEMSTANDARDS, lbl_805F15B4->stdFunc, NULL,
                        rwSTANDARDNUMOFSTANDARD);
            lbl_805F15B0++;
            return TRUE;
        }
        lbl_805F15B4 = &lbl_805E2B48;
        memcpy(lbl_805F15B4, engine, sizeof(RwGlobals));
        RwFree(engine);
        return FALSE;
    }
    lbl_805F15B4 = &lbl_805E2B48;
    RWERROR((E_RW_NOMEM, lbl_8056DFB0.sizeOfStruct));
    return FALSE;
}

/* Opens an initialized engine on the device: the engine globals are allocated with their plugins'
 * data, and the device is registered, opened and asked for its standard functions */
RwBool fn_80486204(RwEngineOpenParams *initParams) {
    RwBool result;

    if (lbl_805F15B4 == NULL) {
        lbl_805F15B4 = &lbl_805E2B48;
    }
    result = (lbl_805F15B4->engineStatus == rwENGINESTATUSINITED);
    if (result) {
        result = (initParams != NULL);
        if (result) {
            RwDevice *device = fn_804950A0();

            result = (device != NULL);
            if (result) {
                result = EngineOpenOnDevice(device, initParams);
                if (result) {
                    lbl_805F15B4->engineStatus = rwENGINESTATUSOPENED;
                }
            }
        } else {
            RWERROR((E_RW_NULLP));
        }
    } else {
        RWERROR((E_RW_BADENGINESTATE));
    }
    return result;
}

/* Terminates an initialized engine once no engine is open */
RwBool fn_804864AC(void) {
    RwBool result;

    result = (lbl_805F15B0 == 0);
    if (result) {
        fn_80481C58();
        fn_8048038C();
        lbl_805F15B4->engineStatus = rwENGINESTATUSIDLE;
    }
    return result;
}

/* Initializes the engine: its free list functions, memory and string functions, plugin
 * registration and the core modules' plugins, then the device's plugins */
RwBool fn_804864F8(const RwMemoryFunctions *memFuncs, RwUInt32 initFlags, RwUInt32 resArenaSize) {
    RwBool result = FALSE;

    lbl_805F15B4 = &lbl_805E2B48;
    if (initFlags & rwENGINEINITNOFREELISTS) {
        lbl_805F15B4->rwfreelistalloc = fn_80485BCC;
        lbl_805F15B4->rwfreelistfree = fn_80485BFC;
        fn_8047FA28(FALSE);
    } else {
        lbl_805F15B4->rwfreelistalloc = fn_8047FDB0;
        lbl_805F15B4->rwfreelistfree = fn_8047FF50;
        fn_8047FA28(TRUE);
    }
    lbl_805F15B4->resArenaInitSize = resArenaSize;

    if (lbl_805F15B4->engineStatus == rwENGINESTATUSIDLE) {
        result = fn_80482E20();
        if (result) {
            result = fn_80480274(memFuncs);
            if (result) {
                result = fn_80481B98();
                if (result) {
                    /* Each offset is negative if its registration failed */
                    RwInt32 offsets;

                    offsets = fn_80481DC8(&lbl_8056DFB0, 8, rwID_ERRORMODULE, fn_8047DB38,
                                          fn_8047DB70, NULL);
                    offsets |= fn_80481DC8(&lbl_8056DFB0, 8, rwID_VECTORMODULE, fn_804825EC,
                                           fn_80482548, NULL);
                    offsets |= fn_80481DC8(&lbl_8056DFB0, 0, rwID_COLORMODULE, fn_8047DB10,
                                           fn_8047DB24, NULL);
                    offsets |= fn_80481DC8(&lbl_8056DFB0, 0x18, rwID_MATRIXMODULE, fn_8047E88C,
                                           fn_8047E828, NULL);
                    offsets |= fn_80481DC8(&lbl_8056DFB0, 4, rwID_FRAMEMODULE, fn_80486888,
                                           fn_8048692C, NULL);
                    offsets |= fn_80481DC8(&lbl_8056DFB0, 4, rwID_STREAMMODULE, fn_80480C6C,
                                           fn_80480CF8, NULL);
                    offsets |= fn_80481DC8(&lbl_8056DFB0, 4, rwID_CAMERAMODULE, fn_80485424,
                                           fn_804853C0, NULL);
                    offsets |= fn_80481DC8(&lbl_8056DFB0, 0x220, rwID_IMAGEMODULE, fn_80487914,
                                           fn_80487BB0, NULL);
                    offsets |= fn_80481DC8(&lbl_8056DFB0, 0x64, rwID_RASTERMODULE, fn_8048A02C,
                                           fn_80489FC0, NULL);
                    offsets |= fn_80481DC8(&lbl_8056DFB0, 0x34, rwID_TEXTUREMODULE, fn_8048D4DC,
                                           fn_8048D300, NULL);
                    offsets |= fn_80481DC8(&lbl_8056DFB0, 0x60, rwID_PIPEMODULE, fn_8049B0CC,
                                           fn_8049B110, NULL);
                    offsets |= fn_80481DC8(&lbl_8056DFB0, 4, rwID_STRINGMODULE, fn_80482BC4,
                                           fn_80482C50, NULL);
                    offsets |= fn_8049B140();
                    offsets |= fn_80481DC8(&lbl_8056DFB0, 0x74, rwID_IMMEDIATEMODULE, fn_8049AFF8,
                                           fn_8049AF94, NULL);
                    offsets |= fn_80481DC8(&lbl_8056DFB0, 0x28, rwID_RESOURCESMODULE, fn_8048053C,
                                           fn_80480690, NULL);

                    result = !(offsets < 0);
                    if (result) {
                        result = fn_80493E64();
                        if (result) {
                            lbl_805F15B4->engineStatus = rwENGINESTATUSINITED;
                            return result;
                        }
                    }
                    fn_80481C58();
                }
                fn_8048038C();
            }
            fn_80482F60();
        }
    }
    return result;
}
