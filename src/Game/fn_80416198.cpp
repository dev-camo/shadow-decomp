/* Control of motions: stMtnAbortBlend to fn_8041FD0C (see Game/fn_804157E8_motion_control.h for
 * the file and its pieces). */
#include <stdio.h>

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
        fn_800091D0(&control->vectors[0].value, position);
    }
    if (rotation == 0) {
        return;
    }
    fn_800091D0(&control->vectors[1].value, rotation);

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

/* Resets the entries of a player in every node of the tree below (and including) a node. */
void fn_8041CC18(MotionNode *node, int playerIndex) {
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
void fn_8041CD6C(MotionNode *node, int playerIndex) {
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

MotionTransform *fn_8041D31C(MotionNode *node, unsigned int nodeId) {
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

void fn_8041E420(MotionScaleStep *step) {
    if (step->scale->x != 1.0f || step->scale->y != 1.0f || step->scale->z != 1.0f) {
        fn_8040B790(TopMatrix(), step->scale);
    }
}

void fn_8041E5E0(MotionOffsetStep *step) {
    fn_8040B59C(TopMatrix(), step->offset);
}

void fn_8041FD0C(MotionScaleStep *step) {
    if (step->scale->x != 1.0f || step->scale->y != 1.0f || step->scale->z != 1.0f) {
        fn_8040B790(TopMatrix(), step->scale);
    }
}
