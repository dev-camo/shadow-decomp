#ifndef GAME_UNKCHARINFOTABLE_800DDF20_H
#define GAME_UNKCHARINFOTABLE_800DDF20_H

#include "Game/CharInfo.h"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

/* An entry of the list that fn_801DCB14 builds (behaviour inferred): a weak pointer to a
 * character info, followed by the links to the next and the previous entry (prev inferred:
 * fn_801DCB14 stores the preceding entry there). The name is a placeholder. */
struct UnkCharInfoNode_801DCB14 {
    /* 0x00 */ boost::weak_ptr<CharInfo> charInfo;
    /* 0x08 */ UnkCharInfoNode_801DCB14 *next;
    /* 0x0C */ UnkCharInfoNode_801DCB14 *prev;
};
CHECK_SIZEOF(UnkCharInfoNode_801DCB14, 0x10);

/* The table of 256 character infos that a function-local static of fn_800DDF20 holds (behaviour
 * inferred), with a placeholder name. The constructor (0x801DCF3C) constructs the three arrays,
 * clears the first two with memset and stores the address of each of the eight entries at 0x1000
 * in its own prev link (next stays 0); the destructor (0x801DCE64) destroys the arrays. The eight
 * entries are the heads of lists keyed by the character info's word at 0x5C (fn_801DCD8C and
 * fn_801DCCF4 index them with it; inferred), which Tornado::Tornado sets to 4 through
 * SetField0x5C. fn_801DCB14 builds its list in the entries at 0x1080. */
class UnkCharInfoTable_800DDF20 {
  public:
    UnkCharInfoTable_800DDF20();
    ~UnkCharInfoTable_800DDF20();

    /* 0x0000 */ UnkCharInfoNode_801DCB14 field_0x0[256];
    /* 0x1000 */ UnkCharInfoNode_801DCB14 field_0x1000[8];
    /* 0x1080 */ UnkCharInfoNode_801DCB14 field_0x1080[256];
};
CHECK_SIZEOF(UnkCharInfoTable_800DDF20, 0x2080);

extern "C" {
/* Returns the table, constructing it on first use (behaviour inferred). */
UnkCharInfoTable_800DDF20 *fn_800DDF20(void);
/* Returns the first entry of a list of the table's character infos whose word at 0x50 (the
 * constructor argument) equals value, or null when there is none (behaviour inferred). */
UnkCharInfoNode_801DCB14 *fn_801DCB14(UnkCharInfoTable_800DDF20 *table, int value);
/* Adds charInfo to the table, and removes it again (behaviour inferred; the Tornado and Monster
 * gadgets add their character info when they are constructed and remove it when they are
 * destroyed). fn_801DCD8C returns whether it added it, false when it finds no free entry
 * (inferred; the Monster gadget ignores the result). */
bool fn_801DCD8C(UnkCharInfoTable_800DDF20 *table, boost::shared_ptr<CharInfo> charInfo);
void fn_801DCCF4(UnkCharInfoTable_800DDF20 *table, boost::shared_ptr<CharInfo> charInfo);
}

#endif
