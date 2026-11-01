/* RW3 Shimmer and parameter-state methods; original filename unknown. */
#include "Game/fn_804342CC.h"
#include "Game/EffectShimmerRW3.h"
#include "Game/EffectSystemRW3.h"
#include "Game/UnkEffectFileData.h"
#include "Game/SharedConstants.h"
#include <cmath>
#include <cstring>

namespace Effect {
namespace RW3 {

UnkEffectRW3Param3::UnkEffectRW3Param3(unsigned char field0, unsigned char field1, int field8,
                                       float fieldC, float field10, float field14, float field18,
                                       float field1C, int field20, float field24, float field28,
                                       float field2C, float field30, float field34)
    : field_0x0(field0), field_0x1(field1) {
    field_0x8.field_0x0 = field8;
    field_0x8.field_0x4 = fieldC;
    field_0x8.field_0x8 = field10;
    field_0x8.field_0xC = field14;
    field_0x8.field_0x10 = field18;
    field_0x8.field_0x14 = field1C;
    field_0x20.field_0x0 = field20;
    field_0x20.field_0x4 = field24;
    field_0x20.field_0x8 = field28;
    field_0x20.field_0xC = field2C;
    field_0x20.field_0x10 = field30;
    field_0x20.field_0x14 = field34;
}

void EffectManagerRW3::fn_80437088(const UnkEffectRW3Param3 *parameters) {
    field_0x48 = new UnkEffectManagerRW3Data48(this, parameters);
}

void EffectManagerRW3::fn_80437044() {
    if (field_0x48 != 0) {
        delete field_0x48;
        field_0x48 = 0;
    }
}

void EffectManagerRW3::fn_80437018(float deltaTime) {
    if (field_0x48 != 0) {
        field_0x48->fn_80436128(deltaTime);
    }
}

void EffectManagerRW3::fn_80436FEC() {
    if (field_0x48 != 0) {
        field_0x48->fn_8043597C();
    }
}

void EffectManagerRW3::fn_80436FC0(const UnkEffectRW3Param3 &parameters) {
    if (field_0x48 != 0) {
        field_0x48->fn_80436AFC(parameters);
    }
}

UnkEffectRW3Param3 EffectManagerRW3::fn_80436F2C() {
    if (field_0x48 != 0) {
        return field_0x48->fn_80436A28();
    }
    UnkEffectRW3Param3 result;
    return result;
}

UnkEffectManagerRW3Data48::UnkEffectManagerRW3Data48(EffectManagerRW3 *manager,
                                                     const UnkEffectRW3Param3 *parameters)
    : m_manager(manager) {
    // The original leaves the remaining storage untouched when no camera is available.
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
        m_raster = m_ownedRaster =
            fn_80489E74(0, 0, 0, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT8888);
        fn_80489DC4(m_raster,
                    fn_80489E74(rect.w, rect.h, 0, rwRASTERTYPECAMERATEXTURE | rwRASTERFORMAT8888),
                    &rect);
    }
    float nearZ = RwIm2DGetNearScreenZ();
    for (int i = 0; i < 4; ++i) {
        m_quad[i].z = nearZ;
        m_quad[i].emissiveColor.red = 255;
        m_quad[i].emissiveColor.green = 255;
        m_quad[i].emissiveColor.blue = 255;
        m_quad[i].emissiveColor.alpha = 255;
    }
    field_0xC4 = rect.w;
    field_0xC8 = rect.h;
    m_grid = 0;
    m_dynamicVertices = 0;
    m_rowOffsets = 0;
    m_columnOffsets = 0;
    if (parameters != 0) {
        fn_80436C48(parameters);
    } else {
        m_parameters.field_0x0 = 0;
    }
}

UnkEffectManagerRW3Data48::~UnkEffectManagerRW3Data48() {
    if (m_grid != 0) {
        delete[] m_grid;
        m_grid = 0;
    }
    if (m_dynamicVertices != 0) {
        delete[] m_dynamicVertices;
        m_dynamicVertices = 0;
    }
    if (m_columnOffsets != 0) {
        delete[] m_columnOffsets;
        m_columnOffsets = 0;
    }
    if (m_rowOffsets != 0) {
        delete[] m_rowOffsets;
        m_rowOffsets = 0;
    }
    if (m_ownedRaster != 0) {
        if (m_ownedRaster->parent != m_ownedRaster) {
            fn_80489BDC(m_ownedRaster->parent);
        }
        fn_80489BDC(m_ownedRaster);
        m_ownedRaster = 0;
    }
    m_raster = 0;
}

void UnkEffectManagerRW3Data48::fn_80436C48(const UnkEffectRW3Param3 *parameters) {
    if (parameters != 0) {
        fn_80436AFC(*parameters);
    }
}

void UnkEffectManagerRW3Data48::fn_80436AFC(const UnkEffectRW3Param3 &parameters) {
    UnkEffectRW3Param3 previous(m_parameters);
    m_parameters = parameters;
    if (m_parameters.field_0x20.field_0x0 < 1) {
        m_parameters.field_0x20.field_0x0 = 1;
    }
    if (m_parameters.field_0x8.field_0x0 < 1) {
        m_parameters.field_0x8.field_0x0 = 1;
    }
    if (m_parameters.field_0x0 != 0) {
        if (previous.field_0x8.field_0x0 != m_parameters.field_0x8.field_0x0 ||
            previous.field_0x20.field_0x0 != m_parameters.field_0x20.field_0x0) {
            fn_804367E0();
        } else if (previous.field_0x8.field_0x14 != m_parameters.field_0x8.field_0x14 ||
                   previous.field_0x8.field_0x10 != m_parameters.field_0x8.field_0x10 ||
                   previous.field_0x20.field_0x14 != m_parameters.field_0x20.field_0x14 ||
                   previous.field_0x20.field_0x10 != m_parameters.field_0x20.field_0x10) {
            field_0xD0 = 0;
            field_0xCC = 0;
        }
    }
}

UnkEffectRW3Param3::UnkEffectRW3Param3(const UnkEffectRW3Param3 &other) {
    field_0x0 = other.field_0x0;
    field_0x1 = other.field_0x1;
    std::memcpy(field_0x2, other.field_0x2, sizeof field_0x2);
    field_0x8.field_0x0 = other.field_0x8.field_0x0;
    field_0x8.field_0x4 = other.field_0x8.field_0x4;
    field_0x8.field_0x8 = other.field_0x8.field_0x8;
    field_0x8.field_0xC = other.field_0x8.field_0xC;
    field_0x8.field_0x10 = other.field_0x8.field_0x10;
    field_0x8.field_0x14 = other.field_0x8.field_0x14;
    field_0x20.field_0x0 = other.field_0x20.field_0x0;
    field_0x20.field_0x4 = other.field_0x20.field_0x4;
    field_0x20.field_0x8 = other.field_0x20.field_0x8;
    field_0x20.field_0xC = other.field_0x20.field_0xC;
    field_0x20.field_0x10 = other.field_0x20.field_0x10;
    field_0x20.field_0x14 = other.field_0x20.field_0x14;
}

UnkEffectRW3Param3 UnkEffectManagerRW3Data48::fn_80436A28() {
    return m_parameters;
}

void UnkEffectManagerRW3Data48::fn_804367E0() {
    if (m_grid != NULL) {
        delete[] m_grid;
        m_grid = NULL;
    }
    if (m_dynamicVertices != NULL) {
        delete[] m_dynamicVertices;
        m_dynamicVertices = NULL;
    }
    if (m_columnOffsets != NULL) {
        delete[] m_columnOffsets;
        m_columnOffsets = NULL;
    }
    if (m_rowOffsets != NULL) {
        delete[] m_rowOffsets;
        m_rowOffsets = NULL;
    }
    m_gridCount = (m_parameters.field_0x8.field_0x0 + 1) * (m_parameters.field_0x20.field_0x0 + 1);
    m_grid = new UnkEffectManagerRW3GridPoint[m_gridCount];
    m_vertexCount =
        m_parameters.field_0x20.field_0x0 * ((m_parameters.field_0x8.field_0x0 + 1) * 2);
    m_dynamicVertices = new RwIm2DVertex[m_vertexCount];
    m_columnOffsets = new float[m_parameters.field_0x8.field_0x0 + 1];
    m_rowOffsets = new float[m_parameters.field_0x20.field_0x0 + 1];
    if (m_grid == NULL || m_dynamicVertices == NULL || m_columnOffsets == NULL ||
        m_rowOffsets == NULL) {
        if (m_grid != NULL) {
            delete[] m_grid;
            m_grid = NULL;
        }
        if (m_dynamicVertices != NULL) {
            delete[] m_dynamicVertices;
            m_dynamicVertices = NULL;
        }
        if (m_columnOffsets != NULL) {
            delete[] m_columnOffsets;
            m_columnOffsets = NULL;
        }
        if (m_rowOffsets != NULL) {
            delete[] m_rowOffsets;
            m_rowOffsets = NULL;
        }
    }
    // The original continues after allocation failure and does not check the cast.
    RwIm2DVertex *vertex = m_dynamicVertices;
    EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
    system->GetCamera();
    float nearZ = RwIm2DGetNearScreenZ();
    for (int y = 0; y < m_parameters.field_0x20.field_0x0; ++y) {
        for (int x = 0; x < m_parameters.field_0x8.field_0x0 + 1; ++x) {
            for (int i = 0; i < 2; ++i) {
                vertex->z = nearZ;
                vertex->emissiveColor.red = 0xFF;
                vertex->emissiveColor.green = 0xFF;
                vertex->emissiveColor.blue = 0xFF;
                vertex->emissiveColor.alpha = 0xFF;
                ++vertex;
            }
        }
    }
    field_0xD0 = 0;
    field_0xCC = 0;
    field_0xBC = 0.0f;
    field_0xC0 = 0.0f;
}

void UnkEffectManagerRW3Data48::fn_804364A0() {
    float xScale = static_cast<float>(field_0xC4) / field_0xCC;
    float xRange[2] = {};
    xRange[0] = (m_parameters.field_0x8.field_0x10 - 0.5f) * xScale + 0.5f;
    xRange[1] = (m_parameters.field_0x8.field_0x14 - 0.5f) * xScale + 0.5f;
    for (int i = 0; i < 2; ++i) {
        if (xRange[i] > 1.0f)
            xRange[i] = 1.0f;
        if (xRange[i] < 0.0f)
            xRange[i] = 0.0f;
    }
    float xStep = field_0xCC * (xRange[1] - xRange[0]) / m_parameters.field_0x8.field_0x0;
    float xOrigin = xRange[0] * field_0xCC;
    float yScale = static_cast<float>(field_0xC8) / field_0xD0;
    float yRange[2] = {};
    yRange[0] = (m_parameters.field_0x20.field_0x10 - 0.5f) * yScale + 0.5f;
    yRange[1] = (m_parameters.field_0x20.field_0x14 - 0.5f) * yScale + 0.5f;
    for (int i = 0; i < 2; ++i) {
        if (yRange[i] > 1.0f)
            yRange[i] = 1.0f;
        if (yRange[i] < 0.0f)
            yRange[i] = 0.0f;
    }
    float yStep = field_0xD0 * (yRange[1] - yRange[0]) / m_parameters.field_0x20.field_0x0;
    float yOrigin = yRange[0] * field_0xD0;
    for (int y = 0; y < m_parameters.field_0x20.field_0x0 + 1; ++y) {
        for (int x = 0; x < m_parameters.field_0x8.field_0x0 + 1; ++x) {
            UnkEffectManagerRW3GridPoint *point =
                &m_grid[x + y * (m_parameters.field_0x8.field_0x0 + 1)];
            point->x = xStep * x + xOrigin;
            point->y = yStep * y + yOrigin;
            if (y == 0 || x == 0 || y == m_parameters.field_0x20.field_0x0 ||
                x == m_parameters.field_0x8.field_0x0) {
                point->weight = 0.0f;
            } else {
                point->weight = 1.0f;
            }
        }
    }
}

void UnkEffectManagerRW3Data48::fn_80436128(float deltaTime) {
    if (m_parameters.field_0x0 == 0) {
        return;
    }
    if (m_grid == 0 || m_dynamicVertices == 0 || m_columnOffsets == 0 || m_rowOffsets == 0) {
        fn_804367E0();
        if (m_grid == 0 || m_dynamicVertices == 0 || m_columnOffsets == 0 || m_rowOffsets == 0) {
            return;
        }
    }
    if (field_0xCC == 0 || field_0xD0 == 0) {
        return;
    }
    field_0xC0 += deltaTime;
    field_0xBC += deltaTime;
    // The original does not reject zero or negative periods.
    while (field_0xC0 > m_parameters.field_0x20.field_0x8) {
        field_0xC0 -= m_parameters.field_0x20.field_0x8;
    }
    while (field_0xBC > m_parameters.field_0x8.field_0x8) {
        field_0xBC -= m_parameters.field_0x8.field_0x8;
    }
    float angleScale = 2.0f * lbl_805FAD38;
    float phase = (field_0xBC * angleScale) / m_parameters.field_0x8.field_0x8;
    float step = (field_0xD0 * angleScale) /
                 (m_parameters.field_0x20.field_0x0 * m_parameters.field_0x20.field_0xC);
    for (int row = 0; row < m_parameters.field_0x20.field_0x0 + 1; ++row) {
        m_rowOffsets[row] = m_parameters.field_0x20.field_0x4 * std::sin(row * step + phase);
    }
    phase = (angleScale * field_0xC0) / m_parameters.field_0x20.field_0x8;
    step = (angleScale * field_0xCC) /
           (m_parameters.field_0x8.field_0x0 * m_parameters.field_0x8.field_0xC);
    for (int column = 0; column < m_parameters.field_0x8.field_0x0 + 1; ++column) {
        m_columnOffsets[column] =
            m_parameters.field_0x8.field_0x4 * std::sin(column * step + phase);
    }
    int columns = m_parameters.field_0x8.field_0x0 + 1;
    RwIm2DVertex *vertex = m_dynamicVertices;
    for (int row = 0; row < m_parameters.field_0x20.field_0x0; ++row) {
        for (int column = 0; column < columns; ++column) {
            int x;
            if (row % 2 != 0) {
                x = m_parameters.field_0x8.field_0x0 - column;
            } else {
                x = column;
            }
            float offsetY = m_columnOffsets[x];
            for (int i = 0; i < 2; ++i) {
                int y = row + i;
                UnkEffectManagerRW3GridPoint *point = &m_grid[x + columns * y];
                float weight = point->weight;
                vertex->x = weight * m_rowOffsets[y] + point->x;
                vertex->y = offsetY * weight + point->y;
                ++vertex;
            }
        }
    }
}

void UnkEffectManagerRW3Data48::fn_8043597C() {
    if (m_parameters.field_0x0 == 0 || m_grid == 0 || m_dynamicVertices == 0 ||
        m_columnOffsets == 0 || m_rowOffsets == 0 || m_raster == 0) {
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
    RwRaster *cameraRaster = camera->frameBuffer;
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
    u[1] = (0.5f + (xOffset + rect.w)) / static_cast<unsigned int>(cameraRaster->parent->width);
    v[0] = (0.5f + yOffset) / static_cast<unsigned int>(cameraRaster->parent->height);
    v[1] = (0.5f + (yOffset + rect.h)) / static_cast<unsigned int>(cameraRaster->parent->height);
    for (int i = 0; i < 4; ++i) {
        m_quad[i].x = (i / 2) * rect.w;
        m_quad[i].y = (i % 2) * rect.h;
        m_quad[i].u = u[i / 2];
        m_quad[i].v = v[i % 2];
    }
    if (rect.w != field_0xCC || rect.h != field_0xD0) {
        field_0xCC = rect.w;
        field_0xD0 = rect.h;
        fn_804364A0();
        fn_80436128(0.0f);
    }
    int columns = m_parameters.field_0x8.field_0x0 + 1;
    RwIm2DVertex *vertex = m_dynamicVertices;
    unsigned char alpha = m_parameters.field_0x1;
    for (int row = 0; row < m_parameters.field_0x20.field_0x0; ++row) {
        for (int column = 0; column < columns; ++column) {
            int x = column;
            if (row % 2 != 0) {
                x = m_parameters.field_0x8.field_0x0 - column;
            }
            for (int i = 0; i < 2; ++i) {
                UnkEffectManagerRW3GridPoint *point = &m_grid[x + columns * (row + i)];
                vertex->u = (u[1] - u[0]) * (point->x / field_0xCC) + u[0];
                vertex->v = (v[1] - v[0]) * (point->y / field_0xD0) + v[0];
                vertex->emissiveColor.red = 0xFF;
                vertex->emissiveColor.green = 0xFF;
                vertex->emissiveColor.blue = 0xFF;
                vertex->emissiveColor.alpha = alpha;
                ++vertex;
            }
        }
    }
    RwInt32 srcBlend;
    RwInt32 dstBlend;
    RwInt32 filter;
    RwBool vertexAlpha;
    RwBool zTest;
    RwBool zWrite;
    RwBool fog;
    RwInt32 cullMode;
    RwRenderStateGet(rwRENDERSTATESRCBLEND, &srcBlend);
    RwRenderStateGet(rwRENDERSTATEDESTBLEND, &dstBlend);
    RwRenderStateGet(rwRENDERSTATETEXTUREFILTER, &filter);
    RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &vertexAlpha);
    RwRenderStateGet(rwRENDERSTATEZTESTENABLE, &zTest);
    RwRenderStateGet(rwRENDERSTATEZWRITEENABLE, &zWrite);
    RwRenderStateGet(rwRENDERSTATEFOGENABLE, &fog);
    RwRenderStateGet(rwRENDERSTATECULLMODE, &cullMode);
    fn_804951B8(m_raster->parent, FALSE);
    if (fn_804854DC(camera)) {
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, m_raster->parent);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERNEAREST);
        RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
        RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_dynamicVertices, m_vertexCount);
        fn_804854B4(camera);
    }
    if (fn_804854DC(camera)) {
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)srcBlend);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)dstBlend);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)vertexAlpha);
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)zTest);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)zWrite);
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)fog);
        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)filter);
        RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)cullMode);
        fn_804854B4(camera);
    }
}

