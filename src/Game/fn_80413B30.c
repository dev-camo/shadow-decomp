/* Reading of animation curves (the original file name is unknown; every name here is inferred
 * from the code). Each curve is a block of keys; a cursor holds the two keys around the current
 * frame. */
#include <string.h>

#include "Game/fn_80413B30_motion_keys.h"

/* The global playback state. */
MotionPlayback lbl_805E12C0;

/* Converts a half-precision float (1 sign, 5 exponent and 10 mantissa bits) to a float by moving
 * the bits and rebiasing the exponent. Only 0x0000 maps to 0.0f (a negative zero becomes -2^-15);
 * denormals, infinities and NaN are not handled.
 * Matching workaround: the else branch returns separately from the final return. With a single
 * return (or an early return for 0x0000) every caller's inlined conversion is scheduled and
 * allocated differently from the original. Revisit if another compiler version or inline setting
 * is found for this code. */
static inline float HalfToFloat(unsigned short half) {
    float result = 0.0f;
    if (half != 0) {
        unsigned int sign = (half & 0x8000) << 16;
        unsigned int magnitude = ((half & 0x7FFF) << 13) + 0x38000000;

        *(unsigned int *)&result = sign | magnitude;
    } else {
        return result;
    }
    return result;
}

/* Reads the key at a position into a MotionKey, stores its header in header and leaves the
 * position after the key.
 * Matching workaround: a macro, not an inline function. At the unit's inline depth of 1
 * (-inline auto,level=1) an inline function containing the HalfToFloat calls is called out of
 * line; with a greater depth it is inlined but the code still differs from the original. Revisit
 * if another compiler version or inline setting is found for this code. */
#define READ_KEY(key, position, header)                                                            \
    do {                                                                                           \
        const unsigned short *key_ = (position);                                                   \
        const unsigned short *data_ = key_ + 2;                                                    \
                                                                                                   \
        (header) = key_[0];                                                                        \
        (key).time = (float)((header) & MOTION_KEY_TIME_MASK);                                     \
        (key).tangentIn = HalfToFloat(key_[1]);                                                    \
        (key).tangentOut =                                                                         \
            ((header) & MOTION_KEY_SHARED_TANGENT) ? (key).tangentIn : HalfToFloat(*data_++);      \
        (key).value = HalfToFloat(*data_++);                                                       \
        (position) = data_;                                                                        \
    } while (0)

/* The distance back from a key to the previous one, in bytes, from the header of the later key. */
static inline int StepBackBytes(unsigned short header) {
    int elements = (header & MOTION_KEY_PREVIOUS_SHORT) ? 3 : 4;

    return elements * 2;
}

/* Copies the four values of a key one by one. */
static inline void CopyKey(MotionKey *to, const MotionKey *from) {
    to->time = from->time;
    to->tangentIn = from->tangentIn;
    to->tangentOut = from->tangentOut;
    to->value = from->value;
}

/* The header of the first key of a block. */
#define KEY_HEADER(block) (((const unsigned short *)((block) + 1))[0])

/* Moves a cursor to the first key of block, or to the last key when the playback runs backwards,
 * and steps to the keys around the current frame. The caller clears the cursor flags first.
 * Matching workaround: a macro because fn_80413E38 contains the same code as fn_80413B30 and, at
 * the inline depth of 1, an inline function holding this code stays an out-of-line local function
 * that both call. Revisit with READ_KEY. */
#define REWIND_KEYS(cursor, block)                                                                 \
    do {                                                                                           \
        ResourceBlock *block_ = (block);                                                           \
        const unsigned short *position_;                                                           \
        unsigned short header_;                                                                    \
                                                                                                   \
        if (lbl_805E12C0.speed < 0.0f) {                                                           \
            (cursor)->index = (cursor)->keyCount - 1;                                              \
            (cursor)->position =                                                                   \
                (const unsigned short *)((char *)block_ + block_->size - block_->trailingBytes -   \
                                         StepBackBytes(KEY_HEADER(block_)));                       \
            position_ = (cursor)->position;                                                        \
            READ_KEY((cursor)->next, position_, header_);                                          \
            (cursor)->position =                                                                   \
                (const unsigned short *)((char *)(cursor)->position - StepBackBytes(header_));     \
            CopyKey(&(cursor)->current, &(cursor)->next);                                          \
            (cursor)->index--;                                                                     \
            fn_80414120(cursor);                                                                   \
        } else {                                                                                   \
            (cursor)->index = 0;                                                                   \
            (cursor)->position = (const unsigned short *)(block_ + 1);                             \
            position_ = (cursor)->position;                                                        \
            READ_KEY((cursor)->current, position_, header_);                                       \
            (cursor)->position = position_;                                                        \
            CopyKey(&(cursor)->next, &(cursor)->current);                                          \
            (cursor)->index++;                                                                     \
            fn_804142B0(cursor);                                                                   \
        }                                                                                          \
    } while (0)

