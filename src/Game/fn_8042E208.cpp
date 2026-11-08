/* RW3 raster filtering and parameter state; original filename and class name unknown. */
#include "Game/EffectManagerRW3.h"
#include "Game/EffectSystemRW3.h"
#include "Game/UnkEffectRW3Parameters.h"
#include "rw/rpworld.h"

float lbl_8056AC30 = 0.95f;

namespace Effect {
namespace RW3 {

/* The methods, field names and signatures are inferred from the native callers and accesses. */
struct UnkEffectManagerRW3Data3C {
    UnkEffectManagerRW3Data3C(EffectManagerRW3 *manager, const UnkEffectRW3Param0 *parameters);
    ~UnkEffectManagerRW3Data3C() {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 2; ++j) {
                if (m_ownedRasters[i][j] != 0) {
                    if (m_ownedRasters[i][j]->parent != m_ownedRasters[i][j]) {
                        fn_80489BDC(m_ownedRasters[i][j]->parent);
                    }
                    fn_80489BDC(m_ownedRasters[i][j]);
                    m_ownedRasters[i][j] = 0;
                }
            }
        }
        if (field_0x44 != 0) {
            fn_80489BDC(field_0x44);
            field_0x44 = 0;
        }
    }
    void fn_8042E208();
    void fn_8042F29C(RwRaster **rasters, int *index, float distance, int xCount, int yCount);
    void fn_8042F800(float x, float y, float width, float height, float u0, float v0, float u1,
                     float v1);
    void fn_8042F8B4();
    void fn_8042FD14();
    UnkEffectRW3Param0 fn_804303D8();
    void fn_80430444(const UnkEffectRW3Param0 &parameters);
    void fn_804304B0(const UnkEffectRW3Param0 *parameters);

    /* 0x00 */ RwRaster *m_frameBuffer;
    /* 0x04 */ RwRaster *m_zBuffer;
    /* 0x08 */ RwRaster *m_rasters[3][2];
    /* 0x20 */ RwRaster *m_ownedRasters[3][2];
    /* 0x38 */ int m_indices[3];
    /* 0x44 */ RwRaster *field_0x44;
    /* 0x48 */ RwIm2DVertex m_quad[4];
    /* 0xA8 */ float m_u[3][2];
    /* 0xC0 */ float m_v[3][2];
    /* 0xD8 */ float m_recipNearPlane;
    /* 0xDC */ RwCamera *m_camera;
    /* 0xE0 */ UnkEffectRW3Param0 m_parameters;
    /* 0x100 */ EffectManagerRW3 *m_manager;

  private:
    UnkEffectManagerRW3Data3C(const UnkEffectManagerRW3Data3C &);
    UnkEffectManagerRW3Data3C &operator=(const UnkEffectManagerRW3Data3C &);
};
CHECK_SIZEOF(UnkEffectManagerRW3Data3C, 0x104);

void EffectManagerRW3::fn_80430A08(const UnkEffectRW3Param0 *parameters) {
    field_0x3C = new UnkEffectManagerRW3Data3C(this, parameters);
}

void EffectManagerRW3::fn_80430944() {
    if (field_0x3C != 0) {
        delete field_0x3C;
        field_0x3C = 0;
    }
}

void EffectManagerRW3::fn_80430918() {
    if (field_0x3C != 0) {
        field_0x3C->fn_8042FD14();
    }
}

void EffectManagerRW3::fn_804308EC() {
    if (field_0x3C != 0) {
        field_0x3C->fn_8042F8B4();
    }
}

void EffectManagerRW3::fn_804308C0(const UnkEffectRW3Param0 &parameters) {
    if (field_0x3C != 0) {
        field_0x3C->fn_80430444(parameters);
    }
}

UnkEffectRW3Param0 EffectManagerRW3::fn_80430844() {
    if (field_0x3C != 0) {
        return field_0x3C->fn_804303D8();
    }
    return UnkEffectRW3Param0();
}

