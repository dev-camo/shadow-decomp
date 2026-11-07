#ifndef GAME_EFFECTPARTICLERW3_H
#define GAME_EFFECTPARTICLERW3_H

#include "Game/EffectParticle.h"

struct RpAtomic;
struct RwFrame;

namespace Effect {
namespace System {
struct UnkEffectElementData2;
}
namespace RW3 {

/* Class/base names come from RTTI. Method roles, member names/types and parameter qualifiers
 * are inferred from their users. */
class EffectParticleRW3 : public System::EffectParticle {
  public:
    EffectParticleRW3(const System::UnkEffectElementData2 *definition,
                      System::EffectLaunchInfo *info, System::EffectManager *manager);
    virtual ~EffectParticleRW3();
    virtual void Update(float deltaTime);
    virtual void Render();

    /* 0x13C */ RpAtomic *m_atomic;
    /* 0x140 */ RwFrame *m_frame;
    /* 0x144 */ unsigned int field_0x144; // Zeroed and tested; meaning is unresolved.
};
CHECK_SIZEOF(EffectParticleRW3, 0x148);

} // namespace RW3
} // namespace Effect

#endif
