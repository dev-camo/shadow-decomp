#ifndef GAME_EFFECTLAUNCHINFORW3_H
#define GAME_EFFECTLAUNCHINFORW3_H

#include "Game/EffectLaunchInfo.h"

struct RwTexDictionary;
struct RpClump;

namespace Effect {
namespace RW3 {

/* Class/base names come from RTTI; pointer fields are borrowed. */
class EffectLaunchInfoRW3 : public System::EffectLaunchInfo {
  public:
    EffectLaunchInfoRW3(System::UnkEffectFileData *file, RwTexDictionary *dictionary,
                        RpClump *clump, unsigned int field10);
    virtual ~EffectLaunchInfoRW3() {}

    /* 0x08 */ RwTexDictionary *field_0x8;
    /* 0x0C */ RpClump *field_0xC;
    /* 0x10 */ unsigned int field_0x10; // Copied word; meaning is unresolved.
};
CHECK_SIZEOF(EffectLaunchInfoRW3, 0x14);

} // namespace RW3
} // namespace Effect

#endif
