/* CRI RNA renderer ARAM resources (rna_res.c, RNARES): unless rnares_setup_fg is set, RNARES_Init
 * allocates 256 KiB of ARAM and splits it into 32 buffers of 0x2000 bytes, which RNARES_Create
 * hands out one per handle. Buffer addresses and sizes are kept in 16-bit sample units.
 * Body from the RE4 GameCube decompilation of the same file.
 */
#include <string.h>

#include <dolphin/ar.h>

#include "CRI/adx_rna.h"
#include "CRI/cri_xpt.h"
#include "layout_check.h"

#define RNARES_MAX_OBJ 32
#define RNARES_DEF_ARAM_SIZE 0x40000
#define RNARES_BUF_SIZE 0x2000

/* one ARAM buffer (name inferred) */
struct RNARES_OBJ {
    Sint32 used; /* 0x0 */
    Uint32 buf;  /* 0x4 ARAM address / 2 */
    Uint32 size; /* 0x8 in 16-bit samples */
};
CHECK_SIZEOF(struct RNARES_OBJ, 0xC);

static Uint32 rnares_init_cnt = 0;
Uint32 rnares_setup_fg = 0;
static Uint32 rnares_nbuf = 0;
Uint32 rnares_aram_size = 0;
Uint32 rnares_aram_ptr = 0;
static struct RNARES_OBJ rnares_obj[RNARES_MAX_OBJ];

/* Size of the handle's buffer in 16-bit samples (0 for no handle). */
Sint32 RNARES_GetBufSize(RNARES res) {
    if (res == NULL) {
        return 0;
    }
    return res->size;
}

/* ARAM address of the handle's buffer in 16-bit units (0 for no handle). */
Uint32 RNARES_GetBuf(RNARES res) {
    if (res == NULL) {
        return 0;
    }
    return res->buf;
}

/* Returns the buffer to the pool. */
void RNARES_Destroy(RNARES res) {
    if (res != NULL) {
        res->used = 0;
    }
}

/* Takes a free buffer; error E1070313 when all are in use. */
RNARES RNARES_Create(void) {
    RNARES res;
    Sint32 i;

    for (i = 0; i < RNARES_MAX_OBJ; i++) {
        if (rnares_obj[i].used == 0) {
            break;
        }
    }
    if (i == RNARES_MAX_OBJ) {
        RNAERR_CallErrFunc("E1070313:Not enough RNARES handle.\n");
        return NULL;
    }
    res = &rnares_obj[i];
    res->used = 1;
    return res;
}

/* On the last call (balancing RNARES_Init): releases the handles and, when RNARES_Init allocated
 * the ARAM, frees it (error E1090601 when the freed block is not that allocation). */
void RNARES_Finish(void) {
    Sint32 i;
    Uint32 size;

    if (--rnares_init_cnt == 0) {
        for (i = 0; i < RNARES_MAX_OBJ; i++) {
            if (rnares_obj[i].used == 1) {
                RNARES_Destroy(&rnares_obj[i]);
            }
        }
        memset(rnares_obj, 0, sizeof(rnares_obj));
        if (rnares_setup_fg == 0) {
            ARFree(&size);
            if (size != rnares_aram_size) {
                RNAERR_CallErrFunc("E1090601:Free area other than ADX buffer.\n");
            }
            rnares_nbuf = 0;
            rnares_aram_size = 0;
            rnares_aram_ptr = 0;
        }
    }
}

/* On the first call: allocates the ARAM unless rnares_setup_fg is set and records the buffers. */
void RNARES_Init(void) {
    Uint32 ofs;
    Uint32 sum;
    Uint32 ptr;
    Uint32 i;
    Uint32 n;
    struct RNARES_OBJ *res;

    if (rnares_init_cnt == 0) {
        if (rnares_setup_fg == 0) {
            rnares_nbuf = RNARES_MAX_OBJ;
            rnares_aram_size = RNARES_DEF_ARAM_SIZE;
            rnares_aram_ptr = ARAlloc(RNARES_DEF_ARAM_SIZE);
        }
        memset(rnares_obj, 0, sizeof(rnares_obj));
        n = rnares_nbuf;
        res = rnares_obj;
        ptr = rnares_aram_ptr;
        ofs = 0;
        for (i = 0; i < n; i++, res++) {
            sum = ptr + ofs;
            ofs += RNARES_BUF_SIZE;
            res->buf = sum >> 1;
            res->size = RNARES_BUF_SIZE / 2;
        }
    }
    rnares_init_cnt++;
}
