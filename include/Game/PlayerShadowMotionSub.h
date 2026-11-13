#ifndef GAME_PLAYERSHADOWMOTIONSUB_H
#define GAME_PLAYERSHADOWMOTIONSUB_H

#include "Game/Task.h"
#include "boost/shared_ptr.hpp"
#include <vector>
#include "layout_check.h"

namespace Player {
class PlayerBase;

namespace Shadow {

/* The 0x20-byte object that MotionSub's constructor (0x8034F828) creates (0x8034FBFC constructs
 * it; type name unknown). */
class UnkMotionSubImpl_8034FBFC;

/* Player::Shadow::MotionSub (class name and Task base from RTTI): a task that ShadowMotion creates
 * through fn_8034F890 and forwards calls to. Only the functions that ShadowMotion calls are
 * declared; their names are placeholders and their signatures follow ShadowMotion's calls. */
class MotionSub : public Task {
  public:
    void fn_8034F66C();
    void fn_8034F6E0();
    void fn_8034F704(std::vector<void *> &objects);
    void fn_8034F728();

    /* 0x28 */ UnkMotionSubImpl_8034FBFC *field_0x28;
    /* 0x2C */ u8 field_0x2C[4]; /* Unrecovered storage. */
};
CHECK_SIZEOF(MotionSub, 0x30);

} // namespace Shadow
} // namespace Player

extern "C" {
/* Returns a new MotionSub for the owner, or null for an owner whose word at 0x64 is not one of the
 * four values the function selects a variant for (placeholder name; behaviour from its body). */
Player::Shadow::MotionSub *fn_8034F890(Task *parent,
                                       const boost::shared_ptr<Player::PlayerBase> &owner);
}

#endif
