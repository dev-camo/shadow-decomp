/* Byte reversal of 16-bit and 32-bit values at 0x804210E4-0x804211CC, in the library code at
 * 0x80410630-0x8042166C (the original file name is unknown; every name here is inferred from the
 * code). Definitions are in address order, with automatic inlining one level deep (see
 * configure.py).
 *
 * Not matching yet: see the notes at the unit in configure.py. */
#include "Game/fn_8042_name_registry.h"

/* Reverses the two bytes of a 16-bit value in place. */
static inline void ReverseBytes16(unsigned short *value) {
    unsigned char *bytes = (unsigned char *)value;
    unsigned char first;

    if (bytes != 0) {
        first = bytes[0];
        bytes[0] = bytes[1];
        bytes[1] = first;
    }
}

void fn_804210E4(void *destination, const void *source, int count) {
    unsigned short *out = destination;
    const unsigned short *in = source;
    unsigned short halves[2];
    int i;

    if (source == 0 || destination == 0) {
        return;
    }

    for (i = 0; i < count; i++) {
        halves[0] = in[0];
        halves[1] = in[1];
        ReverseBytes16(&halves[0]);
        ReverseBytes16(&halves[1]);
        out[0] = halves[1];
        out[1] = halves[0];
        in += 2;
        out += 2;
    }
}

void fn_8042118C(void *destination, const void *source, int count) {
    unsigned char *out = destination;
    const unsigned char *in = source;
    int i;

    if (source == 0 || destination == 0) {
        return;
    }

    for (i = 0; i < count; i++) {
        unsigned char first = in[0];
        unsigned char second = in[1];

        out[0] = second;
        out[1] = first;
        in += 2;
        out += 2;
    }
}
