/* RenderWare immediate mode 3D, GameCube: the nodes of the Im3D pipelines. The transform node
 * keeps the transform's pool; the render node sends its vertices, in order or indexed, straight
 * to the GX FIFO, unlit and coloured by the vertex colours. The original file name is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>

/* Reports an error of the core; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwID_COREPLUGIN;                                                         \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

/* the GX primitive of each RenderWare primitive type */
GXPrimitive lbl_8056EC40[] = {
    (GXPrimitive)0,   GX_LINES,       GX_LINESTRIP, GX_TRIANGLES,
    GX_TRIANGLESTRIP, GX_TRIANGLEFAN, GX_POINTS,
};

rwIm3DPool *lbl_805F16A0; /* the pool of the transform being rendered */

/* Keeps the transform's pool for the render node */
RwBool fn_8049FAFC(RxPipelineNode *self, const RxPipelineNodeParam *params) {
    lbl_805F16A0 = params->dataParam;
    return TRUE;
}

RwChar lbl_8056EC5C[] = "ImmInstance.csl";

RxNodeDefinition lbl_8056EC6C = {
    lbl_8056EC5C,
    {fn_8049FAFC, NULL, NULL, NULL, NULL, NULL, NULL},
    {0, NULL, NULL, 0, NULL},
    0,
    rxNODEDEFCONST,
    0,
};

RxNodeDefinition *fn_8049FB0C(void) {
    return &lbl_8056EC6C;
}

/* Sets the vertex format (positions, colours and, with rwIM3D_VERTEXUV, texture coordinates),
 * unlit vertex colour channels and one TEV stage, textured or not, for the pool's vertices */
static inline void Im3DSetVertexFormat(rwIm3DPool *pool) {
    fn_804950AC(pool->stash.ltm, FALSE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetNumTevStages(1);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);
    if (pool->stash.flags & rwIM3D_VERTEXUV) {
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
        GXSetNumTexGens(1);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE,
                          GX_PTIDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        fn_80497804();
    } else {
        GXSetNumTexGens(0);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    }
}

/* Sends a vertex's position and colour, and its texture coordinates for the textured format */
static inline void Im3DSendVertex(RwIm3DVertex *vertex) {
    GXPosition3f32(vertex->objVertex.x, vertex->objVertex.y, vertex->objVertex.z);
    GXColor4u8(vertex->color.red, vertex->color.green, vertex->color.blue, vertex->color.alpha);
}

static inline void Im3DSendVertexUV(RwIm3DVertex *vertex) {
    GXPosition3f32(vertex->objVertex.x, vertex->objVertex.y, vertex->objVertex.z);
    GXColor4u8(vertex->color.red, vertex->color.green, vertex->color.blue, vertex->color.alpha);
    GXTexCoord2f32(vertex->u, vertex->v);
}

