#ifndef GAME_WARPHOLE_H
#define GAME_WARPHOLE_H

#include "Game/CharCommand.h"
#include "Game/EffectElement.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/Task.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/Player.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class CharInfo;
class UnkGadgetOwner_800C9CD0;

namespace WarpHole {

/* A table of one word per com id (placeholder name; the binary does not name it). A ManagerTask
 * created with a com id of 0 or more sets that entry to 1 and clears it when it is destroyed. A
 * function-local static that fn_80129504 constructs on first use holds it, and GetInstance caches
 * its address. The methods are named after the functions that implement them. */
class UnkWarpTable_80129504 {
  public:
    /* Empty; the static instance registers it for destruction at exit. */
    ~UnkWarpTable_80129504() {}

    /* Return the table, constructing it on first use (names inferred for GetInstance; fn_80129504
     * holds the function-local static). */
    static UnkWarpTable_80129504 *GetInstance();
    static UnkWarpTable_80129504 *fn_80129504();

    /* Both clear every word (fn_80129E38 for the descriptor's first hook, fn_80129E10 for the
     * second). */
    void fn_80129E10();
    void fn_80129E38();

    /* 0x000 */ s32 m_busy[256];
};
CHECK_SIZEOF(WarpHole::UnkWarpTable_80129504, 0x400);

/* WarpHole::ManagerTask (class name from RTTI, with Task as its only base): the task that moves a
 * character to a position (behaviour inferred). The constructor sends the character a
 * ClrVelocityCommand and a Player::StartExternalControlCommand; Update then advances through the
 * states described there and, while the character still exists, places it with fn_800B8000 and
 * finally sends it a Player::EndExternalControlCommand. Member names and the names of the static
 * methods are inferred. */
class ManagerTask : public Task {
  public:
    /* Stores the position, the user and the com id and, for a com id of 0 or more, sets its table
     * entry and bit 0 of m_warpFlags. Sends the user the two commands, passes true to fn_80203970,
     * passes the object of fn_800BADE8 to fn_801E2F28 and fn_801E2EDC and clears its field_0x30,
     * and clears bit 1 of m_warpFlags. It calls through user without testing it. */
    ManagerTask(Task *parent, const Fn8040Vector3 &position,
                const boost::shared_ptr<Player::Player> &user, int comId);
    /* Clears the table entry when bit 0 of m_warpFlags is set; unless the word at 0x8 of
     * fn_80074470's manager is 6, passes false to fn_80203970 when fn_802E5810 is true (USA) or
     * when fn_80342284 does not return 1 (Japan and PAL); then sets field_0x30 of the object of
     * fn_800BADE8 to 1. */
    virtual ~ManagerTask();
    /* Each update first sets field_0x30 of the object of fn_800BADE8 to 1 when fn_802E5810 is
     * false. Then, by state:
     * 0: passes 1.0f to fn_800446D0, plays sound 0xE014 and moves to 1.
     * 1: once the word at 0x14 of fn_80044254's manager is 1: if the user still exists, places it
     *    at m_position with its own rotation through fn_800B8000 (which has its own conditions)
     *    and, when the first entry of lbl_8051CE88 is not null, stores 2 in that object's
     *    field_0x40; then moves to 2 and clears m_counter (whether or not the user exists).
     * 2: on the update that finds m_counter at 1 (the second in this state), when the first
     *    entry of lbl_8051CE88 is not null, passes it to fn_8003A52C and stores 2 in its
     *    field_0x40, then moves to 3 either way; m_counter counts every update in this state.
     * 3: when the first entry of lbl_8051CE88 is not null and both its field_0x40 and its
     *    field_0x978 are 0, sends the user, if it still exists, a
     *    Player::EndExternalControlCommand, passes 0.0f to fn_800447B8, plays sound 0xE015, moves
     *    to 5 and sets bit 1 of m_warpFlags. Otherwise it places the user again through
     *    fn_800B8000, if it still exists; a null entry keeps the task in this state.
     * 5: finishes the task (Task flag 0x01) when bit 0 of m_warpFlags is clear.
     * After the switch, in the same update, it also finishes when bits 1 and 0 are both set, the
     * user has not expired and fn_800B7AD0 is true for it; with bit 0 set this is the only way to
     * finish in this update (behaviour inferred). */
    virtual void Update(float dt);

