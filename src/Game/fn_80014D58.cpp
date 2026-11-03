#include "Game/UnkString_8004F888.h"
UnkString_8004F888::UnkString_8004F888(const std::allocator<char> &allocator) : field_0x0(0) {
    fn_80014DA0(&field_0x4, allocator);
    m_chars = 0;
    /* The native constructor repeats the helper's capacity reset. */
    field_0x4 = 0;
}
