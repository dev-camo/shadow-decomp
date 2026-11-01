#ifndef GAME_EFFECTSYSTEMRW3_H
#define GAME_EFFECTSYSTEMRW3_H

#include "Game/EffectSystem.h"
#include "layout_check.h"

struct RwCamera;

namespace Effect {
namespace RW3 {

/* Class and base names come from RTTI; GetCamera is inferred from the returned object. */
class EffectSystemRW3 : public System::EffectSystem {
  public:
    virtual RwCamera *GetCamera() = 0;
};
/* No added storage is observed; the concrete derived class begins its fields at 0x8. */
CHECK_SIZEOF(EffectSystemRW3, 0x8);

} // namespace RW3
} // namespace Effect

#endif
