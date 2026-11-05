#ifndef GAME_FN_8012773C_H
#define GAME_FN_8012773C_H

extern "C" {
/* Turns the angle current towards target by at most step (behaviour inferred from its body; the
 * original name is unknown). The difference target - current is wrapped into the range -pi to pi
 * (2 pi added or subtracted once); when it is within step, returns target itself, not wrapped.
 * Otherwise returns current moved by step towards target, wrapped the same way. */
float fn_8012773C(float target, float current, float step);
}

#endif
