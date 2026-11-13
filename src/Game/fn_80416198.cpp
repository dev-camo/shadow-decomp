/* Control of motions: stMtnAbortBlend to fn_8041FD0C (see Game/fn_804157E8_motion_control.h for
 * the file and its pieces). */
#include <stdio.h>
#include <string.h>

#include "Game/fn_804110EC_allocator_hooks.h"
#include "Game/fn_80411714_resource_groups.h"
#include "Game/fn_804157E8_motion_control.h"

int stMtnAbortBlend(_stMtnPlayWork *control, int id) {
    int kind = 3;
    int index;
    int position = -1;
    MotionPlayer *player;

    if (control == 0) {
        return 0;
    }
    index = control->playerList.playerIndexOfId[id];
    if (index == -1) {
        return 0;
    }
    player = &control->players[index];
    if (player->stateFlags & MOTION_PLAYER_BLEND_FIRST) {
        kind = player->blend->kindFirst;
    } else if (player->blend->flags & MOTION_BLEND_FLAG_1) {
        kind = player->blend->kindSecond;
    }
    for (int i = 0; i < control->playerList.count * 2; i++) {
        if (control->playerList.listedIndices[i] == -1) {
            break;
        }
        if (control->playerList.listedIndices[i] == index) {
            position = i;
        }
    }
    if (position == -1) {
        return 0;
    }

    switch (kind) {
    default:
        return 0;
    case 1:
    case 2:
        if (player->stateFlags & MOTION_PLAYER_BLEND_FIRST) {
            player->stateFlags &= ~MOTION_PLAYER_REQUEST_MOTION;
            player->stateFlags &= ~MOTION_PLAYER_BLEND_FIRST;
            control->playerList.unknown0C[index + control->playerList.count] = -1;
        } else if (player->blend->flags & MOTION_BLEND_FLAG_1) {
            player->flags &= ~MOTION_PLAYER_ACTIVE;
            player->flags &= ~0x1;
            player->flags &= ~MOTION_PLAYER_NUDGE_AT_LIMIT;
            player->stateFlags &= ~MOTION_PLAYER_STATE_4000;
            player->stateFlags &= ~MOTION_PLAYER_STATE_1000;
            if (player->flags & MOTION_PLAYER_FLAG_10) {
                fn_8041CC18(control->rootNode, index);
                player->flags &= ~MOTION_PLAYER_FLAG_10;
            }
            if (player->motionId != 0) {
                fn_80413240(player->motionId);
            }
            control->playerList.unknown0C[index] = -1;
        }
        for (int i = 0; i < control->playerList.count * 2; i++) {
            int listedIndex = control->playerList.listedIndices[i];
            if (listedIndex == -1) {
                break;
            }
            if (i >= position) {
                control->playerList.listedIndices[i] = -1;
                if (i != position) {
                    control->playerList.listedIndices[i - 1] = listedIndex;
                }
            }
        }
        for (int i = 0; i < control->playerList.count; i++) {
            if (control->playerList.playerIndexOfId[i] == -1) {
                break;
            }
            if (index == control->playerList.playerIndexOfId[i]) {
                control->playerList.playerIndexOfId[i] = -1;
                for (; i < control->playerList.count - 1; i++) {
                    control->playerList.playerIndexOfId[i] =
                        control->playerList.playerIndexOfId[i + 1];
                    control->playerList.playerIndexOfId[i + 1] = -1;
                }
                break;
            }
        }
        break;
    case 0:
        for (int i = 0; i < control->playerList.count * 2; i++) {
            int listedIndex = control->playerList.listedIndices[i];
            if (listedIndex == -1) {
                break;
            }
            MotionPlayer *listedPlayer = &control->players[listedIndex];
            control->playerList.listedIndices[i] = -1;
            if (i >= position) {
                if (i == position) {
                    listedPlayer->stateFlags &= ~MOTION_PLAYER_BLEND_FIRST;
                    listedPlayer->blend->flags &= ~MOTION_BLEND_FLAG_2;
                }
                control->playerList.listedIndices[i - position] = listedIndex;
            } else {
                if (!(listedPlayer->blend->flags & MOTION_BLEND_FLAG_2)) {
                    listedPlayer->flags &= ~MOTION_PLAYER_ACTIVE;
                    listedPlayer->flags &= ~0x1;
                    listedPlayer->flags &= ~MOTION_PLAYER_NUDGE_AT_LIMIT;
                    listedPlayer->stateFlags &= ~MOTION_PLAYER_STATE_4000;
                    listedPlayer->stateFlags &= ~MOTION_PLAYER_STATE_1000;
                    if (listedPlayer->flags & MOTION_PLAYER_FLAG_10) {
                        fn_8041CC18(control->rootNode, listedIndex);
                        listedPlayer->flags &= ~MOTION_PLAYER_FLAG_10;
                    }
                    if (listedPlayer->motionId != 0) {
                        fn_80413240(listedPlayer->motionId);
                    }
                    for (int j = 0; j < control->playerList.count; j++) {
                        if (control->playerList.playerIndexOfId[j] == -1) {
                            break;
                        }
                        if (listedIndex == control->playerList.playerIndexOfId[j]) {
                            control->playerList.playerIndexOfId[j] = -1;
                            for (; j < control->playerList.count - 1; j++) {
                                control->playerList.playerIndexOfId[j] =
                                    control->playerList.playerIndexOfId[j + 1];
                                control->playerList.playerIndexOfId[j + 1] = -1;
                            }
                            break;
                        }
                    }
                }
                listedPlayer->blend->flags &= ~MOTION_BLEND_FLAG_2;
            }
        }
        break;
    }
    return 1;
}

int stMtnPrepareBlendWithTarget(_stMtnPlayWork *control, int target, int first, int last) {
    int needsPlayer = 0;
    int reusePlayer = 0;
    int kind = 3;
    int index = -1;

    if (control == 0) {
        return 0;
    }
    if (first < 0) {
        first = 0;
    }
    if (last < 0) {
        last = first;
    }
    for (int i = 0; i < control->playerList.count; i++) {
        int j;
        for (j = 0; j < control->playerList.count * 2; j++) {
            if (i == control->playerList.listedIndices[j] ||
                control->playerList.listedIndices[j] == -1) {
                break;
            }
        }
        if (control->playerList.listedIndices[j] == -1) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        for (int i = 0; i < control->playerList.count; i++) {
            int listedIndex = control->playerList.listedIndices[i];
            if (listedIndex == -1) {
                break;
            }
            MotionPlayer *player = &control->players[listedIndex];
            if ((player->blend->flags & MOTION_BLEND_FLAG_2) &&
                !(player->stateFlags & MOTION_PLAYER_BLEND_FIRST)) {
                reusePlayer = 1;
                index = listedIndex;
                break;
            }
        }
    }

    switch (target) {
    default:
        return 0;
    case 0:
        if (control->flags & 0x1000000) {
            needsPlayer = 1;
        } else if (index == -1) {
            needsPlayer = 1;
        }
        break;
    case 1:
    case 2:
        first = control->playerList.playerIndexOfId[first];
        if (first == -1) {
            return 0;
        }
        last = control->playerList.playerIndexOfId[last];
        if (last == -1) {
            return 0;
        }
        if (first != last && first != control->playerList.unknown0C[last]) {
            return 0;
        }
        if (index == -1) {
            int position = -1;
            needsPlayer = 1;
            for (int i = 0; i < control->playerList.count * 2; i++) {
                if (control->playerList.listedIndices[i] == -1) {
                    break;
                }
                if (control->playerList.listedIndices[i] == first) {
                    position = i;
                }
            }
            if (position == -1) {
                return 0;
            }
            int found = 0;
            for (int i = 0; i < control->playerList.count * 2; i++) {
                int listedIndex = control->playerList.listedIndices[i];
                if (listedIndex == -1) {
                    break;
                }
                MotionPlayer *player = &control->players[listedIndex];
                if (found && ((player->stateFlags & MOTION_PLAYER_BLEND_FIRST) ||
                              !(player->blend->flags & MOTION_BLEND_FLAG_1) ||
                              player->blend->kindSecond == 0)) {
                    break;
                }
                if (!(player->blend->flags & MOTION_BLEND_FLAG_2)) {
                    if (player->stateFlags & MOTION_PLAYER_BLEND_FIRST) {
                        break;
                    }
                    if (!(player->blend->flags & MOTION_BLEND_FLAG_1)) {
                        found = 1;
                    } else if (player->blend->kindSecond == 0) {
                        found = 1;
                    }
                    if (found && i == position) {
                        needsPlayer = 0;
                        break;
                    }
                }
            }
        }
        break;
    }

    if (needsPlayer) {
        int found = 0;
        for (int i = 0; i < control->playerList.count; i++) {
            int listedIndex = control->playerList.listedIndices[i];
            if (listedIndex == -1) {
                break;
            }
            MotionPlayer *player = &control->players[listedIndex];
            if (found &&
                ((player->stateFlags & MOTION_PLAYER_BLEND_FIRST) ||
                 (!(control->flags & 0x1000000) && (!(player->blend->flags & MOTION_BLEND_FLAG_1) ||
                                                    player->blend->kindSecond == 0)))) {
                break;
            }
            if (!(player->blend->flags & MOTION_BLEND_FLAG_2)) {
                if (player->stateFlags & MOTION_PLAYER_BLEND_FIRST) {
                    break;
                }
                if (control->flags & 0x1000000) {
                    found = 1;
                } else if (!(player->blend->flags & MOTION_BLEND_FLAG_1)) {
                    found = 1;
                } else if (player->blend->kindSecond == 0) {
                    found = 1;
                }
                if (found) {
                    player->blend->flags |= MOTION_BLEND_FLAG_2;
                    player->flags &= ~0x1;
                    player->flags &= ~MOTION_PLAYER_NUDGE_AT_LIMIT;
                    player->stateFlags &= ~MOTION_PLAYER_STATE_4000;
                    player->stateFlags &= ~MOTION_PLAYER_STATE_1000;
                    for (int j = 0; j < control->playerList.count; j++) {
                        if (control->playerList.playerIndexOfId[j] == -1) {
                            break;
                        }
                        if (listedIndex == control->playerList.playerIndexOfId[j]) {
                            control->playerList.playerIndexOfId[j] = -1;
                            for (; j < control->playerList.count - 1; j++) {
                                control->playerList.playerIndexOfId[j] =
                                    control->playerList.playerIndexOfId[j + 1];
                                control->playerList.playerIndexOfId[j + 1] = -1;
                            }
                            break;
                        }
                    }
                }
            }
        }
        if (!found) {
            return 0;
        }
        for (int i = 0; i < control->playerList.count; i++) {
            int listedIndex = control->playerList.listedIndices[i];
            if (listedIndex == -1) {
                break;
            }
            MotionPlayer *player = &control->players[listedIndex];
            if ((player->blend->flags & MOTION_BLEND_FLAG_2) &&
                !(player->stateFlags & MOTION_PLAYER_BLEND_FIRST)) {
                reusePlayer = 1;
                index = listedIndex;
                break;
            }
        }
    }
    if (index == -1) {
        return 0;
    }

    switch (target) {
    default:
        return 0;
    case 0:
        for (int i = 0; i < control->playerList.count * 2; i++) {
            if (control->playerList.listedIndices[i] == -1) {
                control->playerList.listedIndices[i] = index;
                break;
            }
        }
        if (reusePlayer) {
            control->playerList.unknown0C[index + control->playerList.count] = -1;
        } else {
            control->playerList.unknown0C[index] = -1;
        }
        break;
    case 1:
    case 2:
        if (target == 1 && first != last) {
            return 0;
        }
        MotionPlayer *player = &control->players[first];
        if (player->stateFlags & MOTION_PLAYER_BLEND_FIRST) {
            kind = player->blend->kindFirst;
        } else if (player->blend->flags & MOTION_BLEND_FLAG_1) {
            kind = player->blend->kindSecond;
        }
        if (kind == 1) {
            return 0;
        }
        int found = 0;
        for (int i = 0; i < control->playerList.count * 2; i++) {
            int listedIndex = control->playerList.listedIndices[i];
            if (listedIndex == -1) {
                break;
            }
            MotionPlayer *listedPlayer = &control->players[listedIndex];
            if (!(listedPlayer->blend->flags & MOTION_BLEND_FLAG_2)) {
                kind = 3;
                if (listedPlayer->stateFlags & MOTION_PLAYER_BLEND_FIRST) {
                    if (first ==
                        control->playerList.unknown0C[listedIndex + control->playerList.count]) {
                        kind = listedPlayer->blend->kindFirst;
                    }
                } else if ((listedPlayer->blend->flags & MOTION_BLEND_FLAG_1) &&
                           first == control->playerList.unknown0C[listedIndex]) {
                    kind = listedPlayer->blend->kindSecond;
                }
                if (kind == 1) {
                    if (target == 1) {
                        return 0;
                    }
                    if (found) {
                        break;
                    }
                }
                if (listedIndex == last) {
                    found = 1;
                }
            }
        }
        int position = -1;
        for (int i = 0; i < control->playerList.count * 2; i++) {
            if (control->playerList.listedIndices[i] == -1) {
                break;
            }
            if (last == control->playerList.listedIndices[i]) {
                position = i;
            }
        }
        int carriedIndex = index;
        for (int i = position + 1; i < control->playerList.count * 2; i++) {
            int listedIndex = control->playerList.listedIndices[i];
            control->playerList.listedIndices[i] = carriedIndex;
            if (listedIndex == -1) {
                break;
            }
            carriedIndex = listedIndex;
        }
        if (reusePlayer) {
            control->playerList.unknown0C[index + control->playerList.count] = first;
        } else {
            control->playerList.unknown0C[index] = first;
        }
        break;
    }

    int carriedIndex = index;
    for (int i = 0; i < control->playerList.count; i++) {
        int listedIndex = control->playerList.playerIndexOfId[i];
        control->playerList.playerIndexOfId[i] = carriedIndex;
        if (listedIndex == -1) {
            break;
        }
        carriedIndex = listedIndex;
    }
    MotionPlayer *player = &control->players[index];
    player->blend->kindFirst = target;
    player->blend->frames = 0.0f;
    player->blend->speed = 1.0f;
    player->stateFlags |= MOTION_PLAYER_REQUESTED;
    player->stateFlags |= MOTION_PLAYER_REQUEST_MOTION;
    player->stateFlags |= MOTION_PLAYER_BLEND_FIRST;
    return 1;
}

void stMtnRequestFrameSpeed(_stMtnPlayWork *control, int id, float speed) {
    MotionPlayer *player;

    if (control == 0) {
        return;
    }
    player = FindPlayer(control, id);
    if (player == 0) {
        return;
    }
    player->requestedRange.speed = speed;
    player->stateFlags |= MOTION_PLAYER_REQUESTED;
    player->stateFlags |= MOTION_PLAYER_REQUEST_SPEED;
}

void stMtnRequestCurrentFrame(_stMtnPlayWork *control, int id, float frame) {
    MotionPlayer *player;

    if (control == 0) {
        return;
    }
    player = FindPlayer(control, id);
    if (player == 0) {
        return;
    }
    player->requestedRange.currentFrame = frame;
    player->stateFlags |= MOTION_PLAYER_REQUESTED;
    player->stateFlags |= MOTION_PLAYER_REQUEST_CURRENT_FRAME;
}

void stMtnRequestEndFrame(_stMtnPlayWork *control, int id, float frame) {
    MotionPlayer *player;

    if (control == 0) {
        return;
    }
    player = FindPlayer(control, id);
    if (player == 0) {
        return;
    }
    player->requestedRange.endFrame = frame;
    player->stateFlags |= MOTION_PLAYER_REQUESTED;
    player->stateFlags |= MOTION_PLAYER_REQUEST_END_FRAME;
}

void stMtnRequestStartFrame(_stMtnPlayWork *control, int id, float frame) {
    MotionPlayer *player;

    if (control == 0) {
        return;
    }
    player = FindPlayer(control, id);
    if (player == 0) {
        return;
    }
    player->requestedRange.startFrame = frame;
    player->stateFlags |= MOTION_PLAYER_REQUESTED;
    player->stateFlags |= MOTION_PLAYER_REQUEST_START_FRAME;
}

/* Reads, sets or clears one of sixteen flags of a control (11 to 15) or of the player of an id (0
 * to 10) and returns whether the flag was set before: action 0 sets it, 2 clears it and any other
 * value only reads it. Flags 3 to 6 and 8 to 10 can only be read. Two flags set and clear another
 * bit than the one they read, as the original does: 14 reads control bit 0x4000 but changes
 * 0x1000000, and 7 reads MOTION_PLAYER_FLAG_8 but changes MOTION_PLAYER_STATE_800. */
int stMtnControlWorkFlag(_stMtnPlayWork *control, int id, int flag, int action) {
    int result = 0;
    MotionPlayer *player;

    if (control == 0 || flag >= 16) {
        return 0;
    }
    switch (flag) {
    case 11:
        result = (control->flags & 0x1) != 0;
        switch (action) {
        case 0:
            control->flags |= 0x1;
            break;
        case 2:
            control->flags &= ~0x1;
            break;
        }
        return result;
    case 12:
        result = (control->flags & 0x2) != 0;
        switch (action) {
        case 0:
            control->flags |= 0x2;
            break;
        case 2:
            control->flags &= ~0x2;
            break;
        }
        return result;
    case 13:
        result = (control->flags & 0x4) != 0;
        switch (action) {
        case 0:
            control->flags |= 0x4;
            break;
        case 2:
            control->flags &= ~0x4;
            break;
        }
        return result;
    case 14:
        result = (control->flags & 0x4000) != 0;
        switch (action) {
        case 0:
            control->flags |= 0x1000000;
            break;
        case 2:
            control->flags &= ~0x1000000;
            break;
        }
        return result;
    case 15:
        result = (control->flags & MOTION_CONTROL_WHOLE_FRAMES) != 0;
        switch (action) {
        case 0:
            control->flags |= MOTION_CONTROL_WHOLE_FRAMES;
            break;
        case 2:
            control->flags &= ~MOTION_CONTROL_WHOLE_FRAMES;
            break;
        }
        return result;
    }

    player = FindPlayer(control, id);
    if (player == 0) {
        return 0;
    }
    switch (flag) {
    case 0:
        result = (player->flags & 0x1) != 0;
        switch (action) {
        case 0:
            player->flags |= 0x1;
            player->stateFlags |= MOTION_PLAYER_STATE_4000;
            break;
        case 2:
            player->flags &= ~0x1;
            player->stateFlags |= MOTION_PLAYER_STATE_4000;
            break;
        }
        break;
    case 1:
        result = (player->flags & MOTION_PLAYER_NUDGE_AT_LIMIT) != 0;
        switch (action) {
        case 0:
            player->flags |= MOTION_PLAYER_NUDGE_AT_LIMIT;
            player->stateFlags |= MOTION_PLAYER_STATE_1000;
            break;
        case 2:
            player->flags &= ~MOTION_PLAYER_NUDGE_AT_LIMIT;
            player->stateFlags |= MOTION_PLAYER_STATE_1000;
            break;
        }
        break;
    case 2:
        result = (player->stateFlags & MOTION_PLAYER_STATE_400) != 0;
        switch (action) {
        case 0:
            player->stateFlags |= MOTION_PLAYER_STATE_400;
            break;
        case 2:
            player->stateFlags &= ~MOTION_PLAYER_STATE_400;
            break;
        }
        break;
    case 3:
        result = (player->flags & MOTION_PLAYER_FLAG_400000) != 0;
        break;
    case 4:
        result = (player->flags & MOTION_PLAYER_ACTIVE) != 0;
        break;
    case 5:
        result = player->blend->flags & MOTION_BLEND_FLAG_1;
        break;
    case 6:
        result = (player->flags & MOTION_PLAYER_FLAG_800000) != 0;
        break;
    case 7:
        result = (player->flags & MOTION_PLAYER_FLAG_8) != 0;
        switch (action) {
        case 0:
            player->stateFlags |= MOTION_PLAYER_STATE_800;
            break;
        case 2:
            player->stateFlags &= ~MOTION_PLAYER_STATE_800;
            break;
        }
        break;
    case 8:
        result = (player->stateFlags & MOTION_PLAYER_BLEND_FIRST) != 0;
        break;
    case 9:
        result = (player->stateFlags & MOTION_PLAYER_REQUEST_MOTION) != 0;
        break;
    case 10:
        result = (player->flags & MOTION_PLAYER_FLAG_10000) != 0;
        break;
    }
    return result;
}

