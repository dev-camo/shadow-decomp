#ifndef FN_804157E8_MOTION_CONTROL_H
#define FN_804157E8_MOTION_CONTROL_H

#include "Game/fn_8040_vector_types.h"
#include "layout_check.h"

/* The motion control of the unit at 0x804157E8 (every name here is inferred from the code). A
 * control owns an array of players, each of which plays one motion, and the tree of nodes that the
 * motions move. Players are addressed by an id: a table maps an id to the index of its player, or
 * to -1 when the id names no player. */

/* Bits of MotionPlayer.stateFlags. */
/* Makes stMtnRequestBlendRatio try the first ratio of the blend (it falls back to the second when
 * the first kind is not 1 or 2). */
#define MOTION_PLAYER_BLEND_FIRST 0x200

/* Bits of MotionPlayer.flags (fn_80415854 and fn_80415A18 read them to prepare a range). */
/* With MOTION_PLAYER_FOLLOW_TARGET: when the frame is within MOTION_LIMIT_EPSILON of the limit the
 * speed moves towards (endFrame for a non-negative speed, startFrame otherwise), the speed grows by
 * that epsilon in its own direction. */
#define MOTION_PLAYER_NUDGE_AT_LIMIT 0x2
/* fn_80415854, fn_80415A18, fn_80415CDC and fn_80415D40 use a player only when this bit is set,
 * and fn_80415EA4 copies the target frame to the current frame only when it is still set after
 * fn_8041AB3C (meaning inferred). */
#define MOTION_PLAYER_ACTIVE 0x4
/* The speed of the range is replaced by 0. */
#define MOTION_PLAYER_ZERO_SPEED 0x20000
/* With MOTION_PLAYER_NUDGE_AT_LIMIT clear, the speed becomes targetFrame - frame. */
#define MOTION_PLAYER_FOLLOW_TARGET 0x400000

/* The distance within which fn_80415854 and fn_80415A18 treat the frame as being at a limit, and
 * the amount by which they then grow the speed (one .sdata2 constant, 0.0001f, serves both). */
#define MOTION_LIMIT_EPSILON 0.0001f

/* The data that fn_80415BDC reads from the definition of a node (partial layout: the size and the
 * meaning of the fields are not known; it compares the halfword at 0x4 with 1 and tests the low
 * three bits of the halfword at 0x6). */
typedef struct MotionNodeData {
    /* 0x00 */ unsigned char unknown00[4];
    /* 0x04 */ unsigned short unknown04;
    /* 0x06 */ unsigned short unknown06;
} MotionNodeData;
CHECK_OFFSETOF(MotionNodeData, unknown06, 0x6);

/* The definition of a node (partial layout: the size is not known). */
typedef struct MotionNodeDefinition {
    /* 0x00 */ unsigned int id;
    /* 0x04 */ unsigned char unknown04[8];
    /* 0x0C */ MotionNodeData *data;
} MotionNodeDefinition;
CHECK_OFFSETOF(MotionNodeDefinition, data, 0xC);

/* The frames and the speed with which a player plays its motion (names inferred from the way the
 * functions compare them with the current frame). fn_80415854 and fn_80415A18 scale the speed by
 * lbl_805EDD60. */
typedef struct MotionRange {
    /* 0x00 */ float startFrame;
    /* 0x04 */ float endFrame;
    /* 0x08 */ float speed;
    /* 0x0C */ float targetFrame;
} MotionRange;
CHECK_SIZEOF(MotionRange, 0x10);

/* A vector followed by a word that this code does not use (only the 0x10 stride is known). */
typedef struct MotionAnchor {
    /* 0x00 */ Fn8040Vector3 position;
    /* 0x0C */ float unknown0C;
} MotionAnchor;
CHECK_SIZEOF(MotionAnchor, 0x10);

/* A transform of a node: a matrix followed by data that is not mapped yet. */
typedef struct MotionTransform {
    /* 0x00 */ Fn8040Matrix34 matrix;
    /* 0x30 */ unsigned char unknown30[0x40];
} MotionTransform;
CHECK_SIZEOF(MotionTransform, 0x70);

