/* RW3 raster capture and compositing methods; original filename and class name unknown. */
#include "Game/EffectManagerRW3.h"
#include "Game/EffectSystemRW3.h"
#include "Game/UnkEffectRW3Parameters.h"

namespace Effect {
namespace RW3 {

/* Record and method names are placeholders. Member names, types and qualifiers are inferred
 * from native allocation, callers and field operations; the record has no identified RTTI. */
struct UnkEffectManagerRW3Data40 {
    UnkEffectManagerRW3Data40(EffectManagerRW3 *manager, const UnkEffectRW3Param1 *parameters);
    ~UnkEffectManagerRW3Data40() {
        if (m_raster != 0) {
            if (m_raster->parent != m_raster) {
                fn_80489BDC(m_raster->parent);
            }
            fn_80489BDC(m_raster);
            m_raster = 0;
        }
    }

    void fn_8042D204();
    void fn_8042D7E8();
    UnkEffectRW3Param1 fn_8042D974();
    void fn_8042D9A0(const UnkEffectRW3Param1 &parameters);
    void fn_8042D9CC(const UnkEffectRW3Param1 *parameters);

    /* 0x000 */ UnkEffectRW3Param1 m_parameters;
    /* 0x010 */ EffectManagerRW3 *m_manager;
    /* 0x014 */ RwIm2DVertex m_quad[4];
    /* 0x074 */ RwRaster *m_raster;
    /* 0x078 */ unsigned char field_0x78;
    /* 0x07C */ RwIm2DVertex m_halfQuad[4];
    /* 0x0DC */ RwIm2DVertex m_fullQuad[4];