void fn_80413B30(MotionKeyCursor *cursor) {
    ResourceBlock *block = cursor->block;

    cursor->flags = 0;
    REWIND_KEYS(cursor, block);
}

void fn_80413E10(MotionKeyCursor *cursor) {
    memset(cursor, 0, sizeof(MotionKeyCursor));
}

void fn_80413E38(MotionKeyCursor *cursor, ResourceBlock *block) {
    cursor->flags = 0;
    cursor->keyCount = block->count;
    cursor->block = block;
    REWIND_KEYS(cursor, block);
}

void fn_80414120(MotionKeyCursor *cursor) {
    const unsigned short *position = cursor->position;
    short index = cursor->index;
    unsigned short header;

    if (lbl_805E12C0.currentFrame > cursor->current.time - MOTION_FRAME_EPSILON) {
        return;
    }
    while (index > -1) {
        const unsigned short *data = position;

        CopyKey(&cursor->next, &cursor->current);
        READ_KEY(cursor->current, data, header);
        index--;
        position = (const unsigned short *)((char *)position - StepBackBytes(header));
        if (lbl_805E12C0.currentFrame > cursor->current.time - MOTION_FRAME_EPSILON) {
            break;
        }
    }
    cursor->index = index;
    cursor->position = position;
}

void fn_804142B0(MotionKeyCursor *cursor) {
    const unsigned short *position = cursor->position;
    short index = cursor->index;
    short keyCount = cursor->keyCount;
    unsigned short header;

    if (lbl_805E12C0.currentFrame < MOTION_FRAME_EPSILON + cursor->next.time) {
        return;
    }
    while (index < keyCount) {
        CopyKey(&cursor->current, &cursor->next);
        READ_KEY(cursor->next, position, header);
        index++;
        if (lbl_805E12C0.currentFrame < MOTION_FRAME_EPSILON + cursor->next.time) {
            break;
        }
    }
    cursor->index = index;
    cursor->position = position;
}

/* Evaluators of the curve at the playback frame, each in a version for the value and the slope
 * together (fn_80414438, fn_80414CBC), for the slope (fn_8041474C, fn_80414FD0) and for the value
 * (fn_804149E0, fn_8041525C). fn_80414438, fn_8041474C and fn_804149E0 step back through the keys
 * when the frame lies before the current key; fn_80414CBC, fn_80414FD0 and fn_8041525C step forward
 * when it lies after the next key. Between two keys they call the interpolation functions that
 * fn_80415784 stores in the playback state. */
