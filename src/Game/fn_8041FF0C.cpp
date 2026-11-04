/* Control of motions: fn_8041FF0C to fn_804201C0 (see Game/fn_804157E8_motion_control.h for the
 * file and its pieces). */
#include "Game/fn_804157E8_motion_control.h"

/* Reflects the components of a vector about the values that its options hold for them. */
static inline void ReflectVector(Fn8040Vector3 *value, const PackNode *options) {
    if (options->values[3] & MOTION_AXIS_REFLECT_X) {
        value->x = 2.0f * options->rows[1][0] - value->x;
    }
    if (options->values[3] & MOTION_AXIS_REFLECT_Y) {
        value->y = 2.0f * options->rows[1][1] - value->y;
    }
    if (options->values[3] & MOTION_AXIS_REFLECT_Z) {
        value->z = 2.0f * options->rows[1][2] - value->z;
    }
}

/* Moves the components of a vector from the values that the options of its node hold for them to
 * the values that the options of the mirror node hold, reflected where the options say so. */
static inline void MirrorVector(Fn8040Vector3 *value, const PackNode *options,
                                const PackNode *mirrorOptions) {
    if (options->values[3] & MOTION_AXIS_REFLECT_X) {
        value->x = mirrorOptions->rows[1][0] - (value->x - options->rows[1][0]);
    } else {
        value->x = mirrorOptions->rows[1][0] + (value->x - options->rows[1][0]);
    }
    if (options->values[3] & MOTION_AXIS_REFLECT_Y) {
        value->y = mirrorOptions->rows[1][1] - (value->y - options->rows[1][1]);
    } else {
        value->y = mirrorOptions->rows[1][1] + (value->y - options->rows[1][1]);
    }
    if (options->values[3] & MOTION_AXIS_REFLECT_Z) {
        value->z = mirrorOptions->rows[1][2] - (value->z - options->rows[1][2]);
    } else {
        value->z = mirrorOptions->rows[1][2] + (value->z - options->rows[1][2]);
    }
}

/* The second callback of a track whose node has a mirror: when the mirror node has a track of the
 * same kind, moves each of the two vectors to the other node and exchanges them; otherwise reflects
 * the vector of the track as fn_80420158 does. */
void fn_8041FF0C(MotionReflectedVectorTrack *track) {
    PackNode *options = track->options;
    PackNode *mirrorOptions = track->mirrorOptions;
    MotionTrackNode *mirrorTrackNode = track->mirrorTrackNode;

    if (mirrorOptions->values[1] & MOTION_NODE_REFLECTED_TRACK) {
        MotionReflectedVectorTrack *mirrorTrack =
            (MotionReflectedVectorTrack *)(mirrorTrackNode + 1);
        Fn8040Vector3 saved;

        MirrorVector(&track->value, options, mirrorOptions);
        MirrorVector(&mirrorTrack->value, mirrorOptions, options);
        fn_800091D0(&saved, &track->value);
        fn_800091D0(&track->value, &mirrorTrack->value);
        fn_800091D0(&mirrorTrack->value, &saved);
    } else {
        ReflectVector(&track->value, options);
    }
}

void fn_80420158(MotionReflectedVectorTrack *track) {
    ReflectVector(&track->value, track->options);
}

/* The second callback of a scaled track whose node has a mirror: negates the vector of the track as
 * fn_804202B8 does, and when the mirror node has a track of the same kind, negates the mirror's
 * vector as its options say and exchanges the two vectors. */
void fn_804201C0(MotionScaledVectorTrack *track) {
    PackNode *options = track->options;
    PackNode *mirrorOptions = track->mirrorOptions;
    MotionTrackNode *mirrorTrackNode = track->mirrorTrackNode;

    NegateVector(&track->value, options);
    if (mirrorOptions->values[1] & MOTION_NODE_SCALED_TRACK) {
        MotionScaledVectorTrack *mirrorTrack = (MotionScaledVectorTrack *)(mirrorTrackNode + 1);
        Fn8040Vector3 saved;

        NegateVector(&mirrorTrack->value, mirrorOptions);
        fn_800091D0(&saved, &track->value);
        fn_800091D0(&track->value, &mirrorTrack->value);
        fn_800091D0(&mirrorTrack->value, &saved);
    }
}
