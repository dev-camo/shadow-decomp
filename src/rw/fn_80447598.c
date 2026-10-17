/* RenderWare particle tank plugin (plugin ID 0x12F): the atomic plugin data, tank creation with
 * its stream layouts, the tank's render callback, and stream locking. The original file name is
 * unknown. */
#include "rw/rpptank.h"
#include <string.h>

#define rwMEMHINT_PTANK (rwMEMHINTDUR_EVENT | rpID_PTANKPLUGIN)

RwInt32 lbl_805F144C; /* engine plugin offset */
RwInt32 lbl_805F1448; /* atomic plugin offset */

/* The size of each stream element, in the order of the streams in RpPTankData */
const RwInt32 lbl_8051BCA8[9] = {
    sizeof(RwV3d),  sizeof(RwMatrix),        sizeof(RwV3d),
    sizeof(RwV2d),  sizeof(RwRGBA),          4 * sizeof(RwRGBA),
    sizeof(RwReal), 2 * sizeof(RwTexCoords), 4 * sizeof(RwTexCoords),
};

/* Atomic plugin constructor */
void *fn_80447598(void *object, RwInt32 offset, RwInt32 size) {
    PTANKEXT(object) = NULL;
    return object;
}

/* Atomic plugin destructor */
void *fn_804475A8(void *object, RwInt32 offset, RwInt32 size) {
    RpAtomic *atomic = object;

    if (PTANKEXT(atomic) != NULL) {
        fn_8045B270(atomic->geometry->matList.materials[0], NULL);
        if (PTANKEXT(atomic)->rawdata != NULL) {
            RwFree(PTANKEXT(atomic)->rawdata);
        }
        RwFree(PTANKEXT(atomic));
        PTANKEXT(atomic) = NULL;
    }
    return atomic;
}

/* Attaches the plugin */
RwBool fn_80447640(void) {
    lbl_805F144C = fn_80485D78(sizeof(void *), rpID_PTANKPLUGIN, fn_804485BC, fn_80448610);
    if (lbl_805F144C >= 0) {
        lbl_805F1448 = fn_80458B6C(sizeof(RpPTankAtomicExtPrv *), rpID_PTANKPLUGIN, fn_80447598,
                                   fn_804475A8, NULL);
        return lbl_805F1448 >= 0;
    }
    return FALSE;
}

