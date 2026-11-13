#ifndef GAME_UNKFILEMANAGER_800371D8_H
#define GAME_UNKFILEMANAGER_800371D8_H

#include <dolphin/types.h>
#include "layout_check.h"

/* The object that fn_800371D8 returns (placeholder class name; the original class is unknown): a
 * static object (at 0x805EE0C8 in the USA release, GUPE8P) that the function constructs
 * (fn_80045290) on the first call, registering its destructor (fn_80045254), and caches in a
 * static pointer. Only the word at 0x4 is mapped. */
class UnkFileManager_800371D8 {
  public:
    /* 0x0 */ u8 field_0x0[0x4];
    /* The constructor stores fn_80044E80 here, which returns whether fn_8047A67C returns non-zero
     * for the path (the parameter and result types are inferred). TGiPause calls it with the copy
     * that fn_80051810 makes of a texture dictionary's path before it reads the file. */
    /* 0x4 */ bool (*field_0x4)(const char *path);
};
/* Checks only the mapped prefix, not the object's size. */
CHECK_SIZEOF(UnkFileManager_800371D8, 0x8);

/* The names, the parameter types and the const qualifiers are not known (inferred); the
 * behaviour comes from the bodies. */
extern "C" {
UnkFileManager_800371D8 *fn_800371D8(void);
/* Opens the file at path with fn_800513E0 (mode "rb"), reads it into a buffer allocated with
 * fn_80049B90 and returns the buffer; when size is not null it also stores the file's size (the
 * word at 0x4 of the file record) times the count that fn_8047A3E0 returns in *size. It returns
 * null and leaves *size unchanged when fn_800513E0 returns null. It does not read manager. */
void *fn_8004509C(UnkFileManager_800371D8 *manager, const char *path, u32 *size);
/* Calls the function pointer at 0x10C of the object that lbl_805F15B4 points to with data (the
 * void return is inferred). */
void fn_80049B34(void *data);
/* Calls the function pointer at 0x108 of the same object with size and kind + 0x32 and returns
 * its result; callers use the result as storage of size bytes and release it with fn_80049B34
 * (the parameter names and types are inferred). */
void *fn_80049B90(u32 size, int kind);
}

#endif
