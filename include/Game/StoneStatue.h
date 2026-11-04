#ifndef GAME_STONESTATUE_H
#define GAME_STONESTATUE_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/SetAdapterState.h"
#include "Game/Target.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

/* The state that the owner remembers between the creations of a stone statue (class name from
 * RTTI, derived from SetAdapterState): the health that attacks use up (name inferred). The
 * constructor leaves it unset; StoneStatue::ReadOwner resets it. */
class StoneStatueSetState : public SetAdapterState {
  public:
    StoneStatueSetState() {}
    virtual ~StoneStatueSetState() {}

    /* Restores the full health (name inferred). */
    void Reset() {
        m_health = 2.0f;
    }

    /* 0x04 */ float m_health;
};
CHECK_SIZEOF(StoneStatueSetState, 0x8);

/* The gadget parameters in the order of the descriptor's table ("ACTIVE 0:off 1:On",
 * "MIRROR 0:off 1:On", "RANGE_RADIUS m", "RANGE_HEIGHT m"). ReadOwner converts the two ranges from
 * metres to the game's units (times 10). */
struct StoneStatueParams {
    /* 0x0 */ u32 active; /* turns to a character in range and calls fn_8011E938 when broken */
    /* 0x4 */ u32 mirror; /* the model index, and the index of the break file */
    /* 0x8 */ float rangeRadius;
    /* 0xC */ float rangeHeight;
};
CHECK_SIZEOF(StoneStatueParams, 0x10);

/* StoneStatue (class name from RTTI): the gadget of a stone statue (model file STONESTATUE.BIN)
 * that breaks when attacks have used up its health and, when it is active, turns to the nearest
 * character once one has come into its range (behaviour inferred). Method names marked "(name
 * inferred)" are not original. */
class StoneStatue : public Gadget {
  public:
    StoneStatue(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~StoneStatue();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, the rotation and the parameters from the owner, clears m_tracking and
     * restores the remembered health to 2 (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Puts the model and the object at 0x80 where the character info is (name inferred). */
    void UpdateModelMatrix();
    /* Whether the character nearest to the statue is inside the range cylinder: within
     * rangeRadius horizontally, and from s_rangeDepth below the statue's position (m_charInfo) up
     * to rangeHeight above that (name inferred). */
    bool IsCharacterInRange();
    /* Turns the statue about its y axis towards the nearest character by at most 40 degrees a
     * second, and not at all when it is less than 5 degrees off (name inferred). */
    void TurnToCharacter(float dt);
    /* The attack object's callback (behaviour inferred): an attack takes its strength off the
     * remembered health, and a statue whose health is used up breaks. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* Plays the break sound and effect, calls fn_8011E938 when the statue is active and finishes
     * the gadget (name inferred). */
    void Break();
    /* Queues a debug cylinder of the range (name inferred). */
    void DrawRange();

    static const char *s_name;

    /* Set once a character has come into the range; the statue then keeps turning to the nearest
     * character (name inferred). */
    /* 0x30 */ bool m_tracking;
    /* 0x34 */ StoneStatueParams m_params;
    /* 0x44 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x4C */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x54 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* The sound that plays while the statue turns. */
    /* 0x5C */ UnkSoundHandle_801D3DC8 m_sound;
    /* 0x78 */ boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliAttack> m_attack;
    /* 0x80 */ boost::shared_ptr<Target> m_object;
};
CHECK_SIZEOF(StoneStatue, 0x88);

#endif
