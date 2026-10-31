/* CRI Sofdec file header analyser (sfh_main.c): validates the 0x800-byte SofdecStream header and
 * reads the pack/element/feature fields out of it.
 *
 * NonMatching: the seven little-endian 32-bit readers (SFH_AnlyElemSmpHz, SFH_AnlyMaxFrmNum,
 * SFH_AnlyMaxPlyLenVid, SFH_AnlyMaxPlyLenAud, SFH_AnlyPackSiz, SFH_AnlyHdrSiz and
 * SFH_AnlyByteRate) store the byte-swapped word with one `stwbrx`, where the original swaps it
 * with a `rlwinm`/`rlwimi` chain and stores it with a plain `stw`. The readers are 83, 35 and 40
 * words long against the original's 87, 39 and 44; no natural source shape is known that
 * reproduces the original (see SFH_AnlyElemSmpHz). */
#include <string.h>

#include "CRI/cri_xpt.h"
#include "CRI/sfh.h"
#include "layout_check.h"

const Char8 SFH_sbver_str[] = "\nCRI SFH/GC Ver.1.19 Build:Feb 28 2005 19:36:31\n";

/* handle states */
#define SFH_STAT_INVALID (-1)
#define SFH_STAT_FREE 0
#define SFH_STAT_CREATED 1
#define SFH_STAT_VALID 2

#define SFH_HDR_MIN_SIZE 0x800
#define SFH_HDR_ID_OFS 0x20
#define SFH_HDR_ID_LEN 0x18
#define SFH_HDR_VER_MAJOR_OFS 0x38
#define SFH_HDR_VER_MINOR_OFS 0x39
#define SFH_HDR_TOOLSTR_OFS 0x60
#define SFH_HDR_TOOLSTR_LEN 0x20

#define SFH_HDR_SIZ_OFS 0x80
#define SFH_HDR_PACKTYPE_OFS 0x84
#define SFH_HDR_PKETSIZLEN_OFS 0x88
#define SFH_HDR_PACKSIZ_OFS 0x8C
#define SFH_HDR_NUMELEM_TOT_OFS 0xB0
#define SFH_HDR_NUMELEM_AUD_OFS 0xB1
#define SFH_HDR_NUMELEM_VID_OFS 0xB2
#define SFH_HDR_NUMELEM_PRV_OFS 0xB3
#define SFH_HDR_BYTERATE_OFS 0xB4
#define SFH_HDR_MAXPLYLEN_AUD_OFS 0xB8
#define SFH_HDR_MAXPLYLEN_VID_OFS 0xBC
#define SFH_HDR_MAXFRMNUM_OFS 0xC0
#define SFH_HDR_ELEM_OFS 0x180
#define SFH_HDR_ELEM_NUM 26
#define SFH_ELEM_SIZE 0x40

/* stream types */
#define SFH_STM_AUD 0xC0
#define SFH_STM_VID 0xE0
#define SFH_STM_PRV 0xBD

/* header tool versions (major * 100 + minor) */
#define SFH_VER_107 107
#define SFH_VER_110 110
#define SFH_VER_210 210

typedef struct SFH_OBJ {
    Sint32 stat; /* 0x00 */
    Uint8 *hdr;  /* 0x04 */
    Sint32 size; /* 0x08 */
    Sint32 ver;  /* 0x0C */
} SFH_OBJ;
CHECK_SIZEOF(SFH_OBJ, 0x10);

/* one stream element record (0x40 bytes); bytes 0x1A..0x1F are read as audio or video fields
 * depending on the stream type: the 16-bit bit rate and the 32-bit sampling rate are
 * little-endian, the picture size packs two 12-bit values high nibble first */
