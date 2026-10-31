/* CRI ADX error reporting (adx_errs.c): the ADXERR_CallErrFunc1/2 entry points every ADX module
 * reports through. Copies the message into adxerr_msg (256 bytes), calls the optional ADX-level
 * callback and always the SVM error callback (the game's ap_mwply_err_func via ADXM_SetCbErr).
 * Bodies from the RE4 GameCube decompilation of the same file.
 */
#include "CRI/adx.h"
#include "CRI/svm.h"
#include <string.h>

static void (*adxerr_func)(void *obj, Char8 *msg) = 0;
static void *adxerr_obj = 0;
static Char8 adxerr_msg[256];

/* Intended as the decimal formatter for the two-number error suffixes; inlined into ADXERR_ItoA2
 * (the out-of-line copy is stripped by the linker) and verified through those inlined copies.
 * The matched body keeps an inherited CRI quirk: the digit loop stores raw digit values (no '0'
 * offset) into `str`, `buf` is never written, and the reversed copy from the empty `buf` then
 * leaves `str` as an empty string. */
static void adxerr_itoa(Sint32 val, Char8 *str, Sint32 len) {
    static Char8 buf[32];
    Sint32 i;
    Sint32 n;
    Sint32 l;

    for (i = 0; i < 32; i++) {
        str[i] = val % 10;
        val /= 10;
        if (val == 0) {
            str[i] = '\0';
            break;
        }
    }
    l = strlen(buf);
    n = len - 1;
    if (l < n) {
        n = l;
    }
    for (i = 0; i < n; i++) {
        str[i] = buf[n - 1 - i];
    }
    str[i] = '\0';
}

/* Meant to format "a b" into `str` for the talk server's decoder-information error message; with
 * the adxerr_itoa quirk above the numbers come out empty and only the separating space remains. */
void ADXERR_ItoA2(Sint32 a, Sint32 b, Char8 *str, Sint32 len) {
    adxerr_itoa(a, str, len);
    strncat(str, " ", len - strlen(str) - 1);
    /* original quirk: the size is a literal 4 minus the current length, not len */
    adxerr_itoa(b, str + strlen(str), 4 - strlen(str));
}

/* Reports a two-part error message (code prefix + text) to the ADX error callback and the SVM error
 * callback the game installed with ADXM_SetCbErr (ap_mwply_err_func in sofdec.cpp). */
void ADXERR_CallErrFunc2(const Char8 *msg1, const Char8 *msg2) {
    strncpy(adxerr_msg, msg1, 255);
    strncat(adxerr_msg, msg2, 255);
    if (adxerr_func != 0) {
        adxerr_func(adxerr_obj, adxerr_msg);
    }
    SVM_CallErr(adxerr_msg);
}

/* Reports one error message to the ADX error callback and the SVM error callback. */
void ADXERR_CallErrFunc1(const Char8 *msg) {
    strncpy(adxerr_msg, msg, 255);
    if (adxerr_func != 0) {
        adxerr_func(adxerr_obj, adxerr_msg);
    }
    SVM_CallErr(adxerr_msg);
}

/* Clears the message buffer and the callback. */
void ADXERR_Finish(void) {
    memset(adxerr_msg, 0, sizeof(adxerr_msg));
    adxerr_func = 0;
    adxerr_obj = 0;
}

/* Clears the message buffer and the callback. */
void ADXERR_Init(void) {
    memset(adxerr_msg, 0, sizeof(adxerr_msg));
    adxerr_func = 0;
    adxerr_obj = 0;
}
