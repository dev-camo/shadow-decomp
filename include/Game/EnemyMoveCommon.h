#ifndef GAME_ENEMYMOVECOMMON_H
#define GAME_ENEMYMOVECOMMON_H

#include "Game/EnemyMoveFunc.h"
#include "Game/EnemyReferer.h"
#include "Game/Unk801A5E50.h"
#include "layout_check.h"

/* Original RTTI name. The virtual EnemyReferer base is at 0x64, with its compiler link at 0x4. */
class EnemyMoveCommon : public EnemyMoveFunc, public virtual EnemyReferer {
  public:
    EnemyMoveCommon();

    virtual void fn_8019B438();
    virtual void fn_8019B43C(float deltaTime);
    virtual bool fn_80187CBC() {
        return field_0x5F;
    }
    virtual bool fn_8019B440() {
        return field_0x60;
    }
    virtual bool fn_8019588C();
    virtual bool fn_8019B448();
    virtual float fn_8019B458() {
        return field_0x3C;
    }
    virtual unsigned int fn_8017CAC0() {
        return field_0x4C;
    }
    virtual const Fn8040Vector3 *fn_8017CAC8() {
        return &field_0x40;
    }
    virtual void fn_8019B460();

    Fn8040Vector3 fn_801A54E8(float deltaTime);

    /* 0x08 */ Unk801A5E50 field_0x8;
    /* 0x2C */ Fn8040Vector3 field_0x2C;
    /* 0x38 */ float field_0x38;
    /* 0x3C */ float field_0x3C;
    /* 0x40 */ Fn8040Vector3 field_0x40;
    /* 0x4C */ unsigned int field_0x4C;
    /* 0x50 */ Fn8040Vector3 field_0x50;
    /* 0x5C */ bool field_0x5C;
    /* 0x5D */ bool field_0x5D;
    /* 0x5E */ bool field_0x5E;
    /* 0x5F */ bool field_0x5F;
    /* 0x60 */ bool field_0x60;
};
CHECK_SIZEOF(EnemyMoveCommon, 0x68);

#endif