typedef struct {
    Uint8 unk00[0x18]; /* 0x00 */
    Uint8 id;          /* 0x18 */
    Uint8 codec;       /* 0x19 */
    union {
        struct {
            Uint8 layer; /* 0x1A */
            Uint8 chnum; /* 0x1B */
        } aud;
        struct {
            Uint16 bitrate; /* 0x1A */
        } vid;
    } fld1A;
    union {
        struct {
            Uint32 smphz; /* 0x1C */
        } aud;
        struct {
            Uint8 picsz[3]; /* 0x1C */
            Uint8 picrate;  /* 0x1F */
        } vid;
    } fld1C;
    Uint8 ftr_flag;    /* 0x20 */
    Uint8 ftr_coltype; /* 0x21 */
    Uint8 ftr_pictype; /* 0x22 */
    Uint8 ftr_fixflg;  /* 0x23 */
    Uint8 ftr_expand;  /* 0x24 */
    Uint8 ftr_gopn;    /* 0x25 */
    Uint8 ftr_gopm;    /* 0x26 */
    Uint8 ftr_fxtype;  /* 0x27 */
    Uint8 unk28[0x18]; /* 0x28 */
} SFH_ELEM;
CHECK_SIZEOF(SFH_ELEM, 0x40);

typedef struct {
    Sint32 num;  /* 0x00 */
    Sint32 used; /* 0x04 */
    SFH_OBJ *hn; /* 0x08 */
} SFH_OBJINF;
CHECK_SIZEOF(SFH_OBJINF, 0xC);

static Sint32 sfh_init_cont = 0;
static SFH_OBJINF sfh_objinf;
static const Char8 *sfhlib_version_dummy;

#define SWAP16(x) ((Uint16)((((x) & 0xFF) << 8) | (((x) >> 8) & 0xFF)))
#define SWAP32(x)                                                                                  \
    (((x) << 24) | (((x) << 8) & 0x00FF0000) | (((x) >> 8) & 0x0000FF00) | (((x) >> 24) & 0xFF))

/* Picture rate code (1..8) -> frame rate x1000 (23976 .. 60000); 0 for unknown. Not an
 * exported symbol of the original (its bytes exist only inlined into SFH_AnlyElemPicRate), so
 * the name is inferred. */
static inline Sint32 getPicRate(Uint32 code) {
    switch (code) {
    case 1:
        return 23976;
    case 2:
        return 24000;
    case 3:
        return 25000;
    case 4:
        return 29970;
    case 5:
        return 30000;
    case 6:
        return 50000;
    case 7:
        return 59940;
    case 8:
        return 60000;
    default:
        return 0;
    }
}

/* The sfh_* static helpers below are named from what they do; their original names are not
 * known. */

/* The handle has a validated header (SFH_IsSfdHeader succeeded). */
static Bool sfh_IsValid(SFH sfh) {
    Bool ret;

    switch (sfh->stat) {
    case SFH_STAT_INVALID:
    case SFH_STAT_FREE:
    case SFH_STAT_CREATED:
        ret = FALSE;
        break;
    default:
        ret = TRUE;
        break;
    }
    return ret;
}

/* The handle slot is unused. */
static Bool sfh_IsFree(SFH sfh) {
    return (sfh->stat == SFH_STAT_FREE);
}

/* Header tool version 1.07 or >= 1.10 can be analysed. */
static Bool sfh_IsVerOk(SFH sfh) {
    return (sfh->ver == SFH_VER_107 || sfh->ver >= SFH_VER_110);
}

/* Valid handle with an analysable version. */
static Bool sfh_IsAnlyOk(SFH sfh) {
    if (!sfh_IsValid(sfh)) {
        return FALSE;
    }
    if (!sfh_IsVerOk(sfh)) {
        return FALSE;
    }
    return TRUE;
}

/* The 0x40-byte element record of stream `id` among the 26 at offset 0x180, 0 if absent. */
static SFH_ELEM *sfh_SearchElem(Uint8 *hdr, Uint32 id) {
    Sint32 i;
    SFH_ELEM *p;
    SFH_ELEM *elem = 0;

    for (i = 0; i < SFH_HDR_ELEM_NUM; i++) {
        p = (SFH_ELEM *)(hdr + SFH_HDR_ELEM_OFS + i * SFH_ELEM_SIZE);
        if (p->id == id) {
            elem = p;
            break;
        }
    }
    return elem;
}

