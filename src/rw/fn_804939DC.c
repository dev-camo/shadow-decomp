/* RenderWare GameCube device: the device's system function (opening, starting and stopping the
 * device, its video modes and standard functions), the camera begin/end update, clear and raster
 * show functions, and the triple buffered display: frames are copied to alternating external frame
 * buffers, queued with a GX FIFO breakpoint and a draw sync token, and shown on the retrace after
 * the GPU reaches them. The original file name is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>
#include <dolphin/mtx.h>
#include <dolphin/os.h>
#include <dolphin/vi.h>
#include "layout_check.h"

/* video modes */
#define rwVIDEOMODEEXCLUSIVE 0x1

/* The TV format of a custom render mode */
#define TVFORMATCUSTOM 42

/* The device ID of the GameCube (inferred) */
#define rwDEVICE_GAMECUBE 3

/* The largest texture side */
#define MAXTEXTURESIZE 1024

/* The memory hint of the frame buffers and the FIFO (inferred) */
#define rwMEMHINT_FRAMEBUFFERS (rwMEMHINTDUR_GLOBAL | rwID_DRIVERMODULE)

/* A device standard function and the standard it implements */
typedef struct RwStandard {
    RwInt32 nStandard;
    RwStandardFunc fpStandard;
} RwStandard;
CHECK_SIZEOF(RwStandard, 0x8);

/* The display the engine is opened with: the render mode (NULL for the TV format's default), the
 * pixel format and the FIFO size (inferred) */
typedef struct GameCubeDisplay {
    GXRenderModeObj *renderMode;
    RwInt32 pixelFormat;
    RwInt32 fifoSize;
} GameCubeDisplay;
CHECK_SIZEOF(GameCubeDisplay, 0xC);

/* The device's state (inferred) */
typedef struct GameCubeDeviceState {
    RwCamera *camera;
    RwMemoryFunctions *memoryFuncs;
} GameCubeDeviceState;
CHECK_SIZEOF(GameCubeDeviceState, 0x8);

/* A queued frame: the external frame buffer it is copied to and the FIFO position after it */
typedef struct FrameQueueEntry {
    void *frameBuffer;
    void *breakPoint;
} FrameQueueEntry;
CHECK_SIZEOF(FrameQueueEntry, 0x8);

/* The .sbss variables are defined in reverse address order: the compiler lays them out last
 * first */
/* The camera being updated, and the memory functions the device was registered with */
GameCubeDeviceState lbl_805F1650;

/* The render mode */
GXRenderModeObj *lbl_805F164C;

/* The frame buffer line the halves of a double height frame are split at */
RwInt32 lbl_805F1648;

/* The FIFO memory and object */
GXFifoObj *lbl_805F1644;
void *lbl_805F1640;

/* The frames' draw sync tokens */
RwUInt16 lbl_805F1638[4];

/* The threads waiting for the frame queue */
OSThreadQueue lbl_805F1630;

/* Whether GX has been initialized */
RwInt32 lbl_805F162C;

/* Counts the halves of a frame drawn in two halves */
RwInt32 lbl_805F1628;

/* Whether a frame was shown on this retrace */
RwInt32 lbl_805F1624;

/* Whether the GPU reached the shown frame's breakpoint before it was shown */
RwInt32 lbl_805F1620;

/* Whether the GPU has drawn a frame that can be shown. Volatile: the device stop request polls it
 * in a loop that rereads it */
volatile RwInt32 lbl_805F161C;

/* Whether the GX FIFO breakpoint is enabled */
RwInt32 lbl_805F1618;

/* The frame queue: the frame the GPU is drawing, the next free entry, and the token entries the
 * frames' draw sync tokens are written to and read from. Volatile: they are shared with the
 * interrupt callbacks, and every reader rereads them with no store in between */
volatile RwInt32 lbl_805F1614;
volatile RwInt32 lbl_805F1610;
volatile RwInt32 lbl_805F160C;
volatile RwInt32 lbl_805F1608;

/* The external frame buffer being shown. Volatile: the pre-retrace callback fn_80493ABC and the
 * breakpoint callback fn_804939DC read it. */
void *volatile lbl_805F1604;

/* The external frame buffer the next frame is copied to.
 *
 * Matching workaround: volatile, although only task level code uses it. The original loads it at
 * each read in source order; without the qualifier fn_80494C6C keeps its eight loads and two stores
 * of it but schedules the loads later, and 56 of its 269 words differ (registers and scheduling).
 * Revisit when a source form that keeps those loads in place without the qualifier is found. */
void *volatile lbl_805F1600;

