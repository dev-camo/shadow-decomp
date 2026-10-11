/* MSL runtime pool resize (alloc.c __pool_realloc); nonmatching research. */

#define BLOCK_SIZE_MASK 0xFFFFFFF8
#define BLOCK_FLAG_2 0x2
#define BLOCK_FLAG_4 0x4
#define MINIMUM_BLOCK_SIZE 0x50
#define MAXIMUM_ALLOCATOR_REQUEST 0xFFFFFFCF

/*
 * Reader map from the observed 32-bit accesses; the names and C types below are not recovered source
 * declarations. AllocatorBlock words at offsets 0 and 4 hold size_flags and tagged_area; the free
 * links occupy offsets 8 and 12 while a block is free. Sizes clear the low three bits, while this
 * unit separately tests bits 0x2 and 0x4. tagged_area clears bit 0 to recover the area pointer.
 * AllocatorArea's observed prefix is two links, largest_free_size, free_list_offset, and
 * active_block_count at offsets 0, 4, 8, 12, and 16. The head slot is addressed from
 * free_list_offset after masking and subtracting four bytes.
 *
 * Free-list links are circular and doubly linked: an empty list starts with both links pointing at
 * its sole node. Removing that singleton first advances the head to next_free (itself), then the
 * second head check clears it. Keep both checks in UnlinkFreeBlock.
 */
typedef struct AllocatorBlock AllocatorBlock;
typedef struct AllocatorArea AllocatorArea;
struct AllocatorBlock {
    unsigned int size_flags;
    unsigned int tagged_area;
    AllocatorBlock *previous_free;
    AllocatorBlock *next_free;
};
struct AllocatorArea {
    AllocatorArea *previous_area;
    AllocatorArea *next_area;
    unsigned int largest_free_size;
    unsigned int free_list_offset;
    unsigned int active_block_count;
};
extern void *__pool_alloc(void *pool, unsigned int size);
extern void __pool_free(void *pool, void *ptr);
extern void *memcpy(void *destination, const void *source, unsigned int size);
inline static unsigned int BlockSize(AllocatorBlock *block) {
    return block->size_flags & BLOCK_SIZE_MASK;
}

inline static AllocatorArea *BlockArea(AllocatorBlock *block) {
    return (AllocatorArea *)(block->tagged_area & (~1));
}

inline static unsigned int FreeListOffset(AllocatorArea *area) {
    return (area->free_list_offset & BLOCK_SIZE_MASK) - 4;
}