/* Element record of stream `id` after the handle checks. */
static SFH_ELEM *sfh_GetElem(SFH sfh, Uint32 id) {
    Uint8 *hdr = sfh->hdr;

    if (!sfh_IsAnlyOk(sfh)) {
        return 0;
    }
    return sfh_SearchElem(hdr, id);
}

/* Stream id class: 0xC0..0xDF audio, 0xE0..0xEF video, 0xBD/0xBF private, else 0. */
static Uint32 sfh_GetStmType(Uint8 id) {
    Uint32 type = id;

    if (type >= 0xC0 && type <= 0xDF) {
        type = SFH_STM_AUD;
    } else if (type >= 0xE0 && type <= 0xEF) {
        type = SFH_STM_VID;
    } else if (type == 0xBD || type == 0xBF) {
        type = SFH_STM_PRV;
    } else {
        type = 0;
    }
    return type;
}

/* The element is of the expected class and its feature block is present (flag == 1). */
static Bool sfh_IsEffFtr(SFH_ELEM *elem, Uint8 id, Uint32 type) {
    Uint32 flg;

    if (sfh_GetStmType(id) != type) {
        return FALSE;
    }
    flg = elem->ftr_flag;
    if (flg > 1) {
        return FALSE;
    } else if (flg == 0) {
        return FALSE;
    }
    return TRUE;
}

/* ASCII digit test. */
static Bool sfh_IsDigit(Sint32 c) {
    return (c >= '0' && c <= '9');
}

/* Copies the 32-character muxer tool string (offset 0x60) into a NUL-terminated buffer. */
static Bool sfh_GetToolStr(SFH sfh, Uint8 *str, Char8 *buf) {
    if (!sfh_IsValid(sfh)) {
        return FALSE;
    }
    memset(buf, 0, SFH_HDR_TOOLSTR_LEN + 1);
    memcpy(buf, str, SFH_HDR_TOOLSTR_LEN);
    return TRUE;
}

/* Parses a decimal number up to '.', ' ' or NUL, advancing the pointer. */
static Sint32 sfh_Atoi(const Char8 **pp) {
    const Char8 *p = *pp;
    Sint32 c;
    Sint32 num = 0;

    for (;;) {
        c = *p;
        if (c == '.' || c == ' ' || c == '\0') {
            break;
        }
        if (!sfh_IsDigit(c)) {
            break;
        }
        num = num * 10 + (c - '0');
        p++;
    }
    *pp = p;
    return num;
}

/* "Ver.M.m" out of the tool string. */
static Bool sfh_GetStrVer(const Char8 *buf, Sint32 *major, Sint32 *minor) {
    const Char8 *p;

    p = strstr(buf, "Ver.");
    if (p == 0) {
        return FALSE;
    }
    p += 4;
    *major = sfh_Atoi(&p);
    p++;
    *minor = sfh_Atoi(&p);
    return TRUE;
}

/* feature (ftr) fields of a video element */

Bool SFH_AnlyFtrFxType(SFH sfh, Uint8 id, Sint32 *val) {
    SFH_ELEM *elem;

    *val = -1;
    elem = sfh_GetElem(sfh, id);
    if (elem == 0) {
        return FALSE;
    }
    if (!sfh_IsEffFtr(elem, id, SFH_STM_VID)) {
        return FALSE;
    }
    if (sfh->ver < SFH_VER_210) {
        return FALSE;
    }
    *val = elem->ftr_fxtype;
    return TRUE;
}

