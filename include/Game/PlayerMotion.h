#ifndef GAME_PLAYERMOTION_H
#define GAME_PLAYERMOTION_H

#include "Game/Task.h"
#include "layout_check.h"

namespace Player {

/* Player::PlayerMotion (class name from RTTI, with Task as its base): the object that the pointer
 * at 0x230 of a Player::PlayerBase refers to. Its destructor (0x800AA94C) clears that pointer in
 * the player that the shared pointer at 0x58 refers to. Player::Npc::Sonic::MotionImpl derives
 * from it through Player::Npc::NpcMotion (RTTI base list at 0x8051FB9C). This is a view: only the
 * first virtual function that it adds to Task's is declared, and its own members and full size
 * are not recovered.
 *
 * The function is pure in PlayerMotion's vtable (0x80521A84) and is named after its
 * implementation in Player::Npc::Sonic::MotionImpl's vtable (0x8051FBB8). Its purpose is unknown;
 * Player::Behavior::Super passes 0x2B and 10.0f, and the parameter types and the void return are
 * inferred from that call. */
class PlayerMotion : public Task {
  public:
    virtual void fn_8006CB7C(int arg0, float arg1) = 0;

  protected:
    /* The destructor at 0x800AA94C (not recovered): it reads the shared owner pointer at 0x58,
     * which this view does not declare, and releases it. Declared without a body so that code
     * holding this borrowed view cannot destroy it. */
    virtual ~PlayerMotion();

  private:
    /* Not copyable through this view: its members after the Task part are not recovered. Declared
     * and not defined. */
    PlayerMotion(const PlayerMotion &);
    PlayerMotion &operator=(const PlayerMotion &);
};
/* Checks only the declared view (the Task part); the full size is unknown. */
CHECK_SIZEOF(PlayerMotion, 0x28);

} // namespace Player

#endif
