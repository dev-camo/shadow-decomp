#include <cstddef>

extern "C" void *__copy(char *destination, char *source, size_t count) {
    if (destination != 0 && count != 0) {
        char *out = destination;
        do {
            *out++ = *source++;
        } while (--count != 0);
    }
    return destination;
}