/* Video feature: GOP M (P-picture spacing). */
Bool SFH_AnlyFtrGopM(SFH sfh, Uint8 id, Sint32 *val) {
    SFH_ELEM *elem;

    *val = -1;
    elem = sfh_GetElem(sfh, id);
    if (elem == 0) {
        return FALSE;
    }
    if (!sfh_IsEffFtr(elem, id, SFH_STM_VID)) {
        return FALSE;
    }
    *val = elem->ftr_gopm;
    if (*val > 0x3F) {
        *val = -1;
    }
    return TRUE;
}

/* Video feature: GOP N (GOP length). */
Bool SFH_AnlyFtrGopN(SFH sfh, Uint8 id, Sint32 *val) {
    SFH_ELEM *elem;

    *val = -1;
    elem = sfh_GetElem(sfh, id);
    if (elem == 0) {
        return FALSE;
    }
    if (!sfh_IsEffFtr(elem, id, SFH_STM_VID)) {
        return FALSE;
    }
    *val = elem->ftr_gopn;
    if (*val > 0x3F) {
        *val = -1;
    }
    return TRUE;
}

/* Video feature: expand flag. */
Bool SFH_AnlyFtrExpand(SFH sfh, Uint8 id, Sint32 *val) {
    SFH_ELEM *elem;

    *val = 0;
    elem = sfh_GetElem(sfh, id);
    if (elem == 0) {
        return FALSE;
    }
    if (!sfh_IsEffFtr(elem, id, SFH_STM_VID)) {
        return FALSE;
    }
    *val = elem->ftr_expand;
    return TRUE;
}

/* Video feature: sequence-header-code fixed flag. */
Bool SFH_AnlyFtrShcFixFlg(SFH sfh, Uint8 id, Sint32 *val) {
    SFH_ELEM *elem;

    *val = 0;
    elem = sfh_GetElem(sfh, id);
    if (elem == 0) {
        return FALSE;
    }
    if (!sfh_IsEffFtr(elem, id, SFH_STM_VID)) {
        return FALSE;
    }
    *val = (elem->ftr_fixflg >> 4) & 1;
    return TRUE;
}

/* Video feature: fixed flag. */
Bool SFH_AnlyFtrFixFlg(SFH sfh, Uint8 id, Sint32 *val) {
    SFH_ELEM *elem;

    *val = 0;
    elem = sfh_GetElem(sfh, id);
    if (elem == 0) {
        return FALSE;
    }
    if (!sfh_IsEffFtr(elem, id, SFH_STM_VID)) {
        return FALSE;
    }
    *val = elem->ftr_fixflg & 1;
    return TRUE;
}

/* Video feature: picture type restriction. */
Bool SFH_AnlyFtrPicType(SFH sfh, Uint8 id, Sint32 *val) {
    SFH_ELEM *elem;

    *val = -1;
    elem = sfh_GetElem(sfh, id);
    if (elem == 0) {
        return FALSE;
    }
    if (!sfh_IsEffFtr(elem, id, SFH_STM_VID)) {
        return FALSE;
    }
    *val = elem->ftr_pictype;
    return TRUE;
}

/* Video feature: colour type (3 = colour-space conversion needed; drives the SFX colour
 * adjustment). */
Bool SFH_AnlyFtrColType(SFH sfh, Uint8 id, Sint32 *val) {
    SFH_ELEM *elem;

    *val = -1;
    elem = sfh_GetElem(sfh, id);
    if (elem == 0) {
        return FALSE;
    }
    if (!sfh_IsEffFtr(elem, id, SFH_STM_VID)) {
        return FALSE;
    }
    *val = elem->ftr_coltype;
    return TRUE;
}

/* element fields */

Bool SFH_AnlyElemPicRate(SFH sfh, Uint8 id, Sint32 *val) {
    SFH_ELEM *elem;

    *val = 0;
    elem = sfh_GetElem(sfh, id);
    if (elem == 0) {
        return FALSE;
    }
    if (sfh_GetStmType(id) != SFH_STM_VID) {
        return FALSE;
    }
    *val = getPicRate(elem->fld1C.vid.picrate);
    return TRUE;
}

