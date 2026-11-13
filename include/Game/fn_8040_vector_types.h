#ifndef GAME_FN_8040_VECTOR_TYPES_H
#define GAME_FN_8040_VECTOR_TYPES_H

#include "layout_check.h"

/* 2-, 3- and 4-float vectors used by the fn_8040xxxx vector helpers. Layouts come from their accesses;
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
    /* The original has an out-of-line copy of this constructor at 0x80008E1C in USA (GUPE8P), Japan
     * (GUPJ8P) and PAL (GUPP8P), directly after CameraModeMtnViewer's UnkSlot2 (0x80008DA8,
     * fn_80008DA8.cpp), which has its two lowest calls. It calls the copy at 48 places and stores
     * the components in place everywhere else. What follows each of the 48 calls fits a vector
     * constructed in the arguments of an inline function called as a statement of its own (the
     * expressions are inferred): the function that adds or subtracts a vector (26 places,
     * fn_8040CFF8 or fn_8040CE04 follows), the member-by-member assignment of a rotation (12), a
     * matrix copy (5, fn_8040C148 follows), a returned transform (2, fn_8040B67C is called with the
     * function's own result pointer), and wrappers of fn_8000A5D4 (2) and of fn_80010010 (1, not
     * identified). Measured with this compiler on DashPanel's hit callback and the AddVector and
     * SetPoints statements: an inline call that is the whole statement is expanded and the calls in
     * its arguments stay calls, while in a statement whose outermost call is this vector's copy
     * constructor or assignment (neither is expanded, see below) the vector constructor is
     * expanded. DashPanel's hit callback has both forms: the offset it adds to the position is
     * constructed by a call (0x800FFCCC), and the two vectors whose transformed copies initialise
     * named vectors are stored in place (0x800FFD8C, 0x800FFDBC). */
    Fn8040Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    /* The original calls the copy constructor (USA (GUPE8P) 0x800091EC) where a vector is
     * initialised from another, for example in the segment's copy constructor (0x80021DE4), and
     * the assignment (0x800091D0) where an existing vector is overwritten, for example in the
     * segment's assignment (0x8002CCF0) and when PopFooting's UpdateCharInfoPosition stores a
     * transformed result (0x8022BD28); which of the two is which is inferred from the segment's
     * members. Not every copy goes through them: fn_800C9F14 copies a position component by
     * component (0x800C9F18-0x800C9F34).
     * Both are inline functions that the original calls out of line. Their copies directly
     * follow CameraModeMtnViewer's UnkSlot1 (0x80008E2C, fn_80008DA8.cpp), which has the lowest
     * call of each in the game, and together with the copy of operator*(vector, float)
     * (0x800091A4) they are in reverse order of their first calls in it (0x80009040, 0x80008ED4,
     * 0x80008E94). This compiler puts the out-of-line copies of inline functions there: after
     * the function that first calls them.
     * Matching workaround: both bodies store the components through Set. The game is compiled
     * with an inline depth of one, where a function whose body calls an inline function is not
     * expanded at its uses (measured: at a depth of two both members are expanded at every use
     * and have no copies). With the stores written in the bodies, both are expanded at every
     * use. That the inner call is to Set is inferred: Set is the type's own inline member with
     * these stores, and it gives the seven instructions of each copy. Inline functions that
     * copy or assign a vector then stay calls as well, as in the original:
     * ContainerBehavior::GetPosition (0x800EF448), the assignment of ScatteringRing's record
     * (0x8011B0F0) and the constructor of EggBalloon's parameter block (0x80145B58). Revisit
     * when the original vector header or a name source for its members is found. */
    Fn8040Vector3(const Fn8040Vector3 &other) {
        Set(other.x, other.y, other.z);
    }
    Fn8040Vector3 &operator=(const Fn8040Vector3 &other) {
        Set(other.x, other.y, other.z);
        return *this;
    }
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

/* Three components about the x, y and z axes (placeholder name; the original type name is
 * unknown). Their unit depends on the use: radians in CharInfo::m_rotation, degrees in
 * UnkOwnerRecord_800C9D24::rotation (fn_800C9ED4 converts them), angular rates in
 * UnkMeteorsParams_8021FCE4::m_rotateSpeed. It is a type of its own because the original copies
 * these values inline, member by member, in places where it copies positions and directions
 * through the out-of-line copy members of Fn8040Vector3: for example by value to fn_8001DB08,
 * in the rotation-vector interpolation fn_8040F16C, and in the EnergyCoreTask constructor, which
 * does both one after the other. This is not universal: fn_804173A8 copies the angles it stores
 * through the assignment (0x800091D0 at 0x804173E8). */
