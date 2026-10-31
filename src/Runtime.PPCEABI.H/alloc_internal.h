#ifndef RUNTIME_ALLOC_INTERNAL_H
#define RUNTIME_ALLOC_INTERNAL_H
#include "layout_check.h"

/* Types and helpers of MSL alloc.c, shared by the units this project splits it into (calloc.c,
 * allocator_core.c, allocator_release.c). Layouts and macro names follow MSL's alloc.c; the layouts are
 * checked with CHECK_SIZEOF against the game's allocations. */

typedef struct Block {
    struct Block *prev;     /* 0x00 */
    struct Block *next;     /* 0x04 */
    unsigned long max_size; /* 0x08 */
    unsigned long size;     /* 0x0C */
} Block;
CHECK_SIZEOF(Block, 0x10);

typedef struct SubBlock {
    unsigned long size;    /* 0x00 */
    Block *block;          /* 0x04 */
    struct SubBlock *prev; /* 0x08 */
    struct SubBlock *next; /* 0x0C */
} SubBlock;
CHECK_SIZEOF(SubBlock, 0x10);

struct FixSubBlock;

typedef struct FixBlock {
    struct FixBlock *prev_;     /* 0x00 */
    struct FixBlock *next_;     /* 0x04 */
    unsigned long client_size_; /* 0x08 */
    struct FixSubBlock *start_; /* 0x0C */
    unsigned long n_allocated_; /* 0x10 */
} FixBlock;
CHECK_SIZEOF(FixBlock, 0x14);

typedef struct FixSubBlock {
    FixBlock *block_;          /* 0x00 */
    struct FixSubBlock *next_; /* 0x04 */
} FixSubBlock;
CHECK_SIZEOF(FixSubBlock, 0x8);

typedef struct FixStart {
    FixBlock *tail_; /* 0x00 */
    FixBlock *head_; /* 0x04 */
} FixStart;
CHECK_SIZEOF(FixStart, 0x8);

typedef struct __mem_pool_obj {
    Block *start_;         /* 0x00 */
    FixStart fix_start[6]; /* 0x04 */
} __mem_pool_obj;
CHECK_SIZEOF(__mem_pool_obj, 0x34);

typedef struct __mem_pool {
    void *reserved[14]; /* 0x00 */
} __mem_pool;
CHECK_SIZEOF(__mem_pool, 0x38);

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
