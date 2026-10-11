#include "Game/fn_803C8_record_types.h"

/* Inferred side-effect interface; original full argument lists/returns are unknown. */
void SJMEM_PutChunk(const Fn803C8RecordView *record, int mode, Fn803C8DescriptorView *descriptor) {
    const char *diagnostics = lbl_80517120;

    SJCRS_Lock();
    if (record == 0) {
        char message[64];

        strcpy(message, diagnostics + 0xB0);
        strcat(message, diagnostics + 0x1C);
        SJERR_CallErr(message);
    } else if ((int)record->word_4 == 0) {
        char message[64];

        strcpy(message, diagnostics + 0xBC);
        strcat(message, diagnostics + 0x48);
        SJERR_CallErr(message);
    } else if (descriptor->word_4 > 0 && descriptor->word_0 != 0 && mode != 0 && mode != 1) {
        descriptor->word_4 = 0;
        descriptor->word_0 = 0;
        if (record->callback_1C != 0) {
            record->callback_1C(record->argument_20, -3);
        }
    }
    SJCRS_Unlock();
}
