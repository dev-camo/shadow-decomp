/* RenderWare cameras: their view matrices, frustum planes and corners, clip planes and depth
 * scale, and creating and destroying them. The original file name is unknown. */
#include "rw/rwcore.h"
#include "layout_check.h"

#define rwMEMHINTDUR_EVENT 0x30000
#define rwMEMHINTDUR_GLOBAL 0x40000

/* object type and chunk ID */
#define rwCAMERA 4
#define rwID_CAMERA 0x5

#define rwPERSPECTIVE 1

/* standard functions */
#define rwSTANDARDCAMERABEGINUPDATE 1
#define rwSTANDARDCAMERAENDUPDATE 10
#define rwSTANDARDCAMERACLEAR 21

/* errors (inferred) */
#define E_RW_INVCAMERAPROJ 0x80000003 /* reported with a message */

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

/* The engine globals of the camera module */
typedef struct rwCameraGlobals {
    /* 0x0 */ RwFreeList *cameraFreeList;
} rwCameraGlobals;
CHECK_SIZEOF(rwCameraGlobals, 0x4);

#define RWCAMERAGLOBAL(var)                                                                        \
    (RWPLUGINOFFSET(rwCameraGlobals, lbl_805F15B4, lbl_805F15A8.globalsOffset)->var)

/* The frame a camera is attached to */
#define CAMERAFRAME(camera) ((RwFrame *)(camera)->object.parent)

/* Which of a box's two corners on each axis is closest to the plane: 0 when the normal's
 * component is negative, else 1 */
#define FRUSTUMPLANECLOSEST(component) ((RwUInt8)((((RwSplitBits *)&(component))->nInt >> 31) + 1))

/* Sets a frustum plane's closest corner flags from its normal */
#define FRUSTUMPLANESETCLOSEST(frustumPlane)                                                       \
    do {                                                                                           \
        (frustumPlane)->closestX = FRUSTUMPLANECLOSEST((frustumPlane)->plane.normal.x);            \
        (frustumPlane)->closestY = FRUSTUMPLANECLOSEST((frustumPlane)->plane.normal.y);            \
        (frustumPlane)->closestZ = FRUSTUMPLANECLOSEST((frustumPlane)->plane.normal.z);            \
    } while (0)

/* Sets a frustum plane's normal to the normalized cross product of two edges, its distance from
 * a point on it, and its closest corner flags */
#define FRUSTUMPLANEFROMEDGES(frustumPlane, edgeA, edgeB, point)                                   \
    do {                                                                                           \
        RwReal recip_;                                                                             \
                                                                                                   \
        (frustumPlane)->plane.normal.x = (edgeA).y * (edgeB).z - (edgeA).z * (edgeB).y;            \
        (frustumPlane)->plane.normal.y = (edgeA).z * (edgeB).x - (edgeA).x * (edgeB).z;            \
        (frustumPlane)->plane.normal.z = (edgeA).x * (edgeB).y - (edgeA).y * (edgeB).x;            \
        recip_ = fn_80482394((frustumPlane)->plane.normal.x * (frustumPlane)->plane.normal.x +     \
                             (frustumPlane)->plane.normal.y * (frustumPlane)->plane.normal.y +     \
                             (frustumPlane)->plane.normal.z * (frustumPlane)->plane.normal.z);     \
        (frustumPlane)->plane.normal.x *= recip_;                                                  \
        (frustumPlane)->plane.normal.y *= recip_;                                                  \
        (frustumPlane)->plane.normal.z *= recip_;                                                  \
        (frustumPlane)->plane.distance = (point).x * (frustumPlane)->plane.normal.x +              \
                                         (point).y * (frustumPlane)->plane.normal.y +              \
                                         (point).z * (frustumPlane)->plane.normal.z;               \
        FRUSTUMPLANESETCLOSEST(frustumPlane);                                                      \
    } while (0)

