/* Four-word state generator (behaviour inferred; original name and filename unknown). */
#include "Game/fn_8040CB58.h"

extern "C" float fn_8040CB58(unsigned int *state) {
    unsigned int next = state[1];
    next <<= 1;
    if (state[0] & 0x80000000) {
        ++next;
    }
    next <<= 1;
    if (state[0] & 0x40000000) {
        ++next;
    }

    state[3] <<= 1;
    if (state[2] & 0x80000000) {
        ++state[3];
    }
    next ^= state[3];

    state[3] = state[2];
    state[2] = state[1];
    state[1] = state[0];
    state[0] = next;
    return (float)(next >> 1) * (1.0f / 2147483648.0f);
}
