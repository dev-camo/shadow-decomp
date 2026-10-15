/* RenderWare rasters: the raster module, creating and destroying rasters, sub rasters, locking
 * pixels and palettes, mip levels and showing a raster. The original file name is unknown. */
#include "rw/rwcore.h"
#include <string.h>

#define rwMEMHINTDUR_EVENT 0x30000
#define rwMEMHINTDUR_GLOBAL 0x40000

/* module ID */
#define rwID_RASTERMODULE 0x407

/* raster types and flags */
#define rwRASTERTYPENORMAL 0x00
#define rwRASTERDONTALLOCATE 0x80 /* the raster uses another raster's pixels */

/* raster formats */
#define rwRASTERFORMATMIPMAP 0x8000

/* raster private flags: the palette locked for reading and for writing */
#define rwRASTERPALETTELOCKEDREAD 0x08
#define rwRASTERPALETTELOCKEDWRITE 0x10
#define rwRASTERPALETTELOCKED (rwRASTERPALETTELOCKEDREAD | rwRASTERPALETTELOCKEDWRITE)

/* standard functions */
#define rwSTANDARDRASTERCREATE 4
#define rwSTANDARDRASTERDESTROY 5
#define rwSTANDARDRASTERSUBRASTER 12
#define rwSTANDARDRASTERLOCK 15
#define rwSTANDARDRASTERUNLOCK 16
#define rwSTANDARDRASTERSHOWRASTER 20
#define rwSTANDARDRASTERLOCKPALETTE 23
#define rwSTANDARDRASTERUNLOCKPALETTE 24
#define rwSTANDARDRASTERGETMIPLEVELS 28

/* The depth of the raster context stack */
#define rwRASTERCONTEXTSTACKSIZE 10

/* The engine globals of the raster module (0x64 bytes) */
typedef struct rwRasterGlobals {
    /* 0x00 */ RwRaster *rasterStack[rwRASTERCONTEXTSTACKSIZE];
    /* 0x28 */ RwInt32 rasterSP;
    /* 0x2C */ RwRaster dummyRaster; /* the bottom of the stack */
    /* 0x60 */ RwFreeList *rasterFreeList;
} rwRasterGlobals;

#define RWRASTERGLOBAL(var)                                                                        \
    (RWPLUGINOFFSET(rwRasterGlobals, lbl_805F15B4, lbl_805F15C8.globalsOffset)->var)

/* The raster plugin registry */
RwPluginRegistry lbl_8056E018 = {sizeof(RwRaster), sizeof(RwRaster), 0, 0, NULL, NULL};

RwFreeList lbl_805E2CE8; /* the raster free list */

RwInt32 lbl_805EDEF8 = 128; /* rasters per free list block */
RwInt32 lbl_805EDEFC = 1;   /* free list blocks to preallocate */

RwModuleInfo lbl_805F15C8; /* the raster module */

RwRaster *fn_80489B2C(RwRaster *raster, RwInt16 *xOffset, RwInt16 *yOffset) {
    *xOffset = raster->nOffsetX;
    *yOffset = raster->nOffsetY;
    return raster;
}

RwRaster *fn_80489B40(RwRaster *raster) {
    lbl_805F15B4->stdFunc[rwSTANDARDRASTERUNLOCK](NULL, raster, 0);
    return raster;
}

RwRaster *fn_80489B88(RwRaster *raster) {
    lbl_805F15B4->stdFunc[rwSTANDARDRASTERUNLOCKPALETTE](NULL, raster, 0);
    raster->privateFlags &= ~rwRASTERPALETTELOCKED;
    return raster;
}

RwBool fn_80489BDC(RwRaster *raster) {
    fn_80482144(&lbl_8056E018, raster);
    lbl_805F15B4->stdFunc[rwSTANDARDRASTERDESTROY](NULL, raster, 0);
    RwFreeListFree(RWRASTERGLOBAL(rasterFreeList), raster);
    return TRUE;
}

RwInt32 fn_80489C54(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB,
                    RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB) {
    return fn_80481DC8(&lbl_8056E018, size, pluginID, constructCB, destructCB, copyCB);
}

RwUInt8 *fn_80489C94(RwRaster *raster, RwInt32 lockMode) {
    RwUInt8 *palette;

    if (lbl_805F15B4->stdFunc[rwSTANDARDRASTERLOCKPALETTE](&palette, raster, lockMode)) {
        return palette;
    }
    return NULL;
}

/* The number of mip levels: 1 unless the raster is mipmapped, -1 on failure */
RwInt32 fn_80489CE4(RwRaster *raster) {
    RwInt32 numLevels;

    if (!((raster->cFormat << 8) & rwRASTERFORMATMIPMAP)) {
        return 1;
    }
    if (lbl_805F15B4->stdFunc[rwSTANDARDRASTERGETMIPLEVELS](&numLevels, raster, 0)) {
        return numLevels;
    }
    return -1;
}

