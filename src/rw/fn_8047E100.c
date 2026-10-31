/* RenderWare matrices: multiplying, orthonormalizing, inverting, rotating, scaling, translating and
 * transforming matrices, their errors and type flags, and the matrix module and its free list. The
 * original file name is unknown. */
#include "rw/rwcore.h"
#include "rw/rwmath.h"
#include <math.h>
#include "layout_check.h"

/* The matrix chunk ID, for its memory hints */
#define rwID_MATRIX 0xD

#define E_RW_BADPARAM 0x80000003

/* Reports an error of the core; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwID_COREPLUGIN;                                                         \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

/* The engine globals of the matrix module */
typedef struct rwMatrixGlobals {
    /* 0x00 */ RwFreeList *matrixFreeList;
    /* 0x04 */ RwUInt32 matrixOptimizations; /* the flags that let operations skip work */
    /* 0x08 */ void (*multiply)(RwMatrix *dst, const RwMatrix *matrixA, const RwMatrix *matrixB);
    /* 0x0C */ RwMatrixTolerance tolerance; /* the default one */
} rwMatrixGlobals;
CHECK_SIZEOF(rwMatrixGlobals, 0x18);

#define RWMATRIXGLOBAL(var)                                                                        \
    (RWPLUGINOFFSET(rwMatrixGlobals, lbl_805F15B4, lbl_805F1560.globalsOffset)->var)

RwFreeList lbl_805E2A58;    /* the matrix free list */
RwInt32 lbl_805EDE98 = 256; /* matrices per free list block */
RwInt32 lbl_805EDE9C = 1;   /* free list blocks to preallocate */
RwModuleInfo lbl_805F1560;  /* the matrix module */

/* The default tolerance */
const RwMatrixTolerance lbl_8051C2C0 = {0.01f, 0.01f, 0.01f};

/* The product of two matrices, matrixA applied first */
void fn_8047E100(RwMatrix *dst, const RwMatrix *matrixA, const RwMatrix *matrixB) {
    dst->right.x = matrixA->right.x * matrixB->right.x + matrixA->right.y * matrixB->up.x +
                   matrixA->right.z * matrixB->at.x;
    dst->right.y = matrixA->right.x * matrixB->right.y + matrixA->right.y * matrixB->up.y +
                   matrixA->right.z * matrixB->at.y;
    dst->right.z = matrixA->right.x * matrixB->right.z + matrixA->right.y * matrixB->up.z +
                   matrixA->right.z * matrixB->at.z;
    dst->up.x = matrixA->up.x * matrixB->right.x + matrixA->up.y * matrixB->up.x +
                matrixA->up.z * matrixB->at.x;
    dst->up.y = matrixA->up.x * matrixB->right.y + matrixA->up.y * matrixB->up.y +
                matrixA->up.z * matrixB->at.y;
    dst->up.z = matrixA->up.x * matrixB->right.z + matrixA->up.y * matrixB->up.z +
                matrixA->up.z * matrixB->at.z;
    dst->at.x = matrixA->at.x * matrixB->right.x + matrixA->at.y * matrixB->up.x +
                matrixA->at.z * matrixB->at.x;
    dst->at.y = matrixA->at.x * matrixB->right.y + matrixA->at.y * matrixB->up.y +
                matrixA->at.z * matrixB->at.y;
    dst->at.z = matrixA->at.x * matrixB->right.z + matrixA->at.y * matrixB->up.z +
                matrixA->at.z * matrixB->at.z;
    dst->pos.x = matrixA->pos.x * matrixB->right.x + matrixA->pos.y * matrixB->up.x +
                 matrixA->pos.z * matrixB->at.x + matrixB->pos.x;
    dst->pos.y = matrixA->pos.x * matrixB->right.y + matrixA->pos.y * matrixB->up.y +
                 matrixA->pos.z * matrixB->at.y + matrixB->pos.y;
    dst->pos.z = matrixA->pos.x * matrixB->right.z + matrixA->pos.y * matrixB->up.z +
                 matrixA->pos.z * matrixB->at.z + matrixB->pos.z;
}

