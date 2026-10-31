/* Sofdec SFX Z movie conversion (sfx_zmv.c): the Z (depth) component of an SFX frame is 8 bits;
 * the conversion table built here maps each value to a 16 or 32-bit Z buffer value through the
 * original Z range of the stream ("ZMFSIZE" and "ZMFDATA" tags) and the near and far clip planes
 * of the Z object. Also the Z object pool and the "ZMHDR" and "ZMVFRM" tags of the stream
 * information.
 *
 * Live bodies from the RE4 GameCube decompilation of the same file. The frame-format converters
 * of the file are not referenced in this game and were removed by the linker; only their messages
 * and constants remain in .rodata, and the stand-in below reproduces those in order.
 */
#include <stdio.h>
#include <string.h>

#include "CRI/sfx.h"
#include "CRI/sj.h"

#define SFXZ_MAX_HN 8
#define SFXZ_INVALID_Z 0x80000000
#define SFXZ_MAX_Z 0x7FFFFFFF

/* Z object */
struct SFXZ_OBJ {
    Sint32 used;                                                            /* 0x00 */
    Sint32 zbit;                                                            /* 0x04 output depth */
    Sint32 taginf_flg;                                                      /* 0x08 */
    Uint8 *tag_ptr;                                                         /* 0x0C */
    Sint32 tag_len;                                                         /* 0x10 */
    Sint32 field_0x14;                                                      /* 0x14 */
    Sint32 hdr_flg;                                                         /* 0x18 "ZMHDR" */
    Uint8 *hdr_ptr;                                                         /* 0x1C */
    Sint32 hdr_len;                                                         /* 0x20 */
    Sint32 field_0x24;                                                      /* 0x24 */
    Sint32 frm_flg;                                                         /* 0x28 "ZMVFRM" */
    Uint8 *frm_ptr;                                                         /* 0x2C */
    Sint32 frm_len;                                                         /* 0x30 */
    Sint32 field_0x34;                                                      /* 0x34 */
    Sint32 field_0x38;                                                      /* 0x38 */
    Float32 zmin;                                                           /* 0x3C near plane */
    Float32 zmax;                                                           /* 0x40 far plane */
    void (*cnvfunc)(Uint32 *orgtbl, void *tbl, Float32 zmin, Float32 zmax); /* 0x44 */
    Sint32 field_0x48;                                                      /* 0x48 */
};
CHECK_SIZEOF(SFXZ_OBJ, 0x4C);

typedef struct {
    Sint32 cnt;                /* 0x00 objects in use */
    Sint32 linear;             /* 0x04 linear Z mapping */
    Sint32 nobj;               /* 0x08 */
    SFXZ_OBJ obj[SFXZ_MAX_HN]; /* 0x0C */
} SFXZ_WORK;
CHECK_SIZEOF(SFXZ_WORK, 0x26C);

SFXZ_WORK sfxz_work;

void sfxzmv_MakeOrgZ32TblByCCIR(SFXZ_OBJ *sfxz, Uint32 zorg_min, Uint32 zorg_max, Uint32 *tbl);
void sfxzmv_MakeOrgZ32TblByDirect(SFXZ_OBJ *sfxz, Uint32 zorg_min, Uint32 zorg_max, Uint32 *tbl);

/* Matching workaround: order-only stand-in for the stripped frame-format converters
 * sfxcnv_CnvZbitToCft, sfxcnv_CnvFrmZcmn, SFX_CnvFrmZcmn and SFXZ_SetZclip; their bodies are
 * unknown. Their messages, the "COMPO" tag name and the 0.0 and -14.0 constants remain in .rodata
 * in this order. Storing the addresses and values keeps the references. Revisit when the bodies
 * of those functions are found. */
static void sfxzmv_RodataOrder(const Char8 **msg, Float32 *val) {
    msg[0] = "E201313: sfxcnv_CnvZbitToCft : zbit is invalid.";
    msg[1] = "COMPO";
    val[0] = 0.0f;
    msg[2] = "E201315: sfxcnv_CnvFrmZcmn : zclip is not set.";
    msg[3] = "E201191: SFX_CnvFrmZcmn : frmfmt is not support.";
    val[1] = -14.0f;
    msg[4] = "E201314: SFXZ_SetZclip : zclip is invalid.";
}

