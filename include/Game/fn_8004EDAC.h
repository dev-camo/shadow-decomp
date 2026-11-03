#ifndef GAME_FN_8004EDAC_H
#define GAME_FN_8004EDAC_H

#include "Game/UnkTick.h"

class Task;

extern "C" {
/* Source interfaces follow the observed call ABI; original names and signatures are unknown. */
void fn_8004EDAC(Task *task, float dt);
UnkTick fn_8004EE64(const UnkTick &second, const UnkTick &first);
}

#endif