void fn_80414438(float *value, float *slope, MotionKeyCursor *cursor) {
    MotionKey *current = &cursor->current;
    MotionKey *next = &cursor->next;
    const unsigned short *position = cursor->position;
    short index = cursor->index;
    unsigned short header;

    if (cursor->flags & MOTION_CURSOR_BOTH_HELD) {
        *value = cursor->cachedValue;
        *slope = cursor->cachedSlope;
        return;
    }
    if (lbl_805E12C0.currentFrame > next->time - MOTION_FRAME_EPSILON) {
        *value = next->value;
        if (lbl_805E12C0.currentFrame > MOTION_FRAME_EPSILON + next->time) {
            *slope = next->tangentOut;
        } else {
            *slope = 0.5f * (next->tangentIn + next->tangentOut);
        }
        return;
    }
    if (lbl_805E12C0.currentFrame > MOTION_FRAME_EPSILON + current->time) {
        lbl_805E12C0.interpolateBoth(value, slope, current, next, lbl_805E12C0.currentFrame);
        return;
    }
    if (lbl_805E12C0.currentFrame > current->time - MOTION_FRAME_EPSILON) {
        *value = current->value;
        *slope = 0.5f * (current->tangentIn + current->tangentOut);
        return;
    }
    while (index > -1) {
        const unsigned short *data = position;

        CopyKey(next, current);
        READ_KEY(*current, data, header);
        index--;
        position = (const unsigned short *)((char *)position - StepBackBytes(header));
        if (lbl_805E12C0.currentFrame > current->time - MOTION_FRAME_EPSILON) {
            cursor->flags &= ~MOTION_CURSOR_BOTH_HELD;
            if (lbl_805E12C0.currentFrame > MOTION_FRAME_EPSILON + current->time) {
                lbl_805E12C0.interpolateBoth(value, slope, current, next,
                                             lbl_805E12C0.currentFrame);
            } else {
                *value = current->value;
                *slope = 0.5f * (current->tangentIn + current->tangentOut);
            }
            return;
        }
    }
    cursor->index = index;
    cursor->position = position;
    if (current->tangentIn == 0.0f) {
        cursor->flags |= MOTION_CURSOR_BOTH_HELD;
        *value = cursor->cachedValue = current->value;
        *slope = cursor->cachedSlope = current->tangentIn;
    } else {
        *value = current->tangentIn * (lbl_805E12C0.currentFrame - current->time) + current->value;
        *slope = current->tangentIn;
    }
}

float fn_8041474C(MotionKeyCursor *cursor) {
    MotionKey *current = &cursor->current;
    MotionKey *next = &cursor->next;
    const unsigned short *position = cursor->position;
    short index = cursor->index;
    unsigned short header;

    if (cursor->flags & MOTION_CURSOR_SLOPE_HELD) {
        return cursor->cachedSlope;
    }
    if (lbl_805E12C0.currentFrame > next->time - MOTION_FRAME_EPSILON) {
        if (lbl_805E12C0.currentFrame > MOTION_FRAME_EPSILON + next->time) {
            return next->tangentOut;
        }
        return 0.5f * (next->tangentIn + next->tangentOut);
    }
    if (lbl_805E12C0.currentFrame > MOTION_FRAME_EPSILON + current->time) {
        return lbl_805E12C0.interpolateSlope(current, next, lbl_805E12C0.currentFrame);
    }
    if (lbl_805E12C0.currentFrame > current->time - MOTION_FRAME_EPSILON) {
        return 0.5f * (current->tangentIn + current->tangentOut);
    }
    while (index > -1) {
        const unsigned short *data = position;

        CopyKey(next, current);
        READ_KEY(*current, data, header);
        index--;
        position = (const unsigned short *)((char *)position - StepBackBytes(header));
        if (lbl_805E12C0.currentFrame > current->time - MOTION_FRAME_EPSILON) {
            cursor->flags &= ~MOTION_CURSOR_SLOPE_HELD;
            if (lbl_805E12C0.currentFrame > MOTION_FRAME_EPSILON + current->time) {
                return lbl_805E12C0.interpolateSlope(current, next, lbl_805E12C0.currentFrame);
            }
            return 0.5f * (current->tangentIn + current->tangentOut);
        }
    }
    cursor->index = index;
    cursor->position = position;
    cursor->flags |= MOTION_CURSOR_SLOPE_HELD;
    return cursor->cachedSlope = current->tangentIn;
}

