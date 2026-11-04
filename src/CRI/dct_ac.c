/* CRI DCT library, reference transform (dct_ac.c): a double-precision 8x8 DCT by two matrix
 * multiplications with cosine matrices. dct_fsri.c uses the inverse transform only at library
 * init, to build its PreIDCT table.
 * Body from the RE4 GameCube decompilation of the same file; the cosine helper as in the RE4 and
 * MK Deception decompilations.
 *
 * <math.h> is MSL's (see configure.py): its inline sqrtf defines two static double constants,
 * which come first in .rodata. These constants, dctac_Cos and dctac_BssOrder are not in the linked
 * game; the linker strips them.
 */
#include <math.h>

#include "CRI/cri_xpt.h"
#include "CRI/dct.h"

#define DCTAC_PI 3.141592653589793

Float64 dctac_i_const[8][8];
Float64 dctac_f_const[8][8];
static const Char8 *dctac_version_dummy;

/* 8x8 transform by the cosine matrix c: the rows of in, then the columns of the result. */
void dctac_TransDouble(Float64 *in, Float64 *out, Float64 c[8][8]) {
    Float64 tmp[64];
    Float64 s;
    Sint32 i;
    Sint32 j;
    Sint32 k;

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            s = 0.0;
            for (k = 0; k < 8; k++) {
                s += c[k][j] * in[i * 8 + k];
            }
            tmp[i * 8 + j] = s;
        }
    }
    for (j = 0; j < 8; j++) {
        for (i = 0; i < 8; i++) {
            s = 0.0;
            for (k = 0; k < 8; k++) {
                s += c[k][i] * tmp[k * 8 + j];
            }
            out[i * 8 + j] = s;
        }
    }
}

/* cos(w * (j + 1/2)): the cosine of column j for a row's angle step w (name inferred). Compiled
 * here, before DCT_AcInit inlines it, it gives .rodata its 0.5 and int-to-double constants ahead
 * of DCT_AcInit's 1/sqrt(8) and pi/8, as in the original. */
static Float64 dctac_Cos(Float64 w, Sint32 j) {
    return cos(w * (0.5 + (Float64)j));
}

/* Inverse DCT of the 64 coefficients in in (raster order) into out. */
void DCT_AcIdctDouble(Float64 *in, Float64 *out) {
    dctac_TransDouble(in, out, dctac_i_const);
}

/* Matching workaround: order-only stand-in. DCT_AcInit only writes dctac_f_const, yet the original
 * .bss has dctac_f_const before dctac_version_dummy. One explanation, inferred from that layout, is
 * an earlier, separately compiled function that referred to it and was stripped by the linker; the
 * order could also come from a different source shape (for example a pointer to the forward rows
 * set before DCT_GetVerStr() reproduces it, but gives a 67-word DCT_AcInit against the original's
 * 64). Storing its address here keeps that reference. Without it dctac_version_dummy lands before
 * dctac_f_const and DCT_AcInit differs in 2 of its 64 words (measured). Revisit when the cause of
 * the order is known. */
static void dctac_BssOrder(void **out) {
    out[0] = dctac_f_const;
}

/* Fills the inverse cosine matrix and its transpose, the forward matrix. */
void DCT_AcInit(void) {
    Sint32 i;
    Sint32 j;
    Float64 c;
    Float64 w;
    Float64 v;

    dctac_version_dummy = DCT_GetVerStr();
    for (i = 0; i < 8; i++) {
        if (i == 0) {
            c = 0.3535533905932738;
        } else {
            c = 0.5;
        }
        w = (DCTAC_PI / 8.0) * (Float64)i;
        for (j = 0; j < 8; j++) {
            v = c * dctac_Cos(w, j);
            dctac_i_const[i][j] = v;
            dctac_f_const[j][i] = v;
        }
    }
}