/* Sets vector 0 of the control to the translation of a matrix, zeroes vector 1 and copies the
 * matrix to the one at 0x90 without its translation. */
void fn_804172D8(_stMtnPlayWork *control, const Fn8040Matrix34 *matrix) {
    if (control != 0 && matrix != 0) {
        control->vectors[0].value.x = matrix->values[0][3];
        control->vectors[0].value.y = matrix->values[1][3];
        control->vectors[0].value.z = matrix->values[2][3];
        control->vectors[1].value.x = 0.0f;
        control->vectors[1].value.y = 0.0f;
        control->vectors[1].value.z = 0.0f;
        fn_8040C148(&control->matrix, matrix);
        control->matrix.values[0][3] = 0.0f;
        control->matrix.values[1][3] = 0.0f;
        control->matrix.values[2][3] = 0.0f;
    }
}

/* The same as fn_804172D8 without setting vector 0. */
void fn_8041734C(_stMtnPlayWork *control, const Fn8040Matrix34 *matrix) {
    if (control != 0 && matrix != 0) {
        control->vectors[1].value.x = 0.0f;
        control->vectors[1].value.y = 0.0f;
        control->vectors[1].value.z = 0.0f;
        fn_8040C148(&control->matrix, matrix);
        control->matrix.values[0][3] = 0.0f;
        control->matrix.values[1][3] = 0.0f;
        control->matrix.values[2][3] = 0.0f;
    }
}

/* Sets vector 0 of the control to a position and vector 1 to a rotation (angles), and the matrix at
 * 0x90 to the rotation: the angles are applied about z, y and x, or about y, x and z when
 * rotationOrder is 1. A null rotation stops after the position. */
void fn_804173A8(_stMtnPlayWork *control, const Fn8040Vector3 *position,
                 const Fn8040Vector3 *rotation, int rotationOrder) {
    if (control == 0) {
        return;
    }
    if (position != 0) {
        control->vectors[0].value = *position;
    }
    if (rotation == 0) {
        return;
    }
    control->vectors[1].value = *rotation;

    fn_8040C204(lbl_805F13A8);
    fn_8040B6F8(TopMatrix());
    if (rotationOrder != 1) {
        fn_8040B80C(TopMatrix(), rotation->z);
        fn_8040B848(TopMatrix(), rotation->y);
        fn_8040B884(TopMatrix(), rotation->x);
    } else {
        fn_8040B848(TopMatrix(), rotation->y);
        fn_8040B884(TopMatrix(), rotation->x);
        fn_8040B80C(TopMatrix(), rotation->z);
    }
    fn_8040C148(&control->matrix, fn_80411700(lbl_805F13A8));
    fn_8040C244(lbl_805F13A8);
}

void stMtnRequestMotion(_stMtnPlayWork *control, int id, unsigned short motionId) {
    MotionPlayer *player;

    if (control == 0 || motionId == 0) {
        return;
    }
    player = FindPlayer(control, id);
    if (player == 0) {
        return;
    }
    player->requestedMotionId = motionId;
    player->stateFlags |= MOTION_PLAYER_REQUESTED;
    player->stateFlags |= MOTION_PLAYER_REQUEST_MOTION;
    player->stateFlags |= MOTION_PLAYER_STATE_8000;
}

/* stMtnRequestMotion for the player of id 0, without MOTION_PLAYER_STATE_8000. */
void fn_80417568(_stMtnPlayWork *control, unsigned short motionId) {
    MotionPlayer *player;

    if (control == 0 || motionId == 0) {
        return;
    }
    player = FindPlayer(control, 0);
    if (player == 0) {
        return;
    }
    player->requestedMotionId = motionId;
    player->stateFlags |= MOTION_PLAYER_REQUESTED;
    player->stateFlags |= MOTION_PLAYER_REQUEST_MOTION;
}

void fn_804175C8(_stMtnPlayWork *control) {
    int combineOffsets = 0;
    if (control->rootNode->definition->values[0] == 1 &&
        (control->rootNode->definition->values[1] & MOTION_NODE_SCALED_TRACK)) {
        control->vectors[4].value.x = control->transforms->matrix.values[0][3];
        control->vectors[4].value.y = control->transforms->matrix.values[1][3];
        control->vectors[4].value.z = control->transforms->matrix.values[2][3];
        if (control->flags & 0x1) {
            control->vectors[5].value.x = control->vectors[4].value.x - control->vectors[0].value.x;
            control->vectors[5].value.y = control->vectors[4].value.y - control->vectors[0].value.y;
            control->vectors[5].value.z = control->vectors[4].value.z - control->vectors[0].value.z;
            control->vectors[0].value = control->vectors[4].value;
        } else {
            control->vectors[5].value.x = control->vectors[4].value.x - control->vectors[0].value.x;
            control->vectors[5].value.y = 0.0f;
            control->vectors[5].value.z = control->vectors[4].value.z - control->vectors[0].value.z;
            control->vectors[0].value.x = control->vectors[4].value.x;
            control->vectors[0].value.z = control->vectors[4].value.z;
        }
        control->vectors[2].value = control->vectors[0].value;
        control->vectors[3].value = control->vectors[1].value;
        fn_8040C148(&control->unknownC0, &control->matrix);
    }
    fn_8040C204(lbl_805F13A8);
    fn_8040B6F8(TopMatrix());
    fn_8040B59C(TopMatrix(), control->vectors[0].value);
    fn_8040BAC4(TopMatrix(), &control->matrix);
    fn_8040BCD4(TopMatrix());
    for (int i = 0; i < control->playerList.count; i++) {
        int index = control->playerList.listedIndices[i];
        if (index == -1) {
            break;
        }
        MotionPlayer *player = &control->players[index];
        if (i == 0 || (player->flags & MOTION_PLAYER_ACTIVE)) {
            fn_8040C204(lbl_805F13A8);
            fn_8040BAC4(TopMatrix(), &player->unknown00);
            fn_8040C148(&player->unknown00, fn_80411700(lbl_805F13A8));
            fn_8040C244(lbl_805F13A8);
            if (player->flags & MOTION_PLAYER_FLAG_1000000) {
                fn_8040C148(&player->unknown30, &player->unknown00);
            } else {
                fn_804113E8(&player->unknown00, &player->unknown30);
            }
            if (control->rootNode->definition->values[0] == 1 &&
                (control->rootNode->definition->values[1] & MOTION_NODE_SCALED_TRACK)) {
                if (control->flags & 0x1) {
                    player->unknownF0.value.x =
                        player->unknownE0.value.x - player->unknownD0.value.x;
                    player->unknownF0.value.y =
                        player->unknownE0.value.y - player->unknownD0.value.y;
                    player->unknownF0.value.z =
                        player->unknownE0.value.z - player->unknownD0.value.z;
                } else {
                    player->unknownF0.value.x =
                        player->unknownE0.value.x - player->unknownD0.value.x;
                    player->unknownF0.value.y = 0.0f;
                    player->unknownF0.value.z =
                        player->unknownE0.value.z - player->unknownD0.value.z;
                }
                player->unknownD0.value = player->unknownE0.value;
            }
            if (player->blend->flags & MOTION_BLEND_FLAG_1) {
                if (i == 0) {
                    fn_8040C204(lbl_805F13A8);
                    fn_8040BAC4(TopMatrix(), &control->unknown30);
                    fn_8040C148(&control->unknown30, fn_80411700(lbl_805F13A8));
                    fn_8040C244(lbl_805F13A8);
                    if (control->flags & 0x10000) {
                        fn_8040C148(&control->unknown60, &control->unknown30);
                    } else {
                        fn_804113E8(&control->unknown30, &control->unknown60);
                    }
                }
                if (player->blend->kindSecond == 1) {
                    int otherIndex = control->playerList.unknown0C[index];
                    if (otherIndex != -1) {
                        MotionPlayer *other = &control->players[otherIndex];
                        if (other->flags & MOTION_PLAYER_ACTIVE) {
                            float ratio = player->blend->ratioSecond;
                            MotionNodeEntry *entry = &control->rootNode->entries[index];
                            if (entry->unknown00 != 0) {
                                ratio = ratio * entry->unknown08 + entry->unknown0C;
                            }
                            Fn8040Vector3 a;
                            Fn8040Vector3 b;
                            Fn8040Vector3 offset;
                            a = other->unknownF0.value;
                            b = player->unknownF0.value;
                            fn_8040C204(lbl_805F13A8);
                            fn_8040C148(fn_80411700(lbl_805F13A8), &other->unknown00);
                            offset.x = ratio * (b.x - a.x);
                            offset.y = ratio * (b.y - a.y);
                            offset.z = ratio * (b.z - a.z);
                            fn_8040B59C(TopMatrix(), offset);
                            fn_8040C148(&other->unknown00, fn_80411700(lbl_805F13A8));
                            fn_8040C148(fn_80411700(lbl_805F13A8), &player->unknown00);
                            offset.x = (1.0f - ratio) * (a.x - b.x);
                            offset.y = (1.0f - ratio) * (a.y - b.y);
                            offset.z = (1.0f - ratio) * (a.z - b.z);
                            fn_8040B59C(TopMatrix(), offset);
                            fn_8040C148(&player->unknown00, fn_80411700(lbl_805F13A8));
                            fn_8040C244(lbl_805F13A8);
                            player->range.speed =
                                (other->range.speed *
                                 (player->range.endFrame - player->range.startFrame)) /
                                (other->range.endFrame - other->range.startFrame);
                        }
                    }
                } else if (player->blend->kindSecond == 0 && (control->flags & 0x4) &&
                           control->rootNode->definition->values[0] == 1 &&
                           (control->rootNode->definition->values[1] & MOTION_NODE_SCALED_TRACK)) {
                    combineOffsets = 1;
                }
            }
        }
    }
    if (combineOffsets) {
        Fn8040Vector3 movement;
        Fn8040Vector3 offset;
        Fn8040Vector3 reference;
        movement = control->vectors[5].value;
        fn_8041157C(&movement, 0, &movement);
        fn_8040C204(lbl_805F13A8);
        for (int i = 0; i < control->playerList.count; i++) {
            int index = control->playerList.listedIndices[i];
            if (index == -1) {
                break;
            }
            lbl_805E14D0[index] = 0;
            lbl_805E14E0[index] = 0;
        }
        int pending;
        do {
            pending = 0;
            for (int i = 0; i < control->playerList.count; i++) {
                int index = control->playerList.listedIndices[i];
                if (index == -1) {
                    break;
                }
                if (pending && control->playerList.unknown0C[index] == -1) {
                    lbl_805E14C0[index] = 1;
                } else {
                    lbl_805E14C0[index] = 0;
                    for (int j = i + 1; j < control->playerList.count; j++) {
                        int otherIndex = control->playerList.listedIndices[j];
                        if (otherIndex == -1) {
                            break;
                        }
                        if (lbl_805E14D0[otherIndex] == 0 &&
                            index == control->playerList.unknown0C[otherIndex]) {
                            pending = 1;
                            lbl_805E14C0[index] = 1;
                            break;
                        }
                    }
                }
            }
            for (int i = 0; i < control->playerList.count; i++) {
                int index = control->playerList.listedIndices[i];
                if (index == -1) {
                    break;
                }
                if (lbl_805E14D0[index] == 0) {
                    MotionPlayer *player = &control->players[index];
                    offset = player->unknownF0.value;
                    fn_8040C148(fn_80411700(lbl_805F13A8), &player->unknown00);
                    fn_8041157C(&offset, 0, &offset);
                    if (lbl_805E14C0[index] != 0) {
                        if (lbl_805E14E0[index] == 0) {
                            lbl_805E1490[index] = offset;
                            lbl_805E14E0[index] = 1;
                        }
                    } else {
                        lbl_805E14D0[index] = 1;
                        int otherIndex = control->playerList.unknown0C[index];
                        Fn8040Vector3 *destination = &lbl_805E1490[index];
                        if (otherIndex != -1) {
                            destination = &lbl_805E1490[otherIndex];
                            lbl_805E14E0[otherIndex] = 1;
                        }
                        const Fn8040Vector3 *value = &offset;
                        if (lbl_805E14E0[index] != 0) {
                            value = &lbl_805E1490[index];
                        }
                        if ((player->blend->flags & MOTION_BLEND_FLAG_1) && otherIndex != -1) {
                            float ratio = player->blend->ratioSecond;
                            MotionNodeEntry *entry = &control->rootNode->entries[index];
                            if (entry->unknown00 != 0) {
                                ratio = ratio * entry->unknown08 + entry->unknown0C;
                            }
                            float complement = 1.0f - ratio;
                            destination->x = ratio * value->x + complement * destination->x;
                            destination->y = ratio * value->y + complement * destination->y;
                            destination->z = ratio * value->z + complement * destination->z;
                        } else {
                            *destination = *value;
                        }
                    }
                }
            }
        } while (pending);
        for (int i = 0; i < control->playerList.count; i++) {
            int index = control->playerList.listedIndices[i];
            if (index == -1) {
                break;
            }
            MotionPlayer *player = &control->players[index];
            if (player->flags & MOTION_PLAYER_ACTIVE) {
                if (i == 0) {
                    fn_8040B6F8(TopMatrix());
                    fn_8040B59C(TopMatrix(), movement);
                    fn_8040BAC4(TopMatrix(), &control->unknown30);
                    fn_8040C148(&control->unknown30, fn_80411700(lbl_805F13A8));
                }
                if (control->playerList.unknown0C[index] == -1) {
                    reference = lbl_805E1490[index];
                }
                offset.x = movement.x - reference.x;
                offset.y = movement.y - reference.y;
                offset.z = movement.z - reference.z;
                fn_8040B6F8(TopMatrix());
                fn_8040B59C(TopMatrix(), offset);
                fn_8040BAC4(TopMatrix(), &player->unknown00);
                fn_8040C148(&player->unknown00, fn_80411700(lbl_805F13A8));
            }
        }
        fn_8040C244(lbl_805F13A8);
    }
    fn_8040C244(lbl_805F13A8);
    for (int i = 0; i < control->playerList.count; i++) {
        MotionPlayer *player = &control->players[i];
        player->requestedMotionId = 0;
        player->requestedRange.startFrame = -1.0f;
        player->requestedRange.endFrame = -1.0f;
        player->requestedRange.speed = player->range.speed;
        player->requestedRange.currentFrame = -1.0f;
        if (player->stateFlags & MOTION_PLAYER_STATE_2000) {
            fn_8041CD6C(control->rootNode, i);
        }
        player->stateFlags = 0;
    }
    control->flags &= 0xFFFFFF;
    memset(&control->vectors[6].value, 0, sizeof(Fn8040Vector3));
}

void fn_80418044(_stMtnPlayWork *control) {
    fn_8040B59C(TopMatrix(), control->vectors[0].value);
    fn_8040BAC4(TopMatrix(), &control->matrix);
    if (control->rootNode->definition->values[0] == 1 &&
        (control->rootNode->definition->values[1] & MOTION_NODE_SCALED_TRACK)) {
        int changedPosition = 0;
        int changedRotation = 0;
        Fn8040Vector3 offset;
        Fn8040Matrix34 result;
        Fn8040Matrix34 previous;

        for (int i = 0; i < control->playerList.count; i++) {
            int index = control->playerList.listedIndices[i];
            if (index == -1) {
                break;
            }
            MotionPlayer *player = &control->players[index];
            if (player->flags & MOTION_PLAYER_ACTIVE) {
                if (player->flags & MOTION_PLAYER_FLAG_100000) {
                    changedPosition = 1;
                }
                if (player->flags & MOTION_PLAYER_FLAG_200000) {
                    changedRotation = 1;
                }
                MotionScaledVectorTrack *track = (MotionScaledVectorTrack *)(player->tracks + 1);
                player->unknownE0.value = track->value;
                if ((player->flags & 0x1) || (player->stateFlags & MOTION_PLAYER_STATE_400)) {
                    fn_8040C204(lbl_805F13A8);
                    if (player->flags & MOTION_PLAYER_FLAG_80000) {
                        offset.x = -player->unknownE0.value.x;
                        offset.y = (control->flags & 0x1) ? -player->unknownE0.value.y : 0.0f;
                        offset.z = -player->unknownE0.value.z;
                        fn_8040B6F8(TopMatrix());
                        fn_8040B59C(TopMatrix(), player->unknownB0.value);
                        fn_8040BAC4(TopMatrix(), &player->unknown60);
                        fn_8040B59C(TopMatrix(), offset);
                        fn_804114D0(&player->unknownB0.value, 0);
                    } else {
                        offset.x = player->unknownD0.value.x - player->unknownE0.value.x;
                        offset.y = (control->flags & 0x1)
                                       ? player->unknownD0.value.y - player->unknownE0.value.y
                                       : 0.0f;
                        offset.z = player->unknownD0.value.z - player->unknownE0.value.z;
                        fn_8040C148(fn_80411700(lbl_805F13A8), &player->unknown00);
                        fn_8040B59C(TopMatrix(), offset);
                        fn_8040C148(&player->unknown00, fn_80411700(lbl_805F13A8));
                    }
                    fn_8040C244(lbl_805F13A8);
                }
            }
        }
        fn_8040C148(&result, fn_80411700(lbl_805F13A8));
        for (int i = 0; i < control->playerList.count; i++) {
            int index = control->playerList.listedIndices[i];
            if (index == -1) {
                break;
            }
            MotionPlayer *player = &control->players[index];
            if (i == 0 || (player->flags & MOTION_PLAYER_ACTIVE)) {
                fn_8040C204(lbl_805F13A8);
                if (player->flags & MOTION_PLAYER_FLAG_80000) {
                    fn_8040B6F8(TopMatrix());
                    fn_8040B59C(TopMatrix(), player->unknownB0.value);
                    fn_8040BAC4(TopMatrix(), &player->unknown60);
                } else {
                    if (changedPosition || changedRotation) {
                        Fn8040Matrix34 matrix;
                        Fn8040Matrix34 rotation;
                        fn_8040C148(&matrix, fn_80411700(lbl_805F13A8));
                        if (changedPosition) {
                            matrix.values[0][3] = control->vectors[2].value.x;
                            matrix.values[1][3] = control->vectors[2].value.y;
                            matrix.values[2][3] = control->vectors[2].value.z;
                        }
                        if (changedRotation) {
                            fn_8040C204(lbl_805F13A8);
                            fn_8040B6F8(TopMatrix());
                            fn_8040BAC4(TopMatrix(), &control->unknownC0);
                            fn_8040C148(&rotation, fn_80411700(lbl_805F13A8));
                            fn_8040C244(lbl_805F13A8);
                            fn_804113E8(&matrix, &rotation);
                            player->flags |= MOTION_PLAYER_FLAG_80000;
                            player->flags |= MOTION_PLAYER_FLAG_1000000;
                        }
                        fn_8040C148(fn_80411700(lbl_805F13A8), &matrix);
                    }
                    fn_8040BAC4(TopMatrix(), &player->unknown00);
                }
                fn_8040C148(&player->unknown00, fn_80411700(lbl_805F13A8));
                fn_8040C148(&result, &player->unknown00);
                fn_8040C148(&lbl_805E12E0[i], &result);
                fn_8040C244(lbl_805F13A8);
                if (i == 0) {
                    if (player->blend->flags & MOTION_BLEND_FLAG_1) {
                        fn_8040C204(lbl_805F13A8);
                        if (changedPosition || changedRotation) {
                            Fn8040Matrix34 matrix;
                            Fn8040Matrix34 rotation;
                            fn_8040C148(&matrix, fn_80411700(lbl_805F13A8));
                            if (changedPosition) {
                                matrix.values[0][3] = control->vectors[2].value.x;
                                matrix.values[1][3] = control->vectors[2].value.y;
                                matrix.values[2][3] = control->vectors[2].value.z;
                            }
                            if (changedRotation) {
                                fn_8040C204(lbl_805F13A8);
                                fn_8040B6F8(TopMatrix());
                                fn_8040BAC4(TopMatrix(), &control->unknownC0);
                                fn_8040C148(&rotation, fn_80411700(lbl_805F13A8));
                                fn_8040C244(lbl_805F13A8);
                                fn_804113E8(&matrix, &rotation);
                                control->flags |= 0x10000;
                            }
                            fn_8040C148(fn_80411700(lbl_805F13A8), &matrix);
                        }
                        fn_8040BAC4(TopMatrix(), &control->unknown30);
                        fn_8040C148(&control->unknown30, fn_80411700(lbl_805F13A8));
                        fn_8040C148(&lbl_805E12E0[4], &control->unknown30);
                        fn_8040C244(lbl_805F13A8);
                        fn_80420590(&result, &result, &control->unknown30,
                                    player->blend->ratioSecond);
                    }
                } else {
                    fn_80420590(&result, &result, &previous, player->blend->ratioSecond);
                }
                fn_8040C148(&previous, &result);
            }
        }
        fn_8040C148(fn_80411700(lbl_805F13A8), &result);
    }
    for (int i = 0; i < control->playerList.count; i++) {
        int index = control->playerList.listedIndices[i];
        if (index == -1) {
            break;
        }
        lbl_805E14F0[index] = control->players[index].blend->ratioSecond;
    }
    fn_804186A0(control, control->rootNode);
}