/* Allocates the streams of a tank as one array each */
RwBool fn_804476B8(RpPTankAtomicExtPrv *ptank, RwUInt32 dataFlags, RwUInt32 platFlags) {
    RwInt32 hasPosition = (dataFlags & rpPTANKDFLAGPOSITION) != 0;
    RwBool result = FALSE;
    RwInt32 hasColor = (dataFlags & rpPTANKDFLAGCOLOR) != 0;
    RwInt32 hasSize = (dataFlags & rpPTANKDFLAGSIZE) != 0;
    RwInt32 hasMatrix = (dataFlags & rpPTANKDFLAGMATRIX) != 0;
    RwInt32 hasNormal = (dataFlags & rpPTANKDFLAGNORMAL) != 0;
    RwInt32 hasRotate = (dataFlags & rpPTANKDFLAG2DROTATE) != 0;
    RwInt32 hasVtxColor = (dataFlags & rpPTANKDFLAGVTXCOLOR) != 0;
    RwInt32 hasVtx2TexCoords = (dataFlags & rpPTANKDFLAGVTX2TEXCOORDS) != 0;
    RwInt32 hasVtx4TexCoords = (dataFlags & rpPTANKDFLAGVTX4TEXCOORDS) != 0;
    RwInt32 size = 0;
    RwUInt8 *mem;
    RwInt32 offset;
    RwUInt8 *data;

    ptank->format.dataFlags &= ~0x1FF;
    ptank->format.numClusters = 0;
    ptank->lockFlags = 0;

    if (hasPosition == TRUE) {
        ptank->format.numClusters++;
        ptank->format.dataFlags |= rpPTANKDFLAGPOSITION;
        size += lbl_8051BCA8[0] * ptank->maxPCount;
    }
    if (hasMatrix == TRUE) {
        ptank->format.numClusters++;
        ptank->format.dataFlags |= rpPTANKDFLAGMATRIX;
        size += lbl_8051BCA8[1] * ptank->maxPCount;
    }
    if (hasNormal == TRUE) {
        ptank->format.numClusters++;
        ptank->format.dataFlags |= rpPTANKDFLAGNORMAL;
        size += lbl_8051BCA8[2] * ptank->maxPCount;
    }
    if (hasSize == TRUE) {
        ptank->format.numClusters++;
        ptank->format.dataFlags |= rpPTANKDFLAGSIZE;
        size += lbl_8051BCA8[3] * ptank->maxPCount;
    }
    if (hasColor == TRUE) {
        ptank->format.numClusters++;
        ptank->format.dataFlags |= rpPTANKDFLAGCOLOR;
        size += lbl_8051BCA8[4] * ptank->maxPCount;
    }
    if (hasVtxColor == TRUE) {
        ptank->format.numClusters++;
        ptank->format.dataFlags |= rpPTANKDFLAGVTXCOLOR;
        size += lbl_8051BCA8[5] * ptank->maxPCount;
    }
    if (hasRotate == TRUE) {
        ptank->format.numClusters++;
        ptank->format.dataFlags |= rpPTANKDFLAG2DROTATE;
        size += lbl_8051BCA8[6] * ptank->maxPCount;
    }
    if (hasVtx2TexCoords == TRUE) {
        ptank->format.numClusters++;
        ptank->format.dataFlags |= rpPTANKDFLAGVTX2TEXCOORDS;
        size += lbl_8051BCA8[7] * ptank->maxPCount;
    }
    if (hasVtx4TexCoords == TRUE) {
        ptank->format.numClusters++;
        ptank->format.dataFlags |= rpPTANKDFLAGVTX4TEXCOORDS;
        size += lbl_8051BCA8[8] * ptank->maxPCount;
    }

    /* 16 bytes of slack to align the streams */
    mem = RwMalloc(size + 16, rwMEMHINT_PTANK);
    if (mem != NULL) {
        data = (RwUInt8 *)(((RwUInt32)mem + 15) & ~15);
        offset = 0;

        if (hasPosition == TRUE) {
            ptank->publicData.dataPosition.data = data;
            ptank->publicData.dataPosition.stride = sizeof(RwV3d);
            offset += lbl_8051BCA8[0] * ptank->maxPCount;
        }
        if (hasMatrix == TRUE) {
            ptank->publicData.dataMatrix.data = data + offset;
            ptank->publicData.dataMatrix.stride = sizeof(RwMatrix);
            offset += lbl_8051BCA8[1] * ptank->maxPCount;
        }
        if (hasNormal == TRUE) {
            ptank->publicData.dataNormals.data = data + offset;
            ptank->publicData.dataNormals.stride = sizeof(RwV3d);
            offset += lbl_8051BCA8[2] * ptank->maxPCount;
        }
        if (hasSize == TRUE) {
            ptank->publicData.dataSize.data = data + offset;
            ptank->publicData.dataSize.stride = sizeof(RwV2d);
            offset += lbl_8051BCA8[3] * ptank->maxPCount;
        }
        if (hasColor == TRUE) {
            ptank->publicData.dataColor.data = data + offset;
            ptank->publicData.dataColor.stride = sizeof(RwRGBA);
            offset += lbl_8051BCA8[4] * ptank->maxPCount;
        }
        if (hasVtxColor == TRUE) {
            ptank->publicData.dataVtxColor.data = data + offset;
            ptank->publicData.dataVtxColor.stride = 4 * sizeof(RwRGBA);
            offset += lbl_8051BCA8[5] * ptank->maxPCount;
        }
        if (hasRotate == TRUE) {
            ptank->publicData.dataRotate.data = data + offset;
            ptank->publicData.dataRotate.stride = sizeof(RwReal);
            offset += lbl_8051BCA8[6] * ptank->maxPCount;
        }
        if (hasVtx2TexCoords == TRUE) {
            ptank->publicData.dataVtx2TexCoords.data = data + offset;
            ptank->publicData.dataVtx2TexCoords.stride = 2 * sizeof(RwTexCoords);
            offset += lbl_8051BCA8[7] * ptank->maxPCount;
        }
        if (hasVtx4TexCoords == TRUE) {
            ptank->publicData.dataVtx4TexCoords.data = data + offset;
            ptank->publicData.dataVtx4TexCoords.stride = 4 * sizeof(RwTexCoords);
            offset += lbl_8051BCA8[8] * ptank->maxPCount;
        }

        ptank->format.stride = 0;
        result = TRUE;
        ptank->rawdata = mem;
    }
    return result;
}

