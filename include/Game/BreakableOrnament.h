#ifndef GAME_BREAKABLEORNAMENT_H
#define GAME_BREAKABLEORNAMENT_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class GadgetCharInfo;
class GadgetModelInfo;
class UnkResource_80012700;
class UnkTarget_80166AF4;
namespace GadgetBuilder {
class CrashInfoCtrl;
}

namespace ArtWork {

/* ArtWork::BreakableOrnament (class name from RTTI; the "ART GADGET" and "ART GADGET ZONE" gadgets,
 * with the model files ARTISTGADGET.BIN and ARTGADGETZONE.BIN): an ornament that asks its model to
 * show the entry that the crash information of its "model" parameter names (fn_801629D8, which
 * returns false when the model has no entry of that name; the result is ignored) and starts with
 * the health that the crash information gives. When the model file has no crash information for
 * that parameter, the constructor passes the gadget to fn_8015A7EC and fn_8015A794, sets Task flag
 * 0x01 and returns before it selects a model entry or installs the callback. The constructor
 * installs the attack callback only when attaching the character info (fn_80161AB8 or fn_80161F08)
 * returns true and the attack object still exists. While it is
 * intact (state 1), an attack whose attack object has expired or has bit 1 of its word at 0x40 set
 * is rejected; any other attack subtracts its value (the float at 0x50 of the parameter) from the
 * health. When the health is then <= 0 (a NaN health is not), the attack switches to state 2 and
 * passes the attack object's owner (its character info) to fn_8020642C with 0x23 when that owner
 * still exists. The next UpdateBody then attempts the crash effect (StartCrashEffect), drops the
 * part target, stops the sound and attempts to switch to the broken model entry (ShowBrokenModel),
 * or finishes (fn_8015A794, Task flag 0x01) when there is no crash information or it names no
 * broken entry (behaviour inferred).
 * Method and member names and the parameters' pointer and reference spellings are inferred. */
class BreakableOrnament : public Gadget {
  public:
    BreakableOrnament(Task *parent, UnkGadgetOwner_800C9CD0 *owner, const char *fileName);
    virtual ~BreakableOrnament();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Finished (Task flag 0x01 set, true returned) when fn_8015A858 reports it. Otherwise, while
     * m_stateFlags bit 3 is clear, it asks fn_8015A818; when that reports, it sets bit 3, calls
     * the model's fn_8015FC94 with -1 when the model pointer is non-null and returns false. Once
     * bit 3 is set, it finishes when the model pointer is non-null and the model's fn_8015FBF8
     * returns true (name inferred). */
    virtual bool IsFinished();

    /* Copies the owner's position and its rotation (converted to radians), and the model
     * parameter only when the owner's parameter size is 4 (fn_800C9DBC; otherwise m_modelIndex
     * keeps its value); returns to state 0, sets m_stateFlags bits 1 and 2 from the owner (see
     * m_stateFlags) and clears bit 3 (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When the model pointer is non-null, places the model at the character info, turned by the
     * character info's rotation about the y, x and z axes in that order (name inferred). */
    void UpdateModelMatrix();
    /* The model attack object's callback (name inferred; behaviour in the class comment). It
     * reports 3 when the health is <= 0 after the subtraction; otherwise 1 when the attack's value
     * is > 0.9 (the model's fn_8016268C then gets 0.25 when the model pointer is non-null), and 5
     * when it is not (also for a NaN value). A NaN health takes the value test. Outside state 1,
     * and for a rejected attack, it reports 5 and leaves the health unchanged. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* Returns at once when fn_802039E8 is true and fn_802039C8 false for the record at 0xA8 of
     * lbl_8057D6A0, and does nothing without crash information. Otherwise, when the crash
     * information has a table entry whose file gives a clump, passes fn_8015D93C a parameter block
     * with that clump at the character info's position plus the entry's offset (with m_stateFlags
     * bit 1 set, the block also gets the character info's height at 0x40 and 1 at 0x4A); and when
     * it has a resource record, passes the record's sound and the character info's position plus
     * the record's offset to fn_801D54F4 (name inferred). */
    void StartCrashEffect();
    /* Returns false, without a change, when there is no crash information or it names no entry for
     * the broken ornament. Otherwise asks the model to switch to that entry (fn_801629D8); when the
     * model has a motion of that name, attempts to attach the character info through fn_80161AB8
     * at the node the crash information names, passes the entry name to fn_801609F8 and enters
     * state 3; otherwise enters state 4, attempting the attachment through fn_80161F08 only when
     * the crash information names a node. The results of fn_801629D8, fn_80161AB8, fn_801609F8
     * and fn_80161F08 are ignored: a failed selection or attachment still enters state 3 or 4.
     * Then, when the model pointer is non-null, attempts to create a part target (fn_80166AF4,
     * which gives a null target when its allocation fails) and passes the target's weak pointer to
     * fn_8015FA78 with the part callback fn_80169D00, which uses the target only when locking the
     * weak pointer succeeds (the target's fn_80167010 gets -1 only when the target is non-null);
     * when the entry has a resource record, tries to start its sound (fn_801D3E58); and returns
     * true (name inferred). */
    bool ShowBrokenModel();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x48 */ UnkResource_80012700 *m_resource; /* the model file's resource */
    /* 0x4C */ boost::shared_ptr<GadgetBuilder::CrashInfoCtrl> m_crashInfo;
    /* 0x54 */ s32 m_modelIndex; /* the "model" parameter */
    /* 0 after ReadOwner, 1 intact, 2 to break at the next update, 3 playing the broken entry's
     * motion, 4 broken (inferred). */
    /* 0x58 */ s32 m_state;
    /* The model parameter m_crashInfo was loaded for (inferred from the same member of BreakWall,
     * SecurityLaser and PopFooting, which compare it with the model parameter on reset); this class
     * only writes it. */
    /* 0x5C */ s32 m_loadedModelIndex;
    /* 0x60 */ float m_health;
    /* 0x64 */ UnkSoundHandle_801D3DC8 m_sound;
    /* Bit 0 is set or cleared only by the constructor, from whether the model has a motion of the
     * entry's name (fn_80160BA0); ResetFromOwner's reload and ShowBrokenModel leave it unchanged,
     * and state 1 reads it. Every ReadOwner (constructor and reset) sets or clears bit 1 from bit
     * 0x400000 of the owner's flag word (fn_800C9E70) and bit 2 from whether the owner's com id is
     * above 0. While bit 2 is set, UpdateBody reads the entry of that id in the manager of fn_800CB69C: when it reads 1
     * and the model's fn_801626F4 is false, it calls the model's fn_80162790 and, when there is a
     * part target, its fn_80166C70; when it does not and fn_801626F4 is true, fn_8016271C and
     * (with a part target) fn_80166B70. The model pointer is not checked there. Bit 3 is set by
     * IsFinished when fn_8015A818 reports and cleared by ReadOwner. */
    /* 0x80 */ UnkBitFlags_80014E04 m_stateFlags;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(BreakableOrnament, 0x88);

} // namespace ArtWork

#endif
