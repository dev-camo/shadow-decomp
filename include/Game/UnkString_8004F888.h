#ifndef GAME_UNKSTRING_8004F888_H
#define GAME_UNKSTRING_8004F888_H

#include <dolphin/types.h>
#include <memory>
#include "layout_check.h"

/* Native "basic_string::length error" and "string constructor: n > max_size" diagnostics
 * identify this as basic_string. Keep the placeholder until its template arguments and complete
 * type are recovered. The length and capacity words are inferred from string resizing. */
class UnkString_8004F888 {
  public:
    explicit UnkString_8004F888(const std::allocator<char> &allocator = std::allocator<char>());
    UnkString_8004F888(const char *chars,
                       const std::allocator<char> &allocator = std::allocator<char>());
    ~UnkString_8004F888();

    /* 0x0 */ u32 field_0x0; // Length (inferred).
    /* 0x4 */ u32 field_0x4; // Capacity (inferred).
    /* 0x8 */ char *m_chars;
};
CHECK_SIZEOF(UnkString_8004F888, 0xC);

extern "C" {
/* Placeholder for the observed helper ABI: both callers ignore its return, and the native
 * helper does not read the allocator argument. */
void fn_80014DA0(u32 *capacity, const std::allocator<char> &allocator);
/* Accessors of the string: its characters (a null pointer reads as the empty string), a comparison
 * with text that returns 0 when they are equal (behaviour inferred), and assigning the characters
 * of text to the string. */
const char *fn_80014E50(const UnkString_8004F888 *string);
int fn_8003C864(const UnkString_8004F888 *string, const char *text);
void fn_80014CF4(UnkString_8004F888 *string, const char *text);
}

#endif
