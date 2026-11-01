#ifndef GAME_EFFECTLAUNCHINFO_H
#define GAME_EFFECTLAUNCHINFO_H

#include "layout_check.h"

namespace Effect {
namespace System {

struct UnkEffectFileData;

/* Class name from RTTI; the file data is borrowed. */
class EffectLaunchInfo {
  public:
    EffectLaunchInfo(UnkEffectFileData *file) : field_0x4(file) {}
    virtual ~EffectLaunchInfo() {}

    /* 0x04 */ UnkEffectFileData *field_0x4;
};
CHECK_SIZEOF(EffectLaunchInfo, 0x8);

} // namespace System
} // namespace Effect

#endif
