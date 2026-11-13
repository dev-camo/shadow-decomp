#ifndef GAME_PLAYERSHADOWMOTIONEYE_H
#define GAME_PLAYERSHADOWMOTIONEYE_H

#include "Game/Task.h"
#include "boost/shared_ptr.hpp"
#include <vector>
#include "layout_check.h"

namespace Player {
class PlayerBase;

namespace Shadow {

struct UnkMotionResources_800B0D3C;
/* The 0x90-byte object that MotionEye's constructor creates (0x800B623C constructs it; type name
 * unknown). */
class UnkMotionEyeImpl_800B623C;

/* Player::Shadow::MotionEye (class name and Task base from RTTI): a task that ShadowMotion creates
 * (0x800B4BD0) and forwards calls to. Only the functions that ShadowMotion calls are declared;
 * their names are placeholders and their signatures follow ShadowMotion's calls. */
class MotionEye : public Task {
  public:
    MotionEye(Task *parent, const boost::shared_ptr<PlayerBase> &owner,
              const UnkMotionResources_800B0D3C &resources);

    int fn_800B5B90();
    void fn_800B5BC4();
    void fn_800B5BE8();
    void fn_800B5C0C(std::vector<void *> &objects);
    void fn_800B5C30();

    /* 0x28 */ UnkMotionEyeImpl_800B623C *field_0x28;
    /* 0x2C */ u8 field_0x2C[4]; /* Unrecovered storage. */
};
CHECK_SIZEOF(MotionEye, 0x30);

} // namespace Shadow
} // namespace Player

#endif
