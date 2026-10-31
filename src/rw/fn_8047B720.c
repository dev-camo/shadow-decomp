/* RenderWare intersection tests of triangles with spheres and boxes. The original file name is
 * unknown. */
#include "rw/rwcore.h"

/* The bits of a real, for testing its sign */
#define REALBITS(real) (((const RwSplitBits *)&(real))->nInt)

/* Rejects a triangle whose vertex offsets from the sphere's center along one axis all lie beyond
 * the radius on the same side */
#define SPHEREAXISREJECT(a, b, c, radius)                                                          \
    do {                                                                                           \
        RwInt32 negatives_ = (REALBITS(a) >> 31) + (REALBITS(b) >> 31) + (REALBITS(c) >> 31);      \
                                                                                                   \
        if (negatives_ == 0) {                                                                     \
            RwSplitBits beyondA_;                                                                  \
            RwSplitBits beyondB_;                                                                  \
            RwSplitBits beyondC_;                                                                  \
                                                                                                   \
            beyondA_.nReal = (a) - (radius);                                                       \
            beyondB_.nReal = (b) - (radius);                                                       \
            beyondC_.nReal = (c) - (radius);                                                       \
            if ((beyondC_.nInt | (beyondA_.nInt | beyondB_.nInt)) >= 0) {                          \
                return FALSE;                                                                      \
            }                                                                                      \
        } else if (negatives_ == -3) {                                                             \
            RwSplitBits beyondA_;                                                                  \
            RwSplitBits beyondB_;                                                                  \
            RwSplitBits beyondC_;                                                                  \
                                                                                                   \
            beyondA_.nReal = (a) + (radius);                                                       \
            beyondB_.nReal = (b) + (radius);                                                       \
            beyondC_.nReal = (c) + (radius);                                                       \
            if ((beyondC_.nInt & (beyondA_.nInt & beyondB_.nInt)) < 0) {                           \
                return FALSE;                                                                      \
            }                                                                                      \
        }                                                                                          \
    } while (0)

/* The dot product of two 3D vectors */
#define V3DDOT(a, b) ((a).x * (b).x + (a).y * (b).y + (a).z * (b).z)

/* Rejects the triangle when the point of an edge closest to the sphere's center lies outside the
 * sphere, on the far side of the edge from the triangle's third vertex */
#define SPHEREEDGEREJECT(start, end, opposite, radiusSq)                                           \
    do {                                                                                           \
        RwV3d edge_;                                                                               \
        RwV3d scaled_;                                                                             \
        RwV3d closest_;                                                                            \
        RwReal t_;                                                                                 \
        RwReal distanceSq_;                                                                        \
                                                                                                   \
        edge_.y = (end).y - (start).y;                                                             \
        edge_.x = (end).x - (start).x;                                                             \
        edge_.z = (end).z - (start).z;                                                             \
        t_ = V3DDOT(edge_, start) / V3DDOT(edge_, edge_);                                          \
        scaled_.y = edge_.y * t_;                                                                  \
        scaled_.x = edge_.x * t_;                                                                  \
        scaled_.z = edge_.z * t_;                                                                  \
        closest_.y = (start).y - scaled_.y;                                                        \
        closest_.x = (start).x - scaled_.x;                                                        \
        closest_.z = (start).z - scaled_.z;                                                        \
        distanceSq_ = V3DDOT(closest_, closest_);                                                  \
        if (distanceSq_ > (radiusSq) && distanceSq_ < V3DDOT(closest_, opposite)) {                \
            return FALSE;                                                                          \
        }                                                                                          \
    } while (0)

