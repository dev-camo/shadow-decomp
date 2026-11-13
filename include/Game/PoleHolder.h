#ifndef GAME_POLEHOLDER_H
#define GAME_POLEHOLDER_H

#include "Game/CharaColliAttack.h"
#include "Game/CharaColliBase.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_804157E8_motion_control.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <list>
#include <vector>
#include "layout_check.h"

class FallingStone;
class PoleHolder;
class StonePole;

/* The gadget parameters in the order of the descriptor's table ("0:FLAT 1:SLOPE 2:AIR",
 * "FALL L/R 0:L 1:R", "RANGE_R m", "RANGE_HEIGHT m", "ROLLING_SPD m/sec", "ROLLING_DISTANCE m",
 * "STEP_2 0:SOFT 1:HARD" to "STEP_6 0:SOFT 1:HARD"). PoleHolder::ReadParams converts the
 * distances from metres to the game's units (times 10) and halves the range height. */
struct PoleHolderParams {
    /* 0x00 */ u32 type; /* 0 flat, 1 slope or 2 air: the class of the pole */
    /* 0x04 */ u32 fall; /* 0 left or 1 right */
    /* 0x08 */ float rangeRadius;
    /* 0x0C */ float rangeHeight;
    /* Converted by PoleHolder::ReadParams and not used otherwise. */
    /* 0x10 */ float rollingSpeed;
    /* How far a stone rolls before its removal is queued. */
    /* 0x14 */ float rollingDistance;
    /* The kind of each of the five stones: 0 soft or 1 hard. */
    /* 0x18 */ u32 steps[5];
};
CHECK_SIZEOF(PoleHolderParams, 0x2C);

/* FallingStone (class name from RTTI, with no bases): one of the five stones of a stone pole
 * (behaviour inferred). It follows a node of the pole's motion until the pole has fallen; then,
 * unless the pole is flat, it rolls on by the pole's roll velocity and is removed after the
 * rolling distance. FallingStoneSoft and FallingStoneHard (class names from RTTI) implement its
 * pure slots. Method names marked "(name inferred)" are not original. */
class FallingStone {
  public:
    FallingStone(UnkGadgetOwner_800C9CD0 *owner, StonePole *pole, u32 index);
    virtual ~FallingStone();
    /* Called for each stone when the pole starts to fall (name inferred). */
    virtual void OnFallStart() {}
    /* Called for each stone when the pole's fall motion has ended (name inferred). */
    virtual void OnFallEnd() {}
    /* Called for each stone by StonePoleFlat::OnFallEnd (name inferred). */
    virtual void OnStop() {}
    /* Plays the break effect (name inferred). */
    virtual void Break() = 0;
    /* Called at the end of UpdateModelMatrix (name inferred). */
    virtual void UpdateAttachments() = 0;

    /* Creates a hard stone when kind is 1 and a soft one otherwise (name inferred). */
    static boost::shared_ptr<FallingStone> Create(UnkGadgetOwner_800C9CD0 *owner, StonePole *pole,
                                                  u32 index, int kind);

    /* Places the model at the character info's position, rotated about z, x and y in that order
     * (name inferred). */
    void UpdateModelMatrix();
    /* Before the pole has fallen (poleState other than 3) copies the position and the rotation of
     * the stone's node in the pole's motion; after it, unless the pole is flat, rolls the stone on
     * by the pole's roll velocity times dt and queues its removal once it has rolled the rolling
     * distance (name inferred). */
    void UpdatePosition(s32 poleState, float dt);
    /* Places the stone again after the owner's settings changed (name inferred). */
    void ResetFromOwner();
    void Update(s32 poleState, float dt);

    /* 0x04 */ u32 m_index; /* 0 to 4, the index of the stone's STEP parameter */
    /* 0x08 */ StonePole *m_pole;
    /* 0x0C */ UnkGadgetOwner_800C9CD0 *m_owner;
    /* 0x10 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x18 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x20 */ boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliAttack> m_attack;
    /* 0x28 */ float m_rolledDistance;
};
CHECK_SIZEOF(FallingStone, 0x2C);

/* FallingStoneSoft (class name from RTTI, derived from FallingStone): a stone (model entry
 * "StoneSoft") that the pole breaks once its fall motion passes frame 70; an attack on it whose
 * strength has a magnitude of at least 0.0001 starts the fall of a standing pole (behaviour
 * inferred). */
class FallingStoneSoft : public FallingStone {
  public:
    FallingStoneSoft(UnkGadgetOwner_800C9CD0 *owner, StonePole *pole, u32 index);
    virtual ~FallingStoneSoft();
    virtual void Break();
    virtual void UpdateAttachments();