/* Sets a frustum plane opposite another, through a point */
#define FRUSTUMPLANEOPPOSITE(frustumPlane, other, point)                                           \
    do {                                                                                           \
        (frustumPlane)->plane.normal.x = -(other)->plane.normal.x;                                 \
        (frustumPlane)->plane.normal.y = -(other)->plane.normal.y;                                 \
        (frustumPlane)->plane.normal.z = -(other)->plane.normal.z;                                 \
        (frustumPlane)->plane.distance = (point).x * (frustumPlane)->plane.normal.x +              \
                                         (point).y * (frustumPlane)->plane.normal.y +              \
                                         (point).z * (frustumPlane)->plane.normal.z;               \
        FRUSTUMPLANESETCLOSEST(frustumPlane);                                                      \
    } while (0)

/* The camera plugin registry */
RwPluginRegistry lbl_8056DF70 = {sizeof(RwCamera), sizeof(RwCamera), 0, 0, NULL, NULL};

RwFreeList lbl_805E2B20; /* the camera free list */

RwInt32 lbl_805EDED8 = 4; /* cameras per free list block */
RwInt32 lbl_805EDEDC = 1; /* free list blocks to preallocate */

RwModuleInfo lbl_805F15A8; /* the camera module */

/* Sets the depth scale and shift that map a camera's reciprocal depth (or depth, for a parallel
 * camera) into the device's depth range, just inside it */
static void CameraUpdateZShiftScale(RwCamera *camera) {
    RwReal zBufferNear = lbl_805F15B4->dOpenDevice.zBufferNear;
    RwReal zBufferFar = lbl_805F15B4->dOpenDevice.zBufferFar;
    RwReal nearDepth;
    RwReal farDepth;
    RwReal inset;
    RwReal zScale;

    switch (camera->projectionType) {
    case rwPARALLEL:
        farDepth = camera->farPlane;
        nearDepth = camera->nearPlane;
        break;
    case rwPERSPECTIVE:
    default:
        farDepth = 1.0f / camera->farPlane;
        nearDepth = 1.0f / camera->nearPlane;
        break;
    }

    inset = 0.0001f * (zBufferFar - zBufferNear);
    zBufferFar -= inset;
    zBufferNear += inset;
    zScale = (zBufferFar - zBufferNear) / (farDepth - nearDepth);
    camera->zScale = zScale;
    camera->zShift = 0.5f * ((zBufferFar + zBufferNear) - zScale * (farDepth + nearDepth));
}

/* The perspective frustum's corners in the world, near then far, and its planes: far, near, then
 * the four sides */
