/* CRI ADXT library init / finish (adx_inis.c, ADXT/GC Ver.9.44): ADXT_Init brings up every ADX module
 * (stream joints, error, ADXSTM, ADXSJD, ADXF, renderer, LSC, SVM) and registers the server
 * callbacks with the SVM; ADXT_Finish undoes it. Holds the handle table adxt_obj, the build string
 * and the server ids.
 *
 * Bodies from the RE4 GameCube decompilation (Ver.9.31). Ver.9.44 always registers the threaded
 * callbacks (with their names), runs the load scheduler through ADXT_ExecLscSvr and also finishes
 * the critical section; the no-thread main callback and the vsync switch are gone.
 */
#include "CRI/adx.h"
#include "CRI/adx_f.h"
#include "CRI/adx_rna.h"
#include "CRI/adx_stm.h"
#include "CRI/adx_t.h"
#include "CRI/lsc.h"
#include "CRI/sj.h"
#include "CRI/svm.h"

extern void *memset(void *destination, int value, unsigned int size);

Sint32 adxt_exec_fssvr(void *obj);
Sint32 adxt_exec_tsvr(void *obj);
Sint32 adxt_exec_main_thrd(void *obj);
void adxini_lscerr_cbfn(void *obj, Char8 *msg);
static void adxini_rnaerr_cbfn(void *obj, Char8 *msg);

const Char8 adxt_build[] =
    "\nADXT/GC Ver.9.44 Build:Feb 28 2005 15:48:49\n\0Append: MW2407 GC20Apr2004Patch2\n";

Sint32 adxt_init_cnt = 0;
static Sint32 adxt_svr_id = 0;
Sint32 adxt_svr_main_id = 0;
Sint32 adxt_output_mono_flag = 0;
Sint32 adxt_svr_fs_id = 0;
Sint32 adxt_vsync_cnt = 0;
ADXT_OBJ adxt_obj[ADXT_MAX_OBJ];
const Char8 *cri_verstr_ptr;

/* Library shutdown (reference counted): destroys the handles, shuts down the renderer, file
 * system, stream controller and scheduler, removes the server callbacks, and finishes the SVM,
 * decoder, error, stream-joint and critical-section modules.
 */
void ADXT_Finish(void) {
    if (--adxt_init_cnt == 0) {
        ADXT_DestroyAll();
        ADXRNA_Finish();
        ADXF_Finish();
        ADXSTM_Finish();
        LSC_Finish();
        ADXCRS_Lock();
        SVM_DelCbSvr(2, 1);
        SVM_DelCbSvr(4, adxt_svr_fs_id);
        SVM_DelCbSvr(5, adxt_svr_main_id);
        SVM_Finish();
        ADXSJD_Finish();
        ADXERR_Finish();
        SJMEM_Finish();
        SJRBF_Finish();
        SJUNI_Finish();
        ADXCRS_Unlock();
        ADXCRS_Finish();
    }
}

/* Unreferenced, so the linker strips it (body from RE4, not verified); its reference puts
 * adxt_obj before cri_verstr_ptr in .bss.
 */
ADXT ADXT_GetObj(Sint32 no) {
    return &adxt_obj[no];
}

/* Library init (reference counted): brings up the modules, registers the vsync (talk server),
 * file-system and main (load scheduler) callbacks and sets the default 60 Hz server rate.
 */
void ADXT_Init(void) {
    cri_verstr_ptr = adxt_build;
    if (adxt_init_cnt == 0) {
        ADXCRS_Init();
        ADXCRS_Lock();
        SJUNI_Init();
        SJRBF_Init();
        SJMEM_Init();
        ADXERR_Init();
        ADXSTM_Init();
        ADXSJD_Init();
        ADXF_Init();
        ADXRNA_Init();
        LSC_Init();
        SVM_Init();
        ADXRNA_EntryErrFunc(adxini_rnaerr_cbfn, 0);
        LSC_EntryErrFunc(adxini_lscerr_cbfn, 0);
        memset(adxt_obj, 0, sizeof(adxt_obj));
        SVM_SetCbSvrId(2, 1, adxt_exec_tsvr, 0, "adxt_exec_tsvr");
        adxt_svr_fs_id = SVM_SetCbSvr(4, adxt_exec_fssvr, 0, "adxt_exec_fssvr");
        adxt_svr_main_id = SVM_SetCbSvr(5, adxt_exec_main_thrd, 0, "adxt_exec_main_thrd");
        adxt_vsync_cnt = 0;
        adxt_output_mono_flag = 0;
        ADXT_SetDefSvrFreq(60);
        ADXCRS_Unlock();
    }
    adxt_init_cnt++;
}

/* SVM callback (type 4, file system): one ADXT_ExecFsSvr pass. */
Sint32 adxt_exec_fssvr(void *obj) {
    ADXT_ExecFsSvr();
    return 0;
}

/* SVM callback (type 2, vsync): one ADXT_ExecServer pass. */
Sint32 adxt_exec_tsvr(void *obj) {
    ADXT_ExecServer();
    return 0;
}

/* SVM main callback: the load scheduler. */
Sint32 adxt_exec_main_thrd(void *obj) {
    ADXT_ExecLscSvr();
    return 0;
}

/* Forwards load scheduler error messages to the ADX error callback. */
void adxini_lscerr_cbfn(void *obj, Char8 *msg) {
    ADXERR_CallErrFunc1(msg);
}

/* Forwards renderer error messages to the ADX error callback. */
static void adxini_rnaerr_cbfn(void *obj, Char8 *msg) {
    ADXERR_CallErrFunc1(msg);
}