/* Normalizes a vector with the reciprocal of its length; returns the reciprocal */
#define NORMALIZEV3D(v, recip)                                                                     \
    do {                                                                                           \
        (recip) = fn_80482394((v).x * (v).x + (v).y * (v).y + (v).z * (v).z);                      \
        (v).x *= (recip);                                                                          \
        (v).y *= (recip);                                                                          \
        (v).z *= (recip);                                                                          \
    } while (0)

/* Orthonormalizes a matrix: normalizes its axes, keeps the two closest to perpendicular (or the
 * ones that are not degenerate) and rebuilds the third from them */
RwMatrix *fn_8047E298(RwMatrix *dst, const RwMatrix *matrix) {
    RwV3d right = matrix->right;
    RwV3d up = matrix->up;
    RwV3d at = matrix->at;
    RwV3d pos = matrix->pos;
    RwReal recipRight;
    RwReal recipUp;
    RwReal recipAt;
    RwV3d *first;
    RwV3d *second;
    RwV3d *rebuilt;

    NORMALIZEV3D(right, recipRight);
    NORMALIZEV3D(up, recipUp);
    NORMALIZEV3D(at, recipAt);

    if (recipRight > 0.0f) {
        if (recipUp > 0.0f) {
            if (recipAt > 0.0f) {
                RwReal upAt = RwRealAbs(up.x * at.x + up.y * at.y + up.z * at.z);
                RwReal atRight = RwRealAbs(at.x * right.x + at.y * right.y + at.z * right.z);
                RwReal rightUp = RwRealAbs(right.x * up.x + right.y * up.y + right.z * up.z);
                if (upAt < atRight) {
                    if (upAt < rightUp) {
                        first = &up;
                        second = &at;
                        rebuilt = &right;
                    } else {
                        first = &right;
                        second = &up;
                        rebuilt = &at;
                    }
                } else if (atRight < rightUp) {
                    first = &at;
                    second = &right;
                    rebuilt = &up;
                } else {
                    first = &right;
                    second = &up;
                    rebuilt = &at;
                }
            } else {
                first = &right;
                second = &up;
                rebuilt = &at;
            }
        } else {
            first = &at;
            second = &right;
            rebuilt = &up;
        }
    } else {
        first = &up;
        second = &at;
        rebuilt = &right;
    }

    rebuilt->x = first->y * second->z - first->z * second->y;
    rebuilt->y = first->z * second->x - first->x * second->z;
    rebuilt->z = first->x * second->y - first->y * second->x;
    NORMALIZEV3D(*rebuilt, recipRight);
    second->x = rebuilt->y * first->z - rebuilt->z * first->y;
    second->y = rebuilt->z * first->x - rebuilt->x * first->z;
    second->z = rebuilt->x * first->y - rebuilt->y * first->x;
    NORMALIZEV3D(*second, recipRight);

    dst->right = right;
    dst->up = up;
    dst->at = at;
    dst->pos = pos;
    dst->flags = (dst->flags | rwMATRIXTYPEORTHONORMAL) & ~rwMATRIXINTERNALIDENTITY;
    return dst;
}

/* The determinant: the right axis dotted with the cross product of the up and at axes */
RwReal fn_8047E704(const RwMatrix *matrix) {
    RwV3d cross;

    cross.x = matrix->up.y * matrix->at.z - matrix->up.z * matrix->at.y;
    cross.y = matrix->up.z * matrix->at.x - matrix->up.x * matrix->at.z;
    cross.z = matrix->up.x * matrix->at.y - matrix->up.y * matrix->at.x;

    return cross.x * matrix->right.x + cross.y * matrix->right.y + cross.z * matrix->right.z;
}

