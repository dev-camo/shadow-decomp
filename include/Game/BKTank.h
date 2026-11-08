#ifndef GAME_BKTANK_H
#define GAME_BKTANK_H

#include "Game/PathObject.h"
#include "Game/Target.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class GadgetCharInfo;
class GadgetModelInfo;
class UnkHandleBTarget_8015BE64;
class UnkTarget_80166AF4;
namespace Effect {
namespace System {
class EffectElement;
} // namespace System
} // namespace Effect
namespace SonicteamUSA {
namespace System {
namespace CharaColli {
class AttackCallbackParam;
} // namespace CharaColli
} // namespace System
} // namespace SonicteamUSA

namespace BKTank {

/* BKTank::SetBKTank (class name from RTTI, with PathObject as its only base): the object of
 * BKTANK.BIN that this file creates together with its BKTankController::Controller (0x80321E44);
 * the BK_TANK descriptor has no create function. Hits lower its health (350 at construction).
 * At 50 or less it drops its sphere probe and installs OnAttack on its model's collision object
 * (SetStage(0)); at 0 or less it breaks up (SetStage(1)) and starts a 1.5 second countdown, and
 * IsFinished becomes true one update after that has run out (see Update and OnAttack). It keeps
 * up to ten effects, six of which Update moves to node 10 of its model's motion, and plays sounds
 * that depend on whether its character info moved. Behaviour inferred; method, member and
 * constant names are inferred unless PathObject gives them, and so are the parameters' reference
 * and const spellings. */
class SetBKTank : public PathObject {
  public:
    /* The values of m_state (names inferred from what each state does). Nothing stores 1. */
    enum State {
        STATE_START = 0,
        STATE_HEALTH_HIGH = 2, /* until m_health is 149.98999 or less */
        STATE_HEALTH_MID = 3,  /* until m_health is 50 or less; then SetStage(0) */
        STATE_HEALTH_LOW = 4,  /* until m_health is 0 or less; then SetStage(1) */
        STATE_COUNTDOWN = 5,   /* until m_timer is 0 or less */
        STATE_FINISHING = 6,   /* sets FLAG_FINISHED */
        STATE_FINISHED = 7
    };

    /* The bit of m_flags (name inferred): set on leaving STATE_FINISHING; IsFinished returns it.
     * The constructor clears the flags (the flags constructor) and Initialize clears them again
     * (Clear); nothing else in this file clears them. */
    enum Flag { FLAG_FINISHED = 0 };

    /* Stores 350 in m_health and constructs the members, which clears m_flags; the other scalar
     * members are first set by Initialize or Update (field_0x2C and m_speed by Update). */
    SetBKTank();
    /* Destroys the members only. */
    virtual ~SetBKTank();

