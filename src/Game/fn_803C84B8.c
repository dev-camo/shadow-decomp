#include "Game/fn_803C84_entry_types.h"

extern const char lbl_80516FD4[];
extern const char lbl_80517000[];
extern void LSC_CallErrFunc(const char *format, ...);
extern unsigned int strlen(const char *text);
/* Chosen consumed-register/returned-word projection; original full interface unknown. */
int LSC_EntryFileRange(Fn803C84RecordView *record, const char *text, unsigned int word_C,
                       unsigned int word_10, unsigned int word_14) {
    Fn803C84EntryView *entry;
    int generation;
    unsigned int length;
    unsigned int index;
    unsigned int previous;
    if (record == 0) {
        LSC_CallErrFunc(lbl_80516FD4);
        return -1;
    }
    if ((int)record->header.word_24 >= 16) {
        return -1;
    }
    if (text == 0) {
        LSC_CallErrFunc(lbl_80517000);
        return -1;
    }
    previous = record->entries_38[(int)(record->header.word_1C + 15u) % 16].word_0;
    generation = (int)(previous == 0x7FFFFFFF ? 0u : previous + 1u);
    entry = &record->entries_38[record->header.word_1C];
    entry->word_0 = generation;
    entry->text_4 = text;
    length = strlen(text);
    entry->word_8 = 0;
    for (index = 0; index < length; index++) {
        entry->word_8 += (unsigned char)text[index];
    }
    entry->word_10 = word_10;
    entry->word_14 = word_14;
    entry->word_C = word_C;
    entry->word_18 = 0;
    entry->word_1C = 0;
    record->header.word_24++;
    record->header.word_1C = (int)(record->header.word_1C + 1u) % 16;
    if ((int)(unsigned char)record->header.byte_1 == 1) {
        record->header.byte_1 = 2;
    }
    return generation;
}

/* Chosen installed-pointer projection; original full interface unknown. */
void fn_803C86EC(Fn803C82RecordView *record, void *value) {
    record->pointer_28 = value;
}
