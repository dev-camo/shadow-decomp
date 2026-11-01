#ifndef GAME_FN_8040_VECTOR_TYPES_H
#define GAME_FN_8040_VECTOR_TYPES_H

#include "layout_check.h"

/* 2- and 3-float vectors used by the fn_8040xxxx vector helpers. Layouts come from their accesses;
 * the original type and function names are unknown. The original constructs its global vectors
 * at startup in .sbss/.bss, so the C++ view has component constructors, and an empty default
 * constructor for the members and locals declared without an initialiser. */
typedef struct Fn8040Vector2 {
#ifdef __cplusplus
    Fn8040Vector2() {}
    Fn8040Vector2(float x_, float y_) : x(x_), y(y_) {}
#endif
    /* 0x00 */ float x;
    /* 0x04 */ float y;
} Fn8040Vector2;
CHECK_SIZEOF(Fn8040Vector2, 0x8);

typedef struct Fn8040Vector3 {
#ifdef __cplusplus
    Fn8040Vector3() {}
    Fn8040Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    /* Stores the three components (name inferred): the arguments are evaluated before the first
     * store, as the original does where a vector is assigned in place. */
    void Set(float x_, float y_, float z_) {
        x = x_;
        y = y_;
        z = z_;
    }
#endif
    /* 0x00 */ float x;
    /* 0x04 */ float y;
    /* 0x08 */ float z;
} Fn8040Vector3;
CHECK_SIZEOF(Fn8040Vector3, 0xC);

/* 4-float vector; the matrix helpers read a rotation quaternion (x, y, z, w) from it. */
typedef struct Fn8040Vector4 {
    /* 0x00 */ float x;
    /* 0x04 */ float y;
    /* 0x08 */ float z;
    /* 0x0C */ float w;
} Fn8040Vector4;
CHECK_SIZEOF(Fn8040Vector4, 0x10);

/* 3x4 matrix (three rows of rotation and translation) used by the fn_8040xxxx matrix helpers. The
 * original builds a whole matrix from twelve element values, which it evaluates before the first
 * store, and constructs its identity and zero matrices at startup, so the C++ view has a
 * twelve-value constructor and setter (names inferred). */
typedef struct Fn8040Matrix34 {
#ifdef __cplusplus
    Fn8040Matrix34() {}
    Fn8040Matrix34(const Fn8040Matrix34 &other);
    /* Stores the elements itself: delegating to Set changes the startup code of the identity and
     * zero matrices (37 instead of 29 instructions). */
    Fn8040Matrix34(float m00, float m01, float m02, float m03, float m10, float m11, float m12,
                   float m13, float m20, float m21, float m22, float m23) {
        values[0][0] = m00;
        values[0][1] = m01;
        values[0][2] = m02;
        values[0][3] = m03;
        values[1][0] = m10;
        values[1][1] = m11;
        values[1][2] = m12;
        values[1][3] = m13;
        values[2][0] = m20;
        values[2][1] = m21;
        values[2][2] = m22;
        values[2][3] = m23;
    }

    void Set(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13,
             float m20, float m21, float m22, float m23) {
        values[0][0] = m00;
        values[0][1] = m01;
        values[0][2] = m02;
        values[0][3] = m03;
        values[1][0] = m10;
        values[1][1] = m11;
        values[1][2] = m12;
        values[1][3] = m13;
        values[2][0] = m20;
        values[2][1] = m21;
        values[2][2] = m22;
        values[2][3] = m23;
    }
#endif
    /* 0x00 */ float values[3][4];
} Fn8040Matrix34;
CHECK_SIZEOF(Fn8040Matrix34, 0x30);

/* Two points, a line segment from start to end (behaviour inferred from its constructor,
 * fn_80020A20, which copies both, and from the closest-point code in fn_8040F364.cpp; fn_8026DBB8
 * constructs one and passes it to fn_8040FC3C). The default constructor leaves both points as they
 * are and the assignment copies them one after the other; both are called out of line (their
 * copies at 0x80029DA4 and 0x8002CCF0), as the weapon stick does for its array of segments. */