    /* Computes m_speed, advances the model, field_0x14 and the timers, moves the effects 0, 9, 7,
     * 1, 2 and 6 to the GetNodeMatrix matrix (fn_80425E68) and runs the state (see State; leaving
     * STATE_HEALTH_LOW also sets m_timer to 1.5). While field_0x28 is above 0 it subtracts dt
     * from it, and only when the result is below 0 (USA (GUPE8P) 0x8031FACC) does it set
     * field_0x28 to 0, request the effects 1 and 2 to stop (RequestStop) and clear field_0xA8: a
     * result of exactly 0 requests no stop and leaves field_0xA8 as it was. Then, whenever
     * field_0x28 is 0 (in that same update too, and after a result of exactly 0), it sets bit 0 of
     * m_target's flags and clears field_0x2C. A speed of 1800 or more resets m_speed to 0 and
     * stops the effects 0 and 7; once it is lower again, effect 0 (fn_80168A50 id 0x11) is created
     * if it has expired and effect 8 (0x70) is created. On stopping, effect 9 (0x6F) is created if
     * it has expired and effect 0 stops; on starting to move, effect 0 (0x11) is created if it has
     * expired and effect 9 stops. Effect 7 stops when m_speed falls below 180. Sounds at the
     * character info's position: 0x714F (fn_801D53C4) while stationary, 0x714D (fn_801D53C4) while
     * moving and 0x7160 (fn_801D54F4) on starting to move. Stopping an effect here clears its
     * ELEMENT_ACTIVE, and only effects that still exist are changed. */
    virtual void Update(float dt);
    /* Builds the character info (fn_8015CB88(0x91, 4, "SetBKTank")), adds it to the table of
     * fn_800DDF20 (fn_801DCD8C, result unused), sets its word at 0x30 to 1, builds the model and
     * passes 0 to the model's fn_80162A68 (with 0), fn_80160C28, fn_8016082C and fn_80160B18; it
     * requests the attachment of the character info to the model's part 0 (fn_80161FC8) without
     * testing the result. When the model's collision object of part 0 (fn_80161614) still exists,
     * it calls its UnkSlot20 with 0x100000 and -1; when the model's attack object of part 0
     * (fn_80161428) still exists, it calls its fn_80054DD0 with 1, 1, callbacks bound to
     * fn_8031F908 and fn_8031F904, and 1. Then it creates a sphere probe of radius 56 around the
     * character info (fn_800C7DEC with 0x100000, 5, 0 and the zero vector lbl_805E07A0), keeps its
     * handle target in m_handle with bit 1 of its flags cleared and, when the probe's attack object
     * (fn_800C4374) still exists, calls its fn_80054DD0 with 0, 1, the same two callbacks and 1,
     * and installs OnAttack (fn_80096B64(1)). Last, it places the model, keeps the character info's
     * position in m_previousPosition, creates m_target, sets the three timers to 0, field_0xA8,
     * m_moving and field_0xAB to false and m_stationary to true, resets the state and clears the
     * flags. */
    virtual void Initialize();
    /* Removes the character info from the table of fn_800DDF20 (fn_801DCCF4) and clears
     * ELEMENT_ACTIVE of every effect that still exists. */
    virtual void UnkSlot3();
    /* When there is a model, places it at the matrix that fn_8015CEA0 builds from the character
     * info; when m_target is not null, moves it to the character info's position. */
    virtual void UpdateModelMatrix();
    /* Whether FLAG_FINISHED is set. */
    virtual bool IsFinished();
    /* A weak pointer to the character info. */
    virtual boost::weak_ptr<CharInfo> GetCharInfo();
    virtual void UnkSlot7();
    virtual void UnkSlot8();
    virtual void OnCommand(CharCommand &command);

