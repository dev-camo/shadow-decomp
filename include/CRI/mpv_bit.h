#ifndef CRI_MPV_BIT_H
#define CRI_MPV_BIT_H

#include "CRI/cri_xpt.h"

/* CRI Sofdec MPV bit reader used by the picture and slice parsers (mpv_emp.c, mpv_dec.c): a
 * two-word window kept in the caller's locals `bbuf` (current 32 bits, left-aligned), `nbuf` (next
 * word), `bitpos` (bits of `bbuf` consumed) and `ptr` (the word after `nbuf`).
 */

#define MPVBIT_INIT(buf)                                                                           \
    do {                                                                                           \
        ptr = (Uint32 *)((Uint32)(buf) & ~3);                                                      \
        bitpos = ((Uint32)(buf) - (Uint32)ptr) << 3;                                               \
        bbuf = *ptr++;                                                                             \
        bbuf <<= bitpos;                                                                           \
        nbuf = *ptr++;                                                                             \
    } while (0)

#define MPVBIT_PEEK(val, n)                                                                        \
    do {                                                                                           \
        val = bbuf >> (32 - (n));                                                                  \
        if (bitpos > 32 - (n)) {                                                                   \
            val |= nbuf >> (64 - (n) - bitpos);                                                    \
        }                                                                                          \
    } while (0)

/* the first read consumes the whole 32-bit window */
#define MPVBIT_GET32(val)                                                                          \
    do {                                                                                           \
        if (bitpos != 0) {                                                                         \
            val = bbuf | (nbuf >> (32 - bitpos));                                                  \
            bbuf = nbuf << bitpos;                                                                 \
        } else {                                                                                   \
            val = bbuf;                                                                            \
            bbuf = nbuf;                                                                           \
        }                                                                                          \
        nbuf = *ptr++;                                                                             \
    } while (0)

#define MPVBIT_GET(val, n)                                                                         \
    do {                                                                                           \
        if (bitpos >= 32 - (n)) {                                                                  \
            bitpos -= 32 - (n);                                                                    \
            if (bitpos != 0) {                                                                     \
                bbuf |= nbuf >> ((n) - bitpos);                                                    \
                val = bbuf >> (32 - (n));                                                          \
                bbuf = nbuf << bitpos;                                                             \
            } else {                                                                               \
                val = bbuf >> (32 - (n));                                                          \
                bbuf = nbuf;                                                                       \
            }                                                                                      \
            nbuf = *ptr++;                                                                         \
        } else {                                                                                   \
            val = bbuf >> (32 - (n));                                                              \
            bitpos += (n);                                                                         \
            bbuf <<= (n);                                                                          \
        }                                                                                          \
    } while (0)

#define MPVBIT_SKIP(n)                                                                             \
    do {                                                                                           \
        bitpos += (n);                                                                             \
        if (bitpos >= 32) {                                                                        \
            bitpos -= 32;                                                                          \
            bbuf = nbuf << bitpos;                                                                 \
            nbuf = *ptr++;                                                                         \
        } else {                                                                                   \
            bbuf <<= (n);                                                                          \
        }                                                                                          \
    } while (0)

#define MPVBIT_GET1(val)                                                                           \
    do {                                                                                           \
        val = bbuf >> 31;                                                                          \
        if (bitpos == 31) {                                                                        \
            bbuf = nbuf;                                                                           \
            nbuf = *ptr++;                                                                         \
            bitpos = 0;                                                                            \
        } else {                                                                                   \
            bbuf <<= 1;                                                                            \
            bitpos++;                                                                              \
        }                                                                                          \
    } while (0)

/* current byte position (rounded up to the next byte boundary) */
#define MPVBIT_BYTEPTR(q)                                                                          \
    do {                                                                                           \
        q = (Sint8 *)ptr;                                                                          \
        q += (bitpos + 7) >> 3;                                                                    \
        q -= 8;                                                                                    \
    } while (0)

#endif
