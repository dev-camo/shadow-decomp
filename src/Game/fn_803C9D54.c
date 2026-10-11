#include "Game/fn_803C9_record_types.h"

/* Body-inferred comparison/output interface; original full types/arity are unknown. */
int SJRBF_IsGetChunk(const Fn803C9RecordView *record, int mode, int requested, int *actual) {
    const char *diagnostics = lbl_80517288;
    int result;

    SJCRS_Lock();
    if (record == 0) {
        char message[64];

        strcpy(message, diagnostics + 0xF4);
        strcat(message, diagnostics + 0x48);
        SJERR_CallErr(message);
        result = 0;
    } else if ((int)record->word_4 == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x100);
        strcat(message, diagnostics + 0x74);
        SJERR_CallErr(message);
        result = 0;
    } else {
        int count;

        if (mode == 0) {
            int total = (int)(record->word_24 + (record->word_20 - record->word_14));
            int limit = (int)record->word_10 < total ? (int)record->word_10 : total;

            count = requested;
            if (limit < requested) {
                count = limit;
            }
        } else if (mode == 1) {
            int total = (int)(record->word_24 + (record->word_20 - record->word_18));
            int limit = (int)record->word_C < total ? (int)record->word_C : total;

            count = requested;
            if (limit < requested) {
                count = limit;
            }
        } else {
            count = 0;
            if (record->callback_38 != 0) {
                record->callback_38(record->argument_3C, -3);
            }
        }
        *actual = count;
        if (count != requested) {
            result = 0;
        } else {
            result = 1;
        }
    }
    SJCRS_Unlock();
    return result;
}
