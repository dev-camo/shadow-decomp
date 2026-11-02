#ifndef GAME_EGGFLEET_H
#define GAME_EGGFLEET_H

#include "Game/GadgetCharInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "boost/shared_ptr.hpp"
#include <vector>

class FleetHolder;
class EggCannon;
class GadgetModelInfo;
class UnkGadgetOwner_800C9CD0;
class UnkTarget_80166AF4;
class UnkObject_801E72DC;
class UnkObject_802B4254;
struct UnkFleetConfig_80522CC8;

/* Class name from the original smart-pointer RTTI; member names are unknown. */
class EggFleet {
  public:
    EggFleet(FleetHolder *holder, UnkGadgetOwner_800C9CD0 *owner);
    ~EggFleet();
    void fn_800CCEB4();
    void fn_800CCF30(float dt);

    /* 0x00 */ UnkFleetConfig_80522CC8 *field_0x0;
    /* 0x04 */ FleetHolder *field_0x4;
    /* 0x08 */ UnkGadgetOwner_800C9CD0 *field_0x8;
    /* 0x0C */ boost::shared_ptr<GadgetCharInfo> field_0xC;
    /* 0x14 */ boost::shared_ptr<GadgetModelInfo> field_0x14;
    /* 0x1C */ boost::shared_ptr<UnkTarget_80166AF4> field_0x1C;
    /* 0x24 */ boost::shared_ptr<UnkObject_801E72DC> field_0x24;
    /* 0x2C */ boost::shared_ptr<UnkObject_802B4254> field_0x2C;
    /* 0x34 */ s32 field_0x34;
    /* 0x38 */ UnkBitFlags_80014E04 field_0x38;
    /* 0x3C */ float field_0x3C;
    /* 0x40 */ float field_0x40;
    /* 0x44 */ float field_0x44;
    /* 0x48 */ std::vector<boost::shared_ptr<EggCannon> > field_0x48;
    /* Four-byte cannon indices; unsigned representation inferred from their construction. */
    /* 0x54 */ std::vector<u32> field_0x54;
    /* 0x60 */ UnkSoundHandle_801D3DC8 field_0x60;
};
CHECK_SIZEOF(EggFleet, 0x7C);

#endif
