#ifndef _DOLPHIN_TRK_H
#define _DOLPHIN_TRK_H

#include <dolphin/types.h>
#include "MetroTRK/trktypes.h"
#include "MetroTRK/ppc_reg.h"
#include "MetroTRK/msghndlr.h"

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

////// MSG HANDLING FUNCTIONS //////
DSError TRKDoConnect(MessageBuffer *);
DSError TRKDoDisconnect(MessageBuffer *);
DSError TRKDoReset(MessageBuffer *);
DSError TRKDoVersions(MessageBuffer *);
DSError TRKDoSupportMask(MessageBuffer *);
DSError TRKDoOverride(MessageBuffer *);
DSError TRKDoReadMemory(MessageBuffer *);
DSError TRKDoWriteMemory(MessageBuffer *);
DSError TRKDoReadRegisters(MessageBuffer *);
DSError TRKDoWriteRegisters(MessageBuffer *);
DSError TRKDoSetOption(MessageBuffer *);
DSError TRKDoContinue(MessageBuffer *);
DSError TRKDoStep(MessageBuffer *);
DSError TRKDoStop(MessageBuffer *);

void SetBufferPosition(MessageBuffer *, u32);
void TRKUARTInterruptHandler(void);

DSError TRKGetFreeBuffer(int *, MessageBuffer **);
void OutputData(void *data, int length);
void TRKResetBuffer(MessageBuffer *msg, BOOL keepData);

DSError TRKReadBuffer1_ui64(MessageBuffer *buffer, u64 *data);
DSError TRKAppendBuffer1_ui64(MessageBuffer *buffer, const u64 data);
DSError TRKReadBuffer(MessageBuffer *msg, void *data, size_t length);
DSError TRKAppendBuffer(MessageBuffer *msg, const void *data, size_t length);
DSError TRKReadBuffer_ui8(MessageBuffer *buffer, u8 *data, int count);
DSError TRKReadBuffer_ui32(MessageBuffer *buffer, u32 *data, int count);
DSError TRKAppendBuffer_ui32(MessageBuffer *buffer, const u32 *data, int count);
DSError TRKDoNotifyStopped(MessageCommandID cmd);
////////////////////////////////////

/////// DOLPHIN TRK FUNCTIONS //////
void InitMetroTRK(void);
void InitMetroTRK_BBA(void);
void EnableMetroTRKInterrupts(void);

void TRKLoadContext(OSContext *ctx, u32 exception_id);
void TRKSaveExtended1Block(void);
void TRKRestoreExtended1Block(void);
int InitMetroTRKCommTable(int);
void TRK_board_display(char *);
void __TRK_reset(void);
void __TRK_copy_vectors(void);
u32 TRKTargetTranslate(u32 val);
void TRK__read_aram(int mem_addr, u32 aram_addr, size_t *length);
void TRK__write_aram(int mem_addr, u32 aram_addr, size_t *length);
void EnableEXI2Interrupts(void);
void TRK_flush_cache(u32 start, int length);
////////////////////////////////////

////////// GDEV FUNCTIONS //////////
int gdev_cc_initialize(void *inputPendingPtrRef, __OSInterruptHandler monitorCallback);
int gdev_cc_shutdown(void);
int gdev_cc_open(void);
int gdev_cc_close(void);
int gdev_cc_read(u8 *dest, int size);
int gdev_cc_write(const u8 *src, int size);
int gdev_cc_pre_continue(void);
int gdev_cc_post_stop(void);
int gdev_cc_peek(void);
int gdev_cc_initinterrupts(void);
/* Weak stub in an unsplit unit; its body only returns 0. */
int Hu_IsStub(void);
/* Also declared in dolphin/amc/AmcExi2Comm.h, which the TRK sources do not include: that header's
 * anonymous enum would shift this unit's string label numbering against the original. */
int AMC_IsStub(void);
////////////////////////////////////

