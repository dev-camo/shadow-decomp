#ifndef GAME_PLAYERBEHAVIOR_H
#define GAME_PLAYERBEHAVIOR_H

#include "Game/PlayerBase.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharCommand;

/* The pointee of Player::Behavior::Behavior's weak pointer at 0x14 (placeholder; the type is not
 * identified). */
class UnkBehaviorObject_80076C94;

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

    /* 0x04 */ boost::shared_ptr<PlayerBase> m_player;
    /* 0x0C */ const char *m_name;
    /* 0x10 */ int m_id;
    /* Its constructor (0x80076C94) clears both words and its destructor (0x80076BA0) releases a
     * weak count at 0x4, as boost::weak_ptr's do. */
    /* 0x14 */ boost::weak_ptr<UnkBehaviorObject_80076C94> field_0x14;
};
CHECK_SIZEOF(Behavior, 0x1C);

/* A behaviour record (placeholder name). The table at 0x80521CC8 (USA, GUPE8P) holds 35 pointers
 * to behaviour records indexed by id, followed by two zero words. Every record has a creating
 * function, a function pointer and a zero word; the function pointer is an empty function in 29
 * records and null in six (ids 3, 4, 21, 22, 31 and 32). The signature of field_0x4 and the type
 * of the last word are inferred: the targets are empty and the callers of the table are not
 * identified. */
struct UnkBehaviorRecord_80521CC8 {
    /* 0x0 */ Behavior *(*create)(const boost::shared_ptr<PlayerBase> &player);
    /* 0x4 */ void (*field_0x4)(void);
    /* 0x8 */ u32 field_0x8;
};
CHECK_SIZEOF(UnkBehaviorRecord_80521CC8, 0xC);

} // namespace Behavior
} // namespace Player

#endif
