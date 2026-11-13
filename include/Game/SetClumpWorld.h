#ifndef GAME_SETCLUMPWORLD_H
#define GAME_SETCLUMPWORLD_H

#include "rw/rpworld.h"

/* Puts clump in world: unless it is there already, removes it from the world it is in, if any,
 * and adds it to world (behaviour from the code at its uses).
 * Matching workaround: an inline function whose name, header and form as a free function are
 * inferred; the original has no out-of-line copy of it. Its boundary shows in the world getter
 * fn_800593D0 (Game/fn_8003A840.h): the original calls the getter's out-of-line copy at eleven
 * places, each directly followed by this code with the result as the world, and expands the
 * getter everywhere else. The game is compiled with an inline depth of one, where a call in the
 * argument of an inline function's call stays a call. Written as
 * SetClumpWorld(fn_800593D0(...), clump), the functions at USA (GUPE8P) 0x8005930C, 0x8016F88C
 * and 0x802DB2FC, which have no source file yet, compile to the original's words; with the code
 * written out in the caller the getter is expanded. The order of the parameters comes from
 * those functions too: an inline call's arguments are evaluated last to first, and the original
 * reads the clump before it calls the getter. Where the original expands the getter, the world
 * is in a local before the call. Revisit when the original function or its header is found. */
inline void SetClumpWorld(RpWorld *world, RpClump *clump) {
    RpWorld *current = fn_8046256C(clump);
    if (current != world) {
        if (current != 0) {
            fn_804623EC(current, clump);
        }
        fn_804622FC(world, clump);
    }
}

#endif
