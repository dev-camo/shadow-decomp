/* MSL runtime pool release (alloc.c __pool_free). */

extern void deallocate_from_fixed_pools(void *pool, void *ptr, unsigned int size);
extern void deallocate_from_var_pools(void *pool, void *ptr);

void __pool_free(void *pool, void *ptr) {
    void *block_header;
    unsigned int block_size;

    if (ptr != 0) {
        block_header = *(void **)((unsigned char *)ptr - 4);
        if (((unsigned int)block_header & 1) == 0) {
            block_size = *(unsigned int *)((unsigned char *)block_header + 8);
        } else {
            block_size = *(unsigned int *)((unsigned char *)ptr - 8) & ~7;
            block_size = block_size - 8;
        }

        if (block_size <= 0x44) {
            deallocate_from_fixed_pools(pool, ptr, block_size);
        } else {
            deallocate_from_var_pools(pool, ptr);
        }
    }
}
