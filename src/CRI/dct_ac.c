/* CRI DCT library, reference transform (dct_ac.c): a double-precision 8x8 DCT by two matrix
 * multiplications with cosine matrices. dct_fsri.c uses the inverse transform only at library
 * init, to build its PreIDCT table.
 * Body from the RE4 GameCube decompilation of the same file.
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

/* Inverse DCT of the 64 coefficients in in (raster order) into out. */
void DCT_AcIdctDouble(Float64 *in, Float64 *out) {
    dctac_TransDouble(in, out, dctac_i_const);
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
            v = c * cos(w * (0.5 + (Float64)j));
            dctac_i_const[i][j] = v;
            dctac_f_const[j][i] = v;
        }
    }
}
