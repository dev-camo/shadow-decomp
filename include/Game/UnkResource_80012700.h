#ifndef GAME_UNKRESOURCE_80012700_H
#define GAME_UNKRESOURCE_80012700_H

#include <dolphin/types.h>
#include "layout_check.h"

/* One entry of the table that a resource keeps at 0x30 (placeholder name; the entries are 0x1C
 * bytes apart and fn_80208848 finds one by the word at 0x4). Ornament reads the last three
 * floats as a sound-position offset. They remain scalar fields because this borrowed resource
 * record has no recovered C++ subobject construction. */
struct UnkResourceRecord_80208848 {
    /* 0x0 */ const char *name;
    /* 0x4 */ s32 id;
    /* 0x8 */ u32 field_0x8; /* ThreatObj passes it to fn_801D54F4 as a 16-bit value */
    /* 0x0C */ u8 field_0xC[4];
    /* 0x10 */ float field_0x10;
    /* 0x14 */ float field_0x14;
    /* 0x18 */ float field_0x18;
};
CHECK_SIZEOF(UnkResourceRecord_80208848, 0x1C);

/* One entry of the table that a resource keeps at 0x8, with its count at 0xC (placeholder name;
 * the entries are 0x10 bytes apart and fn_80208900 finds one by name). BreakWall passes the word
 * at 0x8 to GadgetModelInfo::fn_80162A68 as its first argument. */
struct UnkResourceEntry_80208900 {
    /* 0x0 */ const char *name;
    /* 0x4 */ u8 field_0x4[0x8 - 0x4];
    /* 0x8 */ s32 field_0x8;
    /* 0xC */ u8 field_0xC[0x10 - 0xC];
};
CHECK_SIZEOF(UnkResourceEntry_80208900, 0x10);

/* The cached resource manager and the payload consumed by fn_80208848/80208888
 * (placeholder names; the original classes are unknown). */
class UnkResourceManager_800122D4;
class UnkResource_80012700;
class Resource;

extern "C" {
UnkResourceManager_800122D4 *fn_800122D4(void);
/* Returns what virtual slot 0x14 of the resource registered under key returns, or 4 when key is
 * absent (behaviour inferred; the PJS launcher's Loader waits for 3). */
int fn_80012408(UnkResourceManager_800122D4 *manager, int key);
/* Returns the resource's 32-bit indexed value, or zero when key is absent (inferred). */
u32 fn_80012608(UnkResourceManager_800122D4 *manager, int key, int index);
void *fn_8001277C(UnkResourceManager_800122D4 *manager, int key, int index);
void *fn_80012684(UnkResourceManager_800122D4 *manager, int key, int index);
/* Unloads and removes the resource registered under key (behaviour inferred). */
void fn_800127F8(UnkResourceManager_800122D4 *manager, int key);
/* Registers resource when key is absent; returns whether it was inserted (inferred). */
bool fn_8001299C(UnkResourceManager_800122D4 *manager, int key, Resource *resource);
/* Registers resource under the first absent key of the 100 from the manager's word at 0x14 on,
 * adds 1 to the manager's word at 0x10 and returns the key, or returns -1 when all 100 are taken
 * (behaviour inferred). */
int fn_800128D8(UnkResourceManager_800122D4 *manager, Resource *resource);
/* Looks up the entry for key arg1 in manager (fn_800124E4, compared with fn_8001247C's result);
 * when present, returns its named file payload through resource virtual slot 0x1C, otherwise null.
 * Payload types vary by file kind; callers select the corresponding type. */
void *fn_80012700(UnkResourceManager_800122D4 *manager, int arg1, const char *name);
/* Returns the record of resource whose word at 0x4 equals id, or null (behaviour inferred). */
UnkResourceRecord_80208848 *fn_80208848(UnkResource_80012700 *resource, int id);
/* Returns the record whose name equals name (strcmp), or null (behaviour inferred). */
UnkResourceRecord_80208848 *fn_80208888(UnkResource_80012700 *resource, const char *name);
/* Returns the entry of the table at 0x8 whose name equals name (strcmp), or null (behaviour
 * inferred). */
UnkResourceEntry_80208900 *fn_80208900(UnkResource_80012700 *resource, const char *name);
}

#endif
