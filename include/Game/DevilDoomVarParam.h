#ifndef GAME_DEVILDOOMVARPARAM_H
#define GAME_DEVILDOOMVARPARAM_H

#include "Game/BossCharInfo.h"
#include "Game/BossVarParam.h"
#include "Game/SharedConstants.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

/* The constructors and destructors below are inline; the copies the linker keeps are those of the
 * DEVIL_DOOM unit. */

/* The 0x118-byte block of settings at 0x74 of DevilDoomVarParam (placeholder name; the meanings of
 * the values are unknown, and the signedness of the integer fields is inferred).
 * DevilDoomVarParam's constructor calls the copy of this inline constructor at 0x8030C86C instead
 * of expanding it. The constructor leaves field_0xE8 uninitialised. */
struct UnkDevilDoomParams_8030C86C {
    UnkDevilDoomParams_8030C86C() {
        field_0x0 = 30.0f;
        field_0x4 = 1000.0f;
        field_0x8 = 500;
        field_0xC = 280.0f;
        field_0x10 = 120.0f;
        field_0x14 = 360.0f;
        field_0x18 = 3.0f;
        field_0x1C = 2.0f;
        field_0x20 = 2.7f;
        field_0x24 = 100.0f;
        field_0x28 = 3000.0f;
        field_0x2C = 2.0f;
        field_0x30 = 15.0f;
        field_0x34 = 2.0f;
        field_0x38 = -1.0f;
        field_0x3C = 4;
        field_0x40 = -300.0f;
        field_0x44 = -200.0f;
        field_0x48 = 100.0f;
        field_0x4C = -50.0f;
        field_0x50 = 50.0f;
        field_0x54 = 250.0f;
        field_0x58 = 500.0f;
        field_0x5C = 45.0f;
        field_0x60 = 3.0f;
        field_0x64 = 30.0f;
        field_0x68 = 1.5f;
        field_0x6C = 1.2f;
        field_0x70 = 250.0f;
        field_0x74 = 400.0f;
        field_0x78 = 5.0f;
        field_0x7C = -200.0f;
        field_0x80 = 3.0f;
        field_0x84 = 6.0f;
        field_0x88 = 800;
        field_0x8C = 2.0f;
        field_0x90 = 10.0f;
        field_0x94 = 2.0f;
        field_0x98 = 1.0f;
        field_0x9C = 1.5f;
        field_0xA0 = 800.0f;
        field_0xA4 = 0.3f;
        field_0xA8 = 100.0f;
        field_0xAC = 100.0f;
        field_0xB0 = 0.2f;
        field_0xB4 = 60.0f;
        field_0xB8 = 12.0f;
        field_0xBC = 2.0f;
        field_0xC0 = 300.0f;
        field_0xC4 = 75.0f;
        field_0xC8 = 1.0f;
        field_0xCC = 3.0f;
        field_0xD0 = 300.0f;
        field_0xD4 = 15.0f;
        field_0xD8 = 2.0f;
        field_0xDC = 0.1f;
        field_0xE0 = 400.0f;
        field_0xE4 = 1;
        field_0xEC = 600.0f;
        field_0xF0 = 5.0f;
        field_0xF4 = 800.0f;
        field_0xF8 = 10.0f;
        field_0xFC = 200.0f;
        field_0x100 = -5.0f;
        field_0x104 = 120.0f;
        field_0x108 = 90.0f;
        field_0x10C = 0.2f;
        field_0x110 = 0.7f;
        field_0x114 = 0.5f;
    }

