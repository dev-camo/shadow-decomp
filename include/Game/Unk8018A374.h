#ifndef GAME_UNK8018A374_H
#define GAME_UNK8018A374_H

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
    void fn_801A687C(const Fn8040Vector3 &position);
    bool fn_801A6944(const Fn8040Vector3 &position) const;

    /* 0x00 */ Unk8018A3D0 field_0x0;
    /* 0x20 */ Unk8018A3CC field_0x20;
};
CHECK_SIZEOF(Unk8018A374, 0x44);

#endif
