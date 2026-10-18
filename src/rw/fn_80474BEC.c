/* RenderWare 2D toolkit paths: building subpaths of lines and curves, flattening curves, locking
 * a path's points into lists for editing and unlocking them into arrays, and destroying paths.
 * The original file name is unknown. */
#include "rw/rt2d.h"
#include <string.h>

/* The object IDs of the memory hints (inferred) */
#define RT2DMEMID 0x190
#define RT2DPATHMEMID 0x1A1

RwFreeList lbl_805E26A8; /* the path free list */

RwInt32 lbl_805EDE50 = 32; /* path free list entries per block */
RwInt32 lbl_805EDE54 = 1;  /* path free list blocks to preallocate */

/* Destroys one subpath and returns the next */
Rt2dPath *fn_80474BEC(Rt2dPath *path) {
    Rt2dPath *next = path->next;

    if (path->points != NULL) {
        if (path->numPoints == -1) {
            fn_8047DFC8(path->points);
        } else {
            RwFree(path->points);
        }
    }
    path->points = NULL;
    RwFreeListFree(lbl_805E1950.pathFreeList, path);
    return next;
}

/* Adds a line to a subpath, averaging the normal at the previous point with the line's */
#define PATHADDLINE(subpath, px, py)                                                               \
    do {                                                                                           \
        RwInt32 numPoints_ = fn_8047E03C((subpath)->points);                                       \
        rt2dPathNode *node_ = fn_8047DDBC((subpath)->points, rwMEMHINTDUR_EVENT | RT2DMEMID);      \
        RwReal dy_;                                                                                \
        RwReal dx_;                                                                                \
        RwReal lengthSq_;                                                                          \
        RwReal length_;                                                                            \
        RwReal recip_;                                                                             \
                                                                                                   \
        node_->type = rt2dLINETO;                                                                  \
        node_->pos.x = (px);                                                                       \
        node_->pos.y = (py);                                                                       \
        dy_ = (py) - node_[-1].pos.y;                                                              \
        dx_ = node_[-1].pos.x - (px);                                                              \
        lengthSq_ = dy_ * dy_ + dx_ * dx_;                                                         \
        length_ = fn_80482340(lengthSq_);                                                          \
        recip_ = fn_80482394(lengthSq_);                                                           \
        node_->normal.x = dy_ * recip_;                                                            \
        node_->normal.y = dx_ * recip_;                                                            \
        node_->dist = node_[-1].dist + length_;                                                    \
        if (numPoints_ > 1) {                                                                      \
            RwReal sumX_;                                                                          \
            RwReal sumY_;                                                                          \
            RwReal scale_;                                                                         \
                                                                                                   \
            sumX_ = node_[-1].normal.x + node_->normal.x;                                          \
            sumY_ = node_[-1].normal.y + node_->normal.y;                                          \
            scale_ = 1.0f / (node_[-1].normal.x * sumX_ + node_[-1].normal.y * sumY_);             \
            node_[-1].normal.x = sumX_ * scale_;                                                   \
            node_[-1].normal.y = sumY_ * scale_;                                                   \
        } else {                                                                                   \
            node_[-1].normal = node_->normal;                                                      \
        }                                                                                          \
    } while (0)

/* Starts a subpath at a point; a subpath that has points continues in a new one */
Rt2dPath *fn_80474C7C(Rt2dPath *path, RwReal x, RwReal y) {
    if (fn_8047E03C(path->curr->points) == 0) {
        rt2dPathNode *node = fn_8047DDBC(path->curr->points, rwMEMHINTDUR_EVENT | RT2DMEMID);

        node->type = rt2dMOVETO;
        node->pos.x = x;
        node->pos.y = y;
        node->normal.x = 0.0f;
        node->normal.y = 0.0f;
        node->dist = 0.0f;
    } else {
        path->curr->next = fn_80476490();
        fn_80476510(path->curr->next);
        path->curr = path->curr->next;
        fn_80474C7C(path, x, y);
    }
    return path;
}