void fn_804842C8(RwCamera *camera) {
    RwMatrix *ltm = &CAMERAFRAME(camera)->ltm;
    RwV3d *corners = camera->frustumCorners;
    RwFrustumPlane *frustumPlane;
    RwV3d scaledRight;
    RwV3d scaledUp;
    RwV3d temp;
    RwV3d offset;
    RwV3d edgeA;
    RwV3d edgeB;
    RwInt32 i;

    /* The corners of the view window, a unit along the view direction */
    scaledRight.x = ltm->right.x * camera->viewWindow.x;
    scaledRight.y = ltm->right.y * camera->viewWindow.x;
    scaledRight.z = ltm->right.z * camera->viewWindow.x;
    scaledUp.x = ltm->up.x * camera->viewWindow.y;
    scaledUp.y = ltm->up.y * camera->viewWindow.y;
    scaledUp.z = ltm->up.z * camera->viewWindow.y;
    temp.x = ltm->at.x + scaledRight.x;
    temp.y = ltm->at.y + scaledRight.y;
    temp.z = ltm->at.z + scaledRight.z;
    temp.x += scaledUp.x;
    temp.y += scaledUp.y;
    temp.z += scaledUp.z;
    corners[0] = temp;
    scaledRight.x *= 2.0f;
    scaledRight.y *= 2.0f;
    scaledRight.z *= 2.0f;
    temp.x -= scaledRight.x;
    temp.y -= scaledRight.y;
    temp.z -= scaledRight.z;
    corners[1] = temp;
    scaledUp.x *= 2.0f;
    scaledUp.y *= 2.0f;
    scaledUp.z *= 2.0f;
    temp.x -= scaledUp.x;
    temp.y -= scaledUp.y;
    temp.z -= scaledUp.z;
    corners[2] = temp;
    temp.x += scaledRight.x;
    temp.y += scaledRight.y;
    temp.z += scaledRight.z;
    corners[3] = temp;

    /* Each corner at the near and far planes, from the view offset */
    offset.x = ltm->up.x * camera->viewOffset.y + ltm->right.x * -camera->viewOffset.x;
    offset.y = ltm->up.y * camera->viewOffset.y + ltm->right.y * -camera->viewOffset.x;
    offset.z = ltm->up.z * camera->viewOffset.y + ltm->right.z * -camera->viewOffset.x;
    for (i = 0; i < 4; i++) {
        temp.x = corners[i].x - offset.x;
        temp.y = corners[i].y - offset.y;
        temp.z = corners[i].z - offset.z;
        corners[i].x = offset.x + ltm->pos.x;
        corners[i].y = offset.y + ltm->pos.y;
        corners[i].z = offset.z + ltm->pos.z;
        corners[i].x = temp.x * camera->nearPlane + corners[i].x;
        corners[i].y = temp.y * camera->nearPlane + corners[i].y;
        corners[i].z = temp.z * camera->nearPlane + corners[i].z;
        corners[i + 4].x = offset.x + ltm->pos.x;
        corners[i + 4].y = offset.y + ltm->pos.y;
        corners[i + 4].z = offset.z + ltm->pos.z;
        corners[i + 4].x = temp.x * camera->farPlane + corners[i + 4].x;
        corners[i + 4].y = temp.y * camera->farPlane + corners[i + 4].y;
        corners[i + 4].z = temp.z * camera->farPlane + corners[i + 4].z;
    }

    /* The far plane faces along the view, through the far corners */
    frustumPlane = &camera->frustumPlanes[0];
    frustumPlane->plane.normal = ltm->at;
    frustumPlane->plane.distance =
        corners[4].x * ltm->at.x + corners[4].y * ltm->at.y + corners[4].z * ltm->at.z;
    FRUSTUMPLANESETCLOSEST(frustumPlane);

    /* The near plane faces back, through the near corners */
    FRUSTUMPLANEOPPOSITE(&camera->frustumPlanes[1], &camera->frustumPlanes[0], corners[0]);

    /* The sides, through the corners about the far plane's second and fourth corners */
    temp.x = corners[1].x - corners[5].x;
    temp.y = corners[1].y - corners[5].y;
    temp.z = corners[1].z - corners[5].z;
    edgeB.x = corners[6].x - corners[5].x;
    edgeB.y = corners[6].y - corners[5].y;
    edgeB.z = corners[6].z - corners[5].z;
    FRUSTUMPLANEFROMEDGES(&camera->frustumPlanes[2], temp, edgeB, corners[1]);
    edgeA.x = corners[4].x - corners[5].x;
    edgeA.y = corners[4].y - corners[5].y;
    edgeA.z = corners[4].z - corners[5].z;
    FRUSTUMPLANEFROMEDGES(&camera->frustumPlanes[3], edgeA, temp, corners[1]);

    temp.x = corners[3].x - corners[7].x;
    temp.y = corners[3].y - corners[7].y;
    temp.z = corners[3].z - corners[7].z;
    edgeB.x = corners[4].x - corners[7].x;
    edgeB.y = corners[4].y - corners[7].y;
    edgeB.z = corners[4].z - corners[7].z;
    FRUSTUMPLANEFROMEDGES(&camera->frustumPlanes[4], temp, edgeB, corners[3]);
    edgeA.x = corners[6].x - corners[7].x;
    edgeA.y = corners[6].y - corners[7].y;
    edgeA.z = corners[6].z - corners[7].z;
    FRUSTUMPLANEFROMEDGES(&camera->frustumPlanes[5], edgeA, temp, corners[3]);
}

