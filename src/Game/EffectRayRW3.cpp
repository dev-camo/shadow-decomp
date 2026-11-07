/* EffectRayRW3 methods; class identity comes from RTTI, original filename unknown. */
#include "Game/EffectRayRW3.h"
#include "Game/EffectRayExecUnit.h"
#include "Game/EffectCommandExec.h"
#include "Game/EffectSystemRW3.h"
#include "rw/rwcore.h"

namespace Effect {
namespace RW3 {

EffectRayRW3::EffectRayRW3(System::UnkEffectElementData6 *definition,
                           System::EffectLaunchInfo *info, System::EffectManager *manager)
    : System::EffectRay(definition, info, manager), m_vertices(0) {
    m_size -= sizeof(System::EffectRay);
    m_size += sizeof(EffectRayRW3);
    if ((m_flags & System::ELEMENT_ACTIVE) == 0) {
        return;
    }
    m_vertices = new RwIm3DVertex[m_capacity][4];
    if (m_vertices == 0) {
        m_flags &= ~System::ELEMENT_ACTIVE;
        return;
    }
    m_size += sizeof(*m_vertices) * m_capacity;
}

EffectRayRW3::~EffectRayRW3() {
    if (m_vertices != 0) {
        delete[] m_vertices;
        m_vertices = 0;
    }
}

void EffectRayRW3::Update(float deltaTime) {
    System::EffectRay::Update(deltaTime);
}

void EffectRayRW3::Render() {
    if ((m_flags & System::ELEMENT_ACTIVE) != 0 && (m_flags & System::ELEMENT_FLAG_8) == 0 &&
        m_count != 0) {
        EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
        RwCamera *camera = system->GetCamera();
        RwFrame *frame = static_cast<RwFrame *>(camera->object.parent);
        Fn8040Vector3 cameraPosition(frame->modelling.pos.x, frame->modelling.pos.y,
                                     frame->modelling.pos.z);
        System::EffectRayExecUnit *ray = m_head;
        /* Both degenerate paths leave the slice unwritten; the later draw loop still submits it. */
        for (int i = 0; i < m_capacity; ++i, ray = ray->m_next) {
            if (ray == 0) {
                break;
            }
            Fn8040Vector3 position;
            Fn8040Vector3 endpoint;
            Fn8040Vector3 direction;
            Fn8040Vector3 cameraDirection;
            position = ray->m_position;
            direction = position - ray->m_origin;
            float length = fn_8040CF50(&direction);
            if (length < 0.0001f) {
                continue;
            }
            if (length < ray->m_length) {
                endpoint = ray->m_origin;
            } else {
                endpoint = position - direction * (ray->m_length / length);
            }
            if ((m_definitionFlags & 0x800) != 0) {
                position = m_transform * position;
                endpoint = m_transform * endpoint;
            }
            direction = position - endpoint;
            cameraDirection = endpoint - cameraPosition;
            Fn8040Vector3 side = fn_8040CFB8(direction, cameraDirection);
            if (fn_8040CF50(&side) < 0.0001f) {
                cameraDirection = position - cameraPosition;
                side = fn_8040CFB8(direction, cameraDirection);
                if (fn_8040CF50(&side) < 0.0001f) {
                    continue;
                }
            }
            fn_8040CE74(&side);
            fn_8040CE38(&side, 0.5f * ray->m_width);

            float redScale = ((ray->m_color[0] >> System::MATERIAL_FRACTION_BITS) & 0xFF) / 255.0f;
            float greenScale =
                ((ray->m_color[1] >> System::MATERIAL_FRACTION_BITS) & 0xFF) / 255.0f;
            float blueScale = ((ray->m_color[2] >> System::MATERIAL_FRACTION_BITS) & 0xFF) / 255.0f;
            float alphaScale = ((ray->m_alpha >> System::MATERIAL_FRACTION_BITS) & 0xFF) / 255.0f;
            int topRed = static_cast<int>(
                ((ray->m_topColor[0] >> System::MATERIAL_FRACTION_BITS) & 0xFF) * redScale);
            int topGreen = static_cast<int>(
                ((ray->m_topColor[1] >> System::MATERIAL_FRACTION_BITS) & 0xFF) * greenScale);
            int topBlue = static_cast<int>(
                ((ray->m_topColor[2] >> System::MATERIAL_FRACTION_BITS) & 0xFF) * blueScale);
            int topAlpha = static_cast<int>(
                ((ray->m_topAlpha >> System::MATERIAL_FRACTION_BITS) & 0xFF) * alphaScale);
            m_vertices[i][0].objVertex.x = position.x + side.x;
            m_vertices[i][0].objVertex.y = position.y + side.y;
            m_vertices[i][0].objVertex.z = position.z + side.z;
            m_vertices[i][1].objVertex.x = position.x - side.x;
            m_vertices[i][1].objVertex.y = position.y - side.y;
            m_vertices[i][1].objVertex.z = position.z - side.z;
            m_vertices[i][0].color.red = topRed;
            m_vertices[i][0].color.green = topGreen;
            m_vertices[i][0].color.blue = topBlue;
            m_vertices[i][0].color.alpha = topAlpha;
            m_vertices[i][1].color.red = topRed;
            m_vertices[i][1].color.green = topGreen;
            m_vertices[i][1].color.blue = topBlue;
            m_vertices[i][1].color.alpha = topAlpha;

            int lastRed = static_cast<int>(
                ((ray->m_lastColor[0] >> System::MATERIAL_FRACTION_BITS) & 0xFF) * redScale);
            int lastGreen = static_cast<int>(
                ((ray->m_lastColor[1] >> System::MATERIAL_FRACTION_BITS) & 0xFF) * greenScale);
            int lastBlue = static_cast<int>(
                ((ray->m_lastColor[2] >> System::MATERIAL_FRACTION_BITS) & 0xFF) * blueScale);
            int lastAlpha = static_cast<int>(
                ((ray->m_lastAlpha >> System::MATERIAL_FRACTION_BITS) & 0xFF) * alphaScale);
            m_vertices[i][2].objVertex.x = endpoint.x + side.x;
            m_vertices[i][2].objVertex.y = endpoint.y + side.y;
            m_vertices[i][2].objVertex.z = endpoint.z + side.z;
            m_vertices[i][3].objVertex.x = endpoint.x - side.x;
            m_vertices[i][3].objVertex.y = endpoint.y - side.y;
            m_vertices[i][3].objVertex.z = endpoint.z - side.z;
            m_vertices[i][2].color.red = lastRed;
            m_vertices[i][2].color.green = lastGreen;
            m_vertices[i][2].color.blue = lastBlue;
            m_vertices[i][2].color.alpha = lastAlpha;
            m_vertices[i][3].color.red = lastRed;
            m_vertices[i][3].color.green = lastGreen;
            m_vertices[i][3].color.blue = lastBlue;
            m_vertices[i][3].color.alpha = lastAlpha;
        }

        RwInt32 cullMode;
        RwBool vertexAlpha;
        RwBool fog;
        RwRaster *raster;
        RwRenderStateGet(rwRENDERSTATECULLMODE, &cullMode);
        RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &vertexAlpha);
        RwRenderStateGet(rwRENDERSTATEFOGENABLE, &fog);
        RwRenderStateGet(rwRENDERSTATETEXTURERASTER, &raster);
        RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
        if ((m_definitionFlags & 2) != 0) {
            RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
        } else {
            RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)TRUE);
        }
        ray = m_head;
        for (int i = 0; i < m_capacity; ++i, ray = ray->m_next) {
            if (ray == 0) {
                break;
            }
            if (fn_8049A82C(m_vertices[i], 4, 0, rwIM3D_VERTEXXYZ | rwIM3D_VERTEXRGBA) != 0) {
                fn_8049AAF8(rwPRIMTYPETRISTRIP);
            }
        }
        fn_8049A90C();
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)fog);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)vertexAlpha);
        RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)cullMode);
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);
    }
}

} // namespace RW3
} // namespace Effect
