#ifndef GAME_FN_80169BE4_H
#define GAME_FN_80169BE4_H

#include "Game/fn_8040_vector_types.h"

extern "C" {
/* Returns false when the squared distance from position to the vector that fn_80010244 returns for
 * index 0, or also for index 1 while the byte at 0xB2 of lbl_8057D6A0 is set, is below
 * squaredDistance (behaviour inferred from its body; the original name is unknown). */
bool fn_80169BE4(const Fn8040Vector3 *position, float squaredDistance);
}

#endif
