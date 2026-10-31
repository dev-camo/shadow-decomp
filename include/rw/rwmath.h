#ifndef RW_RWMATH_H
#define RW_RWMATH_H

#include "rw/rwcore.h"
#include "layout_check.h"

/* RenderWare's single precision maths helpers */

/* The bits of a float, as the C library (fdlibm) reads and writes them */
typedef union RwRealShape {
    RwReal value;
    RwInt32 word;
} RwRealShape;
CHECK_SIZEOF(RwRealShape, 0x4);

#define GET_FLOAT_WORD(i, d)                                                                       \
    do {                                                                                           \
        RwRealShape gf_u;                                                                          \
                                                                                                   \
        gf_u.value = (d);                                                                          \
        (i) = gf_u.word;                                                                           \
    } while (0)

#define SET_FLOAT_WORD(d, i)                                                                       \
    do {                                                                                           \
        RwRealShape sf_u;                                                                          \
                                                                                                   \
        sf_u.word = (i);                                                                           \
        (d) = sf_u.value;                                                                          \
    } while (0)

/* The absolute value of a real; evaluates its argument twice */
#define RwRealAbs(a) ((a) >= 0.0f ? (a) : -(a))

/* acos(x): the single precision acos of fdlibm, adapted from newlib's ef_acos.c, which carries
 * these notices:
 *
 * ef_acos.c -- float version of e_acos.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 *
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* Sets result to acos(x); a macro, so that its temporaries are the caller's */
#define RwACosMacro(result, x)                                                                     \
    do {                                                                                           \
        RwReal x_ = (x);                                                                           \
        RwInt32 hx_;                                                                               \
        RwInt32 ix_;                                                                               \
        RwReal z_;                                                                                 \
        RwReal p_;                                                                                 \
        RwReal q_;                                                                                 \
        RwReal r_;                                                                                 \
        RwReal s_;                                                                                 \
        RwReal w_;                                                                                 \
                                                                                                   \
        GET_FLOAT_WORD(hx_, x_);                                                                   \
        ix_ = hx_ & 0x7FFFFFFF;                                                                    \
        if (ix_ >= 0x3F800000) {                                                                   \
            /* |x| >= 1 */                                                                         \
            if (hx_ > 0) {                                                                         \
                (result) = 0.0f;                                                                   \
            } else {                                                                               \
                (result) = 3.1415927f;                                                             \
            }                                                                                      \
        } else if (ix_ < 0x3F000000) {                                                             \
            /* |x| < 0.5 */                                                                        \
            if (ix_ <= 0x23000000) {                                                               \
                (result) = 1.5707964f;                                                             \
            } else {                                                                               \
                z_ = x_ * x_;                                                                      \
                p_ = z_ * (0.16666667f +                                                           \
                           z_ * (-0.32556581f +                                                    \
                                 z_ * (0.20121253f +                                               \
                                       z_ * (-0.040055536f +                                       \
                                             z_ * (0.000791535f + z_ * 0.00003479331f)))));        \
                q_ = 1.0f + z_ * (-2.403395f +                                                     \
                                  z_ * (2.0209458f + z_ * (-0.688284f + z_ * 0.077038154f)));      \
                r_ = p_ / q_;                                                                      \
                (result) = 1.5707963f - (x_ - (0.000000075497894f - x_ * r_));                     \
            }                                                                                      \
        } else if (hx_ < 0) {                                                                      \
            /* x < -0.5 */                                                                         \
            z_ = (1.0f + x_) * 0.5f;                                                               \
            s_ = fn_80482340(z_);                                                                  \
            p_ = z_ *                                                                              \
                 (0.16666667f +                                                                    \
                  z_ * (-0.32556581f +                                                             \
                        z_ * (0.20121253f +                                                        \
                              z_ * (-0.040055536f + z_ * (0.000791535f + z_ * 0.00003479331f))))); \
            q_ = 1.0f +                                                                            \
                 z_ * (-2.403395f + z_ * (2.0209458f + z_ * (-0.688284f + z_ * 0.077038154f)));    \
            r_ = p_ / q_;                                                                          \
            w_ = r_ * s_ - 0.000000075497894f;                                                     \
            (result) = 3.1415925f - 2.0f * (s_ + w_);                                              \
        } else {                                                                                   \
            /* x > 0.5 */                                                                          \
            RwReal df_;                                                                            \
            RwReal c_;                                                                             \
            RwInt32 idf_;                                                                          \
                                                                                                   \
            z_ = (1.0f - x_) * 0.5f;                                                               \
            s_ = fn_80482340(z_);                                                                  \
            df_ = s_;                                                                              \
            GET_FLOAT_WORD(idf_, df_);                                                             \
            SET_FLOAT_WORD(df_, idf_ & 0xFFFFF000);                                                \
            c_ = (z_ - df_ * df_) / (s_ + df_);                                                    \
            p_ = z_ *                                                                              \
                 (0.16666667f +                                                                    \
                  z_ * (-0.32556581f +                                                             \
                        z_ * (0.20121253f +                                                        \
                              z_ * (-0.040055536f + z_ * (0.000791535f + z_ * 0.00003479331f))))); \
            q_ = 1.0f +                                                                            \
                 z_ * (-2.403395f + z_ * (2.0209458f + z_ * (-0.688284f + z_ * 0.077038154f)));    \
            r_ = p_ / q_;                                                                          \
            w_ = r_ * s_ + c_;                                                                     \
            (result) = 2.0f * (df_ + w_);                                                          \
        }                                                                                          \
    } while (0)

#endif
