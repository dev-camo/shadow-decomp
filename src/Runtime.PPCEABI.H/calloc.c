/* MSL runtime malloc-pool entry points (alloc.c). */
#include <cstring>

#include "alloc_internal.h"
#include "critical_regions.h"

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

/* The malloc pool and its "initialised" flag (MSL get_malloc_pool's protopool and init). Their
 * storage is not split out yet; the lazy initialiser clears the __mem_pool_obj part. */
extern __mem_pool ALLOCATOR_STATE;
extern unsigned char ALLOCATOR_INITIALIZED;

/* Returns the malloc pool, clearing it on first use (MSL's get_malloc_pool). */
static inline __mem_pool *get_malloc_pool(void) {
    if (ALLOCATOR_INITIALIZED == 0) {
        memset(&ALLOCATOR_STATE, 0, sizeof(__mem_pool_obj));
        ALLOCATOR_INITIALIZED = 1;
    }

    return &ALLOCATOR_STATE;
}

void *calloc(unsigned long count, unsigned long size) {
    void *ptr;

    __begin_critical_region(malloc_pool_access);
    ptr = __pool_alloc_clear(get_malloc_pool(), size * count);
    __end_critical_region(malloc_pool_access);
    return ptr;
}

void *realloc(void *ptr, unsigned long size) {
    void *result;

    __begin_critical_region(malloc_pool_access);
    result = __pool_realloc(get_malloc_pool(), ptr, size);
    __end_critical_region(malloc_pool_access);
    return result;
}

void free(void *ptr) {
    __begin_critical_region(malloc_pool_access);
    __pool_free(get_malloc_pool(), ptr);
    __end_critical_region(malloc_pool_access);
}

void *malloc(unsigned long size) {
    void *ptr;

    __begin_critical_region(malloc_pool_access);
    ptr = __pool_alloc(get_malloc_pool(), size);
    __end_critical_region(malloc_pool_access);
    return ptr;
}

void *__pool_alloc_clear(__mem_pool *pool, unsigned long size) {
    void *ptr;

    ptr = __pool_alloc(pool, size);
    if (ptr != 0) {
        memset(ptr, 0, size);
    }
    return ptr;
}
