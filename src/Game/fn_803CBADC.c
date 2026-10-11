#include "Game/fn_803CB_state_types.h"
#include "Game/fn_803CB_text_types.h"
#include "msl_string.h"

/* Chosen message side-effect projection; original full types/arity/return unknown. */
void SVM_CallErr1(const char *text) {
    Fn803CBTextCallback callback;
    strncpy((char *)svmerr_msg, text, 0x7F);
    callback = (Fn803CBTextCallback)svm_err_func.word_0;
    if (callback != 0) {
        callback(svm_err_func.word_4, (const char *)svmerr_msg);
    }
}
