#ifndef FN_804157E8_MOTION_CONTROL_H
#define FN_804157E8_MOTION_CONTROL_H

#include "Game/fn_8040_vector_types.h"
#include "Game/fn_80410630_pack.h"
#include "Game/fn_8041116C_matrix_stack.h"
#include "Game/fn_80413B30_motion_keys.h"
#include "Game/fn_804157E8_motion_range.h"
#include "layout_check.h"

/* The motion control: one original C++ file (its name is unknown) at 0x804157E8-0x80420724 with
 * the pooled float constants at 0x805FAE00-0x805FAE30, the .sbss variables at 0x805F13D0 and the
 * static initializer fn_804206A4. Until all of it matches, the file is built as consecutive units
 * with one source file each: Game/fn_804157E8.c, fn_8041583C.c, fn_80415854.c, fn_80416198.cpp,
 * fn_8041FD6C.c, fn_8041FD70.cpp, fn_8041FF08.c, fn_8041FF0C.cpp, fn_804202B8.c, fn_80420308.cpp,
 * fn_80420438.c and fn_8042043C.cpp, so that the units that already match keep their own objects.
 *
 * The control's record is _stMtnPlayWork (the original name, see below; the code calls it the
 * control); the stMtn function names are original symbols, and every other name here is inferred
 * from the code. A control owns an array of players, each of which plays one motion, and the tree
 * of nodes that the motions move. Players are addressed by an id: a table maps an id to the index
 * of its player, or to -1 when the id names no player. */

/* Bits of MotionPlayer.stateFlags. The stMtnRequest frame and speed setters set a bit for the
 * value they store in the player (MOTION_PLAYER_REQUESTED first, then the bit of the value). */
#define MOTION_PLAYER_REQUESTED 0x1
#define MOTION_PLAYER_REQUEST_START_FRAME 0x8
#define MOTION_PLAYER_REQUEST_END_FRAME 0x10
#define MOTION_PLAYER_REQUEST_CURRENT_FRAME 0x20
#define MOTION_PLAYER_REQUEST_SPEED 0x40
/* Makes stMtnRequestBlendRatio try the first ratio of the blend (it falls back to the second when
 * the first kind is not 1 or 2). */
#define MOTION_PLAYER_BLEND_FIRST 0x200
/* Set by fn_80415F98 together with MOTION_PLAYER_REQUESTED after fn_8041CFB0 (meaning unknown). */
#define MOTION_PLAYER_STATE_2000 0x2000

/* Bits of MotionPlayer.flags (fn_80415854 and fn_80415A18 read them to prepare a range). */
/* With MOTION_PLAYER_FLAG_400000: when MotionPlayer.unknown12C is within MOTION_LIMIT_EPSILON of
 * the limit the speed moves towards (endFrame for a non-negative speed, startFrame otherwise), the
 * speed grows by that epsilon in its own direction. */
#define MOTION_PLAYER_NUDGE_AT_LIMIT 0x2
/* fn_80415854, fn_80415A18, fn_80415CDC and fn_80415D40 use a player only when this bit is set,
 * and fn_80415EA4 copies the current frame of the range to MotionPlayer.unknown12C only when it is
 * still set after fn_8041AB3C (meaning inferred). fn_8041B884 gives the motion of every listed
 * player that has it back to the resource functions. */
#define MOTION_PLAYER_ACTIVE 0x4
/* With this bit fn_804188A0 also runs the second callback of each track of the player (the meaning
 * of the bit is not known). */
#define MOTION_PLAYER_FLAG_8 0x8
/* The speed of the range is replaced by 0. */
#define MOTION_PLAYER_ZERO_SPEED 0x20000
/* Unless MOTION_PLAYER_ZERO_SPEED is set, this bit makes the speed of the range
 * currentFrame - unknown12C, or with MOTION_PLAYER_NUDGE_AT_LIMIT enables that nudge instead (the
 * meaning of the bit is not known). */
#define MOTION_PLAYER_FLAG_400000 0x400000

/* Bit of _stMtnPlayWork.flags that makes fn_804188A0 truncate the current frame of the range it
 * hands to fn_8041575C to a whole number. */
#define MOTION_CONTROL_WHOLE_FRAMES 0x8

/* The distance within which fn_80415854 and fn_80415A18 treat unknown12C as being at a limit, and
 * the amount by which they then grow the speed (one .sdata2 constant, 0.0001f, serves both). */
#define MOTION_LIMIT_EPSILON 0.0001f

