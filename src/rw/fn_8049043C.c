/* RenderWare GameCube 2D immediate mode: lines, triangles and primitives of 2D vertices are drawn
 * with an orthographic projection of the frame buffer, through the current camera's raster
 * viewport. The original file name is unknown. */
#include "rw/rwcore.h"
#include <dolphin/gx.h>
#include <dolphin/vi.h>

/* Reports an error of the core; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwCOREERRORPLUGIN;                                                       \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

#define rwCOREERRORPLUGIN 1

/* The error code of an unsupported primitive type (inferred) */
#define E_RW_INVALIDPRIMTYPE 0x25

/* The GX primitive of each RenderWare primitive type */
GXPrimitive lbl_8056E060[7] = {
    0, GX_LINES, GX_LINESTRIP, GX_TRIANGLES, GX_TRIANGLESTRIP, GX_TRIANGLEFAN, GX_POINTS,
};

/* The orthographic projection of the frame buffer; the x and y scales are set for its size */
f32 lbl_8056E07C[7] = {GX_ORTHOGRAPHIC, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f};

/* The position matrix of 2D vertices; the translation is set for the camera raster's offset */
f32 lbl_8056E098[3][4] = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, -1.0f, 0.0f},
};

/* The projection in use before 2D rendering began */
f32 lbl_805E3158[7];

/* Writes a 2D vertex to the GX FIFO, with its texture coordinates when textured (inferred
 * helpers) */
static inline void Im2DVertexTextured(RwIm2DVertex *vertex) {
    GXPosition3f32(vertex->x, vertex->y, vertex->z);
    GXColor4u8(vertex->emissiveColor.red, vertex->emissiveColor.green, vertex->emissiveColor.blue,
               vertex->emissiveColor.alpha);
    GXTexCoord2f32(vertex->u, vertex->v);
}

static inline void Im2DVertexUntextured(RwIm2DVertex *vertex) {
    GXPosition3f32(vertex->x, vertex->y, vertex->z);
    GXColor4u8(vertex->emissiveColor.red, vertex->emissiveColor.green, vertex->emissiveColor.blue,
               vertex->emissiveColor.alpha);
}

/* Sets up GX for 2D vertices: their vertex format and color channels, texturing when a texture is
 * set, the frame buffer viewport and projection and the camera raster's offset */
