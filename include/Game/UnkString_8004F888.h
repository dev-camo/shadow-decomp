#ifndef GAME_UNKSTRING_8004F888_H
#define GAME_UNKSTRING_8004F888_H

#include <dolphin/types.h>
#include "layout_check.h"
#include <memory>

/* A name as the original passes it around: a string object that fn_8004F888 builds from a C string
 * and an allocator (the message "string constructor: n > max_size" suggests the library's
 * basic_string; only the allocator at 0x4 and the pointer to the characters at 0x8 are mapped).
 * The destructor is fn_80014EC0. */
class UnkString_8004F888 {
  public:
    UnkString_8004F888(const char *chars,
                       const std::allocator<char> &allocator = std::allocator<char>());
    ~UnkString_8004F888();

    /* 0x0 */ u32 field_0x0;
    /* 0x4 */ std::allocator<char> m_allocator;
    /* 0x8 */ char *m_chars;
};
CHECK_SIZEOF(UnkString_8004F888, 0xC);

extern "C" {
/* Accessors of the string: its characters (a null pointer reads as the empty string), a comparison
 * with text that returns 0 when they are equal (behaviour inferred), and assigning the characters
 * of text to the string. */
const char *fn_80014E50(const UnkString_8004F888 *string);
int fn_8003C864(const UnkString_8004F888 *string, const char *text);
void fn_80014CF4(UnkString_8004F888 *string, const char *text);
}

#endif
