#include "Game/fn_803C9_record_types.h"

/* Existing runtime memory-set interface; return unused in this unit. */
extern void *memset(void *destination, int value, unsigned int size);

/* Inferred accessed-word interface; original full types/arity/return identities unknown. */
void SJRBF_Destroy(Fn803C9RecordView *record) {
    const char *diagnostics = lbl_80517288;

    SJCRS_Lock();
    if (record == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x1E4);
        strcat(message, diagnostics + 0x48);
        SJERR_CallErr(message);
    } else if ((int)record->word_4 == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x1F0);
        strcat(message, diagnostics + 0x74);
        SJERR_CallErr(message);
    } else {
        memset(record, 0, 0x40);
        /* Observed literal span; original full record type is unknown. */
        /* The original also stores this word separately after the clear. */
        record->word_4 = 0;
    }
    SJCRS_Unlock();
}