UnkEffectManagerRW3Data3C::UnkEffectManagerRW3Data3C(EffectManagerRW3 *manager,
                                                     const UnkEffectRW3Param0 *parameters)
    : m_manager(manager) {
    EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
    // The original early exits leave the raster storage uninitialized.
    if (system == 0) {
        return;
    }
    if ((m_camera = system->GetCamera()) == 0) {
        return;
    }
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 2; ++j) {
            m_ownedRasters[i][j] = 0;
            m_rasters[i][j] = 0;
        }
        m_indices[i] = 0;
    }
    field_0x44 = 0;
    RwRect rectangle = {0, 0, m_camera->frameBuffer->width, m_camera->frameBuffer->height};
    if (manager->field_0x34 != 0) {
        m_rasters[0][0] = manager->field_0x34;
    } else {
        m_rasters[0][0] = m_ownedRasters[0][0] = fn_80489E74(0, 0, 0, rwRASTERTYPECAMERATEXTURE);
        fn_80489DC4(m_ownedRasters[0][0],
                    fn_80489E74(rectangle.w, rectangle.h, 0, rwRASTERTYPECAMERATEXTURE),
                    &rectangle);
    }
    m_rasters[1][0] = m_ownedRasters[1][0] =
        fn_80489E74(128, 128, 0, rwRASTERFORMAT8888 | rwRASTERTYPECAMERATEXTURE);
    m_rasters[2][0] = m_ownedRasters[2][0] =
        fn_80489E74(64, 64, 0, rwRASTERFORMAT8888 | rwRASTERTYPECAMERATEXTURE);
    if (manager->field_0x38 != 0) {
        m_rasters[0][1] = manager->field_0x38;
    } else {
        m_rasters[0][1] = m_ownedRasters[0][1] = fn_80489E74(0, 0, 0, rwRASTERTYPECAMERATEXTURE);
        fn_80489DC4(m_ownedRasters[0][1],
                    fn_80489E74(rectangle.w, rectangle.h, 0, rwRASTERTYPECAMERATEXTURE),
                    &rectangle);
    }
    if (m_rasters[0][0] == 0) {
        return;
    }
    m_recipNearPlane = 1.0f / m_camera->nearPlane;
    for (int i = 0; i < 4; ++i) {
        m_quad[i].z = RwIm2DGetNearScreenZ();
        m_quad[i].emissiveColor.red = 255;
        m_quad[i].emissiveColor.green = 255;
        m_quad[i].emissiveColor.blue = 255;
        m_quad[i].emissiveColor.alpha = 255;
    }
    if (parameters != 0) {
        fn_804304B0(parameters);
    } else {
        m_parameters.field_0x0 = 1;
        m_parameters.field_0x3 = 0;
        m_parameters.field_0x4 = 0;
        m_parameters.field_0x1 = 1;
        m_parameters.field_0x5 = 0x20;
        m_parameters.field_0x8 = 1.0f;
        m_parameters.field_0x10 = 2;
        m_parameters.field_0x14 = 2;
        m_parameters.field_0x2 = 1;
        m_parameters.field_0x6 = 0x30;
        m_parameters.field_0xC = 1.0f;
        m_parameters.field_0x18 = 4;
        m_parameters.field_0x1C = 4;
    }
}

void UnkEffectManagerRW3Data3C::fn_804304B0(const UnkEffectRW3Param0 *parameters) {
    if (parameters != 0) {
        m_parameters = *parameters;
    }
}

void UnkEffectManagerRW3Data3C::fn_80430444(const UnkEffectRW3Param0 &parameters) {
    m_parameters = parameters;
}

UnkEffectRW3Param0 UnkEffectManagerRW3Data3C::fn_804303D8() {
    return m_parameters;
}

