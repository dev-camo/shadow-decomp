/* RenderWare particle tank plugin (plugin ID 0x12F), GameCube: the engine plugin data that holds
 * the tank pipeline, the choice of particle kernel, and the default create, instance and render
 * callbacks. The original file name is unknown. */
#include "rw/rpptank.h"
#include <dolphin/gx.h>
#include <math.h>
#include <string.h>

RxPipeline *lbl_805F1450; /* the tank pipeline */

/* Engine plugin constructor: creates the tank pipeline */
void *fn_804485BC(void *object, RwInt32 offset, RwInt32 size) {
    lbl_805F1450 = fn_8046335C(rpID_PTANKPLUGIN, 0, NULL, NULL, fn_80448CB0, fn_80448D04);
    return object;
}

/* Engine plugin destructor: destroys the tank pipeline */
void *fn_80448610(void *object, RwInt32 offset, RwInt32 size) {
    if (lbl_805F1450 != NULL) {
        fn_8049B800(lbl_805F1450);
        lbl_805F1450 = NULL;
    }
    return object;
}

/* Chooses the particle kernel and the platform flags for the tank's data flags */
void fn_80448658(RpAtomic *atomic) {
    RpPTankAtomicExtPrv *ptank = PTANKEXT(atomic);
    RwUInt32 platFlags = 0;

    ptank->field_0x24 = 0;
    ptank->kernel = NULL;
    ptank->field_0x2C = 0;
    ptank->field_0x30 = 0;
    ptank->field_0x34 = 0;
    ptank->field_0x38 = 0;

    if (ptank->format.dataFlags & rpPTANKDFLAGMATRIX) {
        if (ptank->format.dataFlags & rpPTANKDFLAGUSECENTER) {
            ptank->kernel = fn_804495D0;
            platFlags |= 0x2;
        } else {
            ptank->kernel = fn_8044920C;
            platFlags |= 0x1;
        }
    } else if (ptank->format.dataFlags & rpPTANKDFLAGCNSMATRIX) {
        if (ptank->format.dataFlags & rpPTANKDFLAGUSECENTER) {
            ptank->kernel = fn_80449D5C;
            platFlags |= 0x8;
        } else {
            ptank->kernel = fn_80449994;
            platFlags |= 0x4;
        }
    } else if (ptank->format.dataFlags & rpPTANKDFLAG2DROTATE) {
        if (ptank->format.dataFlags & rpPTANKDFLAGSIZE) {
            if (ptank->format.dataFlags & rpPTANKDFLAGUSECENTER) {
                ptank->kernel = fn_8044B654;
                platFlags |= 0x200;
            } else {
                ptank->kernel = fn_8044B210;
                platFlags |= 0x100;
            }
        } else {
            if (ptank->format.dataFlags & rpPTANKDFLAGUSECENTER) {
                ptank->kernel = fn_8044ADA8;
                platFlags |= 0x80;
            } else {
                ptank->kernel = fn_8044A93C;
                platFlags |= 0x40;
            }
        }
    } else if (ptank->format.dataFlags & rpPTANKDFLAGSIZE) {
        if (ptank->format.dataFlags & rpPTANKDFLAGUSECENTER) {
            ptank->kernel = fn_8044A540;
            platFlags |= 0x20;
        } else {
            ptank->kernel = fn_8044A168;
            platFlags |= 0x10;
        }
    } else {
        if (ptank->format.dataFlags & rpPTANKDFLAGUSECENTER) {
            ptank->kernel = fn_80449D5C;
            platFlags |= 0x8;
        } else {
            ptank->kernel = fn_80449994;
            platFlags |= 0x4;
        }
    }

    if (ptank->format.dataFlags & rpPTANKDFLAGVTX2TEXCOORDS) {
        platFlags |= 0x100000;
    } else if (ptank->format.dataFlags & rpPTANKDFLAGVTX4TEXCOORDS) {
        platFlags |= 0x200000;
    } else if (ptank->format.dataFlags & rpPTANKDFLAGCNSVTX2TEXCOORDS) {
        platFlags |= 0x400000;
    } else if (ptank->format.dataFlags & rpPTANKDFLAGCNSVTX4TEXCOORDS) {
        platFlags |= 0x800000;
    }

    if (ptank->format.dataFlags & rpPTANKDFLAGCOLOR) {
        platFlags |= 0x10000;
    } else if (ptank->format.dataFlags & rpPTANKDFLAGVTXCOLOR) {
        platFlags |= 0x20000;
    } else if (ptank->format.dataFlags & rpPTANKDFLAGCNSVTXCOLOR) {
        platFlags |= 0x40000;
    } else {
        platFlags |= PTANKPLATFLAGCNSCOLOR;
    }

    if (ptank->format.dataFlags & rpPTANKDFLAGNORMAL) {
        platFlags |= 0x1000000;
    }
    ptank->platFlags = platFlags;
}

/* The default create callback: a one-vertex geometry with one material and one empty mesh, in the
 * tank pipeline */