    /* 0x000 */ float field_0x0;
    /* 0x004 */ float field_0x4;
    /* 0x008 */ s32 field_0x8;
    /* 0x00C */ float field_0xC;
    /* 0x010 */ float field_0x10;
    /* 0x014 */ float field_0x14;
    /* 0x018 */ float field_0x18;
    /* 0x01C */ float field_0x1C;
    /* 0x020 */ float field_0x20;
    /* 0x024 */ float field_0x24;
    /* 0x028 */ float field_0x28;
    /* 0x02C */ float field_0x2C;
    /* 0x030 */ float field_0x30;
    /* 0x034 */ float field_0x34;
    /* 0x038 */ float field_0x38;
    /* 0x03C */ s32 field_0x3C;
    /* 0x040 */ float field_0x40;
    /* 0x044 */ float field_0x44;
    /* 0x048 */ float field_0x48;
    /* 0x04C */ float field_0x4C;
    /* 0x050 */ float field_0x50;
    /* 0x054 */ float field_0x54;
    /* 0x058 */ float field_0x58;
    /* 0x05C */ float field_0x5C;
    /* 0x060 */ float field_0x60;
    /* 0x064 */ float field_0x64;
    /* 0x068 */ float field_0x68;
    /* 0x06C */ float field_0x6C;
    /* 0x070 */ float field_0x70;
    /* 0x074 */ float field_0x74;
    /* 0x078 */ float field_0x78;
    /* 0x07C */ float field_0x7C;
    /* 0x080 */ float field_0x80;
    /* 0x084 */ float field_0x84;
    /* 0x088 */ s32 field_0x88;
    /* 0x08C */ float field_0x8C;
    /* 0x090 */ float field_0x90;
    /* 0x094 */ float field_0x94;
    /* 0x098 */ float field_0x98;
    /* 0x09C */ float field_0x9C;
    /* 0x0A0 */ float field_0xA0;
    /* 0x0A4 */ float field_0xA4;
    /* 0x0A8 */ float field_0xA8;
    /* 0x0AC */ float field_0xAC;
    /* 0x0B0 */ float field_0xB0;
    /* 0x0B4 */ float field_0xB4;
    /* 0x0B8 */ float field_0xB8;
    /* 0x0BC */ float field_0xBC;
    /* 0x0C0 */ float field_0xC0;
    /* 0x0C4 */ float field_0xC4;
    /* 0x0C8 */ float field_0xC8;
    /* 0x0CC */ float field_0xCC;
    /* 0x0D0 */ float field_0xD0;
    /* 0x0D4 */ float field_0xD4;
    /* 0x0D8 */ float field_0xD8;
    /* 0x0DC */ float field_0xDC;
    /* 0x0E0 */ float field_0xE0;
    /* 0x0E4 */ s32 field_0xE4;
    /* 0x0E8 */ u8 field_0xE8[4]; /* unknown */
    /* 0x0EC */ float field_0xEC;
    /* 0x0F0 */ float field_0xF0;
    /* 0x0F4 */ float field_0xF4;
    /* 0x0F8 */ float field_0xF8;
    /* 0x0FC */ float field_0xFC;
    /* 0x100 */ float field_0x100;
    /* 0x104 */ float field_0x104;
    /* 0x108 */ float field_0x108;
    /* 0x10C */ float field_0x10C;
    /* 0x110 */ float field_0x110;
    /* 0x114 */ float field_0x114;
};
CHECK_SIZEOF(UnkDevilDoomParams_8030C86C, 0x118);

/* An 0x20-byte entry of UnkDevilDoomEntry_8030C820::field_0x5C (placeholder name; the layout is
 * not mapped). Its constructor (0x8030C868) does nothing, but the original passes it to
 * __construct_array, so the type has a constructor of its own. */
struct UnkDevilDoomSubEntry_8030C868 {
    UnkDevilDoomSubEntry_8030C868() {}

    /* 0x00 */ u8 field_0x0[0x20]; /* unknown */
};
CHECK_SIZEOF(UnkDevilDoomSubEntry_8030C868, 0x20);

/* An 0x15C-byte entry of DevilDoomVarParam::field_0x214 (placeholder name; the layout is not
 * mapped). Its constructor (0x8030C820) only constructs the eight entries at 0x5C. */
struct UnkDevilDoomEntry_8030C820 {
    /* 0x00 */ u8 field_0x0[0x5C]; /* unknown */
    /* 0x5C */ UnkDevilDoomSubEntry_8030C868 field_0x5C[8];
};
CHECK_SIZEOF(UnkDevilDoomEntry_8030C820, 0x15C);