/* Renders the pool's vertices as its primitive */
RwBool fn_8049FB18(RxPipelineNode *self, const RxPipelineNodeParam *params) {
    rwIm3DPool *pool = lbl_805F16A0;
    RwIm3DVertex *vertices = pool->elements;
    RwUInt16 count;

    if (pool->stash.indices != NULL) {
        RwImVertexIndex *indices = pool->stash.indices;

        Im3DSetVertexFormat(pool);
        GXBegin(lbl_8056EC40[pool->stash.primType], GX_VTXFMT0, pool->stash.numIndices);
        switch (pool->stash.primType) {
        case rwPRIMTYPETRISTRIP:
        case rwPRIMTYPETRIFAN:
        case rwPRIMTYPEPOLYLINE:
            count = pool->stash.numIndices;
            if (pool->stash.flags & rwIM3D_VERTEXUV) {
                while (count--) {
                    RwIm3DVertex *vertex = &vertices[*indices++];

                    Im3DSendVertexUV(vertex);
                }
            } else {
                while (count--) {
                    RwIm3DVertex *vertex = &vertices[*indices++];

                    Im3DSendVertex(vertex);
                }
            }
            break;
        case rwPRIMTYPETRILIST:
            count = pool->stash.numIndices / 3;
            if (pool->stash.flags & rwIM3D_VERTEXUV) {
                while (count--) {
                    Im3DSendVertexUV(&vertices[indices[0]]);
                    Im3DSendVertexUV(&vertices[indices[1]]);
                    Im3DSendVertexUV(&vertices[indices[2]]);
                    indices += 3;
                }
            } else {
                while (count--) {
                    Im3DSendVertex(&vertices[indices[0]]);
                    Im3DSendVertex(&vertices[indices[1]]);
                    Im3DSendVertex(&vertices[indices[2]]);
                    indices += 3;
                }
            }
            break;
        case rwPRIMTYPELINELIST:
            count = pool->stash.numIndices >> 1;
            if (pool->stash.flags & rwIM3D_VERTEXUV) {
                while (count--) {
                    Im3DSendVertexUV(&vertices[indices[0]]);
                    Im3DSendVertexUV(&vertices[indices[1]]);
                    indices += 2;
                }
            } else {
                while (count--) {
                    Im3DSendVertex(&vertices[indices[0]]);
                    Im3DSendVertex(&vertices[indices[1]]);
                    indices += 2;
                }
            }
            break;
        default:
            RWERROR((E_RX_INVALIDPRIMTYPE));
            break;
        }
    } else {
        Im3DSetVertexFormat(pool);
        /* this branch reads the vertex count through the pool pointer each time, as the original
         * does */
        GXBegin(lbl_8056EC40[pool->stash.primType], GX_VTXFMT0, lbl_805F16A0->numElements);
        switch (pool->stash.primType) {
        case rwPRIMTYPETRISTRIP:
        case rwPRIMTYPETRIFAN:
        case rwPRIMTYPEPOLYLINE:
            count = lbl_805F16A0->numElements;
            if (pool->stash.flags & rwIM3D_VERTEXUV) {
                while (count--) {
                    Im3DSendVertexUV(vertices);
                    vertices++;
                }
            } else {
                while (count--) {
                    Im3DSendVertex(vertices);
                    vertices++;
                }
            }
            break;
        case rwPRIMTYPETRILIST:
            count = lbl_805F16A0->numElements / 3;
            if (pool->stash.flags & rwIM3D_VERTEXUV) {
                while (count--) {
                    Im3DSendVertexUV(&vertices[0]);
                    Im3DSendVertexUV(&vertices[1]);
                    Im3DSendVertexUV(&vertices[2]);
                    vertices += 3;
                }
            } else {
                while (count--) {
                    Im3DSendVertex(&vertices[0]);
                    Im3DSendVertex(&vertices[1]);
                    Im3DSendVertex(&vertices[2]);
                    vertices += 3;
                }
            }
            break;
        case rwPRIMTYPELINELIST:
            count = lbl_805F16A0->numElements >> 1;
            if (pool->stash.flags & rwIM3D_VERTEXUV) {
                while (count--) {
                    Im3DSendVertexUV(&vertices[0]);
                    Im3DSendVertexUV(&vertices[1]);
                    vertices += 2;
                }
            } else {
                while (count--) {
                    Im3DSendVertex(&vertices[0]);
                    Im3DSendVertex(&vertices[1]);
                    vertices += 2;
                }
            }
            break;
        default:
            RWERROR((E_RX_INVALIDPRIMTYPE));
            break;
        }
    }
    return TRUE;
}

RwChar lbl_8056ECAC[] = "SubmitNoLight.csl";

RxNodeDefinition lbl_8056ECC0 = {
    lbl_8056ECAC,
    {fn_8049FB18, NULL, NULL, NULL, NULL, NULL, NULL},
    {0, NULL, NULL, 0, NULL},
    0,
    rxNODEDEFCONST,
    0,
};

RxNodeDefinition *fn_804A0760(void) {
    return &lbl_8056ECC0;
}