EffectShimmerRW3::EffectShimmerRW3(System::UnkEffectElementData10 *definition,
                                   System::EffectLaunchInfo *info, System::EffectManager *manager)
    : System::EffectShimmer(definition, info, manager), m_vertices(0), m_grid(0), m_offsetsX(0),
      m_offsetsY(0) {
    m_size -= sizeof(System::EffectShimmer);
    m_size += sizeof(EffectShimmerRW3);
    if ((m_flags & System::ELEMENT_ACTIVE) == 0) {
        return;
    }
    EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
    if (system == 0) {
        return;
    }
    if (system->GetCamera() == 0) {
        return;
    }
    float nearZ = RwIm2DGetNearScreenZ();
    m_quad[0].z = nearZ;
    m_quad[1].z = nearZ;
    m_quad[2].z = nearZ;
    m_quad[3].z = nearZ;
    m_quad[0].emissiveColor.red = 0xFF;
    m_quad[0].emissiveColor.green = 0xFF;
    m_quad[0].emissiveColor.blue = 0xFF;
    m_quad[0].emissiveColor.alpha = 0xFF;
    m_quad[1].emissiveColor.red = 0xFF;
    m_quad[1].emissiveColor.green = 0xFF;
    m_quad[1].emissiveColor.blue = 0xFF;
    m_quad[1].emissiveColor.alpha = 0xFF;
    m_quad[2].emissiveColor.red = 0xFF;
    m_quad[2].emissiveColor.green = 0xFF;
    m_quad[2].emissiveColor.blue = 0xFF;
    m_quad[2].emissiveColor.alpha = 0xFF;
    m_quad[3].emissiveColor.red = 0xFF;
    m_quad[3].emissiveColor.green = 0xFF;
    m_quad[3].emissiveColor.blue = 0xFF;
    m_quad[3].emissiveColor.alpha = 0xFF;
    m_phaseX = m_phaseY = 0.0f;
    m_gridCount = (field_0x108[0] + 1) * (field_0x108[1] + 1);
    m_grid = static_cast<RwV3d *>(system->Allocate(sizeof(RwV3d) * m_gridCount));
    if (m_grid == 0) {
        m_flags &= ~System::ELEMENT_ACTIVE;
        return;
    }
    m_size += sizeof(RwV3d) * m_gridCount;
    float y;
    float stepX = field_0x110[0] / field_0x108[0];
    float stepY = field_0x110[1] / field_0x108[1];
    for (int row = 0; row < field_0x108[1] + 1; ++row) {
        y = stepY * row;
        for (int column = 0; column < field_0x108[0] + 1; ++column) {
            RwV3d *point = &m_grid[column + row * (field_0x108[0] + 1)];
            point->x = stepX * column - 0.5f * field_0x110[0];
            point->y = y - 0.5f * field_0x110[1];
            point->z = 0.0f;
        }
    }
    m_vertexCount = field_0x108[1] * ((field_0x108[0] + 1) * 2);
    if (field_0x108[1] > 1) {
        m_vertexCount -= field_0x108[1] - 1;
    }
    m_vertices =
        static_cast<RwIm2DVertex *>(system->Allocate(sizeof(RwIm2DVertex) * m_vertexCount));
    if (m_vertices == 0) {
        m_flags &= ~System::ELEMENT_ACTIVE;
        return;
    }
    m_size += sizeof(RwIm2DVertex) * m_vertexCount;
    for (int i = 0; i < m_vertexCount; ++i) {
        m_vertices[i].z = nearZ;
        m_vertices[i].emissiveColor.red = 0xFF;
        m_vertices[i].emissiveColor.green = 0xFF;
        m_vertices[i].emissiveColor.blue = 0xFF;
        m_vertices[i].emissiveColor.alpha = 0xFF;
    }
    switch (m_parameters->type) {
    case 0:
        // The original adds one byte, although Update writes one additional float.
        m_offsetsX = static_cast<float *>(system->Allocate(sizeof(float) * field_0x108[0] + 1));
        if (m_offsetsX == 0) {
            m_flags &= ~System::ELEMENT_ACTIVE;
            return;
        }
        m_size += sizeof(float) * field_0x108[0] + 1;
        m_offsetsY = static_cast<float *>(system->Allocate(sizeof(float) * field_0x108[1] + 1));
        if (m_offsetsY == 0) {
            m_flags &= ~System::ELEMENT_ACTIVE;
            return;
        }
        m_size += sizeof(float) * field_0x108[1] + 1;
        break;
    }
}

