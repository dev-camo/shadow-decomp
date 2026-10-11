#include "Game/fn_803C9_record_types.h"

/* Body-inferred raw-word result; original full types/arity are unknown. */
unsigned int SJRBF_GetFlowCnt(const Fn803C9RecordView *record, unsigned int row,
                              unsigned int column) {
    const char *diagnostics = lbl_80517288;
    unsigned int result;

    SJCRS_Lock();
    if (record == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x3C);
        strcat(message, diagnostics + 0x48);
        SJERR_CallErr(message);
        result = 0;
    } else if ((int)record->word_4 == 0) {
        char message[64];

        strcpy(message, diagnostics + 0x68);
        strcat(message, diagnostics + 0x74);
        SJERR_CallErr(message);
        result = 0;
    } else {
        const unsigned char *address = (const unsigned char *)record + (row << 3);

        address += column << 2;
        result = *(const unsigned int *)(address + 0x28);
    }
    SJCRS_Unlock();
    return result;
}
