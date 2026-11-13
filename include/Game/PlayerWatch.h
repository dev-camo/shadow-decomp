#ifndef GAME_PLAYERWATCH_H
#define GAME_PLAYERWATCH_H

#include "Game/Player.h"
#include "Game/PlayerBase.h"
#include "Game/Task.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"

/* The two functions of PlayerWatch.cpp that other files call (placeholder names). Both tasks are
 * classes of that file's unnamed namespace, so the result type is their base class (inferred). */
extern "C" {
/* Creates the file's Watch task for player under parent and returns it. A virtual function of
 * Player::PlayerShadow (USA (GUPE8P) 0x800AD3F8) passes the layer 7 parent task and stores the
 * result at 0x268. */
Task *fn_800B8360(Task *parent, const boost::shared_ptr<Player::Player> &player);
/* Creates the file's WaitTask for player under parent and returns it. All 14 callers pass their
 * task pointer at 0x268 as parent and their weak pointer at 0x260, and none of them uses the
 * result. That virtual function passes the task fn_800B8360 returned; the other
 * 13 callers (0x8020F4AC, for example) store the result of fn_80075E30 at 0x268. */
Task *fn_800B8310(Task *parent, const boost::weak_ptr<Player::PlayerBase> &player);
}

#endif
