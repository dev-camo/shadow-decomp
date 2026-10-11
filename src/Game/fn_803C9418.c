#include "Game/fn_803C8_record_types.h"

/* Inferred raw-word result and two consumed arguments; original full types/arity unknown. */
unsigned int SJMEM_GetNumData(const Fn803C8RecordView *record, int mode) {
    const char *diagnostics = lbl_80517120;
    unsigned int result;

    SJCRS_Lock();
    if (record == 0) {
        char message[64];

        strcpy(message, diagnostics + 0xE0);
        strcat(message, diagnostics + 0x1C);
        SJERR_CallErr(message);
        result = 0;
    } else if ((int)record->word_4 == 0) {
        char message[64];

        strcpy(message, diagnostics + 0xEC);
        strcat(message, diagnostics + 0x48);
        SJERR_CallErr(message);
        result = 0;
    } else if (mode == 1) {
        result = record->word_C;
    } else if (mode == 0) {
        result = 0;
    } else {
        if (record->callback_1C != 0) {
            record->callback_1C(record->argument_20, -3);
        }
        result = 0;
    }
    SJCRS_Unlock();
    return result;
}
