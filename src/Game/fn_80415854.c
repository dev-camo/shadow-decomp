/* Control of motions, part 3 (the original file name is unknown; every name here is inferred from
 * the code). A control owns players, each of which plays one motion, and the tree of nodes that the
 * motions move. See Game/fn_804157E8.c for the other pieces of the file. */
#include "Game/fn_8041116C_matrix_stack.h"
/* Declares fn_80411818 and fn_80411DEC, which fn_80415854 and fn_80415A18 call. */
#include "Game/fn_80411714_resource_groups.h"
#include "Game/fn_804157E8_motion_control.h"

/* The player that fn_80415A18 (lbl_805F13DC) and fn_80415854 (lbl_805F13E0) acted on last (defined
 * in .sbss with the rest of this file's data until it is split). A null control makes them act on
 * this player again. */
extern MotionPlayer *lbl_805F13DC;
extern MotionPlayer *lbl_805F13E0;

/* The player of an id, or null when the id names no player. */
static inline MotionPlayer *FindPlayer(MotionControl *control, int id) {
    int index = control->playerIndexOfId[id];

    if (index == -1) {
        return 0;
    }
    return &control->players[index];
}

/* The player of an id, or null when the id names no player or the player is not active. */
static inline MotionPlayer *FindActivePlayer(MotionControl *control, int id) {
    int index = control->playerIndexOfId[id];
    MotionPlayer *player;

    if (index == -1) {
        return 0;
    }
    player = &control->players[index];
    if (player->flags & MOTION_PLAYER_ACTIVE) {
        return player;
    }
    return 0;
}

/* Hands the motion of an active player to fn_80411818 (behaviour inferred): the range of the
 * player with its speed scaled, or changed when a flag of the player asks for it, and whether a
 * moving speed was replaced by 0 as the last argument. The motion id goes with the first call;
 * while a call returns nonzero, up to repeats further calls without an id follow. With a null
 * control the last player is used again and no call carries an id. Returns the last result. */
int fn_80415854(MotionControl *control, int id, int arg2, int arg3, int repeats) {
    MotionPlayer *player;
    unsigned short motionId = 0;
    int stopped = 0;
    MotionRange range;
    int result;

    if (control != 0) {
        player = FindActivePlayer(control, id);
        lbl_805F13E0 = player;
        if (player == 0) {
            return 0;
        }
        motionId = player->motionId;
    }
    player = lbl_805F13E0;
    if (player == 0) {
        return 0;
    }

    range.startFrame = player->range.startFrame;
    range.endFrame = player->range.endFrame;
    range.speed = player->range.speed;
    range.targetFrame = player->range.targetFrame;
    range.speed *= lbl_805EDD60;
    if (player->flags & MOTION_PLAYER_ZERO_SPEED) {
        if (fn_8000A980(range.speed) > 0.0f) {
            stopped = 1;
        }
        range.speed = 0.0f;
    } else if (player->flags & MOTION_PLAYER_FOLLOW_TARGET) {
        if (player->flags & MOTION_PLAYER_NUDGE_AT_LIMIT) {
            if (range.speed >= 0.0f) {
                if (fn_8000A980(player->frame - range.endFrame) < MOTION_LIMIT_EPSILON) {
                    range.speed += MOTION_LIMIT_EPSILON;
                }
            } else {
                if (fn_8000A980(player->frame - range.startFrame) < MOTION_LIMIT_EPSILON) {
                    range.speed -= MOTION_LIMIT_EPSILON;
                }
            }
        } else {
            range.speed = range.targetFrame - player->frame;
        }
    }

    result = fn_80411818(motionId, arg2, &range, arg3, stopped);
    while (repeats-- != 0 && result != 0) {
        result = fn_80411818(0, arg2, &range, arg3, stopped);
    }
    return result;
}

