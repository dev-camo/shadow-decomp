/* RW3 effect rendering routines; the original filename and state type name are unknown. */
#include "Game/fn_80437118.h"
#include "Game/EffectSystemRW3.h"
#include "Game/fn_8042A444.h"
#include <dolphin/gx/GXPixel.h>

namespace Effect {
namespace RW3 {

void EffectManagerRW3::fn_80437C14(RpClump *clump, unsigned char flag, int count,
                                   const RwRGBA &color) {
    if (field_0x4C != 0) {
        field_0x4C->fn_80437118(clump, flag, count, color);
    }
}

void UnkEffectManagerRW3Data4C::fn_804376EC() {
    if (field_0x68 != 0 && field_0x70 != 0) {
        EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
        if (system != 0) {
            m_camera = system->GetCamera();
            if (m_camera != 0) {
                m_cameraRaster = m_camera->frameBuffer;
                RwRenderStateGet(rwRENDERSTATESRCBLEND, &m_srcBlend);
                RwRenderStateGet(rwRENDERSTATEDESTBLEND, &m_dstBlend);
                RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &m_vertexAlpha);
                RwRenderStateGet(rwRENDERSTATEZTESTENABLE, &m_zTest);
                RwRenderStateGet(rwRENDERSTATEZWRITEENABLE, &m_zWrite);
                RwRenderStateGet(rwRENDERSTATEFOGENABLE, &m_fog);
                RwRenderStateGet(rwRENDERSTATETEXTUREFILTER, &m_filter);
                RwRenderStateGet(rwRENDERSTATESHADEMODE, &m_shadeMode);
                RwRenderStateGet(rwRENDERSTATECULLMODE, &m_cullMode);

                RwInt16 xOffset;
                RwInt16 yOffset;
                fn_80489B2C(m_cameraRaster, &xOffset, &yOffset);
                RwRect rect;
                rect.x = xOffset;
                rect.y = yOffset;
                rect.w = m_cameraRaster->width;
                rect.h = m_cameraRaster->height;
                fn_80489DC4(field_0x68, field_0x68->parent, &rect);
                fn_80489DC4(field_0x70, field_0x70->parent, &rect);

                RwRaster *parent = m_cameraRaster->parent;
                unsigned int parentWidth = parent->width;
                unsigned int parentHeight = parent->height;
                float u[2] = {0.0f, 0.0f};
                float v[2] = {0.0f, 0.0f};
                u[0] = (0.5f + static_cast<float>(xOffset)) / parentWidth;
                u[1] = (0.5f + static_cast<float>(xOffset + rect.w)) / parentWidth;
                v[0] = (0.5f + static_cast<float>(yOffset)) / parentHeight;
                v[1] = (0.5f + static_cast<float>(yOffset + rect.h)) / parentHeight;

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

                fn_804951B8(field_0x70, FALSE);
                RwRGBA clearColor = {0, 0, 0, 0};
                fn_804857BC(m_camera, &clearColor, rwCAMERACLEARIMAGE);
                if (fn_804854DC(m_camera) != 0) {
                    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERNEAREST);
                    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)1); /* Flat shading. */
                    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
                    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
                    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
                    RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLBACK);
                    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
                    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
                    fn_804854B4(m_camera);
                }
            }
        }
    }
}

void UnkEffectManagerRW3Data4C::fn_804372A8(unsigned char flag, int count, const RwRGBA &color) {
    if (m_camera != 0 && field_0x68 != 0 && field_0x70 != 0) {
        for (int i = 0; i < 4; ++i) {
            m_vertices[i].emissiveColor.red = 255;
            m_vertices[i].emissiveColor.green = 255;
            m_vertices[i].emissiveColor.blue = 255;
            m_vertices[i].emissiveColor.alpha = 255;
        }
        fn_804951B8(field_0x68, FALSE);
        if (fn_804854DC(m_camera) != 0) {
            RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, field_0x68->parent);
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
            if (field_0x78 == 0) {
                for (int i = count; i != 0; i /= 2) {
                    RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_vertices, 4);
                }
            }
            fn_804854B4(m_camera);
        }
        fn_804951B8(field_0x68, FALSE);
        if (fn_804854DC(m_camera) != 0) {
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO);
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, field_0x70->parent);
            RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_vertices, 4);
            fn_804854B4(m_camera);
        }
        unsigned char alpha = color.alpha;
        unsigned char red = color.red;
        unsigned char green = color.green;
        unsigned char blue = color.blue;
        for (int i = 0; i < 4; ++i) {
            m_vertices[i].emissiveColor.red = red;
            m_vertices[i].emissiveColor.green = green;
            m_vertices[i].emissiveColor.blue = blue;
            m_vertices[i].emissiveColor.alpha = alpha;
        }
        if (fn_804854DC(m_camera) != 0) {
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, field_0x68->parent);
            if (flag != 0 || field_0x78 != 0) {
                RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
                RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
            } else {
                GXSetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ONE, GX_LO_SET);
            }
            RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_vertices, 4);
            // A failed begin-update leaves the earlier state changes unrestored in the original.
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)m_srcBlend);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)m_dstBlend);
            RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)m_vertexAlpha);
            RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)m_zTest);
            RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)m_zWrite);
            RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)m_fog);
            RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)m_filter);
            RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)m_shadeMode);
            RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)m_cullMode);
            fn_804854B4(m_camera);
        }
    }
}

void UnkEffectManagerRW3Data4C::fn_80437118(RpClump *clump, unsigned char flag, int count,
                                            const RwRGBA &color) {
    EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
    if (system == 0) {
        return;
    }
    RwCamera *camera = system->GetCamera();
    if (camera == 0) {
        return;
    }
    fn_804376EC();
    if (fn_804854DC(camera) != 0) {
        fn_80456BC4(clump);
        fn_804854B4(camera);
    }
    if (m_camera != 0 && field_0x68 != 0 && field_0x70 != 0 && fn_804854DC(m_camera) != 0) {
        RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)3); /* Front-face culling. */
        if (field_0x78 != 0) {
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDZERO);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
        } else {
            GXSetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ONE, GX_LO_SET);
        }
        fn_804854B4(m_camera);
    }
    if (fn_804854DC(camera) != 0) {
        fn_80456BC4(clump);
        fn_804854B4(camera);
    }
    fn_804372A8(flag, count, color);
}

} // namespace RW3
} // namespace Effect