Rt2dPath *fn_80474EA0(Rt2dPath *path, RwReal x, RwReal y) {
    PATHADDLINE(path->curr, x, y);
    return path;
}

/* Sets a curve point's normal and length from the point before it */
#define CURVEPOINT(node, prev, px, py)                                                             \
    do {                                                                                           \
        RwReal dx_;                                                                                \
        RwReal dy_;                                                                                \
        RwReal lengthSq_;                                                                          \
        RwReal length_;                                                                            \
        RwReal recip_;                                                                             \
                                                                                                   \
        (node)->type = rt2dCURVETO;                                                                \
        (node)->pos.x = (px);                                                                      \
        (node)->pos.y = (py);                                                                      \
        dx_ = (prev)->pos.x - (px);                                                                \
        dy_ = (py) - (prev)->pos.y;                                                                \
        lengthSq_ = dy_ * dy_ + dx_ * dx_;                                                         \
        length_ = fn_80482340(lengthSq_);                                                          \
        recip_ = fn_80482394(lengthSq_);                                                           \
        (node)->normal.x = dy_ * recip_;                                                           \
        (node)->normal.y = dx_ * recip_;                                                           \
        (node)->dist = (prev)->dist + length_;                                                     \
    } while (0)

/* Adds a cubic curve through two control points */
Rt2dPath *fn_80474FF8(Rt2dPath *path, RwReal x1, RwReal y1, RwReal x2, RwReal y2, RwReal x3,
                      RwReal y3) {
    Rt2dPath *curr = path->curr;
    RwInt32 numPoints = fn_8047E03C(path->curr->points);
    rt2dPathNode *node = fn_8047DEA0(curr->points, 3, rwMEMHINTDUR_EVENT | RT2DMEMID);

    CURVEPOINT(&node[0], &node[-1], x1, y1);
    if (numPoints > 1) {
        RwReal sumX;
        RwReal sumY;
        RwReal scale;

        sumX = node[-1].normal.x + node[0].normal.x;
        sumY = node[-1].normal.y + node[0].normal.y;
        scale = 1.0f / (node[-1].normal.x * sumX + node[-1].normal.y * sumY);
        node[-1].normal.x = sumX * scale;
        node[-1].normal.y = sumY * scale;
    } else {
        node[-1].normal.x = node[0].normal.x;
        node[-1].normal.y = node[0].normal.y;
    }
    CURVEPOINT(&node[1], &node[0], x2, y2);
    CURVEPOINT(&node[2], &node[1], x3, y3);
    path->curr->flat = FALSE;
    return path;
}

/* Closes the current subpath back to its first point */
Rt2dPath *fn_80475260(Rt2dPath *path) {
    Rt2dPath *curr = path->curr;
    RwInt32 numPoints = fn_8047E03C(curr->points);
    rt2dPathNode *first = fn_8047DFAC(curr->points);
    rt2dPathNode *last = fn_8047E0CC(curr->points, numPoints - 1);
    RwV2d pos;
    RwReal sumX;
    RwReal sumY;
    RwReal scale;
    RwReal dx;
    RwReal dy;

    pos = first->pos;
    dy = pos.y - last->pos.y;
    dx = last->pos.x - pos.x;
    if (0.0f != dy || 0.0f != dx) {
        PATHADDLINE(path->curr, pos.x, pos.y);
        first = fn_8047DFAC(curr->points);
        last = fn_8047E0CC(curr->points, numPoints);
    }
    sumX = first->normal.x + last->normal.x;
    sumY = first->normal.y + last->normal.y;
    scale = 1.0f / (first->normal.x * sumX + first->normal.y * sumY);
    last->normal.x = sumX * scale;
    last->normal.y = sumY * scale;
    first->normal = last->normal;
    curr->closed = TRUE;
    return path;
}