void fn_804186A0(_stMtnPlayWork *control, MotionNode *node) {
    int changedRatios = 0;
    MotionNodeStep *step = node->steps;
    /* When restoration runs, it reads every listed slot, including slots not saved above. */
    float ratios[4];

    if (node->second != 0) {
        fn_8040C204(lbl_805F13A8);
    }
    for (int i = 0; i < control->playerList.count; i++) {
        int index = control->playerList.listedIndices[i];
        if (index == -1) {
            break;
        }
        MotionNodeEntry *entry = &node->entries[index];
        if (entry->unknown00 != 0) {
            float factor = entry->unknown08;
            float offset = entry->unknown0C;
            float ratio = node->blends[index].ratioSecond;
            ratios[index] = lbl_805E14F0[index];
            lbl_805E14F0[index] = ratio * factor + offset;
            changedRatios = 1;
        }
    }
    for (; step != 0; step = step->next) {
        if (step->apply != 0) {
            step->apply(&step->data);
        }
    }
    if (node->callback != 0) {
        MotionNodeCallbackData data;
        data.nodeId = node->definition->id;
        data.matrix = TopMatrix();
        data.argument = node->callbackArgument;
        node->callback(&data);
    }
    if (node->transform != 0) {
        fn_8040C148(&node->transform->matrix, fn_80411700(lbl_805F13A8));
    }
    if (control->flags & 0x2) {
        if (node->second != 0) {
            fn_8040C244(lbl_805F13A8);
        }
    } else {
        if (node->first != 0) {
            fn_804186A0(control, node->first);
        }
        if (changedRatios) {
            for (int i = 0; i < control->playerList.count; i++) {
                int index = control->playerList.listedIndices[i];
                if (index == -1) {
                    break;
                }
                lbl_805E14F0[index] = ratios[index];
            }
        }
        if (node->second != 0) {
            fn_8040C244(lbl_805F13A8);
            fn_804186A0(control, node->second);
        }
    }
}

/* For each listed active player: selects the curve evaluators for the direction of its speed,
 * hands its range to the curve state (with the current frame truncated to a whole number when the
 * control has MOTION_CONTROL_WHOLE_FRAMES) and runs the callbacks of the tracks of the player. */
void fn_804188A0(_stMtnPlayWork *control) {
    int i;
    MotionPlayer *player;
    MotionTrackNode *track;

    for (i = 0; i < control->playerList.count; i++) {
        int playerIndex = control->playerList.listedIndices[i];

        if (playerIndex == -1) {
            break;
        }
        player = &control->players[playerIndex];
        if (player->flags & MOTION_PLAYER_ACTIVE) {
            if (player->range.speed >= 0.0f) {
                motionControlCurveEvaluator0 = fn_8041525C;
                motionControlCurveEvaluator1 = fn_80414FD0;
                motionControlCurveBothEvaluator = fn_80414CBC;
            } else if (player->range.speed < 0.0f) {
                motionControlCurveEvaluator0 = fn_804149E0;
                motionControlCurveEvaluator1 = fn_8041474C;
                motionControlCurveBothEvaluator = fn_80414438;
            }

            if (control->flags & MOTION_CONTROL_WHOLE_FRAMES) {
                MotionRange range = player->range;

                range.currentFrame = (float)(int)range.currentFrame;
                fn_8041575C(&range);
            } else {
                fn_8041575C(&player->range);
            }

            for (track = player->tracks; track != 0; track = track->next) {
                if (track->first != 0) {
                    track->first(track + 1);
                }
            }
            if (player->flags & MOTION_PLAYER_FLAG_8) {
                for (track = player->tracks; track != 0; track = track->next) {
                    if (track->second != 0) {
                        track->second(track + 1);
                    }
                }
            }
        }
    }
}

void fn_80418A4C(_stMtnPlayWork *control) {
    for (int i = 0; i < control->playerList.count; i++) {
        int index = control->playerList.listedIndices[i];
        if (index == -1) {
            break;
        }
        MotionPlayer *player = &control->players[index];
        if (!(player->flags & MOTION_PLAYER_ACTIVE)) {
            continue;
        }
        if (player->blend->flags & MOTION_BLEND_FLAG_1) {
            switch (player->blend->kindSecond) {
            case 0:
                player->blend->unknown20 += player->blend->speed * lbl_805EDD60;
                player->blend->ratioSecond = player->blend->unknown20 / player->blend->frames;
                if (player->blend->ratioSecond > 0.9999f) {
                    player->flags |= MOTION_PLAYER_FLAG_800000;
                    int position = -1;
                    for (int j = 0; j < control->playerList.count; j++) {
                        int listedIndex = control->playerList.listedIndices[j];
                        if (listedIndex == -1) {
                            break;
                        }
                        if (position == -1) {
                            if (listedIndex == index) {
                                position = j;
                            } else {
                                control->players[listedIndex].blend->flags &= ~MOTION_BLEND_FLAG_2;
                                control->players[listedIndex].flags &= ~0x1;
                                control->players[listedIndex].flags &=
                                    ~MOTION_PLAYER_NUDGE_AT_LIMIT;
                                control->players[listedIndex].flags &= ~MOTION_PLAYER_ACTIVE;
                                if (control->players[listedIndex].flags & MOTION_PLAYER_FLAG_10) {
                                    fn_8041CC18(control->rootNode, listedIndex);
                                    control->players[listedIndex].flags &= ~MOTION_PLAYER_FLAG_10;
                                }
                                if (control->players[listedIndex].motionId != 0) {
                                    fn_80413240(control->players[listedIndex].motionId);
                                }
                                for (int k = 0; k < control->playerList.count; k++) {
                                    if (control->playerList.playerIndexOfId[k] == -1) {
                                        break;
                                    }
                                    if (listedIndex == control->playerList.playerIndexOfId[k]) {
                                        control->playerList.playerIndexOfId[k] = -1;
                                        for (; k < control->playerList.count - 1; k++) {
                                            control->playerList.playerIndexOfId[k] =
                                                control->playerList.playerIndexOfId[k + 1];
                                            control->playerList.playerIndexOfId[k + 1] = -1;
                                        }
                                        break;
                                    }
                                }
                            }
                            control->playerList.unknown0C[listedIndex] = -1;
                            control->players[listedIndex].blend->flags &= ~MOTION_BLEND_FLAG_1;
                        }
                        control->playerList.listedIndices[j] = -1;
                        if (position != -1) {
                            control->playerList.listedIndices[j - position] = listedIndex;
                        }
                    }
                    i = 0;
                    if (control->playerList.listedIndices[1] == -1) {
                        fn_8041966C(control->rootNode);
                    }
                }
                break;
            case 1: {
                if (player->blend->ratioSecond < 0.0f) {
                    player->blend->ratioSecond = 0.0f;
                }
                if (player->blend->ratioSecond > 1.0f) {
                    player->blend->ratioSecond = 1.0f;
                }
                int otherIndex = control->playerList.unknown0C[index];
                if (otherIndex != -1) {
                    MotionPlayer *other = &control->players[otherIndex];
                    if (other->flags & MOTION_PLAYER_ACTIVE) {
                        if (other->flags & 0x1) {
                            player->flags |= 0x1;
                        } else {
                            player->flags &= ~0x1;
                        }
                        if (other->flags & MOTION_PLAYER_NUDGE_AT_LIMIT) {
                            player->flags |= MOTION_PLAYER_NUDGE_AT_LIMIT;
                        } else {
                            player->flags &= ~MOTION_PLAYER_NUDGE_AT_LIMIT;
                        }
                        if (other->stateFlags & MOTION_PLAYER_STATE_400) {
                            player->stateFlags |= MOTION_PLAYER_STATE_400;
                        } else {
                            player->stateFlags &= ~MOTION_PLAYER_STATE_400;
                        }
                        float frame = (player->range.endFrame - player->range.startFrame) *
                                          ((other->range.currentFrame - other->range.startFrame) /
                                           (other->range.endFrame - other->range.startFrame)) +
                                      player->range.startFrame;
                        if (other->range.speed >= 0.0f) {
                            if (frame < player->range.currentFrame) {
                                player->range.speed =
                                    player->range.endFrame - player->range.currentFrame;
                                player->range.speed += frame - player->range.startFrame;
                            } else {
                                player->range.speed = frame - player->range.currentFrame;
                            }
                        } else if (other->range.speed < 0.0f) {
                            if (frame > player->range.currentFrame) {
                                player->range.speed =
                                    player->range.startFrame - player->range.currentFrame;
                                player->range.speed += frame - player->range.endFrame;
                            } else {
                                player->range.speed = frame - player->range.currentFrame;
                            }
                        }
                        if (lbl_805EDD60 > MOTION_LIMIT_EPSILON) {
                            player->range.speed /= lbl_805EDD60;
                        }
                        player->flags &= ~MOTION_PLAYER_ZERO_SPEED;
                    }
                }
                break;
            }
            default:
                if (player->blend->ratioSecond < 0.0f) {
                    player->blend->ratioSecond = 0.0f;
                }
                if (player->blend->ratioSecond > 1.0f) {
                    player->blend->ratioSecond = 1.0f;
                }
                break;
            }
        }

        int resetCurves = 0;
        if (player->flags & MOTION_PLAYER_FLAG_40000) {
            resetCurves = 1;
        }
        if (!(player->flags & MOTION_PLAYER_ZERO_SPEED)) {
            player->range.currentFrame += player->range.speed * lbl_805EDD60;
            if (player->flags & MOTION_PLAYER_NUDGE_AT_LIMIT) {
                if (fn_8000A980(player->range.endFrame - player->range.startFrame) <
                    MOTION_LIMIT_EPSILON) {
                    player->range.currentFrame = player->range.endFrame;
                } else {
                    if (player->range.speed > 0.0f) {
                        if (player->range.currentFrame >= player->range.endFrame) {
                            if (control->rootNode->definition->values[0] == 1 &&
                                (control->rootNode->definition->values[1] &
                                 MOTION_NODE_SCALED_TRACK)) {
                                MotionRange range = player->range;
                                Fn8040Vector3 start;
                                Fn8040Vector3 end;
                                Fn8040Vector3 offset;
                                range.currentFrame = range.endFrame;
                                fn_80419BA8(&end, player, &range, resetCurves);
                                range.currentFrame = range.startFrame;
                                fn_80419BA8(&start, player, &range, 1);
                                offset = fn_8040CD74(&end, &start);
                                if (!(control->flags & 0x1)) {
                                    offset.y = 0.0f;
                                }
                                fn_8040C204(lbl_805F13A8);
                                fn_8040C148(fn_80411700(lbl_805F13A8), &player->unknown00);
                                while (player->range.currentFrame >= player->range.endFrame) {
                                    player->range.currentFrame -=
                                        player->range.endFrame - player->range.startFrame;
                                    player->unknownD0.value =
                                        fn_8040CD74(&player->unknownD0.value, &offset);
                                    fn_8040B59C(TopMatrix(), offset);
                                }
                                fn_8040C148(&player->unknown00, fn_80411700(lbl_805F13A8));
                                fn_8040C244(lbl_805F13A8);
                            } else {
                                while (player->range.currentFrame >= player->range.endFrame) {
                                    player->range.currentFrame -=
                                        player->range.endFrame - player->range.startFrame;
                                }
                            }
                            resetCurves = 1;
                            player->flags |= MOTION_PLAYER_FLAG_400000;
                        }
                    } else if (player->range.speed < 0.0f) {
                        if (player->range.currentFrame <= player->range.startFrame) {
                            if (control->rootNode->definition->values[0] == 1 &&
                                (control->rootNode->definition->values[1] &
                                 MOTION_NODE_SCALED_TRACK)) {
                                MotionRange range = player->range;
                                Fn8040Vector3 start;
                                Fn8040Vector3 end;
                                Fn8040Vector3 offset;
                                range.currentFrame = range.startFrame;
                                fn_80419BA8(&start, player, &range, resetCurves);
                                range.currentFrame = range.endFrame;
                                fn_80419BA8(&end, player, &range, 1);
                                offset = fn_8040CD74(&start, &end);
                                if (!(control->flags & 0x1)) {
                                    offset.y = 0.0f;
                                }
                                fn_8040C204(lbl_805F13A8);
                                fn_8040C148(fn_80411700(lbl_805F13A8), &player->unknown00);
                                while (player->range.currentFrame <= player->range.startFrame) {
                                    player->range.currentFrame +=
                                        player->range.endFrame - player->range.startFrame;
                                    player->unknownD0.value =
                                        fn_8040CD74(&player->unknownD0.value, &offset);
                                    fn_8040B59C(TopMatrix(), offset);
                                }
                                fn_8040C148(&player->unknown00, fn_80411700(lbl_805F13A8));
                                fn_8040C244(lbl_805F13A8);
                            } else {
                                while (player->range.currentFrame <= player->range.startFrame) {
                                    player->range.currentFrame +=
                                        player->range.endFrame - player->range.startFrame;
                                }
                            }
                            resetCurves = 1;
                            player->flags |= MOTION_PLAYER_FLAG_400000;
                        }
                    }
                }
            } else if (player->range.speed >= 0.0f) {
                if (player->range.currentFrame >= player->range.endFrame) {
                    player->range.currentFrame = player->range.endFrame;
                    player->flags |= MOTION_PLAYER_FLAG_400000;
                }
            } else if (player->range.speed < 0.0f) {
                if (player->range.currentFrame <= player->range.startFrame) {
                    player->range.currentFrame = player->range.startFrame;
                    player->flags |= MOTION_PLAYER_FLAG_400000;
                }
            }
        }
        if (resetCurves) {
            fn_8041AED0(control, index);
        }
    }
}

void fn_804192EC(_stMtnPlayWork *control) {
    int changedMotion = 0;
    int changedBlend = 0;

    for (int i = 0; i < control->playerList.count; i++) {
        int index = control->playerList.listedIndices[i];
        if (index == -1) {
            break;
        }
        MotionPlayer *player = &control->players[index];
        if (player->stateFlags & MOTION_PLAYER_REQUESTED) {
            if (player->requestedMotionId != 0) {
                fn_80419D5C(control, index);
                if (player->blend->flags & MOTION_BLEND_FLAG_1) {
                    changedBlend = 1;
                }
                changedMotion = 1;
            }
            if (player->flags & MOTION_PLAYER_ACTIVE) {
                if (player->stateFlags &
                    (MOTION_PLAYER_REQUEST_SPEED | MOTION_PLAYER_REQUEST_START_FRAME |
                     MOTION_PLAYER_REQUEST_END_FRAME | MOTION_PLAYER_REQUEST_CURRENT_FRAME)) {
                    if (!(player->stateFlags & MOTION_PLAYER_REQUEST_SPEED)) {
                        player->requestedRange.speed = player->range.speed;
                    }
                    if (!(player->stateFlags & MOTION_PLAYER_REQUEST_START_FRAME)) {
                        player->requestedRange.startFrame = -1.0f;
                    }
                    if (!(player->stateFlags & MOTION_PLAYER_REQUEST_END_FRAME)) {
                        player->requestedRange.endFrame = -1.0f;
                    }
                    if (!(player->stateFlags & MOTION_PLAYER_REQUEST_CURRENT_FRAME)) {
                        player->requestedRange.currentFrame = -1.0f;
                    }
                    int result = fn_8041AD94(&player->range, &player->requestedRange,
                                             player->firstFrame, player->lastFrame);
                    if (result & 0x1) {
                        player->flags |= MOTION_PLAYER_FLAG_40000;
                    }
                    if (result & 0x10) {
                        player->flags |= MOTION_PLAYER_ZERO_SPEED;
                    }
                }
                if (player->stateFlags & MOTION_PLAYER_STATE_2000) {
                    fn_8041CE74(control->rootNode, index);
                    player->flags |= MOTION_PLAYER_FLAG_10;
                }
                if (player->flags & MOTION_PLAYER_FLAG_10000) {
                    fn_80419BA8(&player->unknownD0.value, player, &player->range, 0);
                    if (player->blend->flags & MOTION_BLEND_FLAG_1) {
                        fn_804198E4(control, index);
                    }
                }
            }
        }
    }
    if (changedBlend) {
        fn_804194B8(control->rootNode);
    } else if (changedMotion) {
        fn_8041966C(control->rootNode);
    }
}

void fn_804194B8(MotionNode *node) {
    MotionNodeStep *step = node->steps;
    if (node->definition->values[0] == 1 &&
        (node->definition->values[1] & MOTION_NODE_SCALED_TRACK)) {
        if (node->flags & MOTION_NODE_TRACK_400) {
            step->apply = fn_8041E618;
            step = step->next;
        }
        if (node->flags & MOTION_NODE_SCALED_TRACK) {
            step->apply = fn_8041F290;
        } else {
            step->apply = fn_8041FA20;
        }
        step = step->next;
        if (!(node->flags & MOTION_NODE_TRACK_40)) {
            if (node->flags & MOTION_NODE_REFLECTED_TRACK) {
                step->apply = fn_8041ED8C;
            } else {
                step->apply = fn_8041F7D0;
            }
        }
        step = step->next;
        if (node->flags & MOTION_NODE_TRACK_380) {
            step->apply = fn_8041E9C4;
        } else {
            step->apply = fn_8041F6F0;
        }
    } else {
        if (node->flags & MOTION_NODE_TRACK_400) {
            step->apply = fn_8041D580;
            step = step->next;
        }
        if (node->flags & MOTION_NODE_SCALED_TRACK) {
            step->apply = fn_8041E124;
        } else {
            step->apply = fn_8041E5E0;
        }
        step = step->next;
        if (!(node->flags & MOTION_NODE_TRACK_40)) {
            if (node->flags & MOTION_NODE_REFLECTED_TRACK) {
                step->apply = fn_8041DBD4;
            } else {
                step->apply = fn_8041E480;
            }
        }
        step = step->next;
        if (node->flags & MOTION_NODE_TRACK_380) {
            step->apply = fn_8041D89C;
        } else {
            step->apply = fn_8041E420;
        }
    }
    if (node->first != 0) {
        fn_804194B8(node->first);
    }
    if (node->second != 0) {
        fn_804194B8(node->second);
    }
}