/* The perspective view matrix: from the world into the camera's view, with the view window and
 * offset mapped into [0, 1] */
RwCamera *fn_80484AAC(RwCamera *camera) {
    RwMatrix *ltm = &CAMERAFRAME(camera)->ltm;
    RwMatrix *viewMatrix = &camera->viewMatrix;
    RwReal scale;
    RwReal shift;
    RwV3d scaledAxis;
    RwV3d row;

    scale = -0.5f * camera->recipViewWindow.x;
    shift = 0.5f - scale * camera->viewOffset.x;
    scaledAxis.x = ltm->right.x * scale;
    scaledAxis.y = ltm->right.y * scale;
    scaledAxis.z = ltm->right.z * scale;
    row.x = ltm->at.x * shift + scaledAxis.x;
    row.y = ltm->at.y * shift + scaledAxis.y;
    row.z = ltm->at.z * shift + scaledAxis.z;
    viewMatrix->right.x = row.x;
    viewMatrix->up.x = row.y;
    viewMatrix->at.x = row.z;
    viewMatrix->pos.x =
        0.5f - (shift + (ltm->pos.x * row.x + ltm->pos.y * row.y + ltm->pos.z * row.z));

    scale = -0.5f * camera->recipViewWindow.y;
    shift = scale * camera->viewOffset.y + 0.5f;
    scaledAxis.x = ltm->up.x * scale;
    scaledAxis.y = ltm->up.y * scale;
    scaledAxis.z = ltm->up.z * scale;
    row.x = ltm->at.x * shift + scaledAxis.x;
    row.y = ltm->at.y * shift + scaledAxis.y;
    row.z = ltm->at.z * shift + scaledAxis.z;
    viewMatrix->right.y = row.x;
    viewMatrix->up.y = row.y;
    viewMatrix->at.y = row.z;
    viewMatrix->pos.y =
        0.5f - (shift + (ltm->pos.x * row.x + ltm->pos.y * row.y + ltm->pos.z * row.z));

    viewMatrix->right.z = ltm->at.x;
    viewMatrix->up.z = ltm->at.y;
    viewMatrix->at.z = ltm->at.z;
    viewMatrix->pos.z = -(ltm->pos.x * ltm->at.x + ltm->pos.y * ltm->at.y + ltm->pos.z * ltm->at.z);

    fn_8047E990(viewMatrix, NULL);
    return camera;
}

/* The parallel frustum's corners in the world, near then far, and its planes: far, near, then
 * the four sides */
