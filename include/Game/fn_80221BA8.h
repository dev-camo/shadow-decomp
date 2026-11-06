#ifndef GAME_FN_80221BA8_H
#define GAME_FN_80221BA8_H

#include "Game/fn_8040_vector_types.h"

extern "C" {
/* Allocates a 0x80-byte task under task layer 11 and, when the allocation succeeds (null check
 * at 0x80221BEC, USA (GUPE8P)), constructs it (0x8022212C): the task copies arg2 and keeps the
 * Weapon::Item that fn_8005B9D4(itemType, arg4) creates (WeaponItem.h). When that item is null
 * (check at 0x802221E4) the constructor sets the task's finished flag (bit 0 of m_flags) and
 * returns; otherwise it passes the item, two of the task's members, position and rotation to
 * fn_8005F0F4 (0x8022222C). Behaviour inferred from the two bodies; the original name is unknown.
 * The BA GUNSHIP gadget passes its character info's position and rotation, (0, 1, 0), 0x10 and
 * -1 when it breaks. */
void fn_80221BA8(const Fn8040Vector3 *position, const Fn8040Vector3 *rotation,
                 const Fn8040Vector3 *arg2, int itemType, int arg4);
}

#endif