/* The 32-bit conversion table from the original Z values: the linear mapping keeps the upper
 * bits, the perspective mapping goes between the clip planes. Inlined into sfxzmv_MakeCnvZTbl. */
static void sfxzmv_MakeZ32Tbl(SFXZ_OBJ *sfxz, Uint32 *orgtbl, Uint32 *tbl) {
    Float64 zmin = sfxz->zmin;
    Float64 zmax = sfxz->zmax;

    if (sfxz_work.linear == 1) {
        Sint32 i;

        for (i = 0; i < 256; i++) {
            tbl[i] = orgtbl[i] & 0x7FFFFF80;
            tbl[i] <<= 1;
        }
    } else {
        Sint32 i;
        Uint32 *dst;
        Uint32 *src;
        Float64 rcp;
        Float64 a;
        Float64 b;

        src = orgtbl;
        dst = tbl;
        rcp = 1.0 / (zmax - zmin);
        a = 16777215.0 * zmax * rcp;
        b = zmax * (16777215.0 * rcp * zmin);
        for (i = 0; i < 256; i++) {
            if (*src == 0) {
                *src = 1;
            }
            *dst = (Uint32)(a - b / (zmax * (Float64)*src / 2147483647.0));
            src++;
            dst++;
        }
    }
}

/* The 16-bit conversion table. Inlined into sfxzmv_MakeCnvZTbl. */
static void sfxzmv_MakeZ16Tbl(SFXZ_OBJ *sfxz, Uint32 *orgtbl, Uint16 *tbl) {
    Float64 zmin = sfxz->zmin;
    Float64 zmax = sfxz->zmax;

    if (sfxz_work.linear == 1) {
        Sint32 i;

        for (i = 0; i < 256; i++) {
            tbl[i] = (Uint16)(orgtbl[i] >> 15);
        }
    } else {
        Sint32 i;
        Uint16 *dst;
        Uint32 *src;
        Float64 rcp;
        Float64 a;
        Float64 b;

        src = orgtbl;
        dst = tbl;
        rcp = 1.0 / (zmax - zmin);
        a = 65535.0 * zmax * rcp;
        b = zmax * (65535.0 * rcp * zmin);
        for (i = 0; i < 256; i++) {
            if (*src == 0) {
                *src = 1;
            }
            *dst++ = (Uint16)(a - b / (zmax * (Float64)*src / 2147483647.0));
            src++;
        }
    }
}

/* The 256-entry conversion table at tbl (Uint16 or Uint32 by zbit) from the original 32-bit Z
 * values built at tbl + 0x400, or by the object's own table maker. */
void sfxzmv_MakeCnvZTbl(SFXZ_OBJ *sfxz, Uint32 zorg_min, Uint32 zorg_max, void *tbl) {
    Uint32 *orgtbl;

    orgtbl = (Uint32 *)((Uint8 *)tbl + 0x400);
    memset(tbl, 0, 0x400);
    if (SFX_GetCcirFx() == 1) {
        sfxzmv_MakeOrgZ32TblByCCIR(sfxz, zorg_min, zorg_max, orgtbl);
    } else {
        sfxzmv_MakeOrgZ32TblByDirect(sfxz, zorg_min, zorg_max, orgtbl);
    }
    if (sfxz->cnvfunc == NULL) {
        if (sfxz->zbit == 16) {
            sfxzmv_MakeZ16Tbl(sfxz, orgtbl, (Uint16 *)tbl);
        } else {
            sfxzmv_MakeZ32Tbl(sfxz, orgtbl, (Uint32 *)tbl);
        }
    } else {
        sfxz->cnvfunc(orgtbl, tbl, sfxz->zmin, sfxz->zmax);
    }
}

/* Builds the Z conversion table for frame index frmno: the original Z range (the second and third
 * fields of the frame's "ZMFDATA" entry, which is frmno times the "ZMFSIZE" entry size into that
 * tag) comes from the stream information. When "ZMFSIZE" is present but "ZMFDATA" is not,
 * zorg_max is passed on uninitialised, as in the original.
 *
 * Matching workaround: sfxzmv_MakeCnvZTbl is called out of line in the original, and dont_inline
 * keeps it out of line here. Without the pragma MWCC inlines it into SFXZ_MakeCnvZTbl, which
 * grows from 78 to 347 words. Revisit when the original's inline setting, or a reason the call
 * stays out of line, is found. */