void fn_80484C10(RwCamera *camera) {
    RwV3d *corners = camera->frustumCorners;
    RwMatrix *ltm = &CAMERAFRAME(camera)->ltm;
    RwReal width = camera->viewWindow.x;
    RwReal height = camera->viewWindow.y;
    RwReal nearPlane = camera->nearPlane;
    RwReal farPlane = camera->farPlane;
    RwReal offsetX = -camera->viewOffset.x;
    RwReal offsetY = camera->viewOffset.y;
    RwReal nearOffsetX = (1.0f - nearPlane) * offsetX;
    RwReal nearOffsetY = (1.0f - nearPlane) * offsetY;
    RwReal farOffsetX = (1.0f - farPlane) * offsetX;
    RwReal farOffsetY = (1.0f - farPlane) * offsetY;
    RwV3d edgeA;
    RwV3d edgeB;
    RwV3d edgeC;

    /* The view window's corners at the near and far planes, in the camera's space */
    corners[0].z = corners[1].z = corners[2].z = corners[3].z = nearPlane;
    corners[4].z = corners[5].z = corners[6].z = corners[7].z = farPlane;
    corners[0].x = corners[3].x = width + nearOffsetX;
    corners[1].x = corners[2].x = -width + nearOffsetX;
    corners[4].x = corners[7].x = width + farOffsetX;
    corners[5].x = corners[6].x = -width + farOffsetX;
    corners[0].y = corners[1].y = height + nearOffsetY;
    corners[2].y = corners[3].y = -height + nearOffsetY;
    corners[4].y = corners[5].y = height + farOffsetY;
    corners[6].y = corners[7].y = -height + farOffsetY;
    fn_804824A4(corners, corners, 8, ltm);

    /* The far plane faces along the view, through the far corners */
    camera->frustumPlanes[0].plane.normal = ltm->at;
    camera->frustumPlanes[0].plane.distance =
        corners[4].x * camera->frustumPlanes[0].plane.normal.x +
        corners[4].y * camera->frustumPlanes[0].plane.normal.y +
        corners[4].z * camera->frustumPlanes[0].plane.normal.z;
    FRUSTUMPLANESETCLOSEST(&camera->frustumPlanes[0]);

    /* The near plane faces back, through the near corners */
    FRUSTUMPLANEOPPOSITE(&camera->frustumPlanes[1], &camera->frustumPlanes[0], corners[0]);

    /* Two sides from the far plane's second corner, and the two opposite sides */
    edgeA.x = corners[1].x - corners[5].x;
    edgeA.y = corners[1].y - corners[5].y;
    edgeA.z = corners[1].z - corners[5].z;
    edgeB.x = corners[6].x - corners[5].x;
    edgeB.y = corners[6].y - corners[5].y;
    edgeB.z = corners[6].z - corners[5].z;
    FRUSTUMPLANEFROMEDGES(&camera->frustumPlanes[2], edgeA, edgeB, corners[1]);
    edgeC.x = corners[4].x - corners[5].x;
    edgeC.y = corners[4].y - corners[5].y;
    edgeC.z = corners[4].z - corners[5].z;
    FRUSTUMPLANEFROMEDGES(&camera->frustumPlanes[3], edgeC, edgeA, corners[1]);
    FRUSTUMPLANEOPPOSITE(&camera->frustumPlanes[4], &camera->frustumPlanes[2], corners[3]);
    FRUSTUMPLANEOPPOSITE(&camera->frustumPlanes[5], &camera->frustumPlanes[3], corners[3]);
}

/* The parallel view matrix: from the world into the camera's view, with the view window and
 * offset mapped into [0, 1] */
RwCamera *fn_80485124(RwCamera *camera) {
    RwMatrix *ltm = &CAMERAFRAME(camera)->ltm;
    RwMatrix *viewMatrix = &camera->viewMatrix;
    RwReal scale;
    RwReal shift;
    RwV3d scaledAxis;
    RwV3d row;

    scale = -0.5f * camera->recipViewWindow.x;
    shift = -(scale * camera->viewOffset.x);
    scaledAxis.x = ltm->right.x * scale;
    scaledAxis.y = ltm->right.y * scale;
    scaledAxis.z = ltm->right.z * scale;
    row.x = ltm->at.x * shift + scaledAxis.x;
    row.y = ltm->at.y * shift + scaledAxis.y;
    row.z = ltm->at.z * shift + scaledAxis.z;
    viewMatrix->right.x = row.x;
    viewMatrix->up.x = row.y;
    viewMatrix->at.x = row.z;
    viewMatrix->pos.x =
        0.5f - (shift + (ltm->pos.x * row.x + ltm->pos.y * row.y + ltm->pos.z * row.z));

    scale = -0.5f * camera->recipViewWindow.y;
    shift = scale * camera->viewOffset.y;
    scaledAxis.x = ltm->up.x * scale;
    scaledAxis.y = ltm->up.y * scale;
    scaledAxis.z = ltm->up.z * scale;
    row.x = ltm->at.x * shift + scaledAxis.x;
    row.y = ltm->at.y * shift + scaledAxis.y;
    row.z = ltm->at.z * shift + scaledAxis.z;
    viewMatrix->right.y = row.x;
    viewMatrix->up.y = row.y;
    viewMatrix->at.y = row.z;
    viewMatrix->pos.y =
        0.5f - (shift + (ltm->pos.x * row.x + ltm->pos.y * row.y + ltm->pos.z * row.z));

    viewMatrix->right.z = ltm->at.x;
    viewMatrix->up.z = ltm->at.y;
    viewMatrix->at.z = ltm->at.z;
    viewMatrix->pos.z = -(ltm->pos.x * ltm->at.x + ltm->pos.y * ltm->at.y + ltm->pos.z * ltm->at.z);

    fn_8047E990(viewMatrix, NULL);
    return camera;
}