/* Allocates the streams of a tank interleaved in one structure per particle */
RwBool fn_80447AF8(RpPTankAtomicExtPrv *ptank, RwUInt32 dataFlags, RwUInt32 platFlags) {
    RwInt32 hasPosition = (dataFlags & rpPTANKDFLAGPOSITION) != 0;
    RwBool result = FALSE;
    RwInt32 hasColor = (dataFlags & rpPTANKDFLAGCOLOR) != 0;
    RwInt32 hasSize = (dataFlags & rpPTANKDFLAGSIZE) != 0;
    RwInt32 hasMatrix = (dataFlags & rpPTANKDFLAGMATRIX) != 0;
    RwInt32 hasNormal = (dataFlags & rpPTANKDFLAGNORMAL) != 0;
    RwInt32 hasRotate = (dataFlags & rpPTANKDFLAG2DROTATE) != 0;
    RwInt32 hasVtxColor = (dataFlags & rpPTANKDFLAGVTXCOLOR) != 0;
    RwInt32 hasVtx2TexCoords = (dataFlags & rpPTANKDFLAGVTX2TEXCOORDS) != 0;
    RwInt32 hasVtx4TexCoords = (dataFlags & rpPTANKDFLAGVTX4TEXCOORDS) != 0;
    RwInt32 stride = 0;
    RwUInt8 *mem;
    RwInt32 offset;
    RwUInt8 *data;

    ptank->format.dataFlags &= ~0x1FF;
    ptank->format.numClusters = 0;

    if (hasPosition == TRUE) {
        ptank->format.numClusters++;
        stride += lbl_8051BCA8[0];
        ptank->format.dataFlags |= rpPTANKDFLAGPOSITION;
    }
    if (hasMatrix == TRUE) {
        ptank->format.numClusters++;
        stride += lbl_8051BCA8[1];
        ptank->format.dataFlags |= rpPTANKDFLAGMATRIX;
    }
    if (hasNormal == TRUE) {
        ptank->format.numClusters++;
        stride += lbl_8051BCA8[2];
        ptank->format.dataFlags |= rpPTANKDFLAGNORMAL;
    }
    if (hasSize == TRUE) {
        ptank->format.numClusters++;
        stride += lbl_8051BCA8[3];
        ptank->format.dataFlags |= rpPTANKDFLAGSIZE;
    }
    if (hasColor == TRUE) {
        ptank->format.numClusters++;
        stride += lbl_8051BCA8[4];
        ptank->format.dataFlags |= rpPTANKDFLAGCOLOR;
    }
    if (hasVtxColor == TRUE) {
        ptank->format.numClusters++;
        stride += lbl_8051BCA8[5];
        ptank->format.dataFlags |= rpPTANKDFLAGVTXCOLOR;
    }
    if (hasRotate == TRUE) {
        ptank->format.numClusters++;
        stride += lbl_8051BCA8[6];
        ptank->format.dataFlags |= rpPTANKDFLAG2DROTATE;
    }
    if (hasVtx2TexCoords == TRUE) {
        ptank->format.numClusters++;
        stride += lbl_8051BCA8[7];
        ptank->format.dataFlags |= rpPTANKDFLAGVTX2TEXCOORDS;
    }
    if (hasVtx4TexCoords == TRUE) {
        ptank->format.numClusters++;
        stride += lbl_8051BCA8[8];
        ptank->format.dataFlags |= rpPTANKDFLAGVTX4TEXCOORDS;
    }

    /* matrices are read as words */
    if (hasMatrix) {
        stride = (stride + 3) & ~3;
    }
    ptank->format.stride = stride;

    /* 16 bytes of slack to align the structures */
    mem = RwMalloc(stride * ptank->maxPCount + 16, rwMEMHINTDUR_EVENT);
    if (mem != NULL) {
        data = (RwUInt8 *)(((RwUInt32)mem + 15) & ~15);
        offset = 0;

        if (hasPosition == TRUE) {
            ptank->publicData.dataPosition.data = data;
            ptank->publicData.dataPosition.stride = ptank->format.stride;
            offset = lbl_8051BCA8[0];
        }
        if (hasMatrix == TRUE) {
            ptank->publicData.dataMatrix.data = data + offset;
            ptank->publicData.dataMatrix.stride = ptank->format.stride;
            offset += lbl_8051BCA8[1];
        }
        if (hasNormal == TRUE) {
            ptank->publicData.dataNormals.data = data + offset;
            ptank->publicData.dataNormals.stride = ptank->format.stride;
            offset += lbl_8051BCA8[2];
        }
        if (hasSize == TRUE) {
            ptank->publicData.dataSize.data = data + offset;
            ptank->publicData.dataSize.stride = ptank->format.stride;
            offset += lbl_8051BCA8[3];
        }
        if (hasColor == TRUE) {
            ptank->publicData.dataColor.data = data + offset;
            ptank->publicData.dataColor.stride = ptank->format.stride;
            offset += lbl_8051BCA8[4];
        }
        if (hasVtxColor == TRUE) {
            ptank->publicData.dataVtxColor.data = data + offset;
            ptank->publicData.dataVtxColor.stride = ptank->format.stride;
            offset += lbl_8051BCA8[5];
        }
        if (hasRotate == TRUE) {
            ptank->publicData.dataRotate.data = data + offset;
            ptank->publicData.dataRotate.stride = ptank->format.stride;
            offset += lbl_8051BCA8[6];
        }
        if (hasVtx2TexCoords == TRUE) {
            ptank->publicData.dataVtx2TexCoords.data = data + offset;
            ptank->publicData.dataVtx2TexCoords.stride = ptank->format.stride;
            offset += lbl_8051BCA8[7];
        }
        if (hasVtx4TexCoords == TRUE) {
            ptank->publicData.dataVtx4TexCoords.data = data + offset;
            ptank->publicData.dataVtx4TexCoords.stride = ptank->format.stride;
            offset += lbl_8051BCA8[8];
        }

        ptank->rawdata = mem;
        result = TRUE;
    }
    return result;
}

