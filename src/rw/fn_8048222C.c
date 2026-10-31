/* RenderWare vectors: lengths and normalization through square root lookup tables, and
 * transforming points and vectors by matrices. The original file name is unknown. */
#include "rw/rwcore.h"
#include <math.h>
#include "layout_check.h"

/* Each table has an entry for each of the 4096 values the top 11 mantissa bits and the lowest
 * exponent bit can take; an entry is the mantissa and the low exponent bits of the result */
#define rwSQRTTABLEENTRIES 0x1000
#define rwSQRTTABLESIZE (rwSQRTTABLEENTRIES * sizeof(RwUInt32))

/* The engine globals of the vector module */
typedef struct rwVectorGlobals {
    /* 0x0 */ RwUInt32 *SqrtTab;
    /* 0x4 */ RwUInt32 *InvSqrtTab;
} rwVectorGlobals;
CHECK_SIZEOF(rwVectorGlobals, 0x8);

#define RWVECTORGLOBAL(var)                                                                        \
    (RWPLUGINOFFSET(rwVectorGlobals, lbl_805F15B4, lbl_805F1598.globalsOffset)->var)

RwModuleInfo lbl_805F1598; /* the vector module */

/* Normalizes a vector, returning its length; a zero length reports an error */
RwReal fn_8048222C(RwV3d *out, const RwV3d *in) {
    RwReal lengthSq = in->x * in->x + in->y * in->y + in->z * in->z;
    RwReal length = fn_80482340(lengthSq);
    RwReal recip = fn_80482394(lengthSq);

    out->x = in->x * recip;
    out->y = in->y * recip;
    out->z = in->z * recip;
    if (length <= 0.0f) {
        RWERRORCORE((E_RW_ZEROLENGTH));
    }
    return length;
}

RwReal fn_80482300(const RwV3d *in) {
    return fn_80482340(in->x * in->x + in->y * in->y + in->z * in->z);
}

/* The square root, looked up from the exponent and the top mantissa bits */
RwReal fn_80482340(RwReal x) {
    RwSplitBits value;

    value.nReal = x;
    if (value.nUInt != 0) {
        value.nUInt += 0x800;
        value.nUInt = ((value.nUInt >> 1) & 0x3FC00000) +
                      RWVECTORGLOBAL(SqrtTab)[(value.nUInt >> 12) & 0xFFF];
    }
    return value.nReal;
}

/* The reciprocal square root, looked up likewise */
RwReal fn_80482394(RwReal x) {
    RwSplitBits value;

    value.nReal = x;
    if (value.nUInt != 0) {
        value.nUInt += 0x800;
        value.nUInt = ((~value.nUInt >> 1) & 0x3FC00000) +
                      RWVECTORGLOBAL(InvSqrtTab)[(value.nUInt >> 12) & 0xFFF];
    }
    return value.nReal;
}

RwReal fn_804823F0(const RwV2d *v) {
    return fn_80482340(v->x * v->x + v->y * v->y);
}

/* Transforms a point by a matrix with paired-single arithmetic; the original is hand-written
 * assembly */
/* clang-format off */
asm RwV3d *fn_80482458(register RwV3d *out, register const RwV3d *point,
                       register const RwMatrix *matrix) {
    nofralloc
    psq_l f8, 0x0(r4), 0, 0
    psq_l f0, 0x0(r5), 0, 0
    psq_l f6, 0x30(r5), 0, 0
    psq_l f1, 0x8(r5), 1, 0
    psq_l f7, 0x38(r5), 1, 0
    ps_madds0 f10, f0, f8, f6
    psq_l f2, 0x10(r5), 0, 0
    ps_madds0 f11, f1, f8, f7
    psq_l f3, 0x18(r5), 1, 0
    ps_madds1 f10, f2, f8, f10
    psq_l f9, 0x8(r4), 1, 0
    psq_l f4, 0x20(r5), 0, 0
    ps_madds1 f11, f3, f8, f11
    ps_madds0 f10, f4, f9, f10
    psq_l f5, 0x28(r5), 1, 0
    ps_madds0 f11, f5, f9, f11
    psq_st f10, 0x0(r3), 0, 0
    psq_st f11, 0x8(r3), 1, 0
    blr
}
/* clang-format on */

/* Transforms points by a matrix with paired-single arithmetic; the original is hand-written
 * assembly */
/* clang-format off */
asm RwV3d *fn_804824A4(register RwV3d *pointsOut, register const RwV3d *pointsIn,
                       register RwInt32 numPoints, register const RwMatrix *matrix) {
    nofralloc
    mtctr r5
    psq_l f0, 0x0(r6), 0, 0
    psq_l f1, 0x8(r6), 1, 0
    subi r5, r3, 0x4
    psq_l f6, 0x30(r6), 0, 0
    psq_l f7, 0x38(r6), 1, 0
    psq_l f8, 0x0(r4), 0, 0
    psq_lu f9, 0x8(r4), 1, 0
    psq_l f2, 0x10(r6), 0, 0
    psq_l f3, 0x18(r6), 1, 0
    psq_l f4, 0x20(r6), 0, 0
    psq_l f5, 0x28(r6), 1, 0
loop:
    ps_madds0 f10, f0, f8, f6
    ps_madds0 f11, f1, f8, f7
    ps_madds1 f10, f2, f8, f10
    ps_madds1 f11, f3, f8, f11
    psq_lu f8, 0x4(r4), 0, 0
    ps_madds0 f10, f4, f9, f10
    ps_madds0 f11, f5, f9, f11
    psq_lu f9, 0x8(r4), 1, 0
    psq_stu f10, 0x4(r5), 0, 0
    psq_stu f11, 0x8(r5), 1, 0
    bdnz loop
    blr
}
/* clang-format on */

