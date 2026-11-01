/* RW3 lens-flare and parameter-state methods; original filename unknown. */
#include "Game/fn_80430A54.h"
#include "Game/EffectLensFlareRW3.h"
#include "Game/EffectSystemRW3.h"
#include "Game/UnkEffectFileData.h"

namespace Effect {
namespace RW3 {

UnkEffectRW3Param4::UnkEffectRW3Param4(unsigned char field0, unsigned char field1,
                                       unsigned char field2)
    : field_0x0(field0), field_0x1(field1), field_0x2(field2) {
    RwV3d position = {0.0f, 0.0f, 0.0f};
    m_position = position;
}

void EffectManagerRW3::fn_804322E0(const UnkEffectRW3Param4 *parameters) {
    field_0x50 = new UnkEffectManagerRW3Data50(this, parameters);
}

void EffectManagerRW3::fn_80432278() {
    if (field_0x50 != 0) {
        delete field_0x50;
        field_0x50 = 0;
    }
}

void EffectManagerRW3::fn_8043224C() {
    if (field_0x50 != 0) {
        field_0x50->fn_80431310();
    }
}

void EffectManagerRW3::fn_80432220(unsigned char flag) {
    if (field_0x50 != 0) {
        field_0x50->fn_80430DE8(flag);
    }
}

void EffectManagerRW3::fn_804321F4(const UnkEffectRW3Param4 &parameters) {
    if (field_0x50 != 0) {
        field_0x50->fn_80431F6C(parameters);
    }
}

UnkEffectRW3Param4 EffectManagerRW3::fn_80432174() {
    if (field_0x50 != 0) {
        return field_0x50->fn_80431F38();
    }
    UnkEffectRW3Param4 result;
    return result;
}

void EffectManagerRW3::fn_80432148(RwTexDictionary *dictionary,
                                   const UnkEffectManagerRW3Data50Item *items, int count) {
    if (field_0x50 != 0) {
        field_0x50->fn_80431D18(dictionary, items, count);
    }
}

UnkEffectManagerRW3Data50::UnkEffectManagerRW3Data50(EffectManagerRW3 *manager,
                                                     const UnkEffectRW3Param4 *parameters)
    : m_manager(manager), m_textures(0), m_vertices(0) {
    EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
    if (system == 0) {
        return;
    }
    if (system->GetCamera() == 0) {
        return;
    }
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
    if (parameters != 0) {
        fn_80431FA0(parameters);
    } else {
        m_parameters.field_0x0 = 0;
        m_parameters.m_position.x = 0.0f;
        m_parameters.m_position.y = 1000.0f;
        m_parameters.m_position.z = 0.0f;
        m_parameters.field_0x1 = 0xFF;
        m_parameters.field_0x2 = 0x20;
    }
}

void UnkEffectManagerRW3Data50::fn_80431FA0(const UnkEffectRW3Param4 *parameters) {
    if (parameters != 0) {
        m_parameters = *parameters;
    }
}

void UnkEffectManagerRW3Data50::fn_80431F6C(const UnkEffectRW3Param4 &parameters) {
    m_parameters = parameters;
}

UnkEffectRW3Param4 UnkEffectManagerRW3Data50::fn_80431F38() {
    return m_parameters;
}

void UnkEffectManagerRW3Data50::fn_80431D18(RwTexDictionary *dictionary,
                                            const UnkEffectManagerRW3Data50Item *items, int count) {
    m_items = items;
    m_dictionary = dictionary;
    if (m_dictionary == 0 || m_items == 0) {
        // The original clears the count but leaves both deleted array pointers unchanged.
        delete[] m_vertices;
        delete[] m_textures;
        m_count = 0;
    } else {
        m_count = count;
        EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
        if (system != 0 && system->GetCamera() != 0) {
            m_vertices = new RwIm2DVertex[count * 4];
            m_textures = new RwTexture *[count];
            for (int i = 0; i < m_count; ++i) {
                if (m_items[i].m_name != 0) {
                    m_textures[i] = fn_8048CFFC(m_dictionary, m_items[i].m_name);
                    m_vertices[i * 4].u = m_items[i].m_u[0];
                    m_vertices[i * 4].v = m_items[i].m_v[0];
                    m_vertices[i * 4].z = RwIm2DGetNearScreenZ();
                    m_vertices[i * 4 + 1].u = m_items[i].m_u[0];
                    m_vertices[i * 4 + 1].v = m_items[i].m_v[1];
                    m_vertices[i * 4 + 1].z = RwIm2DGetNearScreenZ();
                    m_vertices[i * 4 + 2].u = m_items[i].m_u[1];
                    m_vertices[i * 4 + 2].v = m_items[i].m_v[0];
                    m_vertices[i * 4 + 2].z = RwIm2DGetNearScreenZ();
                    m_vertices[i * 4 + 3].u = m_items[i].m_u[1];
                    m_vertices[i * 4 + 3].v = m_items[i].m_v[1];
                    m_vertices[i * 4 + 3].z = RwIm2DGetNearScreenZ();
                }
            }
        }
    }
}

void UnkEffectManagerRW3Data50::fn_80431310() {
    field_0x8C = 1;
    field_0x8D = 1;
    EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
    // Zero count still reaches the original first-item processing below.
    if (system == 0 || m_parameters.field_0x0 == 0 || m_vertices == 0 || m_count < 0) {
        return;
    }
    RwCamera *camera = system->GetCamera();
    if (camera == 0) {
        return;
    }
    RwV3d position;
    fn_80482458(&position, &m_parameters.m_position, &camera->viewMatrix);
    float nearPlane = camera->nearPlane;
    float farPlane = camera->farPlane;
    if (position.z < nearPlane) {
        return;
    }
    RwV2d center = {0.0f, 0.0f};
    RwV2d offset = {0.0f, 0.0f};
    float width = camera->frameBuffer->width;
    float height = camera->frameBuffer->height;
    center.x = width * 0.5f;
    center.y = height * 0.5f;
    offset.x = width * (position.x / position.z) - center.x;
    offset.y = height * (position.y / position.z) - center.y;
    float extent = 0.5f * (width > height ? width : height);
    if (!(offset.x > extent || offset.x < -extent || offset.y > extent || offset.y < -extent)) {
        field_0x8D = 0;
        float brightness = (extent - fn_804823F0(&offset)) / extent;
        if (brightness < 0.0f) {
            brightness = 0.0f;
        }
        brightness *= m_parameters.field_0x1 / 255.0f;
        for (int i = 1; i < m_count; ++i) {
            float x = offset.x * m_items[i].field_0x8 + center.x;
            float y = offset.y * m_items[i].field_0x8 + center.y;
            m_vertices[i * 4 + 0].x = -(m_items[i].field_0x6 * 0.5f - x);
            m_vertices[i * 4 + 0].y = -(m_items[i].field_0x4 * 0.5f - y);
            m_vertices[i * 4 + 0].emissiveColor.red = m_items[i].field_0x1C[0];
            m_vertices[i * 4 + 0].emissiveColor.green = m_items[i].field_0x1C[2];
            m_vertices[i * 4 + 0].emissiveColor.blue = m_items[i].field_0x1C[1];
            m_vertices[i * 4 + 0].emissiveColor.alpha = brightness * m_items[i].field_0x1C[3];
            m_vertices[i * 4 + 1].x = -(m_items[i].field_0x6 * 0.5f - x);
            m_vertices[i * 4 + 1].y = m_items[i].field_0x4 * 0.5f + y;
            m_vertices[i * 4 + 1].emissiveColor.red = m_items[i].field_0x1C[0];
            m_vertices[i * 4 + 1].emissiveColor.green = m_items[i].field_0x1C[2];
            m_vertices[i * 4 + 1].emissiveColor.blue = m_items[i].field_0x1C[1];
            m_vertices[i * 4 + 1].emissiveColor.alpha = brightness * m_items[i].field_0x1C[3];
            m_vertices[i * 4 + 2].x = m_items[i].field_0x6 * 0.5f + x;
            m_vertices[i * 4 + 2].y = -(m_items[i].field_0x4 * 0.5f - y);
            m_vertices[i * 4 + 2].emissiveColor.red = m_items[i].field_0x1C[0];
            m_vertices[i * 4 + 2].emissiveColor.green = m_items[i].field_0x1C[2];
            m_vertices[i * 4 + 2].emissiveColor.blue = m_items[i].field_0x1C[1];
            m_vertices[i * 4 + 2].emissiveColor.alpha = brightness * m_items[i].field_0x1C[3];
            m_vertices[i * 4 + 3].x = m_items[i].field_0x6 * 0.5f + x;
            m_vertices[i * 4 + 3].y = m_items[i].field_0x4 * 0.5f + y;
            m_vertices[i * 4 + 3].emissiveColor.red = m_items[i].field_0x1C[0];
            m_vertices[i * 4 + 3].emissiveColor.green = m_items[i].field_0x1C[2];
            m_vertices[i * 4 + 3].emissiveColor.blue = m_items[i].field_0x1C[1];
            m_vertices[i * 4 + 3].emissiveColor.alpha = brightness * m_items[i].field_0x1C[3];
        }
        float color = m_parameters.field_0x2 * brightness;
        m_quad[0].emissiveColor.red = color;
        m_quad[0].emissiveColor.green = color;
        m_quad[0].emissiveColor.blue = color;
        m_quad[0].emissiveColor.alpha = 0xFF;
        m_quad[1].emissiveColor.red = color;
        m_quad[1].emissiveColor.green = color;
        m_quad[1].emissiveColor.blue = color;
        m_quad[1].emissiveColor.alpha = 0xFF;
        m_quad[2].emissiveColor.red = color;
        m_quad[2].emissiveColor.green = color;
        m_quad[2].emissiveColor.blue = color;
        m_quad[2].emissiveColor.alpha = 0xFF;
        m_quad[3].emissiveColor.red = color;
        m_quad[3].emissiveColor.green = color;
        m_quad[3].emissiveColor.blue = color;
        m_quad[3].emissiveColor.alpha = 0xFF;
    }
    if (!(offset.x > width * 1.5f || offset.x < -width * 0.5f || offset.y > height * 1.5f ||
          offset.y < -height * 0.5f)) {
        field_0x8C = 0;
        float nearZ = RwIm2DGetNearScreenZ();
        float farZ = lbl_805F15B4->dOpenDevice.zBufferFar;
        float x = m_items[0].field_0x8 * offset.x + center.x;
        float y = m_items[0].field_0x8 * offset.y + center.y;
        float depthOffset = ((farZ - nearZ) / (farPlane - nearPlane)) * (position.z - nearPlane);
        float projectedDepth = (farPlane / position.z) * depthOffset;
        float depth = nearZ + projectedDepth;
        m_vertices[0].x = -(m_items[0].field_0x6 * 0.5f - x);
        m_vertices[0].y = -(m_items[0].field_0x4 * 0.5f - y);
        m_vertices[0].z = depth;
        m_vertices[0].emissiveColor.red = m_items[0].field_0x1C[0];
        m_vertices[0].emissiveColor.green = m_items[0].field_0x1C[2];
        m_vertices[0].emissiveColor.blue = m_items[0].field_0x1C[1];
        m_vertices[0].emissiveColor.alpha = m_items[0].field_0x1C[3];
        m_vertices[1].x = -(m_items[0].field_0x6 * 0.5f - x);
        m_vertices[1].y = m_items[0].field_0x4 * 0.5f + y;
        m_vertices[1].z = depth;
        m_vertices[1].emissiveColor.red = m_items[0].field_0x1C[0];
        m_vertices[1].emissiveColor.green = m_items[0].field_0x1C[2];
        m_vertices[1].emissiveColor.blue = m_items[0].field_0x1C[1];
        m_vertices[1].emissiveColor.alpha = m_items[0].field_0x1C[3];
        m_vertices[2].x = m_items[0].field_0x6 * 0.5f + x;
        m_vertices[2].y = -(m_items[0].field_0x4 * 0.5f - y);
        m_vertices[2].z = depth;
        m_vertices[2].emissiveColor.red = m_items[0].field_0x1C[0];
        m_vertices[2].emissiveColor.green = m_items[0].field_0x1C[2];
        m_vertices[2].emissiveColor.blue = m_items[0].field_0x1C[1];
        m_vertices[2].emissiveColor.alpha = m_items[0].field_0x1C[3];
        m_vertices[3].x = m_items[0].field_0x6 * 0.5f + x;
        m_vertices[3].y = m_items[0].field_0x4 * 0.5f + y;
        m_vertices[3].z = depth;
        m_vertices[3].emissiveColor.red = m_items[0].field_0x1C[0];
        m_vertices[3].emissiveColor.green = m_items[0].field_0x1C[2];
        m_vertices[3].emissiveColor.blue = m_items[0].field_0x1C[1];
        m_vertices[3].emissiveColor.alpha = m_items[0].field_0x1C[3];
    }
    m_quad[0].x = 0.0f;
    m_quad[0].y = 0.0f;
    m_quad[1].x = 0.0f;
    m_quad[1].y = height;
    m_quad[2].x = width;
    m_quad[2].y = 0.0f;
    m_quad[3].x = width;
    m_quad[3].y = height;
}

void UnkEffectManagerRW3Data50::fn_80430DE8(unsigned char flag) {
    EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
    if (system == 0) {
        return;
    }
    if (m_parameters.field_0x0 == 0) {
        return;
    }
    RwCamera *camera = system->GetCamera();
    if (camera == 0) {
        return;
    }
    if (field_0x8C != 0 && field_0x8D != 0) {
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
    if (fn_804854DC(camera)) {
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERNEAREST);
        if (field_0x8C == 0 && flag != m_items[0].field_0x20) {
            RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, m_textures[0]->raster);
            RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_vertices, 4);
        }
        if (field_0x8D == 0) {
            RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
            for (int i = 1; i < m_count; ++i) {
                if (flag == m_items[i].field_0x20) {
                    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, m_textures[i]->raster);
                    RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, &m_vertices[i * 4], 4);
                }
            }
        }
        fn_804854B4(camera);
    }
    if (fn_804854DC(camera)) {
        if (field_0x8D == 0 && flag == 0) {
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
            RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_quad, 4);
        }
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