RwBool fn_80448850(RpAtomic *atomic, RpPTankData *ptankGlobal, RwInt32 maxPCount,
                   RwUInt32 dataFlags, RwUInt32 platFlags) {
    RpGeometry *geometry;
    RwUInt32 geometryFlags;
    RpGameCubeVertexFormat *format;
    RpMaterial *material;
    RwSurfaceProperties surfaceProps;
    RpTriangle triangle;
    RpMeshHeader *meshHeader;
    RpMesh *mesh;

    format = fn_8046301C();
    geometryFlags = rpGEOMETRYPOSITIONS;
    fn_80462F7C(format, GX_F32, 0);
    if ((dataFlags & rpPTANKDFLAGNORMAL) == rpPTANKDFLAGNORMAL) {
        geometryFlags |= rpGEOMETRYNORMALS | rpGEOMETRYLIGHT;
        fn_80462F88(format, GX_F32, 0);
    }
    if (dataFlags & (rpPTANKDFLAGCOLOR | rpPTANKDFLAGVTXCOLOR | rpPTANKDFLAGCNSVTXCOLOR)) {
        geometryFlags |= rpGEOMETRYPRELIT;
        fn_80462FA4(format, GX_RGBA8);
    }
    if (dataFlags & (rpPTANKDFLAGVTX2TEXCOORDS | rpPTANKDFLAGVTX4TEXCOORDS |
                     rpPTANKDFLAGCNSVTX2TEXCOORDS | rpPTANKDFLAGCNSVTX4TEXCOORDS)) {
        geometryFlags |= rpGEOMETRYTEXTURED;
        fn_80462F94(format, 1, GX_F32, 0);
    }

    geometry = fn_80458D24(1.0f);
    geometry->flags = geometryFlags;
    geometry->numVertices = 1;
    fn_80463104(geometry, format);
    fn_804630C0(format);

    material = fn_8045B13C();
    surfaceProps.ambient = 0.3f;
    surfaceProps.specular = 1.0f;
    surfaceProps.diffuse = 1.0f;
    material->surfaceProps = surfaceProps;
    /* (sic) triangle is an uninitialised scratch value: the call is made for its side effect of
     * adding the material to the geometry's material list, and the material index it writes into
     * the triangle is not used */
    fn_8045905C(geometry, &triangle, material);

    meshHeader = fn_8045BF30(sizeof(RpMeshHeader) + sizeof(RpMesh));
    memset(meshHeader, 0, sizeof(RpMeshHeader) + sizeof(RpMesh));
    meshHeader->flags = (meshHeader->flags & ~rpMESHHEADERPRIMMASK) |
                        RPMESHGLOBAL(primTypeToMeshFlags)[rwPRIMTYPEPOINTLIST];
    mesh = (RpMesh *)(meshHeader + 1);
    /* the mesh has no indices: they would start where it does */
    mesh->indices = (RxVertexIndex *)mesh;
    mesh->material = material;
    meshHeader->numMeshes = 1;
    meshHeader->serialNum = fn_8045CA20();
    meshHeader->firstMeshOffset = 0;
    meshHeader->totalIndicesInMesh = 0;
    geometry->mesh = meshHeader;

    atomic = fn_80456F0C(atomic, geometry, 0);
    fn_8045985C(geometry);
    fn_8045B1F4(material);
    fn_80448658(atomic);
    atomic->pipeline = lbl_805F1450;
    return TRUE;
}

/* The default instance callback: the kernels read the streams while rendering */
RwBool fn_80448A1C(RpAtomic *atomic, RpPTankData *ptankGlobal, RwInt32 actPCount,
                   RwUInt32 instFlags) {
    return TRUE;
}

/* The default render callback: renders the tank facing the current camera with the tank's blend
 * states */
RwBool fn_80448A24(RpAtomic *atomic, RpPTankData *ptankGlobal, RwInt32 actPCount) {
    RpPTankAtomicExtPrv *ptank = PTANKEXT(atomic);
    RwInt32 dataFlags = ptank->format.dataFlags;
    RwV3d axes[2];
    RwMatrix *ltm;
    RwReal sinRotate;
    RwReal cosRotate;
    RwReal negSin;
    RwInt32 srcBlend;
    RwInt32 dstBlend;

    if ((dataFlags & rpPTANKDFLAGCNSMATRIX) == rpPTANKDFLAGCNSMATRIX) {
        axes[0] = ptank->cMatrix.right;
        axes[1] = ptank->cMatrix.up;
    } else {
        ltm = RwFrameGetLTM(lbl_805F15B4->curCamera->object.parent);
        if ((dataFlags & rpPTANKDFLAGCNS2DROTATE) == rpPTANKDFLAGCNS2DROTATE) {
            sinRotate = sin(ptank->cRotate);
            cosRotate = cos(ptank->cRotate);
            negSin = -sinRotate;
            axes[0].x = cosRotate * ltm->right.x + negSin * ltm->up.x;
            axes[0].y = cosRotate * ltm->right.y + negSin * ltm->up.y;
            axes[0].z = cosRotate * ltm->right.z + negSin * ltm->up.z;
            axes[1].x = sinRotate * ltm->right.x + cosRotate * ltm->up.x;
            axes[1].y = sinRotate * ltm->right.y + cosRotate * ltm->up.y;
            axes[1].z = sinRotate * ltm->right.z + cosRotate * ltm->up.z;
        } else {
            axes[0] = ltm->right;
            axes[1] = ltm->up;
        }
    }
    axes[1].x = -1.0f * axes[1].x;
    axes[1].y = -1.0f * axes[1].y;
    axes[1].z = -1.0f * axes[1].z;
    ptank->publicData.cameraAxes = axes;

    if (ptank->vertexAlphaBlend) {
        RwRenderStateGet(rwRENDERSTATESRCBLEND, &srcBlend);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)ptank->srcBlend);
        RwRenderStateGet(rwRENDERSTATEDESTBLEND, &dstBlend);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)ptank->dstBlend);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
    } else {
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
    }

    ptank->defaultRenderCB(atomic);

    if (ptank->vertexAlphaBlend) {
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)srcBlend);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)dstBlend);
    }
    return FALSE;
}
