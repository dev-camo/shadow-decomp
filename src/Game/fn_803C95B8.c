#include "Game/fn_803C8_record_types.h"

/* Inferred accessed-word interface; original full types/arity/return identities unknown. */
void SJMEM_EntryErrFunc(Fn803C8RecordView *record, Fn803C8RecordCallback callback,
                        unsigned int context) {
    const char *diagnostics = lbl_80517120;

    SJCRS_Lock();
    if (record == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x110);
        strcat(message, diagnostics + 0x1C);
        SJERR_CallErr(message);
    } else if ((int)record->word_4 == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x11C);
        strcat(message, diagnostics + 0x48);
        SJERR_CallErr(message);
    } else {
        record->callback_1C = callback;
        record->argument_20 = context;
    }
    SJCRS_Unlock();
}
