#ifndef GAME_UNKMANAGER_80009548_H
#define GAME_UNKMANAGER_80009548_H

#include "Game/fn_8040_vector_types.h"

/* The object that fn_80009548 returns: a static object (at 0x8056EE14 in the USA release) that
 * the function constructs and registers for destruction on first use. Its class and the original
 * function names are unknown. */
class UnkManager_80009548;

extern "C" {
/* Returns the object, constructing it on first use. */
UnkManager_80009548 *fn_80009548(void);
/* Behaviour inferred from the CityLaser gadget's call only: three floats and an integer (0). */
void fn_8000D664(UnkManager_80009548 *manager, int arg1, float arg2, float arg3, float arg4);
/* Returns a vector that the BDTrue gadget uses as angles (behaviour inferred). */
Fn8040Vector3 fn_80109B80(UnkManager_80009548 *manager, int arg1);
/* Returns the address of a position that the BDHologram gadget measures its distance to; it
 * passes 0 for arg1 (behaviour inferred: the function looks the entry up and returns the address
 * 8 bytes into it). */
Fn8040Vector3 *fn_80010244(UnkManager_80009548 *manager, int arg1);
}

#endif
