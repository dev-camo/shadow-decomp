#ifndef GAME_UNKMANAGER_80078C88_H
#define GAME_UNKMANAGER_80078C88_H

/* The object that fn_80078C88 returns (placeholder class name; the original class is unknown): a
 * static object (at 0x805EE37C in the USA release, GUPE8P) that the function constructs
 * (fn_8027AC3C) on the first call, registering its destructor (fn_8027AB5C), and caches in a
 * static pointer. Its layout is not mapped. */
class UnkManager_80078C88;

/* The names, the const qualifiers and the void returns are not known (inferred); the behaviour
 * comes from the bodies. Player::Behavior::Super passes index 0xE to the last three. */
extern "C" {
UnkManager_80078C88 *fn_80078C88(void);
/* True when fn_80012408 returns 3 for fn_800122D4's result and fn_8027AB1C(manager, index). */
bool fn_8027AAA4(UnkManager_80078C88 *manager, int index);
/* Call fn_8027AD6C (fn_8027AA4C) or fn_8027ADC0 (fn_8027AA78) for entry index of the array of
 * 0x10-byte entries that the manager's first word points to. */
void fn_8027AA4C(UnkManager_80078C88 *manager, int index);
void fn_8027AA78(UnkManager_80078C88 *manager, int index);
}

#endif