/* The two external frame buffers, which the shown one and the copy target alternate between */
void *lbl_805F15FC;
void *lbl_805F15F8;

/* The memory of the FIFO and the external frame buffers */
void *lbl_805F15F4;

/* Non-zero while frames are drawn in two halves at double height */
RwInt32 lbl_805F15F0;

/* The TV format, or TVFORMATCUSTOM for a render mode given by the application */
RwInt32 lbl_805F15EC;

/* The pixel format the frame buffer is set up with, and the one it has */
RwInt32 lbl_805F15E8;
RwInt32 lbl_805F15E4;

/* Whether the frame buffer is cleared as it is copied to an external frame buffer */
RwInt32 lbl_805F15E0;

/* Which half of a double height frame is drawn next: the first while set */
RwInt32 lbl_805EDF28 = 1;

/* The size of the GX FIFO */
RwInt32 lbl_805EDF2C = 0x40000;

/* Whether the display is still black from starting the device */
RwInt32 lbl_805EDF30 = 1;

/* The number of entries of the frame queue */
RwInt32 lbl_805EDF34 = 3;

/* Retraces since a frame was last shown, and the retraces a frame is shown for at least */
RwUInt8 lbl_805EDF38 = 1;
RwUInt8 lbl_805EDF39 = 1;

/* The frame queue */
FrameQueueEntry lbl_805E3178[3];

/* The render mode given by the application */
GXRenderModeObj lbl_805E3190;

/* The view matrix of the camera being updated */
RwMatrix lbl_805E31CC;

/* The video modes of NTSC, PAL and MPAL, and of the render mode in use */
RwVideoMode lbl_8056E348[4] = {
    {640, 480, 24, rwVIDEOMODEEXCLUSIVE, 0, 0},
    {640, 528, 24, rwVIDEOMODEEXCLUSIVE, 0, 0},
    {640, 480, 24, rwVIDEOMODEEXCLUSIVE, 0, 0},
    {0, 0, 0, rwVIDEOMODEEXCLUSIVE, 0, 0},
};

/* Shows the next queued frame's position to the GPU: moves the FIFO breakpoint to the frame after
 * it (or disables it when the queue is empty), wakes threads waiting for the queue, and marks a
 * frame as drawn (after both halves of a double height frame) (inferred helper) */
static inline void FrameQueueAdvance(void) {
    RwInt32 next = (lbl_805F1608 + 1) % lbl_805EDF34;

    if (next == lbl_805F160C) {
        GXDisableBreakPt();
        lbl_805F1618 = 0;
    } else {
        GXEnableBreakPt(lbl_805E3178[next].breakPoint);
    }
    lbl_805F1608 = next;
    OSWakeupThread(&lbl_805F1630);

    if (lbl_805F15F0 == 0) {
        lbl_805F161C = 1;
    } else if (lbl_805F1628 == 1) {
        lbl_805F161C = 1;
        lbl_805F1628 = 0;
    } else {
        lbl_805F1628 = 1;
    }
}

/* The GX breakpoint callback: the GPU reached the end of a queued frame */
void fn_804939DC(void) {
    if (lbl_805E3178[lbl_805F1608].frameBuffer != lbl_805F1604) {
        FrameQueueAdvance();
    } else {
        /* The frame's buffer is still shown; advance after the next retrace */
        lbl_805F1620 = 1;
    }
}

/* The VI pre-retrace callback: shows the next drawn frame once its draw sync token is reached and
 * the current one has been shown long enough */
void fn_80493ABC(u32 retraceCount) {
    RwInt16 token;

    lbl_805EDF38++;
    token = lbl_805F1638[lbl_805F1614];
    if (lbl_805F161C == 1 && fn_8049A78C(token) && lbl_805EDF38 >= lbl_805EDF39) {
        lbl_805EDF38 = 0;
        lbl_805F1604 = (lbl_805F1604 == lbl_805F15F8) ? lbl_805F15FC : lbl_805F15F8;
        VISetNextFrameBuffer(lbl_805F1604);
        if (lbl_805EDF30 != 0) {
            VISetBlack(FALSE);
            lbl_805EDF30 = 0;
        }
        VIFlush();
        lbl_805F1614 = (lbl_805F1614 + 1) % lbl_805EDF34;
        lbl_805F161C = 0;
        lbl_805F1624 = 1;
    }
}

/* The VI post-retrace callback: advances the frame queue that waited for a frame to be shown */
void fn_80493B98(u32 retraceCount) {
    if (lbl_805F1620 != 0 && lbl_805F1624 != 0) {
        FrameQueueAdvance();
        lbl_805F1620 = 0;
    }
    lbl_805F1624 = 0;
}