/* DevilDoomVarParam (class name from RTTI, with BossVarParam as its only base): the variables of
 * the DEVIL_DOOM boss. The members are mapped from the constructor, DevilDoom's Init and the
 * DEVIL_DOOM mode units; their meanings are unknown, and the regions marked unknown are used by
 * code that is not reconstructed yet. */
class DevilDoomVarParam : public BossVarParam {
  public:
    DevilDoomVarParam() {
        m_charInfo = boost::shared_ptr<BossCharInfo>(new BossCharInfo(0xAE));
        field_0x24 = 0.0f;
        field_0x48 = lbl_805E07A0;
        field_0x18C = 1;
        field_0x18D = 0;
        field_0x200 = 0.0f;
        field_0x634 = 0;
        field_0x18E = 0;
        field_0x18F = false;
        field_0x190 = false;
        field_0x191[0] = 1;
        field_0x191[1] = 0;
        field_0x193 = 0;
        field_0x194 = 1;
        field_0x195 = 1;
        field_0x199 = 0;
        field_0x19A = 0;
        field_0x204 = 0.0f;
        field_0x208 = lbl_805FAD38;
        field_0x19C = 0;
        field_0x1A4 = 0.0f;
        field_0x210 = 0;
        field_0x20C = 0;
        field_0x628 = 0;
        field_0x62C = 0.0f;
    }
    virtual ~DevilDoomVarParam() {}

    /* 0x074 */ UnkDevilDoomParams_8030C86C m_params;
    /* 0x18C */ u8 field_0x18C;
    /* 0x18D */ u8 field_0x18D;
    /* 0x18E */ u8 field_0x18E;
    /* The mode units toggle the next two with !, whose result the original stores without
     * masking it to a byte (cntlzw and srwi), as for a bool. */
    /* 0x18F */ bool field_0x18F;
    /* 0x190 */ bool field_0x190;
    /* 0x191 */ u8 field_0x191[2];
    /* 0x193 */ u8 field_0x193;
    /* 0x194 */ u8 field_0x194;
    /* 0x195 */ u8 field_0x195;
    /* 0x196 */ u8 field_0x196;
    /* 0x197 */ u8 field_0x197;
    /* 0x198 */ u8 field_0x198;
    /* 0x199 */ u8 field_0x199;
    /* 0x19A */ u8 field_0x19A;
    /* 0x19C */ s32 field_0x19C;
    /* 0x1A0 */ s32 field_0x1A0;
    /* 0x1A4 */ float field_0x1A4;
    /* 0x1A8 */ float field_0x1A8;
    /* 0x1AC */ float field_0x1AC;
    /* 0x1B0 */ u8 field_0x1B0[0x1BC - 0x1B0]; /* unknown */
    /* 0x1BC */ Fn8040Vector3 field_0x1BC;
    /* 0x1C8 */ u8 field_0x1C8[0x1D4 - 0x1C8]; /* unknown */
    /* 0x1D4 */ Fn8040Vector3 field_0x1D4;
    /* 0x1E0 */ u8 field_0x1E0[0x1EC - 0x1E0]; /* unknown */
    /* 0x1EC */ float field_0x1EC;
    /* 0x1F0 */ float field_0x1F0;
    /* 0x1F4 */ float field_0x1F4;
    /* 0x1F8 */ float field_0x1F8;
    /* 0x1FC */ u8 field_0x1FC[4]; /* unknown */
    /* 0x200 */ float field_0x200;
    /* 0x204 */ float field_0x204;
    /* 0x208 */ float field_0x208;
    /* 0x20C */ s32 field_0x20C; /* signedness inferred */
    /* 0x210 */ s32 field_0x210; /* signedness inferred */
    /* 0x214 */ UnkDevilDoomEntry_8030C820 field_0x214[3];
    /* 0x628 */ s32 field_0x628; /* signedness inferred */
    /* 0x62C */ float field_0x62C;
    /* 0x630 */ float field_0x630;
    /* A word that the mode units test against 0 with an unsigned compare and clear (type
     * inferred). */
    /* 0x634 */ u32 field_0x634;
};
CHECK_SIZEOF(DevilDoomVarParam, 0x638);

#endif
