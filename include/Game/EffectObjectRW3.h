#ifndef GAME_EFFECTOBJECTRW3_H
#define GAME_EFFECTOBJECTRW3_H

#include "Game/EffectObject.h"
#include "layout_check.h"

struct RpClump;
struct RpWorld;

namespace Effect {
namespace RW3 {

/* Class and base names come from RTTI; method and member names, signature types and qualifiers
 * are inferred. */
class EffectObjectRW3 : public System::EffectObject {
  public:
    EffectObjectRW3(System::UnkEffectElementData4 *definition, System::EffectLaunchInfo *info,
                    System::EffectManager *manager);
    virtual ~EffectObjectRW3();
    virtual void Update(float deltaTime);
    virtual void Render();

    /* 0x124 */ RpClump *m_clump;
    /* Unrecovered word, zeroed by the constructor; no other use is identified. */
    /* 0x128 */ unsigned int field_0x128;
    /* 0x12C */ RpWorld *m_world;
    /* 0x130 */ unsigned char m_inWorld;
};
CHECK_SIZEOF(EffectObjectRW3, 0x134);

} // namespace RW3
} // namespace Effect

#endif
