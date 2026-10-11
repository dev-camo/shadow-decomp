#ifndef GAME_FN_803C84_ENTRY_TYPES_H
#define GAME_FN_803C84_ENTRY_TYPES_H

#include "Game/fn_803C82_record_types.h"

/* Chosen accessed-field views; original C type and allocation identity unknown. */
typedef struct Fn803C84EntryView {
    /* 0x00 */ unsigned int word_0;
    /* 0x04 */ const char *text_4;
    /* 0x08 */ unsigned int word_8;
    /* 0x0C */ unsigned int word_C;
    /* 0x10 */ unsigned int word_10;
    /* 0x14 */ unsigned int word_14;
    /* 0x18 */ unsigned int word_18;
    /* 0x1C */ unsigned int word_1C;
} Fn803C84EntryView;
typedef struct Fn803C84RecordView {
    /* 0x00 */ Fn803C82RecordView header;
    /* 0x38 */ Fn803C84EntryView entries_38[16];
} Fn803C84RecordView;

#ifdef __cplusplus
extern "C" {
#endif

/* Chosen consumed-register/returned-word projection; original full interface unknown. */
int LSC_EntryFileRange(Fn803C84RecordView *record, const char *text, unsigned int word_C,
                       unsigned int word_10, unsigned int word_14);
/* Chosen installed-pointer/side-effect projection; original full interface unknown. */
void fn_803C86EC(Fn803C82RecordView *record, void *value);

#ifdef __cplusplus
}
#endif

#endif
