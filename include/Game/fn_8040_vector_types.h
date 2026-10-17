#ifndef FN_8040_VECTOR_TYPES_H
#define FN_8040_VECTOR_TYPES_H
#include "layout_check.h"

/* 2- and 3-float vectors used by the fn_8040xxxx vector helpers. Layouts come from their accesses;
 * the original type and function names are unknown. */
typedef struct Fn8040Vector2 {
    /* 0x00 */ float x;
    /* 0x04 */ float y;
} Fn8040Vector2;
CHECK_SIZEOF(Fn8040Vector2, 0x8);

typedef struct Fn8040Vector3 {
    /* 0x00 */ float x;
    /* 0x04 */ float y;
    /* 0x08 */ float z;
} Fn8040Vector3;
CHECK_SIZEOF(Fn8040Vector3, 0xC);

/* 3x4 matrix (three rows of rotation and translation) used by the fn_8040xxxx matrix helpers. */
typedef struct Fn8040Matrix34 {
    /* 0x00 */ float values[3][4];
} Fn8040Matrix34;
CHECK_SIZEOF(Fn8040Matrix34, 0x30);

#ifdef __cplusplus
extern "C" {
#endif

/* Vectors cleared by the static initialisers fn_8040CD60 and fn_8040D02C; used as fallbacks
 * when a vector is too short to normalise. lbl_805E07A0 occupies 0x10 bytes. */
extern Fn8040Vector2 lbl_805F1358;
/* float constants in .sdata2: lbl_805FAD78 (2D clear value), lbl_805FAD80 (3D clear value, also
 * the length fn_8040CE74 returns for a too-short vector) */
extern float lbl_805FAD78;
extern float lbl_805FAD80;
extern Fn8040Vector3 lbl_805E07A0;

float fn_8000FEE4(
    float value); /* not yet decompiled; returns the square root (inferred from use) */
float fn_8040CCEC(const Fn8040Vector2 *vector);
float fn_8040CF50(const Fn8040Vector3 *vector);
/* Squared length. */
float fn_8040CF2C(Fn8040Vector3 *vector);
/* destination = a - b (behaviour inferred). */
void fn_8040CD74(Fn8040Vector3 *destination, const Fn8040Vector3 *a, const Fn8040Vector3 *b);
void fn_800091D0(Fn8040Vector3 *destination, const Fn8040Vector3 *source);
void fn_800091EC(Fn8040Vector3 *destination, const Fn8040Vector3 *source);
void fn_80008E1C(Fn8040Vector3 *vector, float x, float y, float z);
/* Matrix helpers (behaviour inferred): copy, translate by matrix * offset, rotate by an angle. */
void fn_8040C148(Fn8040Matrix34 *destination, const Fn8040Matrix34 *source);
/* Sets matrix to the identity rotation with the given translation (behaviour inferred). */
void fn_8040B034(Fn8040Matrix34 *matrix, const Fn8040Vector3 *translation);
void fn_8040B59C(Fn8040Matrix34 *matrix, const Fn8040Vector3 *offset);
void fn_8040B848(Fn8040Matrix34 *matrix, float angle);
/* The same for the other two axes: fn_8040B848, fn_8040B884 and fn_8040B80C apply the rotations
 * that fn_8001E070 builds from the y, x and z angles of a rotation vector (behaviour inferred). */
void fn_8040B884(Fn8040Matrix34 *matrix, float angle);
void fn_8040B80C(Fn8040Matrix34 *matrix, float angle);
/* Applies the three angles of rotation in the axis order selected by order, 0 to 2 (behaviour
 * inferred). */
void fn_8001E070(Fn8040Matrix34 *matrix, const Fn8040Vector3 *rotation, int order);
/* Scales the matrix's three rotation columns by x, y and z (behaviour inferred). */
void fn_8040B720(Fn8040Matrix34 *matrix, float x, float y, float z);
/* identity matrix, set up at startup by fn_8040C190 */
extern Fn8040Matrix34 lbl_805E0720;

#ifdef __cplusplus
}

/* Sets matrix to the identity rotation with the given translation (name inferred). Where a smart
 * pointer's operator-> supplies the vector, the original calls that operator out of line, which
 * happens for an argument of an inline call such as this wrapper. */
inline void MakeTranslationMatrix(Fn8040Matrix34 *matrix, const Fn8040Vector3 &translation) {
    fn_8040B034(matrix, &translation);
}
#endif

#endif
