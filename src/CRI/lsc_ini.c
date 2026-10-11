/* CRI LSC init / finish (lsc_ini.c, LSC/GC Ver.2.22): the scheduler table lsc_obj and the
 * build string. Names and layout follow the RE4 GameCube decompilation of the same file.
 */

extern void *memset(void *destination, int value, unsigned int size);

/* CRI basic types (cri_xpt.h); 32-bit integers are long. */
typedef unsigned char Uint8;
typedef signed char Sint8;
typedef signed long Sint32;
typedef char Char8;

#define LSC_MAX_OBJ 32
#define LSC_MAX_ENTRY 16

typedef struct {
    /* 0x00 */ Sint32 id;
    /* 0x04 */ Char8 *fname;
    /* 0x08 */ Sint32 fname_sum;
    /* 0x0C */ void *dir;
    /* 0x10 */ Sint32 ofst;
    /* 0x14 */ Sint32 nsct;
    /* 0x18 */ Sint32 stat;
    /* 0x1C */ Sint32 pos;
} LSC_ENTRY;

typedef struct {
    /* 0x00 */ Sint8 used;
    /* 0x01 */ Sint8 stat;
    /* 0x02 */ Sint8 stm_start;
    /* 0x03 */ Sint8 loop_flag;
    /* 0x04 */ Sint8 pause_flag;
    /* 0x05 */ Uint8 pad5[3];
    /* 0x08 */ void *sj;
    /* 0x0C */ Sint32 pad0c;
    /* 0x10 */ Sint32 pad10;
    /* 0x14 */ Sint32 min_val;
    /* 0x18 */ Sint32 bufsize;
    /* 0x1C */ Sint32 wr_idx;
    /* 0x20 */ Sint32 rd_idx;
    /* 0x24 */ Sint32 num_stm;
    /* 0x28 */ void *stm;
    /* 0x2C */ Sint32 cur_nsct;
    /* 0x30 */ Sint32 pad30;
    /* 0x34 */ Sint32 pad34;
    /* 0x38 */ LSC_ENTRY tbl[LSC_MAX_ENTRY];
} LSC_OBJ;

extern void LSC_Destroy(LSC_OBJ *lsc);
extern void LSC_EntryErrFunc(void (*func)(void *obj, Char8 *msg), void *obj);
extern void LSC_LockCrs(Sint32 *msk);
extern void LSC_UnlockCrs(Sint32 *msk);

/* volatile: LSC_Init reads the build string pointer without using it. */
const Char8 *const volatile lsc_build = "\nLSC/GC Ver.2.22 Build:Feb 28 2005 15:48:30\n";

static Sint32 lsc_init_cnt = 0;
LSC_OBJ lsc_obj[LSC_MAX_OBJ];

/* Destroys the live schedulers and clears the table and error callback on the last release. */
void LSC_Finish(void) {
    Sint32 msk;
    LSC_OBJ *lsc;
    Sint32 i;

    LSC_LockCrs(&msk);
    if (--lsc_init_cnt == 0) {
        for (i = 0; i < LSC_MAX_OBJ; i++) {
            lsc = &lsc_obj[i];
            if (lsc->used == 1) {
                LSC_Destroy(lsc);
            }
        }
        memset(lsc_obj, 0, sizeof(lsc_obj));
        LSC_EntryErrFunc(0, 0);
    }
    LSC_UnlockCrs(&msk);
}

/* Clears the table and error callback on the first init. */
void LSC_Init(void) {
    Sint32 msk;

    lsc_build;
    LSC_LockCrs(&msk);
    if (lsc_init_cnt == 0) {
        memset(lsc_obj, 0, sizeof(lsc_obj));
        LSC_EntryErrFunc(0, 0);
    }
    lsc_init_cnt++;
    LSC_UnlockCrs(&msk);
}