/* A vector followed by a word that this code does not use (only the 0x10 stride is known). Most
 * entries of _stMtnPlayWork.vectors are positions, but entry 1 holds the angles that fn_804173A8
 * builds the matrix from. */
typedef struct MotionWorkVector {
    /* 0x00 */ Fn8040Vector3 value;
    /* 0x0C */ float unknown0C;
} MotionWorkVector;
CHECK_SIZEOF(MotionWorkVector, 0x10);

/* A transform of a control: a matrix followed by data that is not mapped yet. */
typedef struct MotionTransform {
    /* 0x00 */ Fn8040Matrix34 matrix;
    /* 0x30 */ unsigned char unknown30[0x40];
} MotionTransform;
CHECK_SIZEOF(MotionTransform, 0x70);

/* What a node holds for one player (the entries of a node are indexed by the index of the player).
 * The floats are pairs of a factor 1 and a value 0 when the entry is reset. */
typedef struct MotionNodeEntry {
    /* 0x00 */ int unknown00;
    /* 0x04 */ int unknown04;
    /* 0x08 */ float unknown08;
    /* 0x0C */ float unknown0C;
    /* 0x10 */ float unknown10;
    /* 0x14 */ float unknown14;
    /* 0x18 */ unsigned char unknown18[8];
} MotionNodeEntry;
CHECK_SIZEOF(MotionNodeEntry, 0x20);

/* A node of the tree of a control (partial layout: the size is not known): the functions that
 * search or walk the tree follow its two child links recursively. Its definition is the node of
 * the pack that the tree is built from (fn_8041B980 starts at the root node of the pack entry and
 * fn_8041BE30 stores each pack node here while it follows the links of the pack nodes); the
 * functions here read its id, and fn_80415BDC compares values[0] with 1 and tests the low three
 * bits of values[1]. */
typedef struct MotionNode {
    /* 0x00 */ unsigned char unknown00[0x0C];
    /* 0x0C */ PackNode *definition;
    /* 0x10 */ MotionTransform *transform;
    /* 0x14 */ unsigned char unknown14[4];
    /* 0x18 */ MotionNodeEntry *entries;
    /* 0x1C */ int unknown1C;
    /* 0x20 */ int unknown20;
    /* 0x24 */ struct MotionNode *first;
    /* 0x28 */ struct MotionNode *second;
} MotionNode;
CHECK_OFFSETOF(MotionNode, second, 0x28);

/* The values of a blend that a player keeps (partial layout: the size is not known; it comes from
 * the functions that set them). stMtnRequestBlendRatio writes a ratio only when its kind is 1 or
 * 2. */
typedef struct MotionBlend {
    /* 0x00 */ unsigned char unknown00[4];
    /* 0x04 */ int kindFirst;
    /* 0x08 */ int kindSecond;
    /* 0x0C */ float ratioFirst;
    /* 0x10 */ float ratioSecond;
    /* 0x14 */ unsigned char unknown14[4];
    /* 0x18 */ float frames;
    /* 0x1C */ float speed;
} MotionBlend;
CHECK_OFFSETOF(MotionBlend, speed, 0x1C);

/* The data that a track node holds after its header. Its layout depends on the node; the
 * callbacks of the node know it. */
struct MotionTrackData;

/* A node of the chain of tracks of a player: two callbacks, the link to the next node, and the
 * data of the track, which follows the node. fn_804188A0 runs the first callback of every node,
 * and the second one as well when the player has MOTION_PLAYER_FLAG_8. */
typedef struct MotionTrackNode {
    /* 0x00 */ void (*first)(struct MotionTrackData *data);
    /* 0x04 */ void (*second)(struct MotionTrackData *data);
    /* 0x08 */ struct MotionTrackNode *next;
    /* 0x0C */ unsigned int unknown0C;
} MotionTrackNode;
CHECK_SIZEOF(MotionTrackNode, 0x10);

/* A player: the state of one motion in a control. */
typedef struct MotionPlayer {
    /* 0x00 */ unsigned char unknown00[0x90];
    /* 0x90 */ unsigned int flags;
    /* 0x94 */ unsigned int stateFlags;
    /* 0x98 */ unsigned char unknown98[8];
    /* 0xA0 */ MotionRange range;
    /* 0xB0 */ unsigned char unknownB0[0x50];
    /* 0x100 */ MotionRange requestedRange; /* stored by the stMtnRequest frame and speed setters */
    /* 0x110 */ unsigned short unknown110;
    /* 0x112 */ unsigned char unknown112[0xE];
    /* 0x120 */ MotionBlend *blend;
    /* 0x124 */ MotionTrackNode *tracks;
    /* 0x128 */ unsigned short motionId;
    /* 0x12A */ unsigned char unknown12A[2];
    /* 0x12C */ float unknown12C;
} MotionPlayer;
CHECK_SIZEOF(MotionPlayer, 0x130);