/* The sum of the squared dot products of the matrix's axes */
RwReal fn_8047E750(const RwMatrix *matrix) {
    RwReal upAt =
        matrix->up.x * matrix->at.x + matrix->up.y * matrix->at.y + matrix->up.z * matrix->at.z;
    RwReal atRight = matrix->at.x * matrix->right.x + matrix->at.y * matrix->right.y +
                     matrix->at.z * matrix->right.z;
    RwReal rightUp = matrix->right.x * matrix->up.x + matrix->right.y * matrix->up.y +
                     matrix->right.z * matrix->up.z;

    return upAt * upAt + atRight * atRight + rightUp * rightUp;
}

/* The sum of the squared errors of the matrix's axis lengths squared from one */
RwReal fn_8047E7A8(const RwMatrix *matrix) {
    RwReal rightError = matrix->right.x * matrix->right.x + matrix->right.y * matrix->right.y +
                        matrix->right.z * matrix->right.z - 1.0f;
    RwReal upError = matrix->up.x * matrix->up.x + matrix->up.y * matrix->up.y +
                     matrix->up.z * matrix->up.z - 1.0f;
    RwReal atError = matrix->at.x * matrix->at.x + matrix->at.y * matrix->at.y +
                     matrix->at.z * matrix->at.z - 1.0f;

    return rightError * rightError + upError * upError + atError * atError;
}

void *fn_8047E828(void *instance, RwInt32 offset, RwInt32 size) {
    if (RWMATRIXGLOBAL(matrixFreeList) != NULL) {
        fn_8047FCDC(RWMATRIXGLOBAL(matrixFreeList));
        RWMATRIXGLOBAL(matrixFreeList) = NULL;
    }
    lbl_805F1560.numInstances--;
    return instance;
}

void *fn_8047E88C(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F1560.globalsOffset = offset;
    RWMATRIXGLOBAL(matrixFreeList) = fn_8047FC04(sizeof(RwMatrix), lbl_805EDE98, 4, lbl_805EDE9C,
                                                 &lbl_805E2A58, rwMEMHINTDUR_GLOBAL | rwID_MATRIX);
    if (RWMATRIXGLOBAL(matrixFreeList) == NULL) {
        instance = NULL;
    } else {
        RwMatrixTolerance tolerance = lbl_8051C2C0;

        RWMATRIXGLOBAL(matrixOptimizations) = rwMATRIXINTERNALIDENTITY;
        RWMATRIXGLOBAL(multiply) = fn_8047E100;
        fn_8047E960(&tolerance);
        lbl_805F1560.numInstances++;
    }
    return instance;
}

RwBool fn_8047E960(const RwMatrixTolerance *tolerance) {
    RWMATRIXGLOBAL(tolerance) = *tolerance;
    return TRUE;
}

/* Sets a matrix's normal, orthogonal and identity flags from its errors */
RwMatrix *fn_8047E990(RwMatrix *matrix, const RwMatrixTolerance *tolerance) {
    RwBool isNormal;
    RwBool isOrthogonal;
    RwBool isIdentity = FALSE;
    RwUInt32 flags;

    if (tolerance == NULL) {
        tolerance = &RWMATRIXGLOBAL(tolerance);
    }
    isNormal = tolerance->Normal >= fn_8047E7A8(matrix);
    isOrthogonal = tolerance->Orthogonal >= fn_8047E750(matrix);
    if (isNormal && isOrthogonal) {
        RwReal rightX = matrix->right.x - 1.0f;
        RwReal upY = matrix->up.y - 1.0f;
        RwReal atZ = matrix->at.z - 1.0f;
        /* each axis's and the position's squared deviation is summed on its own, then the four
         * sums are added; the grouping fixes the rounding the identity test sees */
        RwReal identityError =
            (rightX * rightX + matrix->right.y * matrix->right.y +
             matrix->right.z * matrix->right.z) +
            (matrix->up.x * matrix->up.x + upY * upY + matrix->up.z * matrix->up.z) +
            (matrix->at.x * matrix->at.x + matrix->at.y * matrix->at.y + atZ * atZ) +
            (matrix->pos.x * matrix->pos.x + matrix->pos.y * matrix->pos.y +
             matrix->pos.z * matrix->pos.z);

        if (tolerance->Identity >= identityError) {
            isIdentity = TRUE;
        }
    }
    flags = matrix->flags;
    flags = isNormal ? (flags | 0x1) : (flags & ~0x1);
    flags = isOrthogonal ? (flags | 0x2) : (flags & ~0x2);
    flags = isIdentity ? (flags | rwMATRIXINTERNALIDENTITY) : (flags & ~rwMATRIXINTERNALIDENTITY);
    matrix->flags = flags;
    return matrix;
}

