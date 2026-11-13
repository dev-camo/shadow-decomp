#ifndef GAME_PLAYERMOTIONEVENTARGS_H
#define GAME_PLAYERMOTIONEVENTARGS_H

#include "boost/function.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

namespace Player {

/* The callback binders name this type in RTTI. The event loop copies the halfword id and payload
 * pointer into a temporary before dispatch; member names and this accessed view are inferred. */
struct MotionEventArgs {
    MotionEventArgs(u16 id, void *data) : field_0x0(id), field_0x4(data) {}

    /* 0x0 */ u16 field_0x0;
    /* 0x4 */ void *field_0x4;
};
/* Checks the accessed view; the full event extent remains unknown. */
CHECK_SIZEOF(MotionEventArgs, 0x8);

typedef boost::function1<void, MotionEventArgs &> MotionEventCallback;
CHECK_SIZEOF(MotionEventCallback, 0xC);

} // namespace Player

/* Native callback helpers. Their owning class/template names are not recovered; these signatures
 * are inferred from the receiver at PlayerBase +0x74 and its two-word event temporary. */
extern "C" {
void fn_800AA344(Player::MotionEventCallback *callback, Player::MotionEventArgs &event);
bool fn_800AA370(const Player::MotionEventCallback *callback);
bool fn_800AA390(const Player::MotionEventCallback *callback);
}

#endif
