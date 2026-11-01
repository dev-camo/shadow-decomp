/* EffectLocusRW3 methods; original filename unknown. */
#include "Game/EffectLocusRW3.h"
#include "Game/EffectLocusExecUnit.h"
#include "Game/EffectSystemRW3.h"
#include "Game/EffectCommandExec.h"
#include "Game/UnkEffectFileData.h"
#include "Game/fn_8040C874.h"

namespace Effect {
namespace RW3 {

/* Accessed command prefix; original type name and unused field meanings are unknown. */
struct UnkLocusPointData : System::UnkCommandData {
    /* 0x04 */ Fn8040Vector3 position;
    /* These encoded bytes have no identified reader in this unit. */
    /* 0x10 */ unsigned char field_0x10[12];
    /* 0x1C */ RwRGBA color;
};
CHECK_SIZEOF(UnkLocusPointData, 0x20);

EffectLocusRW3::EffectLocusRW3(System::UnkEffectElementData7 *definition,
                               System::EffectLaunchInfo *info, System::EffectManager *manager)
    : System::EffectLocus(definition, info, manager), m_vertices(0) {
    m_size -= sizeof(System::EffectLocus);
    m_size += sizeof(EffectLocusRW3);
    if ((m_flags & System::ELEMENT_ACTIVE) == 0) {
        return;
    }
    m_vertices = static_cast<RwIm3DVertex *>(
        fn_8042A5D0()->Allocate(sizeof(RwIm3DVertex) * (m_capacity * m_pointCount * 2)));
    if (m_vertices == 0) {
        m_flags &= ~System::ELEMENT_ACTIVE;
        return;
    }
    m_size += sizeof(RwIm3DVertex) * m_capacity * m_pointCount * 2;
}

EffectLocusRW3::~EffectLocusRW3() {
    RwIm3DVertex *vertices = m_vertices;
    if (vertices != 0) {
        fn_8042A5D0()->Free(vertices);
        m_vertices = 0;
    }
}

void EffectLocusRW3::Update(float deltaTime) {
    System::EffectLocus::Update(deltaTime);
    if ((m_flags & System::ELEMENT_ACTIVE) == 0) {
        return;
    }
    if ((m_flags & System::ELEMENT_FLAG_4) != 0) {
        return;
    }
    if (m_unitCount != 0 && m_previousUnit != 0 && (m_definitionFlags & 0x4000) == 0) {
        fn_80439B18();
    }
}

void EffectLocusRW3::Render() {
    if ((m_flags & System::ELEMENT_ACTIVE) && !(m_flags & System::ELEMENT_FLAG_8) &&
        m_unitCount != 0 && m_previousUnit) {
        if (m_definitionFlags & 0x4000) {
            fn_80439B18();
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
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
        if (m_definitionFlags & 2) {
            RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
        } else {
            RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)TRUE);
        }

        if (m_capacity <= m_pointCount) {
            int index = 0;
            for (System::EffectLocusExecUnit *unit = m_head; unit; unit = unit->m_next, ++index) {
                if (fn_8049A82C(m_vertices + index * m_pointCount * 2, m_pointCount * 2, NULL,
                                rwIM3D_VERTEXXYZ | rwIM3D_VERTEXRGBA)) {
                    fn_8049AAF8(rwPRIMTYPETRISTRIP);
                }
            }
            fn_8049A90C();
        } else {
            for (int index = 0; index < m_pointCount - 1; ++index) {
                if (fn_8049A82C(m_vertices + index * m_capacity * 2, (m_unitCount + 1) * 2, NULL,
                                rwIM3D_VERTEXXYZ | rwIM3D_VERTEXRGBA)) {
                    fn_8049AAF8(rwPRIMTYPETRISTRIP);
                }
            }
            // The original does not end the Im3D transform in this branch.
        }

        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)fog);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)vertexAlpha);
        RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)cullMode);
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);
    }
}

