#ifndef GAME_PLAYEREFFECT_H
#define GAME_PLAYEREFFECT_H

#include "Game/Task.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

namespace Effect {
namespace System {
class EffectElement;
}
} // namespace Effect

namespace Player {

class PlayerBase;
/* The object that does Player::Effect's work, defined in Game/PlayerEffect.cpp (name inferred
 * from the <class>Impl pattern of Boss::Camera::LamborghiniImpl, an RTTI name). */
class EffectImpl;

/* Player::Effect (class name from RTTI; base Task): a character's effect task. A virtual function
 * of Player::PlayerShadow (USA (GUPE8P) 0x800AD3F8) creates it. It registers itself at 0x23C of
 * its player and owns an EffectImpl, which does most of its work. */
class Effect : public Task {
  public:
    Effect(Task *parent, const boost::shared_ptr<PlayerBase> &player);
    virtual ~Effect();
    /* Calls fn_800AA5D4 on the player's motion task, when there is one. */
    virtual void Update(float dt);

    /* 0x28 */ boost::scoped_ptr<EffectImpl> m_impl;
};
CHECK_SIZEOF(Effect, 0x30);

} // namespace Player

/* The record at 0x244 of a Player::PlayerBase (placeholder name). Its destructor (USA (GUPE8P)
 * 0x800A25E0) is the only function of it in the game: no code creates the object, and only
 * PlayerBase's destructor destroys it. The member at 0x10 is the only one the destructor
 * destroys; the bytes before it are not mapped. */
class UnkObject_80079BA4 {
  public:
    ~UnkObject_80079BA4();

    /* 0x00 */ u8 field_0x0[0x10];
    /* 0x10 */ boost::scoped_ptr<class UnkObject_800A267C> field_0x10;
};
/* Checks only the recovered view: nothing creates the object, so its size is not known. */
CHECK_SIZEOF(UnkObject_80079BA4, 0x14);

/* The object that UnkObject_80079BA4's member at 0x10 owns (placeholder name; its layout is not
 * mapped). Its destructor is inline and does nothing: the copy of boost::checked_delete for it
 * (0x800A267C) tests the pointer before freeing it. */
class UnkObject_800A267C {
  public:
    ~UnkObject_800A267C() {}
};

/* An 8-byte object whose constructor (0x800A26E0) clears both words and whose destructor
 * (0x800A26A4) does nothing (placeholder name). The function at 0x801E5BD0 constructs it as a
 * function-local static; that function's only caller is fn_801E59C4 (the call at 0x801E5B18). */
class UnkObject_800A26E0 {
  public:
    UnkObject_800A26E0();
    ~UnkObject_800A26E0();

    /* 0x0 */ u32 field_0x0;
    /* 0x4 */ u32 field_0x4;
};
CHECK_SIZEOF(UnkObject_800A26E0, 0x8);

/* One entry of the effect table of Game/PlayerEffect.cpp (placeholder name). field_0x0 and
 * field_0x8 are effect ids, field_0x4 the footmark effect id and field_0xC a sound id; -1 means
 * none. */
struct UnkEffectEntry_805217E0 {
    /* 0x0 */ s32 field_0x0;
    /* 0x4 */ s32 field_0x4;
    /* 0x8 */ s32 field_0x8;
    /* 0xC */ s32 field_0xC;
};
CHECK_SIZEOF(UnkEffectEntry_805217E0, 0x10);

/* The functions of Player::Effect that other files call (placeholder names; behaviour inferred
 * from the bodies). fn_800A26F0 and fn_800A2730 only change an entry of the effect table;
 * fn_800A2754 to fn_800A2854 create an effect for the player; fn_800A28B0, fn_800A28EC and
 * fn_800A2920 act on the player's barrier task, and fn_800A2A78, fn_800A2AD8, fn_800A2B44 and
 * fn_800A2B78 on its Electric and Super tasks, through inline functions of the EffectImpl; the
 * others call a function of the EffectImpl. */
extern "C" {
/* Restore entry index of the effect table to its default (fn_800A26F0) or set it (fn_800A2730). */
void fn_800A26F0(int index);
void fn_800A2730(int index, s32 value0, s32 value4, s32 value8, s32 valueC);
/* Create effect id through fn_801D2BB0, fn_801D2B38, fn_801D2AFC and fn_801D2C28, passing the
 * address of the second float that the pointer at 0x60 of the player's character info points to
 * (a time scale, inferred: CameraDemo multiplies its time step by the same float). */
boost::weak_ptr<Effect::System::EffectElement> fn_800A2754(Player::Effect *effect, int id);
boost::weak_ptr<Effect::System::EffectElement> fn_800A2794(Player::Effect *effect,
                                                           const Fn8040Vector3 *position,
                                                           const UnkRotation3 *rotation, int id);
boost::weak_ptr<Effect::System::EffectElement> fn_800A27D4(Player::Effect *effect,
                                                           const Fn8040Vector3 *position,
                                                           const Fn8040Vector3 *vector, int id);
boost::weak_ptr<Effect::System::EffectElement> fn_800A2814(Player::Effect *effect,
                                                           const Fn8040Matrix34 *matrix, int id);
/* Creates effect id at the matrix of motion record index (fn_800A2814). */
boost::weak_ptr<Effect::System::EffectElement> fn_800A2854(Player::Effect *effect, int index,
                                                           int id);
void fn_800A28B0(Player::Effect *effect);
void fn_800A28EC(Player::Effect *effect);
void fn_800A2920(Player::Effect *effect, int arg1);
/* Placeholder names and signatures inferred from the bodies and motion-event caller. */
void fn_800A2978(Player::Effect *effect, void *payload);
void fn_800A299C(Player::Effect *effect, int index, const Fn8040Vector3 *position,
                 const Fn8040Vector3 *vector);
void fn_800A29C0(Player::Effect *effect, int arg0);
void fn_800A29E4(Player::Effect *effect);
void fn_800A2A08(Player::Effect *effect);
void fn_800A2A30(Player::Effect *effect);
void fn_800A2A54(Player::Effect *effect);
void fn_800A2A78(Player::Effect *effect);
void fn_800A2AD8(Player::Effect *effect);
void fn_800A2B44(Player::Effect *effect);
void fn_800A2B78(Player::Effect *effect);
void fn_800A2BD8(Player::Effect *effect);
void fn_800A2C00(Player::Effect *effect);
void fn_800A2C28(Player::Effect *effect, bool start);
void fn_800A2C4C(Player::Effect *effect);
void fn_800A2C70(Player::Effect *effect);
void fn_800A2C98(Player::Effect *effect);
void fn_800A2CBC(Player::Effect *effect);
void fn_800A2CE4(Player::Effect *effect);
void fn_800A2D08(Player::Effect *effect);
void fn_800A2D30(Player::Effect *effect);
void fn_800A2D54(Player::Effect *effect, float arg1);
Task *fn_800A2D7C(Player::Effect *effect, int type);
}

#endif
