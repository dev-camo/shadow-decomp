#ifndef GAME_EFFECTLOCUS_H
#define GAME_EFFECTLOCUS_H

#include "Game/EffectElement.h"
#include "layout_check.h"

namespace Effect {
namespace System {

class EffectCommandExec;
class EffectLocusExecUnit;
struct UnkCommandData;
struct UnkEffectElementData7;
struct UnkEffectPool;

/* Class and base names come from RTTI; member and non-placeholder method names are inferred. */
class EffectLocus : public EffectElement {
  public:
    EffectLocus(UnkEffectElementData7 *definition, EffectLaunchInfo *info, EffectManager *manager);
    virtual ~EffectLocus();
    virtual void Update(float deltaTime);

    void fn_8042B938(float deltaTime);
    void fn_8042BD8C();
    inline void ExpireUnits(float deltaTime);

    /* 0xFC */ EffectLocusExecUnit *m_head;
    /* 0x100 */ EffectLocusExecUnit *m_tail;
    /* 0x104 */ EffectLocusExecUnit *m_previousUnit;
    /* 0x108 */ unsigned int m_commandFlags;
    /* 0x10C */ unsigned int m_definitionFlags;
    /* 0x110 */ int m_capacity;
    /* 0x114 */ int m_unitCount;
    /* 0x118 */ int m_addedCount;
    /* 0x11C */ int m_removedCount;
    /* 0x120 */ int m_pointCount;
    /* 0x124 */ float m_interval;
    /* 0x128 */ float m_accumulatedTime;
    /* 0x12C */ int m_color[3];
    /* 0x138 */ int m_alpha;
    /* 0x13C */ int m_topColor[3];
    /* 0x148 */ int m_topAlpha;
    /* 0x14C */ int m_lastColor[3];
    /* 0x158 */ int m_lastAlpha;
    /* 0x15C */ const UnkCommandData *m_command;
    /* 0x160 */ EffectCommandExec *m_commandHead;
    /* 0x164 */ EffectCommandExec *m_commandTail;
    /* 0x168 */ UnkEffectPool *m_commandPool;
    /* 0x16C */ UnkEffectPool *m_nodePool;
};
CHECK_SIZEOF(EffectLocus, 0x170);

} // namespace System
} // namespace Effect

#endif
