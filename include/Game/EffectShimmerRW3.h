#ifndef GAME_EFFECTSHIMMERRW3_H
#define GAME_EFFECTSHIMMERRW3_H

#include "Game/EffectShimmer.h"
#include "rw/rwcore.h"
#include "layout_check.h"

namespace Effect {
namespace RW3 {

/* Class and base names come from RTTI; member names and parameter qualifiers are inferred. */
class EffectShimmerRW3 : public System::EffectShimmer {
  public:
    EffectShimmerRW3(System::UnkEffectElementData10 *definition, System::EffectLaunchInfo *info,
                     System::EffectManager *manager);
    virtual ~EffectShimmerRW3();
    virtual void Update(float deltaTime);
    virtual void Render();

    /* 0x158 */ RwIm2DVertex m_quad[4];
    /* 0x1B8 */ RwIm2DVertex *m_vertices;
    /* 0x1BC */ RwV3d *m_grid;
    /* 0x1C0 */ float *m_offsetsX;
    /* 0x1C4 */ float *m_offsetsY;
    /* 0x1C8 */ int m_vertexCount;
    /* 0x1CC */ int m_gridCount;
    /* 0x1D0 */ float m_phaseX;
    /* 0x1D4 */ float m_phaseY;
};
CHECK_SIZEOF(EffectShimmerRW3, 0x1D8);

} // namespace RW3
} // namespace Effect

#endif
