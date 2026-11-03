/* Four-word state generator (behaviour inferred; original name and filename unknown). */
#include "Game/fn_8040CB58.h"

Unk8040CB2C::Unk8040CB2C(unsigned int value) {
    m_words[0] = value;
    m_words[1] = value ^ 0x3CC19185;
    m_words[2] = value ^ 0x01C9C2D0;
    m_words[3] = value ^ 0x2E43C13E;
}

float Unk8040CB2C::fn_8040CB58() {
    unsigned int next = m_words[1];
    next <<= 1;
    if (m_words[0] & 0x80000000) {
        ++next;
    }
    next <<= 1;
    if (m_words[0] & 0x40000000) {
        ++next;
    }

    m_words[3] <<= 1;
    if (m_words[2] & 0x80000000) {
        ++m_words[3];
    }
    next ^= m_words[3];

    m_words[3] = m_words[2];
    m_words[2] = m_words[1];
    m_words[1] = m_words[0];
    m_words[0] = next;
    return (float)(next >> 1) * (1.0f / 2147483648.0f);
}
