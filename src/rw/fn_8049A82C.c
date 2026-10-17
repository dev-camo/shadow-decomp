/* RenderWare immediate mode 3D: transforming a set of vertices with the transform pipeline, then
 * rendering them, in order or indexed, with the render pipeline of the primitive type. The
 * original file name is unknown. */
#include "rw/rwcore.h"
#include <string.h>

/* Reports an error of the core; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwID_COREPLUGIN;                                                         \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

/* The most vertices one transform can take */
#define rwIM3DMAXVERTICES 0x10000

RwModuleInfo lbl_805F1684;
rwImmediGlobals *lbl_805F1680; /* the immediate module's globals, set when it opens */

void *fn_8049A82C(RwIm3DVertex *vertices, RwUInt32 numVertices, RwMatrix *ltm, RwUInt32 flags) {
    if (numVertices > rwIM3DMAXVERTICES) {
        RWERROR((E_RX_TOOMANYVERTS));
    } else {
        RWIMMEDIGLOBAL(curPool.numElements) = numVertices;
        RWIMMEDIGLOBAL(curPool.elements) = vertices;
        RWIMMEDIGLOBAL(curPool.stride) = sizeof(RwIm3DVertex);
        RWIMMEDIGLOBAL(curPool.stash.ltm) = ltm;
        RWIMMEDIGLOBAL(curPool.stash.flags) = flags | rwIM3D_VERTEXXYZ | rwIM3D_VERTEXRGBA;
        if (fn_8049B674(RWIMMEDIGLOBAL(im3DTransformPipeline), &RWIMMEDIGLOBAL(curPool), TRUE) !=
            NULL) {
            return vertices;
        }
    }
    return NULL;
}

RwBool fn_8049A90C(void) {
    rwImmediGlobals *globals =
        RWPLUGINOFFSET(rwImmediGlobals, lbl_805F15B4, lbl_805F1684.globalsOffset);

    if (globals->curPool.elements == NULL) {
        return FALSE;
    }
    memset(&globals->curPool, 0, sizeof(rwIm3DPool));
    return TRUE;
}

RwBool fn_8049A95C(RwInt32 primType, RwImVertexIndex *indices, RwInt32 numIndices) {
    rwImmediGlobals *globals =
        RWPLUGINOFFSET(rwImmediGlobals, lbl_805F15B4, lbl_805F1684.globalsOffset);

    if (globals->curPool.elements != NULL) {
        rwIm3DPoolStash *stash = &globals->curPool.stash;

        stash->pipeline = NULL;
        stash->primType = primType;
        stash->indices = indices;
        stash->numIndices = numIndices;

        /* lists drop the indices of an incomplete last primitive */
        switch (primType) {
        case rwPRIMTYPETRILIST:
            stash->pipeline = RWIMMEDIGLOBAL(im3DRenderPipelines.triList);
            stash->numIndices = numIndices - numIndices % 3;
            break;
        case rwPRIMTYPETRIFAN:
            stash->pipeline = RWIMMEDIGLOBAL(im3DRenderPipelines.triFan);
            break;
        case rwPRIMTYPETRISTRIP:
            stash->pipeline = RWIMMEDIGLOBAL(im3DRenderPipelines.triStrip);
            break;
        case rwPRIMTYPELINELIST:
            stash->pipeline = RWIMMEDIGLOBAL(im3DRenderPipelines.lineList);
            stash->numIndices = numIndices - numIndices % 2;
            break;
        case rwPRIMTYPEPOLYLINE:
            stash->pipeline = RWIMMEDIGLOBAL(im3DRenderPipelines.polyLine);
            break;
        default:
            RWERROR((E_RX_INVALIDPRIMTYPE, primType));
            break;
        }

        if (fn_8049B674(stash->pipeline, stash, FALSE) != NULL) {
            return TRUE;
        }
    } else {
        RWERROR((E_RX_IM3DNOTACTIVE));
    }
    return FALSE;
}