RwMatrix *fn_8047EBA0(RwMatrix *matrix) {
    matrix->flags &= ~(rwMATRIXINTERNALIDENTITY | rwMATRIXTYPEORTHONORMAL);
    return matrix;
}

/* The product of two matrices with paired-single arithmetic, matrixA applied first; the original
 * is hand-written assembly */
/* clang-format off */
asm RwMatrix *fn_8047EBB8(register RwMatrix *dst, register const RwMatrix *matrixA,
                          register const RwMatrix *matrixB) {
    nofralloc
    stwu r1, -0x30(r1)
    stfd f31, 0x20(r1)
    psq_st f31, 0x28(r1), 0, 0
    stfd f30, 0x10(r1)
    psq_st f30, 0x18(r1), 0, 0
    psq_l f8, 0x0(r5), 0, 0
    psq_l f0, 0x0(r4), 0, 0
    psq_l f2, 0x10(r4), 0, 0
    ps_muls0 f12, f8, f0
    psq_l f9, 0x10(r5), 0, 0
    ps_muls0 f13, f8, f2
    psq_l f4, 0x20(r4), 0, 0
    psq_l f6, 0x30(r4), 0, 0
    ps_muls0 f30, f8, f4
    psq_l f11, 0x30(r5), 0, 0
    ps_madds1 f12, f9, f0, f12
    psq_l f10, 0x20(r5), 0, 0
    ps_madds1 f13, f9, f2, f13
    ps_madds0 f31, f8, f6, f11
    psq_l f1, 0x8(r4), 1, 0
    ps_madds1 f30, f9, f4, f30
    psq_l f3, 0x18(r4), 1, 0
    ps_madds0 f12, f10, f1, f12
    psq_l f5, 0x28(r4), 1, 0
    ps_madds1 f31, f9, f6, f31
    psq_l f7, 0x38(r4), 1, 0
    ps_madds0 f13, f10, f3, f13
    psq_l f8, 0x8(r5), 1, 0
    psq_st f12, 0x0(r3), 0, 0
    ps_madds0 f30, f10, f5, f30
    ps_madds0 f31, f10, f7, f31
    psq_l f11, 0x38(r5), 1, 0
    psq_st f13, 0x10(r3), 0, 0
    ps_muls0 f12, f8, f0
    psq_l f9, 0x18(r5), 1, 0
    psq_st f30, 0x20(r3), 0, 0
    ps_muls0 f13, f8, f2
    psq_l f10, 0x28(r5), 1, 0
    ps_madds1 f12, f9, f0, f12
    lwz r0, 0xc(r4)
    lwz r4, 0xc(r5)
    ps_madds0 f11, f8, f6, f11
    ps_muls0 f30, f8, f4
    psq_st f31, 0x30(r3), 0, 0
    ps_madds1 f13, f9, f2, f13
    and r4, r0, r4
    ps_madds1 f11, f9, f6, f11
    stw r4, 0xc(r3)
    ps_madds0 f12, f10, f1, f12
    ps_madds1 f8, f9, f4, f30
    ps_madds0 f13, f10, f3, f13
    psq_st f12, 0x8(r3), 1, 0
    ps_madds0 f11, f10, f7, f11
    ps_madds0 f8, f10, f5, f8
    psq_st f13, 0x18(r3), 1, 0
    psq_st f8, 0x28(r3), 1, 0
    psq_st f11, 0x38(r3), 1, 0
    psq_l f31, 0x28(r1), 0, 0
    lfd f31, 0x20(r1)
    psq_l f30, 0x18(r1), 0, 0
    lfd f30, 0x10(r1)
    addi r1, r1, 0x30
    blr
} /* clang-format on */

