#ifndef GAME_EGGMECAVARPARAM_H
#define GAME_EGGMECAVARPARAM_H

#include "Game/BossCharInfo.h"
#include "Game/BossVarParam.h"
#include "Game/SharedConstants.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

struct UnkMotionEventPayload_802A5A20;

/* The two constructors below are inline: every Egg Mecha unit emits its own copies, and the linker
 * keeps the first (0x802E9E04 and 0x802E9FF0, in the EGG_MECA_0511 unit); the bodies here compile
 * to the instructions of those copies. The literals that only the discarded copies use are dropped
 * with them, which is why the other Egg Mecha units' literal pools have 0.0 and 1.0 ahead of 25.0,
 * although their static initialisers use 25.0 before 1.0. The destructor (0x802E9768) is not
 * inline: declared inline, it would be expanded into the boss tasks' destructors, which call it
 * out of line. */

/* The 0x120-byte block of settings at 0x74 of EggMecaVarParam (placeholder name; the meanings of
 * the values are unknown). The Egg Mecha units' Init functions change field_0x30; those of
 * EggMeca310 and EggMeca411 also change field_0x34, and that of EggMeca511 field_0x110 and
 * field_0x114. */
struct UnkEggMecaParams_802E9FF0 {
    UnkEggMecaParams_802E9FF0() {
        field_0x0 = 120.0f;
        field_0x4 = 120.0f;
        field_0x8 = 90.0f;
        field_0xC = 120.0f;
        field_0x10 = 0.0f;
        field_0x14 = 0.0f;
        field_0x18 = 1.0f;
        field_0x1C = 0.5f;
        field_0x20 = 100;
        field_0x24 = 3;
        field_0x28 = 280.0f;
        field_0x2C = 10.0f;
        field_0x30 = 300.0f;
        field_0x34 = 350.0f;
        field_0x38 = 20.0f;
        field_0x3C = 15.0f;
        field_0x40 = field_0x30;
        field_0x44 = 50.0f;
        field_0x48 = 50.0f;
        field_0x4C = 430.0f;
        field_0x50 = 22.5f;
        field_0x54 = 5000.0f;
        field_0x58 = 2.0f;
        field_0x5C = 12.5f;
        field_0x60 = 25.0f;
        field_0x64 = 100.0f;
        field_0x68 = 1000.0f;
        field_0x6C = 0.25f;
        field_0x70 = 0.3f;
        field_0x74 = 1.24f;
        field_0x78 = 10.0f;
        field_0x7C = 0.35f;
        field_0x80 = 15.0f;
        field_0x84 = 0.5f;
        field_0x88 = 1.5f;
        field_0x8C = 3.0f;
        field_0x90 = 10.0f;
        field_0x94 = 90.0f;
        field_0x98 = 30.0f;
        field_0x9C = 10.0f;
        field_0xA0 = 5.0f;
        field_0xA4 = 45.0f;
        field_0xA8 = 90.0f;
        field_0xAC = 8.0f;
        field_0xB0 = 10.0f;
        field_0xB4 = 45.0f;
        field_0xB8 = 4.0f;
        field_0xBC = lbl_805FAD38;
        field_0xC0 = 1.0f;
        field_0xC4 = 60.0f;
        field_0xC8 = 450.0f;
        field_0xCC = 8;
        field_0xD0 = 400.0f;
        field_0xD4 = 300.0f;
        field_0xD8 = 0.0f;
        field_0xDC = 0.25f * lbl_805FAD38;
        field_0xE0 = 30.0f;
        field_0xE4 = 25.0f;
        field_0xE8 = 480.0f;
        field_0xEC = 370.0f;
        field_0xF0 = 150.0f;
        field_0xF4 = 380.0f;
        field_0xF8 = 90.0f;
        field_0xFC = 16.0f;
        field_0x100 = 0.0f;
        field_0x104 = 0.0f;
        field_0x108 = 22.5f;
        field_0x10C = 0;
        field_0x10D = 0;
        field_0x110 = 23;
        field_0x114 = 5;
        field_0x118 = 140.0f;
        field_0x11C = 370.0f;
    }