void UnkEffectManagerRW3Data3C::fn_8042FD14() {
    if (m_rasters[0][0] == 0) {
        return;
    }
    if (m_parameters.field_0x0 == 0) {
        return;
    }
    if (m_parameters.field_0x1 == 0 && m_parameters.field_0x2 == 0) {
        return;
    }
    EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
    if (system == 0) {
        return;
    }
    if ((m_camera = system->GetCamera()) == 0) {
        return;
    }
    m_frameBuffer = m_camera->frameBuffer;
    m_zBuffer = m_camera->zBuffer;
    RwInt16 xOffset, yOffset;
    fn_80489B2C(m_frameBuffer, &xOffset, &yOffset);
    RwRect rectangle;
    rectangle.x = xOffset;
    rectangle.y = yOffset;
    rectangle.w = m_frameBuffer->width;
    rectangle.h = m_frameBuffer->height;
    fn_80489DC4(m_rasters[0][0], m_rasters[0][0]->parent, &rectangle);
    fn_80489DC4(m_rasters[0][1], m_rasters[0][1]->parent, &rectangle);
    unsigned int width = m_frameBuffer->parent->width;
    unsigned int height = m_frameBuffer->parent->height;
    m_u[0][0] = (xOffset + 0.5f) / width;
    m_v[0][0] = (yOffset + 0.5f) / height;
    m_u[0][1] = (xOffset + rectangle.w + 0.5f) / width;
    m_v[0][1] = (yOffset + rectangle.h + 0.5f) / height;
    m_v[1][0] = 0.0f;
    m_u[1][0] = 0.0f;
    m_v[1][1] = 1.0f;
    m_u[1][1] = 1.0f;
    m_v[2][0] = 0.0f;
    m_u[2][0] = 0.0f;
    m_v[2][1] = 1.0f;
    m_u[2][1] = 1.0f;
    fn_804951B8(m_rasters[0][1], FALSE);
    if (xOffset != 0 || yOffset != 0) {
        rectangle.x = xOffset;
        rectangle.y = yOffset;
        rectangle.w = 128;
        rectangle.h = 128;
        if (m_rasters[1][1] != 0) {
            RwRaster *parent = m_ownedRasters[1][1]->parent;
            if (parent->width != xOffset + 128 || parent->height != yOffset + 128) {
                if (parent != m_ownedRasters[1][1]) {
                    fn_80489BDC(parent);
                }
                fn_80489BDC(m_ownedRasters[1][1]);
                m_ownedRasters[1][1] =
                    fn_80489E74(0, 0, 0, rwRASTERFORMAT8888 | rwRASTERTYPECAMERATEXTURE);
                fn_80489DC4(m_ownedRasters[1][1],
                            fn_80489E74(xOffset + 128, yOffset + 128, 0,
                                        rwRASTERFORMAT8888 | rwRASTERTYPECAMERATEXTURE),
                            &rectangle);
            }
        } else {
            m_ownedRasters[1][1] =
                fn_80489E74(0, 0, 0, rwRASTERFORMAT8888 | rwRASTERTYPECAMERATEXTURE);
            fn_80489DC4(m_ownedRasters[1][1],
                        fn_80489E74(xOffset + 128, yOffset + 128, 0,
                                    rwRASTERFORMAT8888 | rwRASTERTYPECAMERATEXTURE),
                        &rectangle);
        }
        m_rasters[1][1] = m_ownedRasters[1][1];
        m_u[1][0] = (xOffset != 0 ? xOffset + 0.5f : xOffset) / (xOffset + 128);
        m_v[1][0] = (yOffset != 0 ? yOffset + 0.5f : yOffset) / (yOffset + 128);
        m_v[1][1] = 1.0f;
        m_u[1][1] = 1.0f;
        rectangle.w = 64;
        rectangle.h = 64;
        if (m_rasters[2][1] != 0) {
            RwRaster *parent = m_ownedRasters[2][1]->parent;
            if (parent->width != xOffset + 64 || parent->height != yOffset + 64) {
                if (parent != m_ownedRasters[2][1]) {
                    fn_80489BDC(parent);
                }
                fn_80489BDC(m_ownedRasters[2][1]);
                m_ownedRasters[2][1] =
                    fn_80489E74(0, 0, 0, rwRASTERFORMAT8888 | rwRASTERTYPECAMERATEXTURE);
                fn_80489DC4(m_ownedRasters[2][1],
                            fn_80489E74(xOffset + 64, yOffset + 64, 0,
                                        rwRASTERFORMAT8888 | rwRASTERTYPECAMERATEXTURE),
                            &rectangle);
            }
        } else {
            m_ownedRasters[2][1] =
                fn_80489E74(0, 0, 0, rwRASTERFORMAT8888 | rwRASTERTYPECAMERATEXTURE);
            fn_80489DC4(m_ownedRasters[2][1],
                        fn_80489E74(xOffset + 64, yOffset + 64, 0,
                                    rwRASTERFORMAT8888 | rwRASTERTYPECAMERATEXTURE),
                        &rectangle);
        }
        m_rasters[2][1] = m_ownedRasters[2][1];
        m_u[2][0] = (xOffset != 0 ? xOffset + 0.5f : xOffset) / (xOffset + 64);
        m_v[2][0] = (yOffset != 0 ? yOffset + 0.5f : yOffset) / (yOffset + 64);
        m_v[2][1] = 1.0f;
        m_u[2][1] = 1.0f;
        m_indices[1] = 1;
        m_indices[2] = 1;
    } else {
        m_indices[1] = 0;
        m_indices[2] = 0;
    }
    RwRGBA clear = {0, 0, 0, 0};
    fn_804857BC(m_camera, &clear, rwCAMERACLEARIMAGE);
    RpWorld *world = fn_804621B0(m_camera);
    if (world != 0) {
        fn_80462184(world, m_camera);
    }
}

