#include "Game/fn_803C8_record_types.h"

/* Inferred raw 32-bit result; only the two indicated fields are observed. */
unsigned int SJMEM_GetBufSize(const Fn803C8RecordView *record) {
    unsigned int result;
    const char *diagnostics = lbl_80517120;

    SJCRS_Lock();
    if (record == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x10);
        strcat(message, diagnostics + 0x1C);
        SJERR_CallErr(message);
        result = 0;
    } else if ((int)record->word_4 == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x3C);
        strcat(message, diagnostics + 0x48);
        SJERR_CallErr(message);
        result = 0;
    } else {
        result = record->word_18;
    }
    SJCRS_Unlock();
    return result;
}