/* Video element: picture width and height (two 12-bit fields packed in three bytes). */
Bool SFH_AnlyElemPicSz(SFH sfh, Uint8 id, Sint32 *width, Sint32 *height) {
    SFH_ELEM *elem;

    *width = 0;
    *height = 0;
    elem = sfh_GetElem(sfh, id);
    if (elem == 0) {
        return FALSE;
    }
    if (sfh_GetStmType(id) != SFH_STM_VID) {
        return FALSE;
    }
    *width = elem->fld1C.vid.picsz[0];
    *width = (*width << 4) | ((elem->fld1C.vid.picsz[1] >> 4) & 0x0F);
    *width &= 0xFFF;
    *height = elem->fld1C.vid.picsz[1];
    *height = (*height << 8) | elem->fld1C.vid.picsz[2];
    *height &= 0xFFF;
    return TRUE;
}

/* Video element: bit rate (little-endian 16-bit). */
Bool SFH_AnlyElemBitRate(SFH sfh, Uint8 id, Sint32 *val) {
    SFH_ELEM *elem;
    Sint32 rate;

    *val = 0;
    elem = sfh_GetElem(sfh, id);
    if (elem == 0) {
        return FALSE;
    }
    if (sfh_GetStmType(id) != SFH_STM_VID) {
        return FALSE;
    }
    /* The swapped value is converted to Sint16 before widening, so it is sign-extended and the
     * "unknown" test against 0xFFFF below can never succeed. The original compiles the same dead
     * test (extsh, then cmplwi against 0xFFFF). The high byte is taken with a (Uint8) cast: the
     * `& 0xFF` mask of SWAP16 changes the object. */
    rate = (Sint16)(Uint16)(((elem->fld1A.vid.bitrate & 0xFF) << 8) |
                            (Uint8)(elem->fld1A.vid.bitrate >> 8));
    if (rate == 0xFFFF) {
        rate = 0;
    }
    *val = rate;
    return TRUE;
}

/* Video element: codec id. */
Bool SFH_AnlyElemCodecVid(SFH sfh, Uint8 id, Sint32 *val) {
    SFH_ELEM *elem;

    *val = -1;
    elem = sfh_GetElem(sfh, id);
    if (elem == 0) {
        return FALSE;
    }
    if (sfh_GetStmType(id) != SFH_STM_VID) {
        return FALSE;
    }
    *val = elem->codec;
    return TRUE;
}

/* Audio element: sampling rate (little-endian 32-bit). */
/* The original stores the swapped word with `stw` after a `rlwinm`/`rlwimi` chain; the compiler
 * folds this swap and store into `stwbrx`, and no natural form found keeps the `stw`. */
Bool SFH_AnlyElemSmpHz(SFH sfh, Uint8 id, Sint32 *val) {
    SFH_ELEM *elem;

    *val = 0;
    elem = sfh_GetElem(sfh, id);
    if (elem == 0) {
        return FALSE;
    }
    if (sfh_GetStmType(id) != SFH_STM_AUD) {
        return FALSE;
    }
    *val = SWAP32(elem->fld1C.aud.smphz);
    return TRUE;
}

/* Audio element: channel count. */
Bool SFH_AnlyElemChNum(SFH sfh, Uint8 id, Sint32 *val) {
    SFH_ELEM *elem;

    *val = 0;
    elem = sfh_GetElem(sfh, id);
    if (elem == 0) {
        return FALSE;
    }
    if (sfh_GetStmType(id) != SFH_STM_AUD) {
        return FALSE;
    }
    *val = elem->fld1A.aud.chnum;
    return TRUE;
}

