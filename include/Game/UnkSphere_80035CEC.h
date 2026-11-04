#ifndef GAME_UNKSPHERE_80035CEC_H
#define GAME_UNKSPHERE_80035CEC_H

#include "Game/fn_8040_vector_types.h"
#include "layout_check.h"

/* A vector and a float, built by the constructor at 0x80035CEC (placeholder name: the two look like
 * the centre and radius of a sphere, which is inferred). The boss units construct their copies at
 * startup and point their parameter tables at them; much other code builds them too (for example
 * fn_800C7DEC and fn_800C7EA8).
 * Same layout as UnkSphere_80021C04, whose constructor differs; kept separate until evidence links
 * them. */
class UnkSphere_80035CEC {
  public:
    UnkSphere_80035CEC(const Fn8040Vector3 &center, float radius);

    /* 0x00 */ Fn8040Vector3 m_center;
    /* 0x0C */ float m_radius;
};
CHECK_SIZEOF(UnkSphere_80035CEC, 0x10);

#endif
