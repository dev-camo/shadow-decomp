#ifndef GAME_EFFECTSHIMMER_H
#define GAME_EFFECTSHIMMER_H

#include "Game/EffectElement.h"
#include "layout_check.h"

namespace Effect {
namespace System {

class EffectCommandExec;
struct UnkCommandData;
struct UnkEffectElementData10;
struct UnkEffectPool;
struct UnkEffectShimmerData;

/* Class and base names are recorded in RTTI; method and member names are inferred. */
class EffectShimmer : public EffectElement {
  public:
    EffectShimmer(UnkEffectElementData10 *definition, EffectLaunchInfo *info,
                  EffectManager *manager);
    virtual ~EffectShimmer();
    virtual void Update(float deltaTime);

    void fn_8042AAB0();

    /* 0xFC */ unsigned int m_commandFlags;
    /* 0x100 */ unsigned int m_definitionFlags;
    /* 0x104 */ float m_duration;
    /* 0x108 */ int field_0x108[2];
    /* 0x110 */ float field_0x110[2];
    /* 0x118 */ Fn8040Vector3 m_amplitude;
    /* 0x124 */ Fn8040Vector3 m_cycle;
    /* 0x130 */ Fn8040Vector3 m_waveLength;
    /* 0x13C */ unsigned int field_0x13C;
    /* 0x140 */ int m_alpha;
    /* 0x144 */ UnkEffectShimmerData *m_parameters;
    /* 0x148 */ const UnkCommandData *m_command;
    /* 0x14C */ EffectCommandExec *m_commandHead;
    /* 0x150 */ EffectCommandExec *m_commandTail;
    /* 0x154 */ UnkEffectPool *m_commandPool;
};
CHECK_SIZEOF(EffectShimmer, 0x158);

} // namespace System
} // namespace Effect

#endif
