#ifndef GAME_UNKMANAGER_80117514_H
#define GAME_UNKMANAGER_80117514_H

#include <dolphin/types.h>
#include "layout_check.h"

/* The 0x38-byte object that UnkManager_80117514 owns (placeholder name after its constructor at
 * 0x802E5B74); its layout is not mapped here. */
class UnkObject_802E5B74;

/* The object that UnkSingleton<UnkManager_80117514>::Instance (USA (GUPE8P) 0x80117514, kept in
 * Game/SavePoint.cpp and called from other units) returns (placeholder name after that address;
 * the original class is unknown). Its constructor (0x802E5B2C) stores a new
 * UnkObject_802E5B74 at 0x0, or null when the allocation fails; its destructor (0x802E5A5C)
 * destroys that object through fn_802E5ABC and clears the pointer when it is not null. */
class UnkManager_80117514 {
  public:
    UnkManager_80117514();
    ~UnkManager_80117514();

    /* 0x0 */ UnkObject_802E5B74 *field_0x0;
};
CHECK_SIZEOF(UnkManager_80117514, 0x4);

extern "C" {
/* True when the float at 0x18 of the object that the manager's first word points to equals 0,
 * without a null check (behaviour inferred from its body; meaning unknown). */
bool fn_802E5810(UnkManager_80117514 *manager);
}

#if !defined(VERSION_GUPE8P)
/* Japan and PAL only (0x80342284 in Japan, 0x80342AB0 in PAL; the USA build has no copy): returns
 * the word at 0x2C of each of two objects that two static pointers hold, or 0 for a null pointer,
 * combined with a bitwise or (behaviour inferred from its body; meaning unknown). */
extern "C" int fn_80342284(void);
#endif

#endif
