#include <string.h>

/* MSL strncpy. */
char *strncpy(char *destination, const char *source, unsigned int count) {
    /* Target-word addresses preserve the preincrement byte access idiom. */
    unsigned int source_address = (unsigned int)source - 1;
    unsigned int destination_address = (unsigned int)destination - 1;
    count++;
    while (--count != 0) {
        unsigned int byte = *(const unsigned char *)++source_address;
        *(unsigned char *)++destination_address = (unsigned char)byte;
        if (byte == 0) {
            while (--count != 0) {
                *(unsigned char *)++destination_address = 0;
            }
            return destination;
        }
    }
    return destination;
}
