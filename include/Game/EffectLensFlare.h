#ifndef GAME_EFFECTLENSFLARE_H
#define GAME_EFFECTLENSFLARE_H

#include "Game/EffectElement.h"
#include "layout_check.h"

namespace Effect {
namespace System {

struct UnkEffectElementData9;

/* Class and base names are recorded in RTTI; method and member names are inferred. */
class EffectLensFlare : public EffectElement {
  public:
    EffectLensFlare(UnkEffectElementData9 *definition, EffectLaunchInfo *info,
                    EffectManager *manager);
    virtual ~EffectLensFlare();
    virtual void Update(float deltaTime);

    /* 0xFC */ unsigned int m_definitionFlags;
    /* 0x100 */ float m_progress;
    /* 0x104 */ float m_duration;
};
CHECK_SIZEOF(EffectLensFlare, 0x108);

} // namespace System
} // namespace Effect

#endif
