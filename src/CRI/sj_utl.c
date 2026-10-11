/* CRI stream-joint utilities (sj_utl.c). */

/* Inferred word lookup backing; original C type identity is unknown.
 * ASCII hexadecimal digits map to their numeric values; other entries are zero.
 */
unsigned int sj_hexstr_to_val_tbl[112] = {
    /* 0x00..0x0F */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    /* 0x10..0x1F */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    /* 0x20..0x2F */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    /* 0x30..0x3F */
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    0,
    0,
    0,
    0,
    0,
    0,
    /* 0x40..0x4F */
    0,
    10,
    11,
    12,
    13,
    14,
    15,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    /* 0x50..0x5F */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    /* 0x60..0x6F */
    0,
    10,
    11,
    12,
    13,
    14,
    15,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

/* Two-word chunk descriptor view (data address, length) shared by both
 * functions; original field types are unknown. */
typedef struct SjChunkView {
    /* 0x00 */ unsigned int word_0;
    /* 0x04 */ unsigned int word_4;
} SjChunkView;

/* MSL strncmp; see msl_string.h. */
extern int strncmp(const char *left, const char *right, unsigned int count);

/* The original uses signed byte indices without a range check. */
static inline unsigned int read_length(const signed char *record) {
    unsigned int length;

    length = sj_hexstr_to_val_tbl[(int)record[8]];
    length = (length << 4) + sj_hexstr_to_val_tbl[(int)record[9]];
    length = (length << 4) + sj_hexstr_to_val_tbl[(int)record[10]];
    length = (length << 4) + sj_hexstr_to_val_tbl[(int)record[11]];
    length = (length << 4) + sj_hexstr_to_val_tbl[(int)record[12]];
    length = (length << 4) + sj_hexstr_to_val_tbl[(int)record[13]];
    length = (length << 4) + sj_hexstr_to_val_tbl[(int)record[14]];
    return length;
}

/* Selected raw-address/word interface; original full types/arity/return unknown. */
unsigned int SJ_SearchTag(SjChunkView *span, const char *wanted, const char *stop,
                          SjChunkView *output) {
    unsigned int cursor;
    unsigned int end;

    output->word_0 = 0;
    output->word_4 = 0;
    end = span->word_0 + span->word_4;
    cursor = span->word_0;
    while (cursor < end) {
        if (strncmp((const char *)cursor, wanted, 7) == 0) {
            output->word_0 = cursor + 0x10;
            output->word_4 = read_length((const signed char *)cursor);
            break;
        }
        if (stop != 0 && strncmp((const char *)cursor, stop, 7) == 0) {
            return 0;
        }
        cursor += read_length((const signed char *)cursor) + 0x10;
    }
    if (cursor < end) {
        return cursor;
    }
    return 0;
}

/* Chosen side-effect projection; original return and full arity unknown. */
void SJ_SplitChunk(SjChunkView *input, int limit, SjChunkView *prefix, SjChunkView *suffix) {
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