/* Transforms a vector by a matrix, without its translation, with paired-single arithmetic; the
 * original is hand-written assembly */
/* clang-format off */
asm RwV3d *fn_80482504(register RwV3d *out, register const RwV3d *vector,
                       register const RwMatrix *matrix) {
    nofralloc
    psq_l f6, 0x0(r4), 0, 0
    psq_l f0, 0x0(r5), 0, 0
    psq_l f1, 0x8(r5), 1, 0
    ps_muls0 f10, f0, f6
    psq_l f2, 0x10(r5), 0, 0
    ps_muls0 f11, f1, f6
    psq_l f3, 0x18(r5), 1, 0
    psq_l f7, 0x8(r4), 1, 0
    ps_madds1 f10, f2, f6, f10
    psq_l f4, 0x20(r5), 0, 0
    ps_madds1 f11, f3, f6, f11
    psq_l f5, 0x28(r5), 1, 0
    ps_madds0 f10, f4, f7, f10
    ps_madds0 f11, f5, f7, f11
    psq_st f10, 0x0(r3), 0, 0
    psq_st f11, 0x8(r3), 1, 0
    blr
}
/* clang-format on */

void *fn_80482548(void *instance, RwInt32 offset, RwInt32 size) {
    if (RWVECTORGLOBAL(InvSqrtTab) != NULL) {
        RwFree(RWVECTORGLOBAL(InvSqrtTab));
        RWVECTORGLOBAL(InvSqrtTab) = NULL;
    }
    if (RWVECTORGLOBAL(SqrtTab) != NULL) {
        RwFree(RWVECTORGLOBAL(SqrtTab));
        RWVECTORGLOBAL(SqrtTab) = NULL;
    }
    lbl_805F1598.numInstances--;
    return instance;
}

/* The square root as the original's inlined C library sqrtf computes it here: two Newton steps
 * from the reciprocal square root estimate, and NaN for a negative value (inferred) */
static inline float VectorSqrtf(float x) {
    static const double _half = 0.5;
    static const double _three = 3.0;

    if (x > 0.0f) {
        double guess = __frsqrte((double)x);

        guess = _half * guess * (_three - guess * guess * x);
        guess = _half * guess * (_three - guess * guess * x);
        return (float)(x * guess);
    } else if (x < 0.0) {
        return NAN;
    }
    return x;
}

/* Builds the square root table: for each value of the exponent's lowest bit and the top
 * mantissa bits, the bits of the root, less the exponent the lookup adds back (inferred) */
static inline RwBool VectorSqrtTableCreate(void) {
    RwUInt32 *table = RwMalloc(rwSQRTTABLESIZE, rwMEMHINTDUR_GLOBAL | rwID_VECTORMODULE);
    RwUInt32 *entry;
    RwSplitBits value;
    RwInt32 i;

    if (table == NULL) {
        RWERRORCORE((E_RW_NOMEM, rwSQRTTABLESIZE));
        return FALSE;
    }

    /* [1, 2) goes in the second half and [2, 4) in the first */
    value.nReal = 1.0f;
    entry = table + rwSQRTTABLEENTRIES / 2;
    for (i = 0; i < rwSQRTTABLEENTRIES / 2; i++) {
        RwSplitBits root;
        RwSplitBits bits;

        root.nReal = VectorSqrtf(value.nReal);
        bits.nUInt = root.nUInt - 0x1FC00000;
        *entry++ = bits.nUInt;
        value.nUInt += 0x1000;
    }
    entry = table;
    for (i = 0; i < rwSQRTTABLEENTRIES / 2; i++) {
        RwSplitBits root;
        RwSplitBits bits;

        root.nReal = VectorSqrtf(value.nReal);
        bits.nUInt = root.nUInt - 0x20000000;
        *entry++ = bits.nUInt;
        value.nUInt += 0x1000;
    }
    RWVECTORGLOBAL(SqrtTab) = table;
    return TRUE;
}

/* Builds the reciprocal square root table likewise (inferred) */
static inline RwBool VectorInvSqrtTableCreate(void) {
    RwUInt32 *table = RwMalloc(rwSQRTTABLESIZE, rwMEMHINTDUR_GLOBAL | rwID_VECTORMODULE);
    RwUInt32 *entry;
    RwSplitBits value;
    RwInt32 i;

    if (table == NULL) {
        RWERRORCORE((E_RW_NOMEM, rwSQRTTABLESIZE));
        return FALSE;
    }

    value.nReal = 1.0f;
    entry = table + rwSQRTTABLEENTRIES / 2;
    for (i = 0; i < rwSQRTTABLEENTRIES / 2; i++) {
        RwSplitBits recip;
        RwSplitBits bits;

        recip.nReal = 1.0f / VectorSqrtf(value.nReal);
        bits.nUInt = recip.nUInt - 0x20000000;
        *entry++ = bits.nUInt;
        value.nUInt += 0x1000;
    }
    entry = table;
    for (i = 0; i < rwSQRTTABLEENTRIES / 2; i++) {
        RwSplitBits recip;
        RwSplitBits bits;

        recip.nReal = 1.0f / VectorSqrtf(value.nReal);
        bits.nUInt = recip.nUInt - 0x1FC00000;
        *entry++ = bits.nUInt;
        value.nUInt += 0x1000;
    }
    RWVECTORGLOBAL(InvSqrtTab) = table;
    return TRUE;
}

void *fn_804825EC(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F1598.globalsOffset = offset;
    if (!VectorSqrtTableCreate()) {
        return NULL;
    }
    if (!VectorInvSqrtTableCreate()) {
        return NULL;
    }
    lbl_805F1598.numInstances++;
    return instance;
}
