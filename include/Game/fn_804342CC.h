#ifndef GAME_FN_804342CC_H
#define GAME_FN_804342CC_H

#include "Game/EffectManagerRW3.h"
#include "Game/UnkEffectRW3Parameters.h"
#include "rw/rwcore.h"
#include "layout_check.h"

namespace Effect {
namespace RW3 {

/* Original type name unknown; the third component scales displacement at grid boundaries. */
struct UnkEffectManagerRW3GridPoint {
    /* 0x00 */ float x;
    /* 0x04 */ float y;
    /* 0x08 */ float weight;
};
CHECK_SIZEOF(UnkEffectManagerRW3GridPoint, 0xC);

/* Original type name unknown; layout and member names are inferred from allocation and use. */
struct UnkEffectManagerRW3Data48 {
    UnkEffectManagerRW3Data48(EffectManagerRW3 *manager, const UnkEffectRW3Param3 *parameters);
    ~UnkEffectManagerRW3Data48();

    void fn_8043597C();
    void fn_80436128(float deltaTime);
    void fn_804364A0();
    void fn_804367E0();
    UnkEffectRW3Param3 fn_80436A28();
    void fn_80436AFC(const UnkEffectRW3Param3 &parameters);
    void fn_80436C48(const UnkEffectRW3Param3 *parameters);

    /* 0x00 */ EffectManagerRW3 *m_manager;
    /* 0x04 */ RwRaster *m_raster;
    /* 0x08 */ RwRaster *m_ownedRaster;
    /* 0x0C */ UnkEffectRW3Param3 m_parameters;
    /* 0x44 */ RwIm2DVertex m_quad[4];
    /* 0xA4 */ RwIm2DVertex *m_dynamicVertices;
    /* 0xA8 */ UnkEffectManagerRW3GridPoint *m_grid;
    /* 0xAC */ float *m_rowOffsets;
    /* 0xB0 */ float *m_columnOffsets;
    /* 0xB4 */ int m_vertexCount;
    /* 0xB8 */ int m_gridCount;
    /* 0xBC */ float field_0xBC;
    /* 0xC0 */ float field_0xC0;
    /* 0xC4 */ int field_0xC4;
    /* 0xC8 */ int field_0xC8;
    /* 0xCC */ int field_0xCC;
    /* 0xD0 */ int field_0xD0;
};
CHECK_SIZEOF(UnkEffectManagerRW3Data48, 0xD4);

} // namespace RW3
} // namespace Effect

#endif
