/* RenderWare pipeline library: sorting an array of records by an unsigned key word inside each
 * record. Unlocking a pipeline sorts its input requirements with it. The original file name is
 * unknown. */
#include "rw/rwcore.h"

/* the key of a record: the word at keyOffset in it */
#define SORTKEY(record, keyOffset) (*(RwUInt32 *)((RwUInt8 *)(record) + (keyOffset)))

/* Ranges smaller than this many records are left to the insertion sort */
#define SORTMINRANGE 5

/* The most ranges the radix sort keeps waiting; each one splits off at a lower key bit, so 32 is
 * enough for 32-bit keys */
#define SORTMAXRANGES 32

/* A range of records still to be sorted on the key bits from bit down */
typedef struct SortRange {
    /* 0x0 */ RwUInt8 *first;
    /* 0x4 */ RwUInt8 *last; /* inclusive */
    /* 0x8 */ RwUInt32 bit;
} SortRange;
CHECK_SIZEOF(SortRange, 0xC);

/* Swaps two records word by word */
#define SORTSWAP(a, b, size)                                                                       \
    do {                                                                                           \
        RwUInt32 *wordA_ = (RwUInt32 *)(a);                                                        \
        RwUInt32 *wordB_ = (RwUInt32 *)(b);                                                        \
        RwUInt32 remaining_;                                                                       \
                                                                                                   \
        for (remaining_ = (size); remaining_ >= sizeof(RwUInt32);                                  \
             remaining_ -= sizeof(RwUInt32)) {                                                     \
            RwUInt32 temp_ = *wordB_;                                                              \
                                                                                                   \
            *wordB_ = *wordA_;                                                                     \
            *wordA_ = temp_;                                                                       \
            wordA_++;                                                                              \
            wordB_++;                                                                              \
        }                                                                                          \
    } while (0)

/* Radix exchange sort of the records from first to last (inclusive) on the key bits from bit
 * down: records with the bit clear move before those with it set, then both halves are sorted
 * on the next bit. Ranges smaller than SORTMINRANGE records are left unsorted. */
void fn_8049F3F8(RwUInt8 *first, RwUInt8 *last, RwUInt32 size, RwUInt32 keyOffset, RwUInt32 bit) {
    RwUInt32 minRange = size * SORTMINRANGE;
    SortRange stack[SORTMAXRANGES];
    SortRange *top;

    stack[0].first = first;
    stack[0].last = last;
    stack[0].bit = bit;
    top = &stack[1];
    do {
        top--;
        first = top->first;
        last = top->last;
        bit = top->bit;
        for (;;) {
            RwUInt8 *start = first;
            RwUInt8 *end = last;

            /* move the records with the bit set to the end */
            for (;;) {
                while (!(SORTKEY(first, keyOffset) & bit)) {
                    first += size;
                    if (first > last) {
                        goto partitioned;
                    }
                }
                while (SORTKEY(last, keyOffset) & bit) {
                    last -= size;
                    if (first > last) {
                        goto partitioned;
                    }
                }
                SORTSWAP(first, last, size);
                first += size;
                last -= size;
                if (first > last) {
                    break;
                }
            }
        partitioned:
            bit >>= 1;
            if (bit == 0) {
                break;
            }

            /* keep the upper part for later and go on with the lower one */
            if (end >= last + size + minRange) {
                top->first = last + size;
                top->last = end;
                top->bit = bit;
                top++;
            }
            last = first - size;
            first = start;
            if (last < start + minRange) {
                break;
            }
        }
    } while (top != stack);
}

/* Insertion sort of count records whose first record has the smallest key, which stops
 * every search for a record's place */
static inline void SortInsertion(RwUInt8 *record, RwUInt32 count, RwUInt32 size,
                                 RwUInt32 keyOffset) {
    /* Matching workaround: from the second record on, the loop advances and counts down in its
     * condition. Measured in fn_8049F5B4 (190 words in every form): the closest alternative,
     * for (record += size; --count != 0; record += size), differs in 4 words (a register copy
     * becomes an add, one branch target moves and the count decrement is scheduled one slot
     * later); a for (;;) loop that advances, counts down and breaks at its top differs in 64
     * words, and while (--count != 0) with the advance in the body in 63. Revisit with the for
     * form when the original loop form is found. */
    while (record += size, --count != 0) {
        RwUInt32 key;
        RwUInt8 *place;

        /* swap the record down past every record before it with a larger key */
        key = SORTKEY(record, keyOffset);
        place = record;
        while (SORTKEY(place -= size, keyOffset) > key) {
            SORTSWAP(place, place + size, size);
        }
    }
}

/* Sorts numElements records of elementSize bytes into ascending order of the key word at
 * keyOffset, whose values lie between minKey and maxKey: a radix sort brings the records close
 * to their places, then an insertion sort finishes */
void fn_8049F5B4(void *base, RwUInt32 numElements, RwUInt32 elementSize, RwUInt32 keyOffset,
                 RwUInt32 minKey, RwUInt32 maxKey) {
    if (base != NULL && keyOffset + sizeof(RwUInt32) <= elementSize && minKey < maxKey) {
        if (numElements > SORTMINRANGE) {
            RwInt32 topBit; /* of maxKey; the radix sort starts at this bit */

            if (maxKey != 0) {
                topBit = 0;
                while ((maxKey >>= 1) != 0) {
                    topBit++;
                }
            } else {
                topBit = -1;
            }
            fn_8049F3F8(base, (RwUInt8 *)base + (numElements - 1) * elementSize, elementSize,
                        keyOffset, 1 << topBit);
        }

        if (numElements > 1) {
            RwUInt32 i;
            RwUInt32 minValue;
            RwUInt32 minIndex;

            /* the radix sort leaves the smallest key among the first records; move it to the
             * front so that the insertion sort below always stops */
            i = SORTMINRANGE - 1;
            if (numElements - 1 < i) {
                i = numElements - 1;
            }
            minIndex = i;
            minValue = SORTKEY((RwUInt8 *)base + i * elementSize, keyOffset);
            i--;
            do {
                RwUInt32 value = SORTKEY((RwUInt8 *)base + i * elementSize, keyOffset);

                if (value < minValue) {
                    minValue = value;
                    minIndex = i;
                }
            } while (i-- != 0);
            if (minIndex != 0) {
                SORTSWAP(base, (RwUInt8 *)base + minIndex * elementSize, elementSize);
            }

            SortInsertion(base, numElements, elementSize, keyOffset);
        }
    }
}
