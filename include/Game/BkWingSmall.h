#ifndef GAME_BKWINGSMALL_H
#define GAME_BKWINGSMALL_H

#include "Game/FlyerCommonBase.h"
#include "Game/BkWingSmallAI.h"
#include "Game/EnemyTemplate.h"
#include "layout_check.h"

class Task;
class UnkGadgetOwner_800C9CD0;

/* BkWingSmallBase's placement parameters (placeholder type name): the shared flyer layout with no
 * additions; its own constructor only runs the shared one. */
struct UnkBkWingSmallSetData_80191BD0 : public UnkFlyerSetData_801907A8 {};
CHECK_SIZEOF(UnkBkWingSmallSetData_80191BD0, 0x38);

/* Original RTTI name. Method names are the slot placeholders of the classes that introduce them;
 * the declaration order of the overrides reached through secondary bases is the order of their
 * entries appended to the primary vtable. */
class BkWingSmallBase : public FlyerCommonBase {
  public:
    BkWingSmallBase();
    virtual ~BkWingSmallBase();
    // The DOL keeps one copy of this inline at 0x80181F30, outside this file and referenced only
    // by this file's vtables; no build reproduces that placement yet. Settle the inline form and
    // the copy's weak binding when the code around it is decompiled.
    virtual bool fn_80183ACC() {
        return m_setData.appearType == 1;
    }
    virtual bool fn_80183AC4() {
        return m_setData.actionType == 1;
    }
    virtual void fn_80183AD4();
    virtual void fn_80183A58();
    virtual bool fn_8019B40C(UnkEnemyDispEvent_8019B40C *event);
    virtual void fn_8019B35C(UnkGadgetOwner_800C9CD0 *owner);
    virtual void fn_8019B360(UnkGadgetOwner_800C9CD0 *owner);
    virtual int fn_80191A58();
    virtual void fn_801919C0();
    virtual void fn_8019B3A8(float dt);
    virtual void fn_801812C8();
    virtual void fn_8019B414();
    virtual int fn_8017E14C();
    virtual int fn_8017EE98();
    virtual int fn_8019B4C8();
    virtual int fn_8018019C();
    virtual int fn_8017FA78();
    virtual int fn_8019B4D0();
    virtual int fn_80180C58();

    /* Returns a constant 120.0f; its caller (0x80181CF4) is in the enemy AI code. */
    float fn_8019166C();

    /* 0x310 */ UnkBkWingSmallSetData_80191BD0 m_setData;
    /* 0x348 */ float field_0x348; // counts down by dt; reset to a random 8-12 when it expires
};
CHECK_SIZEOF(BkWingSmallBase, 0x350);

/* Original RTTI name: the BK_WINGSMALL gadget. */
class BkWingSmall : public EnemyTemplate<BkWingSmallBase, BkWingSmallAI> {
  public:
    BkWingSmall(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~BkWingSmall();
    /* Allocates the enemy under task layer 14 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);
    static const char *s_name;
};
CHECK_SIZEOF(BkWingSmall, 0x420);

#endif
