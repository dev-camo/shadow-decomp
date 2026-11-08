#ifndef GAME_ENEMYMOTIONEVENTARG_H
#define GAME_ENEMYMOTIONEVENTARG_H

#include <dolphin/types.h>
#include "layout_check.h"

/* EnemyMotionEventArg: the event that Unk801A4430 passes to its callback, an event id and a
 * payload. The name comes from the RTTI names of the callback binders of EnemyDispCommon and of the
 * EggLastMech boss (boost::_mfi::mf1<void, ..., EnemyMotionEventArg &>); this compiler writes the
 * underlying type into such names for a typedef, so it is a type of its own. The handlers take it
 * by non-const reference, so the callback is a boost::function1<void, EnemyMotionEventArg &>. Only
 * the accessed prefix is known: Unk801A4430's event loop stores a halfword at 0x0 and a pointer at
 * 0x4 of its stack copy (USA 0x801A4A04 and 0x801A4A0C) and the boss's handler reads them; the full
 * native size is not recovered (four unused bytes follow that copy before the saved registers,
 * which the type may or may not cover). The size check below checks this view. The member names
 * are placeholders. */
struct EnemyMotionEventArg {
    EnemyMotionEventArg(u16 id, void *data) : field_0x0(id), field_0x4(data) {}

    /* 0x0 */ u16 field_0x0;   /* the event id */
    /* 0x4 */ void *field_0x4; /* the payload */
};
CHECK_SIZEOF(EnemyMotionEventArg, 0x8);

#endif
