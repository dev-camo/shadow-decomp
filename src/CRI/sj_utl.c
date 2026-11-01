/* CRI stream-joint utilities (sj_utl.c). Signatures from the CRI SJ header (sj.h). */
#include <string.h>

#include "CRI/sj.h"

/* ASCII hexadecimal digits map to their numeric values ('0'..'9' to 0..9, 'A'..'F' and 'a'..'f' to
 * 10..15); all other entries are zero. The table has 112 entries, so characters above 'o' are not
 * covered. */
// clang-format off
Uint32 sj_hexstr_to_val_tbl[112] = {
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 0x00..0x0F */
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 0x10..0x1F */
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 0x20..0x2F */
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0,  0,  0,  0,  0,  /* 0x30..0x3F */
     0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 0x40..0x4F */
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 0x50..0x5F */
     0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 0x60..0x6F */
};
// clang-format on

/* Reads a record length through the hex lookup table (inferred helper name). The original uses
 * signed byte indices without a range check. */
static inline Uint32 read_length(const Sint8 *record) {
    Uint32 length;

    length = sj_hexstr_to_val_tbl[record[8]];
    length = (length << 4) + sj_hexstr_to_val_tbl[record[9]];
    length = (length << 4) + sj_hexstr_to_val_tbl[record[10]];
    length = (length << 4) + sj_hexstr_to_val_tbl[record[11]];
    length = (length << 4) + sj_hexstr_to_val_tbl[record[12]];
    length = (length << 4) + sj_hexstr_to_val_tbl[record[13]];
    length = (length << 4) + sj_hexstr_to_val_tbl[record[14]];
    return length;
}

/* Finds the record tagged `kw` in the chunk (a record is a 7-character tag at +0, its data length
 * as 7 hex digits at +8 and the data at +0x10); *cko gets the record's data. Stops at a record
 * tagged `endkw`. Returns the record, or 0. */
Sint8 *SJ_SearchTag(SJCK *cki, const Char8 *kw, const Char8 *endkw, SJCK *cko) {
    Uint8 *cursor;
    Uint8 *end;

    cko->data = 0;
    cko->len = 0;
    end = cki->data + cki->len;
    cursor = cki->data;
    while (cursor < end) {
        if (strncmp((const char *)cursor, kw, 7) == 0) {
            cko->data = cursor + 0x10;
            cko->len = read_length((const Sint8 *)cursor);
            break;
        }
        if (endkw != 0 && strncmp((const char *)cursor, endkw, 7) == 0) {
            return 0;
        }
        cursor += read_length((const Sint8 *)cursor) + 0x10;
    }
    if (cursor < end) {
        return (Sint8 *)cursor;
    }
    return 0;
}

/* Splits *ck at nbyte: *ck1 gets the first nbyte bytes (or all of it), *ck2 the rest (data 0 when
 * empty). ck may alias ck1 or ck2. */
void SJ_SplitChunk(SJCK *ck, Sint32 nbyte, SJCK *ck1, SJCK *ck2) {
    *ck1 = *ck;
    ck2->len = ck1->len;
    if (ck1->len > nbyte) {
        ck1->len = nbyte;
    }
    ck2->len -= ck1->len;
    if (ck2->len == 0) {
        ck2->data = 0;
    } else {
        ck2->data = ck1->data + ck1->len;
    }
}