float fn_804149E0(MotionKeyCursor *cursor) {
    MotionKey *current = &cursor->current;
    MotionKey *next = &cursor->next;
    const unsigned short *position = cursor->position;
    short index = cursor->index;
    unsigned short header;

    if (cursor->flags & MOTION_CURSOR_VALUE_HELD) {
        return cursor->cachedValue;
    }
    if (lbl_805E12C0.currentFrame > next->time - MOTION_FRAME_EPSILON) {
        return next->value;
    }
    if (lbl_805E12C0.currentFrame > MOTION_FRAME_EPSILON + current->time) {
        if (cursor->flags & MOTION_CURSOR_FLAT_SEGMENT) {
            return cursor->cachedValue;
        }
        return lbl_805E12C0.interpolateValue(current, next, lbl_805E12C0.currentFrame);
    }
    if (lbl_805E12C0.currentFrame > current->time - MOTION_FRAME_EPSILON) {
        return current->value;
    }
    while (index > -1) {
        const unsigned short *data = position;

        CopyKey(next, current);
        READ_KEY(*current, data, header);
        index--;
        position = (const unsigned short *)((char *)position - StepBackBytes(header));
        if (lbl_805E12C0.currentFrame > current->time - MOTION_FRAME_EPSILON) {
            cursor->flags &= ~MOTION_CURSOR_VALUE_HELD;
            if (lbl_805E12C0.currentFrame > MOTION_FRAME_EPSILON + current->time) {
                if (current->tangentOut == 0.0f && next->tangentIn == 0.0f &&
                    current->value == next->value) {
                    cursor->flags |= MOTION_CURSOR_FLAT_SEGMENT;
                    return cursor->cachedValue = current->value;
                }
                cursor->flags &= ~MOTION_CURSOR_FLAT_SEGMENT;
                return lbl_805E12C0.interpolateValue(current, next, lbl_805E12C0.currentFrame);
            }
            return current->value;
        }
    }
    cursor->index = index;
    cursor->position = position;
    if (current->tangentIn == 0.0f) {
        cursor->flags |= MOTION_CURSOR_VALUE_HELD;
        return cursor->cachedValue = current->value;
    }
    return current->tangentIn * (lbl_805E12C0.currentFrame - current->time) + current->value;
}

void fn_80414CBC(float *value, float *slope, MotionKeyCursor *cursor) {
    MotionKey *current = &cursor->current;
    MotionKey *next = &cursor->next;
    const unsigned short *position = cursor->position;
    short index = cursor->index;
    short keyCount = cursor->keyCount;
    unsigned short header;

    if (cursor->flags & MOTION_CURSOR_BOTH_HELD) {
        *value = cursor->cachedValue;
        *slope = cursor->cachedSlope;
        return;
    }
    if (lbl_805E12C0.currentFrame < MOTION_FRAME_EPSILON + current->time) {
        *value = current->value;
        if (lbl_805E12C0.currentFrame < current->time - MOTION_FRAME_EPSILON) {
            *slope = current->tangentIn;
        } else {
            *slope = 0.5f * (current->tangentIn + current->tangentOut);
        }
        return;
    }
    if (lbl_805E12C0.currentFrame < next->time - MOTION_FRAME_EPSILON) {
        lbl_805E12C0.interpolateBoth(value, slope, current, next, lbl_805E12C0.currentFrame);
        return;
    }
    if (lbl_805E12C0.currentFrame < MOTION_FRAME_EPSILON + next->time) {
        *value = next->value;
        *slope = 0.5f * (next->tangentIn + next->tangentOut);
        return;
    }
    while (index < keyCount) {
        CopyKey(current, next);
        READ_KEY(*next, position, header);
        index++;
        if (lbl_805E12C0.currentFrame < MOTION_FRAME_EPSILON + next->time) {
            cursor->index = index;
            cursor->position = position;
            cursor->flags &= ~MOTION_CURSOR_BOTH_HELD;
            if (lbl_805E12C0.currentFrame < next->time - MOTION_FRAME_EPSILON) {
                lbl_805E12C0.interpolateBoth(value, slope, current, next,
                                             lbl_805E12C0.currentFrame);
            } else {
                *value = next->value;
                *slope = 0.5f * (next->tangentIn + next->tangentOut);
            }
            return;
        }
    }
    cursor->index = index;
    cursor->position = position;
    if (next->tangentOut == 0.0f) {
        cursor->flags |= MOTION_CURSOR_BOTH_HELD;
        *value = cursor->cachedValue = next->value;
        *slope = cursor->cachedSlope = next->tangentOut;
    } else {
        *value = next->tangentOut * (lbl_805E12C0.currentFrame - next->time) + next->value;
        *slope = next->tangentOut;
    }
}