typedef struct UnkSegment_80020A20 {
#ifdef __cplusplus
    UnkSegment_80020A20();
    UnkSegment_80020A20(const Fn8040Vector3 &start, const Fn8040Vector3 &end);
    UnkSegment_80020A20 &operator=(const UnkSegment_80020A20 &other);
#endif
    /* 0x00 */ Fn8040Vector3 start;
    /* 0x0C */ Fn8040Vector3 end;
} UnkSegment_80020A20;
CHECK_SIZEOF(UnkSegment_80020A20, 0x18);

#ifdef __cplusplus
extern "C" {
#endif

/* Zero vectors constructed by the static initialisers in fn_8040CD60.cpp and fn_8040D02C.cpp;
 * fn_8040CC6C/fn_8040CE74 return them for a vector too short to normalise, and lbl_805E07A0 is
 * read throughout the game code. */
extern Fn8040Vector2 lbl_805F1358;
/* Float literals in .sdata2: lbl_805FAD78 (2D clear value), lbl_805FAD80 (3D clear value, also
 * the length fn_8040CE74 returns for a too-short vector), and the 1e-6f literals lbl_805FAD7C and
 * lbl_805FAD84 of the 2D and 3D normalising functions (fn_8040CC6C and fn_8040CE74): a vector
 * whose length is below these is replaced by the zero vector.
 *
 * Matching workaround: referenced as external symbols because no split owns the .sdata2 range
 * 0x805FAD78-0x805FAD88, where the four literals sit interleaved. A literal in the source would
 * emit a unit-local .sdata2 copy. Revisit when these units are merged into their original files. */
extern float lbl_805FAD78;
extern float lbl_805FAD7C;
extern float lbl_805FAD80;
extern float lbl_805FAD84;
extern Fn8040Vector3 lbl_805E07A0;
/* A zero vector that the startup code at 0x8040EA64 stores (together with a 2D one at 0x805E07B0);
 * the GravityChangeCollision gadget passes it to the box probe factory fn_800C7C34. */
extern Fn8040Vector3 lbl_805E07B8;

/* Lengths of a 2D and a 3D vector, through MSL <cmath>'s inline std::sqrt(float). Retail calls the
 * out-of-line copy of std::sqrt at 0x8000FEE4, which calls the copy of sqrtf at 0x8000FF04. The E3
 * 2005 prototype (G9SJ8P) expands sqrtf inside both helpers instead. */
float fn_8040CCEC(const Fn8040Vector2 *vector);
float fn_8040CF50(const Fn8040Vector3 *vector);
/* Squared length. */
float fn_8040CF2C(Fn8040Vector3 *vector);
/* Squared length of a 4-float vector. */
float fn_8040CA84(const Fn8040Vector4 *vector);
/* Stores sin(angle) and cos(angle) through the two pointers. The E3 2005 prototype (G9SJ8P) calls
 * sin and cos directly in fn_8040D044, so this is probably an inline helper whose out-of-line copy
 * retail calls (inferred). */
void fn_80031568(float *sine, float *cosine, float angle);
/* Normalises the vector in place and returns its previous length, or sets it to the zero vector
 * and returns 0 when it is too short to normalise (behaviour inferred). */
float fn_8040CE74(Fn8040Vector3 *vector);
/* Dot product (behaviour inferred). */
float fn_8040CF90(const Fn8040Vector3 *left, const Fn8040Vector3 *right);
/* result = left x right (behaviour inferred). */
void fn_8040CFB8(Fn8040Vector3 *result, const Fn8040Vector3 *left, const Fn8040Vector3 *right);
/* destination = a - b (behaviour inferred). */
void fn_8040CD74(Fn8040Vector3 *destination, const Fn8040Vector3 *a, const Fn8040Vector3 *b);
/* Subtracts the three components of value in place (behaviour inferred). */
void fn_8040CE04(Fn8040Vector3 *vector, const Fn8040Vector3 *value);
/* vector *= scale (behaviour inferred). */
void fn_8040CE38(Fn8040Vector3 *vector, float scale);
/* Sets all three components to the 3D clear value lbl_805FAD80 (behaviour inferred). */
void fn_8040CE60(Fn8040Vector3 *vector);
/* Zeroes the vector (behaviour inferred). */
void fn_8040E970(Fn8040Vector3 *vector);
/* result += value (behaviour inferred). */
void fn_8040CFF8(Fn8040Vector3 *result, const Fn8040Vector3 *value);
/* The length of a - b (behaviour inferred). */
float fn_8040D1B4(const Fn8040Vector3 *a, const Fn8040Vector3 *b);
void fn_800091D0(Fn8040Vector3 *destination, const Fn8040Vector3 *source);
void fn_800091EC(Fn8040Vector3 *destination, const Fn8040Vector3 *source);
void fn_80008E1C(Fn8040Vector3 *vector, float x, float y, float z);
/* Matrix helpers of the segment in fn_8040B034.cpp. Their original names are unknown and their
 * descriptions are inferred from the code. */
/* Copies source to destination. */
void fn_8040C148(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source);
/* Original matrix conversion wrappers; C++ member identities are unknown. */
void fn_80016370(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source);
Fn8040Matrix34 fn_800163F4(const Fn8040Matrix34 *source);
/* Returns the address of the element at the given row and column. */
float *fn_8040C17C(Fn8040Matrix34 *matrix, int row, int column);
/* Returns the element at the given row and column. */
float fn_8040C168(Fn8040Matrix34 *matrix, int row, int column);
/* A second copy of fn_8040C148 in the original. */
void fn_8040BCB4(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source);
/* Sets matrix to the identity rotation with the given translation. */
void fn_8040B034(Fn8040Matrix34 *matrix, const Fn8040Vector3 *translation);
/* Builds a scale matrix from the components of the vector. */
void fn_8040B07C(Fn8040Matrix34 *matrix, const Fn8040Vector3 *scale);
/* Build the rotation matrices about the z, y and x axes (the angle is passed to the sine and
 * cosine helper fn_80031568). */
void fn_8040B0C0(Fn8040Matrix34 *matrix, float angle);
void fn_8040B138(Fn8040Matrix34 *matrix, float angle);
void fn_8040B1B0(Fn8040Matrix34 *matrix, float angle);
/* Builds the rotation matrix of a quaternion (x, y, z, w). */
void fn_8040B228(Fn8040Matrix34 *matrix, const Fn8040Vector4 *quaternion);
/* Builds the matrix that looks from eye towards target with the given up direction. */
void fn_8040B33C(Fn8040Matrix34 *matrix, const Fn8040Vector3 *eye, const Fn8040Vector3 *up,
                 const Fn8040Vector3 *target);
/* Stores the transpose of the source's rotation, without a translation. */
void fn_8040B4D0(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source);
/* Translates the matrix by an offset that the matrix's rotation turns first. */
void fn_8040B52C(Fn8040Matrix34 *matrix, float x, float y, float z);
void fn_8040B59C(Fn8040Matrix34 *matrix, const Fn8040Vector3 *offset);
/* result = the matrix's rotation applied to vector, without and with the translation. */
void fn_8040B618(Fn8040Vector3 *result, const Fn8040Matrix34 *matrix, const Fn8040Vector3 *vector);
void fn_8040B67C(Fn8040Vector3 *result, const Fn8040Matrix34 *matrix, const Fn8040Vector3 *vector);
/* Copy constructor or assignment operator of the matrix class (inferred), out of line (the
 * original has its weak instance at 0x80325384): calls fn_8040C148 and returns destination. */
Fn8040Matrix34 *fn_80325384(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source);
/* Sets the matrix to the identity. */
void fn_8040B6F8(Fn8040Matrix34 *matrix);
/* Scales the matrix's three rotation columns by x, y and z, or by the components of a vector. */
void fn_8040B720(Fn8040Matrix34 *matrix, float x, float y, float z);
void fn_8040B790(Fn8040Matrix34 *matrix, const Fn8040Vector3 *scale);
/* The three rotations that fn_8001E070 builds from the y, x and z angles of a rotation vector:
 * multiply the matrix by the rotation about the y, x and z axes. */
void fn_8040B848(Fn8040Matrix34 *matrix, float angle);
void fn_8040B884(Fn8040Matrix34 *matrix, float angle);
void fn_8040B80C(Fn8040Matrix34 *matrix, float angle);
/* Multiplies the matrix by the rotation of a quaternion. */
void fn_8040B8C0(Fn8040Matrix34 *matrix, const Fn8040Vector4 *quaternion);
/* result = left * right, applying right first. */
void fn_8040B8FC(Fn8040Matrix34 *result, const Fn8040Matrix34 *left, const Fn8040Matrix34 *right);
/* matrix = matrix * other, applying other first. */
void fn_8040BAC4(Fn8040Matrix34 *matrix, const Fn8040Matrix34 *other);
/* Inverts the matrix in place; a matrix without an inverse stays unchanged. */
void fn_8040BCD4(Fn8040Matrix34 *matrix);
/* Stores the inverse of source in destination, or a copy of source when it has no inverse. */
void fn_8040BEF8(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source);
/* Applies the three angles of rotation in the axis order selected by order, 0 to 2 (behaviour
 * inferred). */
void fn_8001E070(Fn8040Matrix34 *matrix, const Fn8040Vector3 *rotation, int order);
/* Matrix helpers below 0x80050000 called by game units (descriptions inferred from the callers). */
/* result = left * right, applying right first (a by-value wrapper that keeps the result pointer and
 * calls fn_8040B8FC). */
void fn_8001E044(Fn8040Matrix34 *result, const Fn8040Matrix34 *left, const Fn8040Matrix34 *right);
/* Stores the angles of the matrix's rotation in angles (fn_800494F8), and the same angles in the
 * order the rotation vector of a character info uses (fn_800493C4). */
void fn_800494F8(Fn8040Vector3 *angles, const Fn8040Matrix34 *matrix);
void fn_800493C4(Fn8040Vector3 *angles, const Fn8040Matrix34 *matrix);
/* Builds the matrix that rotates by angle about axis. */
void fn_800498A0(Fn8040Matrix34 *matrix, const Fn8040Vector3 *axis, float angle);
/* Quaternion, vector, interpolation and rotation-vector helpers used by the matrix and vector
 * math units, wherever they are defined. Their original names are unknown and the descriptions
 * are inferred from the code. A rotation vector holds the angles about the x, y and z axes; order
 * 0 to 2 selects the order in which they are applied. */
/* Stores the rotation of the matrix as a quaternion. */
void fn_8040C55C(Fn8040Vector4 *quaternion, Fn8040Matrix34 *matrix);
/* Spherical interpolation from one quaternion to another by t, 0 to 1. */
void fn_8040C3EC(Fn8040Vector4 *result, const Fn8040Vector4 *from, const Fn8040Vector4 *to,
                 float t);
/* Copies a 4-float vector (out-of-line copy constructor or assignment; inferred). */
void fn_801B1DD4(Fn8040Vector4 *destination, const Fn8040Vector4 *source);
/* Scales the vector: result = vector * factor. A by-value wrapper: it keeps the result pointer and
 * calls fn_8040CDA8. */
void fn_800091A4(Fn8040Vector3 *result, const Fn8040Vector3 *vector, float factor);
/* result = left - right (fn_8000AD64, a by-value wrapper that keeps the result pointer and calls
 * fn_8040CD74), and result = left + right (fn_8040CDD0). */
void fn_8000AD64(Fn8040Vector3 *result, const Fn8040Vector3 *left, const Fn8040Vector3 *right);
void fn_8040CDD0(Fn8040Vector3 *result, const Fn8040Vector3 *left, const Fn8040Vector3 *right);
/* Wraps an angle in radians into the range -pi to pi (behaviour inferred). */
float fn_8040E99C(float angle);
/* Converts a rotation vector to a quaternion, and a quaternion to a rotation vector (behaviour
 * inferred; the second returns the zero vector for every order except 2). */
void fn_804102F4(Fn8040Vector4 *quaternion, const Fn8040Vector3 *rotation, int order);
Fn8040Vector3 fn_80410204(const Fn8040Vector4 *quaternion, int order);
/* Interpolation of the segment in fn_8040EFA4.cpp: matrices through quaternions with a linear
 * translation, rotation vectors through quaternions, positions linearly, and angles the shorter
 * way round. */
void fn_8040EFA4(Fn8040Matrix34 *result, Fn8040Matrix34 *from, Fn8040Matrix34 *to, float t);
Fn8040Vector3 fn_8040F16C(const Fn8040Vector3 *from, const Fn8040Vector3 *to, int order, float t);
void fn_8040F268(Fn8040Vector3 *result, const Fn8040Vector3 *from, const Fn8040Vector3 *to,
                 float t);
float fn_8040F314(float from, float to, float t);
/* Vector helpers called by fn_8040F364.cpp (descriptions inferred). */
/* Rotates the vector about the y, x and z axes. */
void fn_8040D0AC(Fn8040Vector3 *result, const Fn8040Vector3 *vector, float angle);
void fn_8040D118(Fn8040Vector3 *result, const Fn8040Vector3 *vector, float angle);
void fn_8040D044(Fn8040Vector3 *result, const Fn8040Vector3 *vector, float angle);
/* result = the vector scaled to length 1. */
void fn_8040CEE8(Fn8040Vector3 *result, const Fn8040Vector3 *vector);
/* result = -vector. */
void fn_80016284(Fn8040Vector3 *result, const Fn8040Vector3 *vector);
/* result = vector * factor (fn_800091A4 is a by-value wrapper that calls this). */
void fn_8040CDA8(Fn8040Vector3 *result, const Fn8040Vector3 *vector, float factor);
/* By-value scale wrapper; original function name is unknown. */
Fn8040Vector3 fn_80093230(const Fn8040Vector3 *vector, float factor);
/* atan2(y, x) as a float. */
float fn_80016260(float y, float x);
/* Absolute value. */
float fn_8000A980(float value);
/* Arc cosine as a float. */
float fn_801B1DB0(float value);
/* Functions defined in fn_8040F364.cpp (descriptions inferred; fn_8040FB88 is declared below,
 * after this block, as it returns bool). */
/* Rotation vector of a forward and an up direction, in order 2 (fn_8040F364) and order 0
 * (fn_8040F450), and the one of them that order selects (fn_8040F5F4; any other order gives the
 * zero vector). They return the vector by value. */
Fn8040Vector3 fn_8040F364(const Fn8040Vector3 *forward, const Fn8040Vector3 *up);
Fn8040Vector3 fn_8040F450(const Fn8040Vector3 *forward, const Fn8040Vector3 *up);
Fn8040Vector3 fn_8040F5F4(const Fn8040Vector3 *forward, const Fn8040Vector3 *up, int order);
/* Pitch and yaw of a direction, by two conventions (fn_8040F664, fn_8040F760), and the first of
 * them with a zero roll added (fn_8040F7F8). */
Fn8040Vector2 fn_8040F664(const Fn8040Vector3 *direction);
Fn8040Vector2 fn_8040F760(const Fn8040Vector3 *direction);
Fn8040Vector3 fn_8040F7F8(const Fn8040Vector3 *direction);
/* Stores the pitch and the yaw of a direction (behaviour inferred from its body, which takes
 * atan2 of the x and z components and corrects the quadrant). */
void fn_8003B9E8(const Fn8040Vector3 *direction, float *pitch, float *yaw);
/* Angle between two vectors, in radians. */
float fn_8040F8FC(const Fn8040Vector3 *a, const Fn8040Vector3 *b);
/* Splits vector into its part along onto (parallel) and the remainder (perpendicular); either
 * output may be null. onto is assumed to be of unit length; it is not normalised. */
void fn_8040F990(Fn8040Vector3 *parallel, Fn8040Vector3 *perpendicular, const Fn8040Vector3 *vector,
                 const Fn8040Vector3 *onto);
/* Writes the translation column of the matrix from a vector (fn_8040FA20), and reads it into one
 * (fn_8040FAA0, and fn_8040FB08 that returns it). */
void fn_8040FA20(Fn8040Matrix34 *matrix, const Fn8040Vector3 *translation);
void fn_8040FAA0(Fn8040Vector3 *translation, Fn8040Matrix34 *matrix);
Fn8040Vector3 fn_8040FB08(Fn8040Matrix34 *matrix);
/* The closest point of a segment to a point, with its fraction along the segment when requested
 * (fn_8040FC78; fn_8040FC3C without the fraction). */
Fn8040Vector3 fn_8040FC3C(const UnkSegment_80020A20 *segment, const Fn8040Vector3 *point);
Fn8040Vector3 fn_8040FC78(float *fraction, const UnkSegment_80020A20 *segment,
                          const Fn8040Vector3 *point);
/* Builds the matrix that rotates by angle about the axis. */
void fn_8040FD8C(Fn8040Matrix34 *matrix, const Fn8040Vector3 *axis, float angle);
/* result = matrix rotated by a rotation vector, and by its inverse (fn_8040FEBC). */
void fn_8040FFBC(Fn8040Matrix34 *result, const Fn8040Matrix34 *matrix,
                 const Fn8040Vector3 *rotation, int order);
void fn_8040FEBC(Fn8040Matrix34 *result, const Fn8040Matrix34 *matrix,
                 const Fn8040Vector3 *rotation, int order);
/* result = vector rotated by a rotation vector. */
void fn_80410098(Fn8040Vector3 *result, const Fn8040Vector3 *vector, const Fn8040Vector3 *rotation,
                 int order);
/* Advances a position and a velocity under a constant acceleration and a drag coefficient over
 * a duration (behaviour inferred; the segment is in fn_804103E8.cpp). */
void fn_804103E8(Fn8040Vector3 *position, Fn8040Vector3 *velocity,
                 const Fn8040Vector3 *acceleration, float drag, float duration);
/* The identity and zero matrices, built at startup (a __sinit_ function and a .ctors entry in
 * fn_8040B034.cpp). */
extern Fn8040Matrix34 lbl_805E0720; /* identity */
extern Fn8040Matrix34 lbl_805E0750; /* zero */

#ifdef __cplusplus
}

