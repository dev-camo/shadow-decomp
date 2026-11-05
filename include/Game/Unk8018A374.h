#ifndef GAME_UNK8018A374_H
#define GAME_UNK8018A374_H

#include "Game/UnkEnemySetData.h"
#include "Game/fn_8040_vector_types.h"
#include "layout_check.h"

/* Original type names unknown. Actual empty constructors at 0x8018A3D0 and 0x8018A3CC
 * precede independent clears of these 0x20/0x24-byte subobjects. */
struct Unk8018A3D0 {
    /* 0x00 */ Fn8040Vector3 field_0x0;
    /* 0x0C */ Fn8040Vector3 field_0xC;
    /* 0x18 */ float field_0x18;
    /* 0x1C */ float field_0x1C;
};
CHECK_SIZEOF(Unk8018A3D0, 0x20);

struct Unk8018A3CC {
    /* 0x00 */ Fn8040Vector3 field_0x0;
    /* 0x0C */ Fn8040Vector3 field_0xC;
    /* 0x18 */ float field_0x18;
    /* 0x1C */ float field_0x1C;
    /* 0x20 */ float field_0x20;
};
CHECK_SIZEOF(Unk8018A3CC, 0x24);

struct Unk8018A374 {
    Unk8018A374();
    /* The reference and const qualifiers of the position parameters and the method-const
     * qualifiers of fn_801A6944 and fn_801A69A0 are inferred: the callers (such as fn_801A69A0's
     * at 0x8018B1E8) only pass addresses. */
    void fn_801A687C(const Fn8040Vector3 &position);
    bool fn_801A68C0(const Fn8040Vector3 &position);
    bool fn_801A6944(const Fn8040Vector3 &position) const;
    bool fn_801A69A0(const Fn8040Vector3 &position) const;
    /* setData is either the caller's own UnkParameters_8018A3D4 member (0x2C bytes, passed at
     * 0x8018967C), into which fn_800C9DBC copies the owner's parameter block only when the owner's
     * size equals 0x2C, or the owner's block itself, whose pointer fn_800C9D30 returns
     * (0x801A7328). Only the six common entries are read, and the seventh entry differs per
     * table: BK_CHAOS's table at 0x80555D78 has MaxLife at 0x18, and BkChaosTask reaches
     * 0x801A7328 from 0x802B07B0. Reference and const qualifiers inferred: the callers pass
     * addresses. */
    void fn_801A69FC(const Fn8040Vector3 &position, const UnkRotation3 &rotation,
                     const UnkEnemySetDataCommon &setData);

    /* 0x00 */ Unk8018A3D0 field_0x0;
    /* 0x20 */ Unk8018A3CC field_0x20;
};
CHECK_SIZEOF(Unk8018A374, 0x44);

#endif
