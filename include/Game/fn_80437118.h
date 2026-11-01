#ifndef GAME_FN_80437118_H
#define GAME_FN_80437118_H

#include "Game/EffectManagerRW3.h"
#include "Game/UnkEffectRW3Parameters.h"
#include "rw/rpworld.h"
#include "layout_check.h"

namespace Effect {
namespace RW3 {

/* Accessed prefix only; the original type name and full allocation size are unknown.
 * Member names describe observed uses. Uninterpreted bytes have no assigned field semantics. */
struct UnkEffectManagerRW3Data4C {
    void fn_80437118(RpClump *clump, unsigned char flag, int count, const RwRGBA &color);
    void fn_804372A8(unsigned char flag, int count, const RwRGBA &color);
    void fn_804376EC();

    /* 0x00 */ unsigned char field_0x0[4];
    /* 0x04 */ RwCamera *m_camera;
    /* 0x08 */ RwIm2DVertex m_vertices[4];
    /* 0x68 */ RwRaster *field_0x68;
    /* 0x6C */ unsigned char field_0x6C[4];
    /* 0x70 */ RwRaster *field_0x70;
    /* 0x74 */ unsigned char field_0x74[4];
    /* 0x78 */ unsigned char field_0x78;
    /* 0x7C */ RwRaster *m_cameraRaster;
    /* 0x80 */ RwBool m_vertexAlpha;
    /* 0x84 */ RwBool m_zTest;
    /* 0x88 */ RwBool m_zWrite;
    /* 0x8C */ RwBool m_fog;
    /* 0x90 */ RwInt32 m_srcBlend;
    /* 0x94 */ RwInt32 m_dstBlend;
    /* 0x98 */ RwInt32 m_filter;
    /* 0x9C */ RwInt32 m_shadeMode;
    /* 0xA0 */ RwInt32 m_cullMode;
};
CHECK_SIZEOF(UnkEffectManagerRW3Data4C, 0xA4);

/* Original type name unknown; allocation and member accesses establish the full extent. */
struct UnkEffectManagerRW3Data44 {
    UnkEffectManagerRW3Data44(EffectManagerRW3 *manager, const UnkEffectRW3Param2 *parameters);
    ~UnkEffectManagerRW3Data44() {
        if (m_ownedRaster != 0) {
            if (m_ownedRaster->parent != m_ownedRaster) {
                fn_80489BDC(m_ownedRaster->parent);
            }
            fn_80489BDC(m_ownedRaster);
            m_ownedRaster = 0;
        }
        m_raster = 0;
    }

    void fn_80437C40();
    UnkEffectRW3Param2 fn_80438658();
    void fn_80438684(const UnkEffectRW3Param2 &parameters);
    void fn_804386B0(const UnkEffectRW3Param2 *parameters);

    /* 0x00 */ UnkEffectRW3Param2 m_parameters;
    /* 0x0C */ EffectManagerRW3 *m_manager;
    /* 0x10 */ RwRaster *m_raster;
    /* 0x14 */ RwRaster *m_ownedRaster;
    /* 0x18 */ RwIm2DVertex m_vertices[4];
};
CHECK_SIZEOF(UnkEffectManagerRW3Data44, 0x78);

} // namespace RW3
} // namespace Effect

#endif
