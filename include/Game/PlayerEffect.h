#ifndef GAME_PLAYEREFFECT_H
#define GAME_PLAYEREFFECT_H

namespace Player {
/* Class name from RTTI; the task registered at PlayerBase +0x23C. Only borrowed pointers are
 * exposed until its owning file is recovered. */
class Effect;
} // namespace Player

/* Placeholder names and signatures inferred from the bodies and motion-event caller. */
extern "C" {
void fn_800A2978(Player::Effect *effect, void *payload);
void fn_800A29C0(Player::Effect *effect, int arg0);
}

#endif
