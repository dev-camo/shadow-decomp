#include "Game/fn_803C8_record_types.h"

/* Inferred side-effect interface; original full arity and return type are unknown. */
void SJMEM_GetChunk(Fn803C8RecordView *record, int mode, int requested,
                    Fn803C8DescriptorView *descriptor) {
    const char *diagnostics = lbl_80517120;

    SJCRS_Lock();
    if (record == 0) {
        char message[64];

        strcpy(message, diagnostics + 0xC8);
        strcat(message, diagnostics + 0x1C);
        SJERR_CallErr(message);
    } else if ((int)record->word_4 == 0) {
        char message[64];

        strcpy(message, diagnostics + 0xD4);
        strcat(message, diagnostics + 0x48);
        SJERR_CallErr(message);
    } else if (mode == 0) {
        descriptor->word_4 = 0;
        descriptor->word_0 = 0;
    } else if (mode == 1) {
        descriptor->word_4 = (int)record->word_C < requested ? (int)record->word_C : requested;
        descriptor->word_0 = record->word_14 + record->word_10;
        record->word_10 += descriptor->word_4;
        record->word_C -= descriptor->word_4;
    } else {
        descriptor->word_4 = 0;
        descriptor->word_0 = 0;
        if (record->callback_1C != 0) {
            record->callback_1C(record->argument_20, -3);
        }
    }
    SJCRS_Unlock();
}