/* Audio element: layer. */
Bool SFH_AnlyElemLayer(SFH sfh, Uint8 id, Sint32 *val) {
    SFH_ELEM *elem;

    *val = 0;
    elem = sfh_GetElem(sfh, id);
    if (elem == 0) {
        return FALSE;
    }
    if (sfh_GetStmType(id) != SFH_STM_AUD) {
        return FALSE;
    }
    if (elem->codec != 1) {
        return FALSE;
    }
    *val = elem->fld1A.aud.layer;
    return TRUE;
}

/* Audio element: codec id. */
Bool SFH_AnlyElemCodecAud(SFH sfh, Uint8 id, Sint32 *val) {
    SFH_ELEM *elem;

    *val = -1;
    elem = sfh_GetElem(sfh, id);
    if (elem == 0) {
        return FALSE;
    }
    if (sfh_GetStmType(id) != SFH_STM_AUD) {
        return FALSE;
    }
    *val = elem->codec;
    return TRUE;
}

/* Little-endian 32-bit header field after the handle checks (same residual as
 * SFH_AnlyElemSmpHz). */
static Bool sfh_GetHdrU32(SFH sfh, Sint32 ofs, Sint32 *val) {
    Uint8 *hdr = sfh->hdr;

    if (!sfh_IsAnlyOk(sfh)) {
        return FALSE;
    }
    *val = SWAP32(*(Uint32 *)(hdr + ofs));
    return TRUE;
}

/* Little-endian 16-bit header field after the handle checks. */
static Bool sfh_GetHdrS16(SFH sfh, Sint32 ofs, Sint32 *val) {
    Uint8 *hdr = sfh->hdr;

    if (!sfh_IsAnlyOk(sfh)) {
        return FALSE;
    }
    *val = (Sint16)SWAP16(*(Sint16 *)(hdr + ofs));
    return TRUE;
}

/* Byte header field after the handle checks. */
static Bool sfh_GetHdrU8(SFH sfh, Sint32 ofs, Sint32 *val) {
    Uint8 *hdr = sfh->hdr;

    if (!sfh_IsAnlyOk(sfh)) {
        return FALSE;
    }
    *val = hdr[ofs];
    return TRUE;
}

/* Little-endian 32-bit header field, only for tool versions >= `ver` (same residual as
 * SFH_AnlyElemSmpHz). */
static Bool sfh_GetHdrU32Ver(SFH sfh, Sint32 ofs, Sint32 ver, Sint32 *val) {
    Uint8 *hdr = sfh->hdr;

    if (!sfh_IsAnlyOk(sfh)) {
        return FALSE;
    }
    if (sfh->ver < ver) {
        return FALSE;
    }
    *val = SWAP32(*(Uint32 *)(hdr + ofs));
    return TRUE;
}

/* Total video frame count (offset 0xC0; what mwPlyGetHdrInf reports as maxfrm). */
Bool SFH_AnlyMaxFrmNum(SFH sfh, Sint32 *val) {
    *val = 0;
    return sfh_GetHdrU32(sfh, SFH_HDR_MAXFRMNUM_OFS, val);
}

/* Maximum video packet payload length (0xBC). */
Bool SFH_AnlyMaxPlyLenVid(SFH sfh, Sint32 *val) {
    *val = 0;
    return sfh_GetHdrU32(sfh, SFH_HDR_MAXPLYLEN_VID_OFS, val);
}

/* Maximum audio packet payload length (0xB8). */
Bool SFH_AnlyMaxPlyLenAud(SFH sfh, Sint32 *val) {
    *val = 0;
    return sfh_GetHdrU32(sfh, SFH_HDR_MAXPLYLEN_AUD_OFS, val);
}

/* Byte rate (0xB4; versions before 1.10 report it negative). */
Bool SFH_AnlyByteRate(SFH sfh, Sint32 *val) {
    *val = 0;
    return sfh_GetHdrU32Ver(sfh, SFH_HDR_BYTERATE_OFS, SFH_VER_110, val);
}

