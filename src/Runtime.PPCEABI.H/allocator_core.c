/* MSL runtime pool resize (alloc.c __pool_realloc); nonmatching research. */
#include <cstring>

#include "alloc_internal.h"

#define BLOCK_SIZE_MASK 0xFFFFFFF8
#define BLOCK_FLAG_2 0x2
#define BLOCK_FLAG_4 0x4
#define MINIMUM_BLOCK_SIZE 0x50
#define MAXIMUM_ALLOCATOR_REQUEST 0xFFFFFFCF

/*
 * Block and SubBlock are the MSL alloc.c types (alloc_internal.h). A SubBlock's size word keeps
 * flag bits 0x2 (allocated) and 0x4 (previous block allocated) below the size; its block pointer
 * has bit 0 set for variable-size blocks. Each Block keeps its free-list head in its last word
 * (FreeListOffset), and a free SubBlock ends with a copy of its size.
 *
 * Free-list links are circular and doubly linked: an empty list starts with both links pointing at
 * its sole node. Removing that singleton first advances the head to next (itself), then the second
 * head check clears it. Keep both checks in UnlinkFreeBlock.
 */
inline static unsigned int BlockSize(SubBlock *block) {
    return block->size & BLOCK_SIZE_MASK;
}

inline static Block *BlockArea(SubBlock *block) {
    return (Block *)((unsigned int)block->block & (~1));
}

inline static unsigned int FreeListOffset(Block *area) {
    return (area->size & BLOCK_SIZE_MASK) - 4;
}

/* The head is the block's last word, at the variable offset FreeListOffset() returns (MSL's
 * Block_start). It is read again for the second check, which clears the head when block was the
 * list's only node (see above). */
inline static void UnlinkFreeBlock(Block *area, unsigned int offset, SubBlock *block) {
    if (((SubBlock **)area)[offset / sizeof(SubBlock *)] == block) {
        ((SubBlock **)area)[offset / sizeof(SubBlock *)] = block->next;
    }
    if (((SubBlock **)area)[offset / sizeof(SubBlock *)] == block) {
        ((SubBlock **)area)[offset / sizeof(SubBlock *)] = 0;
    }
    block->next->prev = block->prev;
    block->prev->next = block->next;
}

/*
 * Resize path map: null pointers allocate; size zero releases and returns null. Growth of an
 * area-owned block first tries to merge its following free block, then splits a usable remainder.
 * If in-place growth cannot satisfy the request, the fallback allocates, copies the old payload,
 * and releases the old pointer. Shrinkage splits only when the remainder reaches the minimum size;
 * smaller changes keep the existing block.
 *
 * The target has distinct grow and shrink header-update sequences (including 0x803A3068..0x3088
 * and 0x803A33A8..0x33C4). The source keeps those paths separate to follow the observed code, but
 * the unit is still NonMatching, so whether the repeated split/reinsert blocks reflect required
 * original source expansion or merely the current matching reconstruction remains provisional.
 */
