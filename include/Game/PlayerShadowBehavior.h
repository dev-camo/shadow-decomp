#ifndef GAME_PLAYERSHADOWBEHAVIOR_H
#define GAME_PLAYERSHADOWBEHAVIOR_H

#include "Game/PlayerBase.h"
#include "Game/PlayerBehavior.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace Player {
namespace Shadow {

/* Player::Shadow::ShadowBehavior (class name from RTTI, with Player::Behavior::Factory as its
 * base): the behaviour factory that a virtual function of Player::PlayerShadow (USA (GUPE8P)
 * 0x800AD3F8) creates for its player. Its constructor calls the second function of every record
 * of the behaviour table that has one, passing the player, and its destructor the third. */
class ShadowBehavior : public Behavior::Factory {
  public:
    ShadowBehavior(const boost::shared_ptr<PlayerBase> &player);
    virtual ~ShadowBehavior();
    /* Returns a new behaviour made by the record of id in the behaviour table, with its weak
     * pointer at 0x14 pointing at itself, or an empty pointer when the table entry for id is null
     * or the record has no creating function (id is used as an index without a range check). While
     * bit 0x25 of the player's bit set at 0xA8 is set, ids 0, 2 and 3 give the behaviour of id
     * 0x22 (Player::Behavior::Super) instead. */
    virtual boost::shared_ptr<Behavior::Behavior> Create(int id);

    /* 0x4 */ boost::shared_ptr<PlayerBase> m_player; /* name inferred */
};
CHECK_SIZEOF(ShadowBehavior, 0xC);

} // namespace Shadow
} // namespace Player

extern "C" {
/* Returns a new Player::Shadow::ShadowBehavior for player (placeholder name). The result type is
 * the one whose destructor that Player::PlayerShadow function calls on the returned temporary
 * (0x800A8188), which is also the destructor of the member at 0x38 of the task that receives it
 * (destructor 0x800A80DC). */
boost::shared_ptr<Player::Behavior::Factory>
fn_800AE330(const boost::shared_ptr<Player::PlayerBase> &player);
}

#endif
