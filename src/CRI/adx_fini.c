/* CRI ADX file system init / finish (adx_fini.c, ADXF/GC Ver.7.30): owns the ADXF globals
 * (handle table, AFS partition table, command history) and resets them in ADXF_Init and
 * ADXF_Finish. The handle API is in adx_fs.c.
 *
 * Names and layout follow the RE4 and Mortal Kombat: Deception decompilations (Ver.7.18 and
 * 7.17). Ver.7.30 stores the build string's address in ADXF_Init.
 */
#include "CRI/cri_xpt.h"

extern void *memset(void *destination, int value, unsigned int size);

#define ADXF_MAX_OBJ 16
#define ADXF_OBJ_SIZE 0x44
#define ADXF_MAX_PTINFO 0x100
#define ADXF_CMD_HSTRY_NUM 0x100
#define ADXF_CMD_NUM 8

extern void ADXF_CloseAll(void);

/* Build string followed by the compiler patch line. */
const Char8 adxf_build[] =
    "\nADXF/GC Ver.7.30 Build:Feb 28 2005 15:49:08\n\0Append: MW2407 GC20Apr2004Patch2\n";

Sint32 adxf_init_cnt;
Sint32 adxf_ldptnw_last_stat;
Sint32 adxf_ldptnw_ptid;
void *adxf_ldptnw_hn;
Sint32 adxf_flno;
Sint32 adxf_hstry_no;
Sint32 adxf_cmd_ncall[ADXF_CMD_NUM];
Uint8 adxf_cmd_hstry[ADXF_CMD_HSTRY_NUM];
Sint32 adxf_ocbi_fg;
void *adxf_ptinfo[ADXF_MAX_PTINFO];
Uint8 adxf_obj[ADXF_MAX_OBJ * ADXF_OBJ_SIZE];
/* Address of the build string, set by ADXF_Init (name inferred). */
const Char8 *adxf_build_ptr;

/* Unreferenced, so the linker strips it. Body from the RE4 decompilation (not verified
 * against Shadow); as the first function touching the globals it fixes their .bss order.
 */
Sint32 ADXF_GetNumCmd(Sint32 *ncall) {
    Sint32 i;
    Sint32 n;

    if (adxf_init_cnt == 0) {
        return 0;
    }
    adxf_ldptnw_last_stat = 0;
    adxf_ldptnw_ptid = -1;
    adxf_ldptnw_hn = 0;
    adxf_flno = 0;
    adxf_hstry_no = 0;
    n = 0;
    for (i = 0; i < ADXF_CMD_NUM; i++) {
        ncall[i] = adxf_cmd_ncall[i];
        n += adxf_cmd_hstry[i];
    }
    adxf_ocbi_fg = 0;
    adxf_ptinfo[0] = adxf_obj;
    return n;
}

/* Reference-counted shutdown: closes every handle and clears the partition table, the command
 * history and the no-wait partition-load state.
 */
void ADXF_Finish(void) {
    if (--adxf_init_cnt == 0) {
        ADXF_CloseAll();
        adxf_ldptnw_last_stat = 1;
        adxf_ldptnw_ptid = -1;
        adxf_ldptnw_hn = 0;
        adxf_flno = 0;
        adxf_ocbi_fg = 0;
        adxf_hstry_no = 0;
        memset(adxf_cmd_ncall, 0, sizeof(adxf_cmd_ncall));
        memset(adxf_cmd_hstry, 0xFF, sizeof(adxf_cmd_hstry));
        memset(adxf_ptinfo, 0, sizeof(adxf_ptinfo));
        memset(adxf_obj, 0, sizeof(adxf_obj));
    }
}

/* Reference-counted init: clears the handles, the partition table and the command history. */
void ADXF_Init(void) {
    adxf_build_ptr = adxf_build;
    if (adxf_init_cnt == 0) {
        memset(adxf_obj, 0, sizeof(adxf_obj));
        memset(adxf_ptinfo, 0, sizeof(adxf_ptinfo));
        memset(adxf_cmd_hstry, 0xFF, sizeof(adxf_cmd_hstry));
        memset(adxf_cmd_ncall, 0, sizeof(adxf_cmd_ncall));
        adxf_hstry_no = 0;
        adxf_ocbi_fg = 0;
        adxf_flno = 0;
        adxf_ldptnw_hn = 0;
        adxf_ldptnw_ptid = -1;
        adxf_ldptnw_last_stat = 1;
    }
    adxf_init_cnt++;
}