void fn_8041966C(MotionNode *node) {
    MotionNodeStep *step = node->steps;
    if (node->flags & MOTION_NODE_TRACK_400) {
        step->apply = fn_8041FAD8;
        step = step->next;
    }
    if (node->flags & MOTION_NODE_SCALED_TRACK) {
        step->apply = fn_8041FCC0;
    } else {
        step->apply = fn_8041FED0;
    }
    step = step->next;
    if (!(node->flags & MOTION_NODE_TRACK_40)) {
        if (node->flags & MOTION_NODE_REFLECTED_TRACK) {
            step->apply = fn_8041FB4C;
        } else {
            step->apply = fn_8041FD70;
        }
    }
    step = step->next;
    if (node->flags & MOTION_NODE_TRACK_380) {
        step->apply = fn_8041FAD8;
    } else {
        step->apply = fn_8041FD0C;
    }
    if (node->first != 0) {
        fn_8041966C(node->first);
    }
    if (node->second != 0) {
        fn_8041966C(node->second);
    }
}

void fn_804198E4(_stMtnPlayWork *control, int playerIndex) {
    MotionPlayer *player = &control->players[playerIndex];

    if (player->blend->kindSecond == 1) {
        int otherIndex = control->playerList.unknown0C[playerIndex];
        if (otherIndex != -1) {
            MotionPlayer *other = &control->players[otherIndex];
            if (other->flags & MOTION_PLAYER_ACTIVE) {
                MotionRange range = player->range;
                Fn8040Vector3 offset;
                range.currentFrame = (player->range.endFrame - player->range.startFrame) *
                                         ((other->range.currentFrame - other->range.startFrame) /
                                          (other->range.endFrame - other->range.startFrame)) +
                                     player->range.startFrame;
                if (other->flags & MOTION_PLAYER_FLAG_80000) {
                    fn_80419BA8(&other->unknownD0.value, other, &other->range, 0);
                    fn_80419BA8(&player->unknownD0.value, player, &range, 0);
                    if ((other->flags & 0x1) || (other->stateFlags & MOTION_PLAYER_STATE_400)) {
                        player->unknownB0.value = other->unknownB0.value;
                    } else {
                        fn_8040C204(lbl_805F13A8);
                        fn_8040B6F8(TopMatrix());
                        fn_8040B59C(TopMatrix(), other->unknownB0.value);
                        fn_8040BAC4(TopMatrix(), &other->unknown60);
                        fn_8040B59C(TopMatrix(), other->unknownD0.value);
                        offset.Set(-player->unknownD0.value.x, -player->unknownD0.value.y,
                                   -player->unknownD0.value.z);
                        fn_8040B59C(TopMatrix(), offset);
                        fn_804114D0(&player->unknownB0.value, 0);
                        fn_8040C244(lbl_805F13A8);
                    }
                    player->unknownC0.value = other->unknownC0.value;
                    fn_8040C148(&player->unknown60, &other->unknown60);
                } else {
                    fn_8040C204(lbl_805F13A8);
                    fn_8040C148(fn_80411700(lbl_805F13A8), &other->unknown00);
                    fn_80419BA8(&offset, other, &other->range, 0);
                    fn_8040B59C(TopMatrix(), offset);
                    fn_80419BA8(&player->unknownD0.value, player, &range, 0);
                    offset.Set(-player->unknownD0.value.x, -player->unknownD0.value.y,
                               -player->unknownD0.value.z);
                    fn_8040B59C(TopMatrix(), offset);
                    fn_8040C148(&player->unknown00, fn_80411700(lbl_805F13A8));
                    fn_8040C244(lbl_805F13A8);
                    player->flags &= ~MOTION_PLAYER_FLAG_80000;
                }
            }
        }
    }
}

void fn_80419BA8(Fn8040Vector3 *result, MotionPlayer *player, MotionRange *range, int reset) {
    MotionScaledVectorTrack *track = (MotionScaledVectorTrack *)(player->tracks + 1);
    MotionKeyCursor *curves = track->curves;

    if (range->speed >= 0.0f) {
        motionControlCurveEvaluator0 = fn_8041525C;
        motionControlCurveEvaluator1 = fn_80414FD0;
        motionControlCurveBothEvaluator = fn_80414CBC;
    } else if (range->speed < 0.0f) {
        motionControlCurveEvaluator0 = fn_804149E0;
        motionControlCurveEvaluator1 = fn_8041474C;
        motionControlCurveBothEvaluator = fn_80414438;
    }
    fn_8041575C(range);
    *result = track->value;
    if (curves[0].position != 0) {
        if (reset) {
            fn_80413B30(&curves[0]);
        }
        result->x = motionControlCurveEvaluator0(&curves[0]);
        if (track->scale > 0.0f) {
            result->x *= track->scale;
        }
        if (player->flags & MOTION_PLAYER_FLAG_8) {
            result->x = -result->x;
        }
    }
    if (curves[1].position != 0) {
        if (reset) {
            fn_80413B30(&curves[1]);
        }
        result->y = motionControlCurveEvaluator0(&curves[1]);
        if (track->scale > 0.0f) {
            result->y *= track->scale;
        }
    }
    if (curves[2].position != 0) {
        if (reset) {
            fn_80413B30(&curves[2]);
        }
        result->z = motionControlCurveEvaluator0(&curves[2]);
        if (track->scale > 0.0f) {
            result->z *= track->scale;
        }
    }
}

void fn_80419D5C(_stMtnPlayWork *control, int playerIndex) {
    MotionPlayer *player = &control->players[playerIndex];

    fn_80413318(player->requestedMotionId);
    if (player->flags & MOTION_PLAYER_ACTIVE) {
        fn_80413240(player->motionId);
    }
    memset(&player->unknownD0.value, 0, sizeof(Fn8040Vector3));
    if (!(player->stateFlags & MOTION_PLAYER_STATE_2)) {
        player->unknownB0.value = control->vectors[0].value;
    } else {
        player->flags |= MOTION_PLAYER_FLAG_100000;
    }
    if (!(player->stateFlags & MOTION_PLAYER_STATE_4)) {
        player->unknownC0.value = control->vectors[1].value;
        fn_8040C148(&player->unknown60, &control->matrix);
    } else {
        player->flags |= MOTION_PLAYER_FLAG_200000;
    }
    player->flags |= MOTION_PLAYER_ACTIVE;
    player->flags |= MOTION_PLAYER_FLAG_10000;
    player->flags |= MOTION_PLAYER_ZERO_SPEED;
    player->flags |= MOTION_PLAYER_FLAG_80000;
    player->flags |= MOTION_PLAYER_FLAG_1000000;
    if (!fn_8041B170(control, playerIndex, player->requestedMotionId)) {
        fn_80413240(player->requestedMotionId);
        player->motionId = 0;
    } else {
        player->motionId = player->requestedMotionId;
        if (!(player->stateFlags & MOTION_PLAYER_STATE_800)) {
            if (fn_80411DEC(player->motionId, 2, 0, 0, 0)) {
                player->flags |= MOTION_PLAYER_FLAG_8;
            } else {
                player->flags &= ~MOTION_PLAYER_FLAG_8;
            }
        } else {
            if (fn_80411DEC(player->motionId, 2, 0, 0, 0)) {
                player->flags &= ~MOTION_PLAYER_FLAG_8;
            } else {
                player->flags |= MOTION_PLAYER_FLAG_8;
            }
        }
        if (!(player->stateFlags & MOTION_PLAYER_STATE_1000)) {
            if (fn_80411DEC(player->motionId, 1, 0, 0, 0)) {
                player->flags |= MOTION_PLAYER_NUDGE_AT_LIMIT;
            } else {
                player->flags &= ~MOTION_PLAYER_NUDGE_AT_LIMIT;
            }
        }
    }
}

void fn_80419F5C(_stMtnPlayWork *control) {
    for (int i = 0; i < control->playerList.count; i++) {
        int index = control->playerList.listedIndices[i];
        if (index == -1) {
            break;
        }
        MotionPlayer *player = &control->players[index];
        if (player->blend->flags & MOTION_BLEND_FLAG_2) {
            fn_8041A440(control);
            int position = -1;
            for (int j = 0; j < control->playerList.count * 2; j++) {
                int listedIndex = control->playerList.listedIndices[j];
                if (listedIndex == -1) {
                    break;
                }
                if (position == -1) {
                    if (j > i && (!(control->players[listedIndex].flags & MOTION_PLAYER_ACTIVE) ||
                                  control->players[listedIndex].blend->kindSecond == 0)) {
                        position = j;
                    }
                    if (position == -1) {
                        control->playerList.unknown0C[listedIndex] =
                            control->playerList.unknown0C[listedIndex + control->playerList.count];
                        control->playerList.unknown0C[listedIndex + control->playerList.count] = -1;
                        control->players[listedIndex].blend->flags &= ~MOTION_BLEND_FLAG_2;
                        if (!(control->players[listedIndex].stateFlags &
                              MOTION_PLAYER_STATE_4000)) {
                            control->players[listedIndex].flags &= ~0x1;
                        }
                        if (!(control->players[listedIndex].stateFlags &
                              MOTION_PLAYER_STATE_1000)) {
                            control->players[listedIndex].flags &= ~MOTION_PLAYER_NUDGE_AT_LIMIT;
                        }
                        control->players[listedIndex].flags &= ~MOTION_PLAYER_ACTIVE;
                        if (control->players[listedIndex].flags & MOTION_PLAYER_FLAG_10) {
                            fn_8041CC18(control->rootNode, listedIndex);
                            control->players[listedIndex].flags &= ~MOTION_PLAYER_FLAG_10;
                        }
                        if (control->players[listedIndex].motionId != 0) {
                            fn_80413240(control->players[listedIndex].motionId);
                        }
                    }
                }
                control->playerList.listedIndices[j] = -1;
                if (position != -1) {
                    control->playerList.listedIndices[j - position] = listedIndex;
                }
            }
            i = -1;
        } else if (player->stateFlags & MOTION_PLAYER_REQUESTED) {
            if (player->stateFlags & MOTION_PLAYER_BLEND_FIRST) {
                int blend = 1;
                switch (player->blend->kindFirst) {
                case 1:
                case 2:
                    break;
                default:
                    if (player->blend->frames < MOTION_LIMIT_EPSILON) {
                        blend = 0;
                    }
                    break;
                }
                if (player->requestedMotionId == 0) {
                    player->requestedMotionId = 1;
                }
                if (blend) {
                    player->blend->kindSecond = player->blend->kindFirst;
                    player->blend->ratioSecond = player->blend->ratioFirst;
                    switch (player->blend->kindSecond) {
                    case 1:
                    case 2:
                        break;
                    case 0:
                    default:
                        player->blend->unknown20 = 0.0f;
                        break;
                    }
                    player->blend->flags |= MOTION_BLEND_FLAG_1;
                } else {
                    player->stateFlags &= ~MOTION_PLAYER_BLEND_FIRST;
                    player->blend->flags &= ~MOTION_BLEND_FLAG_2;
                }
            }
            if (player->requestedMotionId != 0 &&
                !(player->stateFlags & MOTION_PLAYER_BLEND_FIRST) &&
                !(player->stateFlags & MOTION_PLAYER_STATE_8000)) {
                int position = -1;
                for (int j = 0; j < control->playerList.count * 2; j++) {
                    int listedIndex = control->playerList.listedIndices[j];
                    if (listedIndex == -1) {
                        break;
                    }
                    if (position == -1) {
                        if (listedIndex == index) {
                            position = j;
                        } else {
                            control->players[listedIndex].blend->flags &= ~MOTION_BLEND_FLAG_2;
                            if (!(control->players[listedIndex].stateFlags &
                                  MOTION_PLAYER_STATE_4000)) {
                                control->players[listedIndex].flags &= ~0x1;
                            }
                            if (!(control->players[listedIndex].stateFlags &
                                  MOTION_PLAYER_STATE_1000)) {
                                control->players[listedIndex].flags &=
                                    ~MOTION_PLAYER_NUDGE_AT_LIMIT;
                            }
                            control->players[listedIndex].flags &= ~MOTION_PLAYER_ACTIVE;
                            if (control->players[listedIndex].flags & MOTION_PLAYER_FLAG_10) {
                                fn_8041CC18(control->rootNode, listedIndex);
                                control->players[listedIndex].flags &= ~MOTION_PLAYER_FLAG_10;
                            }
                            if (control->players[listedIndex].motionId != 0) {
                                fn_80413240(control->players[listedIndex].motionId);
                            }
                            for (int k = 0; k < control->playerList.count; k++) {
                                if (control->playerList.playerIndexOfId[k] == -1) {
                                    break;
                                }
                                if (listedIndex == control->playerList.playerIndexOfId[k]) {
                                    control->playerList.playerIndexOfId[k] = -1;
                                    for (; k < control->playerList.count - 1; k++) {
                                        control->playerList.playerIndexOfId[k] =
                                            control->playerList.playerIndexOfId[k + 1];
                                        control->playerList.playerIndexOfId[k + 1] = -1;
                                    }
                                    break;
                                }
                            }
                        }
                        control->playerList.unknown0C[listedIndex] = -1;
                        control->players[listedIndex].blend->flags &= ~MOTION_BLEND_FLAG_1;
                    }
                    control->playerList.listedIndices[j] = -1;
                    if (position != -1) {
                        control->playerList.listedIndices[j - position] = listedIndex;
                    }
                }
                i = 0;
            }
        }
    }
}

void fn_8041A440(_stMtnPlayWork *control) {
    int playerCount = control->playerList.count;
    int count;
    for (count = 1; count < playerCount * 2; count++) {
        int index = control->playerList.listedIndices[count];
        if (index == -1) {
            break;
        }
        MotionPlayer *player = &control->players[index];
        if (!(player->flags & MOTION_PLAYER_ACTIVE) || player->blend->kindSecond == 0) {
            break;
        }
    }
    control->playerList.count = count;
    for (int i = 0; i < control->playerList.count; i++) {
        int index = control->playerList.listedIndices[i];
        if (index == -1) {
            break;
        }
        MotionPlayer *player = &control->players[index];
        if (i == 0 && (player->blend->flags & MOTION_BLEND_FLAG_1)) {
            fn_8040C148(&lbl_805E12E0[4], &control->unknown30);
        }
        fn_8040C148(&lbl_805E12E0[i], &player->unknown00);
    }
    for (int i = 0; i < control->playerList.count; i++) {
        int index = control->playerList.listedIndices[i];
        if (index == -1) {
            break;
        }
        lbl_805E14D0[index] = 0;
        lbl_805E14E0[index] = 0;
    }
    int pending;
    do {
        pending = 0;
        for (int i = 0; i < control->playerList.count; i++) {
            int index = control->playerList.listedIndices[i];
            if (index == -1) {
                break;
            }
            if (pending && control->playerList.unknown0C[index] == -1) {
                lbl_805E14C0[index] = 1;
            } else {
                lbl_805E14C0[index] = 0;
                for (int j = i + 1; j < control->playerList.count; j++) {
                    int otherIndex = control->playerList.listedIndices[j];
                    if (otherIndex == -1) {
                        break;
                    }
                    if (lbl_805E14D0[otherIndex] == 0 &&
                        index == control->playerList.unknown0C[otherIndex]) {
                        pending = 1;
                        lbl_805E14C0[index] = 1;
                        break;
                    }
                }
            }
        }
        for (int i = 0; i < control->playerList.count; i++) {
            int index = control->playerList.listedIndices[i];
            if (index == -1) {
                break;
            }
            if (lbl_805E14D0[index] == 0) {
                MotionPlayer *player = &control->players[index];
                if (lbl_805E14C0[index] != 0) {
                    if (lbl_805E14E0[index] == 0) {
                        fn_8040C148(&lbl_805E13D0[index], &player->unknown00);
                        lbl_805E14E0[index] = 1;
                    }
                } else {
                    lbl_805E14D0[index] = 1;
                    int otherIndex = control->playerList.unknown0C[index];
                    Fn8040Matrix34 *destination;
                    if (otherIndex == -1) {
                        destination = &control->unknown30;
                    } else {
                        destination = &lbl_805E13D0[otherIndex];
                        lbl_805E14E0[otherIndex] = 1;
                    }
                    Fn8040Matrix34 *matrix = &player->unknown00;
                    if (lbl_805E14E0[index] != 0) {
                        matrix = &lbl_805E13D0[index];
                    }
                    if (player->blend->flags & MOTION_BLEND_FLAG_1) {
                        Fn8040Matrix34 *other = destination;
                        if (i == 0) {
                            other = &control->unknown30;
                        }
                        fn_80420590(destination, matrix, other, lbl_805E14F0[index]);
                    } else {
                        fn_8040C148(destination, matrix);
                    }
                }
            }
        }
    } while (pending);

    control->flags |= 0x10000;
    for (int i = 0; i < control->playerList.count; i++) {
        int index = control->playerList.listedIndices[i];
        if (index == -1) {
            break;
        }
        lbl_805E14F0[index] = control->players[index].blend->ratioSecond;
    }
    fn_8040C204(lbl_805F13A8);
    fn_8040C148(fn_80411700(lbl_805F13A8), &control->unknown30);
    fn_8041A83C(control, control->rootNode);
    fn_8040C244(lbl_805F13A8);
    control->playerList.count = playerCount;
}

void fn_8041A83C(_stMtnPlayWork *control, MotionNode *node) {
    int changedRatios = 0;
    int useInverse = 0;
    MotionNodeStep *step = node->steps;
    Fn8040Matrix34 inverse;
    Fn8040Matrix34 scaleMatrix;
    Fn8040Matrix34 matrix;
    /* When restoration runs, it reads every listed slot, including slots not saved above. */
    float ratios[4];

    if (node->definition->values[0] == 1 &&
        (node->definition->values[1] & MOTION_NODE_SCALED_TRACK)) {
        useInverse = 1;
        fn_8040C204(lbl_805F13A8);
        fn_8040BCD4(TopMatrix());
        fn_8040C148(&inverse, fn_80411700(lbl_805F13A8));
        fn_8040C244(lbl_805F13A8);
    }
    for (int i = 0; i < control->playerList.count; i++) {
        int index = control->playerList.listedIndices[i];
        if (index == -1) {
            break;
        }
        MotionNodeEntry *entry = &node->entries[index];
        if (entry->unknown00 != 0) {
            float factor = entry->unknown08;
            float offset = entry->unknown0C;
            float ratio = node->blends[index].ratioSecond;
            ratios[index] = lbl_805E14F0[index];
            lbl_805E14F0[index] = ratio * factor + offset;
            changedRatios = 1;
        }
    }
    if (node->transform != 0) {
        if (!useInverse) {
            fn_8040B6F8(TopMatrix());
        }
        MotionTransformState *state = step->data.state;
        if (node->flags & MOTION_NODE_TRACK_400) {
            fn_8040C204(lbl_805F13A8);
            fn_8040B6F8(TopMatrix());
            if (step->apply != 0) {
                step->apply(&step->data);
            }
            fn_8040C148(&scaleMatrix, fn_80411700(lbl_805F13A8));
            fn_80411328(&state->unknown30.value, &scaleMatrix);
            fn_8040C244(lbl_805F13A8);
            step = step->next;
        } else {
            state->unknown30.value.x = 1.0f;
            state->unknown30.value.y = 1.0f;
            state->unknown30.value.z = 1.0f;
        }
        for (; step != 0; step = step->next) {
            if (step->apply != 0) {
                step->apply(&step->data);
            }
        }
        if (useInverse) {
            fn_8040C204(lbl_805F13A8);
            fn_8040C148(&matrix, fn_80411700(lbl_805F13A8));
            fn_8040C148(fn_80411700(lbl_805F13A8), &inverse);
            fn_8040BAC4(TopMatrix(), &matrix);
            fn_8040C148(&node->transform->state.matrix, fn_80411700(lbl_805F13A8));
            fn_8040C244(lbl_805F13A8);
        } else {
            fn_8040C148(&node->transform->state.matrix, fn_80411700(lbl_805F13A8));
        }
    }
    if (node->first != 0) {
        fn_8041A83C(control, node->first);
    }
    if (changedRatios) {
        for (int i = 0; i < control->playerList.count; i++) {
            int index = control->playerList.listedIndices[i];
            if (index == -1) {
                break;
            }
            lbl_805E14F0[index] = ratios[index];
        }
    }
    if (node->second != 0) {
        fn_8041A83C(control, node->second);
    }
}

