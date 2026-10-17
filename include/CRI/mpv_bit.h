#ifndef CRI_MPV_BIT_H
#define CRI_MPV_BIT_H

/* CRI Sofdec MPV bit reader used by the picture and slice parsers (mpv_emp.c, mpv_dec.c): a
 * two-word window kept in the caller's locals `bbuf` (current 32 bits, left-aligned), `nbuf` (next
 * word), `bitpos` (bits of `bbuf` consumed) and `ptr` (the word after `nbuf`).
 */

#define MPVBIT_INIT(buf)                                                                           \
    ptr = (Uint32 *)((Uint32)(buf) & ~3);                                                          \
    bitpos = ((Uint32)(buf) - (Uint32)ptr) << 3;                                                   \
    bbuf = *ptr++;                                                                                 \
    bbuf <<= bitpos;                                                                               \
    nbuf = *ptr++

#define MPVBIT_PEEK(val, n)                                                                        \
    val = bbuf >> (32 - (n));                                                                      \
    if (bitpos > 32 - (n)) {                                                                       \
        val |= nbuf >> (64 - (n) - bitpos);                                                        \
    }

/* the first read consumes the whole 32-bit window */
#define MPVBIT_GET32(val)                                                                          \
    if (bitpos != 0) {                                                                             \
        val = bbuf | (nbuf >> (32 - bitpos));                                                      \
        bbuf = nbuf << bitpos;                                                                     \
    } else {                                                                                       \
        val = bbuf;                                                                                \
        bbuf = nbuf;                                                                               \
    }                                                                                              \
    nbuf = *ptr++

#define MPVBIT_GET(val, n)                                                                         \
    if (bitpos >= 32 - (n)) {                                                                      \
        bitpos -= 32 - (n);                                                                        \
        if (bitpos != 0) {                                                                         \
            bbuf |= nbuf >> ((n) - bitpos);                                                        \
            val = bbuf >> (32 - (n));                                                              \
            bbuf = nbuf << bitpos;                                                                 \
        } else {                                                                                   \
            val = bbuf >> (32 - (n));                                                              \
            bbuf = nbuf;                                                                           \
        }                                                                                          \
        nbuf = *ptr++;                                                                             \
    } else {                                                                                       \
        val = bbuf >> (32 - (n));                                                                  \
        bitpos += (n);                                                                             \
        bbuf <<= (n);                                                                              \
    }

#define MPVBIT_SKIP(n)                                                                             \
    bitpos += (n);                                                                                 \
    if (bitpos >= 32) {                                                                            \
        bitpos -= 32;                                                                              \
        bbuf = nbuf << bitpos;                                                                     \
        nbuf = *ptr++;                                                                             \
    } else {                                                                                       \
        bbuf <<= (n);                                                                              \
    }

#define MPVBIT_GET1(val)                                                                           \
    val = bbuf >> 31;                                                                              \
    if (bitpos == 31) {                                                                            \
        bbuf = nbuf;                                                                               \
        nbuf = *ptr++;                                                                             \
        bitpos = 0;                                                                                \
    } else {                                                                                       \
        bbuf <<= 1;                                                                                \
        bitpos++;                                                                                  \
    }

/* current byte position (rounded up to the next byte boundary) */
#define MPVBIT_BYTEPTR(q)                                                                          \
    q = (Sint8 *)ptr;                                                                              \
    q += (bitpos + 7) >> 3;                                                                        \
    q -= 8

#endif
