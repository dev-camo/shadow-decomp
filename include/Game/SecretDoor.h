#ifndef GAME_SECRETDOOR_H
#define GAME_SECRETDOOR_H

#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace SecretDoor {

/* The values of m_state (names inferred from what UpdateBody does in each state): STATE_START is
 * set by ReadOwner, and the next update looks up the character; STATE_UNLOCK makes one removal
 * attempt (RemoveLocks with a count of 1) each time the timer runs out while m_pendingLocks is above
 * 0; STATE_OPENING waits for the end of the model's motion 0. */
enum { STATE_START = 0, STATE_WAIT_KEYS = 1, STATE_UNLOCK = 2, STATE_OPENING = 3, STATE_OPEN = 4 };

/* The number of keys (name inferred): the size of fn_8011BE9C's table, the most lock parts that
 * STATE_START removes when the owner has the bits 0x400000, and the handled count at which the door
 * opens. */
static const int KEY_COUNT = 5;

/* SecretDoor::SecretDoor (class name from RTTI; the gadget's descriptor is "SECRETDOOR"): the
 * SECRETDOOR.BIN model, a door with lock parts that compares the number that fn_8011BE9C returns
 * (the slots of its five-entry table that do not hold -1; read as the keys collected, inferred)
 * with the number it has already handled. When the number has changed and the character is close
 * enough, the door makes one timed removal attempt for each new key, every 0.5 seconds; an attempt
 * removes a lock part's clump only when a part qualifies (see RemoveLocks), and its effect and sound
 * also need the clump's frame. When the number equals the handled one and that is KEY_COUNT or more,
 * it opens (see UpdateBody). Behaviour inferred; method, member and constant names are inferred. */
class SecretDoor : public Gadget {
  public:
    SecretDoor(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SecretDoor();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the character info and its com id into
     * m_comId, and clears m_state, m_timer and m_keyCount; m_pendingLocks and m_flags keep their
     * values (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When the model exists, passes it the character info's world matrix (fn_8015CEA0) (name
     * inferred). */
    void UpdateModelMatrix();
    /* Walks the model's parts (fn_8015FA78) and removes from its world the clump of each part
     * whose word at 0x8 is nonzero and whose clump is in a world, until count clumps have been
     * removed or the parts run out. With showEffect, each removal whose clump has a frame also
     * starts effect 0x296 at the frame's world matrix and plays sound 0xE018 (name inferred; the
     * int and bool parameter types are inferred from the word and byte stores of the context). */
    void RemoveLocks(int count, bool showEffect);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* The character info that fn_800A8B74 returns for lbl_805EE588, looked up on each update in
     * STATE_START. */
    /* 0x40 */ boost::weak_ptr<CharInfo> m_character;
    /* 0x48 */ s32 m_state;    /* STATE_ values */
    /* 0x4C */ float m_timer;  /* the time left until the next removal attempt (STATE_UNLOCK) */
    /* 0x50 */ s32 m_keyCount; /* the result of fn_8011BE9C already handled */
    /* The removal attempts still to make in STATE_UNLOCK; each attempt lowers it, whether or not
     * a clump was removed. */
    /* 0x54 */ s32 m_pendingLocks;
    /* 0x58 */ u8 m_comId; /* the owner's com id (fn_800C9E60); nothing here reads it */
    /* Bit 0 is set when the door starts to open (STATE_OPENING) and cleared in STATE_OPENING once
     * fn_80160040(0) returns true; while it is set, every update plays sound 0xE019. The
     * constructor clears every bit and ReadOwner does not touch them. */
    /* 0x5C */ UnkBitFlags_80014E04 m_flags;
};
CHECK_SIZEOF(SecretDoor, 0x60);

} // namespace SecretDoor

#endif
