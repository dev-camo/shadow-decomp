/* CRI SUD (Sofdec picture user data) analyser (sud_lib.c, "CRI SUD/GC Ver.0.05"): finds the
 * "<SUDPS_>" record the Sofdec encoder writes into each picture's user data and checks two of its
 * bytes (byte 0x12 == 'D', byte 0x13 == 'C').
 * Bodies from the RE4 GameCube decompilation of the same file; this version adds SUD_Finish.
 */
#include <string.h>

#include "CRI/sud.h"

/* Length of a "<SUDPS_>" record. */
#define SUD_DAT_SIZE 0x23

static const Char8 sud_ver_str[] = "\nCRI SUD/GC Ver.0.05 Build:Feb 28 2005 19:36:35\n";

Sint32 sud_init_cnt;
static const Char8 *sud_dummy;

/* the letters compared at bytes 0x12 and 0x13 (names inferred from the functions using them) */
static const Char8 sud_divfld[] = "D";
static const Char8 sud_ccs[] = "C";

/* Finds the last "<SUDPS_>" record in the picture user data; *sud points at it and *sudsize is
 * SUD_DAT_SIZE, or NULL and 0 when there is none.
 */
void SUD_SearchSudDat(Uint8 *dat, Sint32 size, void **sud, Sint32 *sudsize) {
    Sint32 i;
    Uint8 *p = dat;

    *sud = 0;
    *sudsize = 0;
    if (dat == 0 || size <= 0) {
        return;
    }
    for (i = 0; i < size; p++, i++) {
        if (memcmp(p, "<", 1) == 0) {
            if (memcmp(p, "<SUDPS_>", 8) == 0) {
                *sud = p;
                *sudsize = (*sud == 0) ? 0 : SUD_DAT_SIZE;
            }
        }
    }
}

/* Byte 0x13 of the record is 'C'. */
Bool SUD_AnalyTypeCcs(Uint8 *dat, Sint32 size) {
    if (dat == 0 || size < 0) {
        return FALSE;
    }
    return strncmp((Char8 *)dat + 0x13, sud_ccs, 1) == 0;
}

/* Byte 0x12 of the record is 'D'. */
Bool SUD_AnalyTypeDivField(Uint8 *dat, Sint32 size) {
    if (dat == 0 || size < 0) {
        return FALSE;
    }
    return strncmp((Char8 *)dat + 0x12, sud_divfld, 1) == 0;
}

/* Releases one SUD_Init. */
void SUD_Finish(void) {
    if (sud_init_cnt > 0) {
        sud_init_cnt--;
    }
}

/* Module init (once); keeps the version string referenced. */
void SUD_Init(void) {
    if (sud_init_cnt < 1) {
        sud_dummy = sud_ver_str;
        sud_init_cnt++;
    }
}