RwBool fn_8047B720(const RwSphere *sphere, const RwV3d *v0, const RwV3d *v1, const RwV3d *v2,
                   RwV3d *normal, RwReal *distance) {
    RwV3d offset0; /* of each vertex from the sphere's center */
    RwV3d offset1;
    RwV3d offset2;
    RwV3d edge1;
    RwV3d edge2;
    RwReal lengthSq;
    RwReal recipLength;
    RwReal planeDistance;
    RwReal radiusSq;
    RwReal distanceSq0;
    RwReal distanceSq1;
    RwReal distanceSq2;

    offset0.x = v0->x - sphere->center.x;
    offset0.y = v0->y - sphere->center.y;
    offset0.z = v0->z - sphere->center.z;
    offset1.x = v1->x - sphere->center.x;
    offset1.y = v1->y - sphere->center.y;
    offset1.z = v1->z - sphere->center.z;
    offset2.x = v2->x - sphere->center.x;
    offset2.y = v2->y - sphere->center.y;
    offset2.z = v2->z - sphere->center.z;

    SPHEREAXISREJECT(offset0.x, offset1.x, offset2.x, sphere->radius);
    SPHEREAXISREJECT(offset0.y, offset1.y, offset2.y, sphere->radius);
    SPHEREAXISREJECT(offset0.z, offset1.z, offset2.z, sphere->radius);

    edge1.x = v1->x - v0->x;
    edge1.y = v1->y - v0->y;
    edge1.z = v1->z - v0->z;
    edge2.x = v2->x - v0->x;
    edge2.y = v2->y - v0->y;
    edge2.z = v2->z - v0->z;
    normal->x = edge1.y * edge2.z - edge1.z * edge2.y;
    normal->y = edge1.z * edge2.x - edge1.x * edge2.z;
    normal->z = edge1.x * edge2.y - edge1.y * edge2.x;
    lengthSq = V3DDOT(*normal, *normal);
    if (lengthSq <= 0.0f) {
        /* a degenerate triangle */
        return FALSE;
    }
    recipLength = fn_80482394(lengthSq);
    normal->x *= recipLength;
    normal->y *= recipLength;
    normal->z *= recipLength;

    planeDistance = V3DDOT(offset0, *normal);
    if (planeDistance < -sphere->radius || planeDistance > sphere->radius) {
        return FALSE;
    }
    *distance = -planeDistance;

    /* a vertex inside the sphere */
    radiusSq = sphere->radius * sphere->radius;
    distanceSq0 = V3DDOT(offset0, offset0);
    if (distanceSq0 <= radiusSq) {
        return TRUE;
    }
    distanceSq1 = V3DDOT(offset1, offset1);
    if (distanceSq1 <= radiusSq) {
        return TRUE;
    }
    distanceSq2 = V3DDOT(offset2, offset2);
    if (distanceSq2 <= radiusSq) {
        return TRUE;
    }

    /* the triangle lies beyond its vertex closest to the center */
    if (distanceSq0 < distanceSq1) {
        if (distanceSq2 < distanceSq0) {
            if (V3DDOT(offset2, offset0) > distanceSq2 && V3DDOT(offset2, offset1) > distanceSq2) {
                return FALSE;
            }
        } else if (V3DDOT(offset0, offset1) > distanceSq0 &&
                   V3DDOT(offset0, offset2) > distanceSq0) {
            return FALSE;
        }
    } else if (distanceSq2 < distanceSq1) {
        if (V3DDOT(offset2, offset0) > distanceSq2 && V3DDOT(offset2, offset1) > distanceSq2) {
            return FALSE;
        }
    } else if (V3DDOT(offset1, offset0) > distanceSq1 && V3DDOT(offset1, offset2) > distanceSq1) {
        return FALSE;
    }

    SPHEREEDGEREJECT(offset0, offset1, offset2, radiusSq);
    SPHEREEDGEREJECT(offset1, offset2, offset0, radiusSq);
    SPHEREEDGEREJECT(offset2, offset0, offset1, radiusSq);
    return TRUE;
}

/* The outcode bits of a point outside a box: beyond its sup or below its inf on an axis */
#define BBOXOUTX_SUP 0x1
#define BBOXOUTX_INF 0x2
#define BBOXOUTY_SUP 0x4
#define BBOXOUTY_INF 0x8
#define BBOXOUTZ_SUP 0x10
#define BBOXOUTZ_INF 0x20

/* The outcode of a point outside a box on one axis */
#define BBOXAXISOUTCODE(code, value, sup, inf, supBit, infBit)                                     \
    do {                                                                                           \
        if ((value) > (sup)) {                                                                     \
            (code) = (supBit);                                                                     \
        } else if ((value) < (inf)) {                                                              \
            (code) = (infBit);                                                                     \
        } else {                                                                                   \
            (code) = 0;                                                                            \
        }                                                                                          \
    } while (0)

/* Accepts the triangle when an edge crossing a face of the box crosses it inside the box's other
 * two axes; offset is the edge start's offset from the face, along the edge's delta on that axis */
