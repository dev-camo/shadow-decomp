/* Bit-flags methods (names inferred); original filename unknown. */
#include "Game/UnkBitFlags_80014E04.h"

bool UnkBitFlags_80014E04::Test(int bit) const {
    return (m_bits & (1U << bit)) != 0;
}

void UnkBitFlags_80014E04::Set(int bit) {
    m_bits |= 1U << bit;
}

void UnkBitFlags_80014E04::Clear() {
    m_bits = 0;
}

UnkBitFlags_80014E04::UnkBitFlags_80014E04() : m_bits(0) {}
