/* Looks up a configuration entry by key in a table of 16-byte records (called by the CRI timer
 * code). The table and its owner are unknown. */
#include <string.h>
#include "layout_check.h"
#include "CRI/uty.h"

/* One table record as this function reads it: a 12-byte key and a word. The record's real type and
 * the rest of its layout are unknown. */
typedef struct {
    /* 0x00 */ char key[12];
    /* 0x0C */ unsigned int field_0xC;
} Fn803CC9RecordView;
CHECK_SIZEOF(Fn803CC9RecordView, 0x10);

/* record count (compared signed) and table pointer; original types unknown */
extern int lbl_805C0AF0;
extern Fn803CC9RecordView *lbl_805C0AF4;

/* Finds the record whose key matches the first 12 bytes of `key` (inferred helper). */
static inline Fn803CC9RecordView *find_record(Fn803CC9RecordView *entry, const char *key) {
    int index;
    if (key[0] == 0) {
        return 0;
    }
    for (index = 0; index < lbl_805C0AF0; ++index, ++entry) {
        if (strncmp(entry->key, key, 12) == 0) {
            return entry;
        }
    }
    return 0;
}

/* Looks up `key` and stores the record word in *output_word: 0 on success, -1 without a table,
 * -3 when the key is empty or not found. Full interface unknown. */
int fn_803CC9D0(const char *key, unsigned int *output_word) {
    Fn803CC9RecordView *entry = lbl_805C0AF4;
    if (entry == 0) {
        return -1;
    }
    entry = find_record(entry, key);
    if (entry == 0) {
        return -3;
    }
    *output_word = entry->field_0xC;
    return 0;
}