    /* Create a ManagerTask under task layer 11 (names inferred). fn_80117668 calls the first, which
     * passes -1 as the com id, so that the task uses no entry of the table. The position and the
     * user are taken by reference (inferred: the bytes pass addresses). */
    static void Create(const Fn8040Vector3 &position,
                       const boost::shared_ptr<Player::Player> &user);
    static void Create(const Fn8040Vector3 &position, const boost::shared_ptr<Player::Player> &user,
                       int comId);

    static const char *s_name;

    /* 0x28 */ Fn8040Vector3 m_position; /* where the character is placed */
    /* 0x34 */ boost::weak_ptr<Player::Player> m_user;
    /* The com id whose table entry the task sets when it is 0 or more. */
    /* 0x3C */ s32 m_comId;
    /* Counts the updates in state 2; state 1 sets it to 0, the constructor leaves it unset. */
    /* 0x40 */ s32 m_counter;
    /* 0 after construction, then 1, 2, 3 and 5 as Update advances (see Update). */
    /* 0x44 */ s32 m_state;
    /* Bit 0 is set by the constructor when it sets the table entry; bit 1 is cleared by the
     * constructor and set when state 3 moves to 5. Nothing else changes them. */
    /* 0x48 */ UnkBitFlags_80014E04 m_warpFlags;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(WarpHole::ManagerTask, 0x50);

/* The search that the warp hole's command handler passes to FindPartner (placeholder name; the
 * binary does not name it). The handler fills m_self and m_owner; FindPartner fills the rest when
 * it finds the partner. Nothing in this file reads m_self. */
struct UnkPartnerSearch_8012A028 {
    UnkPartnerSearch_8012A028() : m_owner(0), m_found(false) {}

    /* 0x00 */ boost::weak_ptr<GadgetCharInfo> m_self; /* the warp hole's character info */
    /* 0x08 */ UnkGadgetOwner_800C9CD0 *m_owner;       /* the warp hole's owner */
    /* 0x0C */ Fn8040Vector3 m_position;               /* the partner's position */
    /* 0x18 */ boost::weak_ptr<CharInfo> m_user;       /* the character that used the hole */
    /* 0x20 */ bool m_found;
};
CHECK_SIZEOF(WarpHole::UnkPartnerSearch_8012A028, 0x24);

/* WarpHole::SetWarpHole (class name from RTTI; the gadget's descriptor is "WARP_HOLE"): a warp
 * hole with effect 0xE1 and a sphere probe of radius 20 (through a handle target) at the owner's
 * position. On a UseCommand (command id 0xD) in state 3, when its user still exists, it looks for
 * another owner of kind 0x1F with the same com id and, when it finds one and the user's character
 * info casts (dynamic_cast) to Player::Player, creates a ManagerTask that moves the user to
 * that owner's position (behaviour inferred). Member names and the names of the non-virtual
 * methods are inferred. */
class SetWarpHole : public Gadget {
  public:
    SetWarpHole(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetWarpHole();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and com id, returns to state 0 and passes field_0x4C
     * to fn_8040CE60 (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Passes the character info's position to the effect while the effect exists (name
     * inferred). */
    void UpdateEffect();
    /* Clears the effect's active flag while the effect exists (name inferred). */
    void StopEffect();
    /* The character info's command handler (name inferred; see the class comment). */
    void OnCommand(CharCommand &command);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;   /* of the sphere probe */
    /* 0x40 */ boost::weak_ptr<Effect::System::EffectElement> m_effect; /* effect 0xE1 */
    /* 0 after construction or a reset; the next update moves it to 3, in which the hole can be
     * used. In state 4 the update returns to 3 once the table entry of the com id is clear, and
     * state 5 finishes the gadget; nothing in this file sets 4 or 5 (inferred). */
    /* 0x48 */ s32 m_state;
    /* 0x4C */ Fn8040Vector3 field_0x4C; /* ReadOwner passes it to fn_8040CE60; not read here */
    /* 0x58 */ boost::weak_ptr<CharInfo> field_0x58; /* constructed and destroyed only */
    /* 0x60 */ s32 m_comId;                          /* the owner's com id (fn_800C9E50) */
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(WarpHole::SetWarpHole, 0x68);

} // namespace WarpHole

#endif
