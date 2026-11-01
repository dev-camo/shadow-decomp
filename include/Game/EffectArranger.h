#ifndef GAME_EFFECTARRANGER_H
#define GAME_EFFECTARRANGER_H

#include "Game/EffectElement.h"
#include "layout_check.h"

namespace Effect {
namespace System {

class EffectCommandExec;
struct UnkCommandData;
struct UnkEffectElementData3;
struct UnkEffectPool;

/* Launch commands retain a borrowed transform source for this option (name inferred). */
enum { ARRANGER_LINK_TRANSFORM = 0x800 };

/* Class/base identity from RTTI; member and non-placeholder method names are inferred. */
class EffectArranger : public EffectElement {
  public:
    EffectArranger(const UnkEffectElementData3 *definition, EffectLaunchInfo *info,
                   EffectManager *manager);
    virtual ~EffectArranger();
    virtual void Update(float deltaTime);
    virtual void Render() {}

    void fn_80426700();

    /* 0xFC */ unsigned int m_commandFlags;
    /* 0x100 */ unsigned int m_definitionFlags;
    /* 0x104 */ EffectLaunchInfo *m_launchInfo;
    /* 0x108 */ const UnkCommandData *m_command;
    /* 0x10C */ EffectCommandExec *m_commandHead;
    /* 0x110 */ EffectCommandExec *m_commandTail;
    /* 0x114 */ UnkEffectPool *m_commandPool;
    /* 0x118 */ UnkEffectPool *m_launchPool;
};
CHECK_SIZEOF(EffectArranger, 0x11C);

} // namespace System
} // namespace Effect

#endif
