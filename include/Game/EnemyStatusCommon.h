#ifndef GAME_ENEMYSTATUSCOMMON_H
#define GAME_ENEMYSTATUSCOMMON_H

#include "Game/EnemyStatFunc.h"
#include "Game/EnemyReferer.h"
#include "layout_check.h"

/* Argument of slot 0xD0 (placeholder name). Its implementation reads a
 * SonicteamUSA::System::Collision::CollisionResultList pointer from the first word; this
 * argument's own type and whether slot 0xD0 takes it by pointer or reference are not recovered. */
struct Unk801A8258Argument;

/* Original RTTI name. Its vtable replaces all 44 EnemyStatFunc slots, then adds a pure slot (0xB8,
 * named after the first known implementation, BkWingSmallBase's), the virtual destructor (0xBC)
 * and five slots; the parameter lists of 0xC0-0xCC are not recovered. */
class EnemyStatusCommon : public virtual EnemyReferer, public EnemyStatFunc {
  public:
    /* The original (0x8018F294) constructs and initialises the members. */
    EnemyStatusCommon();

    // Matching workaround: slots 0xB8 and 0xBC are declared before the EnemyStatFunc overrides.
    // MWCC creates a derived class's adjustment thunks in this class's declaration order: this
    // order gives BkWorm's thunks (0x8019339C-0x801933BC, USA) their original order, which
    // declaring the added slots after the overrides does not. The slot offsets are the same
    // either way. Revisit if a later file shows a different order.
    virtual int fn_80191A58() = 0;
    virtual ~EnemyStatusCommon();
    virtual void fn_801A9138();
    virtual void fn_801A8CE8();
    virtual void fn_801A86A8(float);
    virtual void fn_801A8440();
    virtual void fn_801A83D8(float);
    virtual void fn_801A833C();
    virtual bool fn_8017B61C(int, bool);
    virtual int fn_8017B6F8();
    virtual unsigned char fn_8017B708();
    virtual unsigned char fn_8017B700();
    virtual void fn_8017B710(unsigned char);
    virtual unsigned char fn_8017B720();
    virtual void fn_801A7FA0();
    virtual void fn_8017B718(unsigned char);
    virtual unsigned char fn_8017B728();
    virtual bool fn_801A8494();
    virtual unsigned char fn_8017B730();
    virtual unsigned char fn_8017B738();
    virtual unsigned char fn_8017B740();
    virtual unsigned char fn_8017B748();
    virtual unsigned char fn_8017B750();
    virtual unsigned char fn_8017B758();
    virtual unsigned char fn_8017B760();
    virtual unsigned char fn_8017B768();
    virtual unsigned char fn_8017B770();
    virtual unsigned char fn_8017B778();
    virtual void fn_801A8020(SonicteamUSA::System::CharaColli::AttackCallbackParam *);
    virtual int fn_8017B780();
    virtual bool fn_801A8388(float);
    virtual void fn_8017B7D8(unsigned char, unsigned char);
    virtual void fn_8017B788(unsigned char);
    virtual void fn_8017B790(unsigned int);
    virtual void fn_8017B7B4(unsigned int);
    virtual void fn_8017B7FC(bool);
    virtual void fn_801A7F64(bool);
    virtual void fn_801A8608(int);
    virtual void fn_801A85B4(const boost::shared_ptr<CharInfo> &);
    virtual void fn_801A8504(const boost::shared_ptr<CharInfo> &);
    virtual void fn_801A84DC();
    virtual void fn_801A84D8();
    virtual UnkBitFlags_80014E04 *fn_8017B83C();
    virtual float fn_8017B844();
    virtual float fn_8017B84C();
    virtual void fn_8017B854(float);
    virtual void fn_801A84D4();
    virtual int fn_8017B8CC();
    virtual bool fn_801A84AC();
    virtual float fn_8017B8D4();
    virtual void fn_801A8258(Unk801A8258Argument *);

    // Unclassified storage between the virtual-base link at 0x4 and the end of the class.
    /* 0x08 */ unsigned char field_0x8[0x90];

  private:
    // Not copyable while the shared owners in this storage (the members at 0x08 and 0x64, whose
    // destructors release shared counts) are stored as bytes (declared only, as in C++03).
    EnemyStatusCommon(const EnemyStatusCommon &);
    EnemyStatusCommon &operator=(const EnemyStatusCommon &);
};
CHECK_SIZEOF(EnemyStatusCommon, 0x9C);

#endif
