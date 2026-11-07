#ifndef GAME_BKWORMAI_H
#define GAME_BKWORMAI_H

#include "Game/EnemyBaseAI.h"
#include "layout_check.h"

class BkWormBase;

/* Original RTTI name. Replaces EnemyBaseAI slots 0x38, 0x3C and 0x58 and the destructor. */
class BkWormAI : public EnemyBaseAI {
  public:
    BkWormAI();
    virtual ~BkWormAI() {}
    virtual void fn_8019D4D8();
    virtual void fn_8019D428();
    virtual void fn_8019CCB4();

    // The constructor (0x8018358C) stores a dynamic_cast to BkWormBase here.
    /* 0x74 */ BkWormBase *field_0x74;
    // Unclassified storage; no accesses are identified here.
    /* 0x78 */ unsigned char field_0x78[0x4];
    /* 0x7C */ unsigned char field_0x7C; // the constructor stores 1
};
CHECK_SIZEOF(BkWormAI, 0x84);

#endif