/* The same as fn_80415854 with fn_80411DEC and its own last player. */
int fn_80415A18(MotionControl *control, int id, int arg2, int arg3, int repeats) {
    MotionPlayer *player;
    unsigned short motionId = 0;
    int stopped = 0;
    MotionRange range;
    int result;

    if (control != 0) {
        player = FindActivePlayer(control, id);
        lbl_805F13DC = player;
        if (player == 0) {
            return 0;
        }
        motionId = player->motionId;
    }
    player = lbl_805F13DC;
    if (player == 0) {
        return 0;
    }

    range.startFrame = player->range.startFrame;
    range.endFrame = player->range.endFrame;
    range.speed = player->range.speed;
    range.targetFrame = player->range.targetFrame;
    range.speed *= lbl_805EDD60;
    if (player->flags & MOTION_PLAYER_ZERO_SPEED) {
        if (fn_8000A980(range.speed) > 0.0f) {
            stopped = 1;
        }
        range.speed = 0.0f;
    } else if (player->flags & MOTION_PLAYER_FOLLOW_TARGET) {
        if (player->flags & MOTION_PLAYER_NUDGE_AT_LIMIT) {
            if (range.speed >= 0.0f) {
                if (fn_8000A980(player->frame - range.endFrame) < MOTION_LIMIT_EPSILON) {
                    range.speed += MOTION_LIMIT_EPSILON;
                }
            } else {
                if (fn_8000A980(player->frame - range.startFrame) < MOTION_LIMIT_EPSILON) {
                    range.speed -= MOTION_LIMIT_EPSILON;
                }
            }
        } else {
            range.speed = range.targetFrame - player->frame;
        }
    }

    result = fn_80411DEC(motionId, arg2, &range, arg3, stopped);
    while (repeats-- != 0 && result != 0) {
        result = fn_80411DEC(0, arg2, &range, arg3, stopped);
    }
    return result;
}

/* Adds offset to the translation of every transform of a node and to some of its anchors: those
 * at index 6 and 0 always, those at 4 and 2 only when the halfword at 0x4 of the data of its
 * definition is 1 and the low three bits of the halfword at 0x6 are not all clear (names
 * inferred). */
void fn_80415BDC(MotionNode *node, const Fn8040Vector3 *offset) {
    int i;
    MotionTransform *transform;
    MotionNodeData *data;

    if (node == 0 || offset == 0) {
        return;
    }

    transform = node->transforms;
    for (i = 0; i < node->transformCount; i++) {
        transform->matrix.values[0][3] += offset->x;
        transform->matrix.values[1][3] += offset->y;
        transform->matrix.values[2][3] += offset->z;
        transform++;
    }

    fn_80411548(&node->anchors[6].position, &node->anchors[6].position, offset);
    data = node->definition->data;
    if (data->unknown04 == 1 && (data->unknown06 & 7)) {
        fn_80411548(&node->anchors[4].position, &node->anchors[4].position, offset);
        fn_80411548(&node->anchors[0].position, &node->anchors[0].position, offset);
        fn_80411548(&node->anchors[2].position, &node->anchors[2].position, offset);
    } else {
        fn_80411548(&node->anchors[0].position, &node->anchors[0].position, offset);
    }
}

/* The motion id of an active player, or 0. */
unsigned short fn_80415CDC(MotionControl *control, int id) {
    MotionPlayer *player;

    if (control == 0) {
        return 0;
    }
    player = FindActivePlayer(control, id);
    if (player == 0) {
        return 0;
    }
    return player->motionId;
}

/* Copies the range of an active player. */
void fn_80415D40(MotionControl *control, int id, MotionRange *range) {
    MotionPlayer *player;

    if (control == 0 || range == 0) {
        return;
    }
    player = FindActivePlayer(control, id);
    if (player == 0) {
        return;
    }
    range->startFrame = player->range.startFrame;
    range->endFrame = player->range.endFrame;
    range->speed = player->range.speed;
    range->targetFrame = player->range.targetFrame;
}

/* Copies the position of the anchors at index 5, 1 and 0 of a node (fn_80415DBC, fn_80415DF8,
 * fn_80415E34). */
void fn_80415DBC(MotionNode *node, Fn8040Vector3 *position) {
    if (node == 0 || position == 0) {
        return;
    }
    fn_800091D0(position, &node->anchors[5].position);
}

void fn_80415DF8(MotionNode *node, Fn8040Vector3 *position) {
    if (node == 0 || position == 0) {
        return;
    }
    fn_800091D0(position, &node->anchors[1].position);
}

void fn_80415E34(MotionNode *node, Fn8040Vector3 *position) {
    if (node == 0 || position == 0) {
        return;
    }
    fn_800091D0(position, &node->anchors[0].position);
}

/* Calls fn_8041D31C for the root node of a control. */
int fn_80415E70(MotionControl *control, unsigned int key) {
    if (control != 0) {
        return fn_8041D31C(control->rootNode, key);
    }
    return 0;
}

