#ifndef GAME_PLAYERATTACK_H
#define GAME_PLAYERATTACK_H

namespace Player {
/* RTTI names the task whose constructor registers it at PlayerBase +0x234. Its layout is not
 * recovered here. */
class Attack;
} // namespace Player

/* Placeholder names and signatures inferred from the bodies and motion-event caller. */
extern "C" {
void fn_8007792C(Player::Attack *attack);
/* The event caller passes arg1; the body does not read it. */
void fn_80077974(Player::Attack *attack, int arg0, int arg1, float arg2);
}

#endif
