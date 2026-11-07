#ifndef GAME_HEAVYDOGVARPARAM_H
#define GAME_HEAVYDOGVARPARAM_H

#include "Game/BossCharInfo.h"
#include "Game/BossVarParam.h"
#include "Game/LockonTarget.h"
#include "Game/Target.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

struct UnkMotionEventPayload_802A5A20;

/* The constructors and destructors below are inline. The linker keeps the copies in the
 * HEAVY_DOG_0410 unit (constructors 0x8024EDFC and 0x8024ED74, destructors 0x8024E1C4 and
 * 0x8024E16C); the HEAVY_DOG_0510 unit's variables call HeavyDogVarParam's constructor and
 * destructor there (0x802541AC, 0x802539E4). */

/* The 0x74-byte block of settings at 0x74 of HeavyDogVarParam (placeholder name; the meanings of
 * the values are unknown). HeavyDog0410's Init copies field_0x48 to HeavyDogVarParam::field_0xE8
 * and passes field_0x70 to its BossCtrlUnit_Shadow. */
struct UnkHeavyDogParams_8024EDFC {
    UnkHeavyDogParams_8024EDFC() {
        field_0x0 = 0.55f;
        field_0x4 = 1.2f;
        field_0x8 = 0.55f;
        field_0xC = 1.2f;
        field_0x10 = 30.0f;
        field_0x14 = 20.0f;
        field_0x18 = 20.0f;
        field_0x1C = 65.0f;
        field_0x20 = 5.0f;
        field_0x24 = 0.5f;
        field_0x28 = 200;
        field_0x2C = 200;
        field_0x30 = 3;
        field_0x34 = 200.0f;
        field_0x38 = 10.0f;
        field_0x3C = 150.0f;
        field_0x40 = 280.0f;
        field_0x44 = 15.0f;
        field_0x48 = 20.0f;
        field_0x4C = 2.0f;
        field_0x50 = 25.0f;
        field_0x54 = 45.0f;
        field_0x58 = 10;
        field_0x5C = 300.0f;
        field_0x60 = 10.0f;
        field_0x64 = 3.5f;
        field_0x68 = 150.0f;
        field_0x6C = 200.0f;
        field_0x70 = 40.0f;
    }

    /* 0x00 */ float field_0x0;
    /* 0x04 */ float field_0x4;
    /* 0x08 */ float field_0x8;
    /* 0x0C */ float field_0xC;
    /* 0x10 */ float field_0x10;
    /* 0x14 */ float field_0x14;
    /* 0x18 */ float field_0x18;
    /* 0x1C */ float field_0x1C;
    /* 0x20 */ float field_0x20;
    /* 0x24 */ float field_0x24;
    /* 0x28 */ s32 field_0x28;
    /* 0x2C */ s32 field_0x2C;
    /* 0x30 */ s32 field_0x30;
    /* 0x34 */ float field_0x34;
    /* 0x38 */ float field_0x38;
    /* 0x3C */ float field_0x3C;
    /* 0x40 */ float field_0x40;
    /* 0x44 */ float field_0x44;
    /* 0x48 */ float field_0x48;
    /* 0x4C */ float field_0x4C;
    /* 0x50 */ float field_0x50;
    /* 0x54 */ float field_0x54;
    /* 0x58 */ s32 field_0x58;
    /* 0x5C */ float field_0x5C;
    /* 0x60 */ float field_0x60;
    /* 0x64 */ float field_0x64;
    /* 0x68 */ float field_0x68;
    /* 0x6C */ float field_0x6C;
    /* 0x70 */ float field_0x70;
};
CHECK_SIZEOF(UnkHeavyDogParams_8024EDFC, 0x74);

/* HeavyDogVarParam (class name from RTTI, with BossVarParam as its only base): the variables of the
 * Heavy Dog bosses. The members are mapped from the constructor and HeavyDog0410's Init; their
 * meanings are unknown, and the regions marked unknown are used by code that is not reconstructed
 * yet. */