void UnkEffectManagerRW3Data3C::fn_8042F8B4() {
    if (m_rasters[0][0] == 0) {
        return;
    }
    if (m_parameters.field_0x0 == 0) {
        return;
    }
    if (m_parameters.field_0x1 == 0 && m_parameters.field_0x2 == 0) {
        return;
    }
    EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
    if (system == 0) {
        return;
    }
    if ((m_camera = system->GetCamera()) == 0) {
        return;
    }
    RwInt32 srcBlend, destBlend;
    RwBool zTest, zWrite;
    RwInt32 address;
    RwBool alpha;
    RwInt32 shade, filter;
    RwRenderStateGet(rwRENDERSTATESRCBLEND, &srcBlend);
    RwRenderStateGet(rwRENDERSTATEDESTBLEND, &destBlend);
    RwRenderStateGet(rwRENDERSTATEZTESTENABLE, &zTest);
    RwRenderStateGet(rwRENDERSTATEZWRITEENABLE, &zWrite);
    RwRenderStateGet(rwRENDERSTATETEXTUREADDRESS, &address);
    RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &alpha);
    RwRenderStateGet(rwRENDERSTATESHADEMODE, &shade);
    RwRenderStateGet(rwRENDERSTATETEXTUREFILTER, &filter);
    // The original detaches the depth raster (stores null), restoring neither it nor ZWRITEENABLE.
    m_camera->zBuffer = 0;
    if (fn_804854DC(m_camera) != 0) {
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
        RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSCLAMP);
        RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);
        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
        fn_804854B4(m_camera);
    }
    fn_804951B8(m_rasters[0][0], FALSE);
    fn_8042E208();
    if (fn_804854DC(m_camera) != 0) {
        fn_8042F800(0.0f, 0.0f, m_frameBuffer->width, m_frameBuffer->height, m_u[0][0], m_v[0][0],
                    m_u[0][1], m_v[0][1]);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, m_rasters[0][1]->parent);
        RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_quad, 4);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)srcBlend);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)destBlend);
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)zTest);
        RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void *)address);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)alpha);
        RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)shade);
        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)filter);
        fn_804854B4(m_camera);
    }
}

void UnkEffectManagerRW3Data3C::fn_8042F800(float x, float y, float width, float height, float u0,
                                            float v0, float u1, float v1) {
    float left = 0.0f;
    if (x >= 0.0f) {
        u1 -= (u1 - u0) * (x / width);
        left = x;
    } else {
        u0 -= (u1 - u0) * (x / width);
        width += x;
    }
    float top = 0.0f;
    if (y >= 0.0f) {
        v1 -= (v1 - v0) * (y / height);
        top = y;
    } else {
        v0 -= (v1 - v0) * (y / height);
        height += y;
    }
    m_quad[0].x = left;
    m_quad[0].y = top;
    m_quad[0].u = u0;
    m_quad[0].v = v0;
    m_quad[1].x = left;
    m_quad[1].y = height;
    m_quad[1].u = u0;
    m_quad[1].v = v1;
    m_quad[2].x = width;
    m_quad[2].y = top;
    m_quad[2].u = u1;
    m_quad[2].v = v0;
    m_quad[3].x = width;
    m_quad[3].y = height;
    m_quad[3].u = u1;
    m_quad[3].v = v1;
}

