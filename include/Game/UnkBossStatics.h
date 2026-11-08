#ifndef GAME_UNKBOSSSTATICS_H
#define GAME_UNKBOSSSTATICS_H

#include "layout_check.h"

/* Matching workaround: placeholder storage for two unknown .bss ranges of the boss files that
 * include this header; the number, types and names of the objects in them are unknown. Each of
 * these files' .bss holds 0x30 bytes before its first sphere and 0x94 bytes after its last vector.
 * Nothing in the DOL reads or writes either range. __sinit forms the address of the start of the
 * head range (USA (GUPE8P) 0x80583E68 in HeavyDog0410, 0x80583F68 in HeavyDog0510) only as a base,
 * and reaches the spheres and vectors by adding 0x30 and higher offsets to it. The
 * float arrays only give the measured sizes and an alignment of 4 (the offsets rule out 8: a
 * 0x94-byte tail aligned to 8 would make a 0xF0-byte .bss 0xF8 bytes long). Each file defines one
 * static of each type, the head before its first sphere and the tail after its last vector (MWCC
 * lays out .bss in definition order); the arrays need no construction, so __sinit gains no code.
 * Without them the spheres' and vectors' offsets in __sinit and in the parameter tables'
 * relocations are 0x30 too low. Revisit when the real objects are identified. */
typedef float UnkBossStaticHead[12];
typedef float UnkBossStaticTail[37];
CHECK_SIZEOF(UnkBossStaticHead, 0x30);
CHECK_SIZEOF(UnkBossStaticTail, 0x94);

#endif
