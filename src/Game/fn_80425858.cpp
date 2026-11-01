/* Base EffectManager factory (method name inferred). Kept separate while the other
 * methods of the original manager unit remain NonMatching. */
#include "Game/EffectManager.h"

namespace Effect {
namespace System {

EffectElement *EffectManager::CreateElement(const UnkEffectElementData *definition,
                                            EffectLaunchInfo *info) {
    return 0;
}

} // namespace System
} // namespace Effect