void fn_8041AB3C(_stMtnPlayWork *control, int index) {
    int kind = 3;
    MotionPlayer *player = &control->players[index];
    if (control->playerList.count == 1) {
        return;
    }
    int otherIndex = control->playerList.unknown0C[index];
    if (otherIndex == -1) {
        return;
    }
    if (player->stateFlags & MOTION_PLAYER_BLEND_FIRST) {
        kind = player->blend->kindFirst;
    } else if (player->blend->flags & MOTION_BLEND_FLAG_1) {
        kind = player->blend->kindSecond;
    }
    if (kind == 1 && ((player->stateFlags & MOTION_PLAYER_REQUEST_SPEED) ||
                      (player->stateFlags & MOTION_PLAYER_REQUEST_CURRENT_FRAME))) {
        MotionPlayer *other = &control->players[otherIndex];
        if ((other->flags & MOTION_PLAYER_ACTIVE) ||
            (other->stateFlags & MOTION_PLAYER_REQUEST_MOTION)) {
            MotionRange otherRange;
            MotionRange range;
            float firstFrame;
            float lastFrame;

            if (player->requestedMotionId != 0) {
                fn_804125F4(&range, player->requestedMotionId);
                firstFrame = range.startFrame;
                lastFrame = range.endFrame;
            } else if (player->stateFlags & MOTION_PLAYER_REQUEST_MOTION) {
                fn_804125F4(&range, 1);
                firstFrame = range.startFrame;
                lastFrame = range.endFrame;
            } else {
                range = player->range;
                firstFrame = player->firstFrame;
                lastFrame = player->lastFrame;
            }
            fn_8041AD94(&range, &player->requestedRange, firstFrame, lastFrame);

            if (other->requestedMotionId != 0) {
                fn_804125F4(&otherRange, other->requestedMotionId);
                firstFrame = otherRange.startFrame;
                lastFrame = otherRange.endFrame;
            } else if (other->stateFlags & MOTION_PLAYER_REQUEST_MOTION) {
                fn_804125F4(&otherRange, 1);
                firstFrame = otherRange.startFrame;
                lastFrame = otherRange.endFrame;
            } else {
                otherRange = other->range;
                firstFrame = other->firstFrame;
                lastFrame = other->lastFrame;
            }
            if (other->stateFlags &
                (MOTION_PLAYER_REQUEST_SPEED | MOTION_PLAYER_REQUEST_START_FRAME |
                 MOTION_PLAYER_REQUEST_END_FRAME | MOTION_PLAYER_REQUEST_CURRENT_FRAME)) {
                fn_8041AD94(&otherRange, &other->requestedRange, firstFrame, lastFrame);
            }
            /* Unordered durations also propagate pending requests. */
            if (!(range.endFrame - range.startFrame < MOTION_LIMIT_EPSILON)) {
                if (player->stateFlags & MOTION_PLAYER_REQUEST_CURRENT_FRAME) {
                    other->requestedRange.currentFrame =
                        (otherRange.endFrame - otherRange.startFrame) *
                            ((range.currentFrame - range.startFrame) /
                             (range.endFrame - range.startFrame)) +
                        otherRange.startFrame;
                    other->stateFlags |= MOTION_PLAYER_REQUESTED;
                    other->stateFlags |= MOTION_PLAYER_REQUEST_CURRENT_FRAME;
                }
                if (player->stateFlags & MOTION_PLAYER_REQUEST_SPEED) {
                    other->requestedRange.speed =
                        range.speed * ((otherRange.endFrame - otherRange.startFrame) /
                                       (range.endFrame - range.startFrame));
                    other->stateFlags |= MOTION_PLAYER_REQUESTED;
                    other->stateFlags |= MOTION_PLAYER_REQUEST_SPEED;
                }
            }
        }
    }
}

/* Applies a requested range to a range (behaviour inferred): always the speed; the start and end
 * frames when they are not negative, after limiting them in the request to firstFrame and
 * lastFrame (an end before the start becomes the start); the current frame when it is not
 * negative, else the range's own, either way kept within the new start and end. Returns 0x10 when
 * the current frame comes from the request or was moved to a limit. 0x1 is added when the old and
 * new speeds multiply to less than MOTION_LIMIT_EPSILON, when the current frame was moved to the
 * start with a speed below MOTION_LIMIT_EPSILON or to the end with a speed above
 * -MOTION_LIMIT_EPSILON, and when a requested current frame lies after the old one with a speed
 * below MOTION_LIMIT_EPSILON or before it with a speed above -MOTION_LIMIT_EPSILON. Its caller
 * fn_804192EC sets bit 0x40000 of MotionPlayer.flags for 0x1 and MOTION_PLAYER_ZERO_SPEED for
 * 0x10. */
int fn_8041AD94(MotionRange *range, MotionRange *request, float firstFrame, float lastFrame) {
    int result = 0;
    float speed;
    float startFrame;
    float endFrame;
    float currentFrame;

    if (range->speed * request->speed < MOTION_LIMIT_EPSILON) {
        result |= 1;
    }
    speed = request->speed;
    range->speed = speed;

    if (request->startFrame < 0.0f) {
        startFrame = range->startFrame;
    } else {
        if (firstFrame > request->startFrame) {
            request->startFrame = firstFrame;
        }
        startFrame = request->startFrame;
    }
    if (request->endFrame < 0.0f) {
        endFrame = range->endFrame;
    } else {
        if (lastFrame < request->endFrame) {
            request->endFrame = lastFrame;
        }
        endFrame = request->endFrame;
    }
    if (endFrame < startFrame) {
        endFrame = startFrame;
    }

    if (request->currentFrame < 0.0f) {
        currentFrame = range->currentFrame;
        if (currentFrame < startFrame) {
            currentFrame = startFrame;
            result |= 0x10;
            if (speed < MOTION_LIMIT_EPSILON) {
                result |= 1;
            }
        }
        if (currentFrame > endFrame) {
            currentFrame = endFrame;
            result |= 0x10;
            if (speed > -MOTION_LIMIT_EPSILON) {
                result |= 1;
            }
        }
    } else {
        currentFrame = request->currentFrame;
        if (currentFrame < startFrame) {
            currentFrame = startFrame;
        }
        if (currentFrame > endFrame) {
            currentFrame = endFrame;
        }
        if ((speed < MOTION_LIMIT_EPSILON && currentFrame > range->currentFrame) ||
            (speed > -MOTION_LIMIT_EPSILON && currentFrame < range->currentFrame)) {
            result |= 1;
        }
        result |= 0x10;
    }

    range->startFrame = startFrame;
    range->endFrame = endFrame;
    range->currentFrame = currentFrame;
    return result;
}

/* Hands the range of a player to the curve state and runs fn_8041AFCC for every node of the
 * tree. */
void fn_8041AED0(_stMtnPlayWork *control, int playerIndex) {
    fn_8041575C(&control->players[playerIndex].range);
    fn_8041AF1C(control->rootNode, playerIndex);
}

/* Runs fn_8041AFCC for every node of the tree below (and including) a node. */
void fn_8041AF1C(MotionNode *node, int playerIndex) {
    fn_8041AFCC(node, playerIndex);
    if (node->first != 0) {
        fn_8041AF1C(node->first, playerIndex);
    }
    if (node->second != 0) {
        fn_8041AF1C(node->second, playerIndex);
    }
}

/* Runs fn_80413B30 on each of three curves whose position is set (name inferred). */
static inline void UpdateCurves(MotionKeyCursor *curves) {
    if (curves[0].position != 0) {
        fn_80413B30(&curves[0]);
    }
    if (curves[1].position != 0) {
        fn_80413B30(&curves[1]);
    }
    if (curves[2].position != 0) {
        fn_80413B30(&curves[2]);
    }
}

/* Runs fn_80413B30 on the curves of the tracks of a player in the steps of a node. */
void fn_8041AFCC(MotionNode *node, int playerIndex) {
    MotionNodeStep *step = node->steps;

    if (node->flags & MOTION_NODE_TRACK_400) {
        UpdateCurves(step->data.tracks[playerIndex].data.vector->curves);
        step = step->next;
    }
    if (node->flags & MOTION_NODE_SCALED_TRACK) {
        UpdateCurves(step->data.tracks[playerIndex].data.scaled->curves);
    }
    step = step->next;
    if (!(node->flags & MOTION_NODE_TRACK_40) && (node->flags & MOTION_NODE_REFLECTED_TRACK)) {
        UpdateCurves(step->data.tracks[playerIndex].data.reflected->curves);
    }
    step = step->next;
    if (node->flags & MOTION_NODE_TRACK_380) {
        UpdateCurves(step->data.tracks[playerIndex].data.vector->curves);
    }
}

int fn_8041B170(_stMtnPlayWork *control, int playerIndex, unsigned short motionId) {
    MotionRange *range = &control->players[playerIndex].range;
    float scale = -1.0f;

    fn_8041B6FC(control->rootNode, playerIndex);
    if (!fn_804125F4(range, motionId)) {
        control->players[playerIndex].firstFrame = 0.0f;
        control->players[playerIndex].lastFrame = 1.0f;
        return 0;
    }
    control->players[playerIndex].firstFrame = range->startFrame;
    control->players[playerIndex].lastFrame = range->endFrame;
    fn_8041575C(range);
    unsigned int id = fn_804123DC(motionId);
    PackEntryInfo *info = fn_804106FC(id);
    if (info == 0) {
        return 0;
    }
    if (id != control->info->id && fn_8000A980(info->value08) > MOTION_LIMIT_EPSILON &&
        fn_8000A980(control->info->value08) > MOTION_LIMIT_EPSILON) {
        scale = control->info->value08 / info->value08;
    }
    fn_8041B2BC(control, (PackNode *)(info + 1), playerIndex, motionId, scale);
    return 1;
}

void fn_8041B2BC(_stMtnPlayWork *control, PackNode *definition, int playerIndex,
                 unsigned short motionId, float scale) {
    MotionNode *node = fn_8041D448(control->rootNode, definition->id);
    MotionNodeStep *step = 0;
    if (node != 0) {
        step = node->steps;
    }
    if (definition->values[1] & MOTION_NODE_TRACK_400) {
        if (node != 0 && (node->flags & MOTION_NODE_TRACK_400)) {
            MotionVectorTrack *track = step->data.tracks[playerIndex].data.vector;
            memcpy(&track->value, node->definition->rows[2], sizeof(Fn8040Vector3));
            fn_80413E38(&track->curves[0], fn_804124DC(motionId));
            fn_80413E38(&track->curves[1], fn_804124DC(0));
            fn_80413E38(&track->curves[2], fn_804124DC(0));
            step = step->next;
            motionId = 0;
        } else {
            fn_804124DC(motionId);
            fn_804124DC(0);
            fn_804124DC(0);
            motionId = 0;
        }
    }
    if (definition->values[1] & MOTION_NODE_SCALED_TRACK) {
        if (node != 0 && (node->flags & MOTION_NODE_SCALED_TRACK)) {
            MotionScaledVectorTrack *track = step->data.tracks[playerIndex].data.scaled;
            track->scale = scale;
            memcpy(&track->value, node->definition->rows[0], sizeof(Fn8040Vector3));
            if (definition->values[1] & 0x1) {
                fn_80413E38(&track->curves[0], fn_804124DC(motionId));
                motionId = 0;
            }
            if (definition->values[1] & 0x2) {
                fn_80413E38(&track->curves[1], fn_804124DC(motionId));
                motionId = 0;
            }
            if (definition->values[1] & 0x4) {
                fn_80413E38(&track->curves[2], fn_804124DC(motionId));
                motionId = 0;
            }
        } else {
            if (definition->values[1] & 0x1) {
                fn_804124DC(motionId);
                motionId = 0;
            }
            if (definition->values[1] & 0x2) {
                fn_804124DC(motionId);
                motionId = 0;
            }
            if (definition->values[1] & 0x4) {
                fn_804124DC(motionId);
                motionId = 0;
            }
        }
    }
    if (step != 0) {
        step = step->next;
    }
    if (!(definition->values[1] & MOTION_NODE_TRACK_40) &&
        (definition->values[1] & MOTION_NODE_REFLECTED_TRACK)) {
        if (node != 0 && (node->flags & MOTION_NODE_REFLECTED_TRACK)) {
            MotionReflectedVectorTrack *track = step->data.tracks[playerIndex].data.reflected;
            memcpy(track, node->definition->rows[1], sizeof(node->definition->rows[1]));
            if (definition->values[1] & 0x8) {
                fn_80413E38(&track->curves[0], fn_804124DC(motionId));
                motionId = 0;
            }
            if (definition->values[1] & 0x10) {
                fn_80413E38(&track->curves[1], fn_804124DC(motionId));
                motionId = 0;
            }
            if (definition->values[1] & 0x20) {
                fn_80413E38(&track->curves[2], fn_804124DC(motionId));
                motionId = 0;
            }
        } else {
            if (definition->values[1] & 0x8) {
                fn_804124DC(motionId);
                motionId = 0;
            }
            if (definition->values[1] & 0x10) {
                fn_804124DC(motionId);
                motionId = 0;
            }
            if (definition->values[1] & 0x20) {
                fn_804124DC(motionId);
                motionId = 0;
            }
        }
    }
    if (step != 0) {
        step = step->next;
    }
    if (definition->values[1] & MOTION_NODE_TRACK_380) {
        if (node != 0 && (node->flags & MOTION_NODE_TRACK_380)) {
            MotionVectorTrack *track = step->data.tracks[playerIndex].data.vector;
            memcpy(&track->value, node->definition->rows[2], sizeof(Fn8040Vector3));
            if (definition->values[1] & 0x80) {
                fn_80413E38(&track->curves[0], fn_804124DC(motionId));
                motionId = 0;
            }
            if (definition->values[1] & 0x100) {
                fn_80413E38(&track->curves[1], fn_804124DC(motionId));
                motionId = 0;
            }
            if (definition->values[1] & 0x200) {
                fn_80413E38(&track->curves[2], fn_804124DC(motionId));
                motionId = 0;
            }
        } else {
            if (definition->values[1] & 0x80) {
                fn_804124DC(motionId);
                motionId = 0;
            }
            if (definition->values[1] & 0x100) {
                fn_804124DC(motionId);
                motionId = 0;
            }
            if (definition->values[1] & 0x200) {
                fn_804124DC(motionId);
                motionId = 0;
            }
        }
    }
    if (definition->first.node != 0) {
        fn_8041B2BC(control, definition->first.node, playerIndex, motionId, scale);
    }
    if (definition->second.node != 0) {
        fn_8041B2BC(control, definition->second.node, playerIndex, motionId, scale);
    }
}

/* Runs fn_80413E10 on each of three curves (name inferred). */
static inline void ResetCurves(MotionKeyCursor *curves) {
    fn_80413E10(&curves[0]);
    fn_80413E10(&curves[1]);
    fn_80413E10(&curves[2]);
}

/* For the tracks of a player in the steps of every node of the tree below (and including) a node:
 * sets the vector of the track of MOTION_NODE_TRACK_400 to 1 and the vectors of the other tracks to
 * rows 0, 1 and 2 of the node's definition (all four values of row 1, the fourth into unknown0C),
 * and runs fn_80413E10 on all their curves. */
void fn_8041B6FC(MotionNode *node, int playerIndex) {
    MotionNodeStep *step = node->steps;

    if (node->flags & MOTION_NODE_TRACK_400) {
        MotionVectorTrack *track = step->data.tracks[playerIndex].data.vector;

        track->value.x = track->value.y = track->value.z = 1.0f;
        ResetCurves(track->curves);
        step = step->next;
    }
    if (node->flags & MOTION_NODE_SCALED_TRACK) {
        MotionScaledVectorTrack *track = step->data.tracks[playerIndex].data.scaled;

        memcpy(&track->value, node->definition->rows[0], sizeof(Fn8040Vector3));
        ResetCurves(track->curves);
    }
    step = step->next;
    if (!(node->flags & MOTION_NODE_TRACK_40) && (node->flags & MOTION_NODE_REFLECTED_TRACK)) {
        MotionReflectedVectorTrack *track = step->data.tracks[playerIndex].data.reflected;

        memcpy(track, node->definition->rows[1], sizeof(node->definition->rows[1]));
        ResetCurves(track->curves);
    }
    step = step->next;
    if (node->flags & MOTION_NODE_TRACK_380) {
        MotionVectorTrack *track = step->data.tracks[playerIndex].data.vector;

        memcpy(&track->value, node->definition->rows[2], sizeof(Fn8040Vector3));
        ResetCurves(track->curves);
    }
    if (node->first != 0) {
        fn_8041B6FC(node->first, playerIndex);
    }
    if (node->second != 0) {
        fn_8041B6FC(node->second, playerIndex);
    }
}

/* Gives the motions of the active players back and frees a control. */
void fn_8041B884(_stMtnPlayWork *control) {
    int i;

    if (control != 0) {
        for (i = 0; i < control->playerList.count; i++) {
            int index = control->playerList.listedIndices[i];
            MotionPlayer *player;

            if (index == -1) {
                break;
            }
            player = &control->players[index];
            if (player->flags & MOTION_PLAYER_ACTIVE) {
                fn_80413240(player->motionId);
            }
        }
        fn_804110FC(control);
    }
}

/* Looks up the pack entry of an id and passes it to fn_8041B980, which reports "Cannot get
 * PlayWork.\n" (behaviour inferred); reports an error and returns null when the entry is missing
 * or is not of bone type 0xFF. */
_stMtnPlayWork *fn_8041B908(unsigned int id, int arg1) {
    PackEntryInfo *info = fn_804106FC(id);
    int boneType;

    if (info == 0) {
        printf("Cannot find bone data.\n");
        return 0;
    }
    /* Matching workaround: the field is copied into an int before the comparison. Comparing the
     * unsigned short with 0xFF directly compiles to an unsigned compare (cmplwi) where the target
     * compares signed (cmpwi). Revisit if the field turns out to be wider or signed. */
    boneType = info->value04;
    if (boneType != 0xFF) {
        printf("Illegal bonetype.\n");
        return 0;
    }
    return fn_8041B980(info, arg1);
}

