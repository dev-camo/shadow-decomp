#ifndef GAME_UNKREGISTRY_801DE414_H
#define GAME_UNKREGISTRY_801DE414_H

#include "rw/rpworld.h"
#include "Game/UnkString_8004F888.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* The registry of named clumps that fn_80130B2C returns (placeholder; the constructor is
 * fn_801DE414 and the destructor fn_801DE2E8). */
class UnkRegistry_801DE414 {
  public:
    UnkRegistry_801DE414();
    ~UnkRegistry_801DE414();

    /* 0x00 */ u8 field_0x0[0x14];
};
CHECK_SIZEOF(UnkRegistry_801DE414, 0x14);

extern "C" {
/* Registers the clump under name, or counts another user of it when the name is known (behaviour
 * inferred from the body). Returns true when it adds a new entry, false when the name is already
 * registered. */
bool fn_801DE0EC(UnkRegistry_801DE414 *registry, const UnkString_8004F888 &name, RpClump *clump);
/* Releases one user of the entry under name; the entry goes with the last one (behaviour inferred
 * from the body). */
void fn_801DDFDC(UnkRegistry_801DE414 *registry, const UnkString_8004F888 &name);

/* Returns the registry, building it on the first call (defined in CircusPole.cpp). */
UnkRegistry_801DE414 *fn_80130B2C(void);
}

#endif