/* The standard function of the standards the device does not implement */
RwBool fn_80493C7C(void *out, void *inOut, RwInt32 numIn) {
    return FALSE;
}

/* Sets the render mode: the given one (and pixel format), or the TV format's default */
void fn_80493C84(GXRenderModeObj *renderMode, RwInt32 pixelFormat) {
    if (renderMode != NULL) {
        RwInt32 format = pixelFormat;
        GXRenderModeObj *mode = &lbl_805E3190;

        lbl_805F164C = mode;
        *mode = *renderMode;
        lbl_805F15EC = TVFORMATCUSTOM;
        /* Antialiasing needs the 16-bit pixel format */
        if (lbl_805F164C->aa) {
            format = GX_PF_RGB565_Z16;
        }
        lbl_805F15E4 = format;
        lbl_805F15E8 = format;
        lbl_8056E348[3].width = lbl_805F164C->fbWidth;
        lbl_8056E348[3].height = lbl_805F164C->efbHeight;
        switch (pixelFormat) {
        case GX_PF_RGB8_Z24:
        case GX_PF_RGBA6_Z24:
            lbl_8056E348[3].depth = 24;
            break;
        case GX_PF_RGB565_Z16:
            lbl_8056E348[3].depth = 16;
            break;
        default:
            RWERRORCORE((E_RW_DEBUGMESSAGE, "Invalid pixel format"));
            break;
        }
    } else {
        switch (VIGetTvFormat()) {
        case VI_NTSC:
            lbl_805F164C = &GXNtsc480IntDf;
            lbl_805F15EC = VI_NTSC;
            break;
        case VI_PAL:
        case VI_EURGB60:
            lbl_805F164C = &GXPal528IntDf;
            lbl_805F15EC = VI_PAL;
            break;
        case VI_MPAL:
            lbl_805F164C = &GXMpal480IntDf;
            lbl_805F15EC = VI_MPAL;
            break;
        default:
            RWERRORCORE((E_RW_DEBUGMESSAGE, "Invalid TV format"));
            break;
        }
    }
}

/* Registers the device's plugins */
RwBool fn_80493E64(void) {
    fn_804953F0();
    fn_80499BF8();
    return TRUE;
}

/* Sets the viewport to the whole frame buffer, jittered for field rendering (inferred helper) */
static inline void SetFrameBufferViewport(GXRenderModeObj *renderMode) {
    if (renderMode->field_rendering) {
        GXSetViewportJitter(0.0f, 0.0f, renderMode->fbWidth, renderMode->xfbHeight, 0.0f, 1.0f,
                            VIGetNextField() ^ 1);
    } else {
        GXSetViewport(0.0f, 0.0f, renderMode->fbWidth, renderMode->xfbHeight, 0.0f, 1.0f);
    }
}

/* Allocates the FIFO and the two external frame buffers (inferred helper) */
static inline RwBool DeviceOpen(RwEngineOpenParams *openParams) {
    GameCubeDisplay *display = (GameCubeDisplay *)openParams->displayID;
    RwInt32 frameBufferSize;
    RwInt32 frameBufferBytes;
    RwUInt8 *first;
    RwUInt8 *second;

    if (display != NULL) {
        fn_80493C84(display->renderMode, display->pixelFormat);
        lbl_805EDF2C = display->fifoSize;
    } else {
        fn_80493C84(NULL, lbl_805F15E4);
    }

    frameBufferSize = ((lbl_805F164C->fbWidth + 15) & 0xFFF0) * lbl_805F164C->xfbHeight;
    frameBufferBytes = frameBufferSize * 2;
    lbl_805F15F4 = RwMalloc(lbl_805EDF2C + frameBufferBytes * 2 + 31, rwMEMHINT_FRAMEBUFFERS);
    lbl_805F1640 = (void *)(((RwUInt32)lbl_805F15F4 + 31) & ~31);
    DCInvalidateRange(lbl_805F1640, lbl_805EDF2C);
    first = (RwUInt8 *)lbl_805F1640 + lbl_805EDF2C;
    second = first + frameBufferBytes;
    lbl_805F1604 = first;
    lbl_805F15F8 = first;
    lbl_805F15FC = second;
    lbl_805F1600 = second;
    return TRUE;
}

