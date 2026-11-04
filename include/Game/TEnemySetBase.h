#ifndef GAME_TENEMYSETBASE_H
#define GAME_TENEMYSETBASE_H

#include "Game/EnemySetFunc.h"
#include "Game/EnemyReferer.h"
#include "layout_check.h"

/* Original RTTI name. Replaces the first nine EnemySetFunc slots and adds the virtual destructor
 * (0x30). */
class TEnemySetBase : public virtual EnemyReferer, public EnemySetFunc {
  public:
    /* The original (0x801A7400) stores 1 at 0x8, clears the two words at 0xC (0x8003A118) and
     * clears the bytes at 0x14 and 0x15. */
    TEnemySetBase();

    virtual int fn_8018EBA0();
    virtual int fn_8019B350();
    virtual void fn_80180BE4();
    virtual void fn_8019B358();
    virtual void fn_80187CB8();
    virtual void fn_8019B35C(UnkGadgetOwner_800C9CD0 *owner);
    virtual void fn_8019B360(UnkGadgetOwner_800C9CD0 *owner);
    virtual void fn_8019B364(float);
    virtual int fn_8019B368();
    virtual ~TEnemySetBase();

    /* 0x08 */ int field_0x8; // read by this class's slot 0x28 (0x8017B554)
                              // Unclassified storage; the type of the two words is not recovered.
    /* 0x0C */ unsigned char field_0xC[0x8];
    /* 0x14 */ unsigned char field_0x14; // returned by slot 0x08 (0x8017B538), set by 0x10
    /* 0x15 */ unsigned char field_0x15; // returned by this class's slot 0x0C (0x8017B540)

  private:
    // Not copyable while the shared owner at 0x0C (its destructor at 0x80039FE8 releases a
    // shared count) is stored as bytes (declared only, as in C++03).
    TEnemySetBase(const TEnemySetBase &);
    TEnemySetBase &operator=(const TEnemySetBase &);
};
CHECK_OFFSETOF(TEnemySetBase, field_0x8, 0x8);
CHECK_OFFSETOF(TEnemySetBase, field_0x15, 0x15);
CHECK_SIZEOF(TEnemySetBase, 0x1C);

#endif
