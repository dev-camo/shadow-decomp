#include "Game/CSD/csdBase.h"
#include "Game/CSD/Matrix.h"
#include <cmath>
#include <vector>
namespace Chao {
namespace CSD {
static std::vector<Matrix *> *s_pStack;
static Matrix s_identity;
static Matrix *s_pSource;
static Matrix *s_pTarget;
static inline void CopyMatrixValue(MatrixValue *dst, const MatrixValue *src) {
    dst->m[0] = src->m[0];
    dst->m[1] = src->m[1];
    dst->m[2] = src->m[2];
    dst->m[3] = src->m[3];
    dst->m[4] = src->m[4];
    dst->m[5] = src->m[5];
}

static inline void SetIdentity(f32 *m) {
    m[0] = 1.0f;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;
    m[4] = 1.0f;
    m[5] = 0.0f;
}
static inline void CopyLinearParts() {
    if (s_pTarget == s_pSource) {
        return;
    }
    s_pTarget->m[0] = s_pSource->m[0];
    s_pTarget->m[1] = s_pSource->m[1];
    s_pTarget->m[3] = s_pSource->m[3];
    s_pTarget->m[4] = s_pSource->m[4];
    if (s_pTarget->sub1 != NULL) {
        s_pTarget->sub1->m[0] = s_pSource->sub1->m[0];
        s_pTarget->sub1->m[1] = s_pSource->sub1->m[1];
        s_pTarget->sub1->m[3] = s_pSource->sub1->m[3];
        s_pTarget->sub1->m[4] = s_pSource->sub1->m[4];
    }
    if (s_pTarget->sub0 != NULL) {
        s_pTarget->sub0->m[0] = s_pSource->sub0->m[0];
        s_pTarget->sub0->m[1] = s_pSource->sub0->m[1];
        s_pTarget->sub0->m[3] = s_pSource->sub0->m[3];
        s_pTarget->sub0->m[4] = s_pSource->sub0->m[4];
    }
}

void InitMatrixStack() {
    s_pStack = new std::vector<Matrix *>;
    s_pStack->reserve(32);
    s_identity.sub0 = static_cast<MatrixValue *>(Alloc(sizeof(MatrixValue), __FILE__, __LINE__));
    s_identity.sub1 = static_cast<MatrixValue *>(Alloc(sizeof(MatrixValue), __FILE__, __LINE__));
    SetMatrixIdentity(&s_identity);
    s_pStack->push_back(&s_identity);
}

/* csdMatrix.cpp (original name from allocation calls): the scene matrix stack. Function,
 * variable and helper names are inferred. Definitions use reverse address order for deferred
 * inlining. The stack starts at s_identity; transformations read s_pSource and write s_pTarget.
 * CopyLinearParts preserves the translations (m[2] and m[5]) and skips identical matrices.
 * Matching workaround: keep the two allocation calls on original lines 53 and 54 so __LINE__
 * preserves their diagnostic metadata. Documentation follows initialization to retain those
 * positions; revisit if the original source layout is recovered. */

void TermMatrixStack() {
    if (s_pStack != NULL) {
        delete s_pStack;
        s_pStack = NULL;
    }
    Free(s_identity.sub0);
    Free(s_identity.sub1);
}

void PushMatrix(Matrix *matrix) {
    s_pSource = s_pStack->back();
    s_pStack->push_back(matrix);
    s_pTarget = s_pStack->back();
}

void PopMatrix() {
    s_pStack->pop_back();
}

void SetMatrixIdentity(Matrix *matrix) {
    SetIdentity(matrix->m);
    if (matrix->sub0 != NULL) {
        SetIdentity(matrix->sub0->m);
    }
    if (matrix->sub1 != NULL) {
        SetIdentity(matrix->sub1->m);
    }
}

void UseTopMatrixAsSource() {
    s_pSource = s_pStack->back();
}

void TranslateMatrix(f32 x, f32 y) {
    s_pTarget->m[2] = s_pSource->m[2] + (s_pSource->m[0] * x + s_pSource->m[1] * y);
    s_pTarget->m[5] = s_pSource->m[5] + (s_pSource->m[3] * x + s_pSource->m[4] * y);
    if (s_pTarget == s_pSource) {
        return;
    }
    s_pTarget->m[0] = s_pSource->m[0];
    s_pTarget->m[1] = s_pSource->m[1];
    s_pTarget->m[3] = s_pSource->m[3];
    s_pTarget->m[4] = s_pSource->m[4];
    if (s_pTarget->sub1 != NULL) {
        CopyMatrixValue(s_pTarget->sub1, s_pSource->sub1);
    }
    if (s_pTarget->sub0 != NULL) {
        CopyMatrixValue(s_pTarget->sub0, s_pSource->sub0);
    }
}

void SetMatrixTranslation(f32 x, f32 y) {
    s_pTarget->m[2] = x;
    s_pTarget->m[5] = y;
    CopyLinearParts();
}

void TranslateMatrixY(f32 x, f32 y) {
    s_pTarget->m[2] = x;
    s_pTarget->m[5] = s_pSource->m[4] * y + s_pSource->m[5];
    CopyLinearParts();
}

void TranslateMatrixX(f32 x, f32 y) {
    s_pTarget->m[2] = s_pSource->m[0] * x + s_pSource->m[2];
    s_pTarget->m[5] = y;
    CopyLinearParts();
}

void RotateMatrix(f32 degrees) {
    f32 angle = degrees * 0.017453292f; /* pi / 180 */
    f32 c = cos(angle);
    f32 s = sin(angle);
    f32 negS = -1.0f * s; /* the original multiplies by a -1.0f constant rather than negating */
    /* Read before the target is written: the target may be the source. */
    f32 m0 = s_pSource->m[0];
    f32 m3 = s_pSource->m[3];
    s_pTarget->m[0] = m0 * c + s_pSource->m[1] * negS;
    s_pTarget->m[1] = m0 * s + s_pSource->m[1] * c;
    s_pTarget->m[2] = s_pSource->m[2];
    s_pTarget->m[3] = s_pSource->m[3] * c + s_pSource->m[4] * negS;
    s_pTarget->m[4] = m3 * s + s_pSource->m[4] * c;
    s_pTarget->m[5] = s_pSource->m[5];
    if (s_pTarget->sub1 != NULL) {
        f32 subM0 = s_pSource->sub1->m[0];
        f32 subM3 = s_pSource->sub1->m[3];
        s_pTarget->sub1->m[0] = subM0 * c + s_pSource->sub1->m[1] * negS;
        s_pTarget->sub1->m[1] = subM0 * s + s_pSource->sub1->m[1] * c;
        s_pTarget->sub1->m[3] = s_pSource->sub1->m[3] * c + s_pSource->sub1->m[4] * negS;
        s_pTarget->sub1->m[4] = subM3 * s + s_pSource->sub1->m[4] * c;
    }
    if (s_pTarget != s_pSource && s_pTarget->sub0 != NULL) {
        CopyMatrixValue(s_pTarget->sub0, s_pSource->sub0);
    }
}

void ScaleMatrix(f32 x, f32 y) {
    s_pTarget->m[0] = s_pSource->m[0] * x;
    s_pTarget->m[1] = s_pSource->m[1] * y;
    s_pTarget->m[3] = s_pSource->m[3] * x;
    s_pTarget->m[4] = s_pSource->m[4] * y;
    if (s_pTarget->sub0 != NULL) {
        s_pTarget->sub0->m[0] = s_pSource->sub0->m[0] * x;
        s_pTarget->sub0->m[1] = s_pSource->sub0->m[1] * y;
        s_pTarget->sub0->m[3] = s_pSource->sub0->m[3] * x;
        s_pTarget->sub0->m[4] = s_pSource->sub0->m[4] * y;
    }
    if (s_pTarget == s_pSource) {
        return;
    }
    s_pTarget->m[2] = s_pSource->m[2];
    s_pTarget->m[5] = s_pSource->m[5];
    if (s_pTarget->sub1 != NULL) {
        CopyMatrixValue(s_pTarget->sub1, s_pSource->sub1);
    }
}

Matrix *GetTopMatrix() {
    return s_pStack->back();
}

} // namespace CSD
} // namespace Chao
