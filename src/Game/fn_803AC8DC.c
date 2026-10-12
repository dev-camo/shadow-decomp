#include <string.h>

/* MSL strncmp. */
int strncmp(const char *left, const char *right, unsigned int count) {
    const unsigned char *left_byte_ptr = (const unsigned char *)left - 1;
    const unsigned char *right_byte_ptr = (const unsigned char *)right - 1;
    unsigned int left_byte;
    unsigned int right_byte;
    unsigned int remaining = count + 1;

    while (--remaining != 0) {
        left_byte = *++left_byte_ptr;
        right_byte = *++right_byte_ptr;
        if (left_byte != right_byte) {
            return (int)left_byte - (int)right_byte;
        }
        if (left_byte == 0) {
            break;
        }
    }
    return 0;
}