/////////// UDP FUNCTIONS //////////
int udp_cc_initialize(void *flagOut, __OSInterruptHandler handler);
int udp_cc_shutdown(void);
int udp_cc_open(void);
int udp_cc_close(void);
int udp_cc_read(u8 *dest, int size);
int udp_cc_write(const u8 *src, int size);
int udp_cc_pre_continue(void);
int udp_cc_post_stop(void);
int udp_cc_peek(void);
int udp_cc_initinterrupts(void);
////////////////////////////////////

/////////// DDH FUNCTIONS //////////
#define DDH_ERR_NOT_INITIALIZED -0x2711
#define DDH_ERR_ALREADY_INITIALIZED -0x2715
#define DDH_ERR_READ_ERROR -0x2719

int ddh_cc_initialize(void *inputPendingPtrRef, __OSInterruptHandler monitorCallback);
int ddh_cc_shutdown(void);
int ddh_cc_open(void);
int ddh_cc_close(void);
int ddh_cc_read(u8 *dest, int size);
int ddh_cc_write(const u8 *src, int size);
int ddh_cc_pre_continue(void);
int ddh_cc_post_stop(void);
int ddh_cc_peek(void);
int ddh_cc_initinterrupts(void);
////////////////////////////////////

////////// EVENT FUNCTIONS /////////
void TRKConstructEvent(TRKEvent *event, int eventType);
void TRKDestructEvent(TRKEvent *event);
DSError TRKPostEvent(TRKEvent *event);
BOOL TRKGetNextEvent(TRKEvent *event);
DSError TRKDispatchMessage(MessageBuffer *);
void *TRKGetBuffer(int);
void TRKReleaseBuffer(int);
void TRKGetInput(void);
MessageBufferID TRKTestForPacket(void);
void TRKProcessInput(int bufferIdx);
////////////////////////////////////

///////// TARGET FUNCTIONS /////////
DSError TRKTargetContinue(void);
DSError TRKTargetInterrupt(TRKEvent *);
BOOL TRKTargetStopped(void);
void TRKTargetSetStopped(unsigned int);
DSError TRKTargetSupportRequest(void);
DSError TRKTargetAccessMemory(void *data, u32 start, size_t *length,
                              MemoryAccessOptions accessOptions, BOOL read);
DSError TRKTargetAccessDefault(u32 firstRegister, u32 lastRegister, MessageBuffer *b,
                               size_t *registersLengthPtr, BOOL read);
DSError TRKTargetAccessFP(u32 firstRegister, u32 lastRegister, MessageBuffer *b,
                          size_t *registersLengthPtr, BOOL read);
DSError TRKTargetAccessExtended1(u32 firstRegister, u32 lastRegister, MessageBuffer *b,
                                 size_t *registersLengthPtr, BOOL read);
DSError TRKTargetAccessExtended2(u32 firstRegister, u32 lastRegister, MessageBuffer *b,
                                 size_t *registerStorageSize, BOOL read);
DSError TRKTargetAccessARAM(u32 mem_addr, u32 aram_addr, size_t *length, BOOL read);
u32 TRKTargetGetPC(void);
DSError TRKTargetSingleStep(u32 count, BOOL stepOver);
DSError TRKTargetStepOutOfRange(u32 rangeStart, u32 rangeEnd, BOOL stepOver);
BOOL TRKTargetStop(void);
u32 __TRK_get_MSR(void);
void __TRK_set_MSR(u32 msr);
////////////////////////////////////

////// NUB AND MEM FUNCTIONS ///////
DSError TRKAppendBuffer_ui8(MessageBuffer *, const u8 *, int);
DSError TRKSetBufferPosition(MessageBuffer *, u32);

DSError TRKMessageSend(MessageBuffer *);
void TRKSwapAndGo(void);
DSError TRKInitializeNub(void);
DSError TRKTerminateNub(void);
BOOL TRKInitializeEndian(void);
void TRKNubWelcome(void);
void TRKNubMainLoop(void);

