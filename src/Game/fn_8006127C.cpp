/* Original filename unknown. */
#include "Game/UnkBitFlags_80014E04.h"

void UnkBitFlags_80014E04::Reset(int bit) {
    m_bits &= ~(1u << bit);
}
