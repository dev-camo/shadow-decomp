#ifndef GAME_EFFECTRAYRW3_H
#define GAME_EFFECTRAYRW3_H

#include "Game/EffectRay.h"
#include "rw/rwcore.h"
#include "layout_check.h"

namespace Effect {
namespace RW3 {

/* Class and base names come from RTTI; method/member names, signature types and qualifiers
 * are inferred from native callers and field operations. */
class EffectRayRW3 : public System::EffectRay {
  public:
    EffectRayRW3(System::UnkEffectElementData6 *definition, System::EffectLaunchInfo *info,
                 System::EffectManager *manager);
    virtual ~EffectRayRW3();
    virtual void Update(float deltaTime);
    virtual void Render();

    /* The renderer submits four adjacent vertices for each ray (inferred grouping). */
    /* 0x134 */ RwIm3DVertex (*m_vertices)[4];

  private:
    /* Reconstructed-interface limit: copying the owned vertex array has not been recovered. */
    EffectRayRW3(const EffectRayRW3 &);
    EffectRayRW3 &operator=(const EffectRayRW3 &);
};
CHECK_SIZEOF(EffectRayRW3, 0x138);

} // namespace RW3
} // namespace Effect

#endif
