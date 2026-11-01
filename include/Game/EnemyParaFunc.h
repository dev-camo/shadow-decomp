#ifndef GAME_ENEMYPARAFUNC_H
#define GAME_ENEMYPARAFUNC_H

#include "layout_check.h"

/* Original type name unknown. The parameter setters retain a borrowed pointer to
 * an immutable per-enemy record; the record layout is not needed here. */
struct UnkEnemyParaData_8017B8E4;

/* EnemyParaFunc is the original RTTI name. Method names identify the base
 * implementations in its original vtable. Query types are inferred from the
 * implementations and their consumers; no semantic getter names are claimed. */
class EnemyParaFunc {
  public:
    /* 0x08 */ virtual int fn_8019B464();
    /* 0x0C */ virtual int fn_8019B46C();
    /* 0x10 */ virtual int fn_8019B474();
    /* 0x14 */ virtual int fn_8019B47C();
    /* 0x18 */ virtual int fn_8019B484();
    /* 0x1C */ virtual const float *fn_8019B48C();
    /* 0x20 */ virtual float fn_80181F04();
    /* 0x24 */ virtual float fn_8019B494();
    /* 0x28 */ virtual float fn_8019B49C();
    /* 0x2C */ virtual float fn_8019B4A4();
    /* 0x30 */ virtual float fn_8019B4AC();
    /* 0x34 */ virtual float fn_8019AD04();
    /* 0x38 */ virtual int fn_8019B4B4();
    /* 0x3C */ virtual int fn_80192B58();
    /* 0x40 */ virtual void fn_8019B4BC(const UnkEnemyParaData_8017B8E4 *data);
    /* 0x44 */ virtual const UnkEnemyParaData_8017B8E4 *fn_8019B4C0();
    /* 0x48 */ virtual int fn_8017E14C();
    /* 0x4C */ virtual int fn_8017EE98();
    /* 0x50 */ virtual int fn_8019B4C8();
    /* 0x54 */ virtual int fn_8018019C();
    /* 0x58 */ virtual int fn_8017FA78();
    /* 0x5C */ virtual int fn_8019B4D0();
    /* 0x60 */ virtual int fn_80180C58();
    /* 0x64 */ virtual int fn_8019B4D8();
    /* 0x68 */ virtual float fn_8019B4E0();
    /* 0x6C */ virtual float fn_8019B4E8();
    /* 0x70 */ virtual const float *fn_8019B4F0();
    /* 0x74 */ virtual int fn_8019B4F8();
};
CHECK_SIZEOF(EnemyParaFunc, 0x4);

#endif
