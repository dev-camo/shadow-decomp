#ifndef GAME_BKWINGSMALLAI_H
#define GAME_BKWINGSMALLAI_H

#include "Game/FlyerCommonAI.h"
#include "layout_check.h"

class BkWingSmallBase;

/* Original RTTI name. Replaces FlyerCommonAI slots 0x3C and 0x44 and the destructor. */
class BkWingSmallAI : public FlyerCommonAI {
  public:
    BkWingSmallAI();
    virtual ~BkWingSmallAI() {}
    virtual void fn_8019D428();
    virtual void fn_8019D278();

    // The constructor stores a dynamic_cast to BkWingSmallBase here.
    /* 0x8C */ BkWingSmallBase *field_0x8C;
    // Unclassified storage; no accesses are identified here.
    /* 0x90 */ unsigned char field_0x90[0xC];
};
CHECK_SIZEOF(BkWingSmallAI, 0xA0);

#endif
