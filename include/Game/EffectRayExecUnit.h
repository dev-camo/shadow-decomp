#ifndef GAME_EFFECTRAYEXECUNIT_H
#define GAME_EFFECTRAYEXECUNIT_H

#include "Game/fn_8040_vector_types.h"
#include "layout_check.h"

namespace Effect {
namespace System {

class EffectCommandExec;
struct UnkCommandData;
struct UnkEffectPool;
struct UnkEffectElementData6;

/* Class name comes from RTTI; method/member names, signature types and qualifiers are inferred. */
class EffectRayExecUnit {
  public:
    EffectRayExecUnit(const UnkEffectElementData6 *definition, UnkEffectPool *commandPool);
    virtual ~EffectRayExecUnit();

    /* 0x04 */ Fn8040Vector3 m_position;
    /* 0x10 */ Fn8040Vector3 m_origin;
    /* 0x1C */ Fn8040Vector3 m_velocity;
    /* 0x28 */ unsigned int m_flags;
    /* 0x2C */ float m_elapsed;
    /* 0x30 */ float m_length;
    /* 0x34 */ float m_width;
    /* 0x38 */ int m_color[3];
    /* 0x44 */ int m_alpha;
    /* 0x48 */ int m_topColor[3];
    /* 0x54 */ int m_topAlpha;
    /* 0x58 */ int m_lastColor[3];
    /* 0x64 */ int m_lastAlpha;
    /* 0x68 */ EffectRayExecUnit *m_prev;
    /* 0x6C */ EffectRayExecUnit *m_next;
    /* 0x70 */ const UnkCommandData *m_command;
    /* 0x74 */ EffectCommandExec *m_commandHead;
    /* 0x78 */ EffectCommandExec *m_commandTail;
    /* 0x7C */ UnkEffectPool *m_commandPool;

  private:
    /* Reconstructed-interface limit: copying this node's owned command list is unrecovered. */
    EffectRayExecUnit(const EffectRayExecUnit &);
    EffectRayExecUnit &operator=(const EffectRayExecUnit &);
};
CHECK_SIZEOF(EffectRayExecUnit, 0x80);

} // namespace System
} // namespace Effect

#endif
