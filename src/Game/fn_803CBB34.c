#include "Game/fn_803CB_state_types.h"
#include "Game/fn_803CB_text_types.h"
#include "mwcc_varargs.h"

extern void *memset(void *destination, int value, unsigned int size);

/* Chosen variadic side-effect projection; original return/full interface unknown. */
void SVM_CallErr(const char *format, ...) {
    MwccVaList args;
    Fn803CBTextCallback callback;
    Fn803CBPairView *pair;

    memset(svmerr_msg, 0, 0x80);
    /* Keep cursor initialization after clearing the output buffer. */
    __builtin_va_info(args);
    vsprintf((char *)svmerr_msg, format, args);
    pair = &svm_err_func;
    callback = (Fn803CBTextCallback)pair->word_0;
    if (callback != 0) {
        callback(pair->word_4, (const char *)svmerr_msg);
    }
}
