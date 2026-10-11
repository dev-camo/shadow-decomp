#include "Game/fn_803C9_record_types.h"

/* Inferred accessed-word interface; original full types/arity/return identities unknown. */
unsigned int SJRBF_GetUuid(const Fn803C9RecordView *record) {
    const char *diagnostics = lbl_80517288;
    unsigned int result;

    SJCRS_Lock();
    if (record == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x1CC);
        strcat(message, diagnostics + 0x48);
        SJERR_CallErr(message);
        result = 0;
    } else if ((int)record->word_4 == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x1D8);
        strcat(message, diagnostics + 0x74);
        SJERR_CallErr(message);
        result = 0;
    } else {
        result = record->word_8;
    }
    SJCRS_Unlock();
    return result;
}
