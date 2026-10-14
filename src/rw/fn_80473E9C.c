/* RenderWare 2D toolkit: opening and closing the toolkit for a camera, the current
 * transformation stack, and the matrices that take the transformation to the camera's view. The
 * original file name is unknown. */
#include "rw/rt2d.h"

Rt2dGlobalVars lbl_805E1950;

RwCamera *fn_80473E9C(RwCamera *camera) {
    return lbl_805E1950.cameraEndUpdate(camera);
}

/* The current transformation */
#define CTM() (lbl_805E1950.ctmStack[lbl_805E1950.ctmIndex])

RwBool fn_80473ECC(void) {
    RwMatrixSetIdentity(CTM());
    lbl_805E1950.deviceMatrixValid = FALSE;
    lbl_805E1950.pixelSizeValid = FALSE;
    return TRUE;
}

/* The device's x and y steps and origin in the current transformation, at the layer depth for a
 * perspective camera */
RwBool fn_80473FD0(RwV2d *xStep, RwV2d *yStep, RwV2d *origin) {
    RwReal depth;
    RwV2d viewWindow;
    RwV2d rasterSize;
    RwV3d v;
    RwMatrix *inverse;

    if (lbl_805E1950.camera->projectionType == rwPARALLEL) {
        depth = 1.0f;
    } else {
        depth = lbl_805E1950.layerDepth;
    }
    viewWindow = lbl_805E1950.viewWindow;
    rasterSize = lbl_805E1950.rasterSize;
    fn_804749CC();
    inverse = &lbl_805E1950.inverseDeviceMatrix;

    v.x = -depth * (viewWindow.x / rasterSize.x);
    v.y = 0.0f;
    v.z = 0.0f;
    fn_80482504(&v, &v, inverse);
    xStep->x = v.x;
    xStep->y = v.y;

    v.x = 0.0f;
    v.y = depth * (viewWindow.y / rasterSize.y);
    v.z = 0.0f;
    fn_80482504(&v, &v, inverse);
    yStep->x = v.x;
    yStep->y = v.y;

    v.x = 0.5f * viewWindow.x * depth;
    v.y = 0.5f * -viewWindow.y * depth;
    v.z = 0.0f;
    fn_80482458(&v, &v, inverse);
    origin->x = v.x;
    origin->y = v.y;
    return TRUE;
}

RwReal fn_80474168(void) {
    if (!lbl_805E1950.pixelSizeValid) {
        RwV3d v;

        fn_804749CC();
        v.x = lbl_805E1950.field_0x324 *
              (lbl_805E1950.layerDepth * (lbl_805E1950.viewWindow.x / lbl_805E1950.rasterSize.x));
        v.y = 0.0f;
        v.z = 0.0f;
        fn_80482504(&v, &v, &lbl_805E1950.inverseDeviceMatrix);
        lbl_805E1950.pixelSizeValid = TRUE;
        lbl_805E1950.pixelSizeSq = v.x * v.x + v.y * v.y + v.z * v.z;
    }
    return lbl_805E1950.pixelSizeSq;
}

RwBool fn_80474228(RwReal depth) {
    if (depth > 0.0f) {
        lbl_805E1950.layerDepth = depth;
        lbl_805E1950.deviceMatrixValid = FALSE;
        return TRUE;
    }
    return FALSE;
}

/* Opens the toolkit for a camera, wrapping the camera's end update */
void fn_80474258(RwCamera *camera) {
    lbl_805E1950.field_0x31C = 0;
    fn_804772C4();
    fn_80476A58();
    fn_804704D4();
    fn_80473260();
    fn_8047721C();
    fn_80474498();

    if (camera != NULL) {
        RwRaster *raster = camera->frameBuffer;

        if (raster != NULL) {
            RwInt32 width = raster->width;
            RwInt32 height = raster->height;

            lbl_805E1950.rasterSize.x = width;
            lbl_805E1950.rasterSize.y = height;
            lbl_805E1950.viewWindow.x = 2.0f * camera->viewWindow.x;
            lbl_805E1950.viewWindow.y = 2.0f * camera->viewWindow.y;
            if (camera->endUpdate != fn_80473E9C && lbl_805E1950.camera != camera) {
                lbl_805E1950.cameraEndUpdate = camera->endUpdate;
                camera->endUpdate = fn_80473E9C;
            }
            lbl_805E1950.camera = camera;
            lbl_805E1950.deviceMatrixValid = FALSE;
            lbl_805E1950.field_0x338.x = 0.0f;
            lbl_805E1950.field_0x338.y = 0.0f;
            if (lbl_805E1950.viewWindow.x > lbl_805E1950.viewWindow.y) {
                lbl_805E1950.aspect.x = lbl_805E1950.viewWindow.x / lbl_805E1950.viewWindow.y;
                lbl_805E1950.aspect.y = 1.0f;
            } else {
                lbl_805E1950.aspect.x = 1.0f;
                lbl_805E1950.aspect.y = lbl_805E1950.viewWindow.y / lbl_805E1950.viewWindow.x;
            }
        }
    } else {
        lbl_805E1950.camera = camera;
    }
}

void fn_804743AC(void) {
    fn_804743F0();
    fn_804771E0();
    fn_80473118();
    fn_80470474();
    fn_80476A0C();
    fn_8047726C();
    lbl_805E1950.camera = NULL;
}

