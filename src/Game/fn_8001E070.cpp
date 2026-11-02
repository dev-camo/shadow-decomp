/* Rotation-order helper (behaviour inferred; original name and filename unknown). */
#include "Game/fn_8040_vector_types.h"

extern "C" void fn_8001E070(Fn8040Matrix34 *matrix, const Fn8040Vector3 *rotation, int order) {
    switch (order) {
    case 0:
        if (rotation->z != 0.0f) {
            Fn8040Matrix34 temporary;
            fn_8040B0C0(&temporary, rotation->z);
            fn_8040BAC4(matrix, &temporary);
        }
        if (rotation->y != 0.0f) {
            Fn8040Matrix34 temporary;
            fn_8040B138(&temporary, rotation->y);
            fn_8040BAC4(matrix, &temporary);
        }
        if (rotation->x != 0.0f) {
            Fn8040Matrix34 temporary;
            fn_8040B1B0(&temporary, rotation->x);
            fn_8040BAC4(matrix, &temporary);
        }
        break;
    case 1:
        if (rotation->z != 0.0f) {
            Fn8040Matrix34 temporary;
            fn_8040B0C0(&temporary, rotation->z);
            fn_8040BAC4(matrix, &temporary);
        }
        if (rotation->x != 0.0f) {
            Fn8040Matrix34 temporary;
            fn_8040B1B0(&temporary, rotation->x);
            fn_8040BAC4(matrix, &temporary);
        }
        if (rotation->y != 0.0f) {
            Fn8040Matrix34 temporary;
            fn_8040B138(&temporary, rotation->y);
            fn_8040BAC4(matrix, &temporary);
        }
        break;
    case 2:
        if (rotation->y != 0.0f) {
            Fn8040Matrix34 temporary;
            fn_8040B138(&temporary, rotation->y);
            fn_8040BAC4(matrix, &temporary);
        }
        if (rotation->x != 0.0f) {
            Fn8040Matrix34 temporary;
            fn_8040B1B0(&temporary, rotation->x);
            fn_8040BAC4(matrix, &temporary);
        }
        if (rotation->z != 0.0f) {
            Fn8040Matrix34 temporary;
            fn_8040B0C0(&temporary, rotation->z);
            fn_8040BAC4(matrix, &temporary);
        }
        break;
    }
}
