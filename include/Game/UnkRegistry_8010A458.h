#ifndef GAME_UNKREGISTRY_8010A458_H
#define GAME_UNKREGISTRY_8010A458_H

#include "Game/UnkString_8004F888.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* Partial storage view of the native container member. The word at +0 is its count;
 * the remaining storage includes the tree header, comparison and first position, whose
 * complete types are unrecovered. Native member construction/destruction owns their lifetimes. */
struct UnkRegistryStorage_8010A458 {
    /* 0x00 */ u32 count;
    /* 0x04 */ u8 field_0x4[0xC];
};
CHECK_SIZEOF(UnkRegistryStorage_8010A458, 0x10);

/* The distinct registry returned by fn_8010A458 (original type name unknown).
 * Native construction and destruction preserve its container subobject's lifetime. */
class UnkRegistry_8010A458 {
  public:
    UnkRegistry_8010A458();
    ~UnkRegistry_8010A458();
    /* Releases one reference to the named entry and erases it after its last user (inferred). */
    void fn_801EB550(const UnkString_8004F888 &name);

    /* 0x00 */ UnkRegistryStorage_8010A458 field_0x0;

  private:
    /* Registry copying has not been recovered. */
    UnkRegistry_8010A458(const UnkRegistry_8010A458 &);
    UnkRegistry_8010A458 &operator=(const UnkRegistry_8010A458 &);
};
CHECK_SIZEOF(UnkRegistry_8010A458, 0x10);

extern "C" UnkRegistry_8010A458 *fn_8010A458();

#endif
