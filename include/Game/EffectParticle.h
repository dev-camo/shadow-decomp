#ifndef GAME_EFFECTPARTICLE_H
#define GAME_EFFECTPARTICLE_H

#include "Game/EffectElement.h"
#include "layout_check.h"

namespace Effect {
namespace System {

class EffectCommandExec;
struct UnkCommandData;
struct UnkEffectPool;
struct UnkEffectParticleData120;
struct UnkEffectParticleData124;
struct UnkEffectElementData2;
struct UnkEffectElementData5;

/* Class name comes from RTTI; method roles, member names/types and parameter qualifiers
 * are inferred from their users. */
class EffectParticleExecUnit {
  public:
    EffectParticleExecUnit(const UnkEffectElementData5 *definition, UnkEffectPool *commandPool);
    EffectParticleExecUnit(const UnkEffectElementData2 *definition, UnkEffectPool *commandPool);
    virtual ~EffectParticleExecUnit();

    void fn_804281D4();

    /* The node allocation and constructor-cleanup paths use the allocator's current pool. */
    static void *operator new(unsigned long size) throw() {
        return fn_8042A50C(fn_8042A5D0());
    }
    static void operator delete(void *block) {
        fn_8042A444(fn_8042A5D0(), block);
    }

    /* 0x04 */ Fn8040Vector3 m_position;
    /* 0x10 */ Fn8040Vector3 m_rotation;
    /* 0x1C */ Fn8040Vector3 m_velocity;
    /* 0x28 */ Fn8040Vector3 m_scale;
    /* 0x34 */ Fn8040Vector2 m_uvLower;
    /* 0x3C */ float field_0x3C;
    /* 0x40 */ Fn8040Vector2 m_uvUpper;
    /* 0x48 */ float field_0x48;
    /* 0x4C */ unsigned int m_flags;
    /* 0x50 */ float m_elapsed;
    /* Color/alpha executors update signed channels with 22 fractional bits; renderers
     * extract each channel byte with an unsigned shift. */
    /* 0x54 */ int m_color[4];
    /* 0x64 */ EffectParticleExecUnit *m_prev;
    /* 0x68 */ EffectParticleExecUnit *m_next;
    /* 0x6C */ const UnkCommandData *m_command;
    /* 0x70 */ EffectCommandExec *m_commandHead;
    /* 0x74 */ EffectCommandExec *m_commandTail;
    /* 0x78 */ UnkEffectPool *m_commandPool;
};
CHECK_SIZEOF(EffectParticleExecUnit, 0x7C);

/* Class/base names come from RTTI. Method roles, member names/types and parameter qualifiers
 * are inferred from their users. */
class EffectParticle : public EffectElement {
  public:
    EffectParticle(const UnkEffectElementData *definition, EffectLaunchInfo *info,
                   EffectManager *manager);
    virtual ~EffectParticle();
    virtual void Update(float deltaTime);

    void fn_80426FF4(float deltaTime);
    void fn_804272E8(float deltaTime);
    void fn_80427970();

    /* 0xFC */ EffectParticleExecUnit *m_head;
    /* 0x100 */ EffectParticleExecUnit *m_tail;
    /* 0x104 */ unsigned int m_commandFlags;
    /* 0x108 */ unsigned int m_definitionFlags;
    /* 0x10C */ int m_maxCount;
    /* 0x110 */ int m_count;
    /* 0x114 */ float field_0x114;
    /* 0x118 */ float field_0x118;
    /* 0x11C */ float field_0x11C;
    /* Borrowed definition views have measured prefixes; full serialized extents are unknown. */
    /* 0x120 */ const UnkEffectParticleData120 *field_0x120;
    /* 0x124 */ const UnkEffectParticleData124 *field_0x124;
    /* 0x128 */ const UnkCommandData *m_command;
    /* 0x12C */ EffectCommandExec *m_commandHead;
    /* 0x130 */ EffectCommandExec *m_commandTail;
    /* 0x134 */ UnkEffectPool *m_commandPool;
    /* 0x138 */ UnkEffectPool *m_particlePool;
};
CHECK_SIZEOF(EffectParticle, 0x13C);

} // namespace System
} // namespace Effect

#endif
