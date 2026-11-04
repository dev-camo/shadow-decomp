#ifndef GAME_FLYERCOMMONBASE_H
#define GAME_FLYERCOMMONBASE_H

#include "Game/EnemyFlyerCommon.h"
#include "Game/UnkEnemySetData.h"
#include "Game/Unk801A6844.h"
#include "layout_check.h"

/* Placement parameters of the flying enemies (placeholder type name): the common enemy block
 * followed by the flyer entries every flying enemy's parameter table lists (BK_WINGSMALL's table
 * names every word except 0x24). The constructor stores the defaults the flyer tables list. */
struct UnkFlyerSetData_801907A8 : public UnkEnemySetData {
    UnkFlyerSetData_801907A8()
        : appearType(0), actionType(0), field_0x24(0), floatWidth(0.0f), attackStart(0.4f),
          attackEnd(0.6f), pathMirror(0) {}

    /* 0x1C */ int appearType;
    /* 0x20 */ int actionType;
    /* 0x24 */ int field_0x24;
    /* 0x28 */ float floatWidth;
    /* 0x2C */ float attackStart;
    /* 0x30 */ float attackEnd;
    /* 0x34 */ int pathMirror;
};
CHECK_SIZEOF(UnkFlyerSetData_801907A8, 0x38);

/* Original RTTI name. Replaces EnemyStatusCommon slot 0x38 (fn_801A7FA0) and appends five virtual
 * functions; the int/bool/float returns follow the base bodies. */
class FlyerCommonBase : public EnemyFlyerCommon {
  public:
    FlyerCommonBase(int kind);
    virtual ~FlyerCommonBase();
    virtual void fn_801A7FA0();
    virtual bool fn_80183ACC();
    virtual bool fn_80183AC4();
    virtual float fn_80183EB4();
    virtual void fn_80183AD4();
    virtual void fn_80183A58();

    /* Applies placement parameters (behaviour, class and parameter type inferred from
     * BkWingSmallBase's call). */
    void fn_80193790(const UnkFlyerSetData_801907A8 *setData);

    // Constructed and destroyed by the constructor (0x80190888) and destructor (0x801905B4).
    /* 0x268 */ Unk801A6844 field_0x268;
    // Unclassified storage: the end of Unk801A6844 is not established, and the constructor
    // initialises further members from 0x2D8 whose types are not recovered.
    /* 0x2D4 */ unsigned char field_0x2D4[0x3C];
};
CHECK_OFFSETOF(FlyerCommonBase, field_0x268, 0x268);
CHECK_OFFSETOF(FlyerCommonBase, field_0x2D4, 0x2D4);
CHECK_SIZEOF(FlyerCommonBase, 0x314);

#endif