#define BBOXFACECROSSING(offset, delta, delta1, sup1, inf1, delta2, sup2, inf2)                    \
    do {                                                                                           \
        RwReal cross_ = (offset) * (delta1);                                                       \
                                                                                                   \
        if ((delta) < 0.0f) {                                                                      \
            if (cross_ < (sup1) * (delta) && cross_ > (inf1) * (delta)) {                          \
                cross_ = (offset) * (delta2);                                                      \
                if (cross_ < (sup2) * (delta) && cross_ > (inf2) * (delta)) {                      \
                    return TRUE;                                                                   \
                }                                                                                  \
            }                                                                                      \
        } else if (cross_ > (sup1) * (delta) && cross_ < (inf1) * (delta)) {                       \
            cross_ = (offset) * (delta2);                                                          \
            if (cross_ > (sup2) * (delta) && cross_ < (inf2) * (delta)) {                          \
                return TRUE;                                                                       \
            }                                                                                      \
        }                                                                                          \
    } while (0)

/* Accepts the triangle when the edge from start to end, whose outcodes share no bit, crosses a
 * face of the box inside it */
#define BBOXEDGECROSSING(start, end, startCode, endCode)                                           \
    do {                                                                                           \
        if (!((startCode) & (endCode))) {                                                          \
            RwInt32 crossed_ = (startCode) ^ (endCode);                                            \
            RwV3d delta_;                                                                          \
            RwV3d fromInf_;                                                                        \
            RwV3d fromSup_;                                                                        \
                                                                                                   \
            delta_.x = (end)->x - (start)->x;                                                      \
            delta_.y = (end)->y - (start)->y;                                                      \
            delta_.z = (end)->z - (start)->z;                                                      \
            fromInf_.x = (start)->x - box->inf.x;                                                  \
            fromInf_.y = (start)->y - box->inf.y;                                                  \
            fromInf_.z = (start)->z - box->inf.z;                                                  \
            fromSup_.x = (start)->x - box->sup.x;                                                  \
            fromSup_.y = (start)->y - box->sup.y;                                                  \
            fromSup_.z = (start)->z - box->sup.z;                                                  \
            if (crossed_ & BBOXOUTX_SUP) {                                                         \
                BBOXFACECROSSING(fromSup_.x, delta_.x, delta_.y, fromSup_.y, fromInf_.y, delta_.z, \
                                 fromSup_.z, fromInf_.z);                                          \
            }                                                                                      \
            if (crossed_ & BBOXOUTX_INF) {                                                         \
                BBOXFACECROSSING(fromInf_.x, delta_.x, delta_.y, fromSup_.y, fromInf_.y, delta_.z, \
                                 fromSup_.z, fromInf_.z);                                          \
            }                                                                                      \
            if (crossed_ & BBOXOUTY_SUP) {                                                         \
                BBOXFACECROSSING(fromSup_.y, delta_.y, delta_.z, fromSup_.z, fromInf_.z, delta_.x, \
                                 fromSup_.x, fromInf_.x);                                          \
            }                                                                                      \
            if (crossed_ & BBOXOUTY_INF) {                                                         \
                BBOXFACECROSSING(fromInf_.y, delta_.y, delta_.z, fromSup_.z, fromInf_.z, delta_.x, \
                                 fromSup_.x, fromInf_.x);                                          \
            }                                                                                      \
            if (crossed_ & BBOXOUTZ_SUP) {                                                         \
                BBOXFACECROSSING(fromSup_.z, delta_.z, delta_.x, fromSup_.x, fromInf_.x, delta_.y, \
                                 fromSup_.y, fromInf_.y);                                          \
            }                                                                                      \
            if (crossed_ & BBOXOUTZ_INF) {                                                         \
                BBOXFACECROSSING(fromInf_.z, delta_.z, delta_.x, fromSup_.x, fromInf_.x, delta_.y, \
                                 fromSup_.y, fromInf_.y);                                          \
            }                                                                                      \
        }                                                                                          \
    } while (0)