void UnkEffectManagerRW3Data3C::fn_8042F29C(RwRaster **rasters, int *index, float distance,
                                            int xCount, int yCount) {
    RwRGBA clear = {0, 0, 0, 255};
    if (xCount != 0) {
        fn_804857BC(m_camera, &clear, rwCAMERACLEARIMAGE);
        if (fn_804854DC(m_camera) != 0) {
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, rasters[*index]->parent);
            fn_8042F800(-0.5f, -0.5f, rasters[*index]->width, rasters[*index]->height,
                        m_u[m_indices[0]][0], m_v[m_indices[0]][0], m_u[m_indices[0]][1],
                        m_v[m_indices[0]][1]);
            RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_quad, 4);
            float sign = 1.0f;
            for (int i = 0; i < xCount; ++i) {
                fn_8042F800((i / 2 + 1) * (sign * distance) - 0.5f, -0.5f, rasters[*index]->width,
                            rasters[*index]->height, m_u[m_indices[0]][0], m_v[m_indices[0]][0],
                            m_u[m_indices[0]][1], m_v[m_indices[0]][1]);
                RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_quad, 4);
                if (sign > 0.0f) {
                    sign = -1.0f;
                } else {
                    sign = 1.0f;
                }
            }
            fn_804854B4(m_camera);
        }
        fn_804951B8(rasters[*index], FALSE);
    }
    if (yCount != 0) {
        fn_804857BC(m_camera, &clear, rwCAMERACLEARIMAGE);
        if (fn_804854DC(m_camera) != 0) {
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, rasters[*index]->parent);
            fn_8042F800(-0.5f, -0.5f, rasters[*index]->width, rasters[*index]->height,
                        m_u[m_indices[0]][0], m_v[m_indices[0]][0], m_u[m_indices[0]][1],
                        m_v[m_indices[0]][1]);
            RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_quad, 4);
            float sign = 1.0f;
            for (int i = 0; i < yCount; ++i) {
                fn_8042F800(-0.5f, (i / 2 + 1) * (sign * distance) - 0.5f, rasters[*index]->width,
                            rasters[*index]->height, m_u[m_indices[0]][0], m_v[m_indices[0]][0],
                            m_u[m_indices[0]][1], m_v[m_indices[0]][1]);
                RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_quad, 4);
                if (sign > 0.0f) {
                    sign = -1.0f;
                } else {
                    sign = 1.0f;
                }
            }
            fn_804854B4(m_camera);
        }
        fn_804951B8(rasters[*index], FALSE);
    }
}