float fn_80414FD0(MotionKeyCursor *cursor) {
    MotionKey *current = &cursor->current;
    MotionKey *next = &cursor->next;
    const unsigned short *position = cursor->position;
    short index = cursor->index;
    short keyCount = cursor->keyCount;
    unsigned short header;

    if (cursor->flags & MOTION_CURSOR_SLOPE_HELD) {
        return cursor->cachedSlope;
    }
    if (lbl_805E12C0.currentFrame < MOTION_FRAME_EPSILON + current->time) {
        if (lbl_805E12C0.currentFrame < current->time - MOTION_FRAME_EPSILON) {
            return current->tangentIn;
        }
        return 0.5f * (current->tangentIn + current->tangentOut);
    }
    if (lbl_805E12C0.currentFrame < next->time - MOTION_FRAME_EPSILON) {
        return lbl_805E12C0.interpolateSlope(current, next, lbl_805E12C0.currentFrame);
    }
    if (lbl_805E12C0.currentFrame < MOTION_FRAME_EPSILON + next->time) {
        return 0.5f * (next->tangentIn + next->tangentOut);
    }
    while (index < keyCount) {
        CopyKey(current, next);
        READ_KEY(*next, position, header);
        index++;
        if (lbl_805E12C0.currentFrame < MOTION_FRAME_EPSILON + next->time) {
            cursor->index = index;
            cursor->position = position;
            cursor->flags &= ~MOTION_CURSOR_SLOPE_HELD;
            if (lbl_805E12C0.currentFrame < next->time - MOTION_FRAME_EPSILON) {
                return lbl_805E12C0.interpolateSlope(current, next, lbl_805E12C0.currentFrame);
            }
            return 0.5f * (next->tangentIn + next->tangentOut);
        }
    }
    cursor->index = index;
    cursor->position = position;
    cursor->flags |= MOTION_CURSOR_SLOPE_HELD;
    return cursor->cachedSlope = next->tangentOut;
}

float fn_8041525C(MotionKeyCursor *cursor) {
    MotionKey *current = &cursor->current;
    MotionKey *next = &cursor->next;
    const unsigned short *position = cursor->position;
    short index = cursor->index;
    short keyCount = cursor->keyCount;
    unsigned short header;

    if (cursor->flags & MOTION_CURSOR_VALUE_HELD) {
        return cursor->cachedValue;
    }
    if (lbl_805E12C0.currentFrame < MOTION_FRAME_EPSILON + current->time) {
        return current->value;
    }
    if (lbl_805E12C0.currentFrame < next->time - MOTION_FRAME_EPSILON) {
        if (cursor->flags & MOTION_CURSOR_FLAT_SEGMENT) {
            return cursor->cachedValue;
        }
        return lbl_805E12C0.interpolateValue(current, next, lbl_805E12C0.currentFrame);
    }
    if (lbl_805E12C0.currentFrame < MOTION_FRAME_EPSILON + next->time) {
        return next->value;
    }
    while (index < keyCount) {
        CopyKey(current, next);
        READ_KEY(*next, position, header);
        index++;
        if (lbl_805E12C0.currentFrame < MOTION_FRAME_EPSILON + next->time) {
            cursor->index = index;
            cursor->position = position;
            cursor->flags &= ~MOTION_CURSOR_VALUE_HELD;
            if (lbl_805E12C0.currentFrame < next->time - MOTION_FRAME_EPSILON) {
                if (current->tangentOut == 0.0f && next->tangentIn == 0.0f &&
                    current->value == next->value) {
                    cursor->flags |= MOTION_CURSOR_FLAT_SEGMENT;
                    return cursor->cachedValue = next->value;
                }
                cursor->flags &= ~MOTION_CURSOR_FLAT_SEGMENT;
                return lbl_805E12C0.interpolateValue(current, next, lbl_805E12C0.currentFrame);
            }
            return next->value;
        }
    }
    cursor->index = index;
    cursor->position = position;
    if (next->tangentOut == 0.0f) {
        cursor->flags |= MOTION_CURSOR_VALUE_HELD;
        return cursor->cachedValue = next->value;
    }
    return next->tangentOut * (lbl_805E12C0.currentFrame - next->time) + next->value;
}

