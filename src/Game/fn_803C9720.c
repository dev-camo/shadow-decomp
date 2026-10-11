#include "Game/fn_803C8_record_types.h"

/* Existing runtime memory-set interface; return unused in this unit. */
extern void *memset(void *destination, int value, unsigned int size);

/* Inferred accessed-word interface; original full types/arity/return identities unknown. */
void SJMEM_Destroy(Fn803C8RecordView *record) {
    const char *diagnostics = lbl_80517120;

    SJCRS_Lock();
    if (record == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x140);
        strcat(message, diagnostics + 0x1C);
        SJERR_CallErr(message);
    } else if ((int)record->word_4 == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x14C);
        strcat(message, diagnostics + 0x48);
        SJERR_CallErr(message);
    } else {
        memset(record, 0, 0x24);
        /* Original stores this word separately after clearing the 0x24-byte span. */
        record->word_4 = 0;
    }
    SJCRS_Unlock();
}
