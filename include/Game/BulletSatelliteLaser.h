#ifndef GAME_BULLETSATELLITELASER_H
#define GAME_BULLETSATELLITELASER_H

#include "Game/CharInfo.h"
#include "Game/EffectElement.h"
#include "Game/Task.h"
#include "Game/UnkParams_80051D10.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace BulletSatelliteLaser {

/* BulletSatelliteLaser::SatelliteLaser (class name from RTTI, with Task as its only base): the
 * task that the bullet records 0x3B and 0x3C of the table lbl_8051D720 create. At the position of
 * the parameters' lock-on target it starts effect 0x28B and plays sound 0xE027; once its timer,
 * which starts at 1, has been counted down by the updates' time steps, it creates a CommonBomb
 * there for the owner, plays sound 0xE028, calls fn_8034DF48(7, -2) and finishes. Without an
 * owner or a target it finishes at once (behaviour inferred). Method and member names are
 * inferred. */
class SatelliteLaser : public Task {
  public:
    SatelliteLaser(const UnkParams_80051D10 &params, int kind);
    virtual ~SatelliteLaser();
    /* Counts m_timer down by dt; when it reaches 0 or less, calls Explode and sets Task flag
     * 0x01. */
    virtual void Update(float dt);

    /* The create functions of the two records: each allocates the task with kind 0 or 1. */
    static void CreateKind0(const UnkParams_80051D10 *params);
    static void CreateKind1(const UnkParams_80051D10 *params);

    /* Creates the CommonBomb at m_position for the owner (fn_801DD040) with type 0 and the value
     * 10 for kind 0, and type 1 and the value 18 for any other kind; its mask is 4 with bit 1 set
     * when field_0x38 is 0 and bit 0 set otherwise. Then plays sound 0xE028 and calls
     * fn_8034DF48(7, -2), discarding the result. */
    void Explode();

    /* 0x28 */ boost::weak_ptr<CharInfo> m_owner;
    /* 0x30 */ boost::weak_ptr<Effect::System::EffectElement> m_effect;
    /* 0x38 */ int field_0x38;           /* the parameters' field_0x34 (meaning unknown) */
    /* 0x3C */ Fn8040Vector3 m_position; /* the target's position at construction */
    /* 0x48 */ int m_kind;               /* 0 or 1, from the create function */
    /* 0x4C */ float m_timer;            /* time left until the explosion; starts at 1 */
};
CHECK_SIZEOF(SatelliteLaser, 0x50);

} // namespace BulletSatelliteLaser

#endif
