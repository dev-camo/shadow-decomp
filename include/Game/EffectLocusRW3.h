#ifndef GAME_EFFECTLOCUSRW3_H
#define GAME_EFFECTLOCUSRW3_H

#include "Game/EffectLocus.h"
#include "rw/rwcore.h"
#include "layout_check.h"

namespace Effect {
namespace RW3 {

/* Class and base names come from RTTI; member names and parameter qualifiers are inferred. */
class EffectLocusRW3 : public System::EffectLocus {
  public:
    EffectLocusRW3(System::UnkEffectElementData7 *definition, System::EffectLaunchInfo *info,
                   System::EffectManager *manager);
    virtual ~EffectLocusRW3();
    virtual void Update(float deltaTime);
    virtual void Render();

    void fn_80439B18();

    /* 0x170 */ RwIm3DVertex *m_vertices;
    /* Original size accounting includes these bytes; their type and use are unknown. */
    /* 0x174 */ unsigned char field_0x174[8];
};
CHECK_SIZEOF(EffectLocusRW3, 0x17C);

} // namespace RW3
} // namespace Effect

#endif