RwBool fn_8049AAF8(RwInt32 primType) {
    RwBool active = RWIMMEDIGLOBAL(curPool.elements) != NULL;

    fn_8049B66C(); /* the original fetches the pipeline heap here and ignores it */
    if (active) {
        rwIm3DPoolStash *stash = &RWIMMEDIGLOBAL(curPool.stash);

        stash->pipeline = NULL;
        stash->primType = primType;
        stash->indices = NULL;
        stash->numIndices = RWIMMEDIGLOBAL(curPool.numElements);

        switch (primType) {
        case rwPRIMTYPETRILIST:
            stash->pipeline = RWIMMEDIGLOBAL(im3DRenderPipelines.triList);
            break;
        case rwPRIMTYPETRIFAN:
            stash->pipeline = RWIMMEDIGLOBAL(im3DRenderPipelines.triFan);
            break;
        case rwPRIMTYPETRISTRIP:
            stash->pipeline = RWIMMEDIGLOBAL(im3DRenderPipelines.triStrip);
            break;
        case rwPRIMTYPELINELIST:
            stash->pipeline = RWIMMEDIGLOBAL(im3DRenderPipelines.lineList);
            break;
        case rwPRIMTYPEPOLYLINE:
            stash->pipeline = RWIMMEDIGLOBAL(im3DRenderPipelines.polyLine);
            break;
        default:
            RWERROR((E_RX_INVALIDPRIMTYPE, primType));
            break;
        }

        if (fn_8049B674(stash->pipeline, stash, FALSE) != NULL) {
            return TRUE;
        }
    } else {
        RWERROR((E_RX_IM3DNOTACTIVE));
    }
    return FALSE;
}

RxPipeline *fn_8049AC90(RxPipeline *pipeline) {
    if (pipeline != NULL) {
        RWIMMEDIGLOBAL(im3DTransformPipeline) = pipeline;
    } else if (RWIMMEDIGLOBAL(platformIm3DTransformPipeline) != NULL) {
        RWIMMEDIGLOBAL(im3DTransformPipeline) = RWIMMEDIGLOBAL(platformIm3DTransformPipeline);
    } else {
        RWIMMEDIGLOBAL(im3DTransformPipeline) = NULL;
    }
    return RWIMMEDIGLOBAL(im3DTransformPipeline);
}

/* NULL restores the platform's pipeline. For the triangle strip, line list, polyline and point
 * list types the original reads the returned pipeline back from the platform's slot; the value is
 * the same. */
