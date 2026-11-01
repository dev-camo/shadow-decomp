#ifndef GAME_EFFECTGLOBALPARAM_H
#define GAME_EFFECTGLOBALPARAM_H

#include "Game/EffectElement.h"
#include "layout_check.h"

namespace Effect {
namespace System {

struct UnkEffectElementData8;
struct UnkEffectGlobalParamData;

/* Class and base names are recorded in RTTI; method and member names are inferred. */
class EffectGlobalParam : public EffectElement {
  public:
    EffectGlobalParam(const UnkEffectElementData8 *definition, EffectLaunchInfo *info,
                      EffectManager *manager);
    virtual ~EffectGlobalParam();
    virtual void Update(float deltaTime);
    virtual void Render();

    /* 0xFC */ unsigned int m_definitionFlags;
    /* 0x100 */ float m_progress;
    /* 0x104 */ float m_duration;
    /* 0x108 */ const UnkEffectGlobalParamData *field_0x108;
};
CHECK_SIZEOF(EffectGlobalParam, 0x10C);

} // namespace System
} // namespace Effect

#endif