RwMatrix *fn_8047ECB4(RwMatrix *dst, const RwMatrix *matrix) {
    return fn_8047E298(dst, matrix);
}

/* Rotates a matrix about a unit axis, given one minus the cosine and the sine of the angle */
RwMatrix *fn_8047ECD4(RwMatrix *matrix, const RwV3d *unitAxis, RwInt32 combine,
                      RwReal oneMinusCosine, RwReal sine) {
    RwMatrix rotation;
    RwMatrix result;
    RwReal x = unitAxis->x;
    RwReal y = unitAxis->y;
    RwReal z = unitAxis->z;

    /* the products are kept apart so that none fuses into a multiply-add */
    RwReal xx;
    RwReal yy;
    RwReal zz;
    RwReal yz;
    RwReal zx;
    RwReal xy;
    RwReal sineTerm;

    xx = x * x;
    yy = y * y;
    zz = z * z;
    xx = 1.0f - xx;
    yy = 1.0f - yy;
    zz = 1.0f - zz;
    yz = y * z;
    zx = z * x;
    xy = x * y;
    xx *= oneMinusCosine;
    yy *= oneMinusCosine;
    zz *= oneMinusCosine;
    rotation.right.x = 1.0f - xx;
    rotation.up.y = 1.0f - yy;
    rotation.at.z = 1.0f - zz;
    yz *= oneMinusCosine;
    sineTerm = x * sine;
    rotation.up.z = yz + sineTerm;
    rotation.at.y = yz - sineTerm;
    zx *= oneMinusCosine;
    sineTerm = y * sine;
    rotation.right.z = zx - sineTerm;
    rotation.at.x = zx + sineTerm;
    xy *= oneMinusCosine;
    sineTerm = z * sine;
    rotation.right.y = xy + sineTerm;
    rotation.up.x = xy - sineTerm;
    rotation.flags = rwMATRIXTYPEORTHONORMAL;
    rotation.pos.x = 0.0f;
    rotation.pos.y = 0.0f;
    rotation.pos.z = 0.0f;

    switch (combine) {
    case rwCOMBINEREPLACE:
        *matrix = rotation;
        break;
    case rwCOMBINEPRECONCAT:
        fn_8047EBB8(&result, &rotation, matrix);
        *matrix = result;
        break;
    case rwCOMBINEPOSTCONCAT:
        fn_8047EBB8(&result, matrix, &rotation);
        *matrix = result;
        break;
    default:
        RWERROR((E_RW_BADPARAM, "Invalid combination type"));
        matrix = NULL;
        break;
    }
    return matrix;
}

/* Rotates a matrix about an axis by an angle in degrees */
RwMatrix *fn_8047EFC8(RwMatrix *matrix, const RwV3d *axis, RwReal angle, RwInt32 combine) {
    RwReal radians = 0.017453292f * angle;
    RwReal recipLength = fn_80482394(axis->x * axis->x + axis->y * axis->y + axis->z * axis->z);
    RwV3d unitAxis;

    unitAxis.x = axis->x * recipLength;
    unitAxis.y = axis->y * recipLength;
    unitAxis.z = axis->z * recipLength;
    fn_8047ECD4(matrix, &unitAxis, combine, 1.0f - (RwReal)cos(radians), (RwReal)sin(radians));
    return matrix;
}

/* Inverts a matrix: a copy of the identity, the transpose of an orthonormal matrix, or the
 * adjugate over the determinant otherwise */
