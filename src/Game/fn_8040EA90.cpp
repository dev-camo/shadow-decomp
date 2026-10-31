/* SonicUSA::System::Random (class name from RTTI): a Mersenne Twister (MT19937) random number
 * generator, the global generator that the game draws from, and the free functions that read it.
 * The original file name is unknown. Names marked "(name inferred)" are not original.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include <libc/math.h>

#include <dolphin/types.h>
#include "Game/fn_8040_random.h"
#include "layout_check.h"

namespace SonicUSA {
namespace System {

class Random {
  public:
    Random(u32 seed);
    virtual ~Random();

    /* Returns the next float from 0 up to but excluding 1 (name inferred). */
    float NextFloat();
    /* Returns the next float from 0 up to and including 1 (name inferred). */
    float NextFloatInclusive();
    /* Returns the next float between min and max (name inferred). */
    float NextFloat(float min, float max);
    /* Returns a random integer from 0 to count - 1 (name inferred). */
    u32 NextInt(u32 count);

  private:
    enum {
        STATE_SIZE = 624,
        STATE_SHIFT = 397,
    };

    /* Returns the next 32 random bits, regenerating the whole state when it is used up. */
    inline u32 NextBits();

    /* 0x04 */ u32 m_state[STATE_SIZE];
    /* 0x9C4 */ u32 m_index;
};
CHECK_SIZEOF(Random, 0x9C8);

/* The generator's twist matrix: the lowest bit of the mixed state words selects whether to xor in
 * the matrix constant (name inferred). It is not const: the original keeps it in .data. */
static u32 s_twistMatrix[2] = {0, 0x9908B0DF};

} // namespace System
} // namespace SonicUSA

using SonicUSA::System::Random;

u32 Random::NextBits() {
    u32 bits;
    u32 kk;
    if (m_index >= STATE_SIZE) {
        for (kk = 0; kk < STATE_SIZE - STATE_SHIFT; kk++) {
            bits = (m_state[kk] & 0x80000000) | (m_state[kk + 1] & 0x7FFFFFFF);
            m_state[kk] = m_state[kk + STATE_SHIFT] ^ (bits >> 1) ^ s_twistMatrix[bits & 1];
        }
        for (; kk < STATE_SIZE - 1; kk++) {
            bits = (m_state[kk] & 0x80000000) | (m_state[kk + 1] & 0x7FFFFFFF);
            m_state[kk] =
                m_state[kk + (STATE_SHIFT - STATE_SIZE)] ^ (bits >> 1) ^ s_twistMatrix[bits & 1];
        }
        bits = (m_state[STATE_SIZE - 1] & 0x80000000) | (m_state[0] & 0x7FFFFFFF);
        m_state[STATE_SIZE - 1] = m_state[STATE_SHIFT - 1] ^ (bits >> 1) ^ s_twistMatrix[bits & 1];
        m_index = 0;
    }

    bits = m_state[m_index++];
    bits ^= bits >> 11;
    bits ^= (bits << 7) & 0x9D2C5680;
    bits ^= (bits << 15) & 0xEFC60000;
    bits ^= bits >> 18;
    return bits;
}

/* The generator that the free functions below draw from (name inferred). */
static Random s_random(0xBADED9A7);

Random::Random(u32 seed) {
    m_state[0] = seed;
    for (m_index = 1; m_index < STATE_SIZE; m_index++) {
        m_state[m_index] =
            1812433253 * (m_state[m_index - 1] ^ (m_state[m_index - 1] >> 30)) + m_index;
    }
}

Random::~Random() {}

float fn_8040EE90(void) {
    return s_random.NextFloat();
}

float Random::NextFloatInclusive() {
    return NextBits() * (1.0 / 4294967295.0);
}

float fn_8040ECF8(void) {
    return s_random.NextFloatInclusive();
}

float Random::NextFloat(float min, float max) {
    return min + (max - min) * NextFloat();
}

float fn_8040EC80(float min, float max) {
    return s_random.NextFloat(min, max);
}

float Random::NextFloat() {
    return NextBits() * (1.0 / 4294967296.0);
}

u32 Random::NextInt(u32 count) {
    return static_cast<u32>(floorf(count * NextFloat()));
}

u32 fn_8040EA90(u32 count) {
    return s_random.NextInt(count);
}
