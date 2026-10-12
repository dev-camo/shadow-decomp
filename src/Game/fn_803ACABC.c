#include <string.h>

/* MSL strncpy. */
char *strncpy(char *destination, const char *source, unsigned int count) {
    const unsigned char *source_ptr = (const unsigned char *)source - 1;
    unsigned char *destination_ptr = (unsigned char *)destination - 1;

    count++;
    while (--count != 0) {
        unsigned int byte = *++source_ptr;

        *++destination_ptr = byte;
        if (byte == 0) {
            while (--count != 0) {
                *++destination_ptr = 0;
            }
            return destination;
        }
    }
    return destination;
}
