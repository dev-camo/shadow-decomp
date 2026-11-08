#ifndef GAME_EFFECTLAUNCHINFOUSA_H
#define GAME_EFFECTLAUNCHINFOUSA_H

#include "Game/EffectLaunchInfo.h"
#include <dolphin/types.h>
#include "layout_check.h"

class ResourceOneFile;

namespace SonicteamUSA {
namespace System {
namespace Effect {

/* Class and base names come from RTTI. The constructor passes its first argument to
 * EffectLaunchInfo and stores the second, a ResourceOneFile (0x80304F60, USA (GUPE8P)), at 0x8.
 * The constructor and destructor are inline (inferred): the PJS launcher's file has the RTTI
 * records of EffectLaunchInfo and EffectLaunchInfoUSA (names at 0x8050B684 and 0x8050B6A8, USA
 * (GUPE8P)) before those of the shared pointer's counter, which a unit gives when it emits weak
 * copies of these members and of the vtable; with the members declared out of line they come after
 * Launcher's (measured). Both call an inline member of EffectLaunchInfo, and under the game's
 * -inline noauto,level=1,deferred setting an inline function that calls another inline function
 * is not expanded at its call sites, so they stay out of line; 0x8002759C and 0x800275C0 are the
 * copies the linker kept. */
class EffectLaunchInfoUSA : public ::Effect::System::EffectLaunchInfo {
  public:
    EffectLaunchInfoUSA(::Effect::System::UnkEffectFileData *file, ResourceOneFile *resource)
        : EffectLaunchInfo(file), field_0x8(resource) {}
    virtual ~EffectLaunchInfoUSA() {}

    /* 0x08 */ ResourceOneFile *field_0x8;
};
CHECK_SIZEOF(EffectLaunchInfoUSA, 0xC);

} // namespace Effect
} // namespace System
} // namespace SonicteamUSA

#endif