RwBool fn_8047BDD0(const RwBBox *box, const RwV3d *v0, const RwV3d *v1, const RwV3d *v2) {
    RwInt32 codeY;
    RwInt32 codeX;
    RwInt32 codeZ;
    RwInt32 code0;
    RwInt32 code1;
    RwInt32 code2;
    RwV3d edge1;
    RwV3d edge2;
    RwV3d normal;
    RwV3d direction;
    RwV3d origin;
    RwV3d q;
    RwInt32 normalXBits;
    RwReal det;
    RwReal t;
    RwReal u;
    RwReal v;

    /* a vertex inside the box */
    BBOXAXISOUTCODE(codeY, v0->y, box->sup.y, box->inf.y, BBOXOUTY_SUP, BBOXOUTY_INF);
    BBOXAXISOUTCODE(codeX, v0->x, box->sup.x, box->inf.x, BBOXOUTX_SUP, BBOXOUTX_INF);
    BBOXAXISOUTCODE(codeZ, v0->z, box->sup.z, box->inf.z, BBOXOUTZ_SUP, BBOXOUTZ_INF);
    code0 = codeZ | (codeX | codeY);
    if (code0 == 0) {
        return TRUE;
    }
    BBOXAXISOUTCODE(codeY, v1->y, box->sup.y, box->inf.y, BBOXOUTY_SUP, BBOXOUTY_INF);
    BBOXAXISOUTCODE(codeX, v1->x, box->sup.x, box->inf.x, BBOXOUTX_SUP, BBOXOUTX_INF);
    BBOXAXISOUTCODE(codeZ, v1->z, box->sup.z, box->inf.z, BBOXOUTZ_SUP, BBOXOUTZ_INF);
    code1 = codeZ | (codeX | codeY);
    if (code1 == 0) {
        return TRUE;
    }
    BBOXAXISOUTCODE(codeY, v2->y, box->sup.y, box->inf.y, BBOXOUTY_SUP, BBOXOUTY_INF);
    BBOXAXISOUTCODE(codeX, v2->x, box->sup.x, box->inf.x, BBOXOUTX_SUP, BBOXOUTX_INF);
    BBOXAXISOUTCODE(codeZ, v2->z, box->sup.z, box->inf.z, BBOXOUTZ_SUP, BBOXOUTZ_INF);
    code2 = codeZ | (codeX | codeY);
    if (code2 == 0) {
        return TRUE;
    }

    /* all the vertices beyond the same face */
    if (code2 & (code0 & code1)) {
        return FALSE;
    }

    /* an edge crossing a face inside the box */
    BBOXEDGECROSSING(v0, v1, code0, code1);
    BBOXEDGECROSSING(v0, v2, code0, code2);
    BBOXEDGECROSSING(v1, v2, code1, code2);

    /* the box's diagonal facing the triangle's plane crossing the triangle */
    edge1.x = v1->x - v0->x;
    edge1.y = v1->y - v0->y;
    edge1.z = v1->z - v0->z;
    edge2.x = v2->x - v0->x;
    edge2.y = v2->y - v0->y;
    edge2.z = v2->z - v0->z;
    normal.x = edge1.y * edge2.z - edge1.z * edge2.y;
    normal.y = edge1.z * edge2.x - edge1.x * edge2.z;
    normal.z = edge1.x * edge2.y - edge1.y * edge2.x;
    direction.x = box->sup.x - box->inf.x;
    origin.x = box->inf.x - v0->x;
    normalXBits = REALBITS(normal.x);
    if ((normalXBits ^ REALBITS(normal.y)) < 0) {
        direction.y = box->inf.y - box->sup.y;
        origin.y = box->sup.y - v0->y;
    } else {
        direction.y = box->sup.y - box->inf.y;
        origin.y = box->inf.y - v0->y;
    }
    if ((normalXBits ^ REALBITS(normal.z)) < 0) {
        direction.z = box->inf.z - box->sup.z;
        origin.z = box->sup.z - v0->z;
    } else {
        direction.z = box->sup.z - box->inf.z;
        origin.z = box->inf.z - v0->z;
    }
    det = -V3DDOT(direction, normal);
    t = V3DDOT(origin, normal);
    if (det < 0.0f) {
        if (0.0f > t && t > det) {
            q.x = origin.y * direction.z - origin.z * direction.y;
            q.y = origin.z * direction.x - origin.x * direction.z;
            q.z = origin.x * direction.y - origin.y * direction.x;
            u = V3DDOT(edge2, q);
            if (0.0f > u && u > det) {
                v = -V3DDOT(edge1, q);
                if (0.0f > v && u + v > det) {
                    return TRUE;
                }
            }
        }
    } else if (0.0f < t && t < det) {
        q.x = origin.y * direction.z - origin.z * direction.y;
        q.y = origin.z * direction.x - origin.x * direction.z;
        q.z = origin.x * direction.y - origin.y * direction.x;
        u = V3DDOT(edge2, q);
        if (0.0f < u && u < det) {
            v = -V3DDOT(edge1, q);
            if (0.0f < v && u + v < det) {
                return TRUE;
            }
        }
    }
    return FALSE;
}
