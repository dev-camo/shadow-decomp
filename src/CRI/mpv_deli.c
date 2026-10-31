/* MPEG video start-code (delimiter) search
 * Bodies from the RE4 GameCube decompilation of the same file.
 */
#include "CRI/cri_xpt.h"
#include "CRI/mpv.h"

/* Byte-state-machine scan for the first 00 00 01 xx start code in [p, p+n) whose class
 * (MPV_CheckDelim) is in `mask`; returns its address or 0. */
Sint8 *MPV_SearchDelim(Sint8 *p, Sint32 n, Sint32 mask) {
    Sint8 *end;
    Sint8 c;
    Sint32 st;

    end = p + n;
    st = 0;
    while (p < end) {
        c = *p++;
        switch (st) {
        case 0:
            if (c == 0) {
                st = 1;
            }
            break;
        case 1:
            if (c == 0) {
                st = 2;
            } else {
                st = 0;
            }
            break;
        case 2:
            if (c == 1) {
                st = 3;
            } else if (c != 0) {
                st = 0;
            }
            break;
        case 3:
            if (MPV_CheckDelim(p - 4) & mask) {
                return p - 4;
            }
            st = 0;
            break;
        }
    }
    return 0;
}

/* The same scan backwards from p over n bytes (used to find the start of the current picture). */
Sint8 *MPV_BsearchDelim(Sint8 *p, Sint32 n, Sint32 mask) {
    Sint8 *end;
    Sint8 c;
    Sint32 st;

    end = p - n;
    st = 0;
    while (end < p) {
        c = *--p;
        switch (st) {
        case 0:
            st = 1;
            break;
        case 1:
            if (c == 1) {
                st = 2;
            }
            break;
        case 2:
            if (c == 0) {
                st = 3;
            } else if (c != 1) {
                st = 1;
            }
            break;
        case 3:
            if (c == 0) {
                if (MPV_CheckDelim(p) & mask) {
                    return p;
                }
                st = 0;
            } else if (c != 1) {
                st = 1;
            } else {
                st = 2;
            }
            break;
        }
    }
    return 0;
}

/* Start-code class (MPV_DLM_*) of the 4 bytes at ptr: picture (0x00), first slice (0x01), other
 * slices (0x02..0xAF), user data (0xB2), sequence header (0xB3), extension (0xB5), sequence end
 * (0xB7), GOP (0xB8); 0 for anything else. */
Sint32 MPV_CheckDelim(void *ptr) {
    Uint8 *p = ptr;
    Sint32 code;
    Sint32 ret;

    code = (p[0] << 8) | p[1];
    code <<= 8;
    code |= p[2];
    code <<= 8;
    code |= p[3];
    if (code == 0x100) {
        ret = MPV_DLM_PIC;
    } else if (code == 0x101) {
        ret = MPV_DLM_SLICE | MPV_DLM_SLICE1;
    } else if (code > 0x101 && code <= 0x1AF) {
        ret = MPV_DLM_SLICE;
    } else if (code == 0x1B2) {
        ret = MPV_DLM_UD;
    } else if (code == 0x1B3) {
        ret = MPV_DLM_SEQ;
    } else if (code == 0x1B5) {
        ret = MPV_DLM_EXT;
    } else if (code == 0x1B7) {
        ret = MPV_DLM_END;
    } else if (code == 0x1B8) {
        ret = MPV_DLM_GOP;
    } else {
        ret = 0;
    }
    return ret;
}
