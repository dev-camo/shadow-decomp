#ifndef GAME_STICK_H
#define GAME_STICK_H

#include "Game/CharaColliAttack.h"
#include "Game/CrashInfoCtrl.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class UnkResource_80012700;

namespace Stick {

/* The gadget parameters in the order of the descriptor's table (names from the table's strings).
 * The constructor clears both; fn_800C9DBC then copies them from the owner only when the owner's
 * parameter size equals sizeof(StickParams), 8. */
struct StickParams {
    StickParams() : model(0), weapon(0) {}

    /* 0x0 */ s32 model;  /* selects the crash information of STICK.BIN */
    /* 0x4 */ s32 weapon; /* the weapon type passed to fn_8011E938 */
};
CHECK_SIZEOF(StickParams, 0x8);

/* Stick::Stick (class name from RTTI; the gadget's descriptor is "STICK"): an object of STICK.BIN
 * that shows the model entry its crash information names; without crash information for its
 * "model" parameter the constructor calls fn_8015A794 and sets Task flag 0x01 instead. An attack
 * that OnAttack accepts in STATE_INTACT breaks it: at the next update it starts the crash effect
 * and switches to the broken entry when the crash information names one; otherwise it releases
 * the model, calls fn_8015A794 and enters STATE_NO_MODEL. Either way m_timer becomes 0.5.
 * m_timer counts down only in STATE_DROP_WAIT and in STATE_NO_MODEL. STATE_DROP_WAIT follows
 * directly when the broken entry has no motion; with a motion, STATE_BREAK_MOTION enters it
 * (after fn_80160400 on the broken entry) once the motion is found and has ended (the two
 * stMtnControlWorkFlag tests, inferred), and stays as it is while fn_801601A0 finds no motion.
 * In both counting states, each update first subtracts dt and then, when m_timer <= 0 (never
 * while it is NaN), calls fn_8011E938 with the type of the "weapon" parameter and the character
 * info's position raised by 20 units, which is the helper's input position: the helper probes
 * downward from it and can replace it with a hit, and it skips the item setup when its item
 * lookup returns null (behaviour inferred). Method and member names are inferred, as are the
 * parameters' pointer, reference and const spellings. */
class Stick : public Gadget {
  public:
    Stick(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Stick();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the character info and the owner's
     * parameters into m_params (only when the owner's parameter size is 8, fn_800C9DBC), then
     * returns to STATE_RESET and clears m_timer (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model, when there is one, at the character info's position, turned by its y, x
     * and z rotations (name inferred). */
    void UpdateModelMatrix();
    /* The model attack object's callback (name inferred). Outside STATE_INTACT it reports 5. In
     * STATE_INTACT an attack whose attack object still exists and has bit 1 of its word at 0x40
     * set is reported 5 and changes nothing; otherwise the attack object's owner, when the attack
     * object and that owner still exist, is passed to fn_8020642C with 0x23, the callback reports
     * 3 and the state becomes STATE_HIT. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* When there is crash information: passes fn_8015D93C a parameter block with the clump that
     * the crash information's table entry names (when there is an entry and its clump loads), at
     * the character info's position plus the entry's offset, then plays the sound of the crash
     * information's resource record (when there is one) at the character info's position plus
     * the record's offset (name inferred). */
    void StartCrashEffect();
    /* Returns false, changing nothing, when there is no crash information or its fn_802087A0
     * name (the broken entry, inferred) is null. Otherwise calls the model's fn_80161880 and
     * fn_80162804, switches the model to that entry and places it. When fn_80160BA0 finds a
     * motion of that name, it calls the model's fn_8015FF60, passes the fn_80208784 name, the
     * character info and the entry's name to fn_80161AB8, passes the entry's name to
     * fn_801609F8, places the model again and enters STATE_BREAK_MOTION; otherwise it attaches
     * the character info to the node of the fn_80208784 name when that name is not null, and
     * enters STATE_DROP_WAIT. Then, when the resource has a record of the entry's name, it starts
     * that record's sound at the character info's position plus the record's offset and keeps it
     * in m_sound; without a record m_sound is left as it is. Returns true. The model pointer is
     * not checked here (name inferred). */
    bool ShowBrokenModel();

    /* The values of m_state (names inferred). */
    enum State {
        STATE_RESET = 0,        /* after construction or a reset */
        STATE_INTACT = 1,       /* waiting for an attack */
        STATE_HIT = 2,          /* breaks at the next update */
        STATE_BREAK_MOTION = 3, /* the broken entry's motion plays */
        STATE_DROP_WAIT = 4,    /* counts m_timer down, then calls fn_8011E938 */
        STATE_DROPPED = 5,      /* fn_8011E938 has been called */
        /* No broken entry (the model is released): each update subtracts dt from m_timer and,
         * while m_timer <= 0, sets Task flag 0x01 and calls fn_8011E938. The state does not
         * change, so the flag and the call repeat on the updates where m_timer stays <= 0 (a
         * later negative dt can make it positive, and NaN never passes the test). */
        STATE_NO_MODEL = 6
    };

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* m_params.model as of the constructor, or of the last ResetFromOwner reload whose crash
     * information exists. */
    /* 0x40 */ s32 m_loadedModel;
    /* 0x44 */ StickParams m_params;
    /* 0x4C */ s32 m_state; /* a State value */
    /* 0x50 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x58 */ UnkResource_80012700 *m_resource; /* the model file's resource */
    /* 0x5C */ boost::shared_ptr<GadgetBuilder::CrashInfoCtrl> m_crashInfo;
    /* 0x64 */ float m_timer; /* time until fn_8011E938 is called (in seconds, inferred) */
    /* 0x68 */ UnkSoundHandle_801D3DC8 m_sound;
    /* Bit 0 is set when the constructor finds a motion for the intact entry (fn_80160BA0) and
     * cleared otherwise; STATE_INTACT plays that motion's sound events while it is set. A reset
     * does not touch it. */
    /* 0x84 */ UnkBitFlags_80014E04 m_motionFlags;
};
CHECK_SIZEOF(Stick, 0x88);

} // namespace Stick

#endif
