#ifndef GAME_BOSSCHARINFO_H
#define GAME_BOSSCHARINFO_H

#include "Game/CharInfo.h"
#include "layout_check.h"

/* BossCharInfo (class name from RTTI, with CharInfo as its only base): the 0x6C-byte character
 * info of a boss. The constructor (0x80233158) passes arg0 to CharInfo. */
class BossCharInfo : public CharInfo {
  public:
    BossCharInfo(int arg0);
    virtual ~BossCharInfo();

    /* An 8-byte smart-pointer-like handle (default constructor fn_8007A380, assigned a new
     * 16-byte object). */
    /* 0x64 */ u8 field_0x64[0x6C - 0x64];
};
CHECK_SIZEOF(BossCharInfo, 0x6C);

#endif