/* Resolves conflicting data flags: a matrix replaces the position, size and rotation, and the layout
 * defaults to interleaved. A stream with its constant also set leaves only that constant flag: the
 * mask keeps the flag rather than clearing it. */
RwUInt32 fn_80447EB4(RwUInt32 dataFlags) {
    RwInt32 hasPosition = (dataFlags & rpPTANKDFLAGPOSITION) != 0;
    RwInt32 hasSize = (dataFlags & rpPTANKDFLAGSIZE) != 0;
    RwInt32 hasMatrix = (dataFlags & rpPTANKDFLAGMATRIX) != 0;
    RwInt32 hasRotate = (dataFlags & rpPTANKDFLAG2DROTATE) != 0;
    RwInt32 hasVtxColor = (dataFlags & rpPTANKDFLAGVTXCOLOR) != 0;
    RwInt32 hasVtx2TexCoords = (dataFlags & rpPTANKDFLAGVTX2TEXCOORDS) != 0;
    RwInt32 hasVtx4TexCoords = (dataFlags & rpPTANKDFLAGVTX4TEXCOORDS) != 0;
    RwInt32 hasCnsMatrix = (dataFlags & rpPTANKDFLAGCNSMATRIX) != 0;
    RwInt32 hasCnsRotate = (dataFlags & rpPTANKDFLAGCNS2DROTATE) != 0;
    RwInt32 hasCnsVtxColor = (dataFlags & rpPTANKDFLAGCNSVTXCOLOR) != 0;
    RwInt32 hasCnsVtx2TexCoords = (dataFlags & rpPTANKDFLAGCNSVTX2TEXCOORDS) != 0;
    RwInt32 hasCnsVtx4TexCoords = (dataFlags & rpPTANKDFLAGCNSVTX4TEXCOORDS) != 0;

    if (hasMatrix == TRUE && hasCnsMatrix == TRUE) {
        dataFlags &= rpPTANKDFLAGCNSMATRIX;
    }
    if (hasRotate == TRUE && hasCnsRotate == TRUE) {
        dataFlags &= rpPTANKDFLAGCNS2DROTATE;
    }
    if (hasVtxColor == TRUE && hasCnsVtxColor == TRUE) {
        dataFlags &= rpPTANKDFLAGCNSVTXCOLOR;
    }
    if (hasVtx2TexCoords == TRUE && hasCnsVtx2TexCoords == TRUE) {
        dataFlags &= rpPTANKDFLAGCNSVTX2TEXCOORDS;
    }
    if (hasVtx4TexCoords == TRUE && hasCnsVtx4TexCoords == TRUE) {
        dataFlags &= rpPTANKDFLAGCNSVTX4TEXCOORDS;
    }
    if (hasMatrix == TRUE && hasPosition == TRUE) {
        dataFlags &= ~rpPTANKDFLAGPOSITION;
    }
    if (hasMatrix == TRUE && hasSize == TRUE) {
        dataFlags &= ~rpPTANKDFLAGSIZE;
    }
    if (hasMatrix == TRUE && hasRotate == TRUE) {
        dataFlags &= ~rpPTANKDFLAG2DROTATE;
    }
    if (hasMatrix == FALSE && hasPosition == FALSE) {
        dataFlags |= rpPTANKDFLAGPOSITION;
    }
    if ((dataFlags & (rpPTANKDFLAGARRAY | rpPTANKDFLAGSTRUCTURE)) == 0) {
        dataFlags |= rpPTANKDFLAGSTRUCTURE;
    }
    return dataFlags;
}

