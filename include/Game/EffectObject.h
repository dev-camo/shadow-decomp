#ifndef GAME_EFFECTOBJECT_H
#define GAME_EFFECTOBJECT_H

#include "Game/EffectElement.h"
#include "layout_check.h"

namespace Effect {
namespace System {

class EffectCommandExec;
struct UnkCommandData;
struct UnkEffectPool;

/* Class and base names from RTTI; member names are inferred. */
class EffectObject : public EffectElement {
  public:
    virtual ~EffectObject();
    virtual void Update(float deltaTime);

    /* 0xFC */ unsigned int m_commandFlags;
    /* 0x100 */ unsigned int m_definitionFlags;
    /* 0x104 */ int m_color[3];
    /* 0x110 */ int m_alpha;
    /* 0x114 */ const UnkCommandData *m_command;
    /* 0x118 */ EffectCommandExec *m_commandHead;
    /* 0x11C */ EffectCommandExec *m_commandTail;
    /* 0x120 */ UnkEffectPool *m_pool;
};
CHECK_SIZEOF(EffectObject, 0x124);

} // namespace System
} // namespace Effect

#endif