typedef struct UnkRotation3 {
#ifdef __cplusplus
    UnkRotation3() {}
    UnkRotation3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    /* Stores the three components (name inferred), as Fn8040Vector3::Set does. */
    void Set(float x_, float y_, float z_) {
        x = x_;
        y = y_;
        z = z_;
    }
#endif
    /* 0x00 */ float x;
    /* 0x04 */ float y;
    /* 0x08 */ float z;
} UnkRotation3;
CHECK_SIZEOF(UnkRotation3, 0xC);

/* 4-float vector; the matrix helpers read a rotation quaternion (x, y, z, w) from it. */
typedef struct Fn8040Vector4 {
#ifdef __cplusplus
    Fn8040Vector4() {}
    Fn8040Vector4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    /* Stores the four components (name inferred): the arguments are evaluated before the first
     * store, as the original does where a vector is assigned in place. */
    void Set(float x_, float y_, float z_, float w_) {
        x = x_;
        y = y_;
        z = z_;
        w = w_;
    }
#endif
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
    Fn8040Matrix34 &operator=(const Fn8040Matrix34 &other);
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
 * are, and the copy constructor and the assignment copy them one after the other with the vector's
 * copy constructor and assignment; all three are called out of line (their copies at 0x80029DA4,
 * 0x80021DE4 and 0x8002CCF0), as the weapon stick does for its array of segments and the debug
 * marker of DebugMissionClearCollision for the segments it copies. */
typedef struct UnkSegment_80020A20 {
#ifdef __cplusplus
    UnkSegment_80020A20();
    UnkSegment_80020A20(const Fn8040Vector3 &start, const Fn8040Vector3 &end);
    UnkSegment_80020A20(const UnkSegment_80020A20 &other);
    UnkSegment_80020A20 &operator=(const UnkSegment_80020A20 &other);
#endif
    /* 0x00 */ Fn8040Vector3 start;
    /* 0x0C */ Fn8040Vector3 end;
} UnkSegment_80020A20;
CHECK_SIZEOF(UnkSegment_80020A20, 0x18);

#ifdef __cplusplus
extern "C" {
#endif

/* Zero vectors constructed by the static initialisers in fn_8040CC5C.cpp and fn_8040CD74.cpp;
 * fn_8040CC6C and fn_8040CE74 copy them into their argument when it is too short to normalise
 * (fn_8040CC6C returns nothing; fn_8040CE74 then returns 0.0f), and lbl_805E07A0 is read
 * throughout the game code. */
extern Fn8040Vector2 lbl_805F1358;
extern Fn8040Vector3 lbl_805E07A0;
/* A zero vector that the startup code at 0x8040EA64 stores (together with a 2D one at 0x805E07B0);
 * the GravityChangeCollision gadget passes it to the box probe factory fn_800C7C34. */
extern UnkRotation3 lbl_805E07B8;

/* Four-component globals constructed by fn_8040CAF8.cpp. lbl_805E0790 = (0, 0, 0, 1)
 * is copied through fn_8007A34C at 0x801B0574 and 0x801B06B4; use as an identity rotation is
 * inferred. lbl_805E0780 is all zero; no reader was found in the original references. */
extern Fn8040Vector4 lbl_805E0780;
extern Fn8040Vector4 lbl_805E0790;

/* Lengths of a 2D and a 3D vector (fn_8040CF50, declared with the C++ functions below), through
 * MSL <cmath>'s inline std::sqrt(float). Retail calls the out-of-line copy of std::sqrt at
 * 0x8000FEE4, which calls the copy of sqrtf at 0x8000FF04. The E3 2005 prototype (G9SJ8P) expands
 * sqrtf inside both helpers instead. */
float fn_8040CCEC(const Fn8040Vector2 *vector);
/* Two-component helpers: clear the vector, normalise it in place, and return its squared
 * length, respectively (behaviour and signatures inferred from their original bodies). */
void fn_8040CC5C(Fn8040Vector2 *vector);
void fn_8040CC6C(Fn8040Vector2 *vector);
float fn_8040CCD4(Fn8040Vector2 *vector);
/* Squared length of a 4-float vector. */
float fn_8040CA84(const Fn8040Vector4 *vector);
/* Stores sin(angle) and cos(angle) through the two pointers. The E3 2005 prototype (G9SJ8P) calls
 * sin and cos directly in fn_8040D044, so this is probably an inline helper whose out-of-line copy
 * retail calls (inferred). */
void fn_80031568(float *sine, float *cosine, float angle);
/* Normalises the vector in place and returns its previous length, or sets it to the zero vector
 * and returns 0 when it is too short to normalise (behaviour inferred). */
float fn_8040CE74(Fn8040Vector3 *vector);
/* Returns a - b by value (behaviour and by-value form inferred). Game code reaches it
 * through operator- below. */
Fn8040Vector3 fn_8040CD74(const Fn8040Vector3 *a, const Fn8040Vector3 *b);
/* Subtracts the three components of value in place (behaviour inferred). */
void fn_8040CE04(Fn8040Vector3 *vector, const Fn8040Vector3 *value);
/* vector *= scale (behaviour inferred). */
void fn_8040CE38(Fn8040Vector3 *vector, float scale);
/* Sets all three components to 0.0f (behaviour inferred). */
void fn_8040CE60(Fn8040Vector3 *vector);
/* Zeroes the vector (behaviour inferred). */
void fn_8040E970(UnkRotation3 *vector);
/* result += value (behaviour inferred). */
void fn_8040CFF8(Fn8040Vector3 *result, const Fn8040Vector3 *value);
/* Squared distance between two 3D vectors (behaviour inferred). */
float fn_8040D180(const Fn8040Vector3 *a, const Fn8040Vector3 *b);
/* The length of a - b (behaviour inferred). */
float fn_8040D1B4(const Fn8040Vector3 *a, const Fn8040Vector3 *b);
/* Matrix helpers of the segment in fn_8040B034.cpp. Their original names are unknown and their
 * descriptions are inferred from the code. */
/* Copies source to destination. */
void fn_8040C148(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source);
/* Original matrix conversion wrappers; C++ member identities are unknown. */
void fn_80016370(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source);
/* The vector counterpart of fn_80016370: assigns source to destination through the vector's
 * assignment (0x800091D0). Its C++ member identity is unknown. */
void fn_80016350(Fn8040Vector3 *destination, const Fn8040Vector3 *source);
/* Has the same instructions as fn_80016370; the destination and source arguments are inferred
 * from its callers, which copy into an existing matrix: in USA (GUPE8P) the matrix at 0(r30) is
 * passed to fn_80015DE0 at 0x800B16FC and then reloaded as this function's destination at
 * 0x800B1708. */
void fn_800163F4(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source);
/* Returns the address of the element at the given row and column. */
float *fn_8040C17C(Fn8040Matrix34 *matrix, int row, int column);
/* Returns the element at the given row and column. */
float fn_8040C168(Fn8040Matrix34 *matrix, int row, int column);
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
/* Sets the matrix to the identity. */
void fn_8040B6F8(Fn8040Matrix34 *matrix);
/* Scales the matrix's three rotation columns by x, y and z (fn_8040B790 below takes them as a
 * vector). */
void fn_8040B720(Fn8040Matrix34 *matrix, float x, float y, float z);
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
/* Inverts the matrix in place; when the computed single-precision determinant is zero the matrix
 * stays unchanged (USA (GUPE8P) test at 0x8040BD74; rounding can make a singular matrix's computed
 * determinant nonzero). */
void fn_8040BCD4(Fn8040Matrix34 *matrix);
/* Returns the inverse of source, or a copy of source when the computed single-precision
 * determinant is zero (test at 0x8040BFAC). The by-value result
 * is inferred; the code is that of a result pointer. In each region 43 of the 44 call sites pass a
 * stack slot for the result and the other one passes on the result pointer it was given (USA
 * (GUPE8P) 0x802CA564). 18 copy the slot with fn_8040C148 (17 right after the call), 19 pass it
 * straight to fn_8040B67C and never use it again, and 6 pass it to another matrix function.
 * Helicopter::OnCollision copies the result into a second stack matrix (0x802F77AC), which a
 * matrix initialised from a by-value result reproduces and a result pointer does not. */
Fn8040Matrix34 fn_8040BEF8(const Fn8040Matrix34 *source);
/* Applies the three angles of rotation in the axis order selected by order, 0 to 2 (behaviour
 * inferred). */
void fn_8001E070(Fn8040Matrix34 *matrix, const UnkRotation3 *rotation, int order);
/* Sets the matrix to the rotation by the three angles in the axis order selected by order, 0 to 2,
 * the same order semantics as fn_8001E070, which multiplies the rotation onto an existing matrix
 * (behaviour inferred from its body). */
void fn_800193B8(Fn8040Matrix34 *matrix, const UnkRotation3 *rotation, int order);
/* Matrix helpers below 0x80050000 called by game units (descriptions inferred from the callers). */
/* Returns left * right, applying right first. The by-value result is inferred; the code is that
 * of a result pointer, which the function passes on to fn_8040B8FC. In USA (GUPE8P) all 55
 * call sites pass a stack slot for the result: 49 copy it with fn_8040C148 right after the call
 * and the other 6 pass it on to another call. In ThrowContainer::Container's UpdateModel that slot
 * (0x8027B508) lies below the slot of fn_8040B034's result from the statement before
 * (0x8027B4F0), which is where this compiler puts the temporary of a later statement. A named
 * matrix filled through a result pointer is placed above that temporary instead (measured: 4
 * words differ). */
Fn8040Matrix34 fn_8001E044(const Fn8040Matrix34 *left, const Fn8040Matrix34 *right);
/* Stores the angles of the matrix's rotation in angles (fn_800494F8), and the same angles in the
 * order the rotation vector of a character info uses (fn_800493C4). */
void fn_800494F8(UnkRotation3 *angles, const Fn8040Matrix34 *matrix);
void fn_800493C4(UnkRotation3 *angles, const Fn8040Matrix34 *matrix);
/* Stores the matrix's translation (its fourth column) in translation (behaviour from its body). */
void fn_80049690(Fn8040Vector3 *translation, const Fn8040Matrix34 *matrix);
/* Returns the matrix's translation, read element by element with fn_8040C168 (behaviour from its
 * body; the by-value result is inferred: the HEALING_UNIT gadget's unit takes the destination's
 * address before the call and assigns the result to it). */
Fn8040Vector3 fn_800496F8(const Fn8040Matrix34 *matrix);
/* Matrix-to-angle sibling with a different rotation order (description inferred). */
void fn_80049290(UnkRotation3 *angles, const Fn8040Matrix34 *matrix);
/* Blends angles using the wrapped difference and relative weights (description inferred). */
float fn_8004924C(float from, float to, float a, float b);
/* Builds the matrix that rotates by angle about axis. */
void fn_800498A0(Fn8040Matrix34 *matrix, const Fn8040Vector3 *axis, float angle);
/* Builds the same rotation as fn_800498A0, storing the elements directly instead of through
 * fn_8040C17C (behaviour inferred from its body). */
void fn_80049778(Fn8040Matrix34 *matrix, const Fn8040Vector3 *axis, float angle);
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
/* Returns left + right by value (behaviour and by-value form inferred). Game code reaches it
 * through operator+ below. */
Fn8040Vector3 fn_8040CDD0(const Fn8040Vector3 *left, const Fn8040Vector3 *right);
/* Wraps an angle in radians into the range -pi to pi (behaviour inferred). */
float fn_8040E99C(float angle);
/* Converts a rotation vector to a quaternion, and a quaternion to a rotation vector (behaviour
 * inferred; the second returns the zero vector for every order except 2). */
void fn_804102F4(Fn8040Vector4 *quaternion, const UnkRotation3 *rotation, int order);
UnkRotation3 fn_80410204(const Fn8040Vector4 *quaternion, int order);
/* Interpolation of the segment in fn_8040EFA4.cpp: matrices through quaternions with a linear
 * translation, rotation vectors through quaternions, positions linearly, and angles the shorter
 * way round. */
void fn_8040EFA4(Fn8040Matrix34 *result, Fn8040Matrix34 *from, Fn8040Matrix34 *to, float t);
UnkRotation3 fn_8040F16C(const UnkRotation3 *from, const UnkRotation3 *to, int order, float t);
Fn8040Vector3 fn_8040F268(const Fn8040Vector3 *from, const Fn8040Vector3 *to, float t);
float fn_8040F314(float from, float to, float t);
/* Returns vector * factor by value (behaviour and by-value form inferred). Game code reaches it
 * through operator* below. */
Fn8040Vector3 fn_8040CDA8(const Fn8040Vector3 *vector, float factor);
/* A second out-of-line by-value wrapper of fn_8040CDA8 with the same body as operator* below; its
 * original name is unknown. */
Fn8040Vector3 fn_80093230(const Fn8040Vector3 *vector, float factor);
/* atan2(y, x) as a float. */
float fn_80016260(float y, float x);
/* Absolute value. Units built with the MSL headers use the inline definition in
 * fn_8000A950_near_zero.h; units built against include/libc call it through this declaration,
 * and the per-function units fn_8000A980.cpp, fn_8000A950.cpp and fn_8000B044.cpp, which keep
 * ordinary definitions, use it too. */
float fn_8000A980(float value);
/* Wraps an angle in radians by one turn: the fractional part (fn_8040EA40) of angle / (2 * pi)
 * times 2 * pi, plus 2 * pi when that is negative (behaviour from its body). */
float fn_8040E9D8(float angle);
/* Arc cosine as a float. */
float fn_801B1DB0(float value);
/* Functions defined in fn_8040F364.cpp (descriptions inferred; fn_8040FB88 is declared below,
 * after this block, as it returns bool, and fn_80410098 below for C++ only, as it takes a
 * reference). */
/* Rotation vector of a forward and an up direction, in order 2 (fn_8040F364, declared below for
 * C++ only, as it takes references) and order 0 (fn_8040F450), and the one of them that order
 * selects (fn_8040F5F4; any other order gives the zero vector). They return the vector by
 * value. */
UnkRotation3 fn_8040F450(const Fn8040Vector3 *forward, const Fn8040Vector3 *up);
UnkRotation3 fn_8040F5F4(const Fn8040Vector3 *forward, const Fn8040Vector3 *up, int order);
/* Pitch and yaw of a direction, by two conventions (fn_8040F664, declared below for C++ only, as
 * it takes a reference, and fn_8040F760), and the first of them with a zero roll added
 * (fn_8040F7F8). */
Fn8040Vector2 fn_8040F760(const Fn8040Vector3 *direction);
UnkRotation3 fn_8040F7F8(const Fn8040Vector3 *direction);
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
/* Returns the matrix that rotates by angle about the axis (by-value result inferred). Callers
 * copy the result into a matrix or use it directly. The by-value boundary keeps Aim's argument
 * calls out of line: USA (GUPE8P) 0x8031D320 and 0x8031D338, Japan (GUPJ8P) 0x8031D8D0,
 * and PAL (GUPP8P) 0x8031E978. A plain output-pointer call expands them. */
Fn8040Matrix34 fn_8040FD8C(const Fn8040Vector3 *axis, float angle);
/* Returns the matrix rotated by a rotation vector in the given rotation order. The by-value
 * result is inferred; the code is that of a result pointer. In USA (GUPE8P) 12 of its 13 call
 * sites pass a stack slot for the result and copy it with fn_8040C148 right after the call, for
 * example Weapon::ChaosSpear::ChaosSpear's UpdateMatrix at 0x802F0B30 and 0x802F0B3C; the
 * thirteenth (0x80237FDC) passes on the result pointer it was given. In UpdateMatrix and in
 * Katana's RequestBullet (0x80275E50) the slot lies below the slot of fn_8040B034's result from
 * the statement before, which is where this compiler puts the temporary of a later statement. A
 * named matrix filled through a result pointer is placed above that temporary instead (measured:
 * 4 words differ in each of the two functions). */
Fn8040Matrix34 fn_8040FFBC(const Fn8040Matrix34 *matrix, const UnkRotation3 *rotation, int order);
/* result = matrix rotated by the inverse of a rotation vector. */
void fn_8040FEBC(Fn8040Matrix34 *result, const Fn8040Matrix34 *matrix, const UnkRotation3 *rotation,
                 int order);
/* A segment helper outside fn_8040F364.cpp: the distance from point to the closest point of the
 * segment, which it also stores in closest unless that is null (behaviour inferred). */
float fn_8003C63C(const Fn8040Vector3 *point, const UnkSegment_80020A20 *segment,
                  Fn8040Vector3 *closest);
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

/* Copy constructor (inferred). The original has an out-of-line copy of it in
 * fn_8040B034.cpp (USA (GUPE8P) 0x8040BCB4, called at 0x8040BAF4 and 0x8040BFB4) that holds
 * fn_8040C148's loop. That file is built with automatic inlining (inferred from this copy, see
 * configure.py), where a function that calls fn_8040C148 is not expanded. */
inline Fn8040Matrix34::Fn8040Matrix34(const Fn8040Matrix34 &other) {
    fn_8040C148(this, &other);
}

/* Copy assignment (inferred). GravitySignBoard's BuildTurnMatrix uses it to store a by-value
 * rotation through a pointer. The original copies a stack matrix filled by fn_800635A4 into
 * existing storage at USA (GUPE8P) 0x80328224 and 0x80328310. These copies compile to the same
 * code as direct fn_8040C148 calls, so they do not distinguish the operator spelling. An
 * out-of-line copy at USA (GUPE8P) 0x80325384 calls fn_8040C148 and returns the destination; its
 * two callers are the inner assignment of a chained one at 0x80324FD0 and fn_8040B6F8. */
inline Fn8040Matrix34 &Fn8040Matrix34::operator=(const Fn8040Matrix34 &other) {
    fn_8040C148(this, &other);
    return *this;
}

struct RwMatrix;
/* Returns the RenderWare matrix as a Fn8040Matrix34 whose columns are its right, up, at and pos
 * vectors. The return by value is inferred: every caller in USA (GUPE8P) passes the address of a
 * stack slot as the first argument (eight in code that is not recovered yet, and the calls at
 * 0x80328218 and 0x80328304), except fn_802C8068, which passes on the address it was given and
 * so has the same code with either spelling. The const is inferred (the matrix is only read). */
extern "C" Fn8040Matrix34 fn_800635A4(const RwMatrix *matrix);

/* The matrix applied to vector, with the translation, returned by value (inferred; fn_8040B618
 * below applies only the rotation). The inline operator* below wraps it, and the operator's
 * out-of-line copy (USA (GUPE8P) 0x8007E4E8) calls it as well (0x8007E4FC), so a call of this
 * function does not show which of the two the source used. CatchStick, UpDownReel, ElecBar,
 * Balloon and StretchGrass call it by name where they assign the result through a smart
 * pointer. That spelling is inferred from the order of the code: the original loads the
 * pointer into a saved register before the call, as in CatchStick::UpdateLine (lwz r31 at
 * 0x800F4D84, bl fn_8040B67C at 0x800F4D94). At each of those statements this compiler gives
 * that order for a call by name and loads the pointer after the call when the operator is
 * used. Helicopter::UpdateModelMatrix has the second order in the original (call at
 * 0x802F7EF0, load of the target's pointer for fn_800CCE38 at 0x802F7EF4) and matches with the
 * operator. DashPanel's hit callback calls it by name in the argument of AddVector, where the
 * original calls it directly (0x800FFCDC) and the operator would be a call of its out-of-line
 * copy. The const references are inferred (the code is that of pointer parameters):
 * ElecBarSub::UpdateModelMatrix builds the vector argument at sp+0x50, below the result at
 * sp+0x5C (0x80150D3C and 0x80150D50), which a temporary passed to the call reproduces and a
 * named vector does not. */
extern "C" Fn8040Vector3 fn_8040B67C(const Fn8040Matrix34 &matrix, const Fn8040Vector3 &vector);

/* Vector arithmetic around the cores fn_8040CDD0 (add), fn_8040CD74 (subtract), fn_8040CDA8
 * (scale) and fn_8040B67C (transform by a matrix), declared to return their results by value
 * (inferred from the callers' temporaries and stack layout; the code is the same as with a
 * destination pointer; operator names inferred). The original has out-of-line copies of these
 * inline functions that keep the hidden result pointer and call the core (USA addresses): the
 * sum at 0x80093204, the difference at 0x8000AD64, the scaled vector at 0x800091A4 and the
 * transformed vector at 0x8007E4E8. It calls a copy where the operator is in the arguments of
 * an inline function that is called as a statement of its own and expanded there, and expands
 * the operator, so that the core is called, in every other statement. This compiler does the
 * same (see the three-float constructor of Fn8040Vector3). fn_8040F268 returns
 * `*from + (*to - *from) * t`: the returned sum is expanded (fn_8040CDD0 at 0x8040F2F4) and the
 * difference and the product in its arguments are calls of the copies (0x8040F2D4,
 * 0x8040F2E4). The scaled vectors that CameraModeMtnViewer's UnkSlot1 passes to AddVector are
 * calls of the copy (0x80009040, 0x8000905C). The sums that fn_804103E8 assigns to a vector
 * call the cores, because the vector's assignment is not expanded. A copy of a result into a
 * named vector goes through the out-of-line copy constructor. Callers that use a
 * result in place pass it on or bind it to a const reference, also when they pass its address to
 * a pointer parameter, so the temporary is used in place without a copy, as in the original. */
inline Fn8040Vector3 operator+(const Fn8040Vector3 &left, const Fn8040Vector3 &right) {
    return fn_8040CDD0(&left, &right);
}

inline Fn8040Vector3 operator-(const Fn8040Vector3 &left, const Fn8040Vector3 &right) {
    return fn_8040CD74(&left, &right);
}

inline Fn8040Vector3 operator*(const Fn8040Vector3 &vector, float factor) {
    return fn_8040CDA8(&vector, factor);
}

inline Fn8040Vector3 operator*(const Fn8040Matrix34 &matrix, const Fn8040Vector3 &vector) {
    return fn_8040B67C(matrix, vector);
}

/* Vector operations that return their result by value and take their vectors by const reference
 * (behaviour, by-value results and references inferred: callers pass temporaries to the
 * normalisation and the cross product and one passes one to the y rotation; the negation and the
 * other rotations follow by analogy; the code is that of pointer parameters): the vector scaled
 * to length 1 (the zero vector when its length is below 1e-6: fn_8040CE74, branch at 0x8040CE90),
 * left x right, the vector rotated about the y, x and z axes by angle radians, and
 * -vector. */
extern "C" Fn8040Vector3 fn_8040CEE8(const Fn8040Vector3 &vector);
extern "C" Fn8040Vector3 fn_8040CFB8(const Fn8040Vector3 &left, const Fn8040Vector3 &right);
extern "C" Fn8040Vector3 fn_8040D0AC(const Fn8040Vector3 &vector, float angle);
extern "C" Fn8040Vector3 fn_8040D118(const Fn8040Vector3 &vector, float angle);
extern "C" Fn8040Vector3 fn_8040D044(const Fn8040Vector3 &vector, float angle);
extern "C" Fn8040Vector3 fn_80016284(const Fn8040Vector3 &vector);
/* Returns the vector rotated by a rotation vector in the given order (behaviour inferred). The
 * reference is inferred (the code is that of a pointer parameter): Boss::Camera's
 * LamborghiniImpl::UpdatePoints passes a vector built for the call, at r1+0x90 below its named
 * locals (USA (GUPE8P) 0x8031A668-0x8031A694). A named vector is placed among them instead and
 * moves the other locals' stack offsets (measured); the instructions are otherwise the same. */
extern "C" Fn8040Vector3 fn_80410098(const Fn8040Vector3 &vector, const UnkRotation3 *rotation,
                                     int order);

/* Length (see fn_8040CCEC above) and squared length of a 3D vector. The const references are
 * inferred; the code is that of pointer parameters. Evidence: at four calls of each function the
 * original builds a difference with fn_8040CD74, copies it with the vector's copy constructor
 * (0x800091EC) and calls the function with the pointer that the copy constructor returns still in
 * r3. In USA (GUPE8P) those calls are at 0x800F6078, 0x80152240, 0x802A2B64 and 0x8031FA34 for
 * fn_8040CF50 and at 0x801547D0, 0x801549EC, 0x802A9A48 and 0x803447CC for fn_8040CF2C. A copy
 * written as the argument, Fn8040Vector3(a - b), gives that code with this compiler. A named
 * copy gives a second load of its address before the call, which is the code of the callers that
 * copy a difference into a named vector, and a - b as the argument gives no copy. */
extern "C" float fn_8040CF50(const Fn8040Vector3 &vector);
extern "C" float fn_8040CF2C(const Fn8040Vector3 &vector);
/* Dot product (behaviour inferred). The const references are inferred; the code is that of
 * pointer parameters. Evidence: many callers pass the result of another function in place. At
 * the 13 calls in USA (GUPE8P) where both arguments are such results, for example 0x8001E910,
 * 0x8003BB58 and 0x802EB1BC, the original produces the second argument first and gives it the
 * lower stack slot. Two temporaries written in the call give that order and those slots in
 * Player::Behavior::Super::fn_802EB0C4 (0x802EB1BC). */
extern "C" float fn_8040CF90(const Fn8040Vector3 &left, const Fn8040Vector3 &right);

/* The rotation vector of a forward and an up direction in order 2 (see fn_8040F5F4 above). The
 * references are inferred from VacuumContainer::Container::Update, which passes two temporaries
 * in place (the code is that of pointer parameters). */
extern "C" UnkRotation3 fn_8040F364(const Fn8040Vector3 &forward, const Fn8040Vector3 &up);

/* Pitch and yaw of a direction (see fn_8040F760 above). The reference is inferred (the code is
 * that of a pointer parameter): six of the seven calls pass a temporary built just before the
 * call, five the vector that fn_80016284 returns and one a copy of fn_8040B67C's result, and load
 * the result slot's address into r3 before the temporary's into r4 (USA (GUPE8P) calls at
 * 0x80087624, 0x800A4128, 0x801ACCEC, 0x801B07A4, 0x802F1D5C and 0x8030542C); the seventh
 * (0x800AC5AC) passes a member. */
extern "C" Fn8040Vector2 fn_8040F664(const Fn8040Vector3 &direction);

/* The matrix's rotation applied to vector, without the translation, returned by value
 * (inferred). The const references are inferred too: the ELEC_SEARCHLIGHT and HEALING_UNIT
 * gadgets build the vector in a stack slot below the result's, where this compiler places an
 * argument temporary made before the result (USA (GUPE8P) 0x80153828 and 0x8010821C), and the
 * HEALING_UNIT unit loads the destination's address before the call (0x80108218). */
extern "C" Fn8040Vector3 fn_8040B618(const Fn8040Matrix34 &matrix, const Fn8040Vector3 &vector);

/* Scales the matrix's three rotation columns by the components of scale. The reference is
 * inferred (the code is that of a pointer parameter): the WEIGHT gadget passes a temporary scale
 * vector (0x8012B0CC), at r1+0x8 below its named locals, where MWCC places temporaries. */
extern "C" void fn_8040B790(Fn8040Matrix34 *matrix, const Fn8040Vector3 &scale);

/* Translates the matrix by an offset that the matrix's rotation turns first (fn_8040B52C with the
 * offset as a vector). The reference is inferred (the code is that of a pointer parameter).
 * Observed: the ESCAPE_PLANE gadget passes the vector that fn_80016284 returns, at r1+0x14 below
 * its named locals (0x8025588C); a temporary passed by reference fits that placement, and the
 * temporary is inferred as well. */
extern "C" void fn_8040B59C(Fn8040Matrix34 *matrix, const Fn8040Vector3 &offset);

/* Returns fn_8000A950(a - b): whether the two values are close enough to count as equal
 * (behaviour inferred from its body). */
extern "C" bool fn_801C455C(float a, float b);
/* Returns whichever of value, low and high lies in between (a clamp; behaviour inferred). The
 * parameters are references: Meteor's calls with literal bounds keep each bound in its own .sdata
 * slot, as the original does. */
extern "C" const float &fn_80070460(const float &value, const float &low, const float &high);
/* Returns a when a < b, otherwise b (behaviour inferred from its body). */
extern "C" const float &fn_800704AC(const float &a, const float &b);
/* Returns a unless a < b, in which case b: the fcmpo/bgelr pair also returns a when the
 * comparison is unordered (either value a NaN) (behaviour from its body). */
extern "C" const float &fn_80070494(const float &a, const float &b);
/* Whether a - b (computed with the out-of-line copy of operator- at 0x8000AD64) is close to zero:
 * its squared length is below the float at 0x805F1730 (1e-4). */
extern "C" bool fn_8000AD08(const Fn8040Vector3 *a, const Fn8040Vector3 *b);

/* Whether the point lies between the planes through the segment's ends (behaviour inferred). */
extern "C" bool fn_8040FB88(const UnkSegment_80020A20 *segment, const Fn8040Vector3 *point);

/* The squared distance from point to the segment; when closest is not null it receives the
 * segment's point nearest to point (behaviour inferred). */
extern "C" float fn_8003C660(const Fn8040Vector3 *point, const UnkSegment_80020A20 *segment,
                             Fn8040Vector3 *closest);
/* The squared distance between two segments (behaviour inferred). closestA and closestB are
 * passed on to fn_8003BC14 with the segments' normalised directions; the ELEC_BAR gadget passes
 * null for both, and what they receive otherwise is inferred from the name only. */
extern "C" float fn_8003C4C4(const UnkSegment_80020A20 *a, const UnkSegment_80020A20 *b,
                             Fn8040Vector3 *closestA, Fn8040Vector3 *closestB);

/* Returns the matrix with the identity rotation and the given translation. The by-value result
 * and the const reference are inferred; the code is that of a result pointer and a pointer
 * parameter. In USA (GUPE8P) all 77 call sites pass a stack slot for the result, and 65 of
 * them copy that slot with fn_8040C148 right after the call into the matrix they go on to use,
 * for example Weapon::ChaosSpear::ChaosSpear's UpdateMatrix at 0x802F0AF0 and 0x802F0AFC. Where
 * the translation comes through a smart pointer, the original calls the pointer's operator->
 * out of line before the call (0x802F0AE4). This compiler does that for the arguments of an
 * inline function that is called as a statement of its own, here the matrix's copy constructor
 * or assignment; a statement that calls this function with a result pointer has the operator
 * expanded in place (measured in ElecPillar's and ElecWakka's UpdateModelMatrix). Five call sites
 * construct the vector for the call with the out-of-line copy of its three-float constructor
 * and pass on the pointer that the constructor returns, for example Boss::Diablon::Parts::Bomb's
 * fn_8031E3B0 at 0x8031E440-0x8031E44C; a constructor call written as the argument gives that
 * code. */
extern "C" Fn8040Matrix34 fn_8040B034(const Fn8040Vector3 &translation);

/* result += value (name inferred).
 * Matching workaround: AddVector is an inline wrapper of fn_8040CFF8 so that the inline calls in
 * its argument stay calls, as in the original: a smart pointer's operator-> supplying the
 * vector, the out-of-line copies of the vector operators, and the out-of-line copy of the
 * three-float constructor (0x80008E1C; 23 of its 48 calls construct a vector that is added
 * with fn_8040CFF8 right after, directly or through a function it is passed to). That happens
 * for an argument of an inline call, whereas a direct call to fn_8040CFF8 expands them in
 * place. The original inline function is unknown. Revisit when it is identified. */
inline void AddVector(Fn8040Vector3 *result, const Fn8040Vector3 &value) {
    fn_8040CFF8(result, &value);
}

/* result -= value (name inferred): a wrapper of fn_8040CE04 of the same kind.
 * Matching workaround: HeavyDog0410CtrlUnit_Move's and HeavyDog0410CtrlUnit_Mine's Update
 * subtract a scaled vector that the out-of-line copy of operator* above computes (calls at USA
 * (GUPE8P) 0x8026D2A0 and 0x8026CAA0), which happens for an argument of an inline call; in a
 * direct call of fn_8040CE04 the product is expanded to a call of fn_8040CDA8 (one word differs
 * in each function). fn_8026F0BC subtracts two such products (calls of the copy at 0x8026F17C
 * and 0x8026F198); with named products passed to fn_8040CE04 it is 0x160 bytes against the
 * original's 0x148. The original inline function is unknown. Revisit when it is identified. */
inline void SubtractVector(Fn8040Vector3 *result, const Fn8040Vector3 &value) {
    fn_8040CE04(result, &value);
}
#endif

#endif
