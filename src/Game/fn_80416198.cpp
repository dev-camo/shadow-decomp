/* Control of motions: stMtnAbortBlend to fn_8041FD0C (see Game/fn_804157E8_motion_control.h for
 * the file and its pieces). */
#include <stdio.h>
#include <string.h>

#include "Game/fn_804110EC_allocator_hooks.h"
#include "Game/fn_80411714_resource_groups.h"
#include "Game/fn_804157E8_motion_control.h"

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
        result = player->blend->flags & 0x1;
        break;
    case 6:
        result = (player->flags & 0x800000) != 0;
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
        result = (player->flags & 0x10000) != 0;
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

/* For each listed active player: selects the curve evaluators for the direction of its speed,
 * hands its range to the curve state (with the current frame truncated to a whole number when the
 * control has MOTION_CONTROL_WHOLE_FRAMES) and runs the callbacks of the tracks of the player. */
void fn_804188A0(_stMtnPlayWork *control) {
    int i;
    MotionPlayer *player;
    MotionTrackNode *track;

    for (i = 0; i < control->playerCount; i++) {
        int playerIndex = control->listedIndices[i];

        if (playerIndex == -1) {
            break;
        }
        player = &control->players[playerIndex];
        if (player->flags & MOTION_PLAYER_ACTIVE) {
            if (player->range.speed >= 0.0f) {
                lbl_805F13D0 = fn_8041525C;
                lbl_805F13D4 = fn_80414FD0;
                lbl_805F13D8 = fn_80414CBC;
            } else if (player->range.speed < 0.0f) {
                lbl_805F13D0 = fn_804149E0;
                lbl_805F13D4 = fn_8041474C;
                lbl_805F13D8 = fn_80414438;
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
                    track->first((MotionTrackData *)(track + 1));
                }
            }
            if (player->flags & MOTION_PLAYER_FLAG_8) {
                for (track = player->tracks; track != 0; track = track->next) {
                    if (track->second != 0) {
                        track->second((MotionTrackData *)(track + 1));
                    }
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
        UpdateCurves(step->tracks[playerIndex].data.vector->curves);
        step = step->next;
    }
    if (node->flags & MOTION_NODE_SCALED_TRACK) {
        UpdateCurves(step->tracks[playerIndex].data.scaled->curves);
    }
    step = step->next;
    if (!(node->flags & MOTION_NODE_TRACK_40) && (node->flags & MOTION_NODE_REFLECTED_TRACK)) {
        UpdateCurves(step->tracks[playerIndex].data.reflected->curves);
    }
    step = step->next;
    if (node->flags & MOTION_NODE_TRACK_380) {
        UpdateCurves(step->tracks[playerIndex].data.vector->curves);
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
        MotionVectorTrack *track = step->tracks[playerIndex].data.vector;

        track->value.x = track->value.y = track->value.z = 1.0f;
        ResetCurves(track->curves);
        step = step->next;
    }
    if (node->flags & MOTION_NODE_SCALED_TRACK) {
        MotionScaledVectorTrack *track = step->tracks[playerIndex].data.scaled;

        memcpy(&track->value, node->definition->rows[0], sizeof(Fn8040Vector3));
        ResetCurves(track->curves);
    }
    step = step->next;
    if (!(node->flags & MOTION_NODE_TRACK_40) && (node->flags & MOTION_NODE_REFLECTED_TRACK)) {
        MotionReflectedVectorTrack *track = step->tracks[playerIndex].data.reflected;

        memcpy(track, node->definition->rows[1], sizeof(node->definition->rows[1]));
        ResetCurves(track->curves);
    }
    step = step->next;
    if (node->flags & MOTION_NODE_TRACK_380) {
        MotionVectorTrack *track = step->tracks[playerIndex].data.vector;

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
        for (i = 0; i < control->playerCount; i++) {
            int index = control->listedIndices[i];
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

int fn_8041D1A0(MotionNode *node, unsigned int nodeId, int arg2, int arg3) {
    if (nodeId == node->definition->id) {
        node->unknown1C = arg2;
        node->unknown20 = arg3;
        return 1;
    }
    if (node->first != 0) {
        if (fn_8041D1A0(node->first, nodeId, arg2, arg3)) {
            return 1;
        }
    }
    if (node->second != 0) {
        if (fn_8041D1A0(node->second, nodeId, arg2, arg3)) {
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

void fn_8041E420(MotionScaleStep *step) {
    if (step->scale->x != 1.0f || step->scale->y != 1.0f || step->scale->z != 1.0f) {
        fn_8040B790(TopMatrix(), step->scale);
    }
}

/* The same as fn_8041FD70: rotates the matrix on top of the matrix stack by the three angles of
 * a vector, each truncated to whole angle units. */
void fn_8041E480(MotionRotationStep *step) {
    int unitsX = (int)(MOTION_ANGLE_UNITS_PER_TURN * step->rotation->x / MOTION_RADIANS_PER_TURN);
    int unitsY = (int)(MOTION_ANGLE_UNITS_PER_TURN * step->rotation->y / MOTION_RADIANS_PER_TURN);
    int unitsZ = (int)(MOTION_ANGLE_UNITS_PER_TURN * step->rotation->z / MOTION_RADIANS_PER_TURN);

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

void fn_8041E5E0(MotionOffsetStep *step) {
    fn_8040B59C(TopMatrix(), step->offset);
}

/* The same as fn_8041E420. */
void fn_8041FD0C(MotionScaleStep *step) {
    if (step->scale->x != 1.0f || step->scale->y != 1.0f || step->scale->z != 1.0f) {
        fn_8040B790(TopMatrix(), step->scale);
    }
}