_stMtnPlayWork *fn_8041B980(PackEntryInfo *info, int playerCount) {
    PackNode *root = (PackNode *)(info + 1);
    int trackBytes = 0;
    int nodeBytes = 0;
    int transformCount = 0;
    if (playerCount < 1) {
        playerCount = 1;
    } else if (playerCount > 4) {
        playerCount = 4;
    }
    fn_8041CA78(&trackBytes, &nodeBytes, &transformCount, playerCount, root);
    unsigned long bytes = sizeof(_stMtnPlayWork) + transformCount * sizeof(MotionTransform) +
                          playerCount * sizeof(MotionPlayer) + playerCount * trackBytes +
                          playerCount * 2 * sizeof(int) + playerCount * sizeof(int) +
                          playerCount * 2 * sizeof(int) + playerCount * sizeof(MotionBlend) +
                          playerCount * info->nodeCount * sizeof(MotionNodeEntry) + nodeBytes;
    _stMtnPlayWork *control = (_stMtnPlayWork *)fn_80411134(1, bytes);
    if (control == 0) {
        printf("Cannot get PlayWork.\n");
        return 0;
    }
    control->info = info;
    control->transformCount = transformCount;
    control->transforms = (MotionTransform *)(control + 1);
    control->players = (MotionPlayer *)(control->transforms + transformCount);
    unsigned char *next = (unsigned char *)(control->players + playerCount);
    for (int i = 0; i < playerCount; i++) {
        if (trackBytes == 0) {
            control->players[i].tracks = 0;
        } else {
            control->players[i].tracks = (MotionTrackNode *)(next);
            next += trackBytes;
        }
    }
    control->playerList.count = playerCount;
    control->playerList.listedIndices = (int *)(next);
    control->playerList.playerIndexOfId = control->playerList.listedIndices + playerCount * 2;
    control->playerList.unknown0C = control->playerList.playerIndexOfId + playerCount;
    MotionBlend *blends = (MotionBlend *)(control->playerList.unknown0C + playerCount * 2);
    for (int i = 0; i < playerCount * 2; i++) {
        control->playerList.listedIndices[i] = -1;
        control->playerList.unknown0C[i] = -1;
    }
    for (int i = 0; i < playerCount; i++) {
        control->playerList.playerIndexOfId[i] = -1;
    }
    control->playerList.listedIndices[0] = 0;
    control->playerList.playerIndexOfId[0] = 0;
    for (int i = 0; i < playerCount; i++) {
        control->players[i].blend = &blends[i];
        control->players[i].range.speed = 1.0f;
        control->players[i].requestedRange.speed = 1.0f;
        control->players[i].requestedRange.currentFrame = -1.0f;
        control->players[i].requestedRange.startFrame = -1.0f;
        control->players[i].requestedRange.endFrame = -1.0f;
    }
    MotionNodeEntry *entries = (MotionNodeEntry *)(blends + playerCount);
    control->rootNode = (MotionNode *)(entries + playerCount * info->nodeCount);
    for (int i = 0; i < playerCount; i++) {
        MotionTrackNode *first = control->players[i].tracks;
        MotionTrackNode *track = first;
        MotionTrackNode *last = 0;
        if (first != 0) {
            fn_8041C854(&track, &last, root);
            track = first;
            fn_8041C2E0(root, root, &track, first);
        }
    }
    MotionTransform *nextTransform = control->transforms;
    MotionNode *nextNode = control->rootNode;
    MotionTrackNode *tracks[4];
    for (int i = 0; i < control->playerList.count; i++) {
        tracks[i] = control->players[i].tracks;
    }
    fn_8041BE30(control, &nextNode, blends, &entries, &nextTransform, tracks, root);
    fn_8040C204(lbl_805F13A8);
    fn_8040B6F8(TopMatrix());
    MotionTransform *transform = control->transforms;
    for (int i = 0; i < control->transformCount; i++) {
        fn_8040C148(&transform->matrix, fn_80411700(lbl_805F13A8));
        fn_8040C148(&transform->state.matrix, fn_80411700(lbl_805F13A8));
        transform++;
    }
    fn_8040C148(&control->matrix, fn_80411700(lbl_805F13A8));
    fn_8040C148(&control->unknownC0, fn_80411700(lbl_805F13A8));
    fn_8040C148(&control->unknown30, fn_80411700(lbl_805F13A8));
    fn_8040C148(&control->unknown60, fn_80411700(lbl_805F13A8));
    for (int i = 0; i < playerCount; i++) {
        fn_8040C148(&control->players[i].unknown30, fn_80411700(lbl_805F13A8));
        fn_8040C148(&control->players[i].unknown00, fn_80411700(lbl_805F13A8));
        fn_8040C148(&control->players[i].unknown60, fn_80411700(lbl_805F13A8));
    }
    fn_8040C244(lbl_805F13A8);
    for (int i = 0; i < playerCount; i++) {
        fn_8041CC18(control->rootNode, i);
    }
    return control;
}

void fn_8041BE30(_stMtnPlayWork *control, MotionNode **nextNode, MotionBlend *blends,
                 MotionNodeEntry **nextEntries, MotionTransform **nextTransform,
                 MotionTrackNode **tracks, PackNode *definition) {
    MotionNode *node = *nextNode;
    node->definition = definition;
    node->flags = 0;
    node->blends = blends;
    node->entries = *nextEntries;
    node->steps = (MotionNodeStep *)(node + 1);
    int playerCount = control->playerList.count;
    *nextEntries += playerCount;
    MotionTransformState *state;
    switch (definition->values[0]) {
    case 3:
    case 4:
        node->transform = *nextTransform;
        (*nextTransform)++;
        state = &node->transform->state;
        break;
    default:
        if (definition->values[1] != 0 || (definition->values[2] & 4)) {
            node->transform = *nextTransform;
            (*nextTransform)++;
            state = &node->transform->state;
        } else {
            node->transform = 0;
            state = 0;
        }
        break;
    }

    MotionNodeStep *step = node->steps;
    if (definition->values[1] & MOTION_NODE_TRACK_400) {
        node->flags |= MOTION_NODE_TRACK_400;
        step->data.playerList = &control->playerList;
        step->data.state = state;
        MotionStepTrack *output = step->data.tracks;
        for (int i = 0; i < playerCount; i++) {
            MotionVectorTrackStorage *storage = (MotionVectorTrackStorage *)(tracks[i]);
            tracks[i] = storage->node.next;
            output->data.vector = &storage->data;
            storage->data.value.x = 1.0f;
            storage->data.value.y = 1.0f;
            storage->data.value.z = 1.0f;
            output->state = &blends[i];
            output++;
        }
        step->apply = fn_8041FAD8;
        step->next = (MotionNodeStep *)(output);
        step = step->next;
    }

    MotionNodeStep *nextStep;
    if (definition->values[1] & MOTION_NODE_SCALED_TRACK) {
        node->flags |= MOTION_NODE_SCALED_TRACK;
        step->data.playerList = &control->playerList;
        step->data.state = state;
        MotionStepTrack *output = step->data.tracks;
        for (int i = 0; i < playerCount; i++) {
            MotionScaledTrackStorage *storage = (MotionScaledTrackStorage *)(tracks[i]);
            tracks[i] = storage->node.next;
            output->data.scaled = &storage->data;
            memcpy(&storage->data.value, definition->rows[0], sizeof(Fn8040Vector3));
            output->state = &blends[i];
            output++;
        }
        nextStep = (MotionNodeStep *)(output);
        if (definition->values[0] == 4) {
            MotionStepExtra *extra = (MotionStepExtra *)(output);
            if (definition->first.node->values[0] == 4) {
                extra->transform = *nextTransform;
                extra->first = definition->first.node->rows[0];
                extra->second = definition->first.node->first.node->rows[0];
            } else {
                extra->transform = 0;
                extra->first = definition->first.node->rows[0];
                extra->second = 0;
            }
            nextStep = (MotionNodeStep *)(extra + 1);
        }
        if (definition->values[0] != 4) {
            step->apply = fn_8041FCC0;
        }
    } else {
        MotionStaticStep *staticStep = (MotionStaticStep *)(step);
        staticStep->data.value = (const Fn8040Vector3 *)definition->rows[0];
        nextStep = (MotionNodeStep *)(staticStep + 1);
        if (definition->values[0] != 4) {
            staticStep->apply = fn_8041FED0;
        }
    }
    step->next = nextStep;
    step = step->next;

    if ((definition->values[1] & MOTION_NODE_TRACK_40) ||
        (definition->values[1] & MOTION_NODE_REFLECTED_TRACK)) {
        if (definition->values[1] & MOTION_NODE_TRACK_40) {
            node->flags |= MOTION_NODE_TRACK_40;
        }
        if (definition->values[1] & MOTION_NODE_REFLECTED_TRACK) {
            node->flags |= MOTION_NODE_REFLECTED_TRACK;
        }
        step->data.playerList = &control->playerList;
        step->data.state = state;
        MotionStepTrack *output = step->data.tracks;
        for (int i = 0; i < playerCount; i++) {
            MotionReflectedTrackStorage *storage = (MotionReflectedTrackStorage *)(tracks[i]);
            tracks[i] = storage->node.next;
            output->data.reflected = &storage->data;
            memcpy(&storage->data, definition->rows[1], sizeof(definition->rows[1]));
            output->state = &blends[i];
            output++;
        }
        step->apply = fn_8041FB4C;
        nextStep = (MotionNodeStep *)(output);
    } else {
        MotionStaticStep *staticStep = (MotionStaticStep *)(step);
        staticStep->data.value = (const Fn8040Vector3 *)definition->rows[1];
        nextStep = (MotionNodeStep *)(staticStep + 1);
        switch (definition->values[4]) {
        case 6:
            staticStep->apply = fn_8041FD6C;
            break;
        default:
            staticStep->apply = fn_8041FD70;
            break;
        }
    }
    step->next = nextStep;
    step = step->next;

    if (definition->values[1] & MOTION_NODE_TRACK_380) {
        node->flags |= MOTION_NODE_TRACK_380;
        step->data.playerList = &control->playerList;
        step->data.state = state;
        MotionStepTrack *output = step->data.tracks;
        for (int i = 0; i < playerCount; i++) {
            MotionVectorTrackStorage *storage = (MotionVectorTrackStorage *)(tracks[i]);
            tracks[i] = storage->node.next;
            output->data.vector = &storage->data;
            memcpy(&storage->data.value, definition->rows[2], sizeof(Fn8040Vector3));
            output->state = &blends[i];
            output++;
        }
        step->apply = fn_8041FAD8;
        nextStep = (MotionNodeStep *)(output);
    } else {
        MotionStaticStep *staticStep = (MotionStaticStep *)(step);
        staticStep->data.value = (const Fn8040Vector3 *)definition->rows[2];
        nextStep = (MotionNodeStep *)(staticStep + 1);
        staticStep->apply = fn_8041FD0C;
    }
    step->next = 0;
    *nextNode = (MotionNode *)(nextStep);
    if (definition->first.node != 0) {
        node->first = *nextNode;
        fn_8041BE30(control, nextNode, blends, nextEntries, nextTransform, tracks,
                    definition->first.node);
    }
    if (definition->second.node != 0) {
        node->second = *nextNode;
        fn_8041BE30(control, nextNode, blends, nextEntries, nextTransform, tracks,
                    definition->second.node);
    }
}

void fn_8041C2E0(PackNode *node, PackNode *root, MotionTrackNode **track,
                 MotionTrackNode *firstTrack) {
    if (node->values[3] & 0x8000) {
        char name[32];
        strcpy(name, node->name);
        /* The original requires this marker in flagged node names. */
        strstr(name, "_l")[1] = 'r';
        unsigned int id = fn_80410684(name);
        MotionTrackNode *mirrorTrack = firstTrack;
        PackNode *mirror = fn_8041C5A8(root, &mirrorTrack, id);
        if (mirror != 0) {
            if (node->values[1] & MOTION_NODE_TRACK_400) {
                *track = (*track)->next;
            }
            if (mirror->values[1] & MOTION_NODE_TRACK_400) {
                mirrorTrack = mirrorTrack->next;
            }
            if (node->values[1] & MOTION_NODE_SCALED_TRACK) {
                MotionScaledTrackStorage *storage = (MotionScaledTrackStorage *)(*track);
                storage->data.mirrorOptions = mirror;
                storage->data.mirrorTrackNode = mirrorTrack;
                (*track)->second = fn_804201C0;
                mirrorTrack->second = 0;
                if (mirror->values[1] & MOTION_NODE_SCALED_TRACK) {
                    mirrorTrack = mirrorTrack->next;
                }
                *track = (*track)->next;
            } else if (mirror->values[1] & MOTION_NODE_SCALED_TRACK) {
                mirrorTrack = mirrorTrack->next;
            }
            if ((node->values[1] & MOTION_NODE_TRACK_40) ||
                (mirror->values[1] & MOTION_NODE_TRACK_40)) {
                if (node->values[1] & MOTION_NODE_TRACK_40) {
                    *track = (*track)->next;
                }
                if (mirror->values[1] & MOTION_NODE_TRACK_40) {
                    mirrorTrack = mirrorTrack->next;
                }
            } else if (node->values[1] & MOTION_NODE_REFLECTED_TRACK) {
                MotionReflectedTrackStorage *storage = (MotionReflectedTrackStorage *)(*track);
                storage->data.mirrorOptions = mirror;
                storage->data.mirrorTrackNode = mirrorTrack;
                (*track)->second = fn_8041FF0C;
                mirrorTrack->second = 0;
                if (mirror->values[1] & MOTION_NODE_REFLECTED_TRACK) {
                    mirrorTrack = mirrorTrack->next;
                }
                *track = (*track)->next;
            } else if (mirror->values[1] & MOTION_NODE_REFLECTED_TRACK) {
                mirrorTrack = mirrorTrack->next;
            }
            if (node->values[1] & MOTION_NODE_TRACK_380) {
                *track = (*track)->next;
            }
            if (mirror->values[1] & MOTION_NODE_TRACK_380) {
                mirrorTrack = mirrorTrack->next;
            }
        }
        /* Failed lookup leaves *track unchanged, including before recursion. */
    } else {
        if (node->values[1] & MOTION_NODE_TRACK_400) {
            *track = (*track)->next;
        }
        if (node->values[1] & MOTION_NODE_SCALED_TRACK) {
            *track = (*track)->next;
        }
        if (node->values[1] & MOTION_NODE_TRACK_40) {
            *track = (*track)->next;
        } else if (node->values[1] & MOTION_NODE_REFLECTED_TRACK) {
            *track = (*track)->next;
        }
        if (node->values[1] & MOTION_NODE_TRACK_380) {
            *track = (*track)->next;
        }
    }
    if (node->first.node != 0) {
        fn_8041C2E0(node->first.node, root, track, firstTrack);
    }
    if (node->second.node != 0) {
        fn_8041C2E0(node->second.node, root, track, firstTrack);
    }
}

PackNode *fn_8041C5A8(PackNode *node, MotionTrackNode **track, unsigned int id) {
    if (id == node->id) {
        return node;
    }
    if (node->values[1] & MOTION_NODE_TRACK_400) {
        *track = (*track)->next;
    }
    if (node->values[1] & MOTION_NODE_SCALED_TRACK) {
        *track = (*track)->next;
    }
    if (node->values[1] & MOTION_NODE_TRACK_40) {
        *track = (*track)->next;
    } else if (node->values[1] & MOTION_NODE_REFLECTED_TRACK) {
        *track = (*track)->next;
    }
    if (node->values[1] & MOTION_NODE_TRACK_380) {
        *track = (*track)->next;
    }
    if (node->first.node != 0) {
        PackNode *result = fn_8041C5A8(node->first.node, track, id);
        if (result != 0) {
            return result;
        }
    }
    if (node->second.node != 0) {
        PackNode *result = fn_8041C5A8(node->second.node, track, id);
        if (result != 0) {
            return result;
        }
    }
    return 0;
}

void fn_8041C854(MotionTrackNode **next, MotionTrackNode **last, PackNode *node) {
    if (node->values[1] & MOTION_NODE_TRACK_400) {
        (*next)->first = fn_80420308;
        (*next)->next = 0;
        if (*last != 0) {
            (*last)->next = *next;
        }
        *last = *next;
        *next = (MotionTrackNode *)((MotionVectorTrackStorage *)*next + 1);
    }
    if (node->values[1] & MOTION_NODE_SCALED_TRACK) {
        (*next)->first = fn_804204C8;
        (*next)->second = fn_804202B8;
        (*next)->next = 0;
        ((MotionScaledTrackStorage *)*next)->data.options = node;
        if (*last != 0) {
            (*last)->next = *next;
        }
        *last = *next;
        *next = (MotionTrackNode *)((MotionScaledTrackStorage *)*next + 1);
    }
    if (node->values[1] & MOTION_NODE_TRACK_40) {
        (*next)->first = fn_80420438;
        (*next)->second = fn_8041FF08;
        (*next)->next = 0;
        ((MotionReflectedTrackStorage *)*next)->data.options = node;
        if (*last != 0) {
            (*last)->next = *next;
        }
        *last = *next;
        *next = (MotionTrackNode *)((MotionReflectedTrackStorage *)*next + 1);
    } else if (node->values[1] & MOTION_NODE_REFLECTED_TRACK) {
        (*next)->first = fn_8042043C;
        (*next)->second = fn_80420158;
        (*next)->next = 0;
        ((MotionReflectedTrackStorage *)*next)->data.options = node;
        if (*last != 0) {
            (*last)->next = *next;
        }
        *last = *next;
        *next = (MotionTrackNode *)((MotionReflectedTrackStorage *)*next + 1);
    }
    if (node->values[1] & MOTION_NODE_TRACK_380) {
        (*next)->first = fn_804203AC;
        (*next)->next = 0;
        if (*last != 0) {
            (*last)->next = *next;
        }
        *last = *next;
        *next = (MotionTrackNode *)((MotionVectorTrackStorage *)*next + 1);
    }
    if (node->first.node != 0) {
        fn_8041C854(next, last, node->first.node);
    }
    if (node->second.node != 0) {
        fn_8041C854(next, last, node->second.node);
    }
}

/* Adds up the sizes of the tree that fn_8041B980 builds (with fn_8041BE30) from the pack nodes
 * below (and including) a node (behaviour inferred): the bytes of the track nodes with their data,
 * the bytes of the nodes with their steps, and the number of nodes that get a transform. In
 * fn_8041BE30 a node takes 0x34 bytes and each step an 8-byte header (callback and link) and its
 * data: two words and an 8-byte track for each player for a step with tracks (three more words
 * for the scaled step of a node whose values[0] is 4), one word for a step without. The first 0x3C
 * bytes are the node and the header of the step of MOTION_NODE_TRACK_400, counted even without
 * that step. Nodes whose values[0] is 3 or 4 always get a transform. */
void fn_8041CA78(int *trackBytes, int *nodeBytes, int *transformCount, int playerCount,
                 PackNode *node) {
    int bytes;
    int tracks = 0;

    bytes = 0x3C;
    if (node->values[1] & MOTION_NODE_TRACK_400) {
        bytes += (playerCount * 2 + 2) * 4;
        tracks = sizeof(MotionTrackNode) + sizeof(MotionVectorTrack);
    }
    bytes += 8;
    if (node->values[1] & MOTION_NODE_SCALED_TRACK) {
        tracks += sizeof(MotionTrackNode) + sizeof(MotionScaledVectorTrack);
        if (node->values[0] == 4) {
            bytes += (playerCount * 2 + 5) * 4;
        } else {
            bytes += (playerCount * 2 + 2) * 4;
        }
    } else {
        bytes += 4;
    }
    bytes += 8;
    if ((node->values[1] & MOTION_NODE_TRACK_40) ||
        (node->values[1] & MOTION_NODE_REFLECTED_TRACK)) {
        tracks += sizeof(MotionTrackNode) + sizeof(MotionReflectedVectorTrack);
        bytes += (playerCount * 2 + 2) * 4;
    } else {
        bytes += 4;
    }
    bytes += 8;
    if (node->values[1] & MOTION_NODE_TRACK_380) {
        tracks += sizeof(MotionTrackNode) + sizeof(MotionVectorTrack);
        bytes += (playerCount * 2 + 2) * 4;
    } else {
        bytes += 4;
    }

    switch (node->values[0]) {
    case 3:
    case 4:
        (*transformCount)++;
        break;
    default:
        if (node->values[1] != 0 || (node->values[2] & 0x4)) {
            (*transformCount)++;
        }
        break;
    }
    *trackBytes += tracks;
    *nodeBytes += bytes;

    if (node->first.node != 0) {
        fn_8041CA78(trackBytes, nodeBytes, transformCount, playerCount, node->first.node);
    }
    if (node->second.node != 0) {
        fn_8041CA78(trackBytes, nodeBytes, transformCount, playerCount, node->second.node);
    }
}

/* Resets the entries of a player in every node of the tree below (and including) a node. */
void fn_8041CC18(const MotionNode *node, int playerIndex) {
    node->entries[playerIndex].unknown00 = 0;
    node->entries[playerIndex].unknown04 = 0;
    node->entries[playerIndex].unknown08 = 1.0f;
    node->entries[playerIndex].unknown0C = 0.0f;
    node->entries[playerIndex].unknown10 = 1.0f;
    node->entries[playerIndex].unknown14 = 0.0f;
    if (node->first != 0) {
        fn_8041CC18(node->first, playerIndex);
    }
    if (node->second != 0) {
        fn_8041CC18(node->second, playerIndex);
    }
}