#pragma dont_inline on
void SFXZ_MakeCnvZTbl(SFXZ_OBJ *sfxz, Sint32 frmno, void *tbl) {
    SJCK out1;
    SJCK inf1;
    SJCK out2;
    SJCK inf2;
    Uint32 zmf_c;
    Uint32 zmf_b;
    Uint32 zmf_a;
    Uint32 n;
    Char8 *p;
    Uint32 zorg_min;
    Uint32 zorg_max;

    if (sfxz->frm_flg != 1 || sfxz->frm_ptr == NULL) {
        p = NULL;
    } else {
        inf1.data = sfxz->frm_ptr;
        inf1.len = sfxz->frm_len;
        SJ_SearchTag(&inf1, "ZMFSIZE", "SFXINFE", &out1);
        p = (Char8 *)out1.data;
    }
    if (p == NULL) {
        zorg_min = 0;
        zorg_max = SFXZ_MAX_Z;
    } else {
        sscanf(p, "%lx", &n);
        if (sfxz->frm_flg != 1 || sfxz->frm_ptr == NULL) {
            p = NULL;
        } else {
            inf2.data = sfxz->frm_ptr;
            inf2.len = sfxz->frm_len;
            SJ_SearchTag(&inf2, "ZMFDATA", "SFXINFE", &out2);
            p = (Char8 *)out2.data;
        }
        if (p == NULL) {
            zorg_min = SFXZ_MAX_Z;
        } else {
            sscanf(p + n * frmno, "%lx %lx %lx", &zmf_a, &zmf_b, &zmf_c);
            zorg_min = zmf_b;
            zorg_max = zmf_c;
        }
    }
    sfxzmv_MakeCnvZTbl(sfxz, zorg_min, zorg_max, tbl);
}
#pragma dont_inline off

/* The 32-bit Z of each of the 256 Z index values: 0 below 9, zorg_min up to 16, a ramp from
 * zorg_min to zorg_max up to 0xDF, zorg_max up to 0xEF and the maximum above.
 *
 * Matching workaround: a statement macro instead of a helper function. As a static function the
 * helper stays out of line (200 words) and its two callers shrink from 444 and 200 words to 267
 * and 11; as a static inline function the callers are 443 and 199 words with 330 and 183 aligned
 * words differing. Revisit when the original's helper, or the inline setting that reproduces
 * both callers, is found. */
#define SFXZ_MAKE_ORG_Z32_TBL(zorg_min, zorg_max, tbl)                                             \
    {                                                                                              \
        Sint32 i;                                                                                  \
        Uint32 *p;                                                                                 \
        if ((zorg_max) == SFXZ_INVALID_Z) {                                                        \
            (zorg_max) = SFXZ_MAX_Z;                                                               \
        }                                                                                          \
        if ((zorg_min) == SFXZ_INVALID_Z) {                                                        \
            (zorg_min) = SFXZ_MAX_Z;                                                               \
        }                                                                                          \
        p = (tbl);                                                                                 \
        for (i = 0; i < 9; i++) {                                                                  \
            *p++ = 0;                                                                              \
        }                                                                                          \
        for (i = 9; i < 17; i++) {                                                                 \
            (tbl)[i] = (zorg_min);                                                                 \
        }                                                                                          \
        if ((zorg_min) == (zorg_max)) {                                                            \
            for (i = 17; i < 0xE0; i++) {                                                          \
                (tbl)[i] = (zorg_min);                                                             \
            }                                                                                      \
        } else {                                                                                   \
            for (i = 17; i < 0xE0; i++) {                                                          \
                (tbl)[i] = (zorg_min) + (i - 17) * (((zorg_max) - (zorg_min)) / (0xE0 - 17));      \
            }                                                                                      \
        }                                                                                          \
        for (i = 0xE0; i < 0xF0; i++) {                                                            \
            (tbl)[i] = (zorg_max);                                                                 \
        }                                                                                          \
        for (i = 0xF0; i < 0x100; i++) {                                                           \
            (tbl)[i] = SFXZ_MAX_Z;                                                                 \
        }                                                                                          \
    }