/* Number of private elements (0xB3). */
Bool SFH_AnlyNumElemPrv(SFH sfh, Sint32 *val) {
    *val = 0;
    return sfh_GetHdrU8(sfh, SFH_HDR_NUMELEM_PRV_OFS, val);
}

/* Number of video elements (0xB2). */
Bool SFH_AnlyNumElemVid(SFH sfh, Sint32 *val) {
    *val = 0;
    return sfh_GetHdrU8(sfh, SFH_HDR_NUMELEM_VID_OFS, val);
}

/* Number of audio elements (0xB1). */
Bool SFH_AnlyNumElemAud(SFH sfh, Sint32 *val) {
    *val = 0;
    return sfh_GetHdrU8(sfh, SFH_HDR_NUMELEM_AUD_OFS, val);
}

/* Total element count (0xB0). */
Bool SFH_AnlyNumElemTot(SFH sfh, Sint32 *val) {
    *val = 0;
    return sfh_GetHdrU8(sfh, SFH_HDR_NUMELEM_TOT_OFS, val);
}

/* Pack size (0x8C; 0x800 for GameCube movies). */
Bool SFH_AnlyPackSiz(SFH sfh, Sint32 *val) {
    *val = 0;
    return sfh_GetHdrU32(sfh, SFH_HDR_PACKSIZ_OFS, val);
}

/* Packet size field length (0x88). */
Bool SFH_AnlyPketSizLen(SFH sfh, Sint32 *val) {
    *val = 0;
    return sfh_GetHdrS16(sfh, SFH_HDR_PKETSIZLEN_OFS, val);
}

/* Pack type (0x84). */
Bool SFH_AnlyPackType(SFH sfh, Sint32 *val) {
    *val = -1;
    return sfh_GetHdrU8(sfh, SFH_HDR_PACKTYPE_OFS, val);
}

/* Header size (0x80). */
Bool SFH_AnlyHdrSiz(SFH sfh, Sint32 *val) {
    *val = 0;
    return sfh_GetHdrU32(sfh, SFH_HDR_SIZ_OFS, val);
}

/* Muxer tool version: the larger of the binary version bytes (0x38/0x39) and the "Ver.M.m" parsed
 * from the tool string. */
Bool SFH_AnlyHdrToolVer(SFH sfh, Sint32 *major, Sint32 *minor) {
    Char8 buf[SFH_HDR_TOOLSTR_LEN + 1];
    Uint8 *hdr;
    Uint8 *str;
    Sint32 smajor, sminor;
    Sint32 hmajor, hminor;

    *major = 0;
    *minor = 0;
    hdr = sfh->hdr;
    str = hdr + SFH_HDR_TOOLSTR_OFS;
    buf[0] = '\0';
    if (!sfh_GetToolStr(sfh, str, buf)) {
        return FALSE;
    }
    hmajor = hdr[SFH_HDR_VER_MAJOR_OFS];
    hminor = hdr[SFH_HDR_VER_MINOR_OFS];
    if (!sfh_GetStrVer(buf, &smajor, &sminor)) {
        return FALSE;
    }
    if (hmajor * 100 + hminor >= smajor * 100 + sminor) {
        *major = hmajor;
        *minor = hminor;
    } else {
        *major = smajor;
        *minor = sminor;
    }
    return TRUE;
}

