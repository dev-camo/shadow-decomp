#ifndef GAME_DEFENSEWALL_H
#define GAME_DEFENSEWALL_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace DefenseWall {

/* The gadget parameters in the order of the descriptor's table (names from the table's strings).
 * The constructor stores the table's default values. */
struct SetDefenseWallParams {
    SetDefenseWallParams() : width(40.0f), height(40.0f), depth(40.0f), speed(4.0f) {}

    /* 0x0 */ float width; /* the probe's sizes, passed in this order to fn_800C7D04 and SetSize */
    /* 0x4 */ float height;
    /* 0x8 */ float depth;
    /* 0xC */ float speed; /* m_offset shrinks by speed times dt at each update while lowering */
};
CHECK_SIZEOF(SetDefenseWallParams, 0x10);

/* DefenseWall::SetDefenseWall (class name from RTTI; the gadget's descriptor is "DEFENSE WALL"): a
 * CONCRETEDOOR.BIN model on a character info that starts s_startOffset above the gadget's position.
 * When its box probe reports a collision, the wall moves down to the position by "speed" times dt
 * at each update. From the first update on, attacks subtract their value from a health of 4, and
 * after one above 0.9 the wall ignores attacks until m_recoveryTime has counted 0.2 down by dt;
 * when the health runs out, the wall shows its crash effect and finishes (behaviour inferred).
 * Member names and the names of the non-virtual methods are inferred from the code. */
class SetDefenseWall : public Gadget {
  public:
    SetDefenseWall(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetDefenseWall();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, rotation and parameters from the owner, resets the state, health,
     * recovery time and offset, places the character info s_startOffset above the position and
     * copies the position and rotation for the probe (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's matrix (name inferred). */
    void UpdateModelMatrix();
    /* The attack object's callback (name inferred). */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* Creates the box probe from the probe position, rotation and sizes, and binds OnCollision
     * to its handle (name inferred). */
    void SetupProbe();
    /* Passes fn_8015D93C a parameter block for the model file's crash information: the file that
     * its table entry names, at the entry's offset from the character info (name inferred). */
    void StartCrashEffect();
    /* The probe's collision callback: starts the lowering once the gadget waits (name inferred). */
    void OnCollision(ColliDetectArgs &args);

    /* True in the states in which attacks count: waiting, lowering and lowered (name
     * inferred). */
    bool IsDamageable() const {
        return m_state == 1 || m_state == 2 || m_state == 3;
    }

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
    /* 0x48 */ SetDefenseWallParams m_params;
    /* 0 after reading the owner, 1 waiting for the probe, 2 lowering, 3 lowered, 4 broken (the
     * next update shows the crash effect), 5 finished (inferred). */
    /* 0x58 */ s32 m_state;
    /* 0x5C */ float m_health;
    /* 0x60 */ float m_recoveryTime; /* attacks are ignored while it is above 0 */
    /* 0x64 */ Fn8040Vector3 m_position;
    /* 0x70 */ float m_offset; /* the height of the character info above m_position */
    /* 0x74 */ Fn8040Vector3 m_probePosition;
    /* 0x80 */ Fn8040Vector3 m_probeRotation;
};
CHECK_SIZEOF(SetDefenseWall, 0x90);

} // namespace DefenseWall

#endif
