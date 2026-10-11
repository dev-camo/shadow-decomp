#include "msl_string.h"

/* Inferred access view: twelve key bytes followed by one raw word.
 * Original record/array identity and complete types remain unknown. */
typedef struct {
    /* 0x00 */ char key_0[12];
    /* 0x0C */ unsigned int word_C;
} Fn803CC9RecordView;

/* Signed count projection follows the observed comparison; original type unknown. */
extern int lbl_805C0AF0;
extern Fn803CC9RecordView *lbl_805C0AF4;

/* Inferred helper; no original helper identity is established. */
static inline Fn803CC9RecordView *find_record(Fn803CC9RecordView *entry, const char *key) {
    int index;
    if ((signed char)key[0] == 0) {
        return 0;
    }
    for (index = 0; index < lbl_805C0AF0; ++index, ++entry) {
        if (strncmp(entry->key_0, key, 12) == 0) {
            return entry;
        }
    }
    return 0;
}

/* Chosen key/output/status projection; original full interface unknown. */
int fn_803CC9D0(const char *key, unsigned int *output_word) {
    Fn803CC9RecordView *entry = lbl_805C0AF4;
    if (entry == 0) {
        return -1;
    }
    entry = find_record(entry, key);
    if (entry == 0) {
        return -3;
    }
    *output_word = entry->word_C;
    return 0;
}
