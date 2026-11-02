/* Quaternion interpolation and matrix conversion (behaviour inferred); the original function
 * names, filename and complete translation-unit boundaries are unknown. */
#include <cmath>

#include "Game/fn_8040_vector_types.h"

static const int nextIndex[3] = {1, 2, 0};

extern "C" {

void fn_8040C55C(Fn8040Vector4 *quaternion, Fn8040Matrix34 *matrix) {
    float trace = fn_8040C168(matrix, 0, 0) + fn_8040C168(matrix, 1, 1) + fn_8040C168(matrix, 2, 2);
    if (trace > 0.0f) {
        float root = std::sqrt(1.0f + trace);
        float scale = 0.5f / root;
        float z = scale * (fn_8040C168(matrix, 1, 0) - fn_8040C168(matrix, 0, 1));
        float y = scale * (fn_8040C168(matrix, 0, 2) - fn_8040C168(matrix, 2, 0));
        float x = scale * (fn_8040C168(matrix, 2, 1) - fn_8040C168(matrix, 1, 2));
        quaternion->x = x;
        quaternion->y = y;
        quaternion->z = z;
        quaternion->w = 0.5f * root;
    } else {
        int i = 0;
        if (fn_8040C168(matrix, 1, 1) > fn_8040C168(matrix, 0, 0)) {
            i = 1;
        }
        if (fn_8040C168(matrix, 2, 2) > fn_8040C168(matrix, i, i)) {
            i = 2;
        }
        int j = nextIndex[i];
        int k = nextIndex[j];
        float components[3];
        float root = std::sqrt(1.0f + (fn_8040C168(matrix, i, i) -
                                       (fn_8040C168(matrix, j, j) + fn_8040C168(matrix, k, k))));
        components[i] = 0.5f * root;
        if (root != 0.0f) {
            root = 0.5f / root;
        }
        components[j] = root * (fn_8040C168(matrix, i, j) + fn_8040C168(matrix, j, i));
        components[k] = root * (fn_8040C168(matrix, i, k) + fn_8040C168(matrix, k, i));
        float w = root * (fn_8040C168(matrix, k, j) - fn_8040C168(matrix, j, k));
        quaternion->x = components[0];
        quaternion->y = components[1];
        quaternion->z = components[2];
        quaternion->w = w;
    }
}

void fn_8040C3EC(Fn8040Vector4 *result, const Fn8040Vector4 *from, const Fn8040Vector4 *to,
                 float t) {
    float dot = from->x * to->x + from->y * to->y + from->z * to->z + from->w * to->w;
    float toWeight = 1.0f;
    if (dot < 0.0f) {
        dot = -dot;
        toWeight = -toWeight;
    }

    float fromWeight;
    if (dot <= 0.99999f) {
        float angle = fn_801B1DB0(dot);
        float sine = std::sin(angle);
        fromWeight = std::sin((1.0f - t) * angle) / sine;
        toWeight *= std::sin(t * angle) / sine;
    } else {
        fromWeight = 1.0f - t;
        toWeight *= t;
    }

    result->x = fromWeight * from->x + toWeight * to->x;
    result->y = fromWeight * from->y + toWeight * to->y;
    result->z = fromWeight * from->z + toWeight * to->z;
    result->w = fromWeight * from->w + toWeight * to->w;
}
}
