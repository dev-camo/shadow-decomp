#ifndef GAME_UNKARCHIVE_8004CA28_H
#define GAME_UNKARCHIVE_8004CA28_H

#include <dolphin/types.h>
#include <cstring>
#include "layout_check.h"

/* The object whose entries fn_8004CA28 returns (placeholder name; the original class is unknown
 * and its layout is not mapped here). The event task keeps a pointer to one and loads the event's
 * files from it by index. */
class UnkArchive_8004CA28;

/* An entry of the archive (placeholder name). fn_8004CA28 steps over the entries in units of 0x38
 * bytes. Callers read a file name from the start of the entry, and fn_8004C3E0 sizes the buffer
 * it allocates for the entry from the word at 0x2C; the name's own length is not known, 0x2C is
 * only the room in front of that word. */
struct UnkArchiveEntry_8004CA28 {
    /* 0x00 */ char name[0x2C];
    /* 0x2C */ u32 field_0x2C;
    /* 0x30 */ u8 field_0x30[0x38 - 0x30];
};
CHECK_SIZEOF(UnkArchiveEntry_8004CA28, 0x38);

/* The function that the task of fn_8004C358 calls with its result pointer and the user pointer
 * when it finishes (name inferred; the parameter types are inferred from the two words that
 * fn_8004C3E0 passes). The result pointer is the loaded data, or null when the read failed:
 * fn_8004C3E0 then frees its buffer and still calls the function. */
typedef void (*UnkArchiveCallback_8004C358)(void *data, void *user);

/* The names, the parameter types and the const qualifiers are not known (inferred); the
 * behaviour comes from the bodies. */
extern "C" {
/* Returns entry index of the archive, or null when the archive has neither a non-zero word at
 * 0x2C nor a non-empty string at 0x0, when index is below 2, or when index is not below 2 plus
 * the count at 0x10 of the object that the archive's word at 0x34 points to. */
UnkArchiveEntry_8004CA28 *fn_8004CA28(UnkArchive_8004CA28 *archive, int index);
/* Creates the 0x60-byte task that loads entry index (fn_8004C69C). With a callback it returns
 * null, and the task later passes its result pointer (null when the read failed) and user to
 * the callback. Without one it steps the task itself (fn_8004C3E0) until the task finishes and
 * returns the result pointer. */
void *fn_8004C358(UnkArchive_8004CA28 *archive, int index, UnkArchiveCallback_8004C358 callback,
                  void *user);
}

/* Matching workaround: an inferred inline function. Its name, the const qualifiers of its
 * parameters and the header that declares it are not known; it is here because the three Load
 * functions named below pass it the name of an archive entry. It returns whether name contains a '.'
 * and the four characters from its first '.' equal the first four of extension.
 *
 * Evidence (addresses of the USA release, GUPE8P):
 * - The function at 0x80171E74 (0x8017221C in Japan, GUPJ8P, and 0x80172258 in PAL, GUPP8P) has
 *   the 22 instruction words that this definition gives when the game's compiler settings emit
 *   it as a function, apart from the two call displacements. No other function in the game has
 *   those words. Two Load functions in two files call it with the name of an entry and their
 *   extension: 0x80171DEC with ".SNB" and 0x8022887C with ".SNN".
 * - 46 places contain the same test in line, build its result as a byte and then test the byte:
 *   UnkCenFile_80329514::Load (0x80329448-0x80329478) and 45 in unrecovered code, of which 12
 *   are in the event code (fn_80170DF8 ".ADB", fn_80170E5C ".CEN", fn_80170EC4 ".SNN",
 *   fn_80170F2C ".SNB", fn_80170F94 ".TXD", three in fn_80172248 and four in fn_801729F0 with
 *   ".DFF", ".MTN", ".BON" and ".DMA") and 33 are between 0x80013CC8 and 0x80160D3C. The game's
 *   settings expand only functions that are declared inline.
 * - Why the two Load functions call the function is not established. A Load of their shape
 *   expands it with the game's settings (as UnkCenFile_80329514::Load does); compiled with
 *   inline expansion off it has the 34 words of the function at 0x80171DEC, call displacements
 *   and addresses apart, and the compiler puts the function right after it, where the original
 *   has it. In the file of 0x80171DEC, fn_80171BA0 calls a constructor that only stores two
 *   floats (0x80171D6C) and a smart pointer's operator-> (0x80171D64) for the arguments of a
 *   call; both follow fn_80171BA0, and a test with the game's settings expands both there.
 * - Written in place as one condition, `if (dot != NULL && strncmp(dot, extension, 4) == 0)`,
 *   the test makes UnkCenFile_80329514::Load branch on the two conditions without building the
 *   byte, and Load is 0xA4 bytes instead of 0xB8. A bool local assigned true and false in the two
 *   arms of an if and then tested gives the original words: that is this function's body
 *   written out at the call site.
 * Revisit when a file of the two Load functions or other code with the test is recovered. */
inline bool HasExtension(const char *name, const char *extension) {
    const char *dot = strchr(name, '.');
    if (dot != NULL && strncmp(dot, extension, 4) == 0) {
        return true;
    }
    return false;
}

#endif
