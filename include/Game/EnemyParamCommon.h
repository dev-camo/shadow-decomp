#ifndef GAME_ENEMYPARAMCOMMON_H
#define GAME_ENEMYPARAMCOMMON_H

#include "Game/EnemyParaFunc.h"
#include "Game/EnemyReferer.h"
#include "Game/UnkEnemyParaData_8017B8E4.h"

/* Original class and base names come from RTTI. */
class EnemyParamCommon : public virtual EnemyReferer, public EnemyParaFunc {
  public:
    EnemyParamCommon();
    ~EnemyParamCommon();

    virtual int fn_8019B464() const;
    virtual const char *fn_8019B46C() const;
    virtual const char *fn_8019B474() const;
    virtual const char *fn_8019B47C() const;
    virtual const char *fn_8019B484() const;
    virtual const float *fn_8019B48C() const;
    virtual float fn_80181F04() const;
    virtual float fn_8019B494() const;
    virtual float fn_8019B49C() const;
    virtual float fn_8019B4A4() const;
    virtual float fn_8019B4AC() const;
    virtual float fn_8019AD04() const;
    virtual int fn_8019B4B4() const;
    virtual int fn_80192B58() const;
    virtual void fn_8019B4BC(const UnkEnemyParaData_8017B8E4 *data);
    virtual const UnkEnemyParaData_8017B8E4 *fn_8019B4C0();
    virtual int fn_8017E14C();
    virtual int fn_8017EE98();
    virtual int fn_8019B4C8();
    virtual int fn_8018019C();
    virtual int fn_8017FA78();
    virtual int fn_8019B4D0();
    virtual int fn_80180C58();
    virtual int fn_8019B4D8();
    virtual float fn_8019B4E0() const;
    virtual float fn_8019B4E8() const;
    virtual const float *fn_8019B4F0() const;
    virtual const UnkEnemyParaEntry_801A6230 *fn_8019B4F8() const;

    /* 0x08 */ const UnkEnemyParaData_8017B8E4 *field_0x8;
};
CHECK_SIZEOF(EnemyParamCommon, 0x10);

/* These parameter tables also have original callers outside this unit. */
extern const float lbl_804CC424[15][7];
extern const float lbl_804CC604[15][3];

unsigned short fn_801A61E4(int kind);

#endif
