#ifndef GAME_EFFECTLAUNCHINFOUSA_H
#define GAME_EFFECTLAUNCHINFOUSA_H

#include "Game/EffectLaunchInfo.h"
#include <dolphin/types.h>
#include "layout_check.h"

class ResourceOneFile;

namespace SonicteamUSA {
namespace System {
namespace Effect {

/* Class and base names come from RTTI. The constructor at 0x8002759C passes its first argument to
 * EffectLaunchInfo and stores the second, a ResourceOneFile (0x80304F60), at 0x8; only the
 * destructor is declared so far. */
class EffectLaunchInfoUSA : public ::Effect::System::EffectLaunchInfo {
  public:
    virtual ~EffectLaunchInfoUSA();

    /* 0x08 */ ResourceOneFile *field_0x8;
};
CHECK_SIZEOF(EffectLaunchInfoUSA, 0xC);

} // namespace Effect
} // namespace System
} // namespace SonicteamUSA

#endif