/* The control (partial layout: the size is not known). The original names it _stMtnPlayWork: the
 * code at 0x80160DA4 wraps the result of fn_8041B908 in a shared_ptr with the deleter fn_8041B884,
 * and the RTTI of that shared_ptr's count names
 * "boost::detail::sp_counted_base_impl<_stMtnPlayWork *, void (*)(_stMtnPlayWork *)>";
 * fn_8041B980 reports "Cannot get PlayWork.\n". Its transforms and vectors are read by the
 * functions that take a control, from fn_80415BDC to fn_80415E34. */
typedef struct _stMtnPlayWork {
    /* 0x00 */ int transformCount;
    /* 0x04 */ MotionTransform *transforms;
    /* 0x08 */ unsigned char unknown08[4];
    /* 0x0C */ MotionNode *rootNode;
    /* 0x10 */ MotionPlayer *players;
    /* 0x14 */ unsigned int flags;
    /* 0x18 */ unsigned char unknown18[8];
    /* 0x20 */ int playerCount;
    /* 0x24 */ int *listedIndices; /* indices of players, ended by -1 (meaning inferred) */
    /* 0x28 */ int *playerIndexOfId;
    /* 0x2C */ int *unknown2C;
    /* 0x30 */ unsigned char unknown30[0x60];
    /* 0x90 */ Fn8040Matrix34 matrix;
    /* 0xC0 */ unsigned char unknownC0[0x30];
    /* 0xF0 */ MotionWorkVector vectors[7]; /* at least 7: index 6 is the highest used */
} _stMtnPlayWork;
CHECK_OFFSETOF(_stMtnPlayWork, playerIndexOfId, 0x28);
CHECK_OFFSETOF(_stMtnPlayWork, matrix, 0x90);
CHECK_OFFSETOF(_stMtnPlayWork, vectors, 0xF0);

/* The data of the tracks that apply a scale (fn_8041E420, fn_8041FD0C), a translation
 * (fn_8041E5E0, fn_8041FED0) or a rotation (fn_8041FD70) to the matrix on top of the matrix
 * stack. */
typedef struct MotionScaleStep {
    /* 0x00 */ const Fn8040Vector3 *scale;
} MotionScaleStep;
CHECK_SIZEOF(MotionScaleStep, 0x4);

typedef struct MotionRotationStep {
    /* 0x00 */ const Fn8040Vector3 *rotation;
} MotionRotationStep;
CHECK_SIZEOF(MotionRotationStep, 0x4);

typedef struct MotionOffsetStep {
    /* 0x00 */ const Fn8040Vector3 *offset;
} MotionOffsetStep;
CHECK_SIZEOF(MotionOffsetStep, 0x4);

/* Selected by fn_804188A0 from the direction of the playback: the evaluators of a curve cursor
 * (two that return a float and one that returns two). */
typedef float (*MotionCurveEvaluator)(MotionKeyCursor *cursor);
typedef void (*MotionCurveBothEvaluator)(float *value, float *slope, MotionKeyCursor *cursor);

/* Three curves, one for each component of a vector, and the vector that they are evaluated
 * into. */
typedef struct MotionVectorTrack {
    /* 0x00 */ Fn8040Vector3 value;
    /* 0x0C */ float unknown0C;
    /* 0x10 */ MotionKeyCursor curves[3];
} MotionVectorTrack;
CHECK_SIZEOF(MotionVectorTrack, 0xD0);

/* The options of a vector track are a node of the pack (see MotionNode). Bits 0x1, 0x2 and 0x4 of
 * its values[3] negate the x, y and z of the vector; bits 0x8, 0x10 and 0x20 reflect them about
 * rows[1][0], rows[1][1] and rows[1][2]. */
#define MOTION_AXIS_NEGATE_X 0x1
#define MOTION_AXIS_NEGATE_Y 0x2
#define MOTION_AXIS_NEGATE_Z 0x4
#define MOTION_AXIS_REFLECT_X 0x8
#define MOTION_AXIS_REFLECT_Y 0x10
#define MOTION_AXIS_REFLECT_Z 0x20

