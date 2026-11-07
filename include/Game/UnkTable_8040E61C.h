#ifndef GAME_UNKTABLE_8040E61C_H
#define GAME_UNKTABLE_8040E61C_H

#include <dolphin/types.h>
#include "layout_check.h"

/* Placeholders for the arrays at 0x18 to 0x24 (inferred). Their element types are unknown, so they
 * stay incomplete. */
struct UnkTableArray18_8040E61C;
struct UnkTableArray1C_8040E61C;
struct UnkTableArray20_8040E61C;
struct UnkTableArray24_8040E61C;

/* A 0x28-byte object built from a loaded file (placeholder name; the original class is unknown).
 * The constructor (0x8040E61C, USA (GUPE8P)) clears the byte at 0x0, reads the file's header,
 * byte-swapping its words unless the byte at 0x17 of the file is 'B', and points the five members
 * at 0x14 to 0x24 into the file from 0x20 on. The destructor (0x8040E57C) frees those five arrays
 * with operator delete[] only when the byte at 0x0 is set (inferred: the table owns them), so a
 * table built from a file borrows the file: the buffer must outlive the table and every entry
 * read through it. This is a view: the members between 0x0 and 0x14 are left as bytes. */
class UnkTable_8040E61C {
  public:
    UnkTable_8040E61C(void *data);
    ~UnkTable_8040E61C();

    /* 0x00 */ u8 field_0x0; // Set when the table owns the arrays (inferred); the constructor
                             // from a file clears it.
    /* 0x01 */ u8 field_0x1[0x14 - 0x1];
    /* 0x14 */ void *field_0x14; // The first array; its element type depends on the file.
    /* 0x18 */ UnkTableArray18_8040E61C *field_0x18;
    /* 0x1C */ UnkTableArray1C_8040E61C *field_0x1C;
    /* 0x20 */ UnkTableArray20_8040E61C *field_0x20;
    /* 0x24 */ UnkTableArray24_8040E61C *field_0x24;

  private:
    /* Copying has not been recovered, and a member-wise copy of an owning table would free its
     * arrays twice (declared only, as in C++03). */
    UnkTable_8040E61C(const UnkTable_8040E61C &);
    UnkTable_8040E61C &operator=(const UnkTable_8040E61C &);
};
CHECK_SIZEOF(UnkTable_8040E61C, 0x28);
CHECK_OFFSETOF(UnkTable_8040E61C, field_0x0, 0x0);
CHECK_OFFSETOF(UnkTable_8040E61C, field_0x14, 0x14);
CHECK_OFFSETOF(UnkTable_8040E61C, field_0x18, 0x18);
CHECK_OFFSETOF(UnkTable_8040E61C, field_0x1C, 0x1C);
CHECK_OFFSETOF(UnkTable_8040E61C, field_0x20, 0x20);
CHECK_OFFSETOF(UnkTable_8040E61C, field_0x24, 0x24);

#endif
