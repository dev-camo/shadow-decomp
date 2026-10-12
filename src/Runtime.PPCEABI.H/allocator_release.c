/* MSL runtime pool release (alloc.c __pool_free). */
#include "alloc_internal.h"

/* Not yet decompiled (static in MSL alloc.c). */
extern void deallocate_from_fixed_pools(__mem_pool_obj *pool_obj, void *ptr, unsigned long size);
extern void deallocate_from_var_pools(__mem_pool_obj *pool_obj, void *ptr);

void __pool_free(__mem_pool *pool, void *ptr) {
    __mem_pool_obj *pool_obj;
    unsigned long size;

    if (ptr == 0) {
        return;
    }

    pool_obj = (__mem_pool_obj *)pool;
    size = __msize_inline(ptr);

    if (size <= 68) {
        deallocate_from_fixed_pools(pool_obj, ptr, size);
    } else {
        deallocate_from_var_pools(pool_obj, ptr);
    }
}