Rt2dPath *fn_8047548C(void) {
    Rt2dPath *path = lbl_805E1950.scratchPath;

    fn_80475E2C(path->next);
    path->next = NULL;
    fn_8047DDB0(path->points);
    path->closed = FALSE;
    path->flat = TRUE;
    path->curr = path;
    return lbl_805E1950.scratchPath;
}

/* Adds a cubic curve as lines, halving it until both control points are within a pixel of the
 * chord or the depth is used up */
void fn_804754F4(Rt2dPath *path, RwV2d *bezier, RwInt32 depth) {
    RwV2d chord;
    RwV2d control;
    RwV2d left[4];
    RwV2d right[4];
    RwBool flat;

    depth--;
    if (depth < 0) {
        PATHADDLINE(path, bezier[3].x, bezier[3].y);
        return;
    }

    chord.x = bezier[3].x - bezier[0].x;
    chord.y = bezier[3].y - bezier[0].y;
    control.x = bezier[1].x - bezier[0].x;
    control.y = bezier[1].y - bezier[0].y;
    flat =
        (chord.x * chord.x + chord.y * chord.y) *
            ((control.x * control.x + control.y * control.y) - lbl_805E1950.pixelSizeSq) <
        (control.x * chord.x + control.y * chord.y) * (control.x * chord.x + control.y * chord.y);
    if (flat) {
        control.x = bezier[2].x - bezier[0].x;
        control.y = bezier[2].y - bezier[0].y;
        flat = (chord.x * chord.x + chord.y * chord.y) *
                   ((control.x * control.x + control.y * control.y) - lbl_805E1950.pixelSizeSq) <
               (control.x * chord.x + control.y * chord.y) *
                   (control.x * chord.x + control.y * chord.y);
    }
    if (flat) {
        PATHADDLINE(path, bezier[3].x, bezier[3].y);
        return;
    }

    left[0] = bezier[0];
    left[1].x = 0.5f * (bezier[0].x + bezier[1].x);
    left[1].y = 0.5f * (bezier[0].y + bezier[1].y);
    right[2].x = 0.5f * (bezier[2].x + bezier[3].x);
    right[2].y = 0.5f * (bezier[2].y + bezier[3].y);
    right[3] = bezier[3];
    {
        RwV2d middle;

        middle.x = 0.5f * (bezier[1].x + bezier[2].x);
        middle.y = 0.5f * (bezier[1].y + bezier[2].y);
        left[2].x = 0.5f * (left[1].x + middle.x);
        left[2].y = 0.5f * (left[1].y + middle.y);
        right[1].x = 0.5f * (right[2].x + middle.x);
        right[1].y = 0.5f * (right[2].y + middle.y);
    }
    left[3].x = 0.5f * (left[2].x + right[1].x);
    left[3].y = 0.5f * (left[2].y + right[1].y);
    right[0] = left[3];
    fn_804754F4(path, left, depth);
    fn_804754F4(path, right, depth);
}

/* Adds a subpath's points to a path, with its curves as lines */
Rt2dPath *fn_804758C8(Rt2dPath *flat, Rt2dPath *path) {
    RwInt32 numPoints;
    rt2dPathNode *node;
    RwInt32 i;

    lbl_805E1950.pixelSizeSq = fn_80474168();
    if (path->numPoints == -1) {
        node = fn_8047DFAC(path->points);
    } else {
        node = path->points;
    }
    numPoints = path->numPoints;
    if (numPoints == -1) {
        numPoints = fn_8047E03C(path->points);
    }
    for (i = 0; i < numPoints; i++, node++) {
        if (node->type == rt2dCURVETO) {
            RwV2d bezier[4];

            bezier[0] = node[-1].pos;
            bezier[1] = node[0].pos;
            bezier[2] = node[1].pos;
            bezier[3] = node[2].pos;
            fn_804754F4(flat, bezier, lbl_805E1950.field_0x330);
            i += 2;
            node += 2;
        } else if (node->type == rt2dLINETO) {
            PATHADDLINE(flat, node->pos.x, node->pos.y);
        } else {
            *(rt2dPathNode *)fn_8047DDBC(flat->points, rwMEMHINTDUR_EVENT | RT2DPATHMEMID) = *node;
        }
    }
    if (path->closed) {
        fn_80475260(flat);
    }
    return flat;
}