/* Initializes GX and VI for the render mode and installs the display callbacks (inferred helper) */
static inline RwBool DeviceStart(void) {
    GXRenderModeObj *renderMode;
    RwInt32 pixelFormat;

    if (lbl_805F162C == 0) {
        lbl_805F1644 = GXInit(lbl_805F1640, lbl_805EDF2C);
        lbl_805F162C = 1;
    } else {
        GXFifoObj fifo;

        GXInitFifoBase(&fifo, lbl_805F1640, lbl_805EDF2C);
        GXSetCPUFifo(&fifo);
        GXSetGPFifo(&fifo);
        GXInitFifoBase(lbl_805F1644, lbl_805F1640, lbl_805EDF2C);
        GXSetCPUFifo(lbl_805F1644);
        GXSetGPFifo(lbl_805F1644);
    }
    while (GXReadDrawSync() != lbl_805F1678) {
        GXSetDrawSync(lbl_805F1678);
    }

    renderMode = lbl_805F164C;
    pixelFormat = lbl_805F15E4;
    VIConfigure(renderMode);
    VISetBlack(TRUE);
    VIFlush();
    SetFrameBufferViewport(renderMode);
    GXSetScissor(0, 0, renderMode->fbWidth, renderMode->efbHeight);
    GXSetScissorBoxOffset(0, 0);
    GXSetDispCopySrc(0, 0, renderMode->fbWidth, renderMode->efbHeight);
    GXSetDispCopyDst(renderMode->fbWidth, renderMode->xfbHeight);
    GXSetCopyFilter(renderMode->aa, renderMode->sample_pattern, GX_TRUE, renderMode->vfilter);
    if (renderMode->aa && renderMode->xfbHeight == renderMode->viHeight) {
        /* Antialiased frames are drawn in two halves */
        GXSetDispCopyYScale(1.0f);
        lbl_805F1648 = renderMode->xfbHeight / 2;
        lbl_805F15F0 = 1;
        lbl_805EDF28 = 1;
    } else {
        GXSetDispCopyYScale((f32)renderMode->xfbHeight / (f32)renderMode->efbHeight);
        lbl_805F15F0 = 0;
    }
    GXSetFieldMode(renderMode->field_rendering, renderMode->xfbHeight < renderMode->viHeight);
    GXSetPixelFmt(pixelFormat, GX_ZC_LINEAR);
    lbl_805EDF30 = 1;
    VIWaitForRetrace();
    if (lbl_805F164C->viTVmode & 1) {
        VIWaitForRetrace();
    }
    GXSetDispCopyGamma(GX_GM_1_0);
    lbl_805EDF38 = lbl_805EDF39;
    VISetPreRetraceCallback(fn_80493ABC);
    VISetPostRetraceCallback(fn_80493B98);
    GXSetBreakPtCallback(fn_804939DC);
    OSInitThreadQueue(&lbl_805F1630);
    return TRUE;
}

/* Fills in the standard functions: the device's, and fn_80493C7C for the others (inferred
 * helper) */
static inline RwBool DeviceStandards(RwStandardFunc *standardFunctions, RwInt32 numStandards) {
    RwStandard rwStandards[] = {
        {rwSTANDARDCAMERABEGINUPDATE, fn_804946A4},
        {rwSTANDARDCAMERAENDUPDATE, fn_80494C5C},
        {rwSTANDARDCAMERACLEAR, fn_804945E8},
        {rwSTANDARDRASTERSHOWRASTER, fn_80494C6C},
        {rwSTANDARDRGBTOPIXEL, fn_80491BAC},
        {rwSTANDARDPIXELTORGB, fn_80491EC0},
        {rwSTANDARDRASTERSETIMAGE, fn_804935C4},
        {rwSTANDARDIMAGEGETRASTER, fn_804929F0},
        {rwSTANDARDRASTERDESTROY, fn_80497008},
        {rwSTANDARDRASTERCREATE, fn_80496C58},
        {rwSTANDARDIMAGEFINDRASTERFORMAT, fn_80493888},
        {rwSTANDARDTEXTURESETRASTER, fn_8049714C},
        {rwSTANDARDRASTERLOCK, fn_80495C54},
        {rwSTANDARDRASTERUNLOCK, fn_804960B4},
        {rwSTANDARDRASTERLOCKPALETTE, fn_80496268},
        {rwSTANDARDRASTERUNLOCKPALETTE, fn_80496338},
        {rwSTANDARDRASTERCLEAR, fn_804994DC},
        {rwSTANDARDRASTERCLEARRECT, fn_804994B8},
        {rwSTANDARDRASTERRENDER, fn_80498A4C},
        {rwSTANDARDRASTERRENDERSCALED, fn_80498A9C},
        {rwSTANDARDRASTERRENDERFAST, fn_80498A74},
        {rwSTANDARDSETRASTERCONTEXT, fn_80499520},
        {rwSTANDARDRASTERSUBRASTER, fn_80497168},
        {rwSTANDARDNATIVETEXTUREGETSIZE, fn_8049952C},
        {rwSTANDARDNATIVETEXTUREWRITE, fn_804995B4},
        {rwSTANDARDNATIVETEXTUREREAD, fn_8049988C},
        {rwSTANDARDRASTERGETMIPLEVELS, fn_80495688},
    };
    RwInt32 i;
    RwInt32 numDriverFunctions = sizeof(rwStandards) / sizeof(RwStandard);

    for (i = 0; i < numStandards; i++) {
        standardFunctions[i] = fn_80493C7C;
    }

    /* The range check reads the first entry for every entry */
    while (numDriverFunctions--) {
        if (rwStandards->nStandard < numStandards && rwStandards->nStandard >= 0) {
            standardFunctions[rwStandards[numDriverFunctions].nStandard] =
                rwStandards[numDriverFunctions].fpStandard;
        }
    }
    return TRUE;
}