    /* The callback that fn_80096B64 installs (name inferred). It first sets the result (param's
     * word at 0x54) to 5. When m_hitTimer is above 0, or the attack object no longer exists, or
     * none of the bits 0x3FA of its word at 0x40 is set, nothing else happens. When one of the
     * bits 0xA is set, it increments field_0x2C, and when that is then 3 or more it clears bit 0
     * of m_target's flags (the pointer is not tested) and returns with the result still 5.
     * Otherwise it subtracts param's float at 0x50 from m_health, sets m_hitTimer to 0.05 and
     * field_0x28 to 2, and creates an effect at param's field_0x18 and field_0x24 (fn_80168C48):
     * effect 3 (0x71) when m_health is now above 149.98999 (it also calls fn_800A31F0 and
     * discards the result), effect 4 (0x64) when it is above 50 and effect 6 (0x7F) otherwise,
     * playing the sound 0x7150 for the first two and 0x7152 for the last. When field_0xA8 is
     * clear, it creates effect 1 (0x12) when m_health is above 149.98999, or effect 2 (0x63) when
     * it is above 50, and sets field_0xA8 in either case. When field_0xA8 is set and m_health has
     * dropped from above 149.98999 to that or less in this hit, it requests effect 1 to stop
     * (RequestStop) and creates effect 2 (0x63); otherwise, when it has dropped from above 50 to
     * 50 or less, it requests effect 2 to stop, creates effect 5 (0x13) and plays the sound
     * 0x7151. The result becomes 1 while m_health stays above 0 and 3 otherwise. The effects 1,
     * 2 and 5 are created at the GetNodeMatrix matrix, the sounds play at the character info's
     * position, and a stop is requested only when the effect still exists. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The first callback passed to fn_80054DD0: stores (0, 1, 0) in param's field_0x34 and 1 in
     * field_0x4C and field_0x50. */
    void fn_8031F908(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The second callback passed to fn_80054DD0; it does nothing. */
    void fn_8031F904(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* When there is a model and its entry 1 (fn_8015F7BC) exists, passes fn_8015D93C a parameter
     * block for the character info with 2 in field_0x34, the clump that fn_80012700 returns for
     * the entry's name (key 1; not tested), 1 in field_0x54 and 150 in field_0x4C. In every case
     * it then plays the sound 0x7153 at the character info's position. */
    void StartCrashEffect();
    /* Stage 0 resets m_handle and, when the model's collision object of part 0 (fn_80161614)
     * still exists and fn_800C42D0 gives it an attack object that still exists, installs OnAttack
     * there (fn_80096B64(1)). Stage 1 calls StartCrashEffect and the model's fn_801604FC and
     * fn_80162804 (the model pointer is not tested) and clears ELEMENT_ACTIVE of the effects 7, 9
     * and 0 when they still exist. Both pass 8, the character info's position and -2 to
     * fn_8034DE08 and discard the result; other values do nothing. */
    void SetStage(int stage);
    /* The matrix of node 10 of the model's motion 0 (fn_80415E70, converted with fn_80016370), or
     * the identity (lbl_805E0720) when that motion has expired. The model pointer is not
     * tested. */
    Fn8040Matrix34 GetNodeMatrix();

    /* 0x04 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x0C */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* Nothing in this file sets it; Update passes it dt (fn_8016761C) when it is not null. */
    /* 0x14 */ boost::shared_ptr<UnkTarget_80166AF4> field_0x14;
    /* 0x1C */ float m_health; /* 350 at construction; OnAttack subtracts param's field_0x50 */
    /* 0.05 after an accepted hit; OnAttack ignores hits while it is above 0. */
    /* 0x20 */ float m_hitTimer;
    /* 0x24 */ float m_timer; /* 1.5 on entering STATE_COUNTDOWN */
    /* 2 after an accepted hit; see Update for what happens when it runs out and while it is 0. */
    /* 0x28 */ float field_0x28;
    /* OnAttack counts the hits that pass its first tests and have a bit of 0xA in the attack's
     * flags; Update clears it while field_0x28 is 0. */
    /* 0x2C */ s32 field_0x2C;
    /* 0x30 */ s32 m_state;                  /* a State */
    /* 0x34 */ UnkBitFlags_80014E04 m_flags; /* Flag bits */
    /* fn_801E7128(the character info, 0x22C00), created by Initialize */
    /* 0x38 */ boost::shared_ptr<Target> m_target;
    /* The character info's position at the end of Initialize and of each Update. */
    /* 0x40 */ Fn8040Vector3 m_previousPosition;
    /* The distance from the character info's position to m_previousPosition divided by dt,
     * computed by Update when dt is not 0. */
    /* 0x4C */ float m_speed;
    /* The handle target of the sphere probe that Initialize creates; SetStage(0) resets it. */
    /* 0x50 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
    /* Created by fn_80168C48 (3, 4 and 6) and fn_80168A50 (0, 1, 2, 5, 8 and 9); nothing in this
     * file creates effect 7. */
    /* 0x58 */ boost::weak_ptr<Effect::System::EffectElement> m_effects[10];
    /* Set when OnAttack creates effect 1 or 2; cleared by the Update in which subtracting dt
     * leaves field_0x28 below 0 (not when it leaves exactly 0). */
    /* 0xA8 */ bool field_0xA8;
    /* Whether the character info's position equalled m_previousPosition in the last Update (true
     * after Initialize). */
    /* 0xA9 */ bool m_stationary;
    /* Whether m_speed was 180 or more in the last Update, after the 1800 test (which clears
     * m_speed). */
    /* 0xAA */ bool m_moving;
    /* Set when m_speed is 1800 or more; cleared by the next Update in which it is below 1800. */
    /* 0xAB */ bool field_0xAB;
};
CHECK_SIZEOF(SetBKTank, 0xAC);
CHECK_OFFSETOF(SetBKTank, m_effects, 0x58);
CHECK_OFFSETOF(SetBKTank, field_0xA8, 0xA8);

} // namespace BKTank

extern "C" {
/* Creates a SetBKTank and a BKTankController::Controller for it under task layer 9, which becomes
 * the file's controller (null when its allocation fails). */
void fn_8031EBEC(void);
/* Destroys the controller when there is one. */
void fn_8031EBA4(void);
/* Does nothing. */
void fn_8031EBA0(void);
/* Destroys the controller when there is one and creates a new SetBKTank with its controller
 * (fn_8031EBEC). */
void fn_8031EB54(void);
/* Listed in the order 0x8031E9CC, 0x8031E9A8, 0x8031E984, 0x8031E960 at 0x804C4CA4, two words
 * after a pointer to the string "Highway", in a record of the table at 0x804C4A18 whose other
 * records also hold four such functions (the meaning of each slot is unknown). Each calls one
 * function above and returns 1 (return type inferred). */
int fn_8031E960(void);
int fn_8031E984(void);
int fn_8031E9A8(void);
int fn_8031E9CC(void);
}

#endif