class HeavyDogVarParam : public BossVarParam {
  public:
    HeavyDogVarParam() {
        m_charInfo = boost::shared_ptr<BossCharInfo>(new BossCharInfo(0xA9));
        field_0xE8 = 0.0f;
        field_0x11C = lbl_805E07A0;
        field_0x128 = lbl_805E07A0;
        field_0x114 = 0;
        field_0x115 = 0;
        field_0x117 = 0;
        field_0x116 = 1;
        field_0x1C8 = 0;
        field_0x1D0 = 0;
        field_0x1D4 = 0;
        /* Two statements: the original stores field_0x1C4 and loads it back for field_0x1C0,
         * which a chained assignment does not do. */
        field_0x1C4 = m_params.field_0x44;
        field_0x1C0 = field_0x1C4;
        field_0x13C = 0;
        field_0x138 = 0;
        field_0x140 = 0;
        field_0x5C = 0.0f;
        field_0x134 = 0.0f;
        field_0x1A8 = boost::shared_ptr<Weapon::LockonTarget>(new Weapon::LockonTarget());
    }
    virtual ~HeavyDogVarParam() {}

    /* 0x074 */ UnkHeavyDogParams_8024EDFC m_params;
    /* 0x0E8 */ float field_0xE8;
    /* 0x0EC */ Fn8040Vector3 field_0xEC;
    /* 0x0F8 */ Fn8040Vector3 field_0xF8;
    /* 0x104 */ float field_0x104;             /* HEAVY_DOG_0510's Init copies field_0x110 here */
    /* 0x108 */ u8 field_0x108[0x110 - 0x108]; /* unknown */
    /* HEAVY_DOG_0510's Init stores fn_80016260(-x, -z) of field_0xEC here. */
    /* 0x110 */ float field_0x110;
    /* 0x114 */ u8 field_0x114;
    /* 0x115 */ u8 field_0x115;
    /* 0x116 */ u8 field_0x116;
    /* 0x117 */ u8 field_0x117;
    /* 0x118 */ u8 field_0x118[0x11C - 0x118]; /* unknown */
    /* 0x11C */ Fn8040Vector3 field_0x11C;
    /* 0x128 */ Fn8040Vector3 field_0x128;
    /* 0x134 */ float field_0x134;
    /* 0x138 */ s32 field_0x138;
    /* 0x13C */ s32 field_0x13C;
    /* 0x140 */ s32 field_0x140;
    /* 0x144 */ u8 field_0x144[0x1A0 - 0x144]; /* unknown */
    /* 0x1A0 */ boost::shared_ptr<Target> field_0x1A0;
    /* 0x1A8 */ boost::shared_ptr<Weapon::LockonTarget> field_0x1A8;
    /* 0x1B0 */ u8 field_0x1B0[0x1C0 - 0x1B0]; /* unknown */
    /* 0x1C0 */ float field_0x1C0;
    /* 0x1C4 */ float field_0x1C4;
    /* 0x1C8 */ u8 field_0x1C8;
    /* 0x1C9 */ u8 field_0x1C9[0x1D0 - 0x1C9]; /* unknown */
    /* 0x1D0 */ s32 field_0x1D0;
    /* The payload of motion event 0x5000, stored by HeavyDogCtrlUnit_CheckMotionAttribute's
     * handler (0x8024A590) like EggMecaVarParam::field_0x21C (placeholder pointee type). */
    /* 0x1D4 */ UnkMotionEventPayload_802A5A20 *field_0x1D4;
};
CHECK_SIZEOF(HeavyDogVarParam, 0x1D8);

/* HeavyDog0410VarParam (class name from RTTI; bases HeavyDogVarParam and BossVarParam): the
 * variables of the HEAVY_DOG_0410 boss. The constructor sets the ten floats below; their meanings
 * are unknown. */
class HeavyDog0410VarParam : public HeavyDogVarParam {
  public:
    HeavyDog0410VarParam() {
        field_0x1D8 = 85.0f;
        field_0x1DC = 60.0f;
        field_0x1E0 = 40.0f;
        field_0x1E4 = 40.0f;
        field_0x1E8 = 5.0f;
        field_0x1EC = 1.0f;
        field_0x1F0 = 0.5f;
        field_0x1F4 = 1.5f;
        field_0x1F8 = 1.2f;
        field_0x1FC = 10.0f;
    }
    virtual ~HeavyDog0410VarParam() {}

    /* 0x1D8 */ float field_0x1D8;
    /* 0x1DC */ float field_0x1DC;
    /* 0x1E0 */ float field_0x1E0;
    /* 0x1E4 */ float field_0x1E4;
    /* 0x1E8 */ float field_0x1E8;
    /* 0x1EC */ float field_0x1EC;
    /* 0x1F0 */ float field_0x1F0;
    /* 0x1F4 */ float field_0x1F4;
    /* 0x1F8 */ float field_0x1F8;
    /* 0x1FC */ float field_0x1FC;
};
CHECK_SIZEOF(HeavyDog0410VarParam, 0x200);

#endif
