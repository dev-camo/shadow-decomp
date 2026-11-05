#ifndef GAME_BREAKWALL_H
#define GAME_BREAKWALL_H

#include "Game/CharCommand.h"
#include "Game/CharaColliAttack.h"
#include "Game/CrashInfoCtrl.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class UnkResource_80012700;

namespace BreakWall {

/* BreakWall::BreakWall (class name from RTTI; the gadget's descriptor is "BREAKWALL"): a wall that
 * shows the BREAKWALL.BIN model entry that its crash information names. An accepted attack that
 * leaves it health (of 3), or command id 11, while it is intact makes it cracked and switches to
 * the entry named after it with "_crack" appended when the model has one; an accepted attack that
 * uses up its health breaks it directly, as does a WallJumpCommand once it is cracked (attacks
 * without the required bits at 0x250 are rejected). On the next update
 * it starts the crash effect, passes the character info that broke it to fn_8020642C and switches
 * to the broken entry, or marks itself finished when there is none (behaviour inferred). Method
 * and member names are inferred. */
class BreakWall : public Gadget {
  public:
    BreakWall(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~BreakWall();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and model parameter into the gadget, returns to
     * state 0 and restores the full health (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info, turned about the y axis by the character info's
     * rotation (name inferred). */
    void UpdateModelMatrix();
    /* The character info's command handler: command id 11 cracks the intact wall, a
     * WallJumpCommand (id 10) breaks the cracked wall (name inferred). */
    void OnCommand(CharCommand &command);
    /* The model attack object's callback (name inferred). It reports 5 and leaves the health
     * alone when the wall is neither intact nor cracked, or when the attack object still exists
     * and has none of the bits 0x250 set in its word at 0x40. Otherwise it subtracts the attack's
     * value (the float at 0x50 of the parameter) from the health; when the attack object is gone
     * the bit test is skipped and the breaker stays empty. Once the health is used up the wall
     * breaks and the callback reports 3; otherwise an intact wall is cracked and the callback
     * reports 1. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* Passes fn_8015D93C a parameter block with the clump that the crash information's table
     * entry names, at the character info's position plus the entry's offset, then plays the
     * sound of the crash information's resource record at the character info's position plus
     * the record's offset (name inferred). */
    void StartCrashEffect();
    /* Switches the model to the entry that the crash information names for the broken wall and
     * enters state 4; returns false, without a change, when there is none (name inferred). */
    bool ShowBrokenModel();
    /* Switches the model to the entry named after the crash information's entry with "_crack"
     * appended and plays the sound of the resource record of that name (name inferred). It
     * returns false at once when there is no model; otherwise it releases m_target first and
     * returns false, leaving the model as it is, when there is no such entry. */
    bool ShowCrackedModel();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* The owner of the attack that used up the health, or the character info of the
     * WallJumpCommand that broke the wall. */
    /* 0x48 */ boost::weak_ptr<CharInfo> m_breaker;
    /* 0x50 */ UnkResource_80012700 *m_resource; /* the model file's resource */
    /* 0x54 */ boost::shared_ptr<GadgetBuilder::CrashInfoCtrl> m_crashInfo;
    /* 0x5C */ s32 m_modelIndex; /* the "model" parameter */
    /* 0 after construction or a reset, 1 intact, 2 cracked, 3 to break at the next update, 4
     * broken (inferred). */
    /* 0x60 */ s32 m_state;
    /* 0x64 */ s32 m_loadedModelIndex; /* the model parameter of m_crashInfo */
    /* 0x68 */ float m_health;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(BreakWall, 0x70);

} // namespace BreakWall

#endif
