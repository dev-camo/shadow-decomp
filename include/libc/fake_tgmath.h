#ifndef _DOLPHIN_FAKE_TGMATH_H_
#define _DOLPHIN_FAKE_TGMATH_H_

/*
 * Matching workaround: float definitions of sqrtf, sqrt, fabs, fabsf, cos and floor for the C SDK
 * units that include this header (AXVPB, ar, mtx, mtx44, reverb_hi, reverb_std). Only sqrt and cos
 * (AXVPB) and fabsf (mtx44) are called; the other definitions are unused.
 * Evidence: the unused definitions change no code, but the objects must also match in their local
 * symbol names: with only the three called functions, the compiler's local labels of all six units
 * are renumbered (for example ar.o names its .data object @12 instead of the original's @21).
 * The SDK's real math header, and which inline functions it defined, is unknown.
 * Revisit when the SDK's real math header is recovered.
 */
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

extern inline float sqrt(float x) {
    const double _half = .5;
    const double _three = 3.0;
    volatile float y;
    if (x > 0.0f) {
        double guess = __frsqrte((double)x);                  // returns an approximation to
        guess = _half * guess * (_three - guess * guess * x); // now have 12 sig bits
        guess = _half * guess * (_three - guess * guess * x); // now have 24 sig bits
        guess = _half * guess * (_three - guess * guess * x); // now have 32 sig bits
        guess = _half * guess * (_three - guess * guess * x); // fourth refinement step (sqrt only)
        y = (float)(x * guess);
        return y;
    }
    return x;
}

extern inline float fabs(float x) {
    (*(int *)&x) &= 0x7fffffff;
    return x;
}

extern inline float fabsf(float x) {
    return __fabsf(x);
}

extern inline float cos(float x) {
    return cosf(x);
}

inline float floor(float x) {
    int i = (int)x;
    float y = x - (float)i;

    if (!y || x > 8388608.0f)
        return x; // x is already an int

    // the int conversion rounds toward zero, so a negative x needs one more decrement
    if (x < 0)
        return (float)--i;
    return (float)i;
}

#endif // _DOLPHIN_FAKE_TGMATH_H_
