#ifndef GAME_BASEDOOR_H
#define GAME_BASEDOOR_H

#include "Game/Door.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

struct _stMtnPlayWork;

namespace Door {

/* Door::SetBaseDoor (class name from RTTI; the gadget's descriptor is "BASE DOOR"): a door made of
 * two models from BASEDOOR.BIN, the entry its "model" parameter selects (m_model) and the entry
 * 1000 above it (m_motionModel), which plays the opening and closing motions. Only one of them is
 * shown at a time (ShowModels). Its parameters have the names, order and layout of the DOOR
 * gadget's, so it uses SetDoorParams. A key door (type 1) waits until the entry of its com id in
 * the manager of fn_800CB69C reports 1 and then opens for good; a normal door starts opening in
 * an update in which fn_8015AC84 accepts one of its box probe's hits (see OnCollision; bit 0 of
 * m_flags), and once it is open, it starts closing in the first update without an accepted hit,
 * even while hits that fn_8015AC84 rejects remain. Member names, method names and the method
 * signatures (argument and return types, const qualifiers) are inferred from the code, except
 * OnCollision's type, which the RTTI of its boost::bind callback gives. */
class SetBaseDoor : public Gadget {
  public:
    /* Reads the owner's parameters (fn_800C9DBC copies them only when the owner's block is 0x20
     * bytes), creates the character info, loads the models and shows m_model, creates the box
     * probe and its handle target, whose collision callback is OnCollision, and then reads the
     * owner and places the models. */
    SetBaseDoor(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetBaseDoor();
    /* Clears bit 0 of m_flags and passes dt to the handle target (fn_8015BE9C, which runs
     * OnCollision for the probe's hits and so may set the bit again), the two models and m_target,
     * whichever are not null. While bit 1 of m_flags is set it plays the sound 0x7849
     * at the character info. Then, by m_state (each step only when its condition holds):
     * - 0: enters 1 for a key door and 2 otherwise; when m_model is not null, creates m_target
     *   (fn_80166AF4, which returns an empty pointer when its allocation fails) and walks the
     *   model's parts with fn_8015FA78, passing fn_80169D00 a weak pointer to m_target (empty
     *   when m_target is): for each part fn_80169D00 locks that weak pointer and, only when the
     *   target exists, passes it and the part's clump (the part's first word) to fn_80166FDC.
     *   Then passes -1 to m_target's fn_80167010 when it exists; when m_motionModel is not null,
     *   does the same with m_motionTarget (without the fn_80167010 call).
     * - 1: when fn_8016904C reports 1 for m_comId, enters 5 and shows m_motionModel, requests its
     *   motion 1, passes -1 to m_motionTarget's fn_80167448 and calls m_target's fn_80166C70 (each
     *   only when that object exists) and plays the sound 0x783C. State 5 has no step.
     * - 2: when bit 0 of m_flags is set, shows m_motionModel, requests its motion 1, makes the same
     *   two target calls as state 1, enters 3 and sets bit 1 of m_flags.
     * - 3: when m_motionModel's motion has ended (fn_80160040), enters 4, clears bit 1 of m_flags
     *   and plays the sound 0x7847.
     * - 4: when bit 0 of m_flags is clear, requests m_motionModel's motion 2 (when the model
     *   exists), enters 6 and sets bit 1 of m_flags.
     * - 6: when m_motionModel's motion has ended, enters 2, shows m_model, sets the start frame of
     *   m_model's motion to -1 when that motion is alive and locks, calls m_target's fn_80166B70
     *   (when it exists), clears bit 1 of m_flags and plays the sound 0x7847.
     * Last it places the models (UpdateModelMatrix). State 3 calls m_motionModel, and state 6
     * calls m_motionModel and m_model, without checking them. */
    virtual void UpdateBody(float dt);
    /* Reads the owner and places the models. When the "model" parameter then differs from
     * m_modelIndex, takes it as m_modelIndex, releases m_target and the models, loads the new
     * models, shows m_model and places the models again. Then, when m_model is not null, calls its
     * fn_80162EF4 and fn_8016108C, and when there is a handle target, moves its probe to the box
     * given by the parameters (as the constructor does) and calls fn_8015BE64. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* With 0 shows m_model and hides m_motionModel, with 1 the other way round (fn_8016271C shows
     * a model and fn_80162790 hides it, inferred); each model is also passed its entry and -1, to
     * fn_8016111C when it is shown and to fn_80161230 when it is hidden. Each model is used only
     * when it is not null; other values do nothing. */
    void ShowModels(int open);
    /* Releases both models and m_motion. */
    void ReleaseModels();
    /* Loads both models from BASEDOOR.BIN (entries index and index + 1000), tries to attach the
     * character info to each of them and keeps in m_motion the weak pointer to m_motionModel's
     * motion control that fn_801602B4 returns. The attachments can fail without effect:
     * fn_80161FC8 returns 0 when the character info has expired or no entry has the id, and
     * fn_80161BD8 returns 0 when it finds no motion for the id; m_motion is empty when
     * fn_801602B4 finds no motion for the entry. When m_model's fn_80160C28 is true for its
     * entry, calls m_model's fn_8015FF60 and requests its motion 1; m_model's fn_8015FF60 is
     * called again after m_motionModel is loaded. The models are used without null checks. */
    void LoadModels(int index);
    /* The collision callback of the box probe: sets bit 0 of m_flags when fn_8015AC84 accepts one
     * of the hits in args, that is, a hit whose probe target and character info are still alive
     * and for which a virtual predicate of that character info is true (0x8015AD90). Rejected hits
     * leave the bit unchanged. */
    void OnCollision(ColliDetectArgs &args);
    /* When both models exist, places them at the character info, turned about the y axis by the
     * character info's rotation. The name, and the method not being const, are inferred: the
     * bytes do not show whether the original was const. */
    void UpdateModelMatrix();
    /* Copies the owner's position, rotation, parameters (only when the owner's block is 0x20
     * bytes) and com id into the gadget, returns to state 0 and clears m_flags. */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<GadgetModelInfo> m_motionModel;
    /* 0x48 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
    /* 0x50 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;       /* m_model's part clumps */
    /* 0x58 */ boost::shared_ptr<UnkTarget_80166AF4> m_motionTarget; /* m_motionModel's, likewise */
    /* 0x60 */ SetDoorParams m_params;
    /* 0 after reading the owner, 1 locked, 2 closed, 3 opening, 4 open, 6 closing, 5 unlocked
     * (the key door has opened and stays open; inferred). */
    /* 0x80 */ s32 m_state;
    /* 0x84 */ s32 m_modelIndex; /* the entry of m_model; m_motionModel uses m_modelIndex + 1000 */
    /* 0x88 */ s32 m_comId;      /* the owner's byte that fn_800C9E50 widens */
    /* 0x8C */ boost::weak_ptr<_stMtnPlayWork> m_motion; /* m_motionModel's motion control */
    /* Bit 0: set by OnCollision, cleared at the start of each UpdateBody; bit 1: set while the
     * door opens or closes (the moving sound plays). ReadOwner clears both. */
    /* 0x94 */ UnkBitFlags_80014E04 m_flags;
};
CHECK_SIZEOF(SetBaseDoor, 0x98);

} // namespace Door

#endif
