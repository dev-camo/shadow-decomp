#ifndef GAME_SWITCH_H
#define GAME_SWITCH_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace Switch {

/* The values of the "type" parameter, named after the descriptor's labels "type : OnOff",
 * "type : OnTouch" and "type : OnAlways". */
enum { TYPE_ON_OFF = 0, TYPE_ON_TOUCH = 1, TYPE_ON_ALWAYS = 2 };

/* Switch::Switch (class name from RTTI; the gadget's descriptor is "SWITCH"): a switch made of
 * three models of resource 0xE1 that keeps its state in the entry of its com id in the manager of
 * fn_800CB69C (1 on, any other value off; inferred). OnTouch and OnRelease are the callbacks at
 * 0x10 and 0x1C of the first model. OnAttack is installed on that model's attack object
 * (fn_80161428(0)) only when the constructor's lock of it succeeds. When the model calls the two collision callbacks is not established here: their names follow
 * what their bodies do with the "type" parameter. Behaviour inferred; method and member names
 * marked "(name inferred)" are not original. */
class Switch : public Gadget {
  public:
    Switch(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Switch();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the character info and its com id into
     * m_comId, copies its parameter block into m_type only when the owner's parameter size is 4
     * (fn_800C9DBC compares it at 0x800C9DD4), and clears all the flags (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Builds a matrix from the character info's position and its y, x and z rotations (applied
     * in that order) and passes it to each of the three models that exists; when the target object
     * exists, gives it the character info's position with 5 added to y (name inferred). */
    void UpdateModelMatrix();
    /* The callback at 0x10 of m_model (name inferred). It does nothing while bit 0 of the flags
     * is set. Otherwise an "OnOff" switch turns over (a state of 1 becomes 0, any other state
     * becomes 1) and tries to play sound 0xE016, an "OnTouch" or "OnAlways" switch whose state
     * is not 1 is set to 1 and tries to play that sound, and any other type changes nothing;
     * then, for every type, ShowState runs and bit 0 of the flags is set. */
    void OnTouch(ColliDetectArgs &args);
    /* The callback at 0x1C of m_model (name inferred): an "OnTouch" switch is set to state 0,
     * without a sound and whatever bit 0 of the flags holds; then ShowState runs for every
     * type. */
    void OnRelease(ColliDetectArgs &args);
    /* The attack object's callback (name inferred). The result is 5, and nothing else happens,
     * when bit 0 of the flags is set, or when the attack object in param.field_0x0 has not
     * expired and has none of the bits 0x240 set in its word at 0x40. Otherwise the result is 1: an "OnOff"
     * switch turns over as in OnTouch, an "OnAlways" switch is set to 1 and other types keep their
     * state (no sound in any case); then ShowState runs and bit 0 of the flags is set. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* When the state of the com id is 1, calls fn_8016271C on m_onModel and fn_80162790 on
     * m_offModel; otherwise the other way round (name inferred). Neither model pointer is
     * checked. fn_8016271C passes each part that has an object at 0x8 to fn_80038F94 and clears
     * bit 2 of the model's flag word at 0x3C; fn_80162790 does the same with fn_80038FC0 and
     * sets that bit. Read as showing and hiding the model (inferred). */
    void ShowState();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* The three models, given the ids 0, 1 and 2 through fn_80162A68; the character info is
     * attached to the first one's node "col" (names inferred). */
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<GadgetModelInfo> m_onModel;
    /* 0x48 */ boost::shared_ptr<GadgetModelInfo> m_offModel;
    /* 0x50 */ boost::shared_ptr<Target> m_target; /* name inferred */
    /* 0x58 */ s32 m_type;                         /* the "type" parameter (TYPE_ values) */
    /* 0x5C */ u8 m_comId;                         /* the owner's com id (fn_800C9E60) */
    /* Bit 0 is set by OnTouch whenever it finds the bit clear and by OnAttack when it gets past
     * its checks; UpdateBody clears it at its end (0x80123694), so on every update where
     * Gadget::Update calls UpdateBody (it skips the call once IsFinished returns true), and
     * ReadOwner (constructor and ResetFromOwner) clears all the flags. */
    /* 0x60 */ UnkBitFlags_80014E04 m_flags;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(Switch, 0x68);

} // namespace Switch

#endif
