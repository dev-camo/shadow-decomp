#include "Game/fn_803C8_record_types.h"

/* Inferred interface: four register arguments, raw output word and 0/1 result. */
int SJMEM_IsGetChunk(const Fn803C8RecordView *record, int mode, int requested, int *actual) {
    int result;
    const char *diagnostics = lbl_80517120;

    SJCRS_Lock();
    if (record == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x80);
        strcat(message, diagnostics + 0x1C);
        SJERR_CallErr(message);
        result = 0;
    } else if ((int)record->word_4 == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x8C);
        strcat(message, diagnostics + 0x48);
        SJERR_CallErr(message);
        result = 0;
    } else {
        int value;

        if (mode == 0) {
            value = 0;
        } else if (mode == 1) {
            value = requested;
            if ((int)record->word_C < requested) {
                value = (int)record->word_C;
            }
        } else {
            value = 0;
            if (record->callback_1C != 0) {
                record->callback_1C(record->argument_20, -3);
            }
        }
        *actual = value;
        if (value != requested) {
            result = 0;
        } else {
            result = 1;
        }
    }
    SJCRS_Unlock();
    return result;
}
