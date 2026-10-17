#ifndef LAYOUT_CHECK_H
#define LAYOUT_CHECK_H

/* Compile-time checks of structure layout as the GameCube compiler (MWCC) lays it out:
 *
 *     CHECK_SIZEOF(RwRaster, 0x34);
 *     CHECK_OFFSETOF(RwRaster, cType, 0x20);
 *
 * They emit no code or data. A failed check stops the build with an error about the negative
 * array size of struct_layout_check, so a header edit that moves a member or changes a size is
 * caught in every file that includes it, not only in the files whose objects are compared.
 *
 * Every passing check redeclares the same one-element array, which both C and C++ allow. C++
 * uses a typedef, so checks inside and outside extern "C" blocks do not conflict.
 *
 * The values describe the GameCube ABI (4-byte pointers, MWCC alignment and enum rules), so the
 * checks are only active under MWCC. Other compilers, for example a host build or an editor, keep
 * compiling the same source with their own layout. */

#if defined(__MWERKS__)
#ifdef __cplusplus
#define LAYOUT_CHECK(condition) typedef char struct_layout_check[(condition) ? 1 : -1]
#else
#define LAYOUT_CHECK(condition) extern char struct_layout_check[(condition) ? 1 : -1]
#endif
#else
#ifdef __cplusplus
#define LAYOUT_CHECK(condition) typedef char struct_layout_check_disabled
#else
#define LAYOUT_CHECK(condition) extern char struct_layout_check_disabled
#endif
#endif

/* The offset of member in type, as an integer constant expression MWCC accepts in C and C++. */
#define LAYOUT_OFFSETOF(type, member) ((unsigned long)&((type *)0)->member)

#define CHECK_SIZEOF(type, size) LAYOUT_CHECK(sizeof(type) == (size))
#define CHECK_OFFSETOF(type, member, offset) LAYOUT_CHECK(LAYOUT_OFFSETOF(type, member) == (offset))

#endif /* LAYOUT_CHECK_H */
