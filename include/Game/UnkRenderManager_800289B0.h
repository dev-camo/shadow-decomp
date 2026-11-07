#ifndef GAME_UNKRENDERMANAGER_800289B0_H
#define GAME_UNKRENDERMANAGER_800289B0_H

#include <dolphin/types.h>
#include "layout_check.h"

class UnkRenderManager_800289B0;

extern "C" {
/* Returns the manager (behaviour inferred): constructs a function-local static object
 * (fn_800487E4) on first use, registers its destructor and caches its address in a static
 * pointer. */
UnkRenderManager_800289B0 *fn_800289B0(void);
/* Stores index in the byte at 0x456 when it lies in 0..16 (behaviour from its body; the meaning
 * of the index is unknown). */
void fn_80048390(UnkRenderManager_800289B0 *manager, s8 index);
/* When index differs from the byte at 0x454, passes the two 16-byte records of the 0x34-byte
 * entry index (at 0x20 + 0x34 * index) to fn_80048238 and the entry's two words, as floats, to
 * fn_8004815C, and stores index at 0x454 (behaviour from its body; purpose unknown). */
void fn_800482E0(UnkRenderManager_800289B0 *manager, s8 index);
/* Copies the byte at 0x455 to 0x456 when it lies in 0..16, then calls fn_800477BC (behaviour
 * from its body; purpose unknown). */
void fn_80047780(UnkRenderManager_800289B0 *manager);
}

/* The object that fn_800289B0 returns (placeholder name; its size is unknown and only the byte
 * the gadgets read is mapped). The drawing code selects an index with fn_80048390 and applies it
 * with ApplyIndex, then restores it with fn_80047780. Besides the RING gadget and
 * ScatteringRing::OnDraw, eight functions (fn_800382DC, fn_800383B4, fn_800384F4, fn_800386C4,
 * fn_80038AB8, fn_801641EC, fn_80238A98 and fn_80357BF4) follow fn_80048390 with the same three
 * instructions (call fn_800289B0, load the byte at 0x456, call fn_800482E0), which
 * suggests the inline below. */
class UnkRenderManager_800289B0 {
  public:
    /* Passes the index at 0x456 to fn_800482E0 (name inferred). */
    void ApplyIndex() {
        fn_800482E0(this, field_0x456);
    }

    /* 0x000 */ u8 field_0x0[0x456];
    /* 0x456 */ s8 field_0x456; /* the index that fn_80048390 stores */
};
CHECK_OFFSETOF(UnkRenderManager_800289B0, field_0x456, 0x456);
/* The size of the mapped view only; the full object may be larger. */
CHECK_SIZEOF(UnkRenderManager_800289B0, 0x457);

#endif