/* Resets only the second pair and the flag of the entries (see fn_8041CC18). */
void fn_8041CD6C(const MotionNode *node, int playerIndex) {
    node->entries[playerIndex].unknown04 = 0;
    node->entries[playerIndex].unknown10 = 1.0f;
    node->entries[playerIndex].unknown14 = 0.0f;
    if (node->first != 0) {
        fn_8041CD6C(node->first, playerIndex);
    }
    if (node->second != 0) {
        fn_8041CD6C(node->second, playerIndex);
    }
}

/* Where the flag of a player's entry (unknown04, see fn_8041CD6C) is set, sets unknown00 and
 * copies the second pair to the first, in every node of the tree below (and including) a node. */
void fn_8041CE74(const MotionNode *node, int playerIndex) {
    if (node->entries[playerIndex].unknown04 != 0) {
        node->entries[playerIndex].unknown00 = 1;
        node->entries[playerIndex].unknown08 = node->entries[playerIndex].unknown10;
        node->entries[playerIndex].unknown0C = node->entries[playerIndex].unknown14;
    }
    if (node->first != 0) {
        fn_8041CE74(node->first, playerIndex);
    }
    if (node->second != 0) {
        fn_8041CE74(node->second, playerIndex);
    }
}

int fn_8041CFB0(MotionNode *node, int playerIndex, unsigned int nodeId, float factor,
                float offset) {
    if (nodeId == node->definition->id) {
        node->entries[playerIndex].unknown10 = factor;
        node->entries[playerIndex].unknown14 = offset;
        node->entries[playerIndex].unknown04 = 1;
        return 1;
    }
    if (node->first != 0) {
        if (fn_8041CFB0(node->first, playerIndex, nodeId, factor, offset)) {
            return 1;
        }
    }
    if (node->second != 0) {
        if (fn_8041CFB0(node->second, playerIndex, nodeId, factor, offset)) {
            return 1;
        }
    }
    return 0;
}

int fn_8041D1A0(MotionNode *node, unsigned int nodeId, MotionNodeCallback callback,
                void *argument) {
    if (nodeId == node->definition->id) {
        node->callback = callback;
        node->callbackArgument = argument;
        return 1;
    }
    if (node->first != 0) {
        if (fn_8041D1A0(node->first, nodeId, callback, argument)) {
            return 1;
        }
    }
    if (node->second != 0) {
        if (fn_8041D1A0(node->second, nodeId, callback, argument)) {
            return 1;
        }
    }
    return 0;
}

MotionTransform *fn_8041D31C(const MotionNode *node, unsigned int nodeId) {
    MotionTransform *transform;

    if (nodeId == node->definition->id) {
        return node->transform;
    }
    if (node->first != 0) {
        transform = fn_8041D31C(node->first, nodeId);
        if (transform != 0) {
            return transform;
        }
    }
    if (node->second != 0) {
        transform = fn_8041D31C(node->second, nodeId);
        if (transform != 0) {
            return transform;
        }
    }
    return 0;
}

MotionNode *fn_8041D448(MotionNode *node, unsigned int nodeId) {
    MotionNode *found;

    if (nodeId == node->definition->id) {
        return node;
    }
    if (node->first != 0) {
        found = fn_8041D448(node->first, nodeId);
        if (found != 0) {
            return found;
        }
    }
    if (node->second != 0) {
        found = fn_8041D448(node->second, nodeId);
        if (found != 0) {
            return found;
        }
    }
    return 0;
}

void fn_8041D580(void *data) {
    MotionDynamicStepData *step = (MotionDynamicStepData *)data;
    MotionPlayerList *players = step->playerList;
    Fn8040Vector3 result(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < players->count; i++) {
        int index = players->listedIndices[i];
        if (index == -1) {
            break;
        }
        lbl_805E14D0[index] = 0;
        lbl_805E14E0[index] = 0;
    }
    int pending;
    do {
        pending = 0;
        for (int i = 0; i < players->count; i++) {
            int index = players->listedIndices[i];
            if (index == -1) {
                break;
            }
            if (pending && players->unknown0C[index] == -1) {
                lbl_805E14C0[index] = 1;
            } else {
                lbl_805E14C0[index] = 0;
                for (int j = i + 1; j < players->count; j++) {
                    int otherIndex = players->listedIndices[j];
                    if (otherIndex == -1) {
                        break;
                    }
                    if (lbl_805E14D0[otherIndex] == 0 && players->unknown0C[otherIndex] == index) {
                        pending = 1;
                        lbl_805E14C0[index] = 1;
                        break;
                    }
                }
            }
        }
        for (int i = 0; i < players->count; i++) {
            int index = players->listedIndices[i];
            if (index == -1) {
                break;
            }
            if (lbl_805E14D0[index] == 0) {
                const Fn8040Vector3 *source = &step->tracks[index].data.vector->value;
                if (lbl_805E14C0[index] != 0) {
                    if (lbl_805E14E0[index] == 0) {
                        lbl_805E1490[index] = *source;
                        lbl_805E14E0[index] = 1;
                    }
                } else {
                    lbl_805E14D0[index] = 1;
                    int otherIndex = players->unknown0C[index];
                    Fn8040Vector3 *destination;
                    if (otherIndex == -1) {
                        destination = &result;
                    } else {
                        destination = &lbl_805E1490[otherIndex];
                        lbl_805E14E0[otherIndex] = 1;
                    }
                    if (lbl_805E14E0[index] != 0) {
                        source = &lbl_805E1490[index];
                    }
                    if (step->tracks[index].state->flags & MOTION_BLEND_FLAG_1) {
                        const Fn8040Vector3 *previous = destination;
                        if (i == 0) {
                            previous = &step->state->unknown30.value;
                        }
                        float ratio = lbl_805E14F0[index];
                        float inverse = 1.0f - ratio;
                        destination->x = ratio * source->x + inverse * previous->x;
                        destination->y = ratio * source->y + inverse * previous->y;
                        destination->z = ratio * source->z + inverse * previous->z;
                    } else {
                        *destination = *source;
                    }
                }
            }
        }
    } while (pending);
    if (result.x != 1.0f || result.y != 1.0f || result.z != 1.0f) {
        fn_8040B790(TopMatrix(), result);
    }
}

void fn_8041D89C(void *data) {
    MotionDynamicStepData *step = (MotionDynamicStepData *)data;
    MotionPlayerList *players = step->playerList;
    Fn8040Vector3 result(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < players->count; i++) {
        int index = players->listedIndices[i];
        if (index == -1) {
            break;
        }
        lbl_805E14D0[index] = 0;
        lbl_805E14E0[index] = 0;
    }
    int pending;
    do {
        pending = 0;
        for (int i = 0; i < players->count; i++) {
            int index = players->listedIndices[i];
            if (index == -1) {
                break;
            }
            if (pending && players->unknown0C[index] == -1) {
                lbl_805E14C0[index] = 1;
            } else {
                lbl_805E14C0[index] = 0;
                for (int j = i + 1; j < players->count; j++) {
                    int otherIndex = players->listedIndices[j];
                    if (otherIndex == -1) {
                        break;
                    }
                    if (lbl_805E14D0[otherIndex] == 0 && players->unknown0C[otherIndex] == index) {
                        pending = 1;
                        lbl_805E14C0[index] = 1;
                        break;
                    }
                }
            }
        }
        for (int i = 0; i < players->count; i++) {
            int index = players->listedIndices[i];
            if (index == -1) {
                break;
            }
            if (lbl_805E14D0[index] == 0) {
                const Fn8040Vector3 *source = &step->tracks[index].data.vector->value;
                if (lbl_805E14C0[index] != 0) {
                    if (lbl_805E14E0[index] == 0) {
                        lbl_805E1490[index] = *source;
                        lbl_805E14E0[index] = 1;
                    }
                } else {
                    lbl_805E14D0[index] = 1;
                    int otherIndex = players->unknown0C[index];
                    Fn8040Vector3 *destination;
                    if (otherIndex == -1) {
                        destination = &result;
                    } else {
                        destination = &lbl_805E1490[otherIndex];
                        lbl_805E14E0[otherIndex] = 1;
                    }
                    if (lbl_805E14E0[index] != 0) {
                        source = &lbl_805E1490[index];
                    }
                    if (step->tracks[index].state->flags & MOTION_BLEND_FLAG_1) {
                        const Fn8040Vector3 *previous = destination;
                        if (i == 0) {
                            fn_80411328(&result, &step->state->matrix);
                            previous = &result;
                        }
                        float ratio = lbl_805E14F0[index];
                        float inverse = 1.0f - ratio;
                        destination->x = ratio * source->x + inverse * previous->x;
                        destination->y = ratio * source->y + inverse * previous->y;
                        destination->z = ratio * source->z + inverse * previous->z;
                    } else {
                        *destination = *source;
                    }
                }
            }
        }
    } while (pending);
    if (result.x != 1.0f || result.y != 1.0f || result.z != 1.0f) {
        fn_8040B790(TopMatrix(), result);
    }
}

void fn_8041DBD4(void *data) {
    MotionDynamicStepData *step = (MotionDynamicStepData *)data;
    MotionPlayerList *players = step->playerList;
    Fn8040Matrix34 result;
    Fn8040Matrix34 current;
    for (int i = 0; i < players->count; i++) {
        int index = players->listedIndices[i];
        if (index == -1) {
            break;
        }
        lbl_805E14D0[index] = 0;
        lbl_805E14E0[index] = 0;
    }
    fn_8040C204(lbl_805F13A8);
    int pending;
    do {
        pending = 0;
        for (int i = 0; i < players->count; i++) {
            int index = players->listedIndices[i];
            if (index == -1) {
                break;
            }
            if (pending && players->unknown0C[index] == -1) {
                lbl_805E14C0[index] = 1;
            } else {
                lbl_805E14C0[index] = 0;
                for (int j = i + 1; j < players->count; j++) {
                    int otherIndex = players->listedIndices[j];
                    if (otherIndex == -1) {
                        break;
                    }
                    if (lbl_805E14D0[otherIndex] == 0 && players->unknown0C[otherIndex] == index) {
                        pending = 1;
                        lbl_805E14C0[index] = 1;
                        break;
                    }
                }
            }
        }
        for (int i = 0; i < players->count; i++) {
            int index = players->listedIndices[i];
            if (index == -1) {
                break;
            }
            if (lbl_805E14D0[index] == 0) {
                const Fn8040Vector3 *rotation = &step->tracks[index].data.reflected->value;
                int unitsX =
                    (int)(MOTION_ANGLE_UNITS_PER_TURN * rotation->x / MOTION_RADIANS_PER_TURN);
                int unitsY =
                    (int)(MOTION_ANGLE_UNITS_PER_TURN * rotation->y / MOTION_RADIANS_PER_TURN);
                int unitsZ =
                    (int)(MOTION_ANGLE_UNITS_PER_TURN * rotation->z / MOTION_RADIANS_PER_TURN);
                if (lbl_805E14C0[index] != 0) {
                    if (lbl_805E14E0[index] == 0) {
                        fn_8040B6F8(TopMatrix());
                        if (unitsZ != 0) {
                            fn_8040B80C(TopMatrix(), MOTION_RADIANS_PER_TURN * (unitsZ & 0xFFFF) /
                                                         MOTION_ANGLE_UNITS_PER_TURN);
                        }
                        if (unitsY != 0) {
                            fn_8040B848(TopMatrix(), MOTION_RADIANS_PER_TURN * (unitsY & 0xFFFF) /
                                                         MOTION_ANGLE_UNITS_PER_TURN);
                        }
                        if (unitsX != 0) {
                            fn_8040B884(TopMatrix(), MOTION_RADIANS_PER_TURN * (unitsX & 0xFFFF) /
                                                         MOTION_ANGLE_UNITS_PER_TURN);
                        }
                        fn_8040C148(&lbl_805E13D0[index], fn_80411700(lbl_805F13A8));
                        lbl_805E14E0[index] = 1;
                    }
                } else {
                    lbl_805E14D0[index] = 1;
                    int otherIndex = players->unknown0C[index];
                    Fn8040Matrix34 *destination;
                    if (otherIndex == -1) {
                        destination = &result;
                    } else {
                        destination = &lbl_805E13D0[otherIndex];
                        lbl_805E14E0[otherIndex] = 1;
                    }
                    const Fn8040Matrix34 *source;
                    if (lbl_805E14E0[index] != 0) {
                        source = &lbl_805E13D0[index];
                    } else {
                        fn_8040B6F8(TopMatrix());
                        if (unitsZ != 0) {
                            fn_8040B80C(TopMatrix(), MOTION_RADIANS_PER_TURN * (unitsZ & 0xFFFF) /
                                                         MOTION_ANGLE_UNITS_PER_TURN);
                        }
                        if (unitsY != 0) {
                            fn_8040B848(TopMatrix(), MOTION_RADIANS_PER_TURN * (unitsY & 0xFFFF) /
                                                         MOTION_ANGLE_UNITS_PER_TURN);
                        }
                        if (unitsX != 0) {
                            fn_8040B884(TopMatrix(), MOTION_RADIANS_PER_TURN * (unitsX & 0xFFFF) /
                                                         MOTION_ANGLE_UNITS_PER_TURN);
                        }
                        fn_8040C148(&current, fn_80411700(lbl_805F13A8));
                        source = &current;
                    }
                    if (step->tracks[index].state->flags & MOTION_BLEND_FLAG_1) {
                        if (i == 0) {
                            fn_8041116C(destination, &step->state->matrix);
                        }
                        fn_8042062C(destination, source, destination, lbl_805E14F0[index]);
                    } else {
                        fn_8040C148(destination, source);
                    }
                }
            }
        }
    } while (pending);
    fn_8040C244(lbl_805F13A8);
    fn_8040BAC4(TopMatrix(), &result);
}

void fn_8041E124(void *data) {
    MotionDynamicStepData *step = (MotionDynamicStepData *)data;
    MotionPlayerList *players = step->playerList;
    Fn8040Vector3 result;
    Fn8040Vector3 translation;
    for (int i = 0; i < players->count; i++) {
        int index = players->listedIndices[i];
        if (index == -1) {
            break;
        }
        lbl_805E14D0[index] = 0;
        lbl_805E14E0[index] = 0;
    }
    int pending;
    do {
        pending = 0;
        for (int i = 0; i < players->count; i++) {
            int index = players->listedIndices[i];
            if (index == -1) {
                break;
            }
            if (pending && players->unknown0C[index] == -1) {
                lbl_805E14C0[index] = 1;
            } else {
                lbl_805E14C0[index] = 0;
                for (int j = i + 1; j < players->count; j++) {
                    int otherIndex = players->listedIndices[j];
                    if (otherIndex == -1) {
                        break;
                    }
                    if (lbl_805E14D0[otherIndex] == 0 && players->unknown0C[otherIndex] == index) {
                        pending = 1;
                        lbl_805E14C0[index] = 1;
                        break;
                    }
                }
            }
        }
        for (int i = 0; i < players->count; i++) {
            int index = players->listedIndices[i];
            if (index == -1) {
                break;
            }
            if (lbl_805E14D0[index] == 0) {
                const Fn8040Vector3 *source = &step->tracks[index].data.scaled->value;
                if (lbl_805E14C0[index] != 0) {
                    if (lbl_805E14E0[index] == 0) {
                        lbl_805E1490[index] = *source;
                        lbl_805E14E0[index] = 1;
                    }
                } else {
                    lbl_805E14D0[index] = 1;
                    int otherIndex = players->unknown0C[index];
                    Fn8040Vector3 *destination;
                    if (otherIndex == -1) {
                        destination = &result;
                    } else {
                        destination = &lbl_805E1490[otherIndex];
                        lbl_805E14E0[otherIndex] = 1;
                    }
                    if (lbl_805E14E0[index] != 0) {
                        source = &lbl_805E1490[index];
                    }
                    if (step->tracks[index].state->flags & MOTION_BLEND_FLAG_1) {
                        const Fn8040Vector3 *previous = destination;
                        if (i == 0) {
                            translation.x = step->state->matrix.values[0][3];
                            translation.y = step->state->matrix.values[1][3];
                            translation.z = step->state->matrix.values[2][3];
                            previous = &translation;
                        }
                        float ratio = lbl_805E14F0[index];
                        float inverse = 1.0f - ratio;
                        destination->x = ratio * source->x + inverse * previous->x;
                        destination->y = ratio * source->y + inverse * previous->y;
                        destination->z = ratio * source->z + inverse * previous->z;
                    } else {
                        *destination = *source;
                    }
                }
            }
        }
    } while (pending);
    fn_8040B59C(TopMatrix(), result);
}

void fn_8041E420(void *data) {
    MotionStaticStepData *step = (MotionStaticStepData *)data;

    if (step->value->x != 1.0f || step->value->y != 1.0f || step->value->z != 1.0f) {
        fn_8040B790(TopMatrix(), *step->value);
    }
}

/* The same as fn_8041FD70: rotates the matrix on top of the matrix stack by the three angles of
 * a vector, each truncated to whole angle units. */
void fn_8041E480(void *data) {
    MotionStaticStepData *step = (MotionStaticStepData *)data;

    int unitsX = (int)(MOTION_ANGLE_UNITS_PER_TURN * step->value->x / MOTION_RADIANS_PER_TURN);
    int unitsY = (int)(MOTION_ANGLE_UNITS_PER_TURN * step->value->y / MOTION_RADIANS_PER_TURN);
    int unitsZ = (int)(MOTION_ANGLE_UNITS_PER_TURN * step->value->z / MOTION_RADIANS_PER_TURN);

    if (unitsZ != 0) {
        fn_8040B80C(TopMatrix(),
                    MOTION_RADIANS_PER_TURN * (unitsZ & 0xFFFF) / MOTION_ANGLE_UNITS_PER_TURN);
    }
    if (unitsY != 0) {
        fn_8040B848(TopMatrix(),
                    MOTION_RADIANS_PER_TURN * (unitsY & 0xFFFF) / MOTION_ANGLE_UNITS_PER_TURN);
    }
    if (unitsX != 0) {
        fn_8040B884(TopMatrix(),
                    MOTION_RADIANS_PER_TURN * (unitsX & 0xFFFF) / MOTION_ANGLE_UNITS_PER_TURN);
    }
}

void fn_8041E5E0(void *data) {
    MotionStaticStepData *step = (MotionStaticStepData *)data;

    fn_8040B59C(TopMatrix(), *step->value);
}

void fn_8041E618(void *data) {
    MotionDynamicStepData *step = (MotionDynamicStepData *)data;
    MotionPlayerList *players = step->playerList;
    Fn8040Vector3 result(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < players->count; i++) {
        int index = players->listedIndices[i];
        if (index == -1) {
            break;
        }
        lbl_805E14D0[index] = 0;
        lbl_805E14E0[index] = 0;
    }
    int pending;
    do {
        pending = 0;
        for (int i = 0; i < players->count; i++) {
            int index = players->listedIndices[i];
            if (index == -1) {
                break;
            }
            if (pending && players->unknown0C[index] == -1) {
                lbl_805E14C0[index] = 1;
            } else {
                lbl_805E14C0[index] = 0;
                for (int j = i + 1; j < players->count; j++) {
                    int otherIndex = players->listedIndices[j];
                    if (otherIndex == -1) {
                        break;
                    }
                    if (lbl_805E14D0[otherIndex] == 0 && players->unknown0C[otherIndex] == index) {
                        pending = 1;
                        lbl_805E14C0[index] = 1;
                        break;
                    }
                }
            }
        }
        for (int i = 0; i < players->count; i++) {
            int index = players->listedIndices[i];
            if (index == -1) {
                break;
            }
            if (lbl_805E14D0[index] == 0) {
                const Fn8040Vector3 *source = &step->tracks[index].data.vector->value;
                if (lbl_805E14C0[index] != 0) {
                    if (lbl_805E14E0[index] == 0) {
                        lbl_805E1490[index] = *source;
                        lbl_805E14E0[index] = 1;
                    }
                } else {
                    lbl_805E14D0[index] = 1;
                    int otherIndex = players->unknown0C[index];
                    Fn8040Vector3 *destination;
                    if (otherIndex == -1) {
                        destination = &result;
                    } else {
                        destination = &lbl_805E1490[otherIndex];
                        lbl_805E14E0[otherIndex] = 1;
                    }
                    if (lbl_805E14E0[index] != 0) {
                        source = &lbl_805E1490[index];
                    }
                    if (step->tracks[index].state->flags & MOTION_BLEND_FLAG_1) {
                        const Fn8040Vector3 *previous = destination;
                        if (i == 0) {
                            previous = &step->state->unknown30.value;
                        }
                        float ratio = lbl_805E14F0[index];
                        float inverse = 1.0f - ratio;
                        destination->x = ratio * source->x + inverse * previous->x;
                        destination->y = ratio * source->y + inverse * previous->y;
                        destination->z = ratio * source->z + inverse * previous->z;
                    } else {
                        *destination = *source;
                    }
                }
            }
        }
    } while (pending);
    if (result.x != 1.0f || result.y != 1.0f || result.z != 1.0f) {
        fn_8040C204(lbl_805F13A8);
        for (int i = 0; i < players->count; i++) {
            if (players->listedIndices[i] == -1) {
                break;
            }
            fn_8040C148(fn_80411700(lbl_805F13A8), &lbl_805E12E0[i]);
            fn_8040B790(TopMatrix(), result);
            fn_8040C148(&lbl_805E12E0[i], fn_80411700(lbl_805F13A8));
        }
        fn_8040C244(lbl_805F13A8);
        fn_8040B790(TopMatrix(), result);
    }
}

