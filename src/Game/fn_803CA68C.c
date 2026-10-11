#include "Game/fn_803C9_record_types.h"

/* Inferred side-effect projection; original full types/arity/return unknown. */
void SJRBF_Reset(Fn803C9RecordView *record) {
    const char *diagnostics = lbl_80517288;

    SJCRS_Lock();
    if (record == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x19C);
        strcat(message, diagnostics + 0x48);
        SJERR_CallErr(message);
    } else if ((int)record->word_4 == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x1A8);
        strcat(message, diagnostics + 0x74);
        SJERR_CallErr(message);
    } else {
        record->word_C = 0;
        record->word_10 = record->word_20;
        record->word_14 = 0;
        record->word_18 = 0;
        record->word_28 = 0;
        record->word_2C = 0;
        record->word_30 = 0;
        record->word_34 = 0;
    }
    SJCRS_Unlock();
}
