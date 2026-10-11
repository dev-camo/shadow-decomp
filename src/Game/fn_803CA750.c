#include "Game/fn_803C9_record_types.h"

/* Inferred accessed-word interface; original full types/arity/return identities unknown. */
void SJRBF_EntryErrFunc(Fn803C9RecordView *record, Fn803C8RecordCallback callback,
                        unsigned int context) {
    const char *diagnostics = lbl_80517288;

    SJCRS_Lock();
    if (record == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x1B4);
        strcat(message, diagnostics + 0x48);
        SJERR_CallErr(message);
    } else if ((int)record->word_4 == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x1C0);
        strcat(message, diagnostics + 0x74);
        SJERR_CallErr(message);
    } else {
        record->callback_38 = callback;
        record->argument_3C = context;
    }
    SJCRS_Unlock();
}