/* The render callback of a tank atomic: instances and renders its active particles */
RpAtomic *fn_80447FC0(RpAtomic *atomic) {
    RwBool render = TRUE;
    RpPTankAtomicExtPrv *ptank = PTANKEXT(atomic);

    if (ptank->actPCount > 0) {
        if (ptank->ptankCallBacks.instance != NULL) {
            ptank->ptankCallBacks.instance(atomic, &ptank->publicData, ptank->actPCount,
                                           ptank->instFlags);
            ptank->instFlags = 0;
        }
        if (ptank->ptankCallBacks.render != NULL) {
            render = ptank->ptankCallBacks.render(atomic, &ptank->publicData, ptank->actPCount);
        }
        if (render == TRUE) {
            ptank->defaultRenderCB(atomic);
        }
    }
    return atomic;
}

/* The default callbacks */
RpPTankCallBacks lbl_8056D6F8 = {NULL, fn_80448850, fn_80448A1C, fn_80448A24};

/* Creates a tank atomic with the default callbacks */
RpAtomic *fn_80448074(RwInt32 maxPCount, RwUInt32 dataFlags, RwUInt32 platFlags) {
    return fn_804480C8(maxPCount, fn_80447EB4(dataFlags), platFlags, &lbl_8056D6F8);
}

