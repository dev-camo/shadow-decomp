#include "Game/fn_803C8_record_types.h"

/* Inferred accessed-word interface; original full types/arity/return identities unknown. */
void SJMEM_Reset(Fn803C8RecordView *record) {
    const char *diagnostics = lbl_80517120;

    SJCRS_Lock();
    if (record == 0) {
        char message[64];

        strcpy(message, diagnostics + 0xF8);
        strcat(message, diagnostics + 0x1C);
        SJERR_CallErr(message);
    } else if ((int)record->word_4 == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x104);
        strcat(message, diagnostics + 0x48);
        SJERR_CallErr(message);
    } else {
        record->word_C = record->word_18;
        record->word_10 = 0;
    }
    SJCRS_Unlock();
}
