#ifndef GAME_FLYERCOMMONAI_H
#define GAME_FLYERCOMMONAI_H

#include "Game/EnemyBaseAI.h"
#include "layout_check.h"

class FlyerCommonBase;

/* Original RTTI name. Replaces EnemyBaseAI slot 0x58 and the destructor. */
class FlyerCommonAI : public EnemyBaseAI {
  public:
    FlyerCommonAI();
    virtual ~FlyerCommonAI() {}
    virtual void fn_8019CCB4();

    // The constructor (0x80183FC4) stores a dynamic_cast to FlyerCommonBase, 1 and 0.0f here.
    /* 0x74 */ FlyerCommonBase *field_0x74;
    /* 0x78 */ unsigned char field_0x78;
    /* 0x7C */ float field_0x7C;
    // Unclassified storage; no accesses are identified here.
    /* 0x80 */ unsigned char field_0x80[0xC];
};
CHECK_SIZEOF(FlyerCommonAI, 0x90);

#endif
