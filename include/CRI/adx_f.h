#ifndef CRI_ADX_F_H
#define CRI_ADX_F_H

/* CRI ADX file system (ADXF: adx_fini.c, adx_fs.c, adx_fcch.c): sector-based file handles on top
 * of the ADX stream controller, plus the AFS partition tables they read from. Layout from the
 * matched adx_fs.c code; field names follow the Street Fighter III 3rd Strike and TMNT2 CRI
 * sources.
 */
#include "CRI/cri_xpt.h"
#include "CRI/sj.h"
#include "CRI/adx_stm.h"
#include "CRI/adx.h"

#define ADXF_OBJ_MAX 16
#define ADXF_PART_MAX 256
#define ADXF_FNAME_MAX 256
#define ADXF_CMD_HSTRY_MAX 16
#define ADXF_CMD_NUM 16
#define ADXF_SCT_SIZE 2048

/* ADX_FS.stat (the ADXSTM status values while a read is running) */
#define ADXF_STAT_STOP 1
#define ADXF_STAT_READING 2
#define ADXF_STAT_READEND 3
#define ADXF_STAT_ERROR 4

#define ADXF_ERR_OK 0
#define ADXF_ERR_FATAL -1
#define ADXF_ERR_INTERNAL -2
#define ADXF_ERR_PRM -3

/* Command history ids (ADXF_CMD_HSTRY.cmdid, index into adxf_cmd_ncall) */
#define ADXF_CMD_OPEN 1
#define ADXF_CMD_OPEN_AFS 2
#define ADXF_CMD_CLOSE 3
#define ADXF_CMD_READ_NW32 4
#define ADXF_CMD_STOP 5
#define ADXF_CMD_STOP_NW 6 /* inferred: only the unreferenced adxf_StopNw records it */

typedef struct {
    /* 0x00 */ Sint8 used;
    /* 0x01 */ Sint8 stat;
    /* 0x02 */ Sint8 sjflag;     /* stream joint supplied by the caller (not destroyed) */
    /* 0x03 */ Sint8 stopnw_flg; /* ADXSTM_StopNw pending */
    /* 0x04 */ ADXSTM stm;
    /* 0x08 */ SJ sj;
    /* 0x0C */ Sint32 fnsct; /* file size in sectors */
    /* 0x10 */ Sint32 fsize; /* file size in bytes */
    /* 0x14 */ Sint32 skpos; /* read position in sectors */
    /* 0x18 */ Sint32 rdstpos;
    /* 0x1C */ Sint32 rqsct;
    /* 0x20 */ Sint32 rdsct;
    /* 0x24 */ void *buf;
    /* 0x28 */ Sint32 bsize;
    /* 0x2C */ Sint32 rqrdsct;
    /* 0x30 */ Sint32 ofst; /* first sector of the file inside its container */
    /* 0x34 */ void *dir;
    /* 0x38 */ const Char8 *fname;
    /* 0x3C */ Sint32 bind_ofst; /* range passed to ADXSTM_BindFile */
    /* 0x40 */ Sint32 bind_nsct;
} ADX_FS;

typedef ADX_FS *ADXF;

/* AFS partition table. Revision 1 stores 32-bit sector counts after top; revision 0 stores a
 * 16-bit top followed by 16-bit counts.
 */
typedef struct ADXF_PTINFO {
    /* 0x000 */ struct ADXF_PTINFO *next;
    /* 0x004 */ Sint32 size;
    /* 0x008 */ Sint32 nfile;
    /* 0x00C */ Uint16 nentry;
    /* 0x00E */ Sint8 type;
    /* 0x00F */ Sint8 rev;
    /* 0x010 */ Char8 fname[ADXF_FNAME_MAX];
    /* 0x110 */ void *curdir;
    /* 0x114 */ Sint32 ofst;
    /* 0x118 */ Sint32 top;
} ADXF_PTINFO;

typedef struct {
    /* 0x00 */ Uint8 cmdid;
    /* 0x01 */ Uint8 fg; /* 0 = entry, 1 = exit */
    /* 0x02 */ Uint16 ncall;
    /* 0x04 */ Sint32 prm[3];
} ADXF_CMD_HSTRY;

#ifdef __cplusplus
extern "C" {
#endif

/* adx_fini.c */
extern Sint32 adxf_init_cnt;
extern Sint32 adxf_ldptnw_last_stat;
extern Sint32 adxf_ldptnw_ptid;
extern ADXF adxf_ldptnw_hn;
extern Sint32 adxf_flno;
extern Sint32 adxf_hstry_no;
extern Uint16 adxf_cmd_ncall[ADXF_CMD_NUM];
extern ADXF_CMD_HSTRY adxf_cmd_hstry[ADXF_CMD_HSTRY_MAX];
extern Sint32 adxf_ocbi_fg;
extern ADXF_PTINFO *adxf_ptinfo[ADXF_PART_MAX];
extern ADX_FS adxf_obj[ADXF_OBJ_MAX];

void ADXF_Init(void);
void ADXF_Finish(void);

/* adx_fcch.c */
void ADXF_Ocbi(void *addr, Uint32 nbytes);

/* adx_fs.c */
Sint32 ADXF_LoadPartitionNw(Sint32 ptid, const Char8 *fname, void *dir, void *ptinfo);
Sint32 ADXF_GetPtStat(Sint32 ptid);
void ADXF_CloseAll(void);
void ADXF_ExecServer(void);
Sint32 ADXF_GetFnameRangeEx(Sint32 ptid, Sint32 flid, Char8 *fname, void **dir, Sint32 *ofst,
                            Sint32 *fnsct);

#ifdef __cplusplus
}
#endif

#endif