void fn_8049043C(void) {
    RwRaster *raster;

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

    if (lbl_805F1664->raster != NULL) {
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

    if (lbl_805F164C->field_rendering) {
        GXSetViewportJitter(0.0f, 0.0f, lbl_805F164C->fbWidth, lbl_805F164C->xfbHeight, 0.0f, 1.0f,
                            VIGetNextField() ^ 1);
    } else {
        GXSetViewport(0.0f, 0.0f, lbl_805F164C->fbWidth, lbl_805F164C->xfbHeight, 0.0f, 1.0f);
    }

    raster = lbl_805F15B4->curCamera->frameBuffer;
    lbl_8056E07C[1] = 2.0f / lbl_805F164C->fbWidth;
    lbl_8056E07C[3] = -2.0f / lbl_805F164C->xfbHeight;
    GXGetProjectionv(lbl_805E3158);
    GXSetProjectionv(lbl_8056E07C);

    lbl_8056E098[0][3] = 0.5f + raster->nOffsetX;
    lbl_8056E098[1][3] = 0.5f + raster->nOffsetY;
    GXLoadPosMtxImm(lbl_8056E098, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
}

/* Restores GX after 2D vertices: the viewport and scissor of the camera's raster when it is a
 * subraster, and the projection */
void fn_80490724(void) {
    RwRaster *raster = lbl_805F15B4->curCamera->frameBuffer;

    if (raster != raster->parent) {
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
    GXSetProjectionv(lbl_805E3158);
}

/* Draws a triangle of 2D vertices (inferred) */
RwBool fn_80490AB8(RwIm2DVertex *vertices, RwInt32 numVertices, RwInt32 vert1, RwInt32 vert2,
                   RwInt32 vert3) {
    RwIm2DVertex *vertex1 = &vertices[vert1];
    RwIm2DVertex *vertex2 = &vertices[vert2];
    RwIm2DVertex *vertex3 = &vertices[vert3];

    fn_8049043C();
    GXBegin(GX_TRIANGLES, GX_VTXFMT0, 3);
    if (lbl_805F1664->raster != NULL) {
        Im2DVertexTextured(vertex1);
        Im2DVertexTextured(vertex2);
        Im2DVertexTextured(vertex3);
    } else {
        Im2DVertexUntextured(vertex1);
        Im2DVertexUntextured(vertex2);
        Im2DVertexUntextured(vertex3);
    }
    fn_80490724();
    return TRUE;
}

/* Draws a line of 2D vertices (inferred) */
RwBool fn_80490CBC(RwIm2DVertex *vertices, RwInt32 numVertices, RwInt32 vert1, RwInt32 vert2) {
    RwIm2DVertex *vertex1 = &vertices[vert1];
    RwIm2DVertex *vertex2 = &vertices[vert2];

    fn_8049043C();
    GXBegin(GX_LINES, GX_VTXFMT0, 2);
    if (lbl_805F1664->raster != NULL) {
        Im2DVertexTextured(vertex1);
        Im2DVertexTextured(vertex2);
    } else {
        Im2DVertexUntextured(vertex1);
        Im2DVertexUntextured(vertex2);
    }
    fn_80490724();
    return TRUE;
}

/* Draws a primitive of 2D vertices (inferred) */
RwBool fn_80490E30(RwInt32 primType, RwIm2DVertex *vertices, RwInt32 numVertices) {
    RwInt32 count;

    fn_8049043C();
    GXBegin(lbl_8056E060[primType], GX_VTXFMT0, numVertices);
    switch (primType) {
    case rwPRIMTYPEPOLYLINE:
    case rwPRIMTYPETRISTRIP:
    case rwPRIMTYPETRIFAN:
        count = numVertices;
        if (lbl_805F1664->raster != NULL) {
            while (count--) {
                Im2DVertexTextured(vertices);
                vertices++;
            }
        } else {
            while (count--) {
                Im2DVertexUntextured(vertices);
                vertices++;
            }
        }
        break;
    case rwPRIMTYPELINELIST:
        count = numVertices >> 1;
        if (lbl_805F1664->raster != NULL) {
            while (count--) {
                Im2DVertexTextured(&vertices[0]);
                Im2DVertexTextured(&vertices[1]);
                vertices += 2;
            }
        } else {
            while (count--) {
                Im2DVertexUntextured(&vertices[0]);
                Im2DVertexUntextured(&vertices[1]);
                vertices += 2;
            }
        }
        break;
    case rwPRIMTYPETRILIST:
        count = numVertices / 3;
        if (lbl_805F1664->raster != NULL) {
            while (count--) {
                Im2DVertexTextured(&vertices[0]);
                Im2DVertexTextured(&vertices[1]);
                Im2DVertexTextured(&vertices[2]);
                vertices += 3;
            }
        } else {
            while (count--) {
                Im2DVertexUntextured(&vertices[0]);
                Im2DVertexUntextured(&vertices[1]);
                Im2DVertexUntextured(&vertices[2]);
                vertices += 3;
            }
        }
        break;
    default:
        RWERROR((E_RW_INVALIDPRIMTYPE));
        break;
    }
    fn_80490724();
    return TRUE;
}

/* Draws an indexed primitive of 2D vertices (inferred) */
RwBool fn_8049140C(RwInt32 primType, RwIm2DVertex *vertices, RwInt32 numVertices,
                   RwImVertexIndex *indices, RwInt32 numIndices) {
    RwInt32 count;

    fn_8049043C();
    GXBegin(lbl_8056E060[primType], GX_VTXFMT0, numIndices);
    switch (primType) {
    case rwPRIMTYPEPOLYLINE:
    case rwPRIMTYPETRISTRIP:
    case rwPRIMTYPETRIFAN:
        count = numIndices;
        if (lbl_805F1664->raster != NULL) {
            while (count--) {
                Im2DVertexTextured(&vertices[*indices]);
                indices++;
            }
        } else {
            while (count--) {
                Im2DVertexUntextured(&vertices[*indices]);
                indices++;
            }
        }
        break;
    case rwPRIMTYPELINELIST:
        count = numIndices >> 1;
        if (lbl_805F1664->raster != NULL) {
            while (count--) {
                Im2DVertexTextured(&vertices[indices[0]]);
                Im2DVertexTextured(&vertices[indices[1]]);
                indices += 2;
            }
        } else {
            while (count--) {
                Im2DVertexUntextured(&vertices[indices[0]]);
                Im2DVertexUntextured(&vertices[indices[1]]);
                indices += 2;
            }
        }
        break;
    case rwPRIMTYPETRILIST:
        count = numIndices / 3;
        if (lbl_805F1664->raster != NULL) {
            while (count--) {
                Im2DVertexTextured(&vertices[indices[0]]);
                Im2DVertexTextured(&vertices[indices[1]]);
                Im2DVertexTextured(&vertices[indices[2]]);
                indices += 3;
            }
        } else {
            while (count--) {
                Im2DVertexUntextured(&vertices[indices[0]]);
                Im2DVertexUntextured(&vertices[indices[1]]);
                Im2DVertexUntextured(&vertices[indices[2]]);
                indices += 3;
            }
        }
        break;
    default:
        RWERROR((E_RW_INVALIDPRIMTYPE));
        break;
    }
    fn_80490724();
    return TRUE;
}
