#include "Game/UnkString_8004F888.h"
UnkString_8004F888::~UnkString_8004F888() {
    if (m_chars != 0) {
        ::operator delete(m_chars);
    }
}
