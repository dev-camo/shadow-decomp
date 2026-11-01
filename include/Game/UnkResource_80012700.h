#ifndef GAME_UNKRESOURCE_80012700_H
#define GAME_UNKRESOURCE_80012700_H

#include <dolphin/types.h>
#include "layout_check.h"

/* One entry of the table that a resource keeps at 0x30 (placeholder name; the entries are 0x1C
 * bytes apart and fn_80208848 finds one by the word at 0x4). Only the word at 0x8 is mapped. */
struct UnkResourceRecord_80208848 {
    /* 0x0 */ u32 field_0x0;
    /* 0x4 */ s32 id;
    /* 0x8 */ u32 field_0x8; /* ThreatObj passes it to fn_801D54F4 as a 16-bit value */
    /* 0xC */ u8 field_0xC[0x1C - 0xC];
};
CHECK_SIZEOF(UnkResourceRecord_80208848, 0x1C);

/* The object fn_800122D4 returns (created by fn_80012308 on the first call, then cached) and the
 * object fn_80012700 returns (placeholder names; the original classes are unknown). */
class UnkResourceManager_800122D4;
class UnkResource_80012700;
class Resource;

extern "C" {
UnkResourceManager_800122D4 *fn_800122D4(void);
void *fn_8001277C(UnkResourceManager_800122D4 *manager, int key, int index);
/* Unloads and removes the resource registered under key (behaviour inferred). */
void fn_800127F8(UnkResourceManager_800122D4 *manager, int key);
/* Registers resource when key is absent; returns whether it was inserted (inferred). */
bool fn_8001299C(UnkResourceManager_800122D4 *manager, int key, Resource *resource);
/* Looks up the entry for key arg1 in manager (fn_800124E4, compared with fn_8001247C's result);
 * when there is one, returns what the virtual function at vtable offset 0x1C of the object at the
 * entry's 0x10 returns for name, otherwise null (meanings unknown). ThreatObj passes 1 as arg1 and
 * a file name as name. */
UnkResource_80012700 *fn_80012700(UnkResourceManager_800122D4 *manager, int arg1, const char *name);
/* Returns the record of resource whose word at 0x4 equals id, or null (behaviour inferred). */
UnkResourceRecord_80208848 *fn_80208848(UnkResource_80012700 *resource, int id);
/* Returns the record of resource whose word at 0x0 points to a string equal to name (compared
 * with strcmp), or null (behaviour inferred). */
UnkResourceRecord_80208848 *fn_80208888(UnkResource_80012700 *resource, const char *name);
}

#endif