/* Updates a camera when its frame moves: its view matrix, frustum and the frustum's bounding box */
void *fn_8048528C(void *object) {
    RwCamera *camera = (RwCamera *)object;

    if (camera->projectionType == rwPERSPECTIVE) {
        fn_80484AAC(camera);
        fn_804842C8(camera);
    } else {
        fn_80485124(camera);
        fn_80484C10(camera);
    }
    fn_8048349C(&camera->frustumBoundBox, camera->frustumCorners, 8);
    return object;
}

/* The default end of a camera update: the device's, after which no camera is current */
RwCamera *fn_804852F0(RwCamera *camera) {
    if (lbl_805F15B4->stdFunc[rwSTANDARDCAMERAENDUPDATE](NULL, camera, 0)) {
        lbl_805F15B4->curCamera = NULL;
        return camera;
    }
    return NULL;
}

/* The default start of a camera update: the camera is current, dirty frames are synchronized,
 * then the device's */
RwCamera *fn_80485354(RwCamera *camera) {
    lbl_805F15B4->curCamera = camera;
    fn_8048B354();
    if (lbl_805F15B4->stdFunc[rwSTANDARDCAMERABEGINUPDATE](NULL, camera, 0)) {
        fn_8049B148(camera);
        return camera;
    }
    return NULL;
}

void *fn_804853C0(void *instance, RwInt32 offset, RwInt32 size) {
    if (RWCAMERAGLOBAL(cameraFreeList) != NULL) {
        fn_8047FCDC(RWCAMERAGLOBAL(cameraFreeList));
        RWCAMERAGLOBAL(cameraFreeList) = NULL;
    }
    lbl_805F15A8.numInstances--;
    return instance;
}

/* Opens the camera module: its free list of cameras with their plugins' data */
void *fn_80485424(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F15A8.globalsOffset = offset;
    RWCAMERAGLOBAL(cameraFreeList) =
        fn_8047FC04(lbl_8056DF70.sizeOfStruct, lbl_805EDED8, 4, lbl_805EDEDC, &lbl_805E2B20,
                    rwMEMHINTDUR_GLOBAL | rwID_CAMERA);
    if (RWCAMERAGLOBAL(cameraFreeList) == NULL) {
        return NULL;
    }
    lbl_805F15A8.numInstances++;
    return instance;
}

RwCamera *fn_804854B4(RwCamera *camera) {
    return camera->endUpdate(camera);
}

RwCamera *fn_804854DC(RwCamera *camera) {
    return camera->beginUpdate(camera);
}

RwCamera *fn_80485504(RwCamera *camera, const RwV2d *offset) {
    camera->viewOffset = *offset;
    if (CAMERAFRAME(camera) != NULL) {
        fn_8048722C(CAMERAFRAME(camera));
    }
    return camera;
}

RwCamera *fn_80485550(RwCamera *camera, RwReal nearPlane) {
    camera->nearPlane = nearPlane;
    CameraUpdateZShiftScale(camera);
    if (CAMERAFRAME(camera) != NULL) {
        fn_8048722C(CAMERAFRAME(camera));
    }
    return camera;
}

RwCamera *fn_8048560C(RwCamera *camera, RwReal farPlane) {
    camera->farPlane = farPlane;
    CameraUpdateZShiftScale(camera);
    if (CAMERAFRAME(camera) != NULL) {
        fn_8048722C(CAMERAFRAME(camera));
    }
    return camera;
}