/* The device's system function */
RwBool fn_80493E8C(RwInt32 request, void *out, void *inOut, RwInt32 numIn) {
    switch (request) {
    case rwDEVICESYSTEMUSEMODE:
        return numIn < 1;
    case rwDEVICESYSTEMGETNUMMODES:
        *(RwInt32 *)out = 1;
        return TRUE;
    case rwDEVICESYSTEMGETMODEINFO:
        if (numIn < 1) {
            RwVideoMode *videoMode = (RwVideoMode *)out;

            switch (lbl_805F15EC) {
            case VI_NTSC:
                *videoMode = lbl_8056E348[0];
                break;
            case VI_PAL:
            case VI_EURGB60:
                *videoMode = lbl_8056E348[1];
                break;
            case VI_MPAL:
                *videoMode = lbl_8056E348[2];
                break;
            case TVFORMATCUSTOM:
                *videoMode = lbl_8056E348[3];
                break;
            }
            return TRUE;
        }
        return FALSE;
    case rwDEVICESYSTEMGETMODE:
        *(RwInt32 *)out = 0;
        return TRUE;
    case rwDEVICESYSTEMFOCUS:
        return TRUE;
    case rwDEVICESYSTEMREGISTER:
        *(RwDevice *)out = *fn_804950A0();
        lbl_805F1650.memoryFuncs = (RwMemoryFunctions *)inOut;
        return TRUE;
    case rwDEVICESYSTEMOPEN:
        return DeviceOpen((RwEngineOpenParams *)inOut);
    case rwDEVICESYSTEMCLOSE:
        GXFlush();
        RwFree(lbl_805F15F4);
        lbl_805F1604 = NULL;
        lbl_805F15F8 = NULL;
        lbl_805F15FC = NULL;
        lbl_805F1600 = NULL;
        lbl_805F1640 = NULL;
        lbl_805F15F4 = NULL;
        return TRUE;
    case rwDEVICESYSTEMSTART:
        return DeviceStart();
    case rwDEVICESYSTEMFINALIZESTART:
        fn_80497198();
        return TRUE;
    case rwDEVICESYSTEMINITIATESTOP:
        fn_8049742C();
        return TRUE;
    case rwDEVICESYSTEMSTOP:
        /* Waits for the queued frames to be shown */
        while (lbl_805F161C == 1 || lbl_805F1610 != lbl_805F1614) {
        }
        return TRUE;
    case rwDEVICESYSTEMINITPIPELINE:
        return TRUE;
    case rwDEVICESYSTEMSTANDARDS:
        return DeviceStandards((RwStandardFunc *)out, numIn);
    case rwDEVICESYSTEMGETMAXTEXTURESIZE:
        *(RwInt32 *)out = MAXTEXTURESIZE;
        return TRUE;
    case rwDEVICESYSTEMGETID:
        *(RwUInt16 *)out = rwDEVICE_GAMECUBE;
        return TRUE;
    }
    return FALSE;
}

/* The projection of the camera being updated */
f32 lbl_8056E430[7] = {GX_PERSPECTIVE, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f};

/* The device */
RwDevice lbl_8056E44C = {
    1.0f,        fn_80493E8C, 0.0f,        0.99999994f, fn_80497904, fn_80497474, fn_80490CBC,
    fn_80490AB8, fn_80490E30, fn_8049140C, NULL,        NULL,        NULL,        NULL,
};

/* Clears a camera's raster: by drawing a rectangle, or with the next frame buffer copy when
 * clearing with copies */
