#ifndef GAME_BKWORM_H
#define GAME_BKWORM_H

#include "Game/EnemyWalkerCommon.h"
#include "Game/BkWormAI.h"
#include "Game/EnemyTemplate.h"
#include "Game/UnkEnemySetData.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class Task;
class UnkGadgetOwner_800C9CD0;

namespace Effect {
namespace System {
class EffectElement;
} // namespace System
} // namespace Effect

namespace Weapon {
class LockonTarget;
} // namespace Weapon

/* BkWormBase's placement parameters (placeholder type name): the common enemy entries, then the
 * BK_WORM table's MoveSpeedRatio, WeaponType, AttackTimes and IntervalSec entries, whose defaults
 * the constructor stores. BkWormBase's constructor (0x801931A8) stores all ten defaults itself. */
struct UnkBkWormSetData_801931A8 : public UnkEnemySetDataCommon {
    /* Matching workaround: this record repeats UnkEnemySetData's seven defaults instead of
     * deriving from UnkEnemySetData (the same inlining limit as the workaround there). Derived
     * from it, MWCC no longer inlines this constructor into BkWormBase's: a weak
     * __ct__25UnkBkWormSetData_801931A8Fv (0x48 bytes) appears and BkWormBase's constructor
     * shrinks from 89 to 74 words. Revisit with UnkEnemySetData's workaround, when the original
     * constructor structure is known or the inline depth limit is understood. */
    UnkBkWormSetData_801931A8() {
        moveRange = 200.0f;
        searchRange = 0.0f;
        searchAngle = 0.0f;
        searchWidth = 100.0f;
        searchHeight = 100.0f;
        searchHeightOffset = 0.0f;
        moveSpeedRatio = 1.0f;
        weaponType = 0;
        attackTimes = 1;
        intervalSec = 3.0f;
    }

    /* 0x18 */ float moveSpeedRatio;
    /* 0x1C */ int weaponType; /* 0 to 2: NORMAL, WIDE, BIG (the table's labels) */
    /* 0x20 */ int attackTimes;
    /* 0x24 */ float intervalSec;
};
CHECK_SIZEOF(UnkBkWormSetData_801931A8, 0x28);

/* One entry of the weapon table that WeaponType indexes (placeholder type name). */
struct UnkBkWormWeapon_804C97C0 {
    /* The model frames (name inferred) the shots start from: ids for the display's fn_8019448C. */
    /* 0x0 */ const int *frames;
    /* 0x4 */ int count;
    /* Passed to fn_80051C34 with each shot and to fn_80221BA8 as the item type of the drop. */
    /* 0x8 */ int kind;
};
CHECK_SIZEOF(UnkBkWormWeapon_804C97C0, 0xC);

/* Original RTTI name. Method names are the slot placeholders of the classes that introduce them;
 * the declaration order of the overrides reached through secondary bases is the order of their
 * entries appended to the primary vtable. The non-virtual methods keep address placeholders. */
class BkWormBase : public EnemyWalkerCommon {
  public:
    BkWormBase();
    virtual ~BkWormBase();
    virtual void fn_8019B3A8(float dt);
    virtual int fn_80191A58();
    virtual void fn_801A7FA0();
    virtual void fn_801A8020(SonicteamUSA::System::CharaColli::AttackCallbackParam *param);
    virtual void fn_801A84DC();
    virtual void fn_801919C0();
    virtual void fn_801834D4();
    virtual bool fn_8019B40C(UnkEnemyDispEvent_8019B40C *event);
    virtual void fn_801A128C();
    virtual void fn_8019B418();
    virtual int fn_8017E14C() const;
    virtual int fn_8019B4D0();

    /* For each of the weapon's frames that the display returns, fires a shot from it with the
     * frame's slot (fn_80192608) and empties the slot; for odd slots it also passes sound 0x6028
     * to fn_8019AA40 (behaviour inferred). Frames the display does not return are skipped. */
    void fn_80192268();
    /* Stores a newly allocated lock-on target in each of the weapon's slots (a null pointer when
     * the allocation fails). When the character info that fn_800A8B74 returns for lbl_805EE588
     * still exists, is a Player::PlayerBase and has an object at 0x24C, it calls the slot's
     * fn_80061054 with that object's Target and then its fn_80060F70, through the stored pointer
     * without testing it (behaviour inferred). */
    void fn_80192394();
    /* When frame is not null, passes kind and a parameter block to fn_80051C34: the enemy's weak
     * pointer at 0x38 as owner, target, 1, the frame's translation, the frame applied to
     * (0, 0, -1) and lbl_805E07A0 (behaviour inferred). */
    void fn_80192608(Fn8040Matrix34 *frame, const boost::shared_ptr<Weapon::LockonTarget> &target,
                     int kind);
    /* Requests a stop of effect index of field_0x2A0 when its weak pointer still locks. */
    void fn_80192744(int index);
    /* Requests a stop of effect index of field_0x2A0 (fn_80192744), then stores in that slot the
     * weak pointer that fn_801A1F54 returns for effect 0x83 at position. Called from 0x80182670
     * and 0x8018311C (USA). */
    void fn_801927CC(const Fn8040Vector3 *position, int index);

    /* 0x270 */ UnkBkWormSetData_801931A8 m_setData;
    /* Effect 0x6D on frame 0xBBF: slot 0x10 starts it when the enemy's status slot 0x40 returns
     * false and the previous one has expired, and requests its stop when that slot returns true. */
    /* 0x298 */ boost::weak_ptr<Effect::System::EffectElement> field_0x298;
    /* 0x2A0 */ boost::weak_ptr<Effect::System::EffectElement> field_0x2A0[2];
    /* Starts at a random 8-12. In updates that do not pass sound 0x602A, slot 0x10 subtracts dt
     * when m_boundary.fn_801A68C0 accepts the vector of the control component's slot 0x10; when
     * the result is below 0 it is reset to a random 8-12 and sound 0x6029 goes to fn_8019A954. */
    /* 0x2B0 */ float field_0x2B0;
    /* 0x2B4 */ boost::shared_ptr<Weapon::LockonTarget> field_0x2B4[6];
};
CHECK_SIZEOF(BkWormBase, 0x2E8);

/* Original RTTI name: the BK_WORM gadget. */
class BkWorm : public EnemyTemplate<BkWormBase, BkWormAI> {
  public:
    BkWorm(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~BkWorm();
    virtual void fn_8019B35C(UnkGadgetOwner_800C9CD0 *owner);
    virtual void fn_8019B360(UnkGadgetOwner_800C9CD0 *owner);
    virtual bool fn_80187CBC();
    /* Allocates the enemy under task layer 14 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);
    static const char *s_name;
};
CHECK_SIZEOF(BkWorm, 0x3A0);

#endif
