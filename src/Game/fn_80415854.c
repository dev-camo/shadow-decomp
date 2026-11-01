/* Control of motions: fn_80415854 to stMtnRequestBlendFrame (see
 * Game/fn_804157E8_motion_control.h for the file and its pieces). */
#include "Game/fn_80411714_resource_groups.h"
#include "Game/fn_804157E8_motion_control.h"

/* The player of an id, or null when the id names no player or the player is not active.
 * Matching workaround: the lookup is repeated here instead of calling FindPlayer. At the inline
 * depth of 1 an inline function that calls another inline function stays an out-of-line call,
 * and the target has the lookup expanded in every caller. Revisit if another inline setting is
 * found for this code. */
static inline MotionPlayer *FindActivePlayer(_stMtnPlayWork *control, int id) {
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
int fn_80415854(_stMtnPlayWork *control, int id, unsigned short *eventId, void **payload,
                int repeats) {
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
    range.currentFrame = player->range.currentFrame;
    range.speed *= lbl_805EDD60;
    if (player->flags & MOTION_PLAYER_ZERO_SPEED) {
        if (fn_8000A980(range.speed) > 0.0f) {
            stopped = 1;
        }
        range.speed = 0.0f;
    } else if (player->flags & MOTION_PLAYER_FLAG_400000) {
        if (player->flags & MOTION_PLAYER_NUDGE_AT_LIMIT) {
            if (range.speed >= 0.0f) {
                if (fn_8000A980(player->unknown12C - range.endFrame) < MOTION_LIMIT_EPSILON) {
                    range.speed += MOTION_LIMIT_EPSILON;
                }
            } else {
                if (fn_8000A980(player->unknown12C - range.startFrame) < MOTION_LIMIT_EPSILON) {
                    range.speed -= MOTION_LIMIT_EPSILON;
                }
            }
        } else {
            range.speed = range.currentFrame - player->unknown12C;
        }
    }

    result = fn_80411818(motionId, eventId, &range, payload, stopped);
    while (repeats-- != 0 && result != 0) {
        result = fn_80411818(0, eventId, &range, payload, stopped);
    }
    return result;
}

/* The same as fn_80415854 with fn_80411DEC and its own last player. */
int fn_80415A18(_stMtnPlayWork *control, int id, unsigned short eventId, void *destination,
                int repeats) {
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
    range.currentFrame = player->range.currentFrame;
    range.speed *= lbl_805EDD60;
    if (player->flags & MOTION_PLAYER_ZERO_SPEED) {
        if (fn_8000A980(range.speed) > 0.0f) {
            stopped = 1;
        }
        range.speed = 0.0f;
    } else if (player->flags & MOTION_PLAYER_FLAG_400000) {
        if (player->flags & MOTION_PLAYER_NUDGE_AT_LIMIT) {
            if (range.speed >= 0.0f) {
                if (fn_8000A980(player->unknown12C - range.endFrame) < MOTION_LIMIT_EPSILON) {
                    range.speed += MOTION_LIMIT_EPSILON;
                }
            } else {
                if (fn_8000A980(player->unknown12C - range.startFrame) < MOTION_LIMIT_EPSILON) {
                    range.speed -= MOTION_LIMIT_EPSILON;
                }
            }
        } else {
            range.speed = range.currentFrame - player->unknown12C;
        }
    }

    result = fn_80411DEC(motionId, eventId, &range, destination, stopped);
    while (repeats-- != 0 && result != 0) {
        result = fn_80411DEC(0, eventId, &range, destination, stopped);
    }
    return result;
}

/* Adds offset to the translation of every transform of a control and to some of its vectors:
 * those at index 6 and 0 always, those at 4 and 2 only when values[0] of the definition of its
 * root node is 1 and the low three bits of its values[1] are not all clear. */
void fn_80415BDC(_stMtnPlayWork *control, const Fn8040Vector3 *offset) {
    int i;
    MotionTransform *transform;
    PackNode *definition;

    if (control == 0 || offset == 0) {
        return;
    }

    transform = control->transforms;
    for (i = 0; i < control->transformCount; i++) {
        transform->matrix.values[0][3] += offset->x;
        transform->matrix.values[1][3] += offset->y;
        transform->matrix.values[2][3] += offset->z;
        transform++;
    }

    fn_80411548(&control->vectors[6].value, &control->vectors[6].value, offset);
    definition = control->rootNode->definition;
    if (definition->values[0] == 1 && (definition->values[1] & 7)) {
        fn_80411548(&control->vectors[4].value, &control->vectors[4].value, offset);
        fn_80411548(&control->vectors[0].value, &control->vectors[0].value, offset);
        fn_80411548(&control->vectors[2].value, &control->vectors[2].value, offset);
    } else {
        fn_80411548(&control->vectors[0].value, &control->vectors[0].value, offset);
    }
}

/* The motion id of an active player, or 0. */
unsigned short fn_80415CDC(_stMtnPlayWork *control, int id) {
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
void fn_80415D40(_stMtnPlayWork *control, int id, MotionRange *range) {
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
    range->currentFrame = player->range.currentFrame;
}

/* Copy the vectors at index 5, 1 and 0 of a control (fn_80415DBC, fn_80415DF8, fn_80415E34);
 * index 1 holds the angles that fn_804173A8 sets. */
void fn_80415DBC(_stMtnPlayWork *control, Fn8040Vector3 *vector) {
    if (control == 0 || vector == 0) {
        return;
    }
    fn_800091D0(vector, &control->vectors[5].value);
}

void fn_80415DF8(_stMtnPlayWork *control, Fn8040Vector3 *vector) {
    if (control == 0 || vector == 0) {
        return;
    }
    fn_800091D0(vector, &control->vectors[1].value);
}

void fn_80415E34(_stMtnPlayWork *control, Fn8040Vector3 *vector) {
    if (control == 0 || vector == 0) {
        return;
    }
    fn_800091D0(vector, &control->vectors[0].value);
}

/* The transform of the node with an id in the tree of a control, or null (fn_8041D31C). */
MotionTransform *fn_80415E70(_stMtnPlayWork *control, unsigned int nodeId) {
    if (control != 0) {
        return fn_8041D31C(control->rootNode, nodeId);
    }
    return 0;
}

/* The step of a listed player when a control is updated: clears the high half of its flags, runs
 * fn_8041AB3C and, for an active player, copies the current frame of its range to unknown12C. */
/* Matching workaround: the step is an inline function. With these statements written in the loop
 * of fn_80415EA4 the loop counter and the player pointer swap callee-saved registers (10 words
 * differ; with the inline 1 word still differs). A static per-player update function is a
 * plausible original form: the unit is built with -inline auto,level=1, which expands such a small
 * static function at its single call, and the expansion fits the target code far closer than the
 * statements written in the loop. Revisit if another compiler version or inline setting is found
 * for this code. */
static inline void UpdateListedPlayer(_stMtnPlayWork *control, int index) {
    MotionPlayer *player = &control->players[index];

    player->flags &= 0xFFFF;
    fn_8041AB3C(control, index);
    if (player->flags & MOTION_PLAYER_ACTIVE) {
        player->unknown12C = player->range.currentFrame;
    }
}

/* Updates a control (behaviour inferred): clears bits 16 to 23 of its flags, steps each listed
 * player (the list ends at -1), and then runs the six update functions for the whole control. All
 * of it happens on a matrix pushed onto the matrix stack and set to the identity by fn_8040B6F8,
 * which is popped at the end. */
void fn_80415EA4(_stMtnPlayWork *control) {
    int i;

    if (control == 0) {
        return;
    }

    fn_8040C204(lbl_805F13A8);
    fn_8040B6F8(TopMatrix());

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

/* Passes the index of the player of an id, a node id and two values to fn_8041CFB0 (it stores the
 * two values in the entry of that player for the node with that id), and sets
 * MOTION_PLAYER_REQUESTED and MOTION_PLAYER_STATE_2000 in the state of the player. */
void fn_80415F98(_stMtnPlayWork *control, int id, unsigned int nodeId, float arg3, float arg4) {
    MotionPlayer *player;

    if (control != 0) {
        player = FindPlayer(control, id);
        if (player != 0) {
            fn_8041CFB0(control->rootNode, control->playerIndexOfId[id], nodeId, arg3, arg4);
            player->stateFlags |= MOTION_PLAYER_REQUESTED;
            player->stateFlags |= MOTION_PLAYER_STATE_2000;
        }
    }
}

/* Sets the ratio of the blend of a player, limited to 0..1 (names inferred): the first ratio when
 * the first blend is in use and has kind 1 or 2, otherwise the second one when its kind is 1 or
 * 2. */
void stMtnRequestBlendRatio(_stMtnPlayWork *control, int id, float ratio) {
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
void stMtnRequestBlendSpeed(_stMtnPlayWork *control, int id, float speed) {
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
void stMtnRequestBlendFrame(_stMtnPlayWork *control, int id, float frames) {
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
