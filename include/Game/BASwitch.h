#ifndef GAME_BASWITCH_H
#define GAME_BASWITCH_H

#include "Game/CharCommand.h"
#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkObject_802B4254.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The id of a free switch record (name inferred). */
enum { SWITCH_RECORD_FREE = -1 };

/* A record of the table of four that the BASwitch gadget keeps of the switches it has set
 * (placeholder name; the constructor is at 0x80301F10). The second id and flag are a saved copy
 * of the first ones, which fn_80301CD0 takes and fn_80301E60 puts back (behaviour inferred). */
struct UnkSwitchRecord_80301F10 {
    /* Makes the record free and clears both flags. */
    UnkSwitchRecord_80301F10();

    /* 0x0 */ s32 id; /* the com id, or SWITCH_RECORD_FREE */
    /* 0x4 */ s32 savedId;
    /* 0x8 */ bool on;
    /* 0x9 */ bool savedOn;
};
CHECK_SIZEOF(UnkSwitchRecord_80301F10, 0xC);

namespace BASwitch {

/* BASwitch::BASwitch (class name from RTTI; the gadget's descriptor is "BA Switch"): a switch
 * that shows three models of BASWITCH.BIN: the base (entry 0), the entry "blue" and the entry
 * "red". An accepted attack (see OnAttack) or command id 0x14 while bit 0 of the flags is clear
 * (see OnCommand) turns the switch of its com id over in the manager of fn_800CB69C, plays a
 * motion of the base and shows the blue part while the switch is off and the red part while it
 * is on. It also marks the switch in the table of switch records when the id has a record or a
 * free record is left, and passes -1 (switched off) or 1 (switched on) to the actor that
 * fn_800740E4 returns for 0 when that actor still exists (behaviour inferred). Method and member
 * names marked "(name inferred)" are not original. */
class BASwitch : public Gadget {
  public:
    BASwitch(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~BASwitch();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and com id into the gadget, returns to state 0 and
     * clears the recovery time and the flags (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places each of the three models that exists at the character info's position, turned
     * about the y axis by the character info's rotation, and each of the two objects that exists
     * above that position (name inferred). */
    void UpdateModelMatrix();
    /* The model attack object's callback (name inferred). It reports 5, unless the recovery time
     * is over, the gadget is in state 1 and the attack object still exists with one of the bits
     * 0x3D1 set in its word at 0x40: then it starts the recovery time, reports 1 and turns the
     * switch over. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* Turns the switch of the com id over (name inferred). A switch whose state in the manager
     * is 1 is set to 0: the base model gets motion 1 (fn_80160A84), ShowState(1) runs, the actor
     * gets -1 and the switch is marked off in the table of switch records. Any other state is set
     * to 1: motion 2, ShowState(2), the actor gets 1, bit 1 of the flags is set and the switch is
     * marked on. The actor is called only when the weak pointer that fn_800740E4 returns can be
     * locked, and the table is written only when the id has a record or a free record is left
     * (fn_80301D04, fn_80301D90). Both play sound 0x7939. */
    void Toggle();
    /* Loads the three models, gives the base model's attack object the callback OnAttack when
     * its weak pointer can be locked, creates a target object for each model that exists and
     * calls fn_80167010(-1) on each target that exists (name inferred). */
    void SetupModels();
    /* With 1 (switched off) shows the blue part and hides the red one, with 2 (switched on) the
     * other way round, and passes the change to the targets of those parts (fn_80166B70 on the
     * shown part's target, fn_80166C70 on the hidden part's); each call is made only when its
     * model or target exists, and other values do nothing (name inferred; fn_8016271C shows a
     * model and fn_80162790 hides it, inferred). */
    void ShowState(int state);
    /* The character info's command handler: command id 0x14 turns the switch over (Toggle) and
     * starts the recovery time and sets bit 0 of the flags, unless bit 0 is already set; unlike
     * OnAttack it tests neither the recovery time nor the state. Every command gets bit 1 of its
     * flags set (name inferred). */
    void OnCommand(CharCommand &command);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_models[3];     /* the base, "blue" and "red" */
    /* 0x50 */ boost::shared_ptr<UnkTarget_80166AF4> m_targets[3]; /* one for each model */
    /* 0x68 */ boost::shared_ptr<UnkObject_802B4254> m_object2;    /* name inferred */
    /* 0x70 */ s32 m_state;          /* 0 until the first update after construction or a reset */
    /* 0x74 */ float m_recoveryTime; /* seconds left in which attacks are rejected */
    /* 0x78 */ s32 m_comId;
    /* 0x7C */ boost::shared_ptr<Target> m_object; /* name inferred */
    /* Bit 0: the recovery time is running; bit 1: sound 0x793A is pending, set when the switch is
     * turned on and cleared when the base model's fn_80160040(0) reports true and the sound is
     * played (inferred). */
    /* 0x84 */ UnkBitFlags_80014E04 m_flags;
};
CHECK_SIZEOF(BASwitch, 0x88);

} // namespace BASwitch

/* The table of switch records (0x80584CE0, built by the static initialiser at 0x80303704). The
 * original names of these functions are unknown; their behaviour is inferred from their bodies. */
extern "C" {
/* Listed in the order 0x80301CAC, 0x80301C88, 0x80301C64, 0x80301C40 at 0x804C5244, in a record
 * of the table at 0x804C4A18 whose other records also hold four such functions (the meaning of
 * each slot is unknown). Each calls one function below and returns 1 (return type inferred). */
int fn_80301C40(void);
int fn_80301C64(void);
int fn_80301C88(void);
int fn_80301CAC(void);
/* Copies each record's id and flag into its saved copy. */
void fn_80301CD0(void);
/* Marks the switch of id off in its record, or in the first free record when id has none;
 * does nothing when every record is taken by other ids. */
void fn_80301D04(s32 id);
/* As fn_80301D04, but marks the switch on. */
void fn_80301D90(s32 id);
/* Returns the flag of the record of id, or false when id has no record. */
bool fn_80301E1C(s32 id);
/* Puts each record's saved id and flag back. */
void fn_80301E60(void);
/* Each calls fn_80301ED4. */
void fn_80301E94(void);
void fn_80301EB4(void);
/* Frees every record and clears both of its flags, as the record constructor does. */
void fn_80301ED4(void);
/* Calls fn_80301CD0; the code at 0x80119158 calls it. */
void fn_80301F2C(void);
/* Calls fn_80301E1C; the BAShield gadget reads its switch through it. */
bool fn_80301F4C(s32 id);
}

#endif
