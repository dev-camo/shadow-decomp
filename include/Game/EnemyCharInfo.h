#ifndef GAME_ENEMYCHARINFO_H
#define GAME_ENEMYCHARINFO_H

#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* Original RTTI name, with CharInfo as its only base: the character info of an enemy
 * (Enemy::m_charInfo). Its constructor (0x801A0280; the caller at 0x801A0048 allocates 0xBC bytes
 * for it) installs the vtable at 0x38 and initialises members from 0x64 through 0xB8, and its
 * destructor (0x801A0218) releases the shared pointer at 0x64. Besides that pointer, the vector at
 * 0x6C and the floats at 0xAC and 0xB4 are mapped; the rest of 0x78 through 0xB8 is not recovered
 * here. */
class EnemyCharInfo : public CharInfo {
  public:
    EnemyCharInfo(int arg0);
    virtual ~EnemyCharInfo();
    /* The overrides in EnemyCharInfo's vtable (0x8019FF24 and 0x8019FD5C), named after the slots
     * they replace. */
    virtual void fn_801DC980(CharCommand *command);
    virtual bool fn_80073634();

    /* Creates a CharCommandUnit in field_0x64 and assigns handler to its handler (0x8019FD64;
     * behaviour from its body, name a placeholder; the const reference is inferred). */
    void fn_8019FD64(const CharCommandHandler &handler);
    /* Returns a float that depends on fn_802039D8 for lbl_8057D6A0's record at 0xA8 (0x8019FFE0);
     * EggLastMech uses it as its update time (behaviour inferred). */
    float fn_8019FFE0();

    /* 0x64 */ boost::shared_ptr<CharCommandUnit> field_0x64;
    /* The constructor copies the position here; EggLastMech's update copies it here before it
     * moves (meaning inferred). */
    /* 0x6C */ Fn8040Vector3 field_0x6C;
    /* 0x78 */ u8 field_0x78[0xAC - 0x78];
    /* 0xAC */ float field_0xAC; /* EggLastMech's constructor stores 700 */
    /* 0xB0 */ u8 field_0xB0[0xB4 - 0xB0];
    /* 0xB4 */ float field_0xB4; /* EggLastMech's constructor stores 2 */
    /* 0xB8 */ u8 field_0xB8[0xBC - 0xB8];
};
CHECK_OFFSETOF(EnemyCharInfo, field_0xAC, 0xAC);
CHECK_OFFSETOF(EnemyCharInfo, field_0xB4, 0xB4);
CHECK_SIZEOF(EnemyCharInfo, 0xBC);

#endif
