#ifndef GAME_ENEMYDISPCOMMON_H
#define GAME_ENEMYDISPCOMMON_H

#include "Game/EnemyDispFunc.h"
#include "Game/EnemyReferer.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class UnkObject_802B4254;

/* Original RTTI name. Its vtable replaces all 21 EnemyDispFunc slots, then adds one slot whose
 * parameter list is not recovered (0x5C), the virtual destructor (0x60) and a pure slot (0x64,
 * named after the first known implementation, BkWingSmallBase's). */
class EnemyDispCommon : public virtual EnemyReferer, public EnemyDispFunc {
  public:
    /* The original (0x801A1CB4) constructs the owning members and stores the flags and floats. */
    EnemyDispCommon();

    virtual void fn_8019B3A0();
    virtual void fn_8019B3A4(float);
    virtual void fn_8019B3A8(float);
    virtual const Fn8040Matrix34 *fn_8019B3AC();
    virtual Unk802CD448Data *fn_8019B3B4();
    virtual Unk801A4430 *fn_8017CC74();
    virtual boost::shared_ptr<Unk802CD448> fn_80181038();
    virtual Fn8040Matrix34 *fn_8019448C(unsigned int);
    virtual Fn8040Vector3 fn_8019B3BC();
    virtual Fn8040Vector3 fn_8019B3DC();
    virtual void fn_80188E0C(float);
    virtual float fn_8019B3FC();
    virtual void fn_8019B404(const Fn8040Vector3 &, Fn8040Vector3);
    virtual void fn_8017F234(bool);
    virtual void fn_8019B408(const Unk801A4430Event &event);
    virtual bool fn_8019B40C(UnkEnemyDispEvent_8019B40C *event);
    virtual void fn_8019B414();
    virtual void fn_801834D4();
    virtual void fn_801812C8();
    virtual void fn_8019B418();
    virtual void fn_8019B41C(bool);
    virtual void fn_801A128C();
    virtual ~EnemyDispCommon();
    virtual void fn_801919C0() = 0;

    /* 0x14 */ boost::shared_ptr<Unk802CD448> field_0x14;
    /* 0x1C */ float field_0x1C;
    // Flags (bool inferred); the constructor sets all three. Slots 0x3C and 0x58 store their
    // arguments in 0x21 (0x801A0B10) and 0x22 (0x8017B614).
    /* 0x20 */ bool field_0x20;
    /* 0x21 */ bool field_0x21;
    /* 0x22 */ bool field_0x22;
    // A shared owner whose pointee is not recovered: the constructor clears both words
    // (0x801A1D54) and the destructor releases its count (0x800BEE0C).
    /* 0x24 */ unsigned char field_0x24[0x8];
    /* 0x2C */ boost::shared_ptr<UnkObject_802B4254> field_0x2C;
    /* 0x34 */ float field_0x34;
    /* 0x38 */ float field_0x38;
    /* 0x3C */ float field_0x3C; // slots 0x34 and 0x30 read and write it (0x801A0BAC, 0x801A0BB4)

  private:
    // Not copyable while the owner at 0x24 is stored as bytes (declared only, as in C++03).
    EnemyDispCommon(const EnemyDispCommon &);
    EnemyDispCommon &operator=(const EnemyDispCommon &);
};
CHECK_OFFSETOF(EnemyDispCommon, field_0x1C, 0x1C);
CHECK_OFFSETOF(EnemyDispCommon, field_0x22, 0x22);
CHECK_OFFSETOF(EnemyDispCommon, field_0x2C, 0x2C);
CHECK_OFFSETOF(EnemyDispCommon, field_0x3C, 0x3C);
CHECK_SIZEOF(EnemyDispCommon, 0x44);

#endif