void EffectLocusRW3::fn_80439B18() {
    const System::UnkEffectElementData7 *definition =
        static_cast<const System::UnkEffectElementData7 *>(m_definition);
    Fn8040Vector3 cameraPosition;
    if ((m_definitionFlags & 0x4000) != 0) {
        EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
        RwCamera *camera = system->GetCamera();
        RwFrame *frame = static_cast<RwFrame *>(camera->object.parent);
        cameraPosition.x = frame->modelling.pos.x;
        cameraPosition.y = frame->modelling.pos.y;
        cameraPosition.z = frame->modelling.pos.z;
    }
    RwRGBAReal baseColor;
    baseColor.red = ((m_color[0] >> System::MATERIAL_FRACTION_BITS) & 0xFF) / 255.0f;
    baseColor.green = ((m_color[1] >> System::MATERIAL_FRACTION_BITS) & 0xFF) / 255.0f;
    baseColor.blue = ((m_color[2] >> System::MATERIAL_FRACTION_BITS) & 0xFF) / 255.0f;
    baseColor.alpha = ((m_alpha >> System::MATERIAL_FRACTION_BITS) & 0xFF) / 255.0f;
    RwRGBAReal topColor;
    topColor.red = ((m_topColor[0] >> System::MATERIAL_FRACTION_BITS) & 0xFF) / 255.0f;
    topColor.green = ((m_topColor[1] >> System::MATERIAL_FRACTION_BITS) & 0xFF) / 255.0f;
    topColor.blue = ((m_topColor[2] >> System::MATERIAL_FRACTION_BITS) & 0xFF) / 255.0f;
    topColor.alpha = ((m_topAlpha >> System::MATERIAL_FRACTION_BITS) & 0xFF) / 255.0f;
    RwRGBAReal lastColor;
    lastColor.red = ((m_lastColor[0] >> System::MATERIAL_FRACTION_BITS) & 0xFF) / 255.0f;
    lastColor.green = ((m_lastColor[1] >> System::MATERIAL_FRACTION_BITS) & 0xFF) / 255.0f;
    lastColor.blue = ((m_lastColor[2] >> System::MATERIAL_FRACTION_BITS) & 0xFF) / 255.0f;
    lastColor.alpha = ((m_lastAlpha >> System::MATERIAL_FRACTION_BITS) & 0xFF) / 255.0f;
    RwRGBAReal color = lastColor;
    int row = 0;
    System::EffectLocusExecUnit *unit = m_head;
    while (unit != 0) {
        if ((m_definitionFlags & 0x100000) != 0) {
            float ratio = static_cast<float>(row) / m_unitCount;
            float inverse = 1.0f - ratio;
            color.red = inverse * lastColor.red + ratio * topColor.red;
            color.green = inverse * lastColor.green + ratio * topColor.green;
            color.blue = inverse * lastColor.blue + ratio * topColor.blue;
            color.alpha = inverse * lastColor.alpha + ratio * topColor.alpha;
        }
        color.red *= baseColor.red;
        color.green *= baseColor.green;
        color.blue *= baseColor.blue;
        color.alpha *= baseColor.alpha;
        bool updatePositions;
        Fn8040Matrix34 matrix;
        if ((m_definitionFlags & 0x800) == 0 && (m_definitionFlags & 0x4000) == 0 &&
            row + m_removedCount <= (m_unitCount - 1) - m_addedCount) {
            updatePositions = false;
        } else {
            updatePositions = true;
            if ((m_definitionFlags & 0x800) != 0) {
                Fn8040Matrix34 transformed;
                fn_8001E044(&transformed, &m_transform, &unit->m_transform);
                fn_8040C148(&matrix, &transformed);
            } else {
                fn_8040C148(&matrix, &unit->m_transform);
            }
            if ((m_definitionFlags & 0x4000) != 0) {
                Fn8040Vector3 reference(1.0f, 0.0f, 0.0f);
                Fn8040Matrix34 inverse;
                fn_8040BEF8(&inverse, &matrix);
                Fn8040Vector3 directionTemporary;
                fn_8040B67C(&directionTemporary, &inverse, &cameraPosition);
                Fn8040Vector3 direction;
                fn_800091D0(&direction, &directionTemporary);
                direction.z = 0.0f;
                fn_8040CE74(&direction);
                Fn8040Vector3 axisTemporary;
                fn_8040CFB8(&axisTemporary, &reference, &direction);
                Fn8040Vector3 axis;
                fn_800091EC(&axis, &axisTemporary);
                if (fn_8040CF50(&axis) > 0.0001f) {
                    fn_8040CE74(&axis);
                    float angle = fn_801B1DB0(fn_8040CF90(&reference, &direction));
                    Fn8040Vector4 rotationTemporary;
                    fn_8040C874(&rotationTemporary, &axis, angle);
                    Fn8040Vector4 rotation;
                    fn_801B1DD4(&rotation, &rotationTemporary);
                    fn_8040B8C0(&matrix, &rotation);
                }
            }
        }
        const System::UnkCommandData *command = definition->field_0x54;
        int pointIndex = 0;
        while (command != 0 && command->type == 0x15) {
            const UnkLocusPointData *point = static_cast<const UnkLocusPointData *>(command);
            int vertexIndex;
            int previousIndex;
            if (m_pointCount < m_capacity) {
                vertexIndex = (row + pointIndex * m_capacity) * 2;
                if (pointIndex < 1) {
                    previousIndex = -1;
                } else {
                    previousIndex = (row + (pointIndex - 1) * m_capacity) * 2 + 1;
                }
            } else {
                vertexIndex = (pointIndex + row * m_pointCount) * 2;
                if (row < 1) {
                    previousIndex = -1;
                } else {
                    previousIndex = (pointIndex + (row - 1) * m_pointCount) * 2 + 1;
                }
            }
            if (updatePositions) {
                Fn8040Vector3 position = point->position;
                Fn8040Vector3 transformed;
                fn_8040B67C(&transformed, &matrix, &position);
                fn_800091D0(&position, &transformed);
                m_vertices[vertexIndex].objVertex.x = position.x;
                m_vertices[vertexIndex].objVertex.y = position.y;
                m_vertices[vertexIndex].objVertex.z = position.z;
            } else {
                int cachedIndex;
                if (m_pointCount < m_capacity) {
                    cachedIndex = (row + m_removedCount + pointIndex * m_capacity) * 2;
                } else {
                    cachedIndex = (pointIndex + (row + m_removedCount) * m_pointCount) * 2;
                }
                m_vertices[vertexIndex] = m_vertices[cachedIndex];
            }
            m_vertices[vertexIndex].color.red = static_cast<int>(color.red * point->color.red);
            m_vertices[vertexIndex].color.green =
                static_cast<int>(color.green * point->color.green);
            m_vertices[vertexIndex].color.blue = static_cast<int>(color.blue * point->color.blue);
            m_vertices[vertexIndex].color.alpha =
                static_cast<int>(color.alpha * point->color.alpha);
            if (previousIndex >= 0) {
                m_vertices[previousIndex] = m_vertices[vertexIndex];
            }
            if (command->hasNext == 0) {
                if ((m_definitionFlags & 0x2000) != 0) {
                    if (m_pointCount < m_capacity) {
                        m_vertices[(row + pointIndex * m_capacity) * 2 + 1] = m_vertices[row * 2];
                    } else {
                        m_vertices[(pointIndex + 1 + row * m_pointCount) * 2] =
                            m_vertices[row * m_pointCount * 2];
                        if (row > 0) {
                            m_vertices[(pointIndex + 1 + (row - 1) * m_pointCount) * 2 + 1] =
                                m_vertices[(row - 1) * m_pointCount * 2 + 1];
                        }
                    }
                }
                break;
            }
            command = reinterpret_cast<const System::UnkCommandData *>(
                reinterpret_cast<const unsigned char *>(command) + sizeof(UnkLocusPointData));
            ++pointIndex;
        }
        System::EffectLocusExecUnit *next = unit->m_next;
        if (next == 0) {
            next = m_previousUnit;
            if (unit == next) {
                next = 0;
            } else {
                color = topColor;
            }
        } else {
            color.red = color.green = color.blue = color.alpha = 1.0f;
        }
        ++row;
        unit = next;
    }
}

} // namespace RW3
} // namespace Effect
