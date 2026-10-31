#ifndef _MATH_H_
#define _MATH_H_

#include "libc/cmath.h"

#define M_PI 3.14159265358979323846f

/* Quiet NaN bit pattern defined by MSL (float.c); NAN reads it as a float, as MSL does. */
extern int __float_nan[];
#define NAN (*(float *)__float_nan)

/* The MSL wrappers keep their C names when C++ calls them. */
#ifdef __cplusplus
extern "C" {
#endif

double fabs(double x);
double pow(double x, double y);
double log(double x);
double log10(double x);
double sin(double x);
double cos(double x);
double atan2(double y, double x);
double fmod(double x, double y);
double ldexp(double x, int exp);
double scalbn(double x, int n);
double copysign(double x, double y);
double floor(double x);
/* Retail calls an out-of-line copy of the MSL inline floorf (floor, then frsp). */
float floorf(float x);

long __fpclassifyf(float x);
long __fpclassifyd(double x);

#ifdef __cplusplus
}
#endif

extern inline float sqrtf(float x) {
    const double _half = .5;
    const double _three = 3.0;
    volatile float y;
    if (x > 0.0f) {
        double guess = __frsqrte((double)x);                  // returns an approximation to
        guess = _half * guess * (_three - guess * guess * x); // now have 12 sig bits
        guess = _half * guess * (_three - guess * guess * x); // now have 24 sig bits
        guess = _half * guess * (_three - guess * guess * x); // now have 32 sig bits
        y = (float)(x * guess);
        return y;
    }
    return x;
}

#define fabs(x) __fabs(x)
#define fabsf(x) __fabsf(x)

#define FP_NAN 1
#define FP_INFINITE 2
#define FP_ZERO 3
#define FP_NORMAL 4
#define FP_SUBNORMAL 5

#define fpclassify(x)                                                                              \
    (sizeof(x) == sizeof(float) ? __fpclassifyf((float)(x)) : __fpclassifyd((double)(x)))
#define isfinite(x) ((fpclassify(x) > FP_INFINITE))

inline float fmodf(float x, float m) {
    float a = fabsf(m);
    float b = fabsf(x);
    if (a > b)
        return x;
    else {
        long long c = (long long)(x / m);
        return x - m * c;
    }
}

#endif