RwMatrix *fn_8047F0B0(RwMatrix *dst, const RwMatrix *matrix) {
    if (matrix->flags & (RWMATRIXGLOBAL(matrixOptimizations) & rwMATRIXINTERNALIDENTITY)) {
        *dst = *matrix;
        return dst;
    }
    if ((matrix->flags & rwMATRIXTYPEORTHONORMAL) == rwMATRIXTYPEORTHONORMAL) {
        dst->right.x = matrix->right.x;
        dst->right.y = matrix->up.x;
        dst->right.z = matrix->at.x;
        dst->up.x = matrix->right.y;
        dst->up.y = matrix->up.y;
        dst->up.z = matrix->at.y;
        dst->at.x = matrix->right.z;
        dst->at.y = matrix->up.z;
        dst->at.z = matrix->at.z;
        dst->pos.x = -(matrix->pos.x * matrix->right.x + matrix->pos.y * matrix->right.y +
                       matrix->pos.z * matrix->right.z);
        dst->pos.y = -(matrix->pos.x * matrix->up.x + matrix->pos.y * matrix->up.y +
                       matrix->pos.z * matrix->up.z);
        dst->pos.z = -(matrix->pos.x * matrix->at.x + matrix->pos.y * matrix->at.y +
                       matrix->pos.z * matrix->at.z);
        dst->flags = rwMATRIXTYPEORTHONORMAL;
        return dst;
    }
    {
        RwSplitBits determinant;
        RwReal recipDeterminant;

        dst->right.x = matrix->up.y * matrix->at.z - matrix->up.z * matrix->at.y;
        dst->right.y = -(matrix->right.y * matrix->at.z - matrix->right.z * matrix->at.y);
        dst->right.z = matrix->right.y * matrix->up.z - matrix->right.z * matrix->up.y;
        determinant.nReal = dst->right.x * matrix->right.x + dst->right.y * matrix->up.x +
                            dst->right.z * matrix->at.x;
        if (determinant.nInt != 0) {
            recipDeterminant = 1.0f / determinant.nReal;
        } else {
            recipDeterminant = 1.0f;
        }
        dst->right.x *= recipDeterminant;
        dst->right.y *= recipDeterminant;
        dst->right.z *= recipDeterminant;
        dst->up.x = recipDeterminant * -(matrix->up.x * matrix->at.z - matrix->up.z * matrix->at.x);
        dst->up.y =
            recipDeterminant * (matrix->right.x * matrix->at.z - matrix->right.z * matrix->at.x);
        dst->up.z =
            recipDeterminant * -(matrix->right.x * matrix->up.z - matrix->right.z * matrix->up.x);
        dst->at.x = recipDeterminant * (matrix->up.x * matrix->at.y - matrix->up.y * matrix->at.x);
        dst->at.y =
            recipDeterminant * -(matrix->right.x * matrix->at.y - matrix->right.y * matrix->at.x);
        dst->at.z =
            recipDeterminant * (matrix->right.x * matrix->up.y - matrix->right.y * matrix->up.x);
        dst->pos.x =
            -(matrix->pos.x * dst->right.x + matrix->pos.y * dst->up.x + matrix->pos.z * dst->at.x);
        dst->pos.y =
            -(matrix->pos.x * dst->right.y + matrix->pos.y * dst->up.y + matrix->pos.z * dst->at.y);
        dst->pos.z =
            -(matrix->pos.x * dst->right.z + matrix->pos.y * dst->up.z + matrix->pos.z * dst->at.z);
        dst->flags = 0;
    }
    return dst;
}

