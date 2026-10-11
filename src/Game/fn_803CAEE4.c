/* Partial word layout only; original type identity and full interface unknown. */
typedef struct Fn803CAEE4Words {
    /* 0x00 */ unsigned int word_0;
    /* 0x04 */ unsigned int word_4;
} Fn803CAEE4Words;

/* Chosen side-effect projection; original return and full arity unknown. */
void SJ_SplitChunk(Fn803CAEE4Words *input, int limit, Fn803CAEE4Words *prefix,
                   Fn803CAEE4Words *suffix) {
    *prefix = *input;
    suffix->word_4 = prefix->word_4;
    if ((int)prefix->word_4 > limit) {
        prefix->word_4 = (unsigned int)limit;
    }
    suffix->word_4 -= prefix->word_4;
    if ((int)suffix->word_4 == 0) {
        suffix->word_0 = 0;
    } else {
        suffix->word_0 = prefix->word_0 + prefix->word_4;
    }
}