/* Whether stream `id` (audio or video) carries a feature block (tool version >= 1.10). */
Bool SFH_IsEffFtrInf(SFH sfh, Uint8 id, Sint32 *flag) {
    SFH_ELEM *elem;
    Sint32 ver = sfh->ver; /* a local copy: the inlined sfh_IsVerOk reloads sfh->ver */
    Uint32 sid;

    if (ver < SFH_VER_110) {
        return FALSE;
    }
    sid = id; /* one zero-extended copy for the switch, the search and the type check */
    switch (sfh_GetStmType(sid)) {
    case SFH_STM_AUD:
        elem = sfh_GetElem(sfh, sid);
        if (elem == 0) {
            return FALSE;
        }
        *flag = sfh_IsEffFtr(elem, sid, SFH_STM_AUD);
        break;
    case SFH_STM_VID:
        elem = sfh_GetElem(sfh, sid);
        if (elem == 0) {
            return FALSE;
        }
        *flag = sfh_IsEffFtr(elem, sid, SFH_STM_VID);
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

/* Whether an element record for stream `id` exists. */
Bool SFH_IsExistStmId(SFH sfh, Uint8 id, Sint32 *flag) {
    Uint8 *hdr;

    *flag = 0;
    hdr = sfh->hdr;
    if (!sfh_IsAnlyOk(sfh)) {
        return FALSE;
    }
    if (sfh_SearchElem(hdr, id) != 0) {
        *flag = 1;
    } else {
        *flag = 0;
    }
    return TRUE;
}

/* Validates the header: >= 0x800 bytes and the "SofdecStream" id at 0x20; records the tool
 * version. */
Bool SFH_IsSfdHeader(SFH sfh, Sint32 *flag) {
    Uint8 *idstr;
    Sint32 major, minor;

    *flag = 0;
    idstr = sfh->hdr + SFH_HDR_ID_OFS;
    if (sfh_IsFree(sfh) == TRUE) {
        return FALSE;
    }
    if ((Uint32)sfh->size < SFH_HDR_MIN_SIZE) {
        sfh->stat = SFH_STAT_INVALID;
        return FALSE;
    }
    if (memcmp(idstr, "SofdecStream            ", SFH_HDR_ID_LEN) != 0) {
        sfh->stat = SFH_STAT_INVALID;
        return FALSE;
    }
    sfh->stat = SFH_STAT_VALID;
    if (!SFH_AnlyHdrToolVer(sfh, &major, &minor)) {
        return FALSE;
    }
    sfh->ver = major * 100 + minor;
    *flag = 1;
    return TRUE;
}

/* Frees the analyser handle. */
void SFH_Destroy(SFH sfh) {
    sfh->stat = SFH_STAT_FREE;
    sfh->hdr = 0;
    sfh->size = 0;
    sfh->ver = 0;
    sfh_objinf.used--;
}

/* Takes a free analyser handle over the header bytes (state CREATED until validated). */
SFH SFH_Create(void *hdr, Sint32 size) {
    Sint32 num = sfh_objinf.num;
    SFH_OBJ *hn = sfh_objinf.hn;
    SFH sfh = 0;
    Sint32 i;

    if (sfh_objinf.used >= num) {
        return 0;
    }
    for (i = 0; i < num; i++) {
        sfh = &hn[i];
        if (sfh->stat == SFH_STAT_FREE) {
            break;
        }
    }
    sfh->stat = SFH_STAT_CREATED;
    sfh->hdr = hdr;
    sfh->size = size;
    sfh_objinf.used++;
    return sfh;
}

/* Clears the handle table. */
static void sfh_ClearHn(SFH_OBJ *hn, Sint32 num) {
    Sint32 i;

    for (i = 0; i < num; i++) {
        hn[i].stat = 0;
        hn[i].hdr = 0;
        hn[i].size = 0;
        hn[i].ver = 0;
    }
}

/* This game only: drops an init reference and clears the handle table at the last one
 * (inferred SFH library finish) */
void fn_80402058(void) {
    if (--sfh_init_cont > 0) {
        return;
    }
    sfh_objinf.num = 0;
    sfh_objinf.used = 0;
    sfh_objinf.hn = 0;
}

/* Library init (once): handle table in the caller's work. */
void SFH_Init(Sint32 num, void *work) {
    if (sfh_init_cont > 0) {
        return;
    }
    sfh_init_cont++;
    sfhlib_version_dummy = SFH_sbver_str;
    sfh_ClearHn(work, num);
    sfh_objinf.num = num;
    sfh_objinf.used = 0;
    sfh_objinf.hn = work;
}