RwBool fn_804945E8(void *out, void *inOut, RwInt32 numIn) {
    RwCamera *camera = (RwCamera *)out;
    RwRGBA *color = (RwRGBA *)inOut;

    if (lbl_805F15E0 == 0) {
        RwRaster *raster = camera->frameBuffer;
        RwRect rect;

        if (lbl_805F15F0 == 0) {
            rect.x = raster->nOffsetX;
            rect.y = raster->nOffsetY;
            rect.w = raster->width;
            rect.h = raster->height;
        } else {
            rect.x = raster->nOffsetX;
            rect.y = raster->nOffsetY * 2;
            rect.w = raster->width;
            rect.h = raster->height * 2;
        }
        fn_80498E08(raster, &rect, color, numIn);
    } else if (numIn & rwCAMERACLEARIMAGE) {
        GXColor clearColor = *(GXColor *)color;

        GXSetCopyClear(clearColor, 0xFFFFFF);
    }
    return TRUE;
}

/* Begins a camera's update: its view matrix and projection, and the viewport and scissor box of
 * its raster */
RwBool fn_804946A4(void *out, void *inOut, RwInt32 numIn) {
    RwCamera *camera = (RwCamera *)inOut;
    RwMatrix viewOffset;
    RwRaster *raster;

    GXSetCurrentGXThread();
    lbl_805F1650.camera = camera;

    /* The camera's view offset as a matrix: unit right and up vectors, the view direction offset
     * by (-x, y) of the view offset and the position by (x, -y); the identity for a zero offset */
    viewOffset.flags = 0;
    viewOffset.right.x = 1.0f;
    viewOffset.right.y = 0.0f;
    viewOffset.right.z = 0.0f;
    viewOffset.up.x = 0.0f;
    viewOffset.up.y = 1.0f;
    viewOffset.up.z = 0.0f;
    viewOffset.at.x = -camera->viewOffset.x;
    viewOffset.at.y = camera->viewOffset.y;
    viewOffset.at.z = 1.0f;
    viewOffset.pos.x = camera->viewOffset.x;
    viewOffset.pos.y = -camera->viewOffset.y;
    viewOffset.pos.z = 0.0f;
    lbl_805E31CC.flags = rwMATRIXTYPEORTHONORMAL | rwMATRIXINTERNALIDENTITY;
    fn_8047F0B0(&lbl_805E31CC, RwFrameGetLTM((RwFrame *)camera->object.parent));
    fn_8047F72C(&lbl_805E31CC, &viewOffset, rwCOMBINEPOSTCONCAT);

    lbl_8056E430[1] = camera->recipViewWindow.x;
    lbl_8056E430[3] = camera->recipViewWindow.y;
    if (camera->projectionType == rwPARALLEL) {
        lbl_8056E430[0] = GX_ORTHOGRAPHIC;
        lbl_8056E430[5] = -1.0f / (camera->farPlane - camera->nearPlane);
        lbl_8056E430[6] = camera->farPlane * lbl_8056E430[5];
    } else {
        lbl_8056E430[0] = GX_PERSPECTIVE;
        lbl_8056E430[5] = -camera->nearPlane / (camera->farPlane - camera->nearPlane);
        lbl_8056E430[6] = camera->farPlane * lbl_8056E430[5];
    }
    GXSetProjectionv(lbl_8056E430);

    raster = camera->frameBuffer;
    /* A camera texture. The original tests the bits of rwRASTERTYPECAMERATEXTURE, so other raster
     * types with bit 0 or bit 2 set take this path too */
    if (raster->cType & rwRASTERTYPECAMERATEXTURE) {
        /* its pixel format has alpha when the raster has */
        if (RWPLUGINOFFSET(RwGameCubeRasterExt, raster->parent, lbl_805F1658)->flags &
            rwGAMECUBERASTERHASALPHA) {
            if (lbl_805F15E8 != GX_PF_RGBA6_Z24) {
                GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
                lbl_805F15E8 = GX_PF_RGBA6_Z24;
            }
        } else if (lbl_805F15E8 != GX_PF_RGB8_Z24) {
            GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
            lbl_805F15E8 = GX_PF_RGB8_Z24;
        }
        GXSetViewport(raster->nOffsetX, raster->nOffsetY, raster->width, raster->height, 0.0f,
                      1.0f);
        GXSetScissor(raster->nOffsetX, raster->nOffsetY, raster->width, raster->height);
        GXSetScissorBoxOffset(0, 0);
    } else {
        if (lbl_805F15E8 != lbl_805F15E4) {
            GXSetPixelFmt(lbl_805F15E4, GX_ZC_LINEAR);
            lbl_805F15E8 = lbl_805F15E4;
        }
        if (lbl_805F15F0 == 0) {
            if (lbl_805F164C->field_rendering) {
                GXSetViewportJitter(raster->nOffsetX, raster->nOffsetY, raster->width,
                                    raster->height, 0.0f, 1.0f, VIGetNextField() ^ 1);
            } else {
                GXSetViewport(raster->nOffsetX, raster->nOffsetY, raster->width, raster->height,
                              0.0f, 1.0f);
            }
            GXSetScissor(raster->nOffsetX, raster->nOffsetY, raster->width, raster->height);
        } else {
            if (lbl_805F164C->field_rendering) {
                GXSetViewportJitter(raster->nOffsetX, raster->nOffsetY, raster->width,
                                    raster->height, 0.0f, 1.0f, VIGetNextField() ^ 1);
            } else {
                GXSetViewport(raster->nOffsetX, raster->nOffsetY * 2, raster->width,
                              raster->height * 2, 0.0f, 1.0f);
            }

            if (lbl_805EDF28 != 0) {
                if ((raster->nOffsetY + raster->height) * 2 <= lbl_805F1648 + 2) {
                    GXSetScissor(raster->nOffsetX, raster->nOffsetY * 2, lbl_805F164C->fbWidth,
                                 raster->height * 2);
                } else if (raster->nOffsetY * 2 > lbl_805F1648 + 2) {
                    GXSetScissor(0, 0, lbl_805F164C->fbWidth, lbl_805F1648 + 2);
                } else {
                    GXSetScissor(raster->nOffsetX, raster->nOffsetY * 2, lbl_805F164C->fbWidth,
                                 lbl_805F1648 + 2);
                }
                GXSetScissorBoxOffset(0, 0);
            } else {
                if (raster->nOffsetY * 2 >= lbl_805F1648 - 2) {
                    GXSetScissor(raster->nOffsetX, raster->nOffsetY * 2, lbl_805F164C->fbWidth,
                                 raster->height * 2);
                } else if ((raster->nOffsetY + raster->height) * 2 < lbl_805F1648 + 2) {
                    GXSetScissor(0, lbl_805F1648 - 2, lbl_805F164C->fbWidth, lbl_805F1648 + 2);
                } else {
                    GXSetScissor(raster->nOffsetX, lbl_805F1648 - 2, lbl_805F164C->fbWidth,
                                 raster->height * 2);
                }
                GXSetScissorBoxOffset(0, lbl_805F1648 - 2);
            }
        }
    }
    return TRUE;
}

