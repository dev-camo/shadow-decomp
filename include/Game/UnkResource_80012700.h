#ifndef GAME_UNKRESOURCE_80012700_H
#define GAME_UNKRESOURCE_80012700_H

#include <dolphin/types.h>
#include "layout_check.h"

/* One entry of the table that a resource keeps at 0x30 (placeholder name; the entries are 0x1C
 * bytes apart and fn_80208848 finds one by the word at 0x4). Only name and id are mapped. */
struct UnkResourceRecord_80208848 {
    /* 0x0 */ const char *name;
    /* 0x4 */ s32 id;
    /* 0x8 */ u32 field_0x8; /* ThreatObj passes it to fn_801D54F4 as a 16-bit value */
    /* 0xC */ u8 field_0xC[0x1C - 0xC];
};
CHECK_SIZEOF(UnkResourceRecord_80208848, 0x1C);

/* The cached resource manager and the payload consumed by fn_80208848/80208888
 * (placeholder names; the original classes are unknown). */
class UnkResourceManager_800122D4;
class UnkResource_80012700;
class Resource;

extern "C" {
UnkResourceManager_800122D4 *fn_800122D4(void);
/* Returns the resource's 32-bit indexed value, or zero when key is absent (inferred). */
u32 fn_80012608(UnkResourceManager_800122D4 *manager, int key, int index);
void *fn_8001277C(UnkResourceManager_800122D4 *manager, int key, int index);
/* Unloads and removes the resource registered under key (behaviour inferred). */
void fn_800127F8(UnkResourceManager_800122D4 *manager, int key);
/* Registers resource when key is absent; returns whether it was inserted (inferred). */
bool fn_8001299C(UnkResourceManager_800122D4 *manager, int key, Resource *resource);
/* Looks up the entry for key arg1 in manager (fn_800124E4, compared with fn_8001247C's result);
 * when present, returns its named file payload through resource virtual slot 0x1C, otherwise null.
 * Payload types vary by file kind; callers select the corresponding type. */
void *fn_80012700(UnkResourceManager_800122D4 *manager, int arg1, const char *name);
/* Returns the record of resource whose word at 0x4 equals id, or null (behaviour inferred). */
UnkResourceRecord_80208848 *fn_80208848(UnkResource_80012700 *resource, int id);
/* Returns the record whose name equals name (strcmp), or null (behaviour inferred). */
UnkResourceRecord_80208848 *fn_80208888(UnkResource_80012700 *resource, const char *name);
}

#endif
