#ifndef FN_8040_VECTOR_TYPES_H
#define FN_8040_VECTOR_TYPES_H

typedef struct Fn8040Vector2 {
    /* 0x00 */ float x;
    /* 0x04 */ float y;
} Fn8040Vector2;

typedef struct Fn8040Vector3 {
    /* 0x00 */ float x;
    /* 0x04 */ float y;
    /* 0x08 */ float z;
} Fn8040Vector3;

#ifdef __cplusplus
extern "C" {
#endif

/* Vectors cleared by the static initialisers fn_8040CD60 and fn_8040D02C; used as fallbacks
 * when a vector is too short to normalise. lbl_805E07A0 occupies 0x10 bytes. */
extern Fn8040Vector2 lbl_805F1358;
/* 0.0f in .sdata2, shared by the 2D vector clear functions */
extern float lbl_805FAD78;
extern Fn8040Vector3 lbl_805E07A0;

float fn_8040CCEC(const Fn8040Vector2 *vector);
float fn_8040CF50(const Fn8040Vector3 *vector);
void fn_800091D0(Fn8040Vector3 *destination, const Fn8040Vector3 *source);
void fn_800091EC(Fn8040Vector3 *destination, const Fn8040Vector3 *source);
void fn_80008E1C(Fn8040Vector3 *vector, float x, float y, float z);

#ifdef __cplusplus
}
#endif

#endif