DSError TRKInitializeMutex(void *);
DSError TRKAcquireMutex(void *);
DSError TRKReleaseMutex(void *);
void *TRK_memcpy(void *dst, const void *src, size_t n);
void *TRK_memset(void *dst, int val, size_t n);
////////////////////////////////////

/////// INITIALIZE FUNCTIONS ///////
DSError TRKInitializeEventQueue(void);
DSError TRKInitializeMessageBuffers(void);
DSError TRKInitializeDispatcher(void);
void InitializeProgramEndTrap(void);
DSError TRKInitializeSerialHandler(void);
DSError TRKTerminateSerialHandler(void);
DSError TRKInitializeTarget(void);
////////////////////////////////////

////////// EXI2 FUNCTIONS //////////
/* EXI2 */
void UnreserveEXI2Port(void);
void ReserveEXI2Port(void);
////////////////////////////////////

/////////// MW FUNCTIONS ///////////
void MWTRACE(u8, char *, ...);
////////////////////////////////////

//////// SUPPORT FUNCTIONS /////////
DSError TRKRequestSend(MessageBuffer *msgBuf, int *bufferId, u32 p1, u32 p2, int p3);
u32 TRKAccessFile(u32 command, u32 file_handle, size_t *length_ptr, u8 *buffer_ptr);
u32 TRKCloseFile(u32 command, u32 file_handle);
DSError TRKSuppAccessFile(u32 file_handle, u8 *data, size_t *count, DSIOResult *io_result,
                          BOOL need_reply, BOOL read);
DSError HandleCloseFileSupportRequest(int replyError, DSIOResult *ioResult);
DSError HandlePositionFileSupportRequest(u32 handle, u32 *position, u8 mode, DSIOResult *ioResult);
////////////////////////////////////

////////// OTHER FUNCTIONS /////////
DSError TRK_main(void);
UARTError InitializeUART(UARTBaudRate baudRate);
DSError TRKInitializeIntDrivenUART(u32, u32, u32, void *);
int TRKPollUART(void);
UARTError TRKReadUARTN(void *, u32);
UARTError TRKWriteUARTN(const void *bytes, u32 length);
void usr_put_initialize(void);
void TRKTargetSetInputPendingPtr(void *);
void SetUseSerialIO(u8);
u8 GetUseSerialIO(void);

////// MSL CONSOLE SUPPORT (mslsupp.c) //////
/* Console I/O for MSL: __read_console and __close_console are installed in its standard streams
 * (ansi_files.h), and its __write_console calls __TRK_write_console (uart_console_io_gcn.h). */
int __read_console(u32 handle, u8 *buffer, size_t *count, void (*idle_fn)(void));
/* Also declared in uart_console_io_gcn.h (with the MSL types), which cannot be included together
 * with this header: both define offsetof. */
int __TRK_write_console(u32 handle, u8 *buffer, size_t *count, void (*idle_fn)(void));
int __close_console(u32 handle);

DSError TRKTargetAddStopInfo(MessageBuffer *);
void TRKTargetAddExceptionInfo(MessageBuffer *);
void TRKInterruptHandler(void);
BOOL usr_puts_serial(const char *msg);
////////////////////////////////////

//////// EXTERNAL DECLARES /////////
extern BOOL gTRKBigEndian;
extern void *gTRKInputPendingPtr;
extern ProcessorState_PPC gTRKCPUState;
extern ProcessorRestoreFlags_PPC gTRKRestoreFlags;
extern u8 gTRKInterruptVectorTable[];
extern TRKState gTRKState;

/* linker-defined top of the debugger stack */
extern u8 _db_stack_addr[];
extern TRKEventQueue gTRKEventQueue;
extern MessageBuffer gTRKMsgBufs[3];
////////////////////////////////////

////////// USEFUL STATICS //////////
static inline DSError TRKAppendBuffer1_ui8(MessageBuffer *buffer, const u8 data) {
    if (buffer->position >= 0x880) {
        return DS_MessageBufferOverflow;
    }

    buffer->data[buffer->position++] = data;
    buffer->length++;
    return DS_NoError;
}
////////////////////////////////////

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif
