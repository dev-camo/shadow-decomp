#ifndef GAME_FN_80430A54_H
#define GAME_FN_80430A54_H

#include "Game/EffectManagerRW3.h"
#include "Game/UnkEffectRW3Parameters.h"
#include "rw/rwcore.h"
#include "layout_check.h"

namespace Effect {
namespace RW3 {

/* Original type names are unknown; layouts and member names are inferred from use. */
struct UnkEffectManagerRW3Data50Item {
    /* 0x00 */ const char *m_name;
    /* 0x04 */ short field_0x4;
    /* 0x06 */ short field_0x6;
    /* 0x08 */ float field_0x8;
    /* 0x0C */ float m_u[2];
    /* 0x14 */ float m_v[2];
    /* 0x1C */ unsigned char field_0x1C[4];
    /* 0x20 */ unsigned char field_0x20;
};
CHECK_SIZEOF(UnkEffectManagerRW3Data50Item, 0x24);

struct UnkEffectManagerRW3Data50 {
    UnkEffectManagerRW3Data50(EffectManagerRW3 *manager, const UnkEffectRW3Param4 *parameters);
    ~UnkEffectManagerRW3Data50() {
        if (m_vertices != 0) {
            delete[] m_vertices;
        }
        if (m_textures != 0) {
            delete[] m_textures;
        }
    }
    void fn_80430DE8(unsigned char flag);
    void fn_80431310();
    void fn_80431D18(RwTexDictionary *dictionary, const UnkEffectManagerRW3Data50Item *items,
                     int count);
    UnkEffectRW3Param4 fn_80431F38();
    void fn_80431F6C(const UnkEffectRW3Param4 &parameters);
    void fn_80431FA0(const UnkEffectRW3Param4 *parameters);

    /* 0x00 */ EffectManagerRW3 *m_manager;
    /* 0x04 */ UnkEffectRW3Param4 m_parameters;
    /* 0x14 */ RwTexDictionary *m_dictionary;
    /* 0x18 */ const UnkEffectManagerRW3Data50Item *m_items;
    /* 0x1C */ RwTexture **m_textures;
    /* 0x20 */ unsigned char field_0x20[4]; // Uninterpreted storage; no established reader/type.
    /* 0x24 */ int m_count;
    /* 0x28 */ RwIm2DVertex *m_vertices;
    /* 0x2C */ RwIm2DVertex m_quad[4];
    /* 0x8C */ unsigned char field_0x8C;
    /* 0x8D */ unsigned char field_0x8D;
};
CHECK_SIZEOF(UnkEffectManagerRW3Data50, 0x90);

} // namespace RW3
} // namespace Effect

#endif
