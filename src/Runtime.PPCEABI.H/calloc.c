/* MSL runtime malloc-pool entry points (alloc.c). */

#if defined(VERSION_GUPJ8P)
#define ALLOCATOR_STATE lbl_805A5860
#define ALLOCATOR_INITIALIZED lbl_805F18F8
#elif defined(VERSION_GUPP8P)
#define ALLOCATOR_STATE lbl_805A6300
#define ALLOCATOR_INITIALIZED lbl_805F2390
#else
#define ALLOCATOR_STATE lbl_805A5240
#define ALLOCATOR_INITIALIZED lbl_805F12E8
#endif

/* The original BSS object is 0x38 bytes; its lazy initializer clears 0x34. */
extern unsigned int ALLOCATOR_STATE[14];
extern unsigned char ALLOCATOR_INITIALIZED;

extern void __begin_critical_region(int region);
extern void __end_critical_region(int region);
extern void *memset(void *destination, int value, unsigned int size);
extern void *__pool_alloc_clear(void *pool, unsigned int size);
extern void *__pool_realloc(void *pool, void *ptr, unsigned int size);
extern void __pool_free(void *pool, void *ptr);
extern void *__pool_alloc(void *pool, unsigned int size);

void *calloc(unsigned int count, unsigned int size) {
    void *ptr;

    __begin_critical_region(1);
    if (ALLOCATOR_INITIALIZED == 0) {
        memset(ALLOCATOR_STATE, 0, 0x34);
        ALLOCATOR_INITIALIZED = 1;
    }

    ptr = __pool_alloc_clear(ALLOCATOR_STATE, size * count);
    __end_critical_region(1);
    return ptr;
}

void *realloc(void *ptr, unsigned int size) {
    void *result;

    __begin_critical_region(1);
    if (ALLOCATOR_INITIALIZED == 0) {
        memset(ALLOCATOR_STATE, 0, 0x34);
        ALLOCATOR_INITIALIZED = 1;
    }

    result = __pool_realloc(ALLOCATOR_STATE, ptr, size);
    __end_critical_region(1);
    return result;
}

void free(void *ptr) {
    __begin_critical_region(1);
    if (ALLOCATOR_INITIALIZED == 0) {
        memset(ALLOCATOR_STATE, 0, 0x34);
        ALLOCATOR_INITIALIZED = 1;
    }

    __pool_free(ALLOCATOR_STATE, ptr);
    __end_critical_region(1);
}

void *malloc(unsigned int size) {
    void *ptr;

    __begin_critical_region(1);
    if (ALLOCATOR_INITIALIZED == 0) {
        memset(ALLOCATOR_STATE, 0, 0x34);
        ALLOCATOR_INITIALIZED = 1;
    }

    ptr = __pool_alloc(ALLOCATOR_STATE, size);
    __end_critical_region(1);
    return ptr;
}

void *__pool_alloc_clear(void *pool, unsigned int size) {
    void *ptr;

    ptr = __pool_alloc(pool, size);
    if (ptr != 0) {
        memset(ptr, 0, size);
    }
    return ptr;
}
