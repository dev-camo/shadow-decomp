#ifndef GAME_UNKCHARINFOTABLE_800DDF20_H
#define GAME_UNKCHARINFOTABLE_800DDF20_H

#include "Game/CharInfo.h"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

/* An entry of the list that fn_801DCB14 builds (behaviour inferred): a weak pointer to a
 * character info, followed by the links to the next and the previous entry (prev inferred: fn_801DCB14
 * stores the preceding entry there). The name is a placeholder. */
struct UnkCharInfoNode_801DCB14 {
    /* 0x00 */ boost::weak_ptr<CharInfo> charInfo;
    /* 0x08 */ UnkCharInfoNode_801DCB14 *next;
    /* 0x0C */ UnkCharInfoNode_801DCB14 *prev;
};
CHECK_SIZEOF(UnkCharInfoNode_801DCB14, 0x10);

/* The table of 256 character infos that a function-local static of fn_800DDF20 holds (behaviour
 * inferred), with a placeholder name. */
class UnkCharInfoTable_800DDF20;

extern "C" {
/* Returns the table, constructing it on first use (behaviour inferred). */
UnkCharInfoTable_800DDF20 *fn_800DDF20(void);
/* Returns the first entry of a list of the table's character infos whose word at 0x50 (the
 * constructor argument) equals value, or null when there is none (behaviour inferred). */
UnkCharInfoNode_801DCB14 *fn_801DCB14(UnkCharInfoTable_800DDF20 *table, int value);
}

#endif
