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
 * static initializer __sinit_fn_8042043C_cpp. All 100 functions have source. The file is kept as
 * consecutive units
 * with one source file each: Game/fn_804157E8.cpp, fn_8041583C.c, fn_80415854.cpp, fn_80416198.cpp,
 * fn_8041FD6C.c, fn_8041FD70.cpp, fn_8041FF08.c, fn_8041FF0C.cpp, fn_804202B8.c, fn_80420308.cpp,
 * fn_80420438.c and fn_8042043C.cpp, so that the units that already match keep their own objects.
 *
 * The control's record is _stMtnPlayWork (the original name, see below; the code calls it the
 * control); the stMtn function names are original symbols, and every other name here is inferred
 * from the code. A control owns an array of players, each of which plays one motion, and the tree
 * of nodes that the motions move. Players are addressed by an id: a table maps an id to the index
 * of its player, or to -1 when the id names no player. */

/* Bits of MotionPlayer.stateFlags. The stMtnRequest frame and speed setters and the motion
 * requests set a bit for the value they store in the player (MOTION_PLAYER_REQUESTED first, then
 * the bit of the value). */
#define MOTION_PLAYER_REQUESTED 0x1
#define MOTION_PLAYER_REQUEST_START_FRAME 0x8
#define MOTION_PLAYER_REQUEST_END_FRAME 0x10
#define MOTION_PLAYER_REQUEST_CURRENT_FRAME 0x20
#define MOTION_PLAYER_REQUEST_SPEED 0x40
/* Set by stMtnRequestMotion and fn_80417568, which store the motion id in
 * MotionPlayer.requestedMotionId (name inferred); stMtnRequestMotion also sets
 * MOTION_PLAYER_STATE_8000 (meaning unknown). */
#define MOTION_PLAYER_REQUEST_MOTION 0x100
/* Makes stMtnRequestBlendRatio try the first ratio of the blend (it falls back to the second when
 * the first kind is not 1 or 2). */
#define MOTION_PLAYER_BLEND_FIRST 0x200
/* Additional state bits used by the request handlers (original names unknown). */
#define MOTION_PLAYER_STATE_2 0x2
#define MOTION_PLAYER_STATE_4 0x4

/* The bits that stMtnControlWorkFlag reads or changes (meaning unknown): 0x400 is its flag 2, 0x800
 * is changed by its flag 7, and 0x1000 and 0x4000 are set when its flags 1 and 0 change a bit of
 * MotionPlayer.flags. */
#define MOTION_PLAYER_STATE_400 0x400
#define MOTION_PLAYER_STATE_800 0x800
#define MOTION_PLAYER_STATE_1000 0x1000
#define MOTION_PLAYER_STATE_4000 0x4000
/* Set by fn_80415F98 together with MOTION_PLAYER_REQUESTED after fn_8041CFB0 (meaning unknown). */
#define MOTION_PLAYER_STATE_2000 0x2000
/* Set by stMtnRequestMotion (see MOTION_PLAYER_REQUEST_MOTION). */
#define MOTION_PLAYER_STATE_8000 0x8000

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
/* Additional player flag bits (original names unknown). */
#define MOTION_PLAYER_FLAG_10 0x10
#define MOTION_PLAYER_FLAG_10000 0x10000
#define MOTION_PLAYER_FLAG_40000 0x40000
#define MOTION_PLAYER_FLAG_80000 0x80000
#define MOTION_PLAYER_FLAG_100000 0x100000
#define MOTION_PLAYER_FLAG_200000 0x200000
#define MOTION_PLAYER_FLAG_800000 0x800000
#define MOTION_PLAYER_FLAG_1000000 0x1000000
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
 * the amount by which they then grow the speed (one .sdata2 constant, 0.0001f, serves both).
 * fn_8041AD94 compares speeds, and the product of two speeds, with the same value: a speed below it
 * counts as not moving forwards and one above its negation as not moving backwards (inferred). */
#define MOTION_LIMIT_EPSILON 0.0001f

/* A vector followed by a word that this code does not use (only the 0x10 stride is known). Most
 * entries of _stMtnPlayWork.vectors are positions, but entry 1 holds the angles that fn_804173A8
 * builds the matrix from. */
typedef struct MotionWorkVector {
    /* 0x00 */ Fn8040Vector3 value;
    /* 0x0C */ float unknown0C;
} MotionWorkVector;
CHECK_SIZEOF(MotionWorkVector, 0x10);

