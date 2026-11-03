#ifndef GAME_UNKREGISTRY_8010A458_H
#define GAME_UNKREGISTRY_8010A458_H

#include "Game/UnkString_8004F888.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* Opaque native tree positions: the first position may identify an allocated node or the
 * embedded end position. Their value-bearing representation is not recovered here. */
struct UnkRegistryPosition;

struct UnkRegistryEndPosition {
    /* 0x00 */ UnkRegistryPosition *root;
};
CHECK_SIZEOF(UnkRegistryEndPosition, 0x4);

/* Unrecovered representation of the comparison subobject. Its native construction copies one
 * byte; the observed comparison operations use the two strings without reading this byte. */
struct UnkRegistryComparison {
    /* 0x00 */ u8 unknown_0x0[1];
};
CHECK_SIZEOF(UnkRegistryComparison, 0x1);

/* The distinct registry returned by fn_8010A458 (original type name unknown).
 * Native construction and destruction preserve its allocator/tree subobject lifetimes. */
class UnkRegistry_8010A458 {
  public:
    UnkRegistry_8010A458();
    ~UnkRegistry_8010A458();
    /* Releases one reference to the named entry and erases it after its last user (inferred). */
    void fn_801EB550(const UnkString_8004F888 &name);

    /* 0x00 */ u32 count;
    /* 0x04 */ UnkRegistryEndPosition end;
    /* 0x08 */ UnkRegistryComparison comparison;
    /* May hold the embedded end or an allocated position; their common type is unrecovered. */
    /* 0x0C */ void *first;

  private:
    /* Registry copying has not been recovered. */
    UnkRegistry_8010A458(const UnkRegistry_8010A458 &);
    UnkRegistry_8010A458 &operator=(const UnkRegistry_8010A458 &);
};
CHECK_SIZEOF(UnkRegistry_8010A458, 0x10);

extern "C" UnkRegistry_8010A458 *fn_8010A458();

#endif