/* The 32-bit Z of each 8-bit Z sample: the CCIR601 luma range (16 to 235) is stretched to 0 to
 * 255 first, then the index maps to the original range. Each loop has its own counter. */
void sfxzmv_MakeOrgZ32TblByCCIR(SFXZ_OBJ *sfxz, Uint32 zorg_min, Uint32 zorg_max, Uint32 *tbl) {
    Sint32 i, j, k, l;
    Uint32 *d;
    Uint8 *ytbl;
    Uint32 *ztbl;

    ztbl = tbl + 0x100;
    ytbl = (Uint8 *)(ztbl + 0x100);
    for (j = 0; j <= 0x0F; j++) {
        ytbl[j] = 0;
    }
    for (i = 0x10; i <= 0xEB; i++) {
        ytbl[i] = 1.164f * (Float32)(i - 16);
    }
    for (k = 0xEC; k <= 0xFF; k++) {
        ytbl[k] = 0xFF;
    }
    SFXZ_MAKE_ORG_Z32_TBL(zorg_min, zorg_max, ztbl);
    d = tbl;
    for (l = 0; l <= 0xFF; l++) {
        *d++ = ztbl[ytbl[l]];
    }
}

/* The same without the luma range correction. */
void sfxzmv_MakeOrgZ32TblByDirect(SFXZ_OBJ *sfxz, Uint32 zorg_min, Uint32 zorg_max, Uint32 *tbl) {
    SFXZ_MAKE_ORG_Z32_TBL(zorg_min, zorg_max, tbl);
}

/* Stores the stream's tag information and searches it for the "ZMHDR" and "ZMVFRM" sub-tags. */
void SFXZ_SetTagInf(SFXZ_OBJ *sfxz, Uint8 *tag_ptr, Sint32 tag_len) {
    SJCK out;
    SJCK inf;

    sfxz->taginf_flg = 1;
    sfxz->tag_ptr = tag_ptr;
    sfxz->tag_len = tag_len;
    if (sfxz->tag_ptr == NULL) {
        sfxz->hdr_flg = 1;
        sfxz->hdr_ptr = NULL;
        sfxz->hdr_len = 0;
        sfxz->frm_flg = 1;
        sfxz->frm_ptr = NULL;
        sfxz->frm_len = 0;
    } else {
        inf.data = sfxz->tag_ptr;
        inf.len = sfxz->tag_len;
        SJ_SearchTag(&inf, "ZMHDR", "SFXINFE", &out);
        sfxz->hdr_flg = 1;
        sfxz->hdr_ptr = out.data;
        sfxz->hdr_len = out.len;
        SJ_SearchTag(&inf, "ZMVFRM", "SFXINFE", &out);
        sfxz->frm_flg = 1;
        sfxz->frm_ptr = out.data;
        sfxz->frm_len = out.len;
    }
}

/* Releases the object. */
void SFXZ_Destroy(SFXZ_OBJ *sfxz) {
    if (sfxz == NULL) {
        return;
    }
    sfxz->used = 0;
    sfxz_work.cnt--;
}

/* First unused Z object, or NULL. Inlined into SFXZ_Create. */
static SFXZ_OBJ *sfxz_search_free(void) {
    SFXZ_OBJ *sfxz;
    Sint32 i;

    sfxz = sfxz_work.obj;
    for (i = 0; i < sfxz_work.nobj; i++) {
        if (sfxz->used == 0) {
            return sfxz;
        }
        sfxz++;
    }
    return NULL;
}

/* Takes a Z object with an empty clip range and no table maker. */
SFXZ_OBJ *SFXZ_Create(void) {
    SFXZ_OBJ *sfxz;

    sfxz = sfxz_search_free();
    if (sfxz == NULL) {
        return sfxz;
    }
    sfxz->zmin = 0.0f;
    sfxz->zmax = 0.0f;
    sfxz->cnvfunc = NULL;
    sfxz->field_0x48 = 0;
    sfxz->zbit = 0;
    sfxz_work.cnt++;
    sfxz->used = 1;
    return sfxz;
}

void SFXZ_Finish(void) {}

/* Clears the Z object pool. */
void SFXZ_Init(void) {
    memset(&sfxz_work, 0, sizeof(sfxz_work));
    sfxz_work.nobj = SFXZ_MAX_HN;
    sfxz_work.linear = 0;
}