/* Creates a tank atomic with the given callbacks */
RpAtomic *fn_804480C8(RwInt32 maxPCount, RwUInt32 dataFlags, RwUInt32 platFlags,
                      RpPTankCallBacks *callbacks) {
    RwBool allocated = FALSE;
    RpAtomic *atomic;
    RpPTankAtomicExtPrv *ptank;

    atomic = fn_80456DD8();
    if (atomic != NULL) {
        ptank = RwMalloc(sizeof(RpPTankAtomicExtPrv), rwMEMHINT_PTANK);
        if (ptank != NULL) {
            ptank->defaultRenderCB = atomic->renderCallBack;
            RpAtomicSetRenderCallBack(atomic, fn_80447FC0);

            ptank->maxPCount = maxPCount;
            ptank->actPCount = 0;
            ptank->format.dataFlags = dataFlags;
            ptank->platFlags = platFlags;
            ptank->instFlags = 0xFFFFFFFF;
            ptank->lockFlags = 0;
            ptank->rawdata = NULL;
            PTANKEXT(atomic) = ptank;
            memcpy(&ptank->ptankCallBacks, callbacks, sizeof(RpPTankCallBacks));

            if (ptank->ptankCallBacks.alloc != NULL) {
                ptank->rawdata = ptank->ptankCallBacks.alloc(&ptank->publicData, ptank->maxPCount,
                                                             dataFlags, platFlags);
            } else if ((dataFlags & rpPTANKDFLAGSTRUCTURE) == rpPTANKDFLAGSTRUCTURE) {
                allocated = fn_80447AF8(ptank, dataFlags, platFlags);
            } else {
                allocated = fn_804476B8(ptank, dataFlags, platFlags);
            }

            if (allocated == TRUE || ptank->rawdata != NULL) {
                if (ptank->ptankCallBacks.create(atomic, &ptank->publicData, ptank->maxPCount,
                                                 dataFlags, platFlags) == TRUE) {
                    RwV2d center = {0.0f, 0.0f};
                    RwV2d size = {1.0f, 1.0f};
                    RwRGBA color = {0, 0, 0, 255};
                    RwTexCoords uv[4] = {{0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}};

                    fn_8045B270(atomic->geometry->matList.materials[0], NULL);

                    ptank->cSize = size;
                    ptank->cRotate = 0.0f;
                    ptank->cCenter = center;
                    RwMatrixSetIdentity(&ptank->cMatrix);
                    ptank->cColor = color;
                    ptank->cVtxColor[0] = color;
                    ptank->cVtxColor[1] = color;
                    ptank->cVtxColor[2] = color;
                    ptank->cVtxColor[3] = color;
                    ptank->cVtxUV[0] = uv[0];
                    ptank->cVtxUV[1] = uv[1];
                    ptank->cVtxUV[2] = uv[2];
                    ptank->cVtxUV[3] = uv[3];

                    RwRenderStateGet(rwRENDERSTATESRCBLEND, &ptank->srcBlend);
                    RwRenderStateGet(rwRENDERSTATEDESTBLEND, &ptank->dstBlend);
                    RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &ptank->vertexAlphaBlend);
                    return atomic;
                }
            }
        }
    }
    fn_80456FD8(atomic);
    return NULL;
}

/* Destroys a tank atomic */
RwBool fn_804483E8(RpAtomic *atomic) {
    return fn_80456FD8(atomic);
}

/* Locks one stream of a tank; locking for writing marks it for instancing */
RwBool fn_80448408(RpAtomic *atomic, RpPTankLockStruct *dst, RwUInt32 dataFlags,
                   RwUInt32 lockFlags) {
    RpPTankAtomicExtPrv *ptank = PTANKEXT(atomic);

    if ((ptank->format.dataFlags & dataFlags) == 0) {
        dst->data = NULL;
        dst->stride = 0;
        return FALSE;
    }

    switch (dataFlags) {
    case rpPTANKDFLAGPOSITION:
        *dst = ptank->publicData.dataPosition;
        break;
    case rpPTANKDFLAGCOLOR:
        *dst = ptank->publicData.dataColor;
        break;
    case rpPTANKDFLAGSIZE:
        *dst = ptank->publicData.dataSize;
        break;
    case rpPTANKDFLAGMATRIX:
        *dst = ptank->publicData.dataMatrix;
        break;
    case rpPTANKDFLAGNORMAL:
        *dst = ptank->publicData.dataNormals;
        break;
    case rpPTANKDFLAG2DROTATE:
        *dst = ptank->publicData.dataRotate;
        break;
    case rpPTANKDFLAGVTXCOLOR:
        *dst = ptank->publicData.dataVtxColor;
        break;
    case rpPTANKDFLAGVTX2TEXCOORDS:
        *dst = ptank->publicData.dataVtx2TexCoords;
        break;
    case rpPTANKDFLAGVTX4TEXCOORDS:
        *dst = ptank->publicData.dataVtx4TexCoords;
        break;
    }

    if (ptank->format.stride != 0) {
        dst->stride = ptank->format.stride;
    }

    if (dst->data != NULL && (lockFlags & rpPTANKLOCKWRITE) == rpPTANKLOCKWRITE) {
        PTANKEXT(atomic)->lockFlags |= lockFlags | dataFlags;
    }
    return TRUE;
}

/* Unlocks a tank: the streams written while locked are instanced before the next render */
RpAtomic *fn_80448598(RpAtomic *atomic) {
    RpPTankAtomicExtPrv *ptank = PTANKEXT(atomic);

    ptank->instFlags |= ptank->lockFlags;
    ptank->lockFlags = 0;
    return atomic;
}