EffectShimmerRW3::~EffectShimmerRW3() {
    EffectSystemRW3 *system = dynamic_cast<EffectSystemRW3 *>(fn_8042A5D0());
    if (system != 0) {
        if (m_grid != 0) {
            system->Free(m_grid);
        }
        if (m_vertices != 0) {
            system->Free(m_vertices);
        }
        if (m_offsetsX != 0) {
            system->Free(m_offsetsX);
        }
        if (m_offsetsY != 0) {
            system->Free(m_offsetsY);
        }
    }
}

void EffectShimmerRW3::Update(float deltaTime) {
    System::EffectShimmer::Update(deltaTime);
    if ((m_flags & System::ELEMENT_ACTIVE) == 0 || (m_flags & System::ELEMENT_FLAG_4) != 0 ||
        m_grid == 0 || m_vertices == 0) {
        return;
    }
    float twoPi = 2.0f * lbl_805FAD38;
    switch (m_parameters->type) {
    case 0: {
        m_phaseX += deltaTime / 60.0f;
        m_phaseY += deltaTime / 60.0f;
        if (m_cycle.x > 0.0001f) {
            while (m_phaseX > m_cycle.x) {
                m_phaseX -= m_cycle.x;
            }
        }
        if (m_cycle.y > 0.0001f) {
            while (m_phaseY > m_cycle.y) {
                m_phaseY -= m_cycle.y;
            }
        }
        float stepX = twoPi * field_0x110[0] / (field_0x108[0] * m_waveLength.x);
        float phaseX = twoPi * m_phaseX / m_cycle.x;
        float stepY = twoPi * field_0x110[1] / (field_0x108[1] * m_waveLength.y);
        float phaseY = twoPi * m_phaseY / m_cycle.y;
        for (int i = 0; i < field_0x108[0] + 1; ++i) {
            m_offsetsX[i] = m_amplitude.x * std::sin(i * stepX + phaseX);
        }
        for (int i = 0; i < field_0x108[1] + 1; ++i) {
            m_offsetsY[i] = m_amplitude.y * std::sin(i * stepY + phaseY);
        }
        break;
    }
    case 1:
        m_phaseX += deltaTime / 60.0f;
        if (field_0x13C == 0 && m_cycle.x > 0.0001f) {
            while (m_phaseX > m_cycle.x) {
                m_phaseX -= m_cycle.x;
            }
        }
        break;
    }
}