    /* 0x00 */ float field_0x0;
    /* 0x04 */ float field_0x4;
    /* 0x08 */ float field_0x8;
    /* 0x0C */ float field_0xC;
    /* 0x10 */ float field_0x10;
    /* 0x14 */ float field_0x14;
    /* 0x18 */ float field_0x18;
    /* 0x1C */ float field_0x1C;
    /* 0x20 */ s32 field_0x20;
    /* 0x24 */ s32 field_0x24;
    /* 0x28 */ float field_0x28;
    /* 0x2C */ float field_0x2C;
    /* 0x30 */ float field_0x30;
    /* 0x34 */ float field_0x34;
    /* 0x38 */ float field_0x38;
    /* 0x3C */ float field_0x3C;
    /* 0x40 */ float field_0x40;
    /* 0x44 */ float field_0x44;
    /* 0x48 */ float field_0x48;
    /* 0x4C */ float field_0x4C;
    /* 0x50 */ float field_0x50;
    /* 0x54 */ float field_0x54;
    /* 0x58 */ float field_0x58;
    /* 0x5C */ float field_0x5C;
    /* 0x60 */ float field_0x60;
    /* 0x64 */ float field_0x64;
    /* 0x68 */ float field_0x68;
    /* 0x6C */ float field_0x6C;
    /* 0x70 */ float field_0x70;
    /* 0x74 */ float field_0x74;
    /* 0x78 */ float field_0x78;
    /* 0x7C */ float field_0x7C;
    /* 0x80 */ float field_0x80;
    /* 0x84 */ float field_0x84;
    /* 0x88 */ float field_0x88;
    /* 0x8C */ float field_0x8C;
    /* 0x90 */ float field_0x90;
    /* 0x94 */ float field_0x94;
    /* 0x98 */ float field_0x98;
    /* 0x9C */ float field_0x9C;
    /* 0xA0 */ float field_0xA0;
    /* 0xA4 */ float field_0xA4;
    /* 0xA8 */ float field_0xA8;
    /* 0xAC */ float field_0xAC;
    /* 0xB0 */ float field_0xB0;
    /* 0xB4 */ float field_0xB4;
    /* 0xB8 */ float field_0xB8;
    /* 0xBC */ float field_0xBC;
    /* 0xC0 */ float field_0xC0;
    /* 0xC4 */ float field_0xC4;
    /* 0xC8 */ float field_0xC8;
    /* 0xCC */ s32 field_0xCC;
    /* 0xD0 */ float field_0xD0;
    /* 0xD4 */ float field_0xD4;
    /* 0xD8 */ float field_0xD8;
    /* 0xDC */ float field_0xDC;
    /* 0xE0 */ float field_0xE0;
    /* 0xE4 */ float field_0xE4;
    /* 0xE8 */ float field_0xE8;
    /* 0xEC */ float field_0xEC;
    /* 0xF0 */ float field_0xF0;
    /* 0xF4 */ float field_0xF4;
    /* 0xF8 */ float field_0xF8;
    /* 0xFC */ float field_0xFC;
    /* 0x100 */ float field_0x100;
    /* 0x104 */ float field_0x104;
    /* 0x108 */ float field_0x108;
    /* 0x10C */ u8 field_0x10C;
    /* 0x10D */ u8 field_0x10D;
    /* 0x110 */ s32 field_0x110;
    /* 0x114 */ s32 field_0x114;
    /* 0x118 */ float field_0x118;
    /* 0x11C */ float field_0x11C;
};
CHECK_SIZEOF(UnkEggMecaParams_802E9FF0, 0x120);

/* EggMecaVarParam (class name from RTTI, with BossVarParam as its only base): the variables of the
 * Egg Mecha bosses. The members are mapped from the constructor; their meanings are unknown. */
class EggMecaVarParam : public BossVarParam {
  public:
    EggMecaVarParam() {
        m_charInfo = boost::shared_ptr<BossCharInfo>(new BossCharInfo(0xAB));
        field_0x24 = 0.0f;
        field_0x48 = lbl_805E07A0;
        field_0x1F8 = 0;
        field_0x1B4 = 0;
        field_0x1B5 = 0;
        field_0x1B6 = 0;
        field_0x1B7 = 1;
        field_0x200 = 0;
        field_0x1FC = 0.0f;
        field_0x21C = 0;
        field_0x220 = -1;
        field_0x224 = 0;
        field_0x248 = 1;
        field_0x234[1] = -1;
        field_0x234[0] = -1;
        field_0x23C[1] = 0;
        field_0x23C[0] = 0;
        field_0x249 = 0;
        field_0x244 = 0.0f;
        field_0x24A = 1;
        field_0x24B = 0;
    }
    virtual ~EggMecaVarParam();

    /* 0x074 */ UnkEggMecaParams_802E9FF0 m_params;
    /* 0x194 */ Fn8040Vector3 field_0x194;
    /* 0x1A0 */ Fn8040Vector3 field_0x1A0;
    /* 0x1AC */ u8 field_0x1AC[0x1B4 - 0x1AC];
    /* 0x1B4 */ u8 field_0x1B4;
    /* 0x1B5 */ u8 field_0x1B5;
    /* 0x1B6 */ u8 field_0x1B6;
    /* 0x1B7 */ u8 field_0x1B7;
    /* 0x1B8 */ u8 field_0x1B8[0x1F8 - 0x1B8];
    /* 0x1F8 */ u8 field_0x1F8;
    /* 0x1FC */ float field_0x1FC;
    /* 0x200 */ s32 field_0x200;
    /* 0x204 */ u8 field_0x204[0x21C - 0x204];
    /* The payload of motion event 0x5000, stored by EggMecaCtrlUnit_CheckMotionAttribute's
     * OnMotionEvent (0x802A5A20, the store at 0x802A5AA8; placeholder pointee type). */
    /* 0x21C */ UnkMotionEventPayload_802A5A20 *field_0x21C;
    /* 0x220 */ s32 field_0x220;
    /* 0x224 */ s32 field_0x224;
    /* 0x228 */ u8 field_0x228[0x234 - 0x228];
    /* One entry of each per EggMecaCtrlUnit_Laser, selected by its constructor's index argument
     * (the constructor resets both, 0x802A345C). EggMeca511's Init creates lasers 0 and 1. */
    /* 0x234 */ s32 field_0x234[2];
    /* 0x23C */ s32 field_0x23C[2];
    /* 0x244 */ float field_0x244;
    /* 0x248 */ u8 field_0x248;
    /* 0x249 */ u8 field_0x249;
    /* 0x24A */ u8 field_0x24A;
    /* 0x24B */ u8 field_0x24B;
};
CHECK_SIZEOF(EggMecaVarParam, 0x24C);

#endif