/* The step of a listed player when a control is updated: clears the high half of its flags, runs
 * fn_8041AB3C and, for an active player, makes its current frame the target frame. */
/* Matching workaround: the step is an inline function. With these statements written in the loop
 * of fn_80415EA4 the loop counter and the player pointer swap callee-saved registers (10 words
 * differ). Revisit if another compiler version or inline setting is found for this code. */
static inline void UpdateListedPlayer(MotionControl *control, int index) {
    MotionPlayer *player = &control->players[index];

    player->flags &= 0xFFFF;
    fn_8041AB3C(control, index);
    if (player->flags & MOTION_PLAYER_ACTIVE) {
        player->frame = player->range.targetFrame;
    }
}

/* Updates a control (behaviour inferred): clears bits 16 to 23 of its flags, steps each listed
 * player (the list ends at -1), and then runs the six update functions for the whole control. All
 * of it happens on a matrix pushed onto the matrix stack and set to the identity by fn_8040B6F8,
 * which is popped at the end. */
void fn_80415EA4(MotionControl *control) {
    int i;

    if (control == 0) {
        return;
    }

    fn_8040C204(lbl_805F13A8);
    fn_8040B6F8(&lbl_805F13A8->matrices[lbl_805F13A8->top]);

    control->flags &= ~0x00FF0000;
    for (i = 0; i < control->playerCount; i++) {
        int index = control->listedIndices[i];

        if (index == -1) {
            break;
        }
        UpdateListedPlayer(control, index);
    }

    fn_80419F5C(control);
    fn_804192EC(control);
    fn_80418A4C(control);
    fn_804188A0(control);
    fn_80418044(control);
    fn_804175C8(control);
    fn_8040C244(lbl_805F13A8);
}

/* Passes the index of the player of an id, a node key and two values to fn_8041CFB0 (it stores the
 * two values in the entry of that player for the node with that key), and sets two bits of the
 * state of the player whose meaning is unknown. */
void fn_80415F98(MotionControl *control, int id, unsigned int key, float arg3, float arg4) {
    MotionPlayer *player;

    if (control != 0) {
        player = FindPlayer(control, id);
        if (player != 0) {
            fn_8041CFB0(control->rootNode, control->playerIndexOfId[id], key, arg3, arg4);
            player->stateFlags |= 0x1;
            player->stateFlags |= 0x2000;
        }
    }
}

/* Sets the ratio of the blend of a player, limited to 0..1 (names inferred): the first ratio when
 * the first blend is in use and has kind 1 or 2, otherwise the second one when its kind is 1 or
 * 2. */
void stMtnRequestBlendRatio(MotionControl *control, int id, float ratio) {
    MotionPlayer *player;

    if (control == 0) {
        return;
    }
    player = FindPlayer(control, id);
    if (player == 0) {
        return;
    }

    if ((player->stateFlags & MOTION_PLAYER_BLEND_FIRST) &&
        (player->blend->kindFirst == 1 || player->blend->kindFirst == 2)) {
        player->blend->ratioFirst = ratio;
        if (player->blend->ratioFirst < 0.0f) {
            player->blend->ratioFirst = 0.0f;
        }
        if (player->blend->ratioFirst > 1.0f) {
            player->blend->ratioFirst = 1.0f;
        }
    } else if (player->blend->kindSecond == 1 || player->blend->kindSecond == 2) {
        player->blend->ratioSecond = ratio;
        if (player->blend->ratioSecond < 0.0f) {
            player->blend->ratioSecond = 0.0f;
        }
        if (player->blend->ratioSecond > 1.0f) {
            player->blend->ratioSecond = 1.0f;
        }
    }
}

/* Sets the speed of the blend of a player; a negative speed is ignored. */
void stMtnRequestBlendSpeed(MotionControl *control, int id, float speed) {
    MotionPlayer *player;

    if (control == 0) {
        return;
    }
    if (speed < 0.0f) {
        return;
    }
    player = FindPlayer(control, id);
    if (player == 0) {
        return;
    }
    player->blend->speed = speed;
}

/* Sets the frames of the blend of a player; a negative number is ignored. */
void stMtnRequestBlendFrame(MotionControl *control, int id, float frames) {
    MotionPlayer *player;

    if (control == 0) {
        return;
    }
    if (frames < 0.0f) {
        return;
    }
    player = FindPlayer(control, id);
    if (player == 0) {
        return;
    }
    player->blend->frames = frames;
}
