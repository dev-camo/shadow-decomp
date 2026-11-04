#ifndef GAME_UNKOWNERSET_80260E48_H
#define GAME_UNKOWNERSET_80260E48_H

#include <dolphin/types.h>
#include "layout_check.h"

class UnkGadgetOwner_800C9CD0;

/* The 0x10-byte object that the ELEC_SECURITY gadget type creates when it is loaded (placeholder
 * name; the class has no RTTI record). Its constructor (0x80260E48) walks the owner lists 1-255 and
 * adds every owner whose kind (fn_800C9D24) equals kind, paired with the id of its list; its
 * destructor is 0x80260DF4. The container at 0x0 is not mapped. Method names are inferred. */
class UnkOwnerSet_80260E48 {
  public:
    UnkOwnerSet_80260E48(u16 kind);
    ~UnkOwnerSet_80260E48();

    /* Removes owner from the set; true when owner was in the set and the set is empty afterwards,
     * false otherwise (behaviour inferred from 0x80260C18). */
    bool RemoveOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Adds owner paired with its com id (fn_800C9E60) (behaviour inferred from 0x80260CE8). */
    void AddOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* 0x00 */ u8 field_0x0[0x10];
};
CHECK_SIZEOF(UnkOwnerSet_80260E48, 0x10);

#endif