/* The step callbacks receive the second matrix and its scale together (name inferred). */
typedef struct MotionTransformState {
    /* 0x00 */ Fn8040Matrix34 matrix;
    /* 0x30 */ MotionWorkVector unknown30;
} MotionTransformState;
CHECK_SIZEOF(MotionTransformState, 0x40);

/* A transform of a control: the traversals write both matrices and the scale of the second. */
typedef struct MotionTransform {
    /* 0x00 */ Fn8040Matrix34 matrix;
    /* 0x30 */ MotionTransformState state;
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

/* The node callback receives the current matrix, its registered argument, and the pack node ID
 * (name inferred from the call in fn_804186A0). */
typedef struct MotionNodeCallbackData {
    /* 0x00 */ Fn8040Matrix34 *matrix;
    /* 0x04 */ void *argument;
    /* 0x08 */ unsigned int nodeId;
} MotionNodeCallbackData;
CHECK_SIZEOF(MotionNodeCallbackData, 0xC);

typedef void (*MotionNodeCallback)(MotionNodeCallbackData *data);

/* A node of the tree of a control: the functions that
 * search or walk the tree follow its two child links recursively. Its definition is the node of
 * the pack that the tree is built from (fn_8041B980 starts at the root node of the pack entry and
 * fn_8041BE30 stores each pack node here while it follows the links of the pack nodes); the
 * functions here read its id, and fn_80415BDC compares values[0] with 1 and tests the low three
 * bits of values[1] (MOTION_NODE_SCALED_TRACK). The flags of the node select its steps (see
 * MotionNodeStep), with the bits of values[1] that select the tracks. */
typedef struct MotionNode {
    /* 0x00 */ unsigned int flags;
    /* 0x04 */ unsigned char unknown04[4];
    /* 0x08 */ struct MotionNodeStep *steps;
    /* 0x0C */ PackNode *definition;
    /* 0x10 */ MotionTransform *transform;
    /* 0x14 */ struct MotionBlend *blends;
    /* 0x18 */ MotionNodeEntry *entries;
    /* 0x1C */ MotionNodeCallback callback;
    /* 0x20 */ void *callbackArgument;
    /* 0x24 */ struct MotionNode *first;
    /* 0x28 */ struct MotionNode *second;
    /* 0x2C */ unsigned char unknown2C[8];
} MotionNode;
CHECK_OFFSETOF(MotionNode, second, 0x28);
CHECK_SIZEOF(MotionNode, 0x34);

#define MOTION_BLEND_FLAG_1 0x1
#define MOTION_BLEND_FLAG_2 0x2

/* The values of a blend that a player keeps. fn_8041B980 allocates 0x2C bytes for each player.
 * stMtnRequestBlendRatio writes a ratio only when its kind is 1 or 2. */
typedef struct MotionBlend {
    /* 0x00 */ unsigned int flags;
    /* 0x04 */ int kindFirst;
    /* 0x08 */ int kindSecond;
    /* 0x0C */ float ratioFirst;
    /* 0x10 */ float ratioSecond;
    /* 0x14 */ unsigned char unknown14[4];
    /* 0x18 */ float frames;
    /* 0x1C */ float speed;
    /* 0x20 */ float unknown20;
    /* 0x24 */ unsigned char unknown24[8];
} MotionBlend;
CHECK_OFFSETOF(MotionBlend, speed, 0x1C);
CHECK_SIZEOF(MotionBlend, 0x2C);

/* A node of the chain of tracks of a player: two callbacks, the link to the next node, and the
 * data of the track, which follows the node. fn_804188A0 runs the first callback of every node,
 * and the second one as well when the player has MOTION_PLAYER_FLAG_8. */
typedef struct MotionTrackNode {
    /* 0x00 */ void (*first)(void *data);
    /* 0x04 */ void (*second)(void *data);
    /* 0x08 */ struct MotionTrackNode *next;
    /* 0x0C */ unsigned int unknown0C;
} MotionTrackNode;
CHECK_SIZEOF(MotionTrackNode, 0x10);

/* A player: the state of one motion in a control. */
typedef struct MotionPlayer {
    /* 0x00 */ Fn8040Matrix34 unknown00;
    /* 0x30 */ Fn8040Matrix34 unknown30;
    /* 0x60 */ Fn8040Matrix34 unknown60;
    /* 0x90 */ unsigned int flags;
    /* 0x94 */ unsigned int stateFlags;
    /* 0x98 */ float firstFrame;
    /* 0x9C */ float lastFrame;
    /* 0xA0 */ MotionRange range;
    /* 0xB0 */ MotionWorkVector unknownB0;
    /* 0xC0 */ MotionWorkVector unknownC0;
    /* 0xD0 */ MotionWorkVector unknownD0;
    /* 0xE0 */ MotionWorkVector unknownE0;
    /* 0xF0 */ MotionWorkVector unknownF0;
    /* 0x100 */ MotionRange requestedRange; /* stored by the stMtnRequest frame and speed setters */
    /* 0x110 */ unsigned short requestedMotionId;
    /* 0x112 */ unsigned char unknown112[0xE];
    /* 0x120 */ MotionBlend *blend;
    /* 0x124 */ MotionTrackNode *tracks;
    /* 0x128 */ unsigned short motionId;
    /* 0x12A */ unsigned char unknown12A[2];
    /* 0x12C */ float unknown12C;
} MotionPlayer;
CHECK_SIZEOF(MotionPlayer, 0x130);

/* The step callbacks receive this block at control+0x20. The list and its two index maps are
 * shared by every step of the control (name inferred). */
typedef struct MotionPlayerList {
    /* 0x00 */ int count;
    /* 0x04 */ int *listedIndices;
    /* 0x08 */ int *playerIndexOfId;
    /* 0x0C */ int *unknown0C;
} MotionPlayerList;
CHECK_SIZEOF(MotionPlayerList, 0x10);

/* The control. The original names it _stMtnPlayWork: the
 * code at 0x80160DA4 wraps the result of fn_8041B908 in a shared_ptr with the deleter fn_8041B884,
 * and the RTTI of that shared_ptr's count names
 * "boost::detail::sp_counted_base_impl<_stMtnPlayWork *, void (*)(_stMtnPlayWork *)>";
 * fn_8041B980 reports "Cannot get PlayWork.\n". Its transforms and vectors are read by the
 * functions that take a control, from fn_80415BDC to fn_80415E34. */
typedef struct _stMtnPlayWork {
    /* 0x00 */ int transformCount;
    /* 0x04 */ MotionTransform *transforms;
    /* 0x08 */ PackEntryInfo *info;
    /* 0x0C */ MotionNode *rootNode;
    /* 0x10 */ MotionPlayer *players;
    /* 0x14 */ unsigned int flags;
    /* 0x18 */ unsigned char unknown18[8];
    /* 0x20 */ MotionPlayerList playerList;
    /* 0x30 */ Fn8040Matrix34 unknown30;
    /* 0x60 */ Fn8040Matrix34 unknown60;
    /* 0x90 */ Fn8040Matrix34 matrix;
    /* 0xC0 */ Fn8040Matrix34 unknownC0;
    /* 0xF0 */ MotionWorkVector vectors[7]; /* at least 7: index 6 is the highest used */
} _stMtnPlayWork;
CHECK_SIZEOF(_stMtnPlayWork, 0x160);
CHECK_OFFSETOF(_stMtnPlayWork, playerList, 0x20);
CHECK_OFFSETOF(_stMtnPlayWork, matrix, 0x90);
CHECK_OFFSETOF(_stMtnPlayWork, vectors, 0xF0);

/* The fixed matrix-step payload: a pointer to its scale, translation or rotation vector. */
typedef struct MotionStaticStepData {
    /* 0x00 */ const Fn8040Vector3 *value;
} MotionStaticStepData;
CHECK_SIZEOF(MotionStaticStepData, 0x4);

/* A full turn in the 16-bit angle units that the rotation steps fn_8041E480 and fn_8041FD70
 * truncate their angles to, and in radians. */
#define MOTION_ANGLE_UNITS_PER_TURN 65536.0f
#define MOTION_RADIANS_PER_TURN 6.2831855f

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

/* The bits of values[1] for which fn_8041C854 creates the track of fn_804204C8 and fn_804202B8
 * (MotionScaledVectorTrack) and the track of fn_8042043C and fn_80420158
 * (MotionReflectedVectorTrack). Bit 0x40 (MOTION_NODE_TRACK_40) takes precedence over the
 * reflected-track bits: it selects the track of the empty fn_80420438 and fn_8041FF08 instead. The
 * groups 0x400 and 0x380 select further steps of a node (see MotionNodeStep; meaning unknown). */
#define MOTION_NODE_SCALED_TRACK 0x7
#define MOTION_NODE_REFLECTED_TRACK 0x38
#define MOTION_NODE_TRACK_40 0x40
#define MOTION_NODE_TRACK_380 0x380
#define MOTION_NODE_TRACK_400 0x400

/* The mirror of a node (names inferred): for a node with bit 0x8000 of values[3], fn_8041C2E0
 * copies the name of the node, replaces the "l" of its first "_l" with "r" and looks up the node of
 * that name. In the node's scaled track, and in its reflected track unless either node has bit
 * 0x40 of values[1], it stores the mirror node (mirrorOptions) and the mirror's corresponding track
 * node (mirrorTrackNode, whose data follows it), installs fn_804201C0 (scaled) or fn_8041FF0C
 * (reflected) as the second callback of the track and clears the second callback of the mirror's
 * track node. Those callbacks use the mirror's track only when the mirror node has a track of the
 * same kind. */

/* The track of fn_804204C8 and fn_804202B8 (name after fn_804204C8): a factor at 0x10 that
 * scales the vector when it is positive, and the options that fn_804202B8 negates the vector by at
 * 0x14. */
typedef struct MotionScaledVectorTrack {
    /* 0x00 */ Fn8040Vector3 value;
    /* 0x0C */ float unknown0C;
    /* 0x10 */ float scale;
    /* 0x14 */ PackNode *options;
    /* 0x18 */ PackNode *mirrorOptions;
    /* 0x1C */ MotionTrackNode *mirrorTrackNode;
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
    /* 0x14 */ PackNode *mirrorOptions;
    /* 0x18 */ MotionTrackNode *mirrorTrackNode;
    /* 0x1C */ unsigned char unknown1C[4];
    /* 0x20 */ MotionKeyCursor curves[3];
} MotionReflectedVectorTrack;
CHECK_OFFSETOF(MotionReflectedVectorTrack, options, 0x10);
CHECK_SIZEOF(MotionReflectedVectorTrack, 0xE0);

/* An access view (names inferred) of the data of a track in a step: which member applies depends
 * on the step (see MotionNodeStep). */
typedef union MotionStepTrackData {
    MotionVectorTrack *vector;
    MotionScaledVectorTrack *scaled;
    MotionReflectedVectorTrack *reflected;
} MotionStepTrackData;
CHECK_SIZEOF(MotionStepTrackData, 0x4);

/* The track of a player in a step of a node: its data, whose kind depends on the step, and the
 * player's blend state. */
typedef struct MotionStepTrack {
    /* 0x00 */ MotionStepTrackData data;
    /* 0x04 */ MotionBlend *state;
} MotionStepTrack;
CHECK_SIZEOF(MotionStepTrack, 0x8);

/* The data of a step of a node, after its header: for a step with tracks, two words and the tracks
 * (see MotionNodeStep); for a step without, a MotionStaticStepData. Its layout depends on the step; the callback of the step knows it. */
typedef struct MotionDynamicStepData {
    /* 0x00 */ MotionPlayerList *playerList;
    /* 0x04 */ MotionTransformState *state;
    /* 0x08 */ MotionStepTrack tracks[1];
} MotionDynamicStepData;
CHECK_SIZEOF(MotionDynamicStepData, 0x10);

/* A step of a node (partial layout, names inferred): fn_804186A0 runs the callback of every step
 * of a node on the data that follows the header of the step, at 0x08. The flags of the node say
 * which steps the node has, in this order: a step for MOTION_NODE_TRACK_400, then always three
 * steps for the bits MOTION_NODE_SCALED_TRACK, MOTION_NODE_TRACK_40 or MOTION_NODE_REFLECTED_TRACK,
 * and MOTION_NODE_TRACK_380. A step selected by its bits has one track for each player at 0x10: a
 * MotionScaledVectorTrack for MOTION_NODE_SCALED_TRACK, a MotionReflectedVectorTrack for
 * MOTION_NODE_TRACK_40 or MOTION_NODE_REFLECTED_TRACK and a MotionVectorTrack for
 * MOTION_NODE_TRACK_400 and MOTION_NODE_TRACK_380. fn_8041AFCC and fn_8041B6FC skip the step of
 * MOTION_NODE_TRACK_40. */
typedef struct MotionNodeStep {
    /* 0x00 */ void (*apply)(void *data);
    /* 0x04 */ struct MotionNodeStep *next;
    /* 0x08 */ MotionDynamicStepData data;
} MotionNodeStep;
CHECK_OFFSETOF(MotionNodeStep, data, 0x08);
/* A view with one track: a step has one track for each player. */
CHECK_SIZEOF(MotionNodeStep, 0x18);

/* The two-word header is shared by fixed and per-player steps. Fixed steps store one pack-row
 * pointer; dynamic steps store the player list, second matrix, and one track per player. */
typedef struct MotionStaticStep {
    /* 0x00 */ void (*apply)(void *data);
    /* 0x04 */ MotionNodeStep *next;
    /* 0x08 */ MotionStaticStepData data;
} MotionStaticStep;
CHECK_SIZEOF(MotionStaticStep, 0xC);

/* Only the translation step of a type-4 pack node has this trailer, after its player tracks. */
typedef struct MotionStepExtra {
    /* 0x00 */ MotionTransform *transform;
    /* 0x04 */ const float *first;
    /* 0x08 */ const float *second;
} MotionStepExtra;
CHECK_SIZEOF(MotionStepExtra, 0xC);

typedef struct MotionVectorTrackStorage {
    /* 0x00 */ MotionTrackNode node;
    /* 0x10 */ MotionVectorTrack data;
} MotionVectorTrackStorage;
CHECK_SIZEOF(MotionVectorTrackStorage, 0xE0);

typedef struct MotionScaledTrackStorage {
    /* 0x00 */ MotionTrackNode node;
    /* 0x10 */ MotionScaledVectorTrack data;
} MotionScaledTrackStorage;
CHECK_SIZEOF(MotionScaledTrackStorage, 0xF0);

typedef struct MotionReflectedTrackStorage {
    /* 0x00 */ MotionTrackNode node;
    /* 0x10 */ MotionReflectedVectorTrack data;
} MotionReflectedTrackStorage;
CHECK_SIZEOF(MotionReflectedTrackStorage, 0xF0);

#ifdef __cplusplus
extern "C" {
#endif

/* The factor that fn_80415854 and fn_80415A18 apply to the speed of a range, set by fn_804157E8. */
extern float lbl_805EDD60;
/* Common-source aliases select the regional symbols; these names are inferred. */
#if defined(VERSION_GUPJ8P)
#define motionControlCurveEvaluator0 lbl_805F19E0
#define motionControlCurveEvaluator1 lbl_805F19E4
#define motionControlCurveBothEvaluator lbl_805F19E8
#define motionControlLastPlayer0 lbl_805F19EC
#define motionControlLastPlayer1 lbl_805F19F0
#elif defined(VERSION_GUPP8P)
#define motionControlCurveEvaluator0 lbl_805F2478
#define motionControlCurveEvaluator1 lbl_805F247C
#define motionControlCurveBothEvaluator lbl_805F2480
#define motionControlLastPlayer0 lbl_805F2484
#define motionControlLastPlayer1 lbl_805F2488
#else
#define motionControlCurveEvaluator0 lbl_805F13D0
#define motionControlCurveEvaluator1 lbl_805F13D4
#define motionControlCurveBothEvaluator lbl_805F13D8
#define motionControlLastPlayer0 lbl_805F13DC
#define motionControlLastPlayer1 lbl_805F13E0
#endif
extern MotionCurveEvaluator motionControlCurveEvaluator0;
extern MotionCurveEvaluator motionControlCurveEvaluator1;
extern MotionCurveBothEvaluator motionControlCurveBothEvaluator;
/* The players that fn_80415A18 and fn_80415854 acted on last (see Game/fn_804157E8.cpp). */
extern MotionPlayer *motionControlLastPlayer0;
extern MotionPlayer *motionControlLastPlayer1;

/* Work arrays used while combining the players' transforms. The initializer constructs the
 * two matrix arrays and the vector array; the traversal functions use the index flags and save or
 * replace existing ratios. */
extern Fn8040Matrix34 lbl_805E12E0[5];
extern Fn8040Matrix34 lbl_805E13D0[4];
extern Fn8040Vector3 lbl_805E1490[4];
extern int lbl_805E14C0[4];
extern int lbl_805E14D0[4];
extern int lbl_805E14E0[4];
extern float lbl_805E14F0[4];

/* The prototypes are inferred from the callers. */
void fn_80418044(_stMtnPlayWork *control);
void fn_804186A0(_stMtnPlayWork *control, MotionNode *node);
void fn_804175C8(_stMtnPlayWork *control);
void fn_804188A0(_stMtnPlayWork *control);
void fn_80418A4C(_stMtnPlayWork *control);
void fn_804192EC(_stMtnPlayWork *control);
void fn_804194B8(MotionNode *node);
void fn_8041966C(MotionNode *node);
void fn_804198E4(_stMtnPlayWork *control, int playerIndex);
void fn_80419BA8(Fn8040Vector3 *result, MotionPlayer *player, MotionRange *range, int reset);
void fn_80419D5C(_stMtnPlayWork *control, int playerIndex);
void fn_80419F5C(_stMtnPlayWork *control);
void fn_8041A440(_stMtnPlayWork *control);
void fn_8041A83C(_stMtnPlayWork *control, MotionNode *node);
void fn_8041AB3C(_stMtnPlayWork *control, int index);
void fn_8041AFCC(MotionNode *node, int playerIndex);
void fn_8041B6FC(MotionNode *node, int playerIndex);
_stMtnPlayWork *fn_8041B980(PackEntryInfo *info, int arg1);
void fn_8041BE30(_stMtnPlayWork *control, MotionNode **nextNode, MotionBlend *blends,
                 MotionNodeEntry **nextEntries, MotionTransform **nextTransform,
                 MotionTrackNode **tracks, PackNode *definition);
void fn_8041C2E0(PackNode *node, PackNode *root, MotionTrackNode **track,
                 MotionTrackNode *firstTrack);
PackNode *fn_8041C5A8(PackNode *node, MotionTrackNode **track, unsigned int id);
void fn_8041C854(MotionTrackNode **next, MotionTrackNode **last, PackNode *node);
int fn_8041CFB0(MotionNode *node, int playerIndex, unsigned int nodeId, float arg3, float arg4);
int fn_8041D1A0(MotionNode *node, unsigned int nodeId, MotionNodeCallback callback, void *argument);
MotionTransform *fn_8041D31C(const MotionNode *node, unsigned int nodeId);

/* The functions that fn_8042062C calls, defined in Game/fn_80420724.cpp (behaviour inferred): the
 * rotation of a matrix as a quaternion, the interpolation of two quaternions (t = 0 gives b and
 * t = 1 gives a; it always returns 1, which the caller ignores), and the rotation matrix of a
 * quaternion. */
void fn_804208EC(Fn8040Vector4 *quaternion, const Fn8040Matrix34 *matrix);
int fn_80420724(Fn8040Vector4 *result, const Fn8040Vector4 *a, const Fn8040Vector4 *b, float t);
void fn_80420B60(Fn8040Matrix34 *matrix, const Fn8040Vector4 *quaternion);

int stMtnControlWorkFlag(_stMtnPlayWork *control, int id, int flag, int action);
int stMtnPrepareBlendWithTarget(_stMtnPlayWork *control, int target, int first, int last);
int stMtnAbortBlend(_stMtnPlayWork *control, int id);
void stMtnRequestMotion(_stMtnPlayWork *control, int id, unsigned short motionId);
void fn_80417568(_stMtnPlayWork *control, unsigned short motionId);
void stMtnRequestBlendRatio(_stMtnPlayWork *control, int id, float ratio);
void stMtnRequestBlendSpeed(_stMtnPlayWork *control, int id, float speed);
void stMtnRequestBlendFrame(_stMtnPlayWork *control, int id, float frames);
void stMtnRequestFrameSpeed(_stMtnPlayWork *control, int id, float speed);
void stMtnRequestCurrentFrame(_stMtnPlayWork *control, int id, float frame);
void stMtnRequestEndFrame(_stMtnPlayWork *control, int id, float frame);
void stMtnRequestStartFrame(_stMtnPlayWork *control, int id, float frame);

float fn_804157E8(float scale);
int fn_80415808(_stMtnPlayWork *control, unsigned int nodeId, MotionNodeCallback callback,
                void *argument);
int fn_8041583C(_stMtnPlayWork *control);
int fn_80415854(_stMtnPlayWork *control, int id, unsigned short *eventId, void **payload,
                int repeats);
int fn_80415A18(_stMtnPlayWork *control, int id, unsigned short eventId, void *destination,
                int repeats);
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

int fn_8041AD94(MotionRange *range, MotionRange *request, float firstFrame, float lastFrame);
void fn_8041AED0(_stMtnPlayWork *control, int playerIndex);
void fn_8041AF1C(MotionNode *node, int playerIndex);
int fn_8041B170(_stMtnPlayWork *control, int playerIndex, unsigned short motionId);
void fn_8041B2BC(_stMtnPlayWork *control, PackNode *definition, int playerIndex,
                 unsigned short motionId, float scale);
void fn_8041B884(_stMtnPlayWork *control);
_stMtnPlayWork *fn_8041B908(unsigned int id, int arg1);
void fn_8041CA78(int *trackBytes, int *nodeBytes, int *transformCount, int playerCount,
                 PackNode *node);
/* fn_8041CC18, fn_8041CD6C and fn_8041CE74 walk the tree below (and including) a node and change
 * only the entries of its nodes, not the nodes. fn_8041D448 returns the node of an id in that tree,
 * or null. */
void fn_8041CC18(const MotionNode *node, int playerIndex);
void fn_8041CD6C(const MotionNode *node, int playerIndex);
void fn_8041CE74(const MotionNode *node, int playerIndex);
MotionNode *fn_8041D448(MotionNode *node, unsigned int nodeId);

void fn_8041D580(void *data);
void fn_8041D89C(void *data);
void fn_8041DBD4(void *data);
void fn_8041E124(void *data);
void fn_8041E618(void *data);
void fn_8041E9C4(void *data);
void fn_8041ED8C(void *data);
void fn_8041F290(void *data);
void fn_8041F6F0(void *data);
void fn_8041F7D0(void *data);
void fn_8041FA20(void *data);
void fn_8041FAD8(void *data);
void fn_8041FB4C(void *data);
void fn_8041FCC0(void *data);
void fn_8041E420(void *data);
void fn_8041E480(void *data);
void fn_8041E5E0(void *data);
void fn_8041FD0C(void *data);
void fn_8041FD70(void *data);
void fn_8041FED0(void *data);
/* The empty callbacks ignore their data: fn_8041FD6C is the callback of a step without tracks
 * (fn_8041BE30 installs it in place of fn_8041FD70 for a node whose values[4] is 6), fn_8041FF08
 * and fn_80420438 sit in track slots. */
void fn_8041FD6C(void *data);
void fn_8041FF08(void *data);
void fn_8041FF0C(void *data);
void fn_80420158(void *data);
void fn_804201C0(void *data);
void fn_804202B8(void *data);
void fn_80420308(void *data);
void fn_804203AC(void *data);
void fn_80420438(void *data);
void fn_8042043C(void *data);
void fn_804204C8(void *data);
/* Blends two matrices (behaviour inferred): t = 0 gives b and t = 1 gives a. */
void fn_80420590(Fn8040Matrix34 *result, Fn8040Matrix34 *a, Fn8040Matrix34 *b, float t);
void fn_8042062C(Fn8040Matrix34 *result, const Fn8040Matrix34 *a, const Fn8040Matrix34 *b, float t);

#ifdef __cplusplus
}
#endif

/* The player of an id, or null when the id names no player. */
static inline MotionPlayer *FindPlayer(_stMtnPlayWork *control, int id) {
    int index = control->playerList.playerIndexOfId[id];

    if (index == -1) {
        return 0;
    }
    return &control->players[index];
}

/* The matrix on top of the current matrix stack. */
static inline Fn8040Matrix34 *TopMatrix(void) {
    return &lbl_805F13A8->matrices[lbl_805F13A8->top];
}

/* Negates the components of a vector as its options say (fn_804202B8 and fn_804201C0). */
static inline void NegateVector(Fn8040Vector3 *value, const PackNode *options) {
    if (options->values[3] & MOTION_AXIS_NEGATE_X) {
        value->x = -value->x;
    }
    if (options->values[3] & MOTION_AXIS_NEGATE_Y) {
        value->y = -value->y;
    }
    if (options->values[3] & MOTION_AXIS_NEGATE_Z) {
        value->z = -value->z;
    }
}

#endif
