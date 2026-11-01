/* RW3 raster-state methods; original type and filename are unknown. */
#include "Game/fn_80437118.h"
#include "Game/EffectSystemRW3.h"

namespace Effect {
namespace RW3 {

void EffectManagerRW3::fn_804389F0(const UnkEffectRW3Param2 *parameters) {
    field_0x44 = new UnkEffectManagerRW3Data44(this, parameters);
}

void EffectManagerRW3::fn_80438974() {
    if (field_0x44 != 0) {
        delete field_0x44;
        field_0x44 = 0;
    }
}

void EffectManagerRW3::fn_80438948() {
    if (field_0x44 != 0) {
        field_0x44->fn_80437C40();
    }
}

void EffectManagerRW3::fn_8043891C(const UnkEffectRW3Param2 &parameters) {
    if (field_0x44 != 0) {
        field_0x44->fn_80438684(parameters);
    }
}

UnkEffectRW3Param2 EffectManagerRW3::fn_804388A4() {
    if (field_0x44 != 0) {
        return field_0x44->fn_80438658();
    }
    UnkEffectRW3Param2 result;
    return result;
}

UnkEffectManagerRW3Data44::UnkEffectManagerRW3Data44(EffectManagerRW3 *manager,
                                                     const UnkEffectRW3Param2 *parameters)
    : m_manager(manager) {
    // The original leaves raster and vertex storage untouched when no camera is available.
    EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
    if (system == 0) {
        return;
    }
    RwCamera *camera = system->GetCamera();
    if (camera == 0) {
        return;
    }
    RwRect rect = {0, 0, camera->frameBuffer->width, camera->frameBuffer->height};
    if (manager->field_0x34 != 0) {
        m_raster = manager->field_0x34;
        m_ownedRaster = 0;
    } else {
        m_raster = m_ownedRaster = fn_80489E74(0, 0, 0, rwRASTERTYPECAMERATEXTURE);
        fn_80489DC4(m_ownedRaster,
                    fn_80489E74(rect.w, rect.h, 0, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT8888),
                    &rect);
    }
    m_vertices[0].z = RwIm2DGetNearScreenZ();
    m_vertices[1].z = RwIm2DGetNearScreenZ();
    m_vertices[2].z = RwIm2DGetNearScreenZ();
    m_vertices[3].z = RwIm2DGetNearScreenZ();
    m_vertices[0].emissiveColor.red = 0xFF;
    m_vertices[0].emissiveColor.green = 0xFF;
    m_vertices[0].emissiveColor.blue = 0xFF;
    m_vertices[0].emissiveColor.alpha = 0xFF;
    m_vertices[1].emissiveColor.red = 0xFF;
    m_vertices[1].emissiveColor.green = 0xFF;
    m_vertices[1].emissiveColor.blue = 0xFF;
    m_vertices[1].emissiveColor.alpha = 0xFF;
    m_vertices[2].emissiveColor.red = 0xFF;
    m_vertices[2].emissiveColor.green = 0xFF;
    m_vertices[2].emissiveColor.blue = 0xFF;
    m_vertices[2].emissiveColor.alpha = 0xFF;
    m_vertices[3].emissiveColor.red = 0xFF;
    m_vertices[3].emissiveColor.green = 0xFF;
    m_vertices[3].emissiveColor.blue = 0xFF;
    m_vertices[3].emissiveColor.alpha = 0xFF;
    if (parameters != 0) {
        fn_804386B0(parameters);
    }
}

void UnkEffectManagerRW3Data44::fn_804386B0(const UnkEffectRW3Param2 *parameters) {
    if (parameters != 0) {
        m_parameters = *parameters;
    }
}

void UnkEffectManagerRW3Data44::fn_80438684(const UnkEffectRW3Param2 &parameters) {
    m_parameters = parameters;
}

UnkEffectRW3Param2 UnkEffectManagerRW3Data44::fn_80438658() {
    return m_parameters;
}

void UnkEffectManagerRW3Data44::fn_80437C40() {
    if ((m_parameters.field_0x0 != 0 || m_parameters.field_0x1 != 0) && m_raster != 0) {
        EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
        if (system != 0) {
            RwCamera *camera = system->GetCamera();
            if (camera != 0) {
                RwRaster *cameraRaster = camera->frameBuffer;
                RwInt32 srcBlend;
                RwInt32 dstBlend;
                RwBool vertexAlpha;
                RwBool zTest;
                RwBool zWrite;
                RwBool fog;
                RwInt32 filter;
                RwRenderStateGet(rwRENDERSTATESRCBLEND, &srcBlend);
                RwRenderStateGet(rwRENDERSTATEDESTBLEND, &dstBlend);
                RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &vertexAlpha);
                RwRenderStateGet(rwRENDERSTATEZTESTENABLE, &zTest);
                RwRenderStateGet(rwRENDERSTATEZWRITEENABLE, &zWrite);
                RwRenderStateGet(rwRENDERSTATEFOGENABLE, &fog);
                RwRenderStateGet(rwRENDERSTATETEXTUREFILTER, &filter);
                RwInt16 xOffset;
                RwInt16 yOffset;
                fn_80489B2C(cameraRaster, &xOffset, &yOffset);
                RwRect rect;
                rect.x = xOffset;
                rect.y = yOffset;
                rect.w = cameraRaster->width;
                rect.h = cameraRaster->height;
                fn_80489DC4(m_raster, m_raster->parent, &rect);
                float u[2] = {0.0f, 0.0f};
                float v[2] = {0.0f, 0.0f};
                u[0] = (0.5f + xOffset) / static_cast<unsigned int>(cameraRaster->parent->width);
                u[1] = (0.5f + (xOffset + rect.w)) /
                       static_cast<unsigned int>(cameraRaster->parent->width);
                v[0] = (0.5f + yOffset) / static_cast<unsigned int>(cameraRaster->parent->height);
                v[1] = (0.5f + (yOffset + rect.h)) /
                       static_cast<unsigned int>(cameraRaster->parent->height);
                m_vertices[0].x = 0.0f;
                m_vertices[0].y = 0.0f;
                m_vertices[0].u = u[0];
                m_vertices[0].v = v[0];
                m_vertices[1].x = 0.0f;
                m_vertices[1].y = rect.h;
                m_vertices[1].u = u[0];
                m_vertices[1].v = v[1];
                m_vertices[2].x = rect.w;
                m_vertices[2].y = 0.0f;
                m_vertices[2].u = u[1];
                m_vertices[2].v = v[0];
                m_vertices[3].x = rect.w;
                m_vertices[3].y = rect.h;
                m_vertices[3].u = u[1];
                m_vertices[3].v = v[1];
                if (m_parameters.field_0x0 != 0 &&
                    (m_parameters.field_0x3.red != 0xFF || m_parameters.field_0x3.green != 0xFF ||
                     m_parameters.field_0x3.blue != 0xFF || m_parameters.field_0x7.alpha != 0)) {
                    if (fn_804854DC(camera)) {
                        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
                        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
                        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
                        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
                        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERNEAREST);
                        fn_804854B4(camera);
                    }
                    if (m_parameters.field_0x3.red != 0xFF ||
                        m_parameters.field_0x3.green != 0xFF ||
                        m_parameters.field_0x3.blue != 0xFF) {
                        fn_804951B8(m_raster, FALSE);
                        m_vertices[0].emissiveColor.red = m_parameters.field_0x3.red;
                        m_vertices[0].emissiveColor.green = m_parameters.field_0x3.green;
                        m_vertices[0].emissiveColor.blue = m_parameters.field_0x3.blue;
                        m_vertices[0].emissiveColor.alpha = m_parameters.field_0x3.alpha;
                        m_vertices[1].emissiveColor.red = m_parameters.field_0x3.red;
                        m_vertices[1].emissiveColor.green = m_parameters.field_0x3.green;
                        m_vertices[1].emissiveColor.blue = m_parameters.field_0x3.blue;
                        m_vertices[1].emissiveColor.alpha = m_parameters.field_0x3.alpha;
                        m_vertices[2].emissiveColor.red = m_parameters.field_0x3.red;
                        m_vertices[2].emissiveColor.green = m_parameters.field_0x3.green;
                        m_vertices[2].emissiveColor.blue = m_parameters.field_0x3.blue;
                        m_vertices[2].emissiveColor.alpha = m_parameters.field_0x3.alpha;
                        m_vertices[3].emissiveColor.red = m_parameters.field_0x3.red;
                        m_vertices[3].emissiveColor.green = m_parameters.field_0x3.green;
                        m_vertices[3].emissiveColor.blue = m_parameters.field_0x3.blue;
                        m_vertices[3].emissiveColor.alpha = m_parameters.field_0x3.alpha;
                        if (fn_804854DC(camera)) {
                            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, m_raster->parent);
                            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
                            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO);
                            RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
                            RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
                            RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
                            RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
                            RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERNEAREST);
                            RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_vertices, 4);
                            fn_804854B4(camera);
                        }
                    }
                    if (m_parameters.field_0x7.alpha != 0) {
                        m_vertices[0].emissiveColor.red = m_parameters.field_0x7.red;
                        m_vertices[0].emissiveColor.green = m_parameters.field_0x7.green;
                        m_vertices[0].emissiveColor.blue = m_parameters.field_0x7.blue;
                        m_vertices[0].emissiveColor.alpha = m_parameters.field_0x7.alpha;
                        m_vertices[1].emissiveColor.red = m_parameters.field_0x7.red;
                        m_vertices[1].emissiveColor.green = m_parameters.field_0x7.green;
                        m_vertices[1].emissiveColor.blue = m_parameters.field_0x7.blue;
                        m_vertices[1].emissiveColor.alpha = m_parameters.field_0x7.alpha;
                        m_vertices[2].emissiveColor.red = m_parameters.field_0x7.red;
                        m_vertices[2].emissiveColor.green = m_parameters.field_0x7.green;
                        m_vertices[2].emissiveColor.blue = m_parameters.field_0x7.blue;
                        m_vertices[2].emissiveColor.alpha = m_parameters.field_0x7.alpha;
                        m_vertices[3].emissiveColor.red = m_parameters.field_0x7.red;
                        m_vertices[3].emissiveColor.green = m_parameters.field_0x7.green;
                        m_vertices[3].emissiveColor.blue = m_parameters.field_0x7.blue;
                        m_vertices[3].emissiveColor.alpha = m_parameters.field_0x7.alpha;
                        if (fn_804854DC(camera)) {
                            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
                            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
                            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
                            RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_vertices, 4);
                            fn_804854B4(camera);
                        }
                    }
                }
                if (m_parameters.field_0x1 != 0) {
                    m_vertices[0].emissiveColor.red = 0xFF;
                    m_vertices[0].emissiveColor.green = 0xFF;
                    m_vertices[0].emissiveColor.blue = 0xFF;
                    m_vertices[0].emissiveColor.alpha = 0xFF;
                    m_vertices[1].emissiveColor.red = 0xFF;
                    m_vertices[1].emissiveColor.green = 0xFF;
                    m_vertices[1].emissiveColor.blue = 0xFF;
                    m_vertices[1].emissiveColor.alpha = 0xFF;
                    m_vertices[2].emissiveColor.red = 0xFF;
                    m_vertices[2].emissiveColor.green = 0xFF;
                    m_vertices[2].emissiveColor.blue = 0xFF;
                    m_vertices[2].emissiveColor.alpha = 0xFF;
                    m_vertices[3].emissiveColor.red = 0xFF;
                    m_vertices[3].emissiveColor.green = 0xFF;
                    m_vertices[3].emissiveColor.blue = 0xFF;
                    m_vertices[3].emissiveColor.alpha = 0xFF;
                    if (fn_804854DC(camera)) {
                        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
                        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDINVDESTCOLOR);
                        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO);
                        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
                        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
                        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
                        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
                        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERNEAREST);
                        RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_vertices, 4);
                        fn_804854B4(camera);
                    }
                }
                if (fn_804854DC(camera)) {
                    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)srcBlend);
                    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)dstBlend);
                    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)vertexAlpha);
                    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)zTest);
                    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)zWrite);
                    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)fog);
                    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)filter);
                    fn_804854B4(camera);
                }
            }
        }
    }
}

} // namespace RW3
} // namespace Effect