/* Whether a sphere is outside (0), crossing (1) or inside (2) a camera's frustum */
RwInt32 fn_804856C8(const RwCamera *camera, const RwSphere *sphere) {
    const RwFrustumPlane *frustumPlane = camera->frustumPlanes;
    RwInt32 result = 2;
    RwInt32 numPlanes = 6;

    while (numPlanes--) {
        RwReal distance;

        distance = sphere->center.x * frustumPlane->plane.normal.x +
                   sphere->center.y * frustumPlane->plane.normal.y +
                   sphere->center.z * frustumPlane->plane.normal.z;
        distance -= frustumPlane->plane.distance;

        if (distance > sphere->radius) {
            return 0;
        }
        if (distance > -sphere->radius) {
            result = 1;
        }
        frustumPlane++;
    }
    return result;
}

/* Clears a camera's rasters with the device */
RwCamera *fn_804857BC(RwCamera *camera, RwRGBA *color, RwInt32 clearMode) {
    if (lbl_805F15B4->stdFunc[rwSTANDARDCAMERACLEAR](camera, color, clearMode)) {
        return camera;
    }
    return NULL;
}

/* Shows a camera's frame buffer */
RwCamera *fn_80485808(RwCamera *camera, void *device, RwUInt32 flags) {
    if (fn_80489D44(camera->frameBuffer, device, flags)) {
        return camera;
    }
    return NULL;
}

RwCamera *fn_8048584C(RwCamera *camera, RwInt32 projection) {
    switch (projection) {
    case rwPERSPECTIVE:
    case rwPARALLEL:
        camera->projectionType = projection;
        if (CAMERAFRAME(camera) != NULL) {
            fn_8048722C(CAMERAFRAME(camera));
        }
        CameraUpdateZShiftScale(camera);
        return camera;
    default:
        RWERROR((E_RW_INVCAMERAPROJ, "Invalid projection type specified"));
        return NULL;
    }
}

RwCamera *fn_80485950(RwCamera *camera, const RwV2d *viewWindow) {
    camera->viewWindow = *viewWindow;
    camera->recipViewWindow.x = 1.0f / camera->viewWindow.x;
    camera->recipViewWindow.y = 1.0f / camera->viewWindow.y;
    if (CAMERAFRAME(camera) != NULL) {
        fn_8048722C(CAMERAFRAME(camera));
    }
    return camera;
}

RwInt32 fn_804859B8(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB,
                    RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB) {
    return fn_80481DC8(&lbl_8056DF70, size, pluginID, constructCB, destructCB, copyCB);
}

RwBool fn_804859F8(RwCamera *camera) {
    fn_80482144(&lbl_8056DF70, camera);
    fn_8048D834(camera);
    RwFreeListFree(RWCAMERAGLOBAL(cameraFreeList), camera);
    return TRUE;
}

/* Creates a perspective camera with a unit view window, no offset or rasters, and default planes */
RwCamera *fn_80485A58(void) {
    RwCamera *camera =
        RwFreeListAlloc(RWCAMERAGLOBAL(cameraFreeList), rwMEMHINTDUR_EVENT | rwID_CAMERA);

    if (camera == NULL) {
        return NULL;
    }
    camera->object.type = rwCAMERA;
    camera->object.subType = 0;
    camera->object.flags = 0;
    camera->object.privateFlags = 0;
    camera->object.parent = NULL;
    camera->sync = fn_8048528C;
    camera->beginUpdate = fn_80485354;
    camera->endUpdate = fn_804852F0;
    camera->viewWindow.y = 1.0f;
    camera->viewWindow.x = 1.0f;
    camera->recipViewWindow.y = 1.0f;
    camera->recipViewWindow.x = 1.0f;
    camera->viewOffset.y = 0.0f;
    camera->viewOffset.x = 0.0f;
    camera->nearPlane = 0.05f;
    camera->farPlane = 10.0f;
    camera->fogPlane = 5.0f;
    camera->frameBuffer = NULL;
    camera->zBuffer = NULL;
    camera->projectionType = rwPERSPECTIVE;
    CameraUpdateZShiftScale(camera);
    camera->viewMatrix.flags = 0;
    fn_8048209C(&lbl_8056DF70, camera);
    return camera;
}
