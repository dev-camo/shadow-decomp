#ifndef GAME_UNKMANAGER_800CB69C_H
#define GAME_UNKMANAGER_800CB69C_H

#include <dolphin/types.h>

/* The manager returned by fn_800CB69C holds a table of 256 eight-byte entries whose first word is a
 * state that gadgets read and set by id (for example the electric circuit switches). Its class and
 * the original function names are unknown. */
class UnkManager_800CB69C;

extern "C" {
/* Returns the manager, constructing it on first use (behaviour inferred). */
UnkManager_800CB69C *fn_800CB69C(void);
/* Returns the state of entry id, or 0 when there is no table or id is outside 0-255 (behaviour
 * inferred). */
u32 fn_8016904C(UnkManager_800CB69C *manager, int id);
/* Sets the state of entry id; ignored when there is no table or id is outside 0-255 (behaviour
 * inferred). */
void fn_801690D0(UnkManager_800CB69C *manager, int id, u32 state);
}

#endif
