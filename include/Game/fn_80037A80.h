#ifndef GAME_FN_80037A80_H
#define GAME_FN_80037A80_H

struct RpClump;

/* Calls fn_80037B54 for each atomic of clump with resource as its data; does nothing
 * when clump is null (behaviour inferred). */
extern "C" void fn_80037A80(RpClump *clump, void *resource);

#endif