inline Fn8040Matrix34::Fn8040Matrix34(const Fn8040Matrix34 &other) {
    fn_8040C148(this, &other);
}

/* Returns whether the value is close enough to 0 to count as 0 (behaviour inferred). */
extern "C" bool fn_8000A950(float value);
/* True when |value| < 0.0001 (behaviour inferred). */
extern "C" bool fn_8000B044(float value);
/* Whether a - b (from fn_8000AD64) is close to zero: its squared length is below the float at
 * 0x805F1730 (1e-4). */
extern "C" bool fn_8000AD08(const Fn8040Vector3 *a, const Fn8040Vector3 *b);

/* Whether the point lies between the planes through the segment's ends (behaviour inferred). */
extern "C" bool fn_8040FB88(const UnkSegment_80020A20 *segment, const Fn8040Vector3 *point);

/* Matching workaround: MakeTranslationMatrix and AddVector are inline wrappers so that a smart
 * pointer's operator-> supplying the vector is called out of line, as in the original; that
 * happens for an argument of an inline call, whereas a direct call to the function expands it in
 * place. The original inline functions are unknown. Revisit when they are identified.
 *
 * Sets matrix to the identity rotation with the given translation (name inferred). */
inline void MakeTranslationMatrix(Fn8040Matrix34 *matrix, const Fn8040Vector3 &translation) {
    fn_8040B034(matrix, &translation);
}

/* result += value (name inferred): a wrapper of fn_8040CFF8 for the same reason (see above). */
inline void AddVector(Fn8040Vector3 *result, const Fn8040Vector3 &value) {
    fn_8040CFF8(result, &value);
}
#endif

#endif
