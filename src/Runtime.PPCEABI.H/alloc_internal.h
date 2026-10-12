#ifndef RUNTIME_ALLOC_INTERNAL_H
#define RUNTIME_ALLOC_INTERNAL_H

/* Types and helpers of MSL alloc.c, shared by the units this project splits it into (calloc.c,
 * allocator_core.c, allocator_release.c). Layouts and macro names follow the MSL alloc.c in the
 * Twilight Princess decompilation. */

typedef struct Block {
    struct Block *prev;
    struct Block *next;
    unsigned long max_size;
    unsigned long size;
} Block;

typedef struct SubBlock {
    unsigned long size;
    Block *block;
    struct SubBlock *prev;
    struct SubBlock *next;
} SubBlock;

struct FixSubBlock;

typedef struct FixBlock {
    struct FixBlock *prev_;
    struct FixBlock *next_;
    unsigned long client_size_;
    struct FixSubBlock *start_;
    unsigned long n_allocated_;
} FixBlock;

typedef struct FixSubBlock {
    FixBlock *block_;
    struct FixSubBlock *next_;
} FixSubBlock;

typedef struct FixStart {
    FixBlock *tail_;
    FixBlock *head_;
} FixStart;

typedef struct __mem_pool_obj {
    Block *start_;
    FixStart fix_start[6];
} __mem_pool_obj;

typedef struct __mem_pool {
    void *reserved[14];
} __mem_pool;

/* Every allocation is preceded by a header word: a fixed-size block stores its FixBlock pointer
 * (bit 0 clear), a variable-size block its size with flag bits (bit 0 set). */
#define SubBlock_size(ths) ((ths)->size & 0xFFFFFFF8)
#define SubBlock_from_pointer(ptr) ((SubBlock *)((char *)(ptr) - 8))
#define FixSubBlock_from_pointer(ptr) ((FixSubBlock *)((char *)(ptr) - 4))
#define FixBlock_client_size(ths) ((ths)->client_size_)
#define FixSubBlock_size(ths) (FixBlock_client_size((ths)->block_))
#define classify(ptr) (*(unsigned long *)((char *)(ptr) - sizeof(unsigned long)) & 1)

/* Usable size of an allocation. */
#define __msize_inline(ptr)                                                                        \
    (!classify(ptr) ? FixSubBlock_size(FixSubBlock_from_pointer(ptr))                              \
                    : SubBlock_size(SubBlock_from_pointer(ptr)) - 8)

void *__pool_alloc(__mem_pool *pool, unsigned long size);
void *__pool_alloc_clear(__mem_pool *pool, unsigned long size);
void *__pool_realloc(__mem_pool *pool, void *ptr, unsigned long size);
void __pool_free(__mem_pool *pool, void *ptr);

#endif