void fn_8041E9C4(void *data) {
    MotionDynamicStepData *step = (MotionDynamicStepData *)data;
    MotionPlayerList *players = step->playerList;
    Fn8040Vector3 result(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < players->count; i++) {
        int index = players->listedIndices[i];
        if (index == -1) {
            break;
        }
        lbl_805E14D0[index] = 0;
        lbl_805E14E0[index] = 0;
    }
    int pending;
    do {
        pending = 0;
        for (int i = 0; i < players->count; i++) {
            int index = players->listedIndices[i];
            if (index == -1) {
                break;
            }
            if (pending && players->unknown0C[index] == -1) {
                lbl_805E14C0[index] = 1;
            } else {
                lbl_805E14C0[index] = 0;
                for (int j = i + 1; j < players->count; j++) {
                    int otherIndex = players->listedIndices[j];
                    if (otherIndex == -1) {
                        break;
                    }
                    if (lbl_805E14D0[otherIndex] == 0 && players->unknown0C[otherIndex] == index) {
                        pending = 1;
                        lbl_805E14C0[index] = 1;
                        break;
                    }
                }
            }
        }
        for (int i = 0; i < players->count; i++) {
            int index = players->listedIndices[i];
            if (index == -1) {
                break;
            }
            if (lbl_805E14D0[index] == 0) {
                const Fn8040Vector3 *source = &step->tracks[index].data.vector->value;
                if (lbl_805E14C0[index] != 0) {
                    if (lbl_805E14E0[index] == 0) {
                        lbl_805E1490[index] = *source;
                        lbl_805E14E0[index] = 1;
                    }
                } else {
                    lbl_805E14D0[index] = 1;
                    int otherIndex = players->unknown0C[index];
                    Fn8040Vector3 *destination;
                    if (otherIndex == -1) {
                        destination = &result;
                    } else {
                        destination = &lbl_805E1490[otherIndex];
                        lbl_805E14E0[otherIndex] = 1;
                    }
                    if (lbl_805E14E0[index] != 0) {
                        source = &lbl_805E1490[index];
                    }
                    if (step->tracks[index].state->flags & MOTION_BLEND_FLAG_1) {
                        const Fn8040Vector3 *previous = destination;
                        if (i == 0) {
                            fn_80411328(&result, &step->state->matrix);
                            previous = &result;
                        }
                        float ratio = lbl_805E14F0[index];
                        float inverse = 1.0f - ratio;
                        destination->x = ratio * source->x + inverse * previous->x;
                        destination->y = ratio * source->y + inverse * previous->y;
                        destination->z = ratio * source->z + inverse * previous->z;
                    } else {
                        *destination = *source;
                    }
                }
            }
        }
    } while (pending);
    if (result.x != 1.0f || result.y != 1.0f || result.z != 1.0f) {
        fn_8040C204(lbl_805F13A8);
        for (int i = 0; i < players->count; i++) {
            if (players->listedIndices[i] == -1) {
                break;
            }
            fn_8040C148(fn_80411700(lbl_805F13A8), &lbl_805E12E0[i]);
            fn_8040B790(TopMatrix(), result);
            fn_8040C148(&lbl_805E12E0[i], fn_80411700(lbl_805F13A8));
        }
        fn_8040C244(lbl_805F13A8);
        fn_8040B790(TopMatrix(), result);
    }
}

void fn_8041ED8C(void *data) {
    MotionDynamicStepData *step = (MotionDynamicStepData *)data;
    MotionPlayerList *players = step->playerList;
    Fn8040Matrix34 result;
    Fn8040Matrix34 rotationMatrix;
    for (int i = 0; i < players->count; i++) {
        int index = players->listedIndices[i];
        if (index == -1) {
            break;
        }
        lbl_805E14D0[index] = 0;
        lbl_805E14E0[index] = 0;
    }
    fn_8040C204(lbl_805F13A8);
    for (int i = 0; i < players->count; i++) {
        int index = players->listedIndices[i];
        if (index == -1) {
            break;
        }
        const Fn8040Vector3 *rotation = &step->tracks[index].data.reflected->value;
        if ((step->tracks[index].state->flags & MOTION_BLEND_FLAG_1) && i == 0) {
            fn_8040C148(&rotationMatrix, &step->state->matrix);
            rotationMatrix.values[2][3] = 0.0f;
            rotationMatrix.values[1][3] = 0.0f;
            rotationMatrix.values[0][3] = 0.0f;
            fn_8040C148(fn_80411700(lbl_805F13A8), &lbl_805E12E0[4]);
            fn_8040BAC4(TopMatrix(), &rotationMatrix);
            fn_8040C148(&lbl_805E12E0[4], fn_80411700(lbl_805F13A8));
        }
        fn_8040C148(fn_80411700(lbl_805F13A8), &lbl_805E12E0[i]);
        int unitsZ = (int)(MOTION_ANGLE_UNITS_PER_TURN * rotation->z / MOTION_RADIANS_PER_TURN);
        int unitsX = (int)(MOTION_ANGLE_UNITS_PER_TURN * rotation->x / MOTION_RADIANS_PER_TURN);
        int unitsY = (int)(MOTION_ANGLE_UNITS_PER_TURN * rotation->y / MOTION_RADIANS_PER_TURN);
        if (unitsZ != 0) {
            fn_8040B80C(TopMatrix(),
                        MOTION_RADIANS_PER_TURN * (unitsZ & 0xFFFF) / MOTION_ANGLE_UNITS_PER_TURN);
        }
        if (unitsY != 0) {
            fn_8040B848(TopMatrix(),
                        MOTION_RADIANS_PER_TURN * (unitsY & 0xFFFF) / MOTION_ANGLE_UNITS_PER_TURN);
        }
        if (unitsX != 0) {
            fn_8040B884(TopMatrix(),
                        MOTION_RADIANS_PER_TURN * (unitsX & 0xFFFF) / MOTION_ANGLE_UNITS_PER_TURN);
        }
        fn_8040C148(&lbl_805E12E0[i], fn_80411700(lbl_805F13A8));
    }
    int pending;
    do {
        pending = 0;
        for (int i = 0; i < players->count; i++) {
            int index = players->listedIndices[i];
            if (index == -1) {
                break;
            }
            if (pending && players->unknown0C[index] == -1) {
                lbl_805E14C0[index] = 1;
            } else {
                lbl_805E14C0[index] = 0;
                for (int j = i + 1; j < players->count; j++) {
                    int otherIndex = players->listedIndices[j];
                    if (otherIndex == -1) {
                        break;
                    }
                    if (lbl_805E14D0[otherIndex] == 0 && players->unknown0C[otherIndex] == index) {
                        pending = 1;
                        lbl_805E14C0[index] = 1;
                        break;
                    }
                }
            }
        }
        for (int i = 0; i < players->count; i++) {
            int index = players->listedIndices[i];
            if (index == -1) {
                break;
            }
            if (lbl_805E14D0[index] == 0) {
                if (lbl_805E14C0[index] != 0) {
                    if (lbl_805E14E0[index] == 0) {
                        fn_8040C148(&lbl_805E13D0[index], &lbl_805E12E0[i]);
                        lbl_805E14E0[index] = 1;
                    }
                } else {
                    lbl_805E14D0[index] = 1;
                    int otherIndex = players->unknown0C[index];
                    Fn8040Matrix34 *destination;
                    if (otherIndex == -1) {
                        destination = &result;
                    } else {
                        destination = &lbl_805E13D0[otherIndex];
                        lbl_805E14E0[otherIndex] = 1;
                    }
                    const Fn8040Matrix34 *source;
                    if (lbl_805E14E0[index] != 0) {
                        source = &lbl_805E13D0[index];
                    } else {
                        source = &lbl_805E12E0[i];
                    }
                    if (step->tracks[index].state->flags & MOTION_BLEND_FLAG_1) {
                        const Fn8040Matrix34 *previous = destination;
                        if (i == 0) {
                            previous = &lbl_805E12E0[4];
                        }
                        fn_8042062C(destination, source, previous, lbl_805E14F0[index]);
                    } else {
                        fn_8040C148(destination, source);
                    }
                }
            }
        }
    } while (pending);
    fn_8040C244(lbl_805F13A8);
    fn_804113E8(0, &result);
}

void fn_8041F290(void *data) {
    MotionDynamicStepData *step = (MotionDynamicStepData *)data;
    MotionPlayerList *players = step->playerList;
    Fn8040Vector3 initialTranslation;
    Fn8040Vector3 previousTranslation;
    Fn8040Vector3 currentTranslation;
    Fn8040Vector3 result;
    Fn8040Matrix34 matrix;
    for (int i = 0; i < players->count; i++) {
        int index = players->listedIndices[i];
        if (index == -1) {
            break;
        }
        lbl_805E14D0[index] = 0;
        lbl_805E14E0[index] = 0;
    }
    fn_8040C204(lbl_805F13A8);
    for (int i = 0; i < players->count; i++) {
        int index = players->listedIndices[i];
        if (index == -1) {
            break;
        }
        const Fn8040Vector3 *translation = &step->tracks[index].data.scaled->value;
        if ((step->tracks[index].state->flags & MOTION_BLEND_FLAG_1) && i == 0) {
            initialTranslation.x = step->state->matrix.values[0][3];
            initialTranslation.y = step->state->matrix.values[1][3];
            initialTranslation.z = step->state->matrix.values[2][3];
            fn_8040C148(fn_80411700(lbl_805F13A8), &lbl_805E12E0[4]);
            fn_8040B59C(TopMatrix(), initialTranslation);
            fn_8040C148(&lbl_805E12E0[4], fn_80411700(lbl_805F13A8));
        }
        fn_8040C148(fn_80411700(lbl_805F13A8), &lbl_805E12E0[i]);
        fn_8040B59C(TopMatrix(), *translation);
        fn_8040C148(&lbl_805E12E0[i], fn_80411700(lbl_805F13A8));
    }
    int pending;
    do {
        pending = 0;
        for (int i = 0; i < players->count; i++) {
            int index = players->listedIndices[i];
            if (index == -1) {
                break;
            }
            if (pending && players->unknown0C[index] == -1) {
                lbl_805E14C0[index] = 1;
            } else {
                lbl_805E14C0[index] = 0;
                for (int j = i + 1; j < players->count; j++) {
                    int otherIndex = players->listedIndices[j];
                    if (otherIndex == -1) {
                        break;
                    }
                    if (lbl_805E14D0[otherIndex] == 0 && players->unknown0C[otherIndex] == index) {
                        pending = 1;
                        lbl_805E14C0[index] = 1;
                        break;
                    }
                }
            }
        }
        for (int i = 0; i < players->count; i++) {
            int index = players->listedIndices[i];
            if (index == -1) {
                break;
            }
            if (lbl_805E14D0[index] == 0) {
                currentTranslation.x = lbl_805E12E0[i].values[0][3];
                currentTranslation.y = lbl_805E12E0[i].values[1][3];
                currentTranslation.z = lbl_805E12E0[i].values[2][3];
                const Fn8040Vector3 *source = &currentTranslation;
                if (lbl_805E14C0[index] != 0) {
                    if (lbl_805E14E0[index] == 0) {
                        lbl_805E1490[index] = *source;
                        lbl_805E14E0[index] = 1;
                    }
                } else {
                    lbl_805E14D0[index] = 1;
                    int otherIndex = players->unknown0C[index];
                    Fn8040Vector3 *destination;
                    if (otherIndex == -1) {
                        destination = &result;
                    } else {
                        destination = &lbl_805E1490[otherIndex];
                        lbl_805E14E0[otherIndex] = 1;
                    }
                    if (lbl_805E14E0[index] != 0) {
                        source = &lbl_805E1490[index];
                    }
                    if (step->tracks[index].state->flags & MOTION_BLEND_FLAG_1) {
                        const Fn8040Vector3 *previous = destination;
                        if (i == 0) {
                            previousTranslation.x = lbl_805E12E0[4].values[0][3];
                            previousTranslation.y = lbl_805E12E0[4].values[1][3];
                            previousTranslation.z = lbl_805E12E0[4].values[2][3];
                            previous = &previousTranslation;
                        }
                        float ratio = lbl_805E14F0[index];
                        float inverse = 1.0f - ratio;
                        destination->x = ratio * source->x + inverse * previous->x;
                        destination->y = ratio * source->y + inverse * previous->y;
                        destination->z = ratio * source->z + inverse * previous->z;
                    } else {
                        *destination = *source;
                    }
                }
            }
        }
    } while (pending);
    fn_8040C244(lbl_805F13A8);
    fn_8040C148(&matrix, fn_80411700(lbl_805F13A8));
    matrix.values[0][3] = result.x;
    matrix.values[1][3] = result.y;
    matrix.values[2][3] = result.z;
    fn_8040C148(fn_80411700(lbl_805F13A8), &matrix);
}

void fn_8041F6F0(void *data) {
    MotionStaticStepData *step = (MotionStaticStepData *)data;
    const Fn8040Vector3 *scale = step->value;
    if (scale->x != 1.0f || scale->y != 1.0f || scale->z != 1.0f) {
        fn_8040C204(lbl_805F13A8);
        for (int i = 0; i < 5; i++) {
            fn_8040C148(fn_80411700(lbl_805F13A8), &lbl_805E12E0[i]);
            fn_8040B790(TopMatrix(), *scale);
            fn_8040C148(&lbl_805E12E0[i], fn_80411700(lbl_805F13A8));
        }
        fn_8040C244(lbl_805F13A8);
        fn_8040B790(TopMatrix(), *scale);
    }
}

void fn_8041F7D0(void *data) {
    MotionStaticStepData *step = (MotionStaticStepData *)data;
    int unitsX = (int)(MOTION_ANGLE_UNITS_PER_TURN * step->value->x / MOTION_RADIANS_PER_TURN);
    int unitsY = (int)(MOTION_ANGLE_UNITS_PER_TURN * step->value->y / MOTION_RADIANS_PER_TURN);
    int unitsZ = (int)(MOTION_ANGLE_UNITS_PER_TURN * step->value->z / MOTION_RADIANS_PER_TURN);

    fn_8040C204(lbl_805F13A8);
    for (int i = 0; i < 5; i++) {
        fn_8040C148(fn_80411700(lbl_805F13A8), &lbl_805E12E0[i]);
        if (unitsZ != 0) {
            fn_8040B80C(TopMatrix(),
                        MOTION_RADIANS_PER_TURN * (unitsZ & 0xFFFF) / MOTION_ANGLE_UNITS_PER_TURN);
        }
        if (unitsY != 0) {
            fn_8040B848(TopMatrix(),
                        MOTION_RADIANS_PER_TURN * (unitsY & 0xFFFF) / MOTION_ANGLE_UNITS_PER_TURN);
        }
        if (unitsX != 0) {
            fn_8040B884(TopMatrix(),
                        MOTION_RADIANS_PER_TURN * (unitsX & 0xFFFF) / MOTION_ANGLE_UNITS_PER_TURN);
        }
        fn_8040C148(&lbl_805E12E0[i], fn_80411700(lbl_805F13A8));
    }
    fn_8040C244(lbl_805F13A8);
    if (unitsZ != 0) {
        fn_8040B80C(TopMatrix(),
                    MOTION_RADIANS_PER_TURN * (unitsZ & 0xFFFF) / MOTION_ANGLE_UNITS_PER_TURN);
    }
    if (unitsY != 0) {
        fn_8040B848(TopMatrix(),
                    MOTION_RADIANS_PER_TURN * (unitsY & 0xFFFF) / MOTION_ANGLE_UNITS_PER_TURN);
    }
    if (unitsX != 0) {
        fn_8040B884(TopMatrix(),
                    MOTION_RADIANS_PER_TURN * (unitsX & 0xFFFF) / MOTION_ANGLE_UNITS_PER_TURN);
    }
}

void fn_8041FA20(void *data) {
    MotionStaticStepData *step = (MotionStaticStepData *)data;
    const Fn8040Vector3 *offset = step->value;
    fn_8040C204(lbl_805F13A8);
    for (int i = 0; i < 5; i++) {
        fn_8040C148(fn_80411700(lbl_805F13A8), &lbl_805E12E0[i]);
        fn_8040B59C(TopMatrix(), *offset);
        fn_8040C148(&lbl_805E12E0[i], fn_80411700(lbl_805F13A8));
    }
    fn_8040C244(lbl_805F13A8);
    fn_8040B59C(TopMatrix(), *offset);
}

void fn_8041FAD8(void *data) {
    MotionDynamicStepData *step = (MotionDynamicStepData *)data;
    Fn8040Vector3 *scale = &step->tracks[step->playerList->listedIndices[0]].data.vector->value;
    if (scale->x != 1.0f || scale->y != 1.0f || scale->z != 1.0f) {
        fn_8040B790(TopMatrix(), *scale);
    }
}

void fn_8041FB4C(void *data) {
    MotionDynamicStepData *step = (MotionDynamicStepData *)data;
    Fn8040Vector3 *rotation =
        &step->tracks[step->playerList->listedIndices[0]].data.reflected->value;
    int unitsX = (int)(MOTION_ANGLE_UNITS_PER_TURN * rotation->x / MOTION_RADIANS_PER_TURN);
    int unitsY = (int)(MOTION_ANGLE_UNITS_PER_TURN * rotation->y / MOTION_RADIANS_PER_TURN);
    int unitsZ = (int)(MOTION_ANGLE_UNITS_PER_TURN * rotation->z / MOTION_RADIANS_PER_TURN);

    if (unitsZ != 0) {
        fn_8040B80C(TopMatrix(),
                    MOTION_RADIANS_PER_TURN * (unitsZ & 0xFFFF) / MOTION_ANGLE_UNITS_PER_TURN);
    }
    if (unitsY != 0) {
        fn_8040B848(TopMatrix(),
                    MOTION_RADIANS_PER_TURN * (unitsY & 0xFFFF) / MOTION_ANGLE_UNITS_PER_TURN);
    }
    if (unitsX != 0) {
        fn_8040B884(TopMatrix(),
                    MOTION_RADIANS_PER_TURN * (unitsX & 0xFFFF) / MOTION_ANGLE_UNITS_PER_TURN);
    }
}

void fn_8041FCC0(void *data) {
    MotionDynamicStepData *step = (MotionDynamicStepData *)data;
    fn_8040B59C(TopMatrix(), step->tracks[step->playerList->listedIndices[0]].data.scaled->value);
}

/* The same as fn_8041E420. */
void fn_8041FD0C(void *data) {
    MotionStaticStepData *step = (MotionStaticStepData *)data;

    if (step->value->x != 1.0f || step->value->y != 1.0f || step->value->z != 1.0f) {
        fn_8040B790(TopMatrix(), *step->value);
    }
}