    /* The attack object's callback (behaviour inferred): passes the attack to the pole as
     * StonePole::OnAttack does. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
};
CHECK_SIZEOF(FallingStoneSoft, 0x2C);

/* FallingStoneHard (class name from RTTI, derived from FallingStone): a stone (model entry
 * "StoneHard") with an object and an attack object, turned on when the pole starts to fall; attacks
 * take their strength off its health, and it breaks when the health is used up (behaviour
 * inferred). */
class FallingStoneHard : public FallingStone {
  public:
    FallingStoneHard(UnkGadgetOwner_800C9CD0 *owner, StonePole *pole, u32 index);
    virtual ~FallingStoneHard();
    /* Turns the attack object and bit 0 of the object's flags on (name inferred). */
    virtual void OnFallStart();
    /* On a slope pole, starts the rolling sound (name inferred). */
    virtual void OnFallEnd();
    /* Turns the attack object off (name inferred). */
    virtual void OnStop();
    virtual void Break();
    /* Moves the object and the rolling sound to the character info's position (name inferred). */
    virtual void UpdateAttachments();

    /* The attack object's callbacks (behaviour inferred). OnAttack takes the attack's strength
     * off the health, as StoneStatue::OnAttack does; OnHit breaks the stone when bit 0 of the
     * result is set; SetPushDirection stores the stone's rolling direction, the length of the
     * roll velocity (scaled by 1.0 first: the distance rolled in one second) and 1 in the
     * parameter. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    void OnHit(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    void SetPushDirection(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    /* 0x2C */ boost::shared_ptr<Target> m_object;
    /* 0x34 */ float m_health;
    /* 0x38 */ UnkSoundHandle_801D3DC8 m_sound;
};
CHECK_SIZEOF(FallingStoneHard, 0x54);

/* StonePole (class name from RTTI, with no bases): the pole of the STONE_POLE gadget (model file
 * STONEPOLE.BIN, motion of the holder's type) with its five stones (behaviour inferred). It falls
 * when the character nearest to it in three dimensions is within its range (only that character
 * is tested, so a farther character inside the range does not trigger it), or when an attack whose
 * strength has a magnitude of at least 0.0001 hits it; StonePoleFlat, StonePoleSlope and
 * StonePoleAir (class names from RTTI) give the stones' motion nodes and their roll velocity
 * (their tables s_stoneNodes and s_rollVelocity have inferred names). The slots named UnkSlot are
 * empty in every class here. Method names marked "(name inferred)" are not original. */
class StonePole {
  public:
    StonePole(PoleHolder *holder, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~StonePole();
    /* The motion node that stone index follows (name inferred). */
    virtual u32 GetStoneNode(u32 index) = 0;
    /* How far the stones roll per second after the fall (name inferred). */
    virtual const Fn8040Vector3 *GetRollVelocity() = 0;
    /* Called at the end of each Update (name inferred). */
    virtual void UpdateSound(float dt) {}
    virtual void UnkSlot4() {}
    virtual void UnkSlot5() {}
    virtual void UnkSlot6() {}
    virtual void UnkSlot7() {}
    /* Called when the fall motion has ended (name inferred). */
    virtual void OnFallEnd() {}

    /* Builds the collision object and gives it the attack callback, unless it exists already
     * (name inferred). */
    void CreateCollision();
    /* Queues a debug cylinder of the range (name inferred). */
    void DrawRange();
    /* Whether the character nearest to the pole (chosen in three dimensions, before the range
     * test) is within rangeRadius horizontally and within rangeHeight (half the RANGE_HEIGHT
     * parameter) above or below it; other characters are not tested (name inferred). */
    int IsCharacterInRange();
    /* The attack object's callback (behaviour inferred): an attack starts the fall of a standing
     * pole unless fn_8000B044 finds its strength zero (magnitudes below 0.0001 count as zero). */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* Starts the fall: the stones' OnFallStart, the sound, the motion and the target (name
     * inferred). */
    void Fall();
    /* Removes the stone with the given index from the list (name inferred). */
    void RemoveStone(u32 index);
    /* Removes the stones whose removal is queued (name inferred). */
    void RemoveQueuedStones();
    /* Builds the five stones again (name inferred). */
    void CreateStones();
    /* Places the model at the character info's position, rotated about y, and the collision
     * object at half of s_collisionHeight (PoleHolder.cpp) above it (name inferred). */
    void UpdateModelMatrix();
    /* Puts the motion and the collision object back to the start (name inferred). */
    void Reset();
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    void Update(float dt);
    /* The stone with the given index, or null (name inferred). */
    FallingStone *FindStone(u32 index);
    void QueueStoneRemoval(u32 index);
    bool IsStoneRemovalQueued(u32 index);