/* Ends a camera's update */
RwBool fn_80494C5C(void *out, void *inOut, RwInt32 numIn) {
    lbl_805F1650.camera = NULL;
    return TRUE;
}

/* Whether the frame queue is full (inferred helper) */
static inline RwBool FrameQueueFull(void) {
    RwBool full = FALSE;

    if (lbl_805F160C - lbl_805F1608 == -1 || lbl_805EDF34 - 1 == lbl_805F160C - lbl_805F1608) {
        full = TRUE;
    }
    return full;
}

/* Queues the frame copied to the next external frame buffer, ending at the FIFO's write position,
 * with the draw sync token when withToken is set, and enables the breakpoint at its end */
#define FRAMEQUEUEADD(writePointer, withToken)                                                     \
    do {                                                                                           \
        BOOL enabled_ = OSDisableInterrupts();                                                     \
                                                                                                   \
        lbl_805E3178[lbl_805F160C].breakPoint = (writePointer);                                    \
        lbl_805E3178[lbl_805F160C].frameBuffer = lbl_805F1600;                                     \
        if (withToken) {                                                                           \
            lbl_805F1638[lbl_805F1610] = lbl_805EDF40;                                             \
        }                                                                                          \
        lbl_805F160C = (lbl_805F160C + 1) % lbl_805EDF34;                                          \
        if (withToken) {                                                                           \
            lbl_805F1610 = (lbl_805F1610 + 1) % lbl_805EDF34;                                      \
        }                                                                                          \
        OSRestoreInterrupts(enabled_);                                                             \
                                                                                                   \
        if (lbl_805F1618 == 0) {                                                                   \
            lbl_805F1618 = 1;                                                                      \
            GXEnableBreakPt(writePointer);                                                         \
        }                                                                                          \
    } while (0)

/* Sets the draw sync token and moves to the next (inferred helper) */
static inline void SetDrawSyncToken(void) {
    GXSetDrawSync(lbl_805EDF40);
    lbl_805EDF40 = (lbl_805EDF40 + 1) % rwGAMECUBEMAXSYNCTOKEN;
}

/* Swaps the external frame buffer the next frame is copied to (inferred helper) */
static inline void SwapCopyFrameBuffer(void) {
    lbl_805F1600 = (lbl_805F1600 == lbl_805F15F8) ? lbl_805F15FC : lbl_805F15F8;
}