/* A node of the tree of a control (partial layout: the size is not known). */
typedef struct MotionNode {
    /* 0x00 */ int transformCount;
    /* 0x04 */ MotionTransform *transforms;
    /* 0x08 */ unsigned char unknown08[4];
    /* 0x0C */ MotionNodeDefinition *definition;
    /* 0x10 */ unsigned char unknown10[0xE0];
    /* 0xF0 */ MotionAnchor anchors[7]; /* at least 7: index 6 is the highest used */
} MotionNode;
CHECK_OFFSETOF(MotionNode, anchors, 0xF0);

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

/* A player: the state of one motion in a control. */
typedef struct MotionPlayer {
    /* 0x00 */ unsigned char unknown00[0x90];
    /* 0x90 */ unsigned int flags;
    /* 0x94 */ unsigned int stateFlags;
    /* 0x98 */ unsigned char unknown98[8];
    /* 0xA0 */ MotionRange range;
    /* 0xB0 */ unsigned char unknownB0[0x70];
    /* 0x120 */ MotionBlend *blend;
    /* 0x124 */ unsigned char unknown124[4];
    /* 0x128 */ unsigned short motionId;
    /* 0x12A */ unsigned char unknown12A[2];
    /* 0x12C */ float frame; /* the current frame */
} MotionPlayer;
CHECK_SIZEOF(MotionPlayer, 0x130);

/* The control (partial layout: the size is not known; a table at 0x2C also belongs to it). */
typedef struct MotionControl {
    /* 0x00 */ unsigned char unknown00[0xC];
    /* 0x0C */ MotionNode *rootNode;
    /* 0x10 */ MotionPlayer *players;
    /* 0x14 */ unsigned int flags;
    /* 0x18 */ unsigned char unknown18[8];
    /* 0x20 */ int playerCount;
    /* 0x24 */ int *listedIndices; /* indices of players, ended by -1 (meaning inferred) */
    /* 0x28 */ int *playerIndexOfId;
} MotionControl;
CHECK_OFFSETOF(MotionControl, playerIndexOfId, 0x28);

#ifdef __cplusplus
extern "C" {
#endif

/* The factor that fn_80415854 and fn_80415A18 apply to the speed of a range, set by fn_804157E8
 * (defined in .sdata with the rest of this file's data until it is split). */
extern float lbl_805EDD60;

/* The prototypes are inferred from the callers. */
void fn_80418044(MotionControl *control);
void fn_804175C8(MotionControl *control);
void fn_804188A0(MotionControl *control);
void fn_80418A4C(MotionControl *control);
void fn_804192EC(MotionControl *control);
void fn_80419F5C(MotionControl *control);
void fn_8041AB3C(MotionControl *control, int index);
int fn_8041CFB0(MotionNode *node, int playerIndex, unsigned int key, float arg3, float arg4);
int fn_8041D1A0(MotionNode *node, unsigned int key, int arg2, int arg3);
int fn_8041D31C(MotionNode *node, unsigned int key);

/* stMtnRequestBlendFrame is also declared in Game/GadgetModelInfo.h for the C++ callers, with
 * UnkMotionControl_801602B4 standing for the control; that class and MotionControl are not unified
 * yet, so a unit cannot include both headers. */
void stMtnRequestBlendRatio(MotionControl *control, int id, float ratio);
void stMtnRequestBlendSpeed(MotionControl *control, int id, float speed);
void stMtnRequestBlendFrame(MotionControl *control, int id, float frames);

float fn_804157E8(float scale);
int fn_80415808(MotionControl *control, unsigned int key, int arg2, int arg3);
int fn_8041583C(MotionControl *control);
int fn_80415854(MotionControl *control, int id, int arg2, int arg3, int repeats);
int fn_80415A18(MotionControl *control, int id, int arg2, int arg3, int repeats);
void fn_80415BDC(MotionNode *node, const Fn8040Vector3 *offset);
unsigned short fn_80415CDC(MotionControl *control, int id);
void fn_80415D40(MotionControl *control, int id, MotionRange *range);
void fn_80415DBC(MotionNode *node, Fn8040Vector3 *position);
void fn_80415DF8(MotionNode *node, Fn8040Vector3 *position);
void fn_80415E34(MotionNode *node, Fn8040Vector3 *position);
int fn_80415E70(MotionControl *control, unsigned int key);
void fn_80415EA4(MotionControl *control);
void fn_80415F98(MotionControl *control, int id, unsigned int key, float arg3, float arg4);

#ifdef __cplusplus
}
#endif

#endif
