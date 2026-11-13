#ifndef GAME_WEAPONITEM_H
#define GAME_WEAPONITEM_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class CharInfo;
class UnkWeaponModel_800631FC;
struct UnkWeaponModelDesc_8006465C;

namespace Weapon {

class WeaponBase;

/* Weapon::Item (class name from RTTI, with no bases): a weapon as an object of the stage, which the
 * weapon type table's records create. Its constructors (0x8005F314, 0x8005F380) store the vtable,
 * create the 4-byte model object at 0x4 and set the word at 0xC to -1; the creator stores the type
 * at 0x8. Only what the SET_WEAPON gadget uses is declared; the names are placeholders or inferred
 * from the bodies. */
class Item {
  public:
    /* The constructor at 0x8005F380: builds the model from desc for an empty owner. The weapon
     * files' item factories pass their item model description. The const on desc and its pointer
     * spelling are inferred (the body only forwards the address, which a reference would too). */
    Item(const UnkWeaponModelDesc_8006465C *desc);
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
    void fn_8005F274(const Fn8040Vector3 *position, const UnkRotation3 *rotation);

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
/* Sets the type's flag byte to 1 (behaviour from its body); the weapon files' load hooks call it
 * for their type. */
void fn_8005B760(int type);
/* Returns the name at the start of the type's description, or an empty string when the type has
 * none. */
const char *fn_8005BEEC(int type);
/* Returns the two floats at 0x10 and 0x14 of the type's description, or lbl_805F1358 when the type
 * has none (the SET_WEAPON gadget sizes the weapon's probe with them). */
Fn8040Vector2 fn_8005B898(int type);
/* Returns the float at 0x28 of the type's description, or the float at USA (GUPE8P) 0x805F1E38
 * when the type has none (behaviour from its body). */
float fn_8005B904(int type);
}

/* A weapon type's description, which the type table's records point to (placeholder name; the
 * accessors at 0x8005B84C-0x8005B9D4 and fn_8005BEEC read its members, behaviour inferred from
 * their bodies). */
struct UnkWeaponDescription_8005B9D4 {
    /* 0x00 */ const char *name; /* returned by fn_8005BEEC */
    /* Creates the weapon that a character carries: fn_8005BBF4 passes its character-info weak
     * pointer and uses the returned weapon. The gun description at 0x80541E7C has fn_801F3F58
     * here, which constructs its weapon with that owner (behaviour inferred). */
    /* 0x04 */ Weapon::WeaponBase *(*field_0x4)(const boost::weak_ptr<CharInfo> &owner);
    /* 0x08 */ Weapon::Item *(*createItem)(void); /* fn_8005B9D4 creates the type's item with it */
    /* fn_8005B84C passes its second argument to it; the callers pass a matrix (inferred from the
     * matrix rotation one of them applies to it first). */
    /* 0x0C */ void (*field_0xC)(const Fn8040Matrix34 *matrix);
    /* fn_8005B898 returns this float and the next as a Fn8040Vector2. The pair is probably one
     * Fn8040Vector2 member (inferred): the GunBattery and BKBattery descriptions get these two
     * floats from a start-up function and everything else statically. MWCC rejects a brace
     * initialiser for a struct with a member that has a constructor, and a constructor for the
     * whole struct would initialise every member at start-up, so the floats stay separate. */
    /* 0x10 */ float field_0x10;
    /* 0x14 */ float field_0x14;
    /* 0x18 */ int field_0x18;   /* returned by fn_8005B9AC */
    /* 0x1C */ int field_0x1C;   /* returned by fn_8005B984 */
    /* 0x20 */ int field_0x20;   /* fn_8005B954 returns its low 16 bits */
    /* 0x24 */ float field_0x24; /* returned by fn_8005B92C */
    /* 0x28 */ float field_0x28; /* returned by fn_8005B904 */
    /* 0x2C */ float field_0x2C; /* returned by fn_8005B8DC */
};
CHECK_SIZEOF(UnkWeaponDescription_8005B9D4, 0x30);

#endif