  private:
    /* Copying the owned raster has not been recovered; this interface prevents shallow copies. */
    UnkEffectManagerRW3Data40(const UnkEffectManagerRW3Data40 &);
    UnkEffectManagerRW3Data40 &operator=(const UnkEffectManagerRW3Data40 &);
};
CHECK_SIZEOF(UnkEffectManagerRW3Data40, 0x13C);

void EffectManagerRW3::fn_8042E1BC(const UnkEffectRW3Param1 *parameters) {
    field_0x40 = new UnkEffectManagerRW3Data40(this, parameters);
}

void EffectManagerRW3::fn_8042E148() {
    if (field_0x40 != 0) {
        delete field_0x40;
        field_0x40 = 0;
    }
}

void EffectManagerRW3::fn_8042E11C() {
    if (field_0x40 != 0) {
        field_0x40->fn_8042D204();
    }
}

void EffectManagerRW3::fn_8042E0F0() {
    if (field_0x40 != 0) {
        field_0x40->fn_8042D7E8();
    }
}

void EffectManagerRW3::fn_8042E0C4(const UnkEffectRW3Param1 *parameters) {
    if (field_0x40 != 0) {
        field_0x40->fn_8042D9CC(parameters);
    }
}

void EffectManagerRW3::fn_8042E098(const UnkEffectRW3Param1 &parameters) {
    if (field_0x40 != 0) {
        field_0x40->fn_8042D9A0(parameters);
    }
}

UnkEffectRW3Param1 EffectManagerRW3::fn_8042E034() {
    if (field_0x40 != 0) {
        return field_0x40->fn_8042D974();
    }
    UnkEffectRW3Param1 result;
    return result;
}

UnkEffectManagerRW3Data40::UnkEffectManagerRW3Data40(EffectManagerRW3 *manager,
                                                     const UnkEffectRW3Param1 *parameters)
    : m_manager(manager) {
    EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
    if (system == 0) {
        return;
    }
    RwCamera *camera = system->GetCamera();
    if (camera == 0) {
        return;
    }
    // The native early returns leave the remaining record storage untouched.
    m_quad[0].z = RwIm2DGetNearScreenZ();
    m_quad[0].emissiveColor.red = 0xFF;
    m_quad[0].emissiveColor.green = 0xFF;
    m_quad[0].emissiveColor.blue = 0xFF;
    m_quad[0].emissiveColor.alpha = 0xFF;
    m_quad[1].z = RwIm2DGetNearScreenZ();
    m_quad[1].emissiveColor.red = 0xFF;
    m_quad[1].emissiveColor.green = 0xFF;
    m_quad[1].emissiveColor.blue = 0xFF;
    m_quad[1].emissiveColor.alpha = 0xFF;
    m_quad[2].z = RwIm2DGetNearScreenZ();
    m_quad[2].emissiveColor.red = 0xFF;
    m_quad[2].emissiveColor.green = 0xFF;
    m_quad[2].emissiveColor.blue = 0xFF;
    m_quad[2].emissiveColor.alpha = 0xFF;
    m_quad[3].z = RwIm2DGetNearScreenZ();
    m_quad[3].emissiveColor.red = 0xFF;
    m_quad[3].emissiveColor.green = 0xFF;
    m_quad[3].emissiveColor.blue = 0xFF;
    m_quad[3].emissiveColor.alpha = 0xFF;
    m_raster = 0;
    field_0x78 = 0;

    RwRaster *parent = camera->frameBuffer->parent;
    unsigned int width = parent->width;
    unsigned int height = parent->height;
    RwRect rect = {0, 0, width, height};
    float u[2] = {0.5f / width, (0.5f + rect.w) / width};
    float v[2] = {0.5f / height, (0.5f + rect.h) / height};

    /* Native construction copies the base vertices before replacing the destination
     * coordinates and UVs; only the base depth and colors have been initialized here. */
    m_fullQuad[0].x = m_quad[0].x;
    m_fullQuad[0].y = m_quad[0].y;
    m_fullQuad[0].z = m_quad[0].z;
    m_fullQuad[0].emissiveColor.red = m_quad[0].emissiveColor.red;
    m_fullQuad[0].emissiveColor.green = m_quad[0].emissiveColor.green;
    m_fullQuad[0].emissiveColor.blue = m_quad[0].emissiveColor.blue;
    m_fullQuad[0].emissiveColor.alpha = m_quad[0].emissiveColor.alpha;
    m_fullQuad[0].u = m_quad[0].u;
    m_fullQuad[0].v = m_quad[0].v;
    m_fullQuad[1].x = m_quad[1].x;
    m_fullQuad[1].y = m_quad[1].y;
    m_fullQuad[1].z = m_quad[1].z;
    m_fullQuad[1].emissiveColor.red = m_quad[1].emissiveColor.red;
    m_fullQuad[1].emissiveColor.green = m_quad[1].emissiveColor.green;
    m_fullQuad[1].emissiveColor.blue = m_quad[1].emissiveColor.blue;
    m_fullQuad[1].emissiveColor.alpha = m_quad[1].emissiveColor.alpha;
    m_fullQuad[1].u = m_quad[1].u;
    m_fullQuad[1].v = m_quad[1].v;
    m_fullQuad[2].x = m_quad[2].x;
    m_fullQuad[2].y = m_quad[2].y;
    m_fullQuad[2].z = m_quad[2].z;
    m_fullQuad[2].emissiveColor.red = m_quad[2].emissiveColor.red;
    m_fullQuad[2].emissiveColor.green = m_quad[2].emissiveColor.green;
    m_fullQuad[2].emissiveColor.blue = m_quad[2].emissiveColor.blue;
    m_fullQuad[2].emissiveColor.alpha = m_quad[2].emissiveColor.alpha;
    m_fullQuad[2].u = m_quad[2].u;
    m_fullQuad[2].v = m_quad[2].v;
    m_fullQuad[3].x = m_quad[3].x;
    m_fullQuad[3].y = m_quad[3].y;
    m_fullQuad[3].z = m_quad[3].z;
    m_fullQuad[3].emissiveColor.red = m_quad[3].emissiveColor.red;
    m_fullQuad[3].emissiveColor.green = m_quad[3].emissiveColor.green;
    m_fullQuad[3].emissiveColor.blue = m_quad[3].emissiveColor.blue;
    m_fullQuad[3].emissiveColor.alpha = m_quad[3].emissiveColor.alpha;
    m_fullQuad[3].u = m_quad[3].u;
    m_fullQuad[3].v = m_quad[3].v;
    m_fullQuad[0].x = 0.0f;
    m_fullQuad[0].y = 0.0f;
    m_fullQuad[0].u = u[0];
    m_fullQuad[0].v = v[0];
    m_fullQuad[1].x = 0.0f;
    m_fullQuad[1].y = rect.h;
    m_fullQuad[1].u = u[0];
    m_fullQuad[1].v = v[1];
    m_fullQuad[2].x = rect.w;
    m_fullQuad[2].y = 0.0f;
    m_fullQuad[2].u = u[1];
    m_fullQuad[2].v = v[0];
    m_fullQuad[3].x = rect.w;
    m_fullQuad[3].y = rect.h;
    m_fullQuad[3].u = u[1];
    m_fullQuad[3].v = v[1];

    rect.w /= 2;
    rect.h /= 2;
    m_halfQuad[0].x = m_fullQuad[0].x;
    m_halfQuad[0].y = m_fullQuad[0].y;
    m_halfQuad[0].z = m_fullQuad[0].z;
    m_halfQuad[0].emissiveColor.red = m_fullQuad[0].emissiveColor.red;
    m_halfQuad[0].emissiveColor.green = m_fullQuad[0].emissiveColor.green;
    m_halfQuad[0].emissiveColor.blue = m_fullQuad[0].emissiveColor.blue;
    m_halfQuad[0].emissiveColor.alpha = m_fullQuad[0].emissiveColor.alpha;
    m_halfQuad[0].u = m_fullQuad[0].u;
    m_halfQuad[0].v = m_fullQuad[0].v;
    m_halfQuad[1].x = m_fullQuad[1].x;
    m_halfQuad[1].y = m_fullQuad[1].y;
    m_halfQuad[1].z = m_fullQuad[1].z;
    m_halfQuad[1].emissiveColor.red = m_fullQuad[1].emissiveColor.red;
    m_halfQuad[1].emissiveColor.green = m_fullQuad[1].emissiveColor.green;
    m_halfQuad[1].emissiveColor.blue = m_fullQuad[1].emissiveColor.blue;
    m_halfQuad[1].emissiveColor.alpha = m_fullQuad[1].emissiveColor.alpha;
    m_halfQuad[1].u = m_fullQuad[1].u;
    m_halfQuad[1].v = m_fullQuad[1].v;
    m_halfQuad[2].x = m_fullQuad[2].x;
    m_halfQuad[2].y = m_fullQuad[2].y;
    m_halfQuad[2].z = m_fullQuad[2].z;
    m_halfQuad[2].emissiveColor.red = m_fullQuad[2].emissiveColor.red;
    m_halfQuad[2].emissiveColor.green = m_fullQuad[2].emissiveColor.green;
    m_halfQuad[2].emissiveColor.blue = m_fullQuad[2].emissiveColor.blue;
    m_halfQuad[2].emissiveColor.alpha = m_fullQuad[2].emissiveColor.alpha;
    m_halfQuad[2].u = m_fullQuad[2].u;
    m_halfQuad[2].v = m_fullQuad[2].v;
    m_halfQuad[3].x = m_fullQuad[3].x;
    m_halfQuad[3].y = m_fullQuad[3].y;
    m_halfQuad[3].z = m_fullQuad[3].z;
    m_halfQuad[3].emissiveColor.red = m_fullQuad[3].emissiveColor.red;
    m_halfQuad[3].emissiveColor.green = m_fullQuad[3].emissiveColor.green;
    m_halfQuad[3].emissiveColor.blue = m_fullQuad[3].emissiveColor.blue;
    m_halfQuad[3].emissiveColor.alpha = m_fullQuad[3].emissiveColor.alpha;
    m_halfQuad[3].u = m_fullQuad[3].u;
    m_halfQuad[3].v = m_fullQuad[3].v;
    m_halfQuad[0].x = 0.0f;
    m_halfQuad[0].y = 0.0f;
    m_halfQuad[1].x = 0.0f;
    m_halfQuad[1].y = rect.h;
    m_halfQuad[2].x = rect.w;
    m_halfQuad[2].y = 0.0f;
    m_halfQuad[3].x = rect.w;
    m_halfQuad[3].y = rect.h;

    m_raster = fn_80489E74(0, 0, 0, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT8888);
    fn_80489DC4(m_raster,
                fn_80489E74(rect.w, rect.h, 0, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT8888),
                &rect);
    if (parameters != 0) {
        fn_8042D9CC(parameters);
    } else {
        m_parameters.field_0x0 = 0;
        m_parameters.field_0x1 = 0x80;
        m_parameters.field_0x4 = 1.0f;
    }
}

void UnkEffectManagerRW3Data40::fn_8042D9CC(const UnkEffectRW3Param1 *parameters) {
    if (parameters != 0) {
        m_parameters = *parameters;
    }
}

void UnkEffectManagerRW3Data40::fn_8042D9A0(const UnkEffectRW3Param1 &parameters) {
    m_parameters = parameters;
}

UnkEffectRW3Param1 UnkEffectManagerRW3Data40::fn_8042D974() {
    return m_parameters;
}

void UnkEffectManagerRW3Data40::fn_8042D7E8() {
    field_0x78 = 0;
    if (m_parameters.field_0x0 == 0) {
        return;
    }
    if (m_parameters.field_0x1 == 0) {
        return;
    }
    EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
    if (system == 0) {
        return;
    }
    RwCamera *camera = system->GetCamera();
    if (camera == 0) {
        return;
    }
    RwRaster *raster = m_manager->field_0x34->parent;
    fn_804951B8(raster, FALSE);
    if (fn_804854DC(camera)) {
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO);
        RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_halfQuad, 4);
        fn_804854B4(camera);
    }
    fn_804951B8(m_raster, FALSE);
    if (fn_804854DC(camera)) {
        RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_fullQuad, 4);
        fn_804854B4(camera);
    }
    // The native flag is set even when either camera update fails.
    field_0x78 = 1;
}

