#ifndef GAME_SUPER_H
#define GAME_SUPER_H

#include "Game/PlayerBehavior.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class CharCommand;

namespace Player {
namespace Behavior {

/* Player::Behavior::Super (class name from RTTI, with Player::Behavior::Behavior as its base): the
 * player behaviour with id 0x22. While field_0x1C is 0, each update calls fn_802EB39C, which
 * sets field_0x1C to 1 once fn_8027AAA4 returns true for index 0xE of fn_80078C88's manager; while
 * it is 1, each update calls fn_802EB0C4. Member names and the names of the non-virtual methods are
 * placeholders; parameter names and the pointer spellings are inferred. */
class Super : public Behavior {
  public:
    Super(const boost::shared_ptr<PlayerBase> &player);
    virtual ~Super();
    /* Returns true. */
    virtual bool fn_802EB530();
    /* Calls fn_8027AA78 for index 0xE of fn_80078C88's manager, sets bits 2, 7 and 9 of the
     * player's flags at 0xA0 and clears field_0x24, field_0x28 and field_0x1C. */
    virtual void fn_802EB4B8();
    /* Calls fn_8027AA4C for index 0xE of fn_80078C88's manager and resets bits 2, 7 and 9 of the
     * player's flags at 0xA0. */
    virtual void fn_802EB454();
    /* Sets bit 1 of the command's flags only while field_0x1C is 0. */
    virtual void fn_80072420(CharCommand *command);
    /* Runs fn_802EB39C when field_0x1C is 0 and fn_802EB0C4 when it is 1; any other value does
     * nothing. */
    virtual void Update(float dt);

    /* Allocates a Super for the player and constructs it only when the allocation succeeds;
     * returns the result, which is null when it fails (name inferred). */
    static Behavior *Create(const boost::shared_ptr<PlayerBase> &player);

    /* Sets field_0x24 to 250 + 150 * factor, where factor is the length of the player's vector at
     * 0x18 divided by 750, replaced by 1 unless it is less than 1 (fn_800704AC). */
    void fn_802EB060();
    /* The update while field_0x1C is 1 (see the definition). */
    void fn_802EB0C4(float dt);
    /* Calls fn_802EB3E0 when fn_8027AAA4 returns true for index 0xE of fn_80078C88's manager. */
    void fn_802EB39C();
    /* Sets field_0x1C to 1 and field_0x20 to 0. */
    void fn_802EB3E0();

    static const char *s_name;

    /* 0x1C */ int field_0x1C;
    /* fn_802EB3E0 clears it; fn_802EB0C4 calls the motion's fn_8006CB7C only while it is 0 and
     * then increments it. */
    /* 0x20 */ int field_0x20;
    /* 0x24 */ float field_0x24;
    /* 0x28 */ float field_0x28;
};
CHECK_SIZEOF(Super, 0x2C);

} // namespace Behavior
} // namespace Player

/* Super's behaviour record, the one that entry 0x22 of the table at 0x80521CC8 (USA, GUPE8P)
 * points to (placeholder name). */
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_80507B50;

#endif