void *__pool_realloc(__mem_pool *pool, void *ptr, unsigned long size) {
    SubBlock *block;
    SubBlock *following;
    unsigned int free_head_offset;
    Block *area;
    unsigned int owner;
    SubBlock *coalesced_block;
    unsigned int old_size;
    unsigned int block_size;
    unsigned int required_size;
    unsigned int combined_size;
    void *new_ptr;
    if (ptr == 0) {
        return __pool_alloc(pool, size);
    }
    if (size == 0) {
        __pool_free(pool, ptr);
        return 0;
    }
    owner = *(unsigned int *)((unsigned char *)ptr - 4); /* the header word classify() tests */
    if ((owner & 1) == 0) {
        old_size = FixBlock_client_size((FixBlock *)owner);
    } else {
        block = SubBlock_from_pointer(ptr);
        old_size = BlockSize(block) - 8;
    }
    if (size > old_size) {
        if ((owner & 1) != 0) {
            if (size > MAXIMUM_ALLOCATOR_REQUEST) {
                return 0;
            }
            required_size = (size + 0xF) & BLOCK_SIZE_MASK;
            if (required_size < MINIMUM_BLOCK_SIZE) {
                required_size = MINIMUM_BLOCK_SIZE;
            }
            block = SubBlock_from_pointer(ptr);
            block_size = BlockSize(block);
            area = BlockArea(block);
            free_head_offset = FreeListOffset(area);
            following = (SubBlock *)((unsigned char *)block + block_size);
            if ((following->size & BLOCK_FLAG_2) == 0) {
                combined_size = block_size + BlockSize(following);
                block->size &= 7;
                block->size |= combined_size & BLOCK_SIZE_MASK;
                if ((block->size & BLOCK_FLAG_2) == 0) {
                    *(unsigned int *)(((unsigned char *)block + combined_size) - 4) = combined_size;
                }
                if ((block->size & BLOCK_FLAG_2) == 0) {
                    *(unsigned int *)((unsigned char *)block + combined_size) &= ~BLOCK_FLAG_4;
                } else {
                    *(unsigned int *)((unsigned char *)block + combined_size) |= BLOCK_FLAG_4;
                }
                coalesced_block = block;
                UnlinkFreeBlock(area, free_head_offset,
                                (SubBlock *)((unsigned char *)block + block_size));
                combined_size = BlockSize(coalesced_block);
                if (combined_size >= required_size) {
                    if ((combined_size - required_size) >= MINIMUM_BLOCK_SIZE) {
                        {
                            SubBlock *split_block = coalesced_block;
                            unsigned int split_size = required_size;
                            unsigned int split_old_size;
                            unsigned int split_old_flags;
                            int split_is_free;
                            int split_is_allocated;
                            unsigned int split_remaining_size;
                            unsigned int split_owner;
                            unsigned int split_boundary_flags;
                            unsigned int split_boundary_size;
                            SubBlock *split_remaining;
                            SubBlock *split_following;
                            split_old_size = BlockSize(split_block);
                            split_old_flags = split_block->size;
                            split_is_free = !(split_old_flags & BLOCK_FLAG_2);
                            split_is_allocated = !split_is_free;
                            split_owner = ((unsigned int)split_block->block & (~1)) | 1;
                            split_remaining =
                                (SubBlock *)((unsigned char *)split_block + split_size);
                            split_block->block = (Block *)split_owner;
                            split_block->size = split_size;
                            if ((split_old_flags & BLOCK_FLAG_4) != 0) {
                                split_block->size |= BLOCK_FLAG_4;
                            }
                            if (split_is_allocated != 0) {
                                split_block->size |= BLOCK_FLAG_2;
                                /* The target ORs this header before overwriting its size below. */
                                split_remaining->size |= BLOCK_FLAG_4;
                            } else {
                                *(unsigned int *)((unsigned char *)split_remaining - 4) =
                                    split_size;
                            }
                            split_remaining->block = (Block *)split_owner;
                            split_remaining_size = split_old_size - split_size;
                            split_remaining->size = split_remaining_size;
                            if (split_is_allocated != 0) {
                                split_remaining->size |= BLOCK_FLAG_4;
                            }
                            if (split_is_allocated != 0) {
                                split_remaining->size |= BLOCK_FLAG_2;
                                split_following = (SubBlock *)((unsigned char *)split_remaining +
                                                               split_remaining_size);
                                split_following->size |= BLOCK_FLAG_4;
                            } else {
                                split_following = (SubBlock *)((unsigned char *)split_remaining +
                                                               split_remaining_size);
                                *(unsigned int *)((unsigned char *)split_following - 4) =
                                    split_remaining_size;
                            }
                            if (split_is_free != 0) {
                                split_remaining->next = split_block->next;
                                split_remaining->next->prev = split_remaining;
                                split_remaining->prev = split_block;
                                split_block->next = split_remaining;
                            }
                            split_boundary_flags = split_remaining->size;
                            split_boundary_size = split_boundary_flags & BLOCK_SIZE_MASK;
                            split_remaining->size = split_boundary_flags & (~BLOCK_FLAG_2);
                            split_following = (SubBlock *)((unsigned char *)split_remaining +
                                                           split_boundary_size);
                            split_following->size &= ~BLOCK_FLAG_4;
                            *(unsigned int *)((unsigned char *)split_following - 4) =
                                split_boundary_size;
                            {
                                Block *insert_area = BlockArea(split_block);
                                unsigned int insert_head_offset;
                                SubBlock *insert_following;
                                SubBlock *insert_previous;
                                SubBlock *insert_block = split_remaining;
                                unsigned int insert_size;
                                unsigned int insert_previous_size;
                                unsigned int insert_following_size;
                                insert_head_offset = FreeListOffset(insert_area);
                                if (((SubBlock **)
                                         insert_area)[insert_head_offset / (sizeof(SubBlock *))] !=
                                    0) {
                                    insert_block->prev =
                                        ((SubBlock **)
                                             insert_area)[insert_head_offset / (sizeof(SubBlock *))]
                                            ->prev;
                                    insert_block->prev->next = insert_block;
                                    insert_block->next =
                                        ((SubBlock **)insert_area)[insert_head_offset /
                                                                   (sizeof(SubBlock *))];
                                    ((SubBlock **)
                                         insert_area)[insert_head_offset / (sizeof(SubBlock *))]
                                        ->prev = insert_block;
                                    ((SubBlock **)
                                         insert_area)[insert_head_offset / (sizeof(SubBlock *))] =
                                        insert_block;
                                    insert_block = ((SubBlock **)insert_area)[insert_head_offset /
                                                                              (sizeof(SubBlock *))];
                                    /* A free block footer supplies the prior block's size. */
                                    if ((insert_block->size & BLOCK_FLAG_4) == 0) {
                                        insert_previous_size =
                                            *((unsigned int *)((unsigned char *)insert_block - 4));
                                        if ((insert_previous_size & BLOCK_FLAG_2) != 0) {
                                            insert_previous = insert_block;
                                        } else {
                                            insert_previous =
                                                (SubBlock *)((unsigned char *)insert_block -
                                                             insert_previous_size);
                                            insert_previous->size &= 7;
                                            insert_previous->size |=
                                                (insert_previous_size + BlockSize(insert_block)) &
                                                BLOCK_SIZE_MASK;
                                            if ((insert_previous->size & BLOCK_FLAG_2) == 0) {
                                                insert_size =
                                                    insert_previous_size + BlockSize(insert_block);
                                                *(unsigned int *)((((unsigned char *)
                                                                        insert_previous) +
                                                                   insert_size) -
                                                                  4) = insert_size;
                                            }
                                            if (((SubBlock **)insert_area)[insert_head_offset /
                                                                           (sizeof(SubBlock *))] ==
                                                insert_block) {
                                                ((SubBlock **)insert_area)[insert_head_offset /
                                                                           (sizeof(SubBlock *))] =
                                                    insert_block->next;
                                            }
                                            insert_block->next->prev = insert_block->prev;
                                            insert_block->prev->next = insert_block->next;
                                        }
                                    } else {
                                        insert_previous = insert_block;
                                    }
                                    ((SubBlock **)
                                         insert_area)[insert_head_offset / (sizeof(SubBlock *))] =
                                        insert_previous;
                                    insert_block = ((SubBlock **)insert_area)[insert_head_offset /
                                                                              (sizeof(SubBlock *))];
                                    insert_size = BlockSize(insert_block);
                                    insert_following =
                                        (SubBlock *)((unsigned char *)insert_block + insert_size);
                                    if ((((SubBlock *)((unsigned char *)insert_block + insert_size))
                                             ->size &
                                         BLOCK_FLAG_2) == 0) {
                                        insert_following_size = BlockSize(insert_following);
                                        insert_block->size &= 7;
                                        insert_size += insert_following_size;
                                        insert_block->size |= insert_size & BLOCK_SIZE_MASK;
                                        if ((insert_block->size & BLOCK_FLAG_2) == 0) {
                                            *(unsigned int *)(((unsigned char *)insert_block +
                                                               insert_size) -
                                                              4) = insert_size;
                                        }
                                        if ((insert_block->size & BLOCK_FLAG_2) == 0) {
                                            *(unsigned int *)((unsigned char *)insert_block +
                                                              insert_size) &= ~BLOCK_FLAG_4;

                                        } else {
                                            *(unsigned int *)((unsigned char *)insert_block +
                                                              insert_size) |= BLOCK_FLAG_4;
                                        }
                                        if (((SubBlock **)insert_area)[insert_head_offset /
                                                                       (sizeof(SubBlock *))] ==
                                            insert_following) {
                                            ((SubBlock **)insert_area)[insert_head_offset /
                                                                       (sizeof(SubBlock *))] =
                                                insert_following->next;
                                        }
                                        if (((SubBlock **)insert_area)[insert_head_offset /
                                                                       (sizeof(SubBlock *))] ==
                                            insert_following) {
                                            ((SubBlock **)insert_area)[insert_head_offset /
                                                                       (sizeof(SubBlock *))] = 0;
                                        }
                                        insert_following->next->prev = insert_following->prev;
                                        insert_following->prev->next = insert_following->next;
                                    }
                                } else {
                                    /* An empty free list starts as a circular doubly linked
                                     * list. */
                                    ((SubBlock **)
                                         insert_area)[insert_head_offset / (sizeof(SubBlock *))] =
                                        insert_block;
                                    insert_block->prev = insert_block;
                                    insert_block->next = insert_block;
                                }
                                insert_size = BlockSize(
                                    ((SubBlock **)
                                         insert_area)[insert_head_offset / (sizeof(SubBlock *))]);
                                if (insert_area->max_size < insert_size) {
                                    insert_area->max_size = insert_size;
                                }
                            }
                        }
                    }
                    return ptr;
                }
            }
        }
        new_ptr = __pool_alloc(pool, size);
        if (new_ptr == 0) {
            return 0;
        }
        memcpy(new_ptr, ptr, old_size);
        __pool_free(pool, ptr);
        return new_ptr;
    }
    if ((owner & 1) != 0) {
        required_size = (size + 0xF) & BLOCK_SIZE_MASK;
        if (required_size < MINIMUM_BLOCK_SIZE) {
            required_size = MINIMUM_BLOCK_SIZE;
        }
        block = SubBlock_from_pointer(ptr);
        block_size = BlockSize(block);
        if ((block_size - required_size) >= MINIMUM_BLOCK_SIZE) {
            {
                SubBlock *split_block = block;
                unsigned int split_size = required_size;
                unsigned int split_old_size;
                unsigned int split_old_flags;
                int split_is_free;
                int split_is_allocated;
                unsigned int split_remaining_size;
                unsigned int split_owner;
                unsigned int split_boundary_flags;
                unsigned int split_boundary_size;
                SubBlock *split_remaining;
                SubBlock *split_following;
                split_old_size = BlockSize(split_block);
                split_old_flags = split_block->size;
                split_is_free = !(split_old_flags & BLOCK_FLAG_2);
                split_is_allocated = !split_is_free;
                split_owner = ((unsigned int)split_block->block & (~1)) | 1;
                split_remaining = (SubBlock *)((unsigned char *)split_block + split_size);
                split_block->block = (Block *)split_owner;
                split_block->size = split_size;
                if ((split_old_flags & BLOCK_FLAG_4) != 0) {
                    split_block->size |= BLOCK_FLAG_4;
                }
                if (split_is_allocated != 0) {
                    split_block->size |= BLOCK_FLAG_2;
                    /* The target ORs this header before overwriting its size below. */
                    split_remaining->size |= BLOCK_FLAG_4;
                } else {
                    *(unsigned int *)((unsigned char *)split_remaining - 4) = split_size;
                }
                split_remaining->block = (Block *)split_owner;
                split_remaining_size = split_old_size - split_size;
                split_remaining->size = split_remaining_size;
                if (split_is_allocated != 0) {
                    split_remaining->size |= BLOCK_FLAG_4;
                }
                if (split_is_allocated != 0) {
                    split_remaining->size |= BLOCK_FLAG_2;
                    split_following =
                        (SubBlock *)((unsigned char *)split_remaining + split_remaining_size);
                    split_following->size |= BLOCK_FLAG_4;
                } else {
                    split_following =
                        (SubBlock *)((unsigned char *)split_remaining + split_remaining_size);
                    *(unsigned int *)((unsigned char *)split_following - 4) = split_remaining_size;
                }
                if (split_is_free != 0) {
                    split_remaining->next = split_block->next;
                    split_remaining->next->prev = split_remaining;
                    split_remaining->prev = split_block;
                    split_block->next = split_remaining;
                }
                split_boundary_flags = split_remaining->size;
                split_boundary_size = split_boundary_flags & BLOCK_SIZE_MASK;
                split_remaining->size = split_boundary_flags & (~BLOCK_FLAG_2);
                split_following =
                    (SubBlock *)((unsigned char *)split_remaining + split_boundary_size);
                split_following->size &= ~BLOCK_FLAG_4;
                *(unsigned int *)((unsigned char *)split_following - 4) = split_boundary_size;
                {
                    Block *insert_area = BlockArea(split_block);
                    unsigned int insert_head_offset;
                    SubBlock *insert_following;
                    SubBlock *insert_previous;
                    SubBlock *insert_block = split_remaining;
                    unsigned int insert_size;
                    unsigned int insert_previous_size;
                    int insert_following_size;
                    insert_head_offset = FreeListOffset(insert_area);
                    if (((SubBlock **)insert_area)[insert_head_offset / (sizeof(SubBlock *))] !=
                        0) {
                        insert_block->prev =
                            ((SubBlock **)insert_area)[insert_head_offset / (sizeof(SubBlock *))]
                                ->prev;
                        insert_block->prev->next = insert_block;
                        insert_block->next =
                            ((SubBlock **)insert_area)[insert_head_offset / (sizeof(SubBlock *))];
                        ((SubBlock **)insert_area)[insert_head_offset / (sizeof(SubBlock *))]
                            ->prev = insert_block;
                        ((SubBlock **)insert_area)[insert_head_offset / (sizeof(SubBlock *))] =
                            insert_block;
                        insert_block =
                            ((SubBlock **)insert_area)[insert_head_offset / (sizeof(SubBlock *))];
                        /* A free block footer supplies the prior block's size. */
                        if ((insert_block->size & BLOCK_FLAG_4) == 0) {
                            insert_previous_size =
                                *(unsigned int *)((unsigned char *)insert_block - 4);
                            if ((insert_previous_size & BLOCK_FLAG_2) != 0) {
                                insert_previous = insert_block;
                            } else {
                                insert_previous = (SubBlock *)((unsigned char *)insert_block -
                                                               insert_previous_size);
                                insert_previous->size &= 7;
                                insert_previous->size |=
                                    (insert_previous_size + BlockSize(insert_block)) &
                                    BLOCK_SIZE_MASK;
                                if ((insert_previous->size & BLOCK_FLAG_2) == 0) {
                                    insert_size = insert_previous_size + BlockSize(insert_block);
                                    *(unsigned int *)(((unsigned char *)insert_previous +
                                                       insert_size) -
                                                      4) = insert_size;
                                }
                                if (((SubBlock **)
                                         insert_area)[insert_head_offset / (sizeof(SubBlock *))] ==
                                    insert_block) {
                                    ((SubBlock **)
                                         insert_area)[insert_head_offset / (sizeof(SubBlock *))] =
                                        insert_block->next;
                                }
                                insert_block->next->prev = insert_block->prev;
                                insert_block->prev->next = insert_block->next;
                            }
                        } else {
                            insert_previous = insert_block;
                        }
                        ((SubBlock **)insert_area)[insert_head_offset / (sizeof(SubBlock *))] =
                            insert_previous;
                        insert_block =
                            ((SubBlock **)insert_area)[insert_head_offset / (sizeof(SubBlock *))];
                        insert_size = BlockSize(insert_block);
                        insert_following =
                            (SubBlock *)((unsigned char *)insert_block + insert_size);
                        if ((((SubBlock *)((unsigned char *)insert_block + insert_size))->size &
                             BLOCK_FLAG_2) == 0) {
                            insert_following_size = BlockSize(insert_following);
                            insert_block->size &= 7;
                            insert_size += insert_following_size;
                            insert_block->size |= insert_size & BLOCK_SIZE_MASK;
                            if ((insert_block->size & BLOCK_FLAG_2) == 0) {
                                *(unsigned int *)(((unsigned char *)insert_block + insert_size) -
                                                  4) = insert_size;
                            }
                            if ((insert_block->size & BLOCK_FLAG_2) == 0) {
                                *(unsigned int *)((unsigned char *)insert_block + insert_size) &=
                                    ~BLOCK_FLAG_4;

                            } else {
                                *(unsigned int *)((unsigned char *)insert_block + insert_size) |=
                                    BLOCK_FLAG_4;
                            }
                            if (((SubBlock **)
                                     insert_area)[insert_head_offset / (sizeof(SubBlock *))] ==
                                insert_following) {
                                ((SubBlock **)
                                     insert_area)[insert_head_offset / (sizeof(SubBlock *))] =
                                    insert_following->next;
                            }
                            if (((SubBlock **)
                                     insert_area)[insert_head_offset / (sizeof(SubBlock *))] ==
                                insert_following) {
                                ((SubBlock **)
                                     insert_area)[insert_head_offset / (sizeof(SubBlock *))] = 0;
                            }
                            insert_following->next->prev = insert_following->prev;
                            insert_following->prev->next = insert_following->next;
                        }
                    } else {
                        /* An empty free list starts as a circular doubly linked list. */
                        ((SubBlock **)insert_area)[insert_head_offset / (sizeof(SubBlock *))] =
                            insert_block;
                        insert_block->prev = insert_block;
                        insert_block->next = insert_block;
                    }
                    insert_size = BlockSize(
                        ((SubBlock **)insert_area)[insert_head_offset / (sizeof(SubBlock *))]);
                    if (insert_area->max_size < insert_size) {
                        insert_area->max_size = insert_size;
                    }
                }
            }
        }
    }
    return ptr;
}
