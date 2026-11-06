#ifndef GAME_SAVEPOINT_H
#define GAME_SAVEPOINT_H

#include "Game/CharCommand.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/SavePointSelect.h"
#include "Game/Task.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace SavePoint {

/* SavePoint::SavePoint (class name from RTTI; the gadget's descriptor is "SAVEPOINT"): a gadget of
 * the model resource 0xDC. ReadOwner puts it in state 0. In state 0 an update goes to state 2 when
 * bit 0x400000 of the owner's flag word is set, creating the second model first when it does not
 * exist, and to state 1 when the bit is clear. Only in state 1 does the first model's collision
 * callback act: it passes the hits to fn_8015AA84 with the hit callback (fn_80118F68), which acts
 * on the first character info it is called with; when the hit callback has acted, the gadget then
 * creates the second model when it does not exist, sets bit 0x400000 of the owner's flag word and
 * goes to state 2. In state 2 a command with id 0xD sent to its character info sets flag 0. At the
 * end of every update a set flag 0 is cleared and flag 1 set, so the update after that, in state
 * 2, acts on flag 1: it clears flag 1 when CanSave is false; when CanSave is true, fn_80330CB0 is
 * true and fn_8016F3A0 is positive, it creates a SavePointSelectTask under task layer 11 (the
 * callbacks are assigned without checking the allocation), binds OnSelect and OnCancel to it, goes
 * to state 3, passes true to fn_80203970 for the record at 0xA8 of lbl_8057D6A0, clears flag 1,
 * passes the object of fn_800BADE8 to fn_801E2F28 and fn_801E2EDC and stores 0 in its word at
 * 0x30; in any other case flag 1 stays set for the next update. Both callbacks store state 2. The
 * second model (the entry with id 1 of the same resource) gets the first model's matrix. While the
 * byte at 0x84 of lbl_8057D6A0 is set, the PauseWatch task the constructor creates clears flags 0
 * and 1 in its own updates. Behaviour inferred; method and member names marked "(name inferred)"
 * are not original. */
class SavePoint : public Gadget {
  public:
    SavePoint(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SavePoint();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11; nothing is constructed when the allocation fails
     * (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the character info, copies its parameter
     * block into m_priority only when the owner's parameter size is 4 (fn_800C9DBC compares it at
     * 0x800C9DD4), returns to state 0 and sets bit 0x200 of the owner's flag word (name
     * inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Starts from the matrix at lbl_805E0720. When the first model pointer is not null, applies
     * the character info's position and its y, x and z rotations (in that order) and gives the
     * matrix to the first model. The second model, when it exists, then gets the same matrix (the
     * unmodified one when the first model pointer is null) (name inferred). */
    void UpdateModelMatrix();
    /* The first model's collision callback; acts only in state 1 (see the class comment; name
     * inferred). */
    void OnCollision(ColliDetectArgs &args);
    /* The SavePointSelectTask's first callback. When the argument's word is not -1, it copies the
     * vector that fn_8016F344 returns for that index in row 0 and, when the character info that
     * fn_800A8B74 returns for lbl_805EE588 is alive, passes the vector and that info cast to
     * Player::Player (an empty pointer when the cast fails) to WarpHole::ManagerTask::Create,
     * which creates a task that moves that character to the vector. The cast result is not
     * checked: the call at 0x80117738 receives it as it is, and when the task's allocation succeeds
     * its constructor dereferences it (0x80129D28): when the cast has failed, that dereferences
     * an empty pointer.
     * If that call returns (or is not made), it then stores state 2 and passes 10.0 to the first
     * model's fn_80161308 without checking the model pointer (name inferred). */
    void OnSelect(SavePointSelect::Args &args);
    /* The SavePointSelectTask's second callback: stores state 2, passes false to fn_80203970 for
     * the record at 0xA8 of lbl_8057D6A0 unless the word at 0x8 of the manager of fn_80074470 is
     * 6, and stores 1 in the word at 0x30 of the object of fn_800BADE8; the argument is not used
     * (name inferred). */
    void OnCancel(SavePointSelect::Args &args);
    /* The character info's command handler: a command with id 0xD sets flag 0 when the gadget is
     * in state 2; nothing else happens (name inferred). */
    void OnCommand(CharCommand &command);
    /* Whether a save may start: false while the byte at 0x84 of lbl_8057D6A0 is set, when
     * fn_8020261C is false for the UnkSingleton<UnkManager_80117578> instance, or when the word at
     * 0x8 of the manager of fn_80074470 is 6; otherwise whether fn_802E5810 is true for the
     * UnkSingleton<UnkManager_80117514> instance, tested in that order (name inferred). */
    bool CanSave();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<GadgetModelInfo> m_secondModel;
    /* 0 after ReadOwner; 1 when the owner's bit 0x400000 was clear; 2 once that bit is set or
     * the collision callback has acted; 3 after a SavePointSelectTask was opened, until one of its
     * callbacks stores 2. */
    /* 0x48 */ s32 m_state;
    /* 0x4C */ s32 m_priority; /* the "priority" parameter */
    /* 0x50 */ UnkBitFlags_80014E04 m_flags;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(SavePoint, 0x58);

/* SavePoint::PauseWatch (class name from RTTI; the name it stores is "PauseWatch::PauseWatch"): a
 * task that the gadget's constructor creates as the gadget's child (nothing is constructed when
 * the allocation fails). Each update clears flags 0 and 1 of the gadget's flag word while the byte
 * at 0x84 of lbl_8057D6A0 is set (behaviour inferred; the member name is inferred). */
class PauseWatch : public Task {
  public:
    /* Stores flags and sets bit 0x100 of the task flags (meaning unknown). */
    PauseWatch(Task *parent, UnkBitFlags_80014E04 *flags);
    /* Clears m_gadgetFlags before Task's destructor runs. */
    virtual ~PauseWatch();
    /* Does nothing when m_gadgetFlags is null. */
    virtual void Update(float dt);

    static const char *s_name;

    /* 0x28 */ UnkBitFlags_80014E04 *m_gadgetFlags; /* the gadget's m_flags */
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(PauseWatch, 0x30);

} // namespace SavePoint

#endif
