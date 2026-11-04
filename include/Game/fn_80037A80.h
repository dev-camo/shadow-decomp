#ifndef GAME_FN_80037A80_H
#define GAME_FN_80037A80_H

struct RpClump;
struct RwRGBA;

/* Calls fn_80037B54 for each atomic of clump with resource as its data; does nothing
 * when clump is null (behaviour inferred). */
extern "C" void fn_80037A80(RpClump *clump, void *resource);

/* Calls fn_80037BB0 for each atomic of clump with color as its data (behaviour inferred;
 * ElecFanDraw passes the colour it fades the clump with). */
extern "C" void fn_80037B84(RpClump *clump, const RwRGBA *color);

#endif
