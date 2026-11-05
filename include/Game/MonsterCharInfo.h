#ifndef GAME_MONSTERCHARINFO_H
#define GAME_MONSTERCHARINFO_H

#include "Game/GadgetCharInfo.h"
#include <dolphin/types.h>
#include "layout_check.h"

class Monster;
class MonsterControl;

/* MonsterCharInfo (class name from RTTI, with CharInfo and GadgetCharInfo as its bases): the
 * 0x70-byte character info that the Monster gadget's constructor creates with the constructor
 * argument 0x4B and a pointer to the gadget at 0x6C. Only that pointer is mapped. Its methods are
 * defined in Monster.cpp. */
class MonsterCharInfo : public GadgetCharInfo {
  public:
    MonsterCharInfo(Monster *monster) : GadgetCharInfo(0x4B), m_monster(monster) {}
    virtual ~MonsterCharInfo();

    /* Passes kind to the monster's OnControl, then calls control's DisableCollision (the
     * original name is unknown). The int type of kind is inferred: the body only passes it on,
     * and OnControl's switch compares it with signed compares. */
    void fn_80139F98(int kind, MonsterControl *control);

    /* 0x6C */ Monster *m_monster;
};
CHECK_SIZEOF(MonsterCharInfo, 0x70);

#endif
