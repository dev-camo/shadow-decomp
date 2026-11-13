#ifndef GAME_PLAYERBEHAVIOR_H
#define GAME_PLAYERBEHAVIOR_H

#include "Game/PlayerBase.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharCommand;

namespace Player {
namespace Behavior {

/* Player::Behavior::Behavior (class name from RTTI): the base of the player behaviours (Super,
 * Ground, Jump and the other Player::Behavior classes), 0x1C bytes. Its constructor (0x80076C3C)
 * and destructor (0x80076B38) are defined with the class elsewhere.
 *
 * Its vtable (0x80520030) leaves slots 1, 2, 3 and 5 pure. The pure slots are named after
 * Player::Behavior::Super's implementations and the others after their own implementations; their
 * purpose is unknown. The parameter of fn_80072420 is inferred from Super's override, which sets
 * bit 1 of the flags at 0x7 of the command; the base implementation is empty. Update's name is
 * inferred: Super's implementation runs its per-state update with the float. */
class Behavior {
  public:
    /* Stores the shared pointer, the name and the id at 0x4, 0xC and 0x10 and clears the weak
     * pointer at 0x14. Super passes its own name, "Super", and 0x22, the index of its record in
     * the table of pointers to behaviour records at 0x80521CC8 (USA, GUPE8P). */
    Behavior(const boost::shared_ptr<PlayerBase> &player, const char *name, int id);
    virtual ~Behavior();
    virtual bool fn_802EB530() = 0;
    virtual void fn_802EB4B8() = 0;
    virtual void fn_802EB454() = 0;
    virtual void fn_80072420(CharCommand *command);
    virtual void Update(float dt) = 0;
    virtual void fn_80076CA4();

    /* Assigns the weak pointer at 0x14 (name inferred from the member's placeholder). Its callers
     * call the copy at 0x800AE5D8 (USA, GUPE8P), which follows the first of them,
     * Player::Shadow::ShadowBehavior::Create. */
    void SetField0x14(const boost::weak_ptr<Behavior> &value) {
        field_0x14 = value;
    }

    /* 0x04 */ boost::shared_ptr<PlayerBase> m_player;
    /* 0x0C */ const char *m_name;
    /* 0x10 */ int m_id;
    /* Player::Shadow::ShadowBehavior::Create points it at the behaviour itself: it assigns the
     * shared pointer that owns the new behaviour. */
    /* 0x14 */ boost::weak_ptr<Behavior> field_0x14;
};
CHECK_SIZEOF(Behavior, 0x1C);

/* A behaviour record (placeholder name). The table at 0x80521CC8 (USA, GUPE8P) holds a pointer to
 * the record of each of the 35 behaviour ids, followed by two null pointers. create returns a new
 * behaviour for the player. Player::Shadow::ShadowBehavior's constructor calls field_0x4 and its
 * destructor field_0x8 of each record, skipping null records and null function pointers, with the
 * address of its shared pointer to the player, which it also passes to create. Their parameter
 * type is inferred from create's and their void results from the unused results. field_0x4 is an
 * empty function in 29 records and null in six (ids 3, 4, 21, 22, 31 and 32); field_0x8 is null in
 * all 35. */
struct UnkBehaviorRecord_80521CC8 {
    /* 0x0 */ Behavior *(*create)(const boost::shared_ptr<PlayerBase> &player);
    /* 0x4 */ void (*field_0x4)(const boost::shared_ptr<PlayerBase> &player);
    /* 0x8 */ void (*field_0x8)(const boost::shared_ptr<PlayerBase> &player);
};
CHECK_SIZEOF(UnkBehaviorRecord_80521CC8, 0xC);

/* Player::Behavior::Factory (class name from RTTI): the interface that creates a player's
 * behaviours. Its vtable (0x80521D78 in USA, GUPE8P) has the destructor and one pure slot;
 * Create's name and its parameter's are inferred from Player::Shadow::ShadowBehavior's override,
 * which returns a new behaviour for a behaviour id. */
class Factory {
  public:
    virtual ~Factory() {}
    virtual boost::shared_ptr<Behavior> Create(int id) = 0;
};
CHECK_SIZEOF(Factory, 0x4);

} // namespace Behavior
} // namespace Player

#endif