    /* 0x04 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x0C */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x14 */ boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> m_collision;
    /* 0x1C */ boost::weak_ptr<_stMtnPlayWork> m_motion;
    /* Set when UpdateSound has played its sound (name inferred). */
    /* 0x24 */ bool m_soundPlayed;
    /* 0x28 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x30 */ std::list<boost::shared_ptr<FallingStone> > m_stones;
    /* 0x3C */ PoleHolder *m_holder;
    /* 0x40 */ UnkGadgetOwner_800C9CD0 *m_owner;
    /* The indices of the stones that RemoveQueuedStones removes next (name inferred). */
    /* 0x44 */ std::vector<u32> m_removalQueue;
    /* 0 standing, 1 falling, 3 fallen (values from Fall and Update). */
    /* 0x50 */ s32 m_state;
    /* Set once the soft stones have been broken during the fall (name inferred). */
    /* 0x54 */ bool m_softStonesBroken;
    /* Set while bit 0 of the flags of the motion of the holder's type is set (fn_80160480 sets
     * that bit and fn_80160384 clears it). */
    /* 0x55 */ bool m_motionFlagSet;
};
CHECK_SIZEOF(StonePole, 0x58);

/* StonePoleFlat (class name from RTTI, derived from StonePole): a pole whose stones stay where the
 * fall leaves them (FallingStone::UpdatePosition does not roll the stones of type 0, and its roll
 * velocity is zero; behaviour inferred). */
class StonePoleFlat : public StonePole {
  public:
    StonePoleFlat(PoleHolder *holder, UnkGadgetOwner_800C9CD0 *owner) : StonePole(holder, owner) {}
    virtual ~StonePoleFlat();
    virtual u32 GetStoneNode(u32 index) {
        return s_stoneNodes[index];
    }
    virtual const Fn8040Vector3 *GetRollVelocity() {
        return &s_rollVelocity;
    }
    /* Plays a sound once the fall motion passes frame 65 while stones are left (behaviour
     * inferred). */
    virtual void UpdateSound(float dt);
    virtual void UnkSlot4();
    virtual void UnkSlot5();
    virtual void UnkSlot6();
    virtual void UnkSlot7();
    virtual void OnFallEnd();

    static const u32 s_stoneNodes[5];
    static Fn8040Vector3 s_rollVelocity;
};
CHECK_SIZEOF(StonePoleFlat, 0x58);

/* StonePoleSlope (class name from RTTI, derived from StonePole): a pole on a slope whose stones
 * roll down after the fall (behaviour inferred). */
class StonePoleSlope : public StonePole {
  public:
    StonePoleSlope(PoleHolder *holder, UnkGadgetOwner_800C9CD0 *owner) : StonePole(holder, owner) {
        UnkSlot5();
    }
    virtual ~StonePoleSlope();
    virtual u32 GetStoneNode(u32 index) {
        return s_stoneNodes[index];
    }
    virtual const Fn8040Vector3 *GetRollVelocity() {
        return &s_rollVelocity;
    }
    /* Plays a sound once the fall motion passes frame 80 while stones are left (behaviour
     * inferred). */
    virtual void UpdateSound(float dt);
    virtual void UnkSlot5();
    virtual void OnFallEnd();

    static const u32 s_stoneNodes[5];
    static Fn8040Vector3 s_rollVelocity;
};
CHECK_SIZEOF(StonePoleSlope, 0x58);

/* StonePoleAir (class name from RTTI, derived from StonePole): a pole in the air whose stones drop
 * after the fall (behaviour inferred). */
class StonePoleAir : public StonePole {
  public:
    StonePoleAir(PoleHolder *holder, UnkGadgetOwner_800C9CD0 *owner) : StonePole(holder, owner) {
        UnkSlot5();
    }
    virtual ~StonePoleAir();
    virtual u32 GetStoneNode(u32 index) {
        return s_stoneNodes[index];
    }
    virtual const Fn8040Vector3 *GetRollVelocity() {
        return &s_rollVelocity;
    }
    virtual void UnkSlot5();
    virtual void OnFallEnd();

    static const u32 s_stoneNodes[5];
    static Fn8040Vector3 s_rollVelocity;
};
CHECK_SIZEOF(StonePoleAir, 0x58);

/* PoleHolder (class name from RTTI): the gadget of the "STONE_POLE" descriptor, which holds the
 * stone pole of the class its type parameter selects (behaviour inferred). Method names marked
 * "(name inferred)" are not original. */
class PoleHolder : public Gadget {
  public:
    PoleHolder(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~PoleHolder();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 and its pole (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Builds the pole of the class the type parameter selects (name inferred). */
    void CreatePole(UnkGadgetOwner_800C9CD0 *owner);
    /* Reads the parameters from the owner (name inferred). */
    void ReadParams(UnkGadgetOwner_800C9CD0 *owner);
    /* The parameters (name inferred; so are the pointer return and the definition in the class).
     * Matching workaround: an inferred inline accessor of the same form as ChaoHolder's (see
     * ChaoHolder.cpp), used wherever the pole and its stones read the parameters. Through it,
     * StonePole::Update and StonePole::CreateStones load a stone kind as the original does: the
     * step offset added to the holder's address, then a load at 0x50 from it; with the member
     * read directly they add 0x50 to the offset and use an indexed load. Revisit when the
     * original accessor is found. */
    const PoleHolderParams *GetParams() {
        return &m_params;
    }

    /* The class name the constructor stores in m_name (name inferred). */
    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<StonePole> m_pole;
    /* 0x38 */ PoleHolderParams m_params;
};
CHECK_SIZEOF(PoleHolder, 0x68);

#endif
