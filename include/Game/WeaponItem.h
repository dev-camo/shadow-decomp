#ifndef GAME_WEAPONITEM_H
#define GAME_WEAPONITEM_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class UnkWeaponModel_800631FC;

namespace Weapon {

/* Weapon::Item (class name from RTTI, with no bases): a weapon as an object of the stage, which the
 * weapon type table's records create. Its constructors (0x8005F314, 0x8005F380) store the vtable,
 * create the 4-byte model object at 0x4 and set the word at 0xC to -1; the creator stores the type
 * at 0x8. Only what the SET_WEAPON gadget uses is declared; the names are placeholders or inferred
 * from the bodies. */
class Item {
  public:
    virtual ~Item();
    /* Stores value at 0xC (the type table's creator passes its second argument here). */
    virtual void fn_8005BBEC(int value);
    /* Passes dt to fn_800632A0 of the model (name inferred). */
    virtual void Update(float dt);

    /* The weapon type (name inferred from the creator, which stores the type it was asked for).
     * The SET_WEAPON gadget's file has the copy of this getter that the linker kept. */
    int GetType() const {
        return m_type;
    }

    /* Passes the model to fn_800631D0, which sets bit 1 of a flag word of the object it refers to
     * (purpose unknown). */
    void fn_8005F09C();
    /* Passes position and rotation to fn_800632E8 of the model, then a constant to
     * fn_800632A0. */
    void fn_8005F274(const Fn8040Vector3 *position, const Fn8040Vector3 *rotation);

    /* The weapon model object that the functions at 0x800631D0-0x80063484 work on, as at 0x1C of
     * Weapon::WeaponBase (type inferred from those functions; it is null when not created). */
    /* 0x04 */ UnkWeaponModel_800631FC *m_model;
    /* 0x08 */ int m_type;
    /* 0x0C */ int field_0xC;
};
CHECK_SIZEOF(Weapon::Item, 0x10);

} // namespace Weapon

/* The weapon type table: sixteen-byte records whose first word points to the type's description
 * and whose byte at 0x4 is a flag (placeholder names; behaviour inferred from the bodies). */
extern "C" {
/* Creates the weapon item of the type with the description's factory, stores the type in it, passes
 * arg1 to its fn_8005BBEC and returns it, or an empty pointer when the type is not available. */
boost::shared_ptr<Weapon::Item> fn_8005B9D4(int type, int arg1);
/* Returns the type's flag byte. */
bool fn_8005BF14(int type);
/* Returns the name at the start of the type's description, or an empty string when the type has
 * none. */
const char *fn_8005BEEC(int type);
/* Returns the two floats at 0x10 and 0x14 of the type's description, or lbl_805F1358 when the type
 * has none (the SET_WEAPON gadget sizes the weapon's probe with them). */
Fn8040Vector2 fn_8005B898(int type);
}

#endif
