#ifndef GAME_SETWEAPON_H
#define GAME_SETWEAPON_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Task.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/WeaponItem.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class CharCommand;

namespace SetWeapon {

/* SetWeapon::Arms (class name from the RTTI of boost's checked_deleter<SetWeapon::Arms>, which owns
 * it through shared pointers): a weapon item lying in the stage with the model 0xDF and a
 * character info whose command handler gives the weapon to the character that uses it. Member
 * names and method names are inferred from the code. */
class Arms {
  public:
    /* Sets that bit of m_flags when bit is not 0 (fn_8011E630 passes 1, which keeps UpdateModels
     * from placing the item; the other creators pass 0). */
    Arms(const boost::shared_ptr<Weapon::Item> &item, u32 bit);
    ~Arms();

    /* Moves the character info and updates the models (name inferred). rotation is not const:
     * the original loads each of its components only after storing the previous one, as this
     * compiler does when the stores may change the source. */
    void SetTransform(const Fn8040Vector3 *position, UnkRotation3 *rotation);
    /* Places the model at the character info, and the item 5 units above it unless bit 1 of
     * m_flags is set (name inferred). */
    void UpdateModels();
    /* Advances the model and the item by dt (name inferred). */
    void Update(float dt);
    /* Returns fn_80169BE4 of the character info's position: false while a player is closer than
     * the square root of squaredDistance (name inferred). */
    bool IsFarFromPlayers(float squaredDistance);
    /* Passes on to the item's fn_8005F09C when there is an item; WeaponTask calls it once, when
     * its time falls to 3 seconds (purpose unknown). */
    void fn_8011F350();
    /* The command handler of the character info: on a UseCommand, unless the weapon was taken
     * already, sends a Player::AddWeaponCommand for the item to the user, calls the model's
     * fn_80161A24 with 0 and sets bit 0 of m_flags (name inferred). */
    void OnCommand(CharCommand &command);

    /* Whether a character took the weapon (bit 0 of m_flags; name inferred). */
    bool IsTaken() const {
        return m_flags.Test(0);
    }

    /* 0x00 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x08 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x10 */ boost::shared_ptr<Weapon::Item> m_item;
    /* 0x18 */ UnkBitFlags_80014E04 m_flags;
};
CHECK_SIZEOF(SetWeapon::Arms, 0x1C);

/* SetWeapon::WeaponTask (class name from RTTI, with Task as its only base): the task that keeps a
 * weapon lying in the stage for 30 seconds (behaviour inferred). It finishes when the time runs
 * out, when every player is at least 1000 units away or when the weapon has been taken. Member
 * names are inferred from the code. */
class WeaponTask : public Task {
  public:
    WeaponTask(Task *parent, boost::shared_ptr<Arms> arms);
    virtual ~WeaponTask();
    virtual void Update(float dt);

    static const char *s_name;

    /* 0x28 */ boost::shared_ptr<Arms> m_arms;
    /* 0x30 */ float m_timer; /* seconds left */
    /* Bit 2 finishes the task, bit 3 is set from 3 seconds before the end, bit 4 once Arms's
     * fn_8011F350 has been called (inferred). */
    /* 0x34 */ UnkBitFlags_80014E04 m_state;
};
CHECK_SIZEOF(SetWeapon::WeaponTask, 0x38);

/* SetWeapon::SetWeaponTask (class name from RTTI; the gadget's descriptor is "SET_WEAPON"): the
 * gadget that places a weapon of the type its "weapon" parameter selects. Member names and the
 * names of the non-virtual methods are inferred from the code. */
class SetWeaponTask : public Gadget {
  public:
    SetWeaponTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetWeaponTask();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, rotation and weapon type from the owner (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Moves the weapon to the character info's position and rotation (name inferred). */
    void UpdateArms();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<Arms> m_arms;
    /* Holds the weapon's position and rotation (m_position, m_rotation), which ReadOwner fills
     * and UpdateArms passes on. */
    /* 0x38 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x40 */ s32 m_weaponParam; /* the "weapon" parameter */
    /* 0x44 */ s32 m_weaponType;  /* m_weaponParam as last read */
};
CHECK_SIZEOF(SetWeapon::SetWeaponTask, 0x48);

} // namespace SetWeapon

extern "C" {
/* Puts the weapon item down at position, turned towards direction, and keeps it there with a
 * WeaponTask (behaviour inferred; the original name is unknown). The function at 0x80221DD0 passes
 * an item it holds. */
void fn_8011E630(const boost::shared_ptr<Weapon::Item> &item, const Fn8040Vector3 *position,
                 const Fn8040Vector3 *direction);
/* Creates a weapon item of the type on the ground below position (the nearest hit of a probe 1000
 * units down, or position itself), with the given rotation, and keeps it there with a WeaponTask
 * (behaviour inferred; the original name is unknown). A breaking active stone statue passes 0x1F,
 * its position and its rotation; Stick::Stick::UpdateBody calls it twice as well (USA
 * (GUPE8P) 0x80122900 and 0x8012296C). rotation is passed on to Arms::SetTransform. */
void fn_8011E938(int type, const Fn8040Vector3 *position, UnkRotation3 *rotation);
}

#endif
