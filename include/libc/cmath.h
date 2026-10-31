#ifndef _DOLPHIN_CMATH_H_
#define _DOLPHIN_CMATH_H_

/* Single-precision functions of the C math library, shared by <math.h> and dolphin/types.h. */
#ifdef __cplusplus
extern "C" {
#endif

float sinf(float x);
float cosf(float x);
float tanf(float x);
float acosf(float x);
float atan2f(float y, float x);
float powf(float base, float exponent);

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_CMATH_H_
