#ifndef GAME_EFFECTARRANGERRW3_H
#define GAME_EFFECTARRANGERRW3_H

#include "Game/EffectArranger.h"

namespace Effect {
namespace RW3 {

/* Class and base names come from RTTI. */
class EffectArrangerRW3 : public System::EffectArranger {
  public:
    EffectArrangerRW3(const System::UnkEffectElementData3 *definition,
                      System::EffectLaunchInfo *info, System::EffectManager *manager);
    virtual ~EffectArrangerRW3();
};
CHECK_SIZEOF(EffectArrangerRW3, 0x11C);

} // namespace RW3
} // namespace Effect

#endif
