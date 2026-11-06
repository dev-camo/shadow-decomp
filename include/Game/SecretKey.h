#ifndef GAME_SECRETKEY_H
#define GAME_SECRETKEY_H

#include "Game/EffectElement.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/fn_8011BE9C.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace SecretKey {

/* SecretKey::SecretKey (class name from RTTI; the gadget's descriptor is "SECRETKEY"): a key with
 * model 0xDE, effect 0x8D at its position and a Target. Construction and a reset set state 0.
 * UpdateBody first updates the model, which can run the collision callback, and then reads the
 * state. The callback sets state 2 whenever fn_8015AC84 is true for the hit, whatever the state
 * was. In state 0 the key finishes if its id is already in the file's key list and otherwise moves
 * to state 1, where it waits. In state 2 it passes its id to fn_8011BED0 (stored only when the
 * list has a free entry), requests sound 0xE017 from fn_801D560C, whose result is not checked, and
 * finishes. A hit for which fn_8015AC84 returns true, reported by the model update of the first
 * update after construction or a reset, therefore takes the state 2 path without the list check;
 * a rejected hit leaves state 0, and the list check runs (behaviour inferred). Method and member
 * names are inferred. */
class SecretKey : public Gadget {
  public:
    SecretKey(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SecretKey();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and key id into the gadget and clears m_state (name
     * inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model, while there is one, at the character info's position and rotation, then
     * passes that position to the effect, when its lock is not null, and to the Target, when
     * there is one (name inferred). */
    void UpdateModelMatrix();
    /* The model's collision callback: a hit for which fn_8015AC84 is true sets m_state to 2, in
     * any state (name inferred). */
    void OnCollision(ColliDetectArgs &args);
    /* Passes m_keyId and 1 to fn_801690D0 for fn_800CB69C's table (which ignores ids outside 0-255
     * and a missing table), passes the gadget to fn_8015A794 and sets Task flag 0x01 (name
     * inferred). */
    void Finish();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* ReadOwner sets it to 0, UpdateBody to 1 in state 0 when the key is not in the list, and
     * OnCollision to 2. UpdateBody also handles 3, which nothing in this file stores. */
    /* 0x40 */ s32 m_state;
    /* 0x44 */ boost::weak_ptr<Effect::System::EffectElement> m_effect; /* effect 0x8D */
    /* The owner's byte that fn_800C9E50 widens (the value sibling gadgets store as m_comId); the
     * id the key list stores (name inferred). */
    /* 0x4C */ s32 m_keyId;
    /* 0x50 */ boost::shared_ptr<Target> m_target;
};
CHECK_SIZEOF(SecretKey, 0x58);

} // namespace SecretKey

/* The file's list of five key ids, in which -1 marks a free entry (behaviour inferred; the
 * original names are unknown, and the bool and u32 results and the s32 parameter are inferred). */
extern "C" {
/* Copies the list into the five words at 0x4C of the record of fn_800BB74C's object that
 * lbl_8057D6A0.field_0xA8.field_0x4 selects and returns true; returns false without copying when
 * fn_800BB74C returns null. Its only caller is fn_80204D00. */
bool fn_8011BDA4(void);
/* Sets every entry to -1, then copies those five words back when fn_800BB74C does not return
 * null. Its callers are the SECRETKEY descriptor's first hook and fn_80335808, which calls it
 * when byte 0xB1 of lbl_8057D6A0 is set. */
void fn_8011BE14(void);
/* fn_8011BE9C, declared in Game/fn_8011BE9C.h, returns the number of entries that are not -1. Its
 * callers are the SECRETDOOR gadget and fn_803357B8, which returns 0 instead when byte 0xB1 of
 * lbl_8057D6A0 is set. */
/* Stores id in the first entry that is -1, passes the address of field_0x724 of fn_80079808's
 * object to fn_803357F4 and returns true; returns false when no entry is -1. Its only caller is
 * SecretKey::UpdateBody. */
bool fn_8011BED0(s32 id);
}

#endif
