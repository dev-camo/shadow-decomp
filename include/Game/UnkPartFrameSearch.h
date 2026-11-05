#ifndef GAME_UNKPARTFRAMESEARCH_H
#define GAME_UNKPARTFRAMESEARCH_H

#include "rw/rwcore.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* What the ELEC_FIREWALL and ELEC_CORE gadgets pass to GadgetModelInfo::fn_8015FA78 as the user
 * data of their part callbacks (fn_801F7778 and fn_801F6240, one copy in each file): the id of the
 * part to look for and the frame found (placeholder names). UnkFrameSearch_80169E8C has the same
 * layout but another meaning: its callback looks up a frame by id below the clump's frame, where
 * these callbacks compare the id with the part's word at 0x8 and take the clump's frame. */
struct UnkPartFrameSearch {
    UnkPartFrameSearch() : id(0), frame(0) {}

    /* 0x0 */ s32 id;
    /* 0x4 */ RwFrame *frame;
};
CHECK_SIZEOF(UnkPartFrameSearch, 0x8);

#endif
