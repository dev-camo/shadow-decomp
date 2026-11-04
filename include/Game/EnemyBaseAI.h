#ifndef GAME_ENEMYBASEAI_H
#define GAME_ENEMYBASEAI_H

#include "Game/EnemyCtrlFunc.h"
#include "Game/EnemyReferer.h"
#include "layout_check.h"

class CharCommand;

/* Original RTTI name. Replaces all seven EnemyCtrlFunc slots, then adds the virtual destructor
 * (0x24) and fifteen slots. Method placeholders name the base implementations in its original
 * vtable. Slot 0x28 is bound as a boost::function1<void, CharCommand &> handler (0x8019EF04); the
 * parameter lists and returns of the other added slots are not recovered. */
class EnemyBaseAI : public virtual EnemyReferer, public EnemyCtrlFunc {
  public:
    /* The original (0x8019EE18) also constructs the members and binds slot 0x28. */
    EnemyBaseAI();

    virtual void fn_8019B420();
    virtual void fn_8019B424(float);
    virtual const Fn8040Vector3 *fn_8017C3C8();
    virtual const Fn8040Vector3 *fn_8019B428();
    virtual float fn_8019B430();
    virtual Fn8040Vector3 fn_80196628(float);
    virtual void fn_8017C680();
    virtual ~EnemyBaseAI() {}
    virtual void fn_8019DC9C(CharCommand &command);
    virtual void fn_8019E798();
    virtual void fn_8019E794();
    virtual void fn_8019D504();
    virtual void fn_8019D4D8();
    virtual void fn_8019D428();
    virtual void fn_8019D378();
    virtual void fn_8019D278();
    virtual void fn_8019D1C8();
    virtual void fn_8019D0C8();
    virtual void fn_8019CF6C();
    virtual void fn_8019CE10();
    virtual void fn_8019CCB4();
    virtual void fn_8019CCB0();
    virtual void fn_8019CB4C();

    /* 0x20 */ float field_0x20; // slot 0x0C's override stores its argument here (0x8019E724)
    // Unclassified storage: the constructor clears the words at 0x28-0x30 and constructs objects
    // at 0x38 (0x8019F85C) and 0x60 (0x8019F2D4), whose types are not recovered.
    /* 0x24 */ unsigned char field_0x24[0x48];
    /* 0x6C */ float field_0x6C;
    /* 0x70 */ float field_0x70;
};
CHECK_OFFSETOF(EnemyBaseAI, field_0x20, 0x20);
CHECK_OFFSETOF(EnemyBaseAI, field_0x70, 0x70);
CHECK_SIZEOF(EnemyBaseAI, 0x78);

#endif
