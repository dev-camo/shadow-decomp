#include <string.h>

/* MSL strncmp. */
int strncmp(const char *left, const char *right, unsigned int count) {
    /* Modular target addresses preserve the preincrement load idiom. */
    unsigned int left_address = (unsigned int)left - 1;
    unsigned int right_address = (unsigned int)right - 1;
    unsigned int left_byte;
    unsigned int right_byte;
    unsigned int remaining = count + 1;
    while (--remaining != 0) {
        left_byte = *(const unsigned char *)++left_address;
        right_byte = *(const unsigned char *)++right_address;
        if (left_byte != right_byte) {
            return (int)left_byte - (int)right_byte;
        }
        if (left_byte == 0) {
            break;
        }
    }
    return 0;
}
