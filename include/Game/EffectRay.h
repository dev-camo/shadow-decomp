#ifndef GAME_EFFECTRAY_H
#define GAME_EFFECTRAY_H

#include "Game/EffectElement.h"
#include "layout_check.h"

namespace Effect {
namespace System {

class EffectRayExecUnit;
class EffectCommandExec;
struct UnkCommandData;
struct UnkEffectPool;
struct UnkEffectElementData6;

/* Class and base names come from RTTI; method/member names, signature types and qualifiers
 * are inferred from the native callers and field operations. */
class EffectRay : public EffectElement {
  public:
    EffectRay(UnkEffectElementData6 *definition, EffectLaunchInfo *info, EffectManager *manager);
    virtual ~EffectRay();
    virtual void Update(float deltaTime);

    void fn_80428B20(float deltaTime);
    void fn_80428DFC(float deltaTime);
    void fn_804293B4();

    /* 0xFC */ EffectRayExecUnit *m_head;
    /* 0x100 */ EffectRayExecUnit *m_tail;
    /* 0x104 */ unsigned int m_commandFlags;
    /* 0x108 */ unsigned int m_definitionFlags;
    /* 0x10C */ int m_capacity;
    /* 0x110 */ int m_count;
    /* 0x114 */ float m_friction;
    /* 0x118 */ float m_genQuantity;
    /* Unrecovered storage; no access is identified in the native base or RW3 methods. */
    /* 0x11C */ unsigned char field_0x11C[4];
    /* 0x120 */ const UnkCommandData *m_command;
    /* 0x124 */ EffectCommandExec *m_commandHead;
    /* 0x128 */ EffectCommandExec *m_commandTail;
    /* 0x12C */ UnkEffectPool *m_commandPool;
    /* 0x130 */ UnkEffectPool *m_rayPool;

  private:
    /* Reconstructed-interface limit: copying the owned pools has not been recovered. */
    EffectRay(const EffectRay &);
    EffectRay &operator=(const EffectRay &);
};
CHECK_SIZEOF(EffectRay, 0x134);

} // namespace System
} // namespace Effect

#endif