EffectLensFlareRW3::EffectLensFlareRW3(System::UnkEffectElementData9 *definition,
                                       System::EffectLaunchInfo *info,
                                       System::EffectManager *manager)
    : System::EffectLensFlare(definition, info, manager) {
    m_size -= sizeof(System::EffectLensFlare);
    m_size += sizeof(EffectLensFlareRW3);
    if ((m_flags & System::ELEMENT_ACTIVE) == 0) {
        return;
    }
    EffectManagerRW3 *rwManager = dynamic_cast<EffectManagerRW3 *>(m_manager);
    if (rwManager != 0) {
        m_initial = rwManager->fn_80432174();
        if ((m_definitionFlags & 0x10000) != 0) {
            m_target.field_0x0 = 1;
        } else {
            m_target.field_0x0 = 0;
        }
        m_target.field_0x1 = definition->field_0x30;
        m_target.field_0x2 = definition->field_0x31;
        m_target.m_position.x = definition->field_0x38;
        m_target.m_position.y = definition->field_0x3C;
        m_target.m_position.z = definition->field_0x40;
    }
}

EffectLensFlareRW3::~EffectLensFlareRW3() {}

void EffectLensFlareRW3::Update(float deltaTime) {
    System::EffectLensFlare::Update(deltaTime);
    if ((m_flags & System::ELEMENT_ACTIVE) == 0) {
        return;
    }
    if ((m_flags & System::ELEMENT_FLAG_4) != 0) {
        return;
    }
    EffectManagerRW3 *manager = dynamic_cast<EffectManagerRW3 *>(m_manager);
    if (manager == 0) {
        return;
    }
    UnkEffectRW3Param4 result = m_target;
    if (m_progress < 0.9999f) {
        if (result.field_0x0 || m_initial.field_0x0) {
            result.field_0x0 = 1;
        } else {
            result.field_0x0 = 0;
        }
        if (result.field_0x1 != m_initial.field_0x1) {
            result.field_0x1 = static_cast<int>(result.field_0x1 * m_progress +
                                                m_initial.field_0x1 * (1.0f - m_progress));
        }
        if (result.field_0x2 != m_initial.field_0x2) {
            result.field_0x2 = static_cast<int>(result.field_0x2 * m_progress +
                                                m_initial.field_0x2 * (1.0f - m_progress));
        }
        result.m_position.x =
            result.m_position.x * m_progress + m_initial.m_position.x * (1.0f - m_progress);
        result.m_position.y =
            result.m_position.y * m_progress + m_initial.m_position.y * (1.0f - m_progress);
        result.m_position.z =
            result.m_position.z * m_progress + m_initial.m_position.z * (1.0f - m_progress);
    }
    manager->fn_804321F4(result);
}

} // namespace RW3
} // namespace Effect
