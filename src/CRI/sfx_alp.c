/* Sofdec SFX alpha tables (sfx_alp.c, SFXA): per-converter alpha settings, either three fixed
 * alpha levels or a luma-driven ramp between lumi_min and lumi_max, and the calls that build the
 * ARGB8888 conversion tables from them through the table makers registered in the object.
 *
 * Names follow the RE4 GameCube decompilation of the same file.
 */
#include <string.h>

#include "CRI/sfx.h"

#define SFXA_MAX_HN 8

/* table makers registered in the object (cftyp422_ppc.c) */
typedef void (*SFXA_MakeAlp3Fn)(void *tbl, Uint8 alp0, Uint8 alp1, Uint8 alp2);
typedef void (*SFXA_MakeLumiFn)(Sint32 lumi_min, Sint32 lumi_max, Sint32 lumi_rate, void *tbl);

/* alpha object */
struct SFXA_OBJ {
    Sint32 used;                  /* 0x00 */
    Sint32 need_update;           /* 0x04 luma settings changed since the table was built */
    Sint32 lumi_min;              /* 0x08 */
    Sint32 lumi_max;              /* 0x0C */
    Sint32 lumi_rate;             /* 0x10 */
    Uint8 alp0;                   /* 0x14 */
    Uint8 alp1;                   /* 0x15 */
    Uint8 alp2;                   /* 0x16 */
    SFXA_MakeLumiFn make_lumi;    /* 0x18 */
    SFXA_MakeAlp3Fn make_alp3110; /* 0x1C */
    SFXA_MakeAlp3Fn make_alp3211; /* 0x20 */
};
CHECK_SIZEOF(SFXA_OBJ, 0x24);

typedef struct {
    Sint32 cnt;                /* 0x00 objects in use */
    Sint32 nobj;               /* 0x04 */
    SFXA_OBJ obj[SFXA_MAX_HN]; /* 0x08 */
} SFXA_WORK;
CHECK_SIZEOF(SFXA_WORK, 0x128);

SFXA_WORK sfxa_work;

/* Whether the luma settings changed since the table was last built. */
Sint32 SFXA_IsNeedUpdateLumiTbl(SFXA_OBJ *sfxa) {
    return sfxa->need_update;
}

/* Builds the table with the "3211" variant of the three alpha levels (alp0, alp1, alp2). */
void SFXA_MakeAlp3211Tbl(SFXA_OBJ *sfxa, Sint32 frmno, void *tbl) {
    SFXA_MakeAlp3Fn make = sfxa->make_alp3211;
    Uint8 alp0 = sfxa->alp0;
    Uint8 alp1 = sfxa->alp1;
    Uint8 alp2 = sfxa->alp2;

    if (make != NULL) {
        make(tbl, alp0, alp1, alp2);
    }
}

/* Builds the table with the "3110" variant of the three alpha levels. */
void SFXA_MakeAlp3110Tbl(SFXA_OBJ *sfxa, Sint32 frmno, void *tbl) {
    SFXA_MakeAlp3Fn make = sfxa->make_alp3110;
    Uint8 alp0 = sfxa->alp0;
    Uint8 alp1 = sfxa->alp1;
    Uint8 alp2 = sfxa->alp2;

    if (make != NULL) {
        make(tbl, alp0, alp1, alp2);
    }
}

/* Builds the table with alpha following the luma, and clears the update flag. */
void SFXA_MakeAlpLumiTbl(SFXA_OBJ *sfxa, Sint32 frmno, void *tbl) {
    SFXA_MakeLumiFn make = sfxa->make_lumi;
    Sint32 lumi_min = sfxa->lumi_min;
    Sint32 lumi_max = sfxa->lumi_max;
    Sint32 lumi_rate = sfxa->lumi_rate;

    if (make != NULL) {
        make(lumi_min, lumi_max, lumi_rate, tbl);
    }
    sfxa->need_update = 0;
}

/* Releases the object. */
void SFXA_Destroy(SFXA_OBJ *sfxa) {
    if (sfxa == NULL) {
        return;
    }
    sfxa->used = 0;
    sfxa_work.cnt--;
}

/* First unused alpha object, or NULL. Inlined into SFXA_Create. */
static SFXA_OBJ *sfxa_search_free(void) {
    SFXA_OBJ *sfxa;
    Sint32 i;

    sfxa = sfxa_work.obj;
    for (i = 0; i < sfxa_work.nobj; i++) {
        if (sfxa->used == 0) {
            return sfxa;
        }
        sfxa++;
    }
    return NULL;
}

/* Takes an alpha object with the default settings: luma range 0 to 31 at rate 100, alpha levels
 * 0, 127 and 255, and a table update pending. */
SFXA_OBJ *SFXA_Create(void) {
    SFXA_OBJ *sfxa;

    sfxa = sfxa_search_free();
    if (sfxa == NULL) {
        return sfxa;
    }
    sfxa->lumi_min = 0;
    sfxa->lumi_max = 31;
    sfxa->lumi_rate = 100;
    sfxa->need_update = 1;
    sfxa->alp0 = 0;
    sfxa->alp1 = 127;
    sfxa->alp2 = 255;
    sfxa_work.cnt++;
    sfxa->used = 1;
    return sfxa;
}

void SFXA_Finish(void) {}

/* Clears the alpha object pool. */
void SFXA_Init(void) {
    memset(&sfxa_work, 0, sizeof(sfxa_work));
    sfxa_work.nobj = SFXA_MAX_HN;
}
