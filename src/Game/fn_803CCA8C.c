/* Inferred callback signature from the three-argument indirect calls in this unit. */
typedef void (*Fn803CCErrorCallback)(void *context, const char *text, int value);

/* Inferred handle access view from the fields read and written by these functions. */
typedef struct {
    /* 0x00 */ unsigned char unk_0[2];
    /* 0x02 */ signed char byte_2;
    /* 0x03 */ unsigned char unk_3[0xD];
    /* 0x10 */ int word_10;
    /* 0x14 */ int word_14;
    /* 0x18 */ int word_18;
    /* 0x1C */ int word_1C;
    /* 0x20 */ int word_20;
    /* 0x24 */ int word_24;
} Fn803CCHandleView;

extern void *lbl_805C0B04;
extern Fn803CCErrorCallback lbl_805C0B08;
extern const char lbl_80517954[];
extern const char lbl_8051796C[];
extern const char lbl_80517984[];
extern const char lbl_805179A0[];

/* Chosen handle/word projection; original full interface unknown. */
int fn_803CCA8C(Fn803CCHandleView *handle) {
    if (handle == 0) {
        if (lbl_805C0B08 != 0) {
            lbl_805C0B08(lbl_805C0B04, lbl_80517954, 0);
        }
        return 0;
    }
    return handle->word_20;
}

/* Chosen handle/value projection; callers do not consume a result. */
void fn_803CCAEC(Fn803CCHandleView *handle, int value) {
    int product;
    if (handle == 0) {
        if (lbl_805C0B08 != 0) {
            lbl_805C0B08(lbl_805C0B04, lbl_8051796C, 0);
        }
        return;
    }
    if (handle->word_10 % 32 != 0) {
        if (lbl_805C0B08 != 0) {
            lbl_805C0B08(lbl_805C0B04, lbl_80517984, 0);
        }
        return;
    }
    product = handle->word_1C * handle->word_10;
    handle->word_10 = value;
    /* This operand order reproduces the target's add-then-decrement sequence. */
    handle->word_18 = (handle->word_10 - 1 + handle->word_14) / handle->word_10;
    handle->word_1C = product / handle->word_10;
    handle->word_20 = handle->word_24 * value;
}

/* Chosen handle/word projection; original full interface unknown. */
int fn_803CCBD0(Fn803CCHandleView *handle) {
    if (handle == 0) {
        if (lbl_805C0B08 != 0) {
            lbl_805C0B08(lbl_805C0B04, lbl_805179A0, 0);
        }
        return 0;
    }
    return handle->word_10;
}

/* Chosen handle/byte projection; original full interface unknown. */
int fn_803CCC30(Fn803CCHandleView *handle) {
    if (handle == 0) {
        if (lbl_805C0B08 != 0) {
            lbl_805C0B08(lbl_805C0B04, lbl_80517954, 0);
        }
        return 0;
    }
    return handle->byte_2;
}
