#ifndef GAME_MONSTERCHARINFO_H
#define GAME_MONSTERCHARINFO_H

#include "Game/GadgetCharInfo.h"
#include <dolphin/types.h>
#include "layout_check.h"

class Monster;
class MonsterControl;

/* MonsterCharInfo (class name from RTTI, with CharInfo and GadgetCharInfo as its bases): the
 * 0x70-byte character info that the Monster gadget's constructor creates with the constructor
 * argument 0x4B and a pointer to the gadget at 0x6C. Only that pointer is mapped. */
class MonsterCharInfo : public GadgetCharInfo {
  public:
    /* Passes kind to the monster's function at 0x801389D0, then calls control's
     * DisableCollision (behaviour inferred; the original name is unknown). The int type of kind is
     * inferred too (the body only passes it on, and 0x801389D0 compares it with cmpwi): re-check
     * the mangled name when 0x80139F98 is sourced. */
    void fn_80139F98(int kind, MonsterControl *control);

    /* 0x6C */ Monster *m_monster;
};
CHECK_SIZEOF(MonsterCharInfo, 0x70);

#endif
