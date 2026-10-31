#ifndef _DOLPHIN_DBINTERFACE_H_
#define _DOLPHIN_DBINTERFACE_H_

#include <dolphin/os.h>
#include "layout_check.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DBInterface {
    u32 bPresent;
    u32 exceptionMask;
    void (*ExceptionDestination)(void);
    void *exceptionReturn;
} DBInterface;
CHECK_SIZEOF(DBInterface, 0x10);

extern DBInterface *__DBInterface;

/* Notification callback of the debugger driver; MWCallback calls it with 0 when input arrives. */
typedef void (*MTRCallbackType)(int);

void DBInit(void);
/* Stores the address of the driver's input-pending flag in *inputFlagPtr and installs mtrCallback
 * (see MTRCallbackType); implemented in DebuggerDriver.c. */
void DBInitComm(u8 **inputFlagPtr, MTRCallbackType mtrCallback);
void __DBExceptionDestination(void);
void __DBExceptionDestinationAux(void);
BOOL __DBIsExceptionMarked(__OSException exception);
void __DBMarkException(u8 exception, int value);
void __DBSetPresent(u32 value);

/* DebuggerDriver.c */
void DBOpen(void);
void DBClose(void);
BOOL DBWrite(void *src, u32 size);
BOOL DBRead(u32 *buffer, s32 count);
u32 DBQueryData(void);
void DBInitInterrupts(void);
/* Stores the address of the driver's input-pending flag in *inputFlagPtr and installs mtrCallback
 * (called with an int when input arrives). */

#ifdef __cplusplus
}
#endif

#endif