/* Drops the subpaths that hold a single point, freeing them, and returns the rest */
Rt2dPath *fn_80475B4C(Rt2dPath *path) {
    if (path != NULL) {
        path->next = fn_80475B4C(path->next);
        if (fn_8047E03C(path->points) == 1) {
            path = fn_80474BEC(path);
        }
    }
    return path;
}

RwBool fn_80475E2C(Rt2dPath *path) {
    RwBool result = path != NULL;

    if (result) {
        Rt2dPath *next = fn_80474BEC(path);

        if (next != NULL) {
            fn_80475E2C(next);
        }
    }
    return result;
}

void fn_804760C8(Rt2dPath *path) {
    if (path->numPoints == -1) {
        fn_8047DFC8(path->points);
    } else if (path->points != NULL) {
        RwFree(path->points);
    }
    path->points = NULL;
    path->numPoints = 0;
    if (path->next != NULL) {
        fn_804760C8(path->next);
        fn_80475E2C(path->next);
        path->next = NULL;
    }
}

Rt2dPath *fn_80476490(void) {
    Rt2dPath *path = RwFreeListAlloc(lbl_805E1950.pathFreeList, rwMEMHINTDUR_EVENT | RT2DPATHMEMID);

    if (path != NULL) {
        path->points = NULL;
        path->numPoints = 0;
        path->closed = FALSE;
        path->flat = TRUE;
        path->next = NULL;
        path->curr = path;
        path->field_0x10 = 0.0f;
        fn_80476510(path);
    }
    return path;
}

/* Turns the points of each subpath into lists for adding to */
Rt2dPath *fn_80476510(Rt2dPath *path) {
    if (path != NULL && path->numPoints != -1) {
        RwSList *list = fn_8047DC90(sizeof(rt2dPathNode), rwMEMHINTDUR_EVENT | RT2DPATHMEMID);

        if (list != NULL) {
            void *points;
            RwInt32 numPoints;

            if (path->numPoints == -1) {
                points = fn_8047DFAC(path->points);
            } else {
                points = path->points;
            }
            numPoints = path->numPoints;
            if (numPoints == -1) {
                numPoints = fn_8047E03C(path->points);
            }
            if (numPoints != 0) {
                void *entries = fn_8047DEA0(list, numPoints, rwMEMHINTDUR_EVENT | RT2DPATHMEMID);

                if (entries != NULL) {
                    memcpy(entries, points, numPoints * sizeof(rt2dPathNode));
                }
                RwFree(points);
            }
            path->numPoints = -1;
            path->points = list;
            fn_80476510(path->next);
        } else {
            path = NULL;
        }
    }
    return path;
}

/* Turns the points of each subpath back into arrays */
Rt2dPath *fn_80476830(Rt2dPath *path) {
    if (path != NULL && path->numPoints == -1) {
        RwInt32 numPoints = fn_8047E03C(path->points);

        path->points = fn_8047E044(path->points);
        path->numPoints = numPoints;
        fn_80476830(path->next);
    }
    return path;
}

void fn_80476A0C(void) {
    fn_80475E2C(lbl_805E1950.scratchPath);
    fn_8047FCDC(lbl_805E1950.pathFreeList);
    lbl_805E1950.pathFreeList = NULL;
}

RwBool fn_80476A58(void) {
    lbl_805E1950.pathFreeList = fn_8047FC04(sizeof(Rt2dPath), lbl_805EDE50, 4, lbl_805EDE54,
                                            &lbl_805E26A8, rwMEMHINTDUR_GLOBAL | RT2DPATHMEMID);
    lbl_805E1950.scratchPath = fn_80476490();
    fn_80476510(lbl_805E1950.scratchPath);
    return TRUE;
}
