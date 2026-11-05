#ifndef GAME_UNKFRAMESEARCH_80169E8C_H
#define GAME_UNKFRAMESEARCH_80169E8C_H

#include "Game/GadgetModelInfo.h"
#include "rw/rwcore.h"
#include "layout_check.h"

/* The user data of the part callback fn_80169E8C (placeholder names). For a part with a clump, the
 * callback looks up the frame with the given id below the clump's frame (fn_80038E84) and stores
 * the result in frame; it returns null once a frame is found and the part otherwise (behaviour
 * inferred). The callers zero both members and then set the id. */
struct UnkFrameSearch_80169E8C {
    UnkFrameSearch_80169E8C() : id(0), frame(0) {}

    /* 0x0 */ int id;
    /* 0x4 */ RwFrame *frame;
};
CHECK_SIZEOF(UnkFrameSearch_80169E8C, 0x8);

extern "C" UnkModelPart_8015FA78 *fn_80169E8C(UnkModelPart_8015FA78 *part, void *userData);

#endif