/* Three tracks keep the options, at 0x10 or 0x14; two of them have curves that use them. The track
 * creator fn_8041C854 stores there the pack node whose values[1] selects the tracks it creates,
 * and steps 0xF0 bytes past either curve track node (the node and 0xE0 bytes of data). */

/* The track of fn_804204C8 and fn_804202B8 (name after fn_804204C8): a factor at 0x10 that
 * scales the vector when it is positive, and the options that fn_804202B8 negates the vector by at
 * 0x14. */
typedef struct MotionScaledVectorTrack {
    /* 0x00 */ Fn8040Vector3 value;
    /* 0x0C */ float unknown0C;
    /* 0x10 */ float scale;
    /* 0x14 */ PackNode *options;
    /* 0x18 */ unsigned char unknown18[8];
    /* 0x20 */ MotionKeyCursor curves[3];
} MotionScaledVectorTrack;
CHECK_OFFSETOF(MotionScaledVectorTrack, options, 0x14);
CHECK_SIZEOF(MotionScaledVectorTrack, 0xE0);

/* The track of fn_8042043C and fn_80420158 (name after fn_80420158): the options that fn_80420158
 * reflects the vector by at 0x10. fn_8041C854 also stores the options at 0x10 for the track of the
 * empty fn_80420438 and fn_8041FF08. */
typedef struct MotionReflectedVectorTrack {
    /* 0x00 */ Fn8040Vector3 value;
    /* 0x0C */ float unknown0C;
    /* 0x10 */ PackNode *options;
    /* 0x14 */ unsigned char unknown14[0xC];
    /* 0x20 */ MotionKeyCursor curves[3];
} MotionReflectedVectorTrack;
CHECK_OFFSETOF(MotionReflectedVectorTrack, options, 0x10);
CHECK_SIZEOF(MotionReflectedVectorTrack, 0xE0);