/* The target reloads the head between these checks; preserve both updates in order. */
inline static void UnlinkFreeBlock(AllocatorArea *area, unsigned int offset,
                                   AllocatorBlock *block) {
    if (((AllocatorBlock **)area)[offset / (sizeof(AllocatorBlock *))] == block) {
        ((AllocatorBlock **)area)[offset / (sizeof(AllocatorBlock *))] = block->next_free;
    }
    if (((AllocatorBlock **)area)[offset / (sizeof(AllocatorBlock *))] == block) {
        ((AllocatorBlock **)area)[offset / (sizeof(AllocatorBlock *))] = 0;
    }
    block->next_free->previous_free = block->previous_free;
    block->previous_free->next_free = block->next_free;
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
void *__pool_realloc(void *pool, void *ptr, unsigned int size) {
    AllocatorBlock *block;
    AllocatorBlock *following;
    unsigned int free_head_offset;
    AllocatorArea *area;
    unsigned int owner;
    AllocatorBlock *coalesced_block;
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
    owner = *((unsigned int *)(((unsigned char *)ptr) - 4));
    if ((owner & 1) == 0) {
        old_size = *((unsigned int *)(((unsigned char *)owner) + 8));
    } else {
        block = (AllocatorBlock *)(((unsigned char *)ptr) - 8);
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
            block = (AllocatorBlock *)(((unsigned char *)ptr) - 8);
            block_size = BlockSize(block);
            area = BlockArea(block);
            free_head_offset = FreeListOffset(area);
            following = (AllocatorBlock *)(((unsigned char *)block) + block_size);
            if ((following->size_flags & BLOCK_FLAG_2) == 0) {
                combined_size = block_size + BlockSize(following);
                block->size_flags &= 7;
                block->size_flags |= combined_size & BLOCK_SIZE_MASK;
                if ((block->size_flags & BLOCK_FLAG_2) == 0) {
                    *((unsigned int *)((((unsigned char *)block) + combined_size) - 4)) =
                        combined_size;
                }
                if ((block->size_flags & BLOCK_FLAG_2) == 0) {
                    *((unsigned int *)(((unsigned char *)block) + combined_size)) &= ~BLOCK_FLAG_4;
                } else {
                    *((unsigned int *)(((unsigned char *)block) + combined_size)) |= BLOCK_FLAG_4;
                }
                coalesced_block = block;
                UnlinkFreeBlock(area, free_head_offset,
                                (AllocatorBlock *)(((unsigned char *)block) + block_size));
                combined_size = BlockSize(coalesced_block);
                if (combined_size >= required_size) {
                    if ((combined_size - required_size) >= MINIMUM_BLOCK_SIZE) {
                        do {
                            AllocatorBlock *split_block = coalesced_block;
                            unsigned int split_size = required_size;
                            unsigned int split_old_size;
                            unsigned int split_old_flags;
                            int split_is_free;
                            int split_is_allocated;
                            unsigned int split_remaining_size;
                            unsigned int split_owner;
                            unsigned int split_boundary_flags;
                            unsigned int split_boundary_size;
                            AllocatorBlock *split_remaining;
                            AllocatorBlock *split_following;
                            split_old_size = BlockSize(split_block);
                            split_old_flags = split_block->size_flags;
                            split_is_free = !(split_old_flags & BLOCK_FLAG_2);
                            split_is_allocated = !split_is_free;
                            split_owner = (split_block->tagged_area & (~1)) | 1;
                            split_remaining =
                                (AllocatorBlock *)(((unsigned char *)split_block) + split_size);
                            split_block->tagged_area = split_owner;
                            split_block->size_flags = split_size;
                            if ((split_old_flags & BLOCK_FLAG_4) != 0) {
                                split_block->size_flags |= BLOCK_FLAG_4;
                            }
                            if (split_is_allocated != 0) {
                                split_block->size_flags |= BLOCK_FLAG_2;
                                /* The target ORs this header before overwriting its size below. */
                                split_remaining->size_flags |= BLOCK_FLAG_4;
                            } else {
                                *((unsigned int *)(((unsigned char *)split_remaining) - 4)) =
                                    split_size;
                            }
                            split_remaining->tagged_area = split_owner;
                            split_remaining_size = split_old_size - split_size;
                            split_remaining->size_flags = split_remaining_size;
                            if (split_is_allocated != 0) {
                                split_remaining->size_flags |= BLOCK_FLAG_4;
                            }
                            if (split_is_allocated != 0) {
                                split_remaining->size_flags |= BLOCK_FLAG_2;
                                split_following =
                                    (AllocatorBlock *)(((unsigned char *)split_remaining) +
                                                       split_remaining_size);
                                split_following->size_flags |= BLOCK_FLAG_4;
                            } else {
                                split_following =
                                    (AllocatorBlock *)(((unsigned char *)split_remaining) +
                                                       split_remaining_size);
                                *((unsigned int *)(((unsigned char *)split_following) - 4)) =
                                    split_remaining_size;
                            }
                            if (split_is_free != 0) {
                                split_remaining->next_free = split_block->next_free;
                                split_remaining->next_free->previous_free = split_remaining;
                                split_remaining->previous_free = split_block;
                                split_block->next_free = split_remaining;
                            }
                            split_boundary_flags = split_remaining->size_flags;
                            split_boundary_size = split_boundary_flags & BLOCK_SIZE_MASK;
                            split_remaining->size_flags = split_boundary_flags & (~BLOCK_FLAG_2);
                            split_following =
                                (AllocatorBlock *)(((unsigned char *)split_remaining) +
                                                   split_boundary_size);
                            split_following->size_flags &= ~BLOCK_FLAG_4;
                            *((unsigned int *)(((unsigned char *)split_following) - 4)) =
                                split_boundary_size;
                            do {
                                AllocatorArea *insert_area = BlockArea(split_block);
                                unsigned int insert_head_offset;
                                AllocatorBlock *insert_following;
                                AllocatorBlock *insert_previous;
                                AllocatorBlock *insert_block = split_remaining;
                                unsigned int insert_size;
                                unsigned int insert_previous_size;
                                unsigned int insert_following_size;
                                insert_head_offset = FreeListOffset(insert_area);
                                if (((AllocatorBlock **)insert_area)[insert_head_offset /
                                                                     (sizeof(AllocatorBlock *))] !=
                                    0) {
                                    insert_block->previous_free =
                                        ((AllocatorBlock **)insert_area)[insert_head_offset /
                                                                         (sizeof(AllocatorBlock *))]
                                            ->previous_free;
                                    insert_block->previous_free->next_free = insert_block;
                                    insert_block->next_free = ((
                                        AllocatorBlock **)insert_area)[insert_head_offset /
                                                                       (sizeof(AllocatorBlock *))];
                                    ((AllocatorBlock **)insert_area)[insert_head_offset /
                                                                     (sizeof(AllocatorBlock *))]
                                        ->previous_free = insert_block;
                                    ((AllocatorBlock **)insert_area)[insert_head_offset /
                                                                     (sizeof(AllocatorBlock *))] =
                                        insert_block;
                                    insert_block = ((
                                        AllocatorBlock **)insert_area)[insert_head_offset /
                                                                       (sizeof(AllocatorBlock *))];
                                    /* A free block footer supplies the prior block's size. */
                                    if ((insert_block->size_flags & BLOCK_FLAG_4) == 0) {
                                        insert_previous_size = *(
                                            (unsigned int *)(((unsigned char *)insert_block) - 4));
                                        if ((insert_previous_size & BLOCK_FLAG_2) != 0) {
                                            insert_previous = insert_block;
                                        } else {
                                            insert_previous =
                                                (AllocatorBlock *)(((unsigned char *)insert_block) -
                                                                   insert_previous_size);
                                            insert_previous->size_flags &= 7;
                                            insert_previous->size_flags |=
                                                (insert_previous_size + BlockSize(insert_block)) &
                                                BLOCK_SIZE_MASK;
                                            if ((insert_previous->size_flags & BLOCK_FLAG_2) == 0) {
                                                insert_size =
                                                    insert_previous_size + BlockSize(insert_block);
                                                *((unsigned int *)((((unsigned char *)
                                                                         insert_previous) +
                                                                    insert_size) -
                                                                   4)) = insert_size;
                                            }
                                            if (((AllocatorBlock **)
                                                     insert_area)[insert_head_offset /
                                                                  (sizeof(AllocatorBlock *))] ==
                                                insert_block) {
                                                ((AllocatorBlock **)
                                                     insert_area)[insert_head_offset /
                                                                  (sizeof(AllocatorBlock *))] =
                                                    insert_block->next_free;
                                            }
                                            insert_block->next_free->previous_free =
                                                insert_block->previous_free;
                                            insert_block->previous_free->next_free =
                                                insert_block->next_free;
                                        }
                                    } else {
                                        insert_previous = insert_block;
                                    }
                                    ((AllocatorBlock **)insert_area)[insert_head_offset /
                                                                     (sizeof(AllocatorBlock *))] =
                                        insert_previous;
                                    insert_block = ((
                                        AllocatorBlock **)insert_area)[insert_head_offset /
                                                                       (sizeof(AllocatorBlock *))];
                                    insert_size = BlockSize(insert_block);
                                    insert_following =
                                        (AllocatorBlock *)(((unsigned char *)insert_block) +
                                                           insert_size);
                                    if ((((AllocatorBlock *)(((unsigned char *)insert_block) +
                                                             insert_size))
                                             ->size_flags &
                                         BLOCK_FLAG_2) == 0) {
                                        insert_following_size = BlockSize(insert_following);
                                        insert_block->size_flags &= 7;
                                        insert_size += insert_following_size;
                                        insert_block->size_flags |= insert_size & BLOCK_SIZE_MASK;
                                        if ((insert_block->size_flags & BLOCK_FLAG_2) == 0) {
                                            *((unsigned int *)((((unsigned char *)insert_block) +
                                                                insert_size) -
                                                               4)) = insert_size;
                                        }
                                        if ((insert_block->size_flags & BLOCK_FLAG_2) == 0) {
                                            *((unsigned int *)(((unsigned char *)insert_block) +
                                                               insert_size)) &= ~BLOCK_FLAG_4;

                                        } else {
                                            *((unsigned int *)(((unsigned char *)insert_block) +
                                                               insert_size)) |= BLOCK_FLAG_4;
                                        }
                                        if (((AllocatorBlock **)
                                                 insert_area)[insert_head_offset /
                                                              (sizeof(AllocatorBlock *))] ==
                                            insert_following) {
                                            ((AllocatorBlock **)
                                                 insert_area)[insert_head_offset /
                                                              (sizeof(AllocatorBlock *))] =
                                                insert_following->next_free;
                                        }
                                        if (((AllocatorBlock **)
                                                 insert_area)[insert_head_offset /
                                                              (sizeof(AllocatorBlock *))] ==
                                            insert_following) {
                                            ((AllocatorBlock **)
                                                 insert_area)[insert_head_offset /
                                                              (sizeof(AllocatorBlock *))] = 0;
                                        }
                                        insert_following->next_free->previous_free =
                                            insert_following->previous_free;
                                        insert_following->previous_free->next_free =
                                            insert_following->next_free;
                                    }
                                } else {
                                    /* An empty free list starts as a circular doubly linked list. */
                                    ((AllocatorBlock **)insert_area)[insert_head_offset /
                                                                     (sizeof(AllocatorBlock *))] =
                                        insert_block;
                                    insert_block->previous_free = insert_block;
                                    insert_block->next_free = insert_block;
                                }
                                insert_size = BlockSize(
                                    ((AllocatorBlock **)insert_area)[insert_head_offset /
                                                                     (sizeof(AllocatorBlock *))]);
                                if (insert_area->largest_free_size < insert_size) {
                                    insert_area->largest_free_size = insert_size;
                                }
                            } while (0);
                        } while (0);
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
        block = (AllocatorBlock *)(((unsigned char *)ptr) - 8);
        block_size = BlockSize(block);
        if ((block_size - required_size) >= MINIMUM_BLOCK_SIZE) {
            do {
                AllocatorBlock *split_block = block;
                unsigned int split_size = required_size;
                unsigned int split_old_size;
                unsigned int split_old_flags;
                int split_is_free;
                int split_is_allocated;
                unsigned int split_remaining_size;
                unsigned int split_owner;
                unsigned int split_boundary_flags;
                unsigned int split_boundary_size;
                AllocatorBlock *split_remaining;
                AllocatorBlock *split_following;
                split_old_size = BlockSize(split_block);
                split_old_flags = split_block->size_flags;
                split_is_free = !(split_old_flags & BLOCK_FLAG_2);
                split_is_allocated = !split_is_free;
                split_owner = (split_block->tagged_area & (~1)) | 1;
                split_remaining = (AllocatorBlock *)(((unsigned char *)split_block) + split_size);
                split_block->tagged_area = split_owner;
                split_block->size_flags = split_size;
                if ((split_old_flags & BLOCK_FLAG_4) != 0) {
                    split_block->size_flags |= BLOCK_FLAG_4;
                }
                if (split_is_allocated != 0) {
                    split_block->size_flags |= BLOCK_FLAG_2;
                    /* The target ORs this header before overwriting its size below. */
                    split_remaining->size_flags |= BLOCK_FLAG_4;
                } else {
                    *((unsigned int *)(((unsigned char *)split_remaining) - 4)) = split_size;
                }
                split_remaining->tagged_area = split_owner;
                split_remaining_size = split_old_size - split_size;
                split_remaining->size_flags = split_remaining_size;
                if (split_is_allocated != 0) {
                    split_remaining->size_flags |= BLOCK_FLAG_4;
                }
                if (split_is_allocated != 0) {
                    split_remaining->size_flags |= BLOCK_FLAG_2;
                    split_following = (AllocatorBlock *)(((unsigned char *)split_remaining) +
                                                         split_remaining_size);
                    split_following->size_flags |= BLOCK_FLAG_4;
                } else {
                    split_following = (AllocatorBlock *)(((unsigned char *)split_remaining) +
                                                         split_remaining_size);
                    *((unsigned int *)(((unsigned char *)split_following) - 4)) =
                        split_remaining_size;
                }
                if (split_is_free != 0) {
                    split_remaining->next_free = split_block->next_free;
                    split_remaining->next_free->previous_free = split_remaining;
                    split_remaining->previous_free = split_block;
                    split_block->next_free = split_remaining;
                }
                split_boundary_flags = split_remaining->size_flags;
                split_boundary_size = split_boundary_flags & BLOCK_SIZE_MASK;
                split_remaining->size_flags = split_boundary_flags & (~BLOCK_FLAG_2);
                split_following =
                    (AllocatorBlock *)(((unsigned char *)split_remaining) + split_boundary_size);
                split_following->size_flags &= ~BLOCK_FLAG_4;
                *((unsigned int *)(((unsigned char *)split_following) - 4)) = split_boundary_size;
                do {
                    AllocatorArea *insert_area = BlockArea(split_block);
                    unsigned int insert_head_offset;
                    AllocatorBlock *insert_following;
                    AllocatorBlock *insert_previous;
                    AllocatorBlock *insert_block = split_remaining;
                    unsigned int insert_size;
                    unsigned int insert_previous_size;
                    int insert_following_size;
                    insert_head_offset = FreeListOffset(insert_area);
                    if (((AllocatorBlock **)
                             insert_area)[insert_head_offset / (sizeof(AllocatorBlock *))] != 0) {
                        insert_block->previous_free =
                            ((AllocatorBlock **)
                                 insert_area)[insert_head_offset / (sizeof(AllocatorBlock *))]
                                ->previous_free;
                        insert_block->previous_free->next_free = insert_block;
                        insert_block->next_free =
                            ((AllocatorBlock **)
                                 insert_area)[insert_head_offset / (sizeof(AllocatorBlock *))];
                        ((AllocatorBlock **)
                             insert_area)[insert_head_offset / (sizeof(AllocatorBlock *))]
                            ->previous_free = insert_block;
                        ((AllocatorBlock **)
                             insert_area)[insert_head_offset / (sizeof(AllocatorBlock *))] =
                            insert_block;
                        insert_block =
                            ((AllocatorBlock **)
                                 insert_area)[insert_head_offset / (sizeof(AllocatorBlock *))];
                        /* A free block footer supplies the prior block's size. */
                        if ((insert_block->size_flags & BLOCK_FLAG_4) == 0) {
                            insert_previous_size =
                                *((unsigned int *)(((unsigned char *)insert_block) - 4));
                            if ((insert_previous_size & BLOCK_FLAG_2) != 0) {
                                insert_previous = insert_block;
                            } else {
                                insert_previous =
                                    (AllocatorBlock *)(((unsigned char *)insert_block) -
                                                       insert_previous_size);
                                insert_previous->size_flags &= 7;
                                insert_previous->size_flags |=
                                    (insert_previous_size + BlockSize(insert_block)) &
                                    BLOCK_SIZE_MASK;
                                if ((insert_previous->size_flags & BLOCK_FLAG_2) == 0) {
                                    insert_size = insert_previous_size + BlockSize(insert_block);
                                    *((unsigned int *)((((unsigned char *)insert_previous) +
                                                        insert_size) -
                                                       4)) = insert_size;
                                }
                                if (((AllocatorBlock **)insert_area)[insert_head_offset /
                                                                     (sizeof(AllocatorBlock *))] ==
                                    insert_block) {
                                    ((AllocatorBlock **)insert_area)[insert_head_offset /
                                                                     (sizeof(AllocatorBlock *))] =
                                        insert_block->next_free;
                                }
                                insert_block->next_free->previous_free =
                                    insert_block->previous_free;
                                insert_block->previous_free->next_free = insert_block->next_free;
                            }
                        } else {
                            insert_previous = insert_block;
                        }
                        ((AllocatorBlock **)
                             insert_area)[insert_head_offset / (sizeof(AllocatorBlock *))] =
                            insert_previous;
                        insert_block =
                            ((AllocatorBlock **)
                                 insert_area)[insert_head_offset / (sizeof(AllocatorBlock *))];
                        insert_size = BlockSize(insert_block);
                        insert_following =
                            (AllocatorBlock *)(((unsigned char *)insert_block) + insert_size);
                        if ((((AllocatorBlock *)(((unsigned char *)insert_block) + insert_size))
                                 ->size_flags &
                             BLOCK_FLAG_2) == 0) {
                            insert_following_size = BlockSize(insert_following);
                            insert_block->size_flags &= 7;
                            insert_size += insert_following_size;
                            insert_block->size_flags |= insert_size & BLOCK_SIZE_MASK;
                            if ((insert_block->size_flags & BLOCK_FLAG_2) == 0) {
                                *((unsigned int *)((((unsigned char *)insert_block) + insert_size) -
                                                   4)) = insert_size;
                            }
                            if ((insert_block->size_flags & BLOCK_FLAG_2) == 0) {
                                *((unsigned int *)(((unsigned char *)insert_block) +
                                                   insert_size)) &= ~BLOCK_FLAG_4;

                            } else {
                                *((unsigned int *)(((unsigned char *)insert_block) +
                                                   insert_size)) |= BLOCK_FLAG_4;
                            }
                            if (((AllocatorBlock **)insert_area)[insert_head_offset /
                                                                 (sizeof(AllocatorBlock *))] ==
                                insert_following) {
                                ((AllocatorBlock **)
                                     insert_area)[insert_head_offset / (sizeof(AllocatorBlock *))] =
                                    insert_following->next_free;
                            }
                            if (((AllocatorBlock **)insert_area)[insert_head_offset /
                                                                 (sizeof(AllocatorBlock *))] ==
                                insert_following) {
                                ((AllocatorBlock **)
                                     insert_area)[insert_head_offset / (sizeof(AllocatorBlock *))] =
                                    0;
                            }
                            insert_following->next_free->previous_free =
                                insert_following->previous_free;
                            insert_following->previous_free->next_free =
                                insert_following->next_free;
                        }
                    } else {
                        /* An empty free list starts as a circular doubly linked list. */
                        ((AllocatorBlock **)
                             insert_area)[insert_head_offset / (sizeof(AllocatorBlock *))] =
                            insert_block;
                        insert_block->previous_free = insert_block;
                        insert_block->next_free = insert_block;
                    }
                    insert_size = BlockSize(
                        ((AllocatorBlock **)
                             insert_area)[insert_head_offset / (sizeof(AllocatorBlock *))]);
                    if (insert_area->largest_free_size < insert_size) {
                        insert_area->largest_free_size = insert_size;
                    }
                } while (0);
            } while (0);
        }
    }
    return ptr;
}

#undef MAXIMUM_ALLOCATOR_REQUEST
#undef MINIMUM_BLOCK_SIZE
#undef BLOCK_FLAG_4
#undef BLOCK_FLAG_2
#undef BLOCK_SIZE_MASK
#undef __pool_free
#undef __pool_alloc
#undef __pool_realloc
