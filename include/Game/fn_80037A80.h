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

/* Sets every delta morph target weight of each atomic of clump to value, clamped to 0..1; does
 * nothing when clump is null (behaviour inferred: it passes the clamped value to fn_80037AB4,
 * which stores it in each weight that fn_804442E4 returns for the atomic). The MORPHDOOR gadget
 * passes its morph time fraction. */
extern "C" void fn_800379E8(RpClump *clump, float value);

#endif
