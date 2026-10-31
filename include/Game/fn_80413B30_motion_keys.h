#ifndef FN_80413B30_MOTION_KEYS_H
#define FN_80413B30_MOTION_KEYS_H

#include "Game/fn_80411714_resource_groups.h"
#include "layout_check.h"

/* Reading of the keys of an animation curve (the unit at 0x80413B30; every name here is inferred
 * from the code). The elements of a resource block hold the keys of one curve. A key is a 16-bit
 * header followed by up to three half-precision floats. */

/* Bits of the 16-bit header of a key; the other 14 bits hold the time. */
#define MOTION_KEY_SHARED_TANGENT 0x8000 /* tangentOut is not stored: it equals tangentIn */
/* The key before this one is three half-words long (it shares its tangent) instead of four. The
 * last key's size is taken from this bit of the first key. */
#define MOTION_KEY_PREVIOUS_SHORT 0x4000
#define MOTION_KEY_TIME_MASK 0x3FFF

/* The frame is compared with a key time within this distance (inferred from the comparisons). */
#define MOTION_FRAME_EPSILON 0.0001f

/* Bits of MotionKeyCursor.flags. A held bit means that the evaluator of that output found a
 * constant result and keeps it in cachedValue and cachedSlope. */
#define MOTION_CURSOR_VALUE_HELD 0x1   /* the value outside the keys is cachedValue */
#define MOTION_CURSOR_SLOPE_HELD 0x2   /* the slope outside the keys is cachedSlope */
#define MOTION_CURSOR_BOTH_HELD 0x4    /* the value and the slope outside the keys are cached */
#define MOTION_CURSOR_FLAT_SEGMENT 0x8 /* the value between the two keys is cachedValue */

/* A key of a curve: the time and three values. */
typedef struct MotionKey {
    /* 0x00 */ float time;
    /* 0x04 */ float tangentIn;
    /* 0x08 */ float tangentOut;
    /* 0x0C */ float value;
} MotionKey;
CHECK_SIZEOF(MotionKey, 0x10);

/* The reading position in the keys of one block, with the two keys around the current frame. */
typedef struct MotionKeyCursor {
    /* 0x00 */ short index;
    /* 0x02 */ short keyCount;
    /* 0x04 */ unsigned int flags;
    /* 0x08 */ const unsigned short *position;
    /* 0x0C */ ResourceBlock *block;
    /* 0x10 */ float cachedValue;
    /* 0x14 */ float cachedSlope;
    /* 0x18 */ unsigned char unknown18[8];
    /* 0x20 */ MotionKey current;
    /* 0x30 */ MotionKey next;
} MotionKeyCursor;
CHECK_SIZEOF(MotionKeyCursor, 0x40);

/* The playback state that the curve functions read (the 0x20 bytes at 0x805E12C0). */
typedef struct MotionPlayback {
    /* 0x00 */ float unknown00;
    /* 0x04 */ float unknown04;
    /* 0x08 */ float direction; /* negative when the playback runs backwards */
    /* 0x0C */ float frame;
    /* 0x10 */ unsigned int unknown10;
    /* 0x14 */ float (*interpolateValue)(MotionKey *from, MotionKey *to, float frame);
    /* 0x18 */ float (*interpolateSlope)(MotionKey *from, MotionKey *to, float frame);
    /* 0x1C */ void (*interpolateBoth)(float *value, float *slope, MotionKey *from, MotionKey *to,
                                       float frame);
} MotionPlayback;
CHECK_SIZEOF(MotionPlayback, 0x20);

#ifdef __cplusplus
extern "C" {
#endif

void fn_80413B30(MotionKeyCursor *cursor);
void fn_80413E10(MotionKeyCursor *cursor);
void fn_80413E38(MotionKeyCursor *cursor, ResourceBlock *block);
void fn_80414120(MotionKeyCursor *cursor);
void fn_804142B0(MotionKeyCursor *cursor);
void fn_80414438(float *value, float *slope, MotionKeyCursor *cursor);
float fn_8041474C(MotionKeyCursor *cursor);
float fn_804149E0(MotionKeyCursor *cursor);
void fn_80414CBC(float *value, float *slope, MotionKeyCursor *cursor);
float fn_80414FD0(MotionKeyCursor *cursor);
float fn_8041525C(MotionKeyCursor *cursor);
void fn_80415530(float *value, float *slope, MotionKey *from, MotionKey *to, float frame);
float fn_8041564C(MotionKey *from, MotionKey *to, float frame);
float fn_804156D8(MotionKey *from, MotionKey *to, float frame);
void fn_8041575C(const float *state);
void fn_80415784(void);

#ifdef __cplusplus
}
#endif

#endif
