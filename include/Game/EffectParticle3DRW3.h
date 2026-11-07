#ifndef GAME_EFFECTPARTICLE3DRW3_H
#define GAME_EFFECTPARTICLE3DRW3_H

#include "Game/EffectParticle.h"

struct RpClump;
struct RwTexture;
struct RpWorld;

namespace Effect {
namespace System {
struct UnkEffectElementData5;
}
namespace RW3 {

struct UnkEffectParticle3DRW3Data;

/* Class/base names come from RTTI. Method roles, member names/types and parameter qualifiers
 * are inferred from their users. */
class EffectParticle3DRW3 : public System::EffectParticle {
  public:
    EffectParticle3DRW3(const System::UnkEffectElementData5 *definition,
                        System::EffectLaunchInfo *info, System::EffectManager *manager);
    virtual ~EffectParticle3DRW3();
    virtual void Update(float deltaTime);
    virtual void Render();

    /* 0x13C */ UnkEffectParticle3DRW3Data *m_particles;
    /* 0x140 */ RpClump *m_clump;     // Borrowed template for the per-particle clones.
    /* 0x144 */ RwTexture *m_texture; // Borrowed.
    /* 0x148 */ RpWorld *m_world;     // Borrowed.
    /* 0x14C */ Fn8040Vector3 m_corners[4];
};
CHECK_SIZEOF(EffectParticle3DRW3, 0x17C);

} // namespace RW3
} // namespace Effect

#endif