void fn_804743F0(void) {
    RwInt32 i;

    for (i = 0; i < rt2dCTMSTACKSIZE; i++) {
        if (lbl_805E1950.ctmStack[i] != NULL) {
            fn_8047F958(lbl_805E1950.ctmStack[i]);
            lbl_805E1950.ctmStack[i] = NULL;
        }
    }
    lbl_805E1950.ctmIndex = 0;
    if (lbl_805E1950.field_0x944 != NULL) {
        RwFree(lbl_805E1950.field_0x944);
        lbl_805E1950.field_0x944 = NULL;
        lbl_805E1950.field_0x948 = 0;
    }
}

RwBool fn_80474498(void) {
    RwInt32 i;

    lbl_805E1950.ctmIndex = 0;
    lbl_805E1950.ctmStack[0] = fn_8047F994();
    RwMatrixSetIdentity(CTM());
    lbl_805E1950.deviceMatrixValid = FALSE;
    lbl_805E1950.pixelSizeValid = FALSE;
    lbl_805E1950.field_0x324 = 0.5f;
    lbl_805E1950.pixelSizeValid = FALSE;
    lbl_805E1950.layerDepth = 1.0f;
    lbl_805E1950.deviceMatrixValid = FALSE;
    lbl_805E1950.field_0x330 = 7;
    RwMatrixSetIdentity(&lbl_805E1950.baseMatrix);
    for (i = 0; i < 64; i++) {
        lbl_805E1950.indices[i] = i;
    }
    for (i = 1; i < rt2dCTMSTACKSIZE; i++) {
        lbl_805E1950.ctmStack[i] = NULL;
    }
    lbl_805E1950.camera = NULL;
    lbl_805E1950.field_0x338.x = 0.0f;
    lbl_805E1950.field_0x338.y = 0.0f;
    lbl_805E1950.aspect.x = 1.0f;
    lbl_805E1950.aspect.y = 1.0f;
    lbl_805E1950.field_0x944 = NULL;
    lbl_805E1950.field_0x948 = 0;
    return TRUE;
}

RwBool fn_804747A0(void) {
    RwBool result = lbl_805E1950.ctmIndex < rt2dCTMSTACKSIZE - 1;

    if (result) {
        lbl_805E1950.ctmIndex++;
        if (lbl_805E1950.ctmStack[lbl_805E1950.ctmIndex] == NULL) {
            lbl_805E1950.ctmStack[lbl_805E1950.ctmIndex] = fn_8047F994();
        }
        *lbl_805E1950.ctmStack[lbl_805E1950.ctmIndex] =
            *lbl_805E1950.ctmStack[lbl_805E1950.ctmIndex - 1];
    }
    return result;
}

RwBool fn_804748C8(void) {
    RwBool result = lbl_805E1950.ctmIndex > 0;

    if (result) {
        lbl_805E1950.ctmIndex--;
        lbl_805E1950.deviceMatrixValid = FALSE;
        lbl_805E1950.pixelSizeValid = FALSE;
    }
    return result;
}

RwBool fn_804748FC(RwReal x, RwReal y) {
    RwV3d scale;

    scale.x = x;
    scale.y = y;
    scale.z = 1.0f;
    fn_8047F398(CTM(), &scale, rwCOMBINEPRECONCAT);
    lbl_805E1950.deviceMatrixValid = FALSE;
    lbl_805E1950.pixelSizeValid = FALSE;
    return TRUE;
}

RwBool fn_80474964(RwReal x, RwReal y) {
    RwV3d translation;

    translation.x = x;
    translation.y = y;
    translation.z = 0.0f;
    fn_8047F59C(CTM(), &translation, rwCOMBINEPRECONCAT);
    lbl_805E1950.deviceMatrixValid = FALSE;
    lbl_805E1950.pixelSizeValid = FALSE;
    return TRUE;
}

/* The current transformation in the camera's frame: scaled to the layer depth and the view
 * window, with y down from the view window's top left */
RwMatrix *fn_804749CC(void) {
    if (!lbl_805E1950.deviceMatrixValid) {
        RwMatrix matrix;
        RwV3d scale;
        RwV3d translation;
        RwV2d viewWindow;
        RwReal depth;
        RwReal size;

        if (lbl_805E1950.camera->projectionType == rwPARALLEL) {
            depth = 1.0f;
        } else {
            depth = lbl_805E1950.layerDepth;
        }
        fn_8047EBB8(&matrix, CTM(), &lbl_805E1950.baseMatrix);
        scale.x = depth;
        viewWindow = lbl_805E1950.viewWindow;
        scale.y = depth;
        scale.z = 1.0f;
        fn_8047F398(&matrix, &scale, rwCOMBINEPOSTCONCAT);

        size = viewWindow.x < viewWindow.y ? viewWindow.x : viewWindow.y;
        scale.x = -size;
        scale.y = size;
        scale.z = 1.0f;
        fn_8047F398(&matrix, &scale, rwCOMBINEPOSTCONCAT);

        translation.x = 0.5f * viewWindow.x * depth;
        translation.y = -(0.5f * viewWindow.y * depth);
        translation.z = 0.0f;
        fn_8047F59C(&matrix, &translation, rwCOMBINEPOSTCONCAT);

        lbl_805E1950.deviceMatrix = matrix;
        fn_8047F0B0(&lbl_805E1950.inverseDeviceMatrix, &matrix);
        lbl_805E1950.deviceMatrixValid = TRUE;
    }
    fn_8047EBB8(&lbl_805E1950.ltm, &lbl_805E1950.deviceMatrix,
                RwFrameGetLTM(lbl_805E1950.camera->object.parent));
    return &lbl_805E1950.ltm;
}
