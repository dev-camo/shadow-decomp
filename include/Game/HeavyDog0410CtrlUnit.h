#ifndef GAME_HEAVYDOG0410CTRLUNIT_H
#define GAME_HEAVYDOG0410CTRLUNIT_H

#include "Game/BossCharInfo.h"
#include "Game/BossCtrlUnit.h"
#include "Game/CharaColliBase.h"
#include "Game/EffectElement.h"
#include "Game/HeavyDogCtrlUnit.h"
#include "Game/HeavyDogVarParam.h"
#include "Game/Target.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

struct RpClump;
struct RwFrame;

/* The control units of the HEAVY_DOG_0410 boss (class names and bases from RTTI; see
 * HeavyDogCtrlUnit.h). The parameter types are inferred from the boss task, which passes its
 * HeavyDog0410VarParam. */

/* The constructor and the destructor are inline; the copies the linker keeps (constructor
 * 0x8024EB1C, destructor 0x8024EA8C) and the vtable are those of the HEAVY_DOG_0410 unit. The
 * constructor keeps param at 0xC as well. */
class HeavyDog0410CtrlUnit : public HeavyDogCtrlUnit {
  public:
    HeavyDog0410CtrlUnit(HeavyDog0410VarParam *param)
        : BossCtrlUnit(param), HeavyDogCtrlUnit(param), m_heavyDog0410Param(param) {}
    virtual ~HeavyDog0410CtrlUnit() {}

    /* 0x0C */ HeavyDog0410VarParam *m_heavyDog0410Param;
};
CHECK_SIZEOF(HeavyDog0410CtrlUnit, 0x20);

/* The three classes below are defined in HeavyDog0410CtrlUnit.cpp. */

/* Moves the boss (see Update in HeavyDog0410CtrlUnit.cpp). */
class HeavyDog0410CtrlUnit_Move : public HeavyDog0410CtrlUnit {
  public:
    HeavyDog0410CtrlUnit_Move(HeavyDog0410VarParam *param);
    virtual ~HeavyDog0410CtrlUnit_Move();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(HeavyDog0410CtrlUnit_Move, 0x20);

/* Holds effect 0x16 of the boss's effect launcher. The constructor creates it and, when it
 * exists, makes the position of the boss its translation source and the float at 0x24 of
 * lbl_8057D6A0 its time scale source. The destructor asks the effect to stop when it still exists,
 * and Update passes sound 0x7D3C to fn_801D53C4 at the position of the boss with this unit as its
 * owner and returns false. */
class HeavyDog0410CtrlUnit_Dust : public HeavyDog0410CtrlUnit {
  public:
    HeavyDog0410CtrlUnit_Dust(HeavyDog0410VarParam *param);
    virtual ~HeavyDog0410CtrlUnit_Dust();
    virtual bool Update(float arg);

    /* The result of EffectManager::Create (name inferred). */
    /* 0x10 */ boost::weak_ptr<Effect::System::EffectElement> m_effect;
};
CHECK_SIZEOF(HeavyDog0410CtrlUnit_Dust, 0x28);

/* A mine that the ATK_MINE mode unit throws (see the functions in HeavyDog0410CtrlUnit.cpp). The
 * constructor's parameter names are inferred from what it does with them: position, velocity and
 * destination are copied to m_position, m_velocity and m_destination. */
class HeavyDog0410CtrlUnit_Mine : public HeavyDog0410CtrlUnit, public AttackCallbackContainer {
  public:
    HeavyDog0410CtrlUnit_Mine(HeavyDog0410VarParam *param, const Fn8040Vector3 *position,
                              const Fn8040Vector3 *velocity, const Fn8040Vector3 *destination);
    virtual ~HeavyDog0410CtrlUnit_Mine();
    virtual bool Update(float arg);
    /* Overrides container slot 2 (0x8026C7C4; the container's vtable reaches it through the
     * adjustment thunk 0x8026D50C). When the constructor gets a probe and the probe's attack
     * object still exists, it gives the binding of this slot (fn_80335A80) to that attack object
     * as its second callback. */
    virtual void fn_8023357C(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    /* Clones of resources 8 and 9 of resource key 0x16, and their frames (names inferred). An
     * entry of m_clumps is only set when its resource exists; nothing else initialises it. */
    /* 0x14 */ RpClump *m_clumps[2];
    /* 0x1C */ RwFrame *m_frames[2];
    /* 0 to 4; see Update. */
    /* 0x24 */ s32 m_state;
    /* A time that Update adds its argument to and compares with the limits of the states (name
     * inferred). Update clears it when state 0 becomes state 2 and when state 1 or 2 becomes
     * state 3, but not when state 3 becomes state 4; fn_8023357C clears it on every call. */
    /* 0x28 */ float m_time;
    /* When the state was not 4 at its start, Update adds the acceleration (m_destination -
     * m_position) * HeavyDog0410VarParam::field_0x1F4 - m_velocity * field_0x1F8 times its
     * argument to m_velocity, and then m_velocity times its argument to m_position (names
     * inferred). */
    /* 0x2C */ Fn8040Vector3 m_position;
    /* 0x38 */ Fn8040Vector3 m_velocity;
    /* 0x44 */ Fn8040Vector3 m_destination;
    /* A character info of the mine's own, the owner of m_probe and m_target (names inferred
     * from the factories' parameters). */
    /* 0x50 */ boost::shared_ptr<BossCharInfo> m_charInfo;
    /* 0x58 */ boost::shared_ptr<Target> m_target;
    /* 0x60 */ boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> m_probe;

  private:
    /* Copying is disabled in this reconstructed interface because the copy contract is
     * unrecovered: the destructor destroys both clumps, which a memberwise copy would share.
     * Declared and not defined. */
    HeavyDog0410CtrlUnit_Mine(const HeavyDog0410CtrlUnit_Mine &);
    HeavyDog0410CtrlUnit_Mine &operator=(const HeavyDog0410CtrlUnit_Mine &);
};
CHECK_SIZEOF(HeavyDog0410CtrlUnit_Mine, 0x78);

#endif