#ifdef __cplusplus
extern "C" {
#endif

/* The factor that fn_80415854 and fn_80415A18 apply to the speed of a range, set by fn_804157E8. */
extern float lbl_805EDD60;
extern MotionCurveEvaluator lbl_805F13D0;
extern MotionCurveEvaluator lbl_805F13D4;
extern MotionCurveBothEvaluator lbl_805F13D8;
/* The players that fn_80415A18 and fn_80415854 acted on last (see Game/fn_804157E8.c). */
extern MotionPlayer *lbl_805F13DC;
extern MotionPlayer *lbl_805F13E0;

/* The prototypes are inferred from the callers. */
void fn_80418044(_stMtnPlayWork *control);
void fn_804175C8(_stMtnPlayWork *control);
void fn_804188A0(_stMtnPlayWork *control);
void fn_80418A4C(_stMtnPlayWork *control);
void fn_804192EC(_stMtnPlayWork *control);
void fn_80419F5C(_stMtnPlayWork *control);
void fn_8041AB3C(_stMtnPlayWork *control, int index);
void fn_8041AFCC(MotionNode *node, int playerIndex);
_stMtnPlayWork *fn_8041B980(PackEntryInfo *info, int arg1);
int fn_8041CFB0(MotionNode *node, int playerIndex, unsigned int nodeId, float arg3, float arg4);
int fn_8041D1A0(MotionNode *node, unsigned int nodeId, int arg2, int arg3);
MotionTransform *fn_8041D31C(MotionNode *node, unsigned int nodeId);

/* The functions that fn_8042062C calls, defined in Game/fn_80420724.cpp (behaviour inferred): the
 * rotation of a matrix as a quaternion, the interpolation of two quaternions (t = 0 gives b and
 * t = 1 gives a; it always returns 1, which the caller ignores), and the rotation matrix of a
 * quaternion. */
void fn_804208EC(Fn8040Vector4 *quaternion, const Fn8040Matrix34 *matrix);
int fn_80420724(Fn8040Vector4 *result, const Fn8040Vector4 *a, const Fn8040Vector4 *b, float t);
void fn_80420B60(Fn8040Matrix34 *matrix, const Fn8040Vector4 *quaternion);

/* stMtnRequestBlendFrame, stMtnRequestFrameSpeed, stMtnRequestCurrentFrame and fn_80415D40 are
 * also declared in Game/GadgetModelInfo.h for the C++ callers, with UnkMotionControl_801602B4 for
 * the control. Both declarations name the same _stMtnPlayWork type (fn_80160C90 builds that
 * shared_ptr at 0x80160DA4 and destroys it with the destructor of
 * shared_ptr<UnkMotionControl_801602B4>), but that class keeps its placeholder name for now, so a
 * unit cannot include both headers. */
void stMtnRequestBlendRatio(_stMtnPlayWork *control, int id, float ratio);
void stMtnRequestBlendSpeed(_stMtnPlayWork *control, int id, float speed);
void stMtnRequestBlendFrame(_stMtnPlayWork *control, int id, float frames);
void stMtnRequestFrameSpeed(_stMtnPlayWork *control, int id, float speed);
void stMtnRequestCurrentFrame(_stMtnPlayWork *control, int id, float frame);
void stMtnRequestEndFrame(_stMtnPlayWork *control, int id, float frame);
void stMtnRequestStartFrame(_stMtnPlayWork *control, int id, float frame);

float fn_804157E8(float scale);
int fn_80415808(_stMtnPlayWork *control, unsigned int nodeId, int arg2, int arg3);
int fn_8041583C(_stMtnPlayWork *control);
int fn_80415854(_stMtnPlayWork *control, int id, int arg2, int arg3, int repeats);
int fn_80415A18(_stMtnPlayWork *control, int id, int arg2, int arg3, int repeats);
void fn_80415BDC(_stMtnPlayWork *control, const Fn8040Vector3 *offset);
unsigned short fn_80415CDC(_stMtnPlayWork *control, int id);
void fn_80415D40(_stMtnPlayWork *control, int id, MotionRange *range);
void fn_80415DBC(_stMtnPlayWork *control, Fn8040Vector3 *vector);
void fn_80415DF8(_stMtnPlayWork *control, Fn8040Vector3 *vector);
void fn_80415E34(_stMtnPlayWork *control, Fn8040Vector3 *vector);
MotionTransform *fn_80415E70(_stMtnPlayWork *control, unsigned int nodeId);
void fn_80415EA4(_stMtnPlayWork *control);
void fn_80415F98(_stMtnPlayWork *control, int id, unsigned int nodeId, float arg3, float arg4);

void fn_804172D8(_stMtnPlayWork *control, const Fn8040Matrix34 *matrix);
void fn_8041734C(_stMtnPlayWork *control, const Fn8040Matrix34 *matrix);
void fn_804173A8(_stMtnPlayWork *control, const Fn8040Vector3 *position,
                 const Fn8040Vector3 *rotation, int rotationOrder);

void fn_8041AED0(_stMtnPlayWork *control, int playerIndex);
void fn_8041AF1C(MotionNode *node, int playerIndex);
void fn_8041B884(_stMtnPlayWork *control);
_stMtnPlayWork *fn_8041B908(unsigned int id, int arg1);
void fn_8041CC18(MotionNode *node, int playerIndex);
void fn_8041CD6C(MotionNode *node, int playerIndex);

void fn_8041E420(MotionScaleStep *step);
void fn_8041E5E0(MotionOffsetStep *step);
void fn_8041FD0C(MotionScaleStep *step);
void fn_8041FD70(MotionRotationStep *step);
void fn_8041FED0(MotionOffsetStep *step);
/* The empty callbacks sit in the track slots, which pass the track data; they ignore it. */
void fn_8041FD6C(struct MotionTrackData *data);
void fn_8041FF08(struct MotionTrackData *data);
void fn_80420158(MotionReflectedVectorTrack *track);
void fn_804202B8(MotionScaledVectorTrack *track);
void fn_80420308(MotionVectorTrack *track);
void fn_804203AC(MotionVectorTrack *track);
void fn_80420438(struct MotionTrackData *data);
void fn_8042043C(MotionReflectedVectorTrack *track);
void fn_804204C8(MotionScaledVectorTrack *track);
/* Blends two matrices (behaviour inferred): t = 0 gives b and t = 1 gives a. */
void fn_80420590(Fn8040Matrix34 *result, const Fn8040Matrix34 *a, const Fn8040Matrix34 *b, float t);
void fn_8042062C(Fn8040Matrix34 *result, const Fn8040Matrix34 *a, const Fn8040Matrix34 *b, float t);

#ifdef __cplusplus
}
#endif

/* The player of an id, or null when the id names no player. */
static inline MotionPlayer *FindPlayer(_stMtnPlayWork *control, int id) {
    int index = control->playerIndexOfId[id];

    if (index == -1) {
        return 0;
    }
    return &control->players[index];
}

/* The matrix on top of the current matrix stack. */
static inline Fn8040Matrix34 *TopMatrix(void) {
    return &lbl_805F13A8->matrices[lbl_805F13A8->top];
}

#endif