/* The interpolated value and slope between two keys: fn_804156D8 and fn_8041564C in one pass. */
void fn_80415530(float *value, float *slope, MotionKey *from, MotionKey *to, float frame) {
    float elapsed = frame - from->time;
    float elapsedSquared = elapsed * elapsed;
    float inverseSpan = 1.0f / (to->time - from->time);
    float inverseSpanSquared = inverseSpan * inverseSpan;
    float cubicTerm = inverseSpanSquared * (elapsedSquared * elapsed);
    float quadraticTerm = elapsedSquared * inverseSpan;
    float cubicWeight = 2.0f * cubicTerm * inverseSpan;
    float squareWeight = 3.0f * elapsedSquared * inverseSpanSquared;
    float fromWeight = cubicWeight - squareWeight;
    float toWeight = -cubicWeight + squareWeight;
    float fromSlopeWeight = elapsed + (cubicTerm - quadraticTerm - quadraticTerm);
    float toSlopeWeight = cubicTerm - quadraticTerm;
    float fromTerm = 6.0f * elapsedSquared * inverseSpanSquared * inverseSpan;
    float middleTerm = 6.0f * elapsed * inverseSpanSquared;
    float stepTerm = 2.0f * elapsed * inverseSpan;
    float toSlopeTerm = squareWeight - stepTerm;

    float interpolatedValue = from->value * (1.0f + fromWeight) + to->value * toWeight +
                              from->tangentOut * fromSlopeWeight + to->tangentIn * toSlopeWeight;
    float interpolatedSlope =
        from->value * (fromTerm - middleTerm) + to->value * (-fromTerm + middleTerm) +
        from->tangentOut * (1.0f + (toSlopeTerm - stepTerm)) + to->tangentIn * toSlopeTerm;

    *value = interpolatedValue;
    *slope = interpolatedSlope;
}

/* The derivative of fn_804156D8 with respect to the frame. */
float fn_8041564C(MotionKey *from, MotionKey *to, float frame) {
    float elapsed = frame - from->time;
    float elapsedSquared = elapsed * elapsed;
    float inverseSpan = 1.0f / (to->time - from->time);
    float inverseSpanSquared = inverseSpan * inverseSpan;
    float fromTerm = 6.0f * elapsedSquared * inverseSpanSquared * inverseSpan;
    float middleTerm = 6.0f * elapsed * inverseSpanSquared;
    float stepTerm = 2.0f * elapsed * inverseSpan;
    /* Matching workaround: a statement of its own, as in fn_80415530. In one expression with the
     * subtraction of stepTerm, the multiply and the subtraction compile to a fused fmsubs, which
     * the original does not have. Revisit if another compiler version or a -fp_contract setting is
     * found for this code. */
    float squareWeight = 3.0f * elapsedSquared * inverseSpanSquared;
    float toSlopeTerm = squareWeight - stepTerm;

    return from->value * (fromTerm - middleTerm) + to->value * (-fromTerm + middleTerm) +
           from->tangentOut * (1.0f + (toSlopeTerm - stepTerm)) + to->tangentIn * toSlopeTerm;
}

/* The cubic Hermite interpolation of the value between two keys, with the tangentOut of the first
 * and the tangentIn of the second. */
float fn_804156D8(MotionKey *from, MotionKey *to, float frame) {
    float elapsed = frame - from->time;
    float elapsedSquared = elapsed * elapsed;
    float inverseSpan = 1.0f / (to->time - from->time);
    float inverseSpanSquared = inverseSpan * inverseSpan;
    float cubicTerm = inverseSpanSquared * (elapsedSquared * elapsed);
    float quadraticTerm = elapsedSquared * inverseSpan;
    float cubicWeight = 2.0f * cubicTerm * inverseSpan;
    float squareWeight = 3.0f * elapsedSquared * inverseSpanSquared;
    float fromWeight = cubicWeight - squareWeight;
    float toWeight = -cubicWeight + squareWeight;
    float fromSlopeWeight = elapsed + (cubicTerm - quadraticTerm - quadraticTerm);
    float toSlopeWeight = cubicTerm - quadraticTerm;

    return from->value * (1.0f + fromWeight) + to->value * toWeight +
           from->tangentOut * fromSlopeWeight + to->tangentIn * toSlopeWeight;
}

void fn_8041575C(const MotionRange *range) {
    lbl_805E12C0.startFrame = range->startFrame;
    lbl_805E12C0.endFrame = range->endFrame;
    lbl_805E12C0.speed = range->speed;
    lbl_805E12C0.currentFrame = range->currentFrame;
}

void fn_80415784(void) {
    memset(&lbl_805E12C0, 0, sizeof(lbl_805E12C0));
    lbl_805E12C0.interpolateValue = fn_804156D8;
    lbl_805E12C0.interpolateSlope = fn_8041564C;
    lbl_805E12C0.interpolateBoth = fn_80415530;
    lbl_805E12C0.unknown10 = 0;
}
