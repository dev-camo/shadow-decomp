#ifndef GAME_FN_803C82_RECORD_INLINE_H
#define GAME_FN_803C82_RECORD_INLINE_H

#include "Game/fn_803C82_record_types.h"

extern const char lbl_80516F50[];
extern const char lbl_80516F7C[];
extern void LSC_CallErrFunc(const char *format, ...);
/* Chosen consumed-pointer/side-effect projection; original full interface unknown. */
extern void fn_803C0018(void *object);

/* Inlined CRI LSC queue clear (lsc.c; empties a stopped scheduler's queue). */
static inline void fn_803C82_clear_idle_fields(Fn803C82RecordView *object) {
    if (object == 0) {
        LSC_CallErrFunc(lbl_80516F7C);
    } else if (object->byte_1 == 0) {
        object->word_1C = 0;
        object->word_20 = 0;
        object->word_24 = 0;
    }
}
/* Inlined body of CRI LSC_Stop. */
static inline void fn_803C82_reset_record(Fn803C82RecordView *object) {
    if (object == 0) {
        LSC_CallErrFunc(lbl_80516F50);
    } else if (object->byte_1 != 0) {
        object->byte_1 = 0;
        if (object->pointer_28 != 0 && (int)object->byte_2 == 1) {
            fn_803C0018(object->pointer_28);
            object->byte_2 = 0;
        }
        object->word_2C = 0;
        fn_803C82_clear_idle_fields(object);
        object->word_34 = 0;
    }
}

#endif