/* Shows a raster on the device; frames are started first (inferred) */
RwRaster *fn_80489D44(RwRaster *raster, void *device, RwUInt32 flags) {
    RwStandardFunc showRaster = lbl_805F15B4->stdFunc[rwSTANDARDRASTERSHOWRASTER];

    fn_804807CC();
    if (showRaster(raster, device, flags)) {
        return raster;
    }
    return NULL;
}

/* Makes a raster created without pixels a view of a rectangle of another raster */
RwRaster *fn_80489DC4(RwRaster *subRaster, RwRaster *raster, RwRect *rect) {
    if (!(subRaster->cFlags & rwRASTERDONTALLOCATE)) {
        return NULL;
    }
    subRaster->width = rect->w;
    subRaster->height = rect->h;
    subRaster->nOffsetX = raster->nOffsetX + (RwInt16)rect->x;
    subRaster->nOffsetY = raster->nOffsetY + (RwInt16)rect->y;
    if (lbl_805F15B4->stdFunc[rwSTANDARDRASTERSUBRASTER](subRaster, raster, 0)) {
        subRaster->parent = raster->parent;
        return subRaster;
    }
    return NULL;
}

RwRaster *fn_80489E74(RwInt32 width, RwInt32 height, RwInt32 depth, RwInt32 flags) {
    RwRaster *raster;

    raster = (RwRaster *)RwFreeListAlloc(RWRASTERGLOBAL(rasterFreeList),
                                         rwMEMHINTDUR_EVENT | rwID_RASTERMODULE);
    if (raster != NULL) {
        RwStandardFunc createRaster = lbl_805F15B4->stdFunc[rwSTANDARDRASTERCREATE];

        raster->privateFlags = 0;
        raster->cFlags = 0;
        raster->width = width;
        raster->height = height;
        raster->nOffsetX = 0;
        raster->nOffsetY = 0;
        raster->depth = depth;
        raster->parent = raster;
        raster->cpPixels = NULL;
        raster->palette = NULL;
        if (!createRaster(NULL, raster, flags)) {
            RwFreeListFree(RWRASTERGLOBAL(rasterFreeList), raster);
            return NULL;
        }
        fn_8048209C(&lbl_8056E018, raster);
        return raster;
    }
    return NULL;
}

/* Locks a mip level's pixels: the level is passed above the lock mode */
RwUInt8 *fn_80489F70(RwRaster *raster, RwUInt8 level, RwInt32 lockMode) {
    RwUInt8 *pixels;

    if (lbl_805F15B4->stdFunc[rwSTANDARDRASTERLOCK](&pixels, raster, lockMode + (level << 8))) {
        return pixels;
    }
    return NULL;
}

void *fn_80489FC0(void *instance, RwInt32 offset, RwInt32 size) {
    if (RWRASTERGLOBAL(rasterFreeList) != NULL) {
        fn_8047FCDC(RWRASTERGLOBAL(rasterFreeList));
        RWRASTERGLOBAL(rasterFreeList) = NULL;
    }
    lbl_805F15C8.numInstances--;
    return instance;
}

/* Opens the raster module: the raster context stack holds a dummy raster without pixels, and the
 * free list of rasters with their plugins' data */
void *fn_8048A02C(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F15C8.globalsOffset = offset;
    memset(&RWRASTERGLOBAL(dummyRaster), 0, sizeof(RwRaster));
    RWRASTERGLOBAL(dummyRaster).width = 0;
    RWRASTERGLOBAL(dummyRaster).height = 0;
    RWRASTERGLOBAL(dummyRaster).depth = 0;
    RWRASTERGLOBAL(dummyRaster).cFlags = rwRASTERDONTALLOCATE;
    RWRASTERGLOBAL(dummyRaster).cpPixels = NULL;
    RWRASTERGLOBAL(dummyRaster).palette = NULL;
    RWRASTERGLOBAL(dummyRaster).cType = rwRASTERTYPENORMAL;
    RWRASTERGLOBAL(rasterSP) = 0;
    RWRASTERGLOBAL(rasterStack)[0] = &RWRASTERGLOBAL(dummyRaster);

    RWRASTERGLOBAL(rasterFreeList) =
        fn_8047FC04(lbl_8056E018.sizeOfStruct, lbl_805EDEF8, 4, lbl_805EDEFC, &lbl_805E2CE8,
                    rwMEMHINTDUR_GLOBAL | rwID_RASTERMODULE);
    if (RWRASTERGLOBAL(rasterFreeList) == NULL) {
        return NULL;
    }
    lbl_805F15C8.numInstances++;
    return instance;
}