void EffectShimmerRW3::Render() {
    if ((m_flags & System::ELEMENT_ACTIVE) == 0 || (m_flags & System::ELEMENT_FLAG_8) != 0 ||
        m_grid == 0 || m_vertices == 0) {
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
    RwRaster *cameraRaster = camera->frameBuffer;
    EffectManagerRW3 *manager = dynamic_cast<EffectManagerRW3 *>(m_manager);
    RwRaster *raster = manager->field_0x34;
    if (raster == 0) {
        return;
    }
    RwRaster *parent = cameraRaster->parent;
    if (parent == 0) {
        return;
    }
    RwV3d position = {0.0f, 0.0f, 0.0f};
    position.x = m_previousTranslation.x;
    position.y = m_previousTranslation.y;
    position.z = m_previousTranslation.z;
    fn_80482458(&position, &position, &camera->viewMatrix);
    float nearPlane = camera->nearPlane;
    float farPlane = camera->farPlane;
    if (position.z < nearPlane || position.z > farPlane) {
        return;
    }
    RwInt16 offsetX;
    RwInt16 offsetY;
    fn_80489B2C(cameraRaster, &offsetX, &offsetY);
    RwRect rect;
    rect.x = offsetX;
    rect.y = offsetY;
    rect.w = cameraRaster->width;
    rect.h = cameraRaster->height;
    fn_80489DC4(raster, raster->parent, &rect);
    RwV2d uvX = {0.0f, 0.0f};
    RwV2d uvY = {0.0f, 0.0f};
    RwV2d screenPosition = {0.0f, 0.0f};
    uvX.x = (offsetX + 0.5f) / static_cast<unsigned int>(parent->width);
    uvX.y = (offsetX + rect.w + 0.5f) / static_cast<unsigned int>(parent->width);
    uvY.x = (offsetY + 0.5f) / static_cast<unsigned int>(parent->height);
    uvY.y = (offsetY + rect.h + 0.5f) / static_cast<unsigned int>(parent->height);
    float width = static_cast<float>(rect.w);
    float height = static_cast<float>(rect.h);
    float reciprocalZ = 1.0f / position.z;
    screenPosition.x = reciprocalZ * (position.x * rect.w);
    screenPosition.y = reciprocalZ * (position.y * rect.h);
    float nearZ = RwIm2DGetNearScreenZ();
    float farZ = lbl_805F15B4->dOpenDevice.zBufferFar;
    float depth = (farPlane / position.z) *
                      ((position.z - nearPlane) * ((farZ - nearZ) / (farPlane - nearPlane))) +
                  nearZ;
    int largest = rect.h;
    if (rect.w > largest) {
        largest = rect.w;
    }
    float scale = reciprocalZ * largest;
    float spanU = uvX.y - uvX.x;
    float spanV = uvY.y - uvY.x;
    for (int i = 0; i < 4; ++i) {
        int corner;
        switch (i) {
        case 0:
            corner = 0;
            break;
        case 1:
            corner = field_0x108[1] * (field_0x108[0] + 1);
            break;
        case 2:
            corner = field_0x108[0];
            break;
        case 3:
            corner = m_gridCount - 1;
            break;
        }
        float x = scale * m_grid[corner].x + screenPosition.x;
        float y = scale * m_grid[corner].y + screenPosition.y;
        if (x < 0.0f) {
            x = 0.0f;
        }
        if (x > width) {
            x = width;
        }
        if (y < 0.0f) {
            y = 0.0f;
        }
        if (y > height) {
            y = height;
        }
        m_quad[i].x = x;
        m_quad[i].y = y;
        m_quad[i].u = spanU * (x / rect.w) + uvX.x;
        m_quad[i].v = spanV * (y / rect.h) + uvY.x;
    }
    const System::UnkEffectElementData10 *definition =
        static_cast<const System::UnkEffectElementData10 *>(m_definition);
    unsigned char alpha = definition->field_0x30;
    float twoPi = 2.0f * lbl_805FAD38;
    switch (m_parameters->type) {
    case 0: {
        float outsideU = uvX.x + uvX.y;
        float outsideV = uvY.x + uvY.y;
        for (int row = 0; row < field_0x108[1]; ++row) {
            for (int column = 0; column < field_0x108[0] + 1; ++column) {
                int gridColumn = column;
                if (row % 2 != 0) {
                    gridColumn = field_0x108[0] - column;
                }
                float waveX = m_offsetsX[gridColumn] * scale;
                for (int side = 0; side < 2; ++side) {
                    if (row != 0 && column == 0 && side == 0) {
                        continue;
                    }
                    int gridIndex = gridColumn + (field_0x108[0] + 1) * (row + side);
                    int vertexIndex = column * 2 + row * (field_0x108[0] + 1) * 2 + side - row;
                    float nx = m_grid[gridIndex].x / (0.5f * field_0x110[0]);
                    float ny = m_grid[gridIndex].y / (0.5f * field_0x110[1]);
                    float falloff = (1.0f - nx * nx) * (1.0f - ny * ny);
                    float waveY = m_offsetsY[row + side];
                    float x = scale * m_grid[gridIndex].x + screenPosition.x;
                    if (x < 0.0f) {
                        m_vertices[vertexIndex].u = uvX.x;
                        m_vertices[vertexIndex].x = 0.0f;
                    } else if (x > rect.w) {
                        m_vertices[vertexIndex].u = outsideU;
                        m_vertices[vertexIndex].x = width;
                    } else {
                        m_vertices[vertexIndex].u = spanU * (x / rect.w) + uvX.x;
                        m_vertices[vertexIndex].x = falloff * (waveY * scale) + x;
                    }
                    float y = scale * m_grid[gridIndex].y + screenPosition.y;
                    if (y < 0.0f) {
                        m_vertices[vertexIndex].v = uvY.x;
                        m_vertices[vertexIndex].y = 0.0f;
                    } else if (y > rect.h) {
                        m_vertices[vertexIndex].v = outsideV;
                        m_vertices[vertexIndex].y = height;
                    } else {
                        m_vertices[vertexIndex].v = spanV * (y / rect.h) + uvY.x;
                        m_vertices[vertexIndex].y = falloff * waveX + y;
                    }
                    m_vertices[vertexIndex].z = depth;
                    m_vertices[vertexIndex].emissiveColor.red = 0xFF;
                    m_vertices[vertexIndex].emissiveColor.green = 0xFF;
                    m_vertices[vertexIndex].emissiveColor.blue = 0xFF;
                    m_vertices[vertexIndex].emissiveColor.alpha = alpha;
                }
            }
        }
        break;
    }
    case 1: {
        float phase = twoPi * m_phaseX / m_cycle.x;
        float frequency = twoPi / m_waveLength.x;
        float repetitions = static_cast<float>(field_0x13C);
        bool allExpired = true;
        float outsideU = uvX.x + uvX.y;
        float outsideV = uvY.x + uvY.y;
        for (int row = 0; row < field_0x108[1]; ++row) {
            for (int column = 0; column < field_0x108[0] + 1; ++column) {
                int gridColumn = column;
                if (row % 2 != 0) {
                    gridColumn = field_0x108[0] - column;
                }
                for (int side = 0; side < 2; ++side) {
                    if (row != 0 && column == 0 && side == 0) {
                        continue;
                    }
                    int gridIndex = gridColumn + (field_0x108[0] + 1) * (row + side);
                    int vertexIndex = column * 2 + row * (field_0x108[0] + 1) * 2 + side - row;
                    float px = m_grid[gridIndex].x;
                    float py = m_grid[gridIndex].y;
                    float squareX = px * px;
                    float squareY = py * py;
                    float radius = std::sqrt(squareX + squareY);
                    float displacement;
                    if (field_0x13C == 0) {
                        displacement = m_amplitude.x * std::sin(radius * frequency + phase);
                        allExpired = false;
                    } else {
                        float progress =
                            (m_phaseX - radius * m_cycle.x / m_waveLength.x) / m_cycle.x;
                        if (progress > repetitions) {
                            displacement = 0.0f;
                        } else if (progress < 0.0f) {
                            displacement = 0.0f;
                            allExpired = false;
                        } else {
                            displacement = m_amplitude.x * std::sin(twoPi * progress);
                            allExpired = false;
                        }
                    }
                    float radialScale;
                    if (radius > 0.0001f) {
                        radialScale = displacement / radius;
                    } else {
                        radialScale = 0.0f;
                    }
                    float nx = m_grid[gridIndex].x / (0.5f * field_0x110[0]);
                    float ny = m_grid[gridIndex].y / (0.5f * field_0x110[1]);
                    float yFalloff = 1.0f - ny * ny;
                    float falloff = (1.0f - nx * nx) * yFalloff;
                    float x = scale * m_grid[gridIndex].x + screenPosition.x;
                    if (x < 0.0f) {
                        m_vertices[vertexIndex].u = uvX.x;
                        m_vertices[vertexIndex].x = 0.0f;
                    } else if (x > rect.w) {
                        m_vertices[vertexIndex].u = outsideU;
                        m_vertices[vertexIndex].x = width;
                    } else {
                        m_vertices[vertexIndex].u = spanU * (x / rect.w) + uvX.x;
                        // The radial path applies the vertical falloff twice.
                        m_vertices[vertexIndex].x =
                            falloff * (yFalloff * (scale * (radialScale * m_grid[gridIndex].x))) +
                            x;
                    }
                    float y = scale * m_grid[gridIndex].y + screenPosition.y;
                    if (y < 0.0f) {
                        m_vertices[vertexIndex].v = uvY.x;
                        m_vertices[vertexIndex].y = 0.0f;
                    } else if (y > rect.h) {
                        m_vertices[vertexIndex].v = outsideV;
                        m_vertices[vertexIndex].y = height;
                    } else {
                        m_vertices[vertexIndex].v = spanV * (y / rect.h) + uvY.x;
                        m_vertices[vertexIndex].y =
                            falloff * (yFalloff * (scale * (radialScale * m_grid[gridIndex].y))) +
                            y;
                    }
                    m_vertices[vertexIndex].z = depth;
                    m_vertices[vertexIndex].emissiveColor.red = 0xFF;
                    m_vertices[vertexIndex].emissiveColor.green = 0xFF;
                    m_vertices[vertexIndex].emissiveColor.blue = 0xFF;
                    m_vertices[vertexIndex].emissiveColor.alpha = alpha;
                }
            }
        }
        if (allExpired) {
            m_flags &= ~System::ELEMENT_ACTIVE;
            return;
        }
        break;
    }
    }
    fn_804951B8(raster->parent, FALSE);
    if (fn_804854DC(camera) != 0) {
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, raster->parent);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERNEAREST);
        RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_vertices, m_vertexCount);
        fn_804854B4(camera);
    }
}

} // namespace RW3
} // namespace Effect
