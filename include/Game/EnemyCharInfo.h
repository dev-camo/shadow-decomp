#ifndef GAME_ENEMYCHARINFO_H
#define GAME_ENEMYCHARINFO_H

#include "Game/CharInfo.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class CharCommandUnit;

/* Original RTTI name, with CharInfo as its only base: the character info of an enemy
 * (Enemy::m_charInfo). Its constructor (0x801A0280; the caller at 0x801A0048 allocates 0xBC bytes
 * for it) installs the vtable at 0x38 and initialises members from 0x64 through 0xB8, and its
 * destructor (0x801A0218) releases the shared pointer at 0x64. The other members are not
 * recovered here. */
class EnemyCharInfo : public CharInfo {
  public:
    /* 0x64 */ boost::shared_ptr<CharCommandUnit> field_0x64;
    /* 0x6C */ u8 field_0x6C[0xBC - 0x6C];
};
CHECK_SIZEOF(EnemyCharInfo, 0xBC);

#endif