void UnkEffectManagerRW3Data3C::fn_8042E208() {
    if (m_parameters.field_0x1 != 0) {
        m_indices[0] = 0;
        RwRaster *source = m_rasters[0][0];
        RwRaster *destination = m_rasters[1][m_indices[1]];
        int color = m_parameters.field_0x5 * lbl_8056AC30;
        for (int i = 0; i < 4; ++i) {
            m_quad[i].emissiveColor.red = color;
            m_quad[i].emissiveColor.green = color;
            m_quad[i].emissiveColor.blue = color;
            m_quad[i].emissiveColor.alpha = 255;
        }
        if (source != 0 && fn_804854DC(m_camera) != 0) {
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, source->parent);
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO);
            fn_8042F800(-0.5f, -0.5f, destination->width, destination->height, m_u[m_indices[0]][0],
                        m_v[m_indices[0]][0], m_u[m_indices[0]][1], m_v[m_indices[0]][1]);
            RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_quad, 4);
            fn_804854B4(m_camera);
        }
        fn_804951B8(destination, FALSE);
        for (int i = 0; i < 4; ++i) {
            m_quad[i].emissiveColor.red = 255;
            m_quad[i].emissiveColor.green = 255;
            m_quad[i].emissiveColor.blue = 255;
            m_quad[i].emissiveColor.alpha = 255;
        }
        m_indices[0] = 1;
        fn_8042F29C(m_rasters[1], &m_indices[1], m_parameters.field_0x8, m_parameters.field_0x10,
                    m_parameters.field_0x14);
    }
    if (m_parameters.field_0x2 != 0) {
        if (m_parameters.field_0x4 != 0) {
            m_indices[0] = 0;
            RwRaster *source = m_rasters[0][0];
            RwRaster *destination = m_rasters[2][m_indices[2]];
            int color = m_parameters.field_0x6 * lbl_8056AC30;
            for (int i = 0; i < 4; ++i) {
                m_quad[i].emissiveColor.red = color;
                m_quad[i].emissiveColor.green = color;
                m_quad[i].emissiveColor.blue = color;
                m_quad[i].emissiveColor.alpha = 255;
            }
            if (source != 0 && fn_804854DC(m_camera) != 0) {
                RwRenderStateSet(rwRENDERSTATETEXTURERASTER, source->parent);
                RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
                RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO);
                fn_8042F800(-0.5f, -0.5f, destination->width, destination->height,
                            m_u[m_indices[0]][0], m_v[m_indices[0]][0], m_u[m_indices[0]][1],
                            m_v[m_indices[0]][1]);
                RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_quad, 4);
                fn_804854B4(m_camera);
            }
            fn_804951B8(destination, FALSE);
            for (int i = 0; i < 4; ++i) {
                m_quad[i].emissiveColor.red = 255;
                m_quad[i].emissiveColor.green = 255;
                m_quad[i].emissiveColor.blue = 255;
                m_quad[i].emissiveColor.alpha = 255;
            }
        } else if (m_parameters.field_0x1 != 0) {
            m_indices[0] = 1;
            RwRaster *source = m_rasters[1][m_indices[1]];
            RwRaster *destination = m_rasters[2][m_indices[2]];
            unsigned char color = m_parameters.field_0x6;
            for (int i = 0; i < 4; ++i) {
                m_quad[i].emissiveColor.red = color;
                m_quad[i].emissiveColor.green = color;
                m_quad[i].emissiveColor.blue = color;
                m_quad[i].emissiveColor.alpha = 255;
            }
            if (source != 0 && fn_804854DC(m_camera) != 0) {
                RwRenderStateSet(rwRENDERSTATETEXTURERASTER, source->parent);
                RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
                RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO);
                fn_8042F800(-0.5f, -0.5f, destination->width, destination->height,
                            m_u[m_indices[0]][0], m_v[m_indices[0]][0], m_u[m_indices[0]][1],
                            m_v[m_indices[0]][1]);
                RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_quad, 4);
                fn_804854B4(m_camera);
            }
            fn_804951B8(destination, FALSE);
            for (int i = 0; i < 4; ++i) {
                m_quad[i].emissiveColor.red = 255;
                m_quad[i].emissiveColor.green = 255;
                m_quad[i].emissiveColor.blue = 255;
                m_quad[i].emissiveColor.alpha = 255;
            }
        } else {
            m_indices[0] = 0;
            RwRaster *source = m_rasters[0][0];
            RwRaster *destination = m_rasters[2][m_indices[2]];
            int color = m_parameters.field_0x6 * lbl_8056AC30;
            for (int i = 0; i < 4; ++i) {
                m_quad[i].emissiveColor.red = color;
                m_quad[i].emissiveColor.green = color;
                m_quad[i].emissiveColor.blue = color;
                m_quad[i].emissiveColor.alpha = 255;
            }
            if (source != 0 && fn_804854DC(m_camera) != 0) {
                RwRenderStateSet(rwRENDERSTATETEXTURERASTER, source->parent);
                RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
                RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO);
                fn_8042F800(-0.5f, -0.5f, destination->width, destination->height,
                            m_u[m_indices[0]][0], m_v[m_indices[0]][0], m_u[m_indices[0]][1],
                            m_v[m_indices[0]][1]);
                RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_quad, 4);
                fn_804854B4(m_camera);
            }
            fn_804951B8(destination, FALSE);
            for (int i = 0; i < 4; ++i) {
                m_quad[i].emissiveColor.red = 255;
                m_quad[i].emissiveColor.green = 255;
                m_quad[i].emissiveColor.blue = 255;
                m_quad[i].emissiveColor.alpha = 255;
            }
        }
        m_indices[0] = 2;
        fn_8042F29C(m_rasters[2], &m_indices[2], m_parameters.field_0xC, m_parameters.field_0x18,
                    m_parameters.field_0x1C);
    }
    RwRGBA clear = {0, 0, 0, 0};
    fn_804857BC(m_camera, &clear, rwCAMERACLEARIMAGE);
    if (m_parameters.field_0x3 != 0) {
        if (m_parameters.field_0x1 != 0) {
            m_indices[0] = 1;
            RwRaster *destination = m_rasters[0][0];
            RwRaster *source = m_rasters[1][m_indices[1]];
            if (fn_804854DC(m_camera) != 0) {
                fn_8042F800(0.0f, 0.0f, destination->width, destination->height,
                            m_u[m_indices[0]][0], m_v[m_indices[0]][0], m_u[m_indices[0]][1],
                            m_v[m_indices[0]][1]);
                RwRenderStateSet(rwRENDERSTATETEXTURERASTER, source->parent);
                RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
                RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
                RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_quad, 4);
                fn_804854B4(m_camera);
            }
        }
        if (m_parameters.field_0x2 != 0) {
            m_indices[0] = 2;
            RwRaster *destination = m_rasters[0][0];
            RwRaster *source = m_rasters[2][m_indices[2]];
            if (fn_804854DC(m_camera) != 0) {
                fn_8042F800(0.0f, 0.0f, destination->width, destination->height,
                            m_u[m_indices[0]][0], m_v[m_indices[0]][0], m_u[m_indices[0]][1],
                            m_v[m_indices[0]][1]);
                RwRenderStateSet(rwRENDERSTATETEXTURERASTER, source->parent);
                RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
                RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
                RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_quad, 4);
                fn_804854B4(m_camera);
            }
        }
    } else {
        if (m_parameters.field_0x2 != 0) {
            if (m_parameters.field_0x1 != 0) {
                m_indices[0] = 1;
                RwRaster *source = m_rasters[1][m_indices[1]];
                if (fn_804854DC(m_camera) != 0) {
                    fn_8042F800(0.0f, 0.0f, source->width, source->height, m_u[m_indices[0]][0],
                                m_v[m_indices[0]][0], m_u[m_indices[0]][1], m_v[m_indices[0]][1]);
                    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, source->parent);
                    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
                    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
                    RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_quad, 4);
                    fn_804854B4(m_camera);
                }
                m_indices[0] = 2;
                {
                    RwRaster *destination = m_rasters[1][m_indices[1]];
                    RwRaster *source = m_rasters[2][m_indices[2]];
                    if (fn_804854DC(m_camera) != 0) {
                        fn_8042F800(0.0f, 0.0f, destination->width, destination->height,
                                    m_u[m_indices[0]][0], m_v[m_indices[0]][0],
                                    m_u[m_indices[0]][1], m_v[m_indices[0]][1]);
                        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, source->parent);
                        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
                        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
                        RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_quad, 4);
                        fn_804854B4(m_camera);
                    }
                }
                fn_804951B8(m_rasters[1][m_indices[1]], FALSE);
                fn_804857BC(m_camera, &clear, rwCAMERACLEARIMAGE);
            } else {
                m_indices[0] = 2;
                RwRaster *destination = m_rasters[0][0];
                RwRaster *source = m_rasters[2][m_indices[2]];
                if (fn_804854DC(m_camera) != 0) {
                    fn_8042F800(0.0f, 0.0f, destination->width, destination->height,
                                m_u[m_indices[0]][0], m_v[m_indices[0]][0], m_u[m_indices[0]][1],
                                m_v[m_indices[0]][1]);
                    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, source->parent);
                    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
                    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
                    RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_quad, 4);
                    fn_804854B4(m_camera);
                }
            }
        }
        if (m_parameters.field_0x1 != 0) {
            m_indices[0] = 1;
            RwRaster *destination = m_rasters[0][0];
            RwRaster *source = m_rasters[1][m_indices[1]];
            if (fn_804854DC(m_camera) != 0) {
                fn_8042F800(0.0f, 0.0f, destination->width, destination->height,
                            m_u[m_indices[0]][0], m_v[m_indices[0]][0], m_u[m_indices[0]][1],
                            m_v[m_indices[0]][1]);
                RwRenderStateSet(rwRENDERSTATETEXTURERASTER, source->parent);
                RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
                RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
                RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_quad, 4);
                fn_804854B4(m_camera);
            }
        }
    }
    m_indices[0] = 0;
    RwRaster *source = m_rasters[0][0];
    if (fn_804854DC(m_camera) != 0) {
        fn_8042F800(0.0f, 0.0f, source->width, source->height, m_u[m_indices[0]][0],
                    m_v[m_indices[0]][0], m_u[m_indices[0]][1], m_v[m_indices[0]][1]);
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, source->parent);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
        RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_quad, 4);
        fn_804854B4(m_camera);
    }
}

} // namespace RW3
} // namespace Effect