void UnkEffectManagerRW3Data40::fn_8042D204() {
    if (m_parameters.field_0x0 == 0) {
        return;
    }
    if (m_parameters.field_0x1 == 0) {
        return;
    }
    EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
    if (system == 0) {
        return;
    }
    RwCamera *camera = system->GetCamera();
    if (camera == 0) {
        return;
    }
    if (field_0x78 == 0) {
        return;
    }
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

    RwRaster *raster = camera->frameBuffer;
    RwInt16 xOffset;
    RwInt16 yOffset;
    fn_80489B2C(raster, &xOffset, &yOffset);
    int width = raster->width;
    int height = raster->height;
    unsigned int parentWidth = raster->parent->width;
    unsigned int parentHeight = raster->parent->height;
    float u[2] = {(0.5f + xOffset) / parentWidth, (0.5f + (xOffset + width)) / parentWidth};
    float v[2] = {(0.5f + yOffset) / parentHeight, (0.5f + (yOffset + height)) / parentHeight};
    m_quad[0].u = u[0];
    m_quad[0].v = v[0];
    m_quad[1].u = u[0];
    m_quad[1].v = v[1];
    m_quad[2].u = u[1];
    m_quad[2].v = v[0];
    m_quad[3].u = u[1];
    m_quad[3].v = v[1];
    m_quad[0].emissiveColor.red = 0xFF;
    m_quad[0].emissiveColor.green = 0xFF;
    m_quad[0].emissiveColor.blue = 0xFF;
    m_quad[0].emissiveColor.alpha = m_parameters.field_0x1;
    m_quad[1].emissiveColor.red = 0xFF;
    m_quad[1].emissiveColor.green = 0xFF;
    m_quad[1].emissiveColor.blue = 0xFF;
    m_quad[1].emissiveColor.alpha = m_parameters.field_0x1;
    m_quad[2].emissiveColor.red = 0xFF;
    m_quad[2].emissiveColor.green = 0xFF;
    m_quad[2].emissiveColor.blue = 0xFF;
    m_quad[2].emissiveColor.alpha = m_parameters.field_0x1;
    m_quad[3].emissiveColor.red = 0xFF;
    m_quad[3].emissiveColor.green = 0xFF;
    m_quad[3].emissiveColor.blue = 0xFF;
    m_quad[3].emissiveColor.alpha = m_parameters.field_0x1;
    float xExpansion = 0.5f * ((m_parameters.field_0x4 - 1.0f) * width);
    float yExpansion = 0.5f * ((m_parameters.field_0x4 - 1.0f) * height);
    m_quad[0].x = -xExpansion;
    m_quad[0].y = -yExpansion;
    m_quad[1].x = -xExpansion;
    m_quad[1].y = height + yExpansion;
    m_quad[2].x = width + xExpansion;
    m_quad[2].y = -yExpansion;
    m_quad[3].x = width + xExpansion;
    m_quad[3].y = height + yExpansion;

    RwRaster *parent = m_raster->parent;
    if (fn_804854DC(camera)) {
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, parent);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERNEAREST);
        RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_quad, 4);
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

} // namespace RW3
} // namespace Effect
