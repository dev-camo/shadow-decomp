#ifndef GAME_EFFECTLENSFLARERW3_H
#define GAME_EFFECTLENSFLARERW3_H

#include "Game/EffectLensFlare.h"
#include "Game/UnkEffectRW3Parameters.h"

namespace Effect {
namespace RW3 {

/* Class and base names come from RTTI; member names and parameter qualifiers are inferred. */
class EffectLensFlareRW3 : public System::EffectLensFlare {
  public:
    EffectLensFlareRW3(System::UnkEffectElementData9 *definition, System::EffectLaunchInfo *info,
                       System::EffectManager *manager);
    virtual ~EffectLensFlareRW3();
    virtual void Update(float deltaTime);
    virtual void Render() {}

    /* 0x108 */ UnkEffectRW3Param4 m_initial;
    /* 0x118 */ UnkEffectRW3Param4 m_target;
};
CHECK_SIZEOF(EffectLensFlareRW3, 0x128);

} // namespace RW3
} // namespace Effect

#endif