RwMatrix *fn_8047F398(RwMatrix *matrix, const RwV3d *scale, RwInt32 combine) {
    switch (combine) {
    case rwCOMBINEREPLACE:
        RwMatrixSetIdentity(matrix);
        matrix->right.x = scale->x;
        matrix->up.y = scale->y;
        matrix->at.z = scale->z;
        break;
    case rwCOMBINEPRECONCAT:
        matrix->right.x *= scale->x;
        matrix->right.y *= scale->x;
        matrix->right.z *= scale->x;
        matrix->up.x *= scale->y;
        matrix->up.y *= scale->y;
        matrix->up.z *= scale->y;
        matrix->at.x *= scale->z;
        matrix->at.y *= scale->z;
        matrix->at.z *= scale->z;
        break;
    case rwCOMBINEPOSTCONCAT:
        matrix->right.x *= scale->x;
        matrix->right.y *= scale->y;
        matrix->right.z *= scale->z;
        matrix->up.x *= scale->x;
        matrix->up.y *= scale->y;
        matrix->up.z *= scale->z;
        matrix->at.x *= scale->x;
        matrix->at.y *= scale->y;
        matrix->at.z *= scale->z;
        matrix->pos.x *= scale->x;
        matrix->pos.y *= scale->y;
        matrix->pos.z *= scale->z;
        break;
    default:
        RWERROR((E_RW_BADPARAM, "Invalid combination type"));
        /* as in the original, the flags are still updated, through the NULL result */
        matrix = NULL;
        break;
    }
    matrix->flags &= ~(rwMATRIXINTERNALIDENTITY | rwMATRIXTYPEORTHONORMAL);
    return matrix;
}

RwMatrix *fn_8047F59C(RwMatrix *matrix, const RwV3d *translation, RwInt32 combine) {
    switch (combine) {
    case rwCOMBINEREPLACE:
        RwMatrixSetIdentity(matrix);
        matrix->pos.x = translation->x;
        matrix->pos.y = translation->y;
        matrix->pos.z = translation->z;
        break;
    case rwCOMBINEPRECONCAT:
        matrix->pos.x += translation->x * matrix->right.x + translation->y * matrix->up.x +
                         translation->z * matrix->at.x;
        matrix->pos.y += translation->x * matrix->right.y + translation->y * matrix->up.y +
                         translation->z * matrix->at.y;
        matrix->pos.z += translation->x * matrix->right.z + translation->y * matrix->up.z +
                         translation->z * matrix->at.z;
        break;
    case rwCOMBINEPOSTCONCAT:
        matrix->pos.x += translation->x;
        matrix->pos.y += translation->y;
        matrix->pos.z += translation->z;
        break;
    default:
        RWERROR((E_RW_BADPARAM, "Invalid combination type"));
        /* as in the original, the flags are still updated, through the NULL result */
        matrix = NULL;
        break;
    }
    matrix->flags &= ~rwMATRIXINTERNALIDENTITY;
    return matrix;
}

RwMatrix *fn_8047F72C(RwMatrix *matrix, const RwMatrix *transform, RwInt32 combine) {
    switch (combine) {
    case rwCOMBINEREPLACE:
        *matrix = *transform;
        break;
    case rwCOMBINEPRECONCAT: {
        RwMatrix result;

        fn_8047EBB8(&result, transform, matrix);
        *matrix = result;
        break;
    }
    case rwCOMBINEPOSTCONCAT: {
        RwMatrix result;

        fn_8047EBB8(&result, matrix, transform);
        *matrix = result;
        break;
    }
    default:
        RWERROR((E_RW_BADPARAM, "Invalid combination type"));
        matrix = NULL;
        break;
    }
    return matrix;
}

RwBool fn_8047F958(RwMatrix *matrix) {
    RwFreeListFree(RWMATRIXGLOBAL(matrixFreeList), matrix);
    return TRUE;
}

RwMatrix *fn_8047F994(void) {
    RwMatrix *matrix =
        RwFreeListAlloc(RWMATRIXGLOBAL(matrixFreeList), rwMEMHINTDUR_EVENT | rwID_MATRIX);

    if (matrix != NULL) {
        matrix->flags = rwMATRIXTYPEORTHONORMAL;
        RwMatrixSetIdentity(matrix);
    }
    return matrix;
}