/* Shows the frame: copies the frame buffer to the next external frame buffer and queues it to be
 * shown, waiting while the queue is full; a double height frame is copied in two halves */
RwBool fn_80494C6C(void *out, void *inOut, RwInt32 numIn) {
    BOOL enabled;
    void *readPointer;
    void *writePointer;
    /* Matching workaround: volatile. The original stores the flag to the stack and reloads it at
     * once before the compare, which the compiler only emits for a volatile object; without the
     * qualifier fn_80494C6C is 264 words against the original 269, 244 of them differing. Revisit
     * when a source form without the qualifier reproduces the reload. */
    volatile RwBool full;

    enabled = OSDisableInterrupts();
    full = FrameQueueFull() != FALSE;
    if (full == TRUE) {
        OSSleepThread(&lbl_805F1630);
    }
    OSRestoreInterrupts(enabled);

    if (lbl_805F15F0 == 0) {
        GXFlush();
        GXGetFifoPtrs(GXGetCPUFifo(), &readPointer, &writePointer);
        FRAMEQUEUEADD(writePointer, TRUE);
        GXCopyDisp(lbl_805F1600, (GXBool)lbl_805F15E0);
        SetDrawSyncToken();
        GXFlush();
        SwapCopyFrameBuffer();
    } else if (lbl_805EDF28 != 0) {
        /* The first half */
        GXFlush();
        GXGetFifoPtrs(GXGetCPUFifo(), &readPointer, &writePointer);
        FRAMEQUEUEADD(writePointer, FALSE);
        GXCopyDisp(lbl_805F1600, (GXBool)lbl_805F15E0);
        SetDrawSyncToken();
        GXFlush();
        lbl_805EDF28 = 0;
    } else {
        /* The second half, copied below the first */
        RwInt32 width;

        GXFlush();
        GXGetFifoPtrs(GXGetCPUFifo(), &readPointer, &writePointer);
        FRAMEQUEUEADD(writePointer, TRUE);
        GXSetCopyClamp(GX_CLAMP_BOTTOM);
        GXSetDispCopySrc(0, 2, lbl_805F164C->fbWidth, lbl_805F164C->efbHeight - 2);
        width = (lbl_805F164C->fbWidth + 15) & 0xFFF0;
        GXCopyDisp((RwUInt8 *)lbl_805F1600 + width * (lbl_805F164C->efbHeight - 2) * 2,
                   (GXBool)lbl_805F15E0);
        SetDrawSyncToken();
        GXSetCopyClamp(GX_CLAMP_TOP | GX_CLAMP_BOTTOM);
        GXSetDispCopySrc(0, 0, lbl_805F164C->fbWidth, lbl_805F164C->efbHeight);
        GXFlush();
        SwapCopyFrameBuffer();
        lbl_805EDF28 = 1;
    }
    return TRUE;
}

/* The device */
RwDevice *fn_804950A0(void) {
    return &lbl_8056E44C;
}

/* Loads a matrix (or the identity) times the camera's view matrix as GX's position matrix, and as
 * its normal matrix when normals is set */
void fn_804950AC(RwMatrix *matrix, RwBool normals) {
    RwMatrix viewMatrix;
    RwMatrix *transform;
    Mtx gxMatrix;

    if (matrix != NULL) {
        viewMatrix.flags = rwMATRIXTYPEORTHONORMAL | rwMATRIXINTERNALIDENTITY;
        fn_8047EBB8(&viewMatrix, matrix, &lbl_805E31CC);
        transform = &viewMatrix;
    } else {
        transform = &lbl_805E31CC;
    }

    /* The right, up, at and pos vectors become the columns of GX's matrix, with their x and z
     * components negated */
    gxMatrix[0][0] = -transform->right.x;
    gxMatrix[0][1] = -transform->up.x;
    gxMatrix[0][2] = -transform->at.x;
    gxMatrix[0][3] = -transform->pos.x;
    gxMatrix[1][0] = transform->right.y;
    gxMatrix[1][1] = transform->up.y;
    gxMatrix[1][2] = transform->at.y;
    gxMatrix[1][3] = transform->pos.y;
    gxMatrix[2][0] = -transform->right.z;
    gxMatrix[2][1] = -transform->up.z;
    gxMatrix[2][2] = -transform->at.z;
    gxMatrix[2][3] = -transform->pos.z;
    GXLoadPosMtxImm(gxMatrix, GX_PNMTX0);
    if (normals) {
        GXLoadNrmMtxImm(gxMatrix, GX_PNMTX0);
    }
    GXSetCurrentMtx(GX_PNMTX0);
}
