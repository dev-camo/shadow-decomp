/* Runtime allocation operators. Zero-size allocations request four bytes. */
#include <new>
#include <alloc.h>

#if defined(VERSION_GUPJ8P)
#define NEW_HANDLER lbl_805F18E0
#elif defined(VERSION_GUPP8P)
#define NEW_HANDLER lbl_805F2378
#else
#define NEW_HANDLER lbl_805F12D0
#endif

/* The allocation-failure callback has no recovered symbol name. */
void (*NEW_HANDLER)();

void *operator new(unsigned long size) {
    void *ptr;
    if (size == 0) {
        size = 4;
    }
    for (;;) {
        ptr = malloc(size);
        if (ptr != 0) {
            break;
        }
        if (NEW_HANDLER != 0) {
            NEW_HANDLER();
        } else {
            throw std::bad_alloc();
        }
    }
    return ptr;
}

void operator delete(void *ptr) throw() {
    if (ptr != 0) {
        free(ptr);
    }
}

void *operator new[](unsigned long size) {
    return operator new(size);
}

void operator delete[](void *ptr) throw() {
    operator delete(ptr);
}