RxPipeline *fn_8049ACE0(RxPipeline *pipeline, RwInt32 primType) {
    if (pipeline != NULL) {
        switch (primType) {
        case rwPRIMTYPETRILIST:
            RWIMMEDIGLOBAL(im3DRenderPipelines.triList) = pipeline;
            return pipeline;
        case rwPRIMTYPETRIFAN:
            RWIMMEDIGLOBAL(im3DRenderPipelines.triFan) = pipeline;
            return pipeline;
        case rwPRIMTYPETRISTRIP:
            RWIMMEDIGLOBAL(im3DRenderPipelines.triStrip) = pipeline;
            return pipeline;
        case rwPRIMTYPELINELIST:
            RWIMMEDIGLOBAL(im3DRenderPipelines.lineList) = pipeline;
            return pipeline;
        case rwPRIMTYPEPOLYLINE:
            RWIMMEDIGLOBAL(im3DRenderPipelines.polyLine) = pipeline;
            return pipeline;
        case rwPRIMTYPEPOINTLIST:
            RWIMMEDIGLOBAL(im3DRenderPipelines.pointList) = pipeline;
            return pipeline;
        default:
            RWERROR((E_RX_INVALIDPRIMTYPE));
            break;
        }
    } else {
        switch (primType) {
        case rwPRIMTYPETRILIST:
            if (RWIMMEDIGLOBAL(platformIm3DRenderPipelines.triList) != NULL) {
                RWIMMEDIGLOBAL(im3DRenderPipelines.triList) =
                    RWIMMEDIGLOBAL(platformIm3DRenderPipelines.triList);
            } else {
                RWIMMEDIGLOBAL(im3DRenderPipelines.triList) = NULL;
            }
            return RWIMMEDIGLOBAL(im3DRenderPipelines.triList);
        case rwPRIMTYPETRIFAN:
            if (RWIMMEDIGLOBAL(platformIm3DRenderPipelines.triFan) != NULL) {
                RWIMMEDIGLOBAL(im3DRenderPipelines.triFan) =
                    RWIMMEDIGLOBAL(platformIm3DRenderPipelines.triFan);
            } else {
                RWIMMEDIGLOBAL(im3DRenderPipelines.triFan) = NULL;
            }
            return RWIMMEDIGLOBAL(im3DRenderPipelines.triFan);
        case rwPRIMTYPETRISTRIP:
            if (RWIMMEDIGLOBAL(platformIm3DRenderPipelines.triStrip) != NULL) {
                RWIMMEDIGLOBAL(im3DRenderPipelines.triStrip) =
                    RWIMMEDIGLOBAL(platformIm3DRenderPipelines.triStrip);
            } else {
                RWIMMEDIGLOBAL(im3DRenderPipelines.triStrip) = NULL;
            }
            return RWIMMEDIGLOBAL(platformIm3DRenderPipelines.triStrip);
        case rwPRIMTYPELINELIST:
            if (RWIMMEDIGLOBAL(platformIm3DRenderPipelines.lineList) != NULL) {
                RWIMMEDIGLOBAL(im3DRenderPipelines.lineList) =
                    RWIMMEDIGLOBAL(platformIm3DRenderPipelines.lineList);
            } else {
                RWIMMEDIGLOBAL(im3DRenderPipelines.lineList) = NULL;
            }
            return RWIMMEDIGLOBAL(platformIm3DRenderPipelines.lineList);
        case rwPRIMTYPEPOLYLINE:
            if (RWIMMEDIGLOBAL(platformIm3DRenderPipelines.polyLine) != NULL) {
                RWIMMEDIGLOBAL(im3DRenderPipelines.polyLine) =
                    RWIMMEDIGLOBAL(platformIm3DRenderPipelines.polyLine);
            } else {
                RWIMMEDIGLOBAL(im3DRenderPipelines.polyLine) = NULL;
            }
            return RWIMMEDIGLOBAL(platformIm3DRenderPipelines.polyLine);
        case rwPRIMTYPEPOINTLIST:
            if (RWIMMEDIGLOBAL(platformIm3DRenderPipelines.pointList) != NULL) {
                RWIMMEDIGLOBAL(im3DRenderPipelines.pointList) =
                    RWIMMEDIGLOBAL(platformIm3DRenderPipelines.pointList);
            } else {
                RWIMMEDIGLOBAL(im3DRenderPipelines.pointList) = NULL;
            }
            return RWIMMEDIGLOBAL(platformIm3DRenderPipelines.pointList);
        default:
            RWERROR((E_RX_INVALIDPRIMTYPE));
            break;
        }
    }
    return NULL;
}

/* Closes the immediate module */
void *fn_8049AF94(void *instance, RwInt32 offset, RwInt32 size) {
    fn_8049F990(&RWIMMEDIGLOBAL(platformIm3DRenderPipelines));
    fn_8049F948(&RWIMMEDIGLOBAL(platformIm3DTransformPipeline));
    lbl_805F1684.numInstances--;
    return instance;
}

/* Opens the immediate module with its globals at offset in the engine instance and creates the
 * platform's pipelines */
void *fn_8049AFF8(void *instance, RwInt32 offset, RwInt32 size) {
    RwBool created;

    lbl_805F1684.globalsOffset = offset;
    lbl_805F1680 = RWPLUGINOFFSET(rwImmediGlobals, lbl_805F15B4, offset);
    lbl_805F1684.numInstances++;
    memset(lbl_805F1680, 0, sizeof(rwImmediGlobals));

    created = fn_8049F8AC(&RWIMMEDIGLOBAL(platformIm3DTransformPipeline));
    if (created) {
        created = fn_8049FA1C(&RWIMMEDIGLOBAL(platformIm3DRenderPipelines));
    }
    if (created) {
        return instance;
    }

    fn_8049F990(&RWIMMEDIGLOBAL(platformIm3DRenderPipelines));
    fn_8049F948(&RWIMMEDIGLOBAL(platformIm3DTransformPipeline));
    lbl_805F1684.numInstances--;
    return NULL;
}
